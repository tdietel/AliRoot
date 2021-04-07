// $Id$

//**************************************************************************
//* This file is property of and copyright by the ALICE HLT Project        * 
//* ALICE Experiment at CERN, All rights reserved.                         *
//*                                                                        *
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *
//*                  for The ALICE HLT Project.                            *
//*                                                                        *
//* Permission to use, copy, modify and distribute this software and its   *
//* documentation strictly for non-commercial purposes is hereby granted   *
//* without fee, provided that the above copyright notice appears in all   *
//* copies and that both the copyright notice and this permission notice   *
//* appear in the supporting documentation. The authors make no claims     *
//* about the suitability of this software for any purpose. It is          *
//* provided "as is" without express or implied warranty.                  *
//**************************************************************************

/// @file   AliHLTTPCTrackGeometry.cxx
/// @author Matthias Richter
/// @date   2011-05-20
/// @brief  Desciption of a track by a sequence of track points
///

#include "AliHLTTPCTrackGeometry.h"
#include "AliHLTTPCGeometry.h"
#include "AliHLTTPCSpacePointData.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliHLTTPCSpacePointContainer.h"
#include "AliHLTTPCRawSpacePointContainer.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTComponent.h"
#include "AliHLTGlobalBarrelTrack.h"
#include "AliHLTDataDeflater.h"
#include "AliHLTErrorGuard.h"
#include "TMath.h"
#include "TH2F.h"
#include <memory>
#include <sstream>
using namespace std;

#if __cplusplus > 201402L
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTPCTrackGeometry)

AliHLTTPCTrackGeometry::AliHLTTPCTrackGeometry()
  : AliHLTTrackGeometry()
  , fRawTrackPoints()
  , fDriftTimeFactorA(0.)
  , fDriftTimeOffsetA(0.)
  , fDriftTimeFactorC(0.)
  , fDriftTimeOffsetC(0.)
{
  /// standard constructor
}

AliHLTTPCTrackGeometry::AliHLTTPCTrackGeometry(const AliHLTTPCTrackGeometry& src)
  : AliHLTTrackGeometry(src)
  , fRawTrackPoints(src.fRawTrackPoints)
  , fDriftTimeFactorA(0.)
  , fDriftTimeOffsetA(0.)
  , fDriftTimeFactorC(0.)
  , fDriftTimeOffsetC(0.)
{
  /// copy constructor
}

AliHLTTPCTrackGeometry& AliHLTTPCTrackGeometry::operator=(const AliHLTTPCTrackGeometry& src)
{
  /// assignment operator
  AliHLTTrackGeometry::operator=(src);
  fRawTrackPoints.assign(src.fRawTrackPoints.begin(), src.fRawTrackPoints.end());
  return *this;
}

AliHLTTPCTrackGeometry::~AliHLTTPCTrackGeometry()
{
  /// destructor
}

float AliHLTTPCTrackGeometry::GetPlaneAlpha(AliHLTUInt32_t planeId) const
{
  /// alpha of the plane
  UInt_t slice=AliHLTTPCGeometry::CluID2Slice(planeId);
  float alpha=( slice + 0.5 ) * TMath::Pi() / 9.0;
  if (alpha>TMath::TwoPi()) alpha-=TMath::TwoPi();
  return alpha;
}

float AliHLTTPCTrackGeometry::GetPlaneR(AliHLTUInt32_t planeId) const
{
  /// radial distance from global {0,0,0}
  UInt_t partition=AliHLTTPCGeometry::CluID2Partition(planeId);
  UInt_t number=AliHLTTPCGeometry::CluID2Index(planeId);
  Int_t row=AliHLTTPCGeometry::GetFirstRow(partition)+number;
  return AliHLTTPCGeometry::Row2X(row);
}

float AliHLTTPCTrackGeometry::GetPlaneTheta(AliHLTUInt32_t /*planeId*/) const
{
  /// theta of the plane
  return 0.0;
}

bool AliHLTTPCTrackGeometry::CheckBounds(AliHLTUInt32_t planeId, float u, float /*v*/) const
{
  /// check bounds in u and v coordinate
  float r=GetPlaneR(planeId);
  if (r<AliHLTTPCGeometry::GetFirstRow(0)) return false;

  // TODO: check if the pad width needs to be considered here
  return TMath::Abs(TMath::ASin(u/r))<=TMath::Pi()/18;
}

int AliHLTTPCTrackGeometry::CalculateTrackPoints(const AliHLTExternalTrackParam& track)
{
  /// calculate the track points, expects the global magnetic field to be initialized
  AliHLTGlobalBarrelTrack bt(track);
  return CalculateTrackPoints(bt);
}

int AliHLTTPCTrackGeometry::CalculateTrackPoints(AliHLTGlobalBarrelTrack& track)
{
  /// calculate the track points, expects the global magnetic field to be initialized
  /// depending on the x coordinate of the first track point the corresponding padrow
  /// is searched and the points are calculated outwards. Eventually the points are
  /// calculated inwards as a second step.
  int iResult=0;
  int firstpadrow=0;
  for (;
       firstpadrow<AliHLTTPCGeometry::GetNRows() && 
	 AliHLTTPCGeometry::Row2X(firstpadrow)+AliHLTTPCGeometry::GetPadLength(firstpadrow)<track.GetX();
       firstpadrow++);
  if (firstpadrow>=AliHLTTPCGeometry::GetNRows()) return 0;
  // first calculated outwards
  iResult=CalculateTrackPoints(track, firstpadrow, 1);
  if (iResult>=0 && firstpadrow>0)
    // now calculate inwards
    iResult=CalculateTrackPoints(track, firstpadrow-1, -1);
  return iResult;
}

int AliHLTTPCTrackGeometry::CalculateTrackPoints(AliHLTGlobalBarrelTrack& track, int firstpadrow, int step)
{
  /// calculate the track points, expects the global magnetic field to be initialized
  float offsetAlpha=0.0;
  for (int padrow=firstpadrow; padrow>=0 && padrow<AliHLTTPCGeometry::GetNRows(); padrow+=step) {
    float x=AliHLTTPCGeometry::Row2X(padrow);
    float y=0.0;
    float z=0.0;

    int maxshift=9;
    int shift=0;
    int result=0;
    do {
      // start calculation of crossing points with padrow planes in the slice of the first point
      // plane alpha corresponds to alpha of the track, switch to neighboring slice if the result
      // is out of bounds
      if ((result=track.CalculateCrossingPoint(x, track.GetAlpha()-offsetAlpha, y, z))<1) break;
      float pointAlpha=TMath::ATan(y/x);
      if (TMath::Abs(pointAlpha)>TMath::Pi()/18) {
	offsetAlpha+=(pointAlpha>0?-1:1)*TMath::Pi()/9;
      	result=0;
      }
    } while (result==0 && shift++<maxshift);
    if (result<1) continue;
    float planealpha=track.GetAlpha()-offsetAlpha;
    if (planealpha<0) planealpha+=TMath::TwoPi();
    if (planealpha>TMath::TwoPi()) planealpha-=TMath::TwoPi();
    int slice=int(9*planealpha/TMath::Pi());
    if (z<0) slice+=18;
    if (slice>=36) {
      HLTError("invalid slice %d calculated from alpha %f", slice, track.GetAlpha());
    }
    int partition=AliHLTTPCGeometry::GetPatch(padrow);
    int row=padrow-AliHLTTPCGeometry::GetFirstRow(partition);
    UInt_t id=AliHLTTPCGeometry::CreateClusterID(slice, partition, row);
    if (TMath::Abs(planealpha-GetPlaneAlpha(id))>0.0001) {
      HLTError("alpha missmatch for plane %08x (slice %d): alpha from id %f (%.0f deg), expected %f (%.0f deg)", id, slice, GetPlaneAlpha(id), 180*GetPlaneAlpha(id)/TMath::Pi(), planealpha, 180*planealpha/TMath::Pi());
    }
    if (AddTrackPoint(AliHLTTrackPoint(id, y, z), AliHLTTPCGeometry::CreateClusterID(slice, partition, 0))>=0) {
      Float_t rpt[3]={0.,y,z}; // row pad time
      AliHLTTPCGeometry::LocHLT2Raw(rpt, slice, padrow);
      float m=fDriftTimeFactorA;
      float n=fDriftTimeOffsetA;
      if (slice>=18) {
	m=fDriftTimeFactorC;
	n=fDriftTimeOffsetC;
      }
      if (TMath::Abs(m)>0.) {
      	rpt[2]=(z-n)/m;
	if (step>0) {
	  // ascending padrows added at end
	  fRawTrackPoints.push_back(AliHLTTrackPoint(id, rpt[1], rpt[2]));
	} else {
	  // descending padrows added at begin
	  fRawTrackPoints.insert(fRawTrackPoints.begin(), AliHLTTrackPoint(id, rpt[1], rpt[2]));
	}
	// FIXME: implement a Print function for the raw track points
	// stringstream sout;
	// sout << "  slice "  << setfill(' ') << setw(3) << fixed << right << slice
	//      << "  row "    << setfill(' ') << setw(3) << fixed << right << padrow
	//      << "  id 0x"     << hex << setw(8) << id << dec
	//      << "  y "      << setfill(' ') << setw(5) << fixed << right << setprecision (2) << y
	//      << "  z "      << setfill(' ') << setw(5) << fixed << right << setprecision (2) << z
	//      << "  pad "    << setfill(' ') << setw(5) << fixed << right << setprecision (2) << rpt[1]
	//      << "  time "   << setfill(' ') << setw(5) << fixed << right << setprecision (2) << rpt[2]
	//      << endl;
	// cout << sout.str();
      } else {
	ALIHLTERRORGUARD(1, "drift time correction not initialized, can not add track points in raw coordinates");
      }
    }
  }
  return 0;
}

int AliHLTTPCTrackGeometry::FindMatchingTrackPoint(AliHLTUInt32_t spacepointId, float spacepoint[3], AliHLTUInt32_t& planeId)
{
  /// find the track point which can be associated to a spacepoint with coordinates and id
  UInt_t slice=AliHLTTPCGeometry::CluID2Slice(spacepointId);
  UInt_t partition=AliHLTTPCGeometry::CluID2Partition(spacepointId);
  int row=AliHLTTPCGeometry::GetPadRow(spacepoint[0]);
  bool bSpecialRow=row==30 || row==90 || row==139;
  if (row<AliHLTTPCGeometry::GetFirstRow(partition) || row>AliHLTTPCGeometry::GetLastRow(partition)) {
    HLTError("row number %d calculated from x value %f is outside slice %d partition %d", row, spacepoint[0], slice, partition);
    return -EINVAL;
  }

  // find the crossing point of the track with the padrow plane where
  // the spacepoint is
  // 1) calculate plane id from slice, partition and row (within partition)
  row-=AliHLTTPCGeometry::GetFirstRow(partition);
  UInt_t id=AliHLTTPCGeometry::CreateClusterID(slice, partition, row);
  const AliHLTTrackPoint* point=GetTrackPoint(id);
  // track might be outside the partition and cross the central membrane
  // search in the other half of the TPC
  if (!point && slice<18) {
    // search in the neighboring partition on the C side
    id=AliHLTTPCGeometry::CreateClusterID(slice+18, partition, row);
    point=GetTrackPoint(id);
  } else if (!point && slice>=18) {
    // search in the neighboring partition on the A side
    id=AliHLTTPCGeometry::CreateClusterID(slice-18, partition, row);
    point=GetTrackPoint(id);
  }
  
  // search in the neighboring partition, this takes account for rows
  // 30, 90, and 139 which are partly in one and the other partition
  if (!point && bSpecialRow) {
    row+=AliHLTTPCGeometry::GetFirstRow(partition);
    row-=AliHLTTPCGeometry::GetFirstRow(partition-1);
    id=AliHLTTPCGeometry::CreateClusterID(slice, partition-1, row);
    point=GetTrackPoint(id);
    if (!point && slice<18) {
      // search in the neighboring partition on the C side
      id=AliHLTTPCGeometry::CreateClusterID(slice+18, partition-1, row);
      point=GetTrackPoint(id);
    } else if (!point && slice>=18) {
      // search in the neighboring partition on the A side
      id=AliHLTTPCGeometry::CreateClusterID(slice-18, partition-1, row);
      point=GetTrackPoint(id);
    }
  }

  if (point) {
    planeId=id;
    if (point->HaveAssociatedSpacePoint()) {
      if (GetVerbosity()>2) HLTInfo("descarding spacepoint 0x%08x z=%f y=%f z=%f: track point 0x%08x already occupied", spacepoint[0], spacepoint[1], spacepoint[2], planeId);
      return 0; // already occupied
    }
    float maxdy=2.;
    float maxdz=2.;
    if (TMath::Abs(point->GetU()-spacepoint[1])>maxdy) {
      if (GetVerbosity()>0) HLTInfo("descarding spacepoint 0x%08x y=%f z=%f: track point 0x%08x y %f outside tolerance %f", spacepoint[1], spacepoint[2], planeId, point->GetU(), maxdy);
      return -ENOENT;
    }
    if (TMath::Abs(point->GetV()-spacepoint[2])>maxdz) {
      if (GetVerbosity()>0) HLTInfo("descarding spacepoint 0x%08x y=%f z=%f: track point 0x%08x z %f outside tolerance %f", spacepoint[1], spacepoint[2], planeId, point->GetV(), maxdz);
      return -ENOENT;
    }
    return 1;
  }
  return -ENOENT;
}


int AliHLTTPCTrackGeometry::RegisterTrackPoints(AliHLTTrackGrid* pGrid) const
{
  /// register track points in the index grid, at this step the number
  /// of tracks in each cell is counted
  if (!pGrid) return -EINVAL;
  int iResult=0;
  for (vector<AliHLTTrackPoint>::const_iterator tp=TrackPoints().begin();
       tp!=TrackPoints().end() && iResult>=0; tp++) {
    AliHLTUInt32_t id=tp->GetId();
    iResult=pGrid->CountSpacePoint(AliHLTTPCGeometry::CluID2Slice(id),
				   AliHLTTPCGeometry::CluID2Partition(id),
				   AliHLTTPCGeometry::CluID2Index(id));
  }
  return iResult;
}

int AliHLTTPCTrackGeometry::FillTrackPoints(AliHLTTrackGrid* pGrid) const
{
  /// fill track points to index grid
  if (!pGrid) return -EINVAL;
  int iResult=0;
  for (vector<AliHLTTrackPoint>::const_iterator tp=TrackPoints().begin();
       tp!=TrackPoints().end() && iResult>=0; tp++) {
    AliHLTUInt32_t id=tp->GetId();
    iResult=pGrid->AddSpacePoint(GetTrackId(),
				 AliHLTTPCGeometry::CluID2Slice(id),
				 AliHLTTPCGeometry::CluID2Partition(id),
				 AliHLTTPCGeometry::CluID2Index(id));
  }
  return iResult;
}

AliHLTSpacePointContainer* AliHLTTPCTrackGeometry::ConvertToSpacePoints(bool bAssociated) const
{
  /// create a collection of all points
  AUTO_PTR<AliHLTTPCSpacePointContainer> spacepoints(new AliHLTTPCSpacePointContainer);
  if (!spacepoints.get()) return NULL;

  const vector<AliHLTTrackPoint>& trackPoints=TrackPoints();
  unsigned i=0;
  while (i<trackPoints.size()) {
    // allocate buffer for all points, even though the buffer might not be filled
    // completely because of a partition change
    int nofPoints=trackPoints.size()-i;
    int blocksize=sizeof(AliHLTTPCClusterData)+nofPoints*sizeof(AliHLTTPCSpacePointData);
    AliHLTUInt8_t* pBuffer=spacepoints->Alloc(blocksize);
    if (!pBuffer) return NULL;
    AliHLTTPCClusterData* pClusterData=reinterpret_cast<AliHLTTPCClusterData*>(pBuffer);
    pClusterData->fSpacePointCnt=0;
    AliHLTTPCSpacePointData* pClusters=pClusterData->fSpacePoints;
    int currentSlice=-1;
    int currentPartition=-1;
    for (; i<trackPoints.size(); i++) {
      if (bAssociated && !trackPoints[i].HaveAssociatedSpacePoint()) continue;
      AliHLTUInt32_t planeId=trackPoints[i].GetId();
      int slice=AliHLTTPCGeometry::CluID2Slice(planeId);
      int partition=AliHLTTPCGeometry::CluID2Partition(planeId);
      int number=AliHLTTPCGeometry::CluID2Index(planeId);
      if ((currentSlice>=0 && currentSlice!=slice) || (currentPartition>=0 && currentPartition!=partition)) {
	// change of partition or slice, need to go to next block
	// 2011-07-26 currently all spacepoints go into one block, if separated
	// blocks per partition are needed one has to leave the inner loop here
	// and set the data block specification below
	// Caution: not tested, only the last block seems to make it through
	//break;
      }
      currentSlice=slice;
      currentPartition=partition;
      pClusters[pClusterData->fSpacePointCnt].fX=GetPlaneR(planeId);
      pClusters[pClusterData->fSpacePointCnt].fY=trackPoints[i].GetU();
      pClusters[pClusterData->fSpacePointCnt].fZ=trackPoints[i].GetV();
      pClusters[pClusterData->fSpacePointCnt].fRawID=planeId;
      pClusters[pClusterData->fSpacePointCnt].fPadRow=AliHLTTPCGeometry::GetFirstRow(partition)+number;
      pClusters[pClusterData->fSpacePointCnt].fSigmaY2=0.;
      pClusters[pClusterData->fSpacePointCnt].fSigmaZ2=0.;
      pClusters[pClusterData->fSpacePointCnt].fCharge=0;
      pClusters[pClusterData->fSpacePointCnt].fQMax=0;
      pClusters[pClusterData->fSpacePointCnt].fUsed=0;
      pClusters[pClusterData->fSpacePointCnt].fTrackN=0;
      pClusterData->fSpacePointCnt++;
    }
    AliHLTComponentBlockData bd;
    AliHLTComponent::FillBlockData(bd);
    bd.fPtr=pBuffer;
    bd.fSize=sizeof(AliHLTTPCClusterData)+pClusterData->fSpacePointCnt*sizeof(AliHLTTPCSpacePointData);
    AliHLTComponent::SetDataType(bd.fDataType, "CLUSTERS", "TPC ");
    bd.fSpecification=kAliHLTVoidDataSpec;//AliHLTTPCDefinitions::EncodeDataSpecification(currentSlice, currentSlice, currentPartition, currentPartition);
    spacepoints->AddInputBlock(&bd);
  }

  return spacepoints.release();
}

const AliHLTTrackGeometry::AliHLTTrackPoint* AliHLTTPCTrackGeometry::GetRawTrackPoint(AliHLTUInt32_t id) const
{
  /// get raw track point of id
  const AliHLTTrackGeometry::AliHLTTrackPoint* p=find(&fRawTrackPoints[0], &fRawTrackPoints[fRawTrackPoints.size()], id);
  if (p==&fRawTrackPoints[fRawTrackPoints.size()]) return 0;
  return p;
}

AliHLTTrackGeometry::AliHLTTrackPoint* AliHLTTPCTrackGeometry::GetRawTrackPoint(AliHLTUInt32_t id)
{
  /// get raw track point of id
  AliHLTTrackGeometry::AliHLTTrackPoint* p=find(&fRawTrackPoints[0], &fRawTrackPoints[fRawTrackPoints.size()], id);
  if (p==&fRawTrackPoints[fRawTrackPoints.size()]) return 0;
  return p;
}

int AliHLTTPCTrackGeometry::FillRawResidual(int coordinate, TH2* histo, AliHLTSpacePointContainer* points) const
{
  // fill residual histogram
  if (!histo || !points) return -EINVAL;
  const vector<AliHLTTrackPoint>& trackPoints=TrackPoints();
  for (vector<AliHLTTrackPoint>::const_iterator trackpoint=trackPoints.begin();
       trackpoint!=trackPoints.end(); trackpoint++) {
    if (!trackpoint->HaveAssociatedSpacePoint()) continue;
    for (vector<AliHLTTrackSpacepoint>::const_iterator sp=(trackpoint->GetSpacepoints()).begin();
	 sp!=(trackpoint->GetSpacepoints()).end(); sp++) {
    AliHLTUInt32_t spacepointId=sp->fId;
    vector<AliHLTTrackPoint>::const_iterator rawpoint=find(fRawTrackPoints.begin(), fRawTrackPoints.end(), trackpoint->GetId());
    if (rawpoint==fRawTrackPoints.end()) {
      HLTError("can not find track raw coordinates of track point 0x%08x", trackpoint->GetId());
      continue;
    }
    if (!points->Check(spacepointId)) {
      //HLTError("can not find associated space point 0x%08x of track point 0x%08x", spacepointId, trackpoint->GetId());
      continue;
    }
    float value=0.;
    if (coordinate==0) {
      value=rawpoint->GetU()-points->GetY(spacepointId);
      histo->Fill(GetPlaneR(trackpoint->GetId()), value);
    } else {
      value=rawpoint->GetV()-points->GetZ(spacepointId);
      //histo->Fill(GetPlaneR(trackpoint->GetId()), value);
      histo->Fill(rawpoint->GetV(), value);
    }
    }
  }
  return 0;
}

int AliHLTTPCTrackGeometry::Write(const AliHLTGlobalBarrelTrack& track,
				  AliHLTSpacePointContainer* pSpacePoints,
				  AliHLTDataDeflater* pDeflater,
				  AliHLTUInt8_t* outputPtr,
				  AliHLTUInt32_t size,
				  vector<AliHLTUInt32_t>* writtenClusterIds,
				  const char* option) const
{
  // write track block to buffer
  if (size<=sizeof(AliHLTTPCTrackBlock)) return -ENOSPC;
  AliHLTTPCTrackBlock* pTrackBlock=reinterpret_cast<AliHLTTPCTrackBlock*>(outputPtr);
  pTrackBlock->fSize=sizeof(AliHLTTPCTrackBlock); // size of cluster block added later
  float alpha=track.GetAlpha();
  while (alpha<0.) alpha+=TMath::TwoPi();
  while (alpha>TMath::TwoPi()) alpha-=TMath::TwoPi();
  pTrackBlock->fSlice=AliHLTUInt8_t(9*alpha/TMath::Pi());
  if (pTrackBlock->fSlice>=36) {
    HLTError("invalid slice %d calculated from alpha %f", pTrackBlock->fSlice, track.GetAlpha());
  }
  pTrackBlock->fReserved=0;
  pTrackBlock->fX      = track.GetX();
  pTrackBlock->fY      = track.GetY();
  pTrackBlock->fZ      = track.GetZ();
  pTrackBlock->fSinPhi = track.GetSnp();
  pTrackBlock->fTgl    = track.GetTgl();
  pTrackBlock->fq1Pt   = track.GetSigned1Pt();

  pDeflater->Clear();
  pDeflater->InitBitDataOutput(reinterpret_cast<AliHLTUInt8_t*>(outputPtr+sizeof(AliHLTTPCTrackBlock)), size-sizeof(AliHLTTPCTrackBlock));
  int result=WriteAssociatedClusters(pSpacePoints, pDeflater, writtenClusterIds, option);
  if (result<0) return result;
  pTrackBlock->fSize+=result;
  return pTrackBlock->fSize;
}

int AliHLTTPCTrackGeometry::WriteAssociatedClusters(AliHLTSpacePointContainer* pSpacePoints,
						    AliHLTDataDeflater* pDeflater,
						    vector<AliHLTUInt32_t>* writtenClusterIds,
						    const char* /*option*/) const
{
  // write associated clusters to buffer via deflater
  if (!pDeflater || !pSpacePoints) return -EINVAL;
  AliHLTTPCRawSpacePointContainer* pTPCRawSpacePoints=dynamic_cast<AliHLTTPCRawSpacePointContainer*>(pSpacePoints);
  if (!pTPCRawSpacePoints) {
    HLTError("invalid type of SpacePointContainer \"%s\", required AliHLTTPCRawSpacePointContainer", pSpacePoints->ClassName());
    return -EINVAL;
  }
  bool bReverse=true;
  bool bWriteSuccess=true;
  int writtenClusters=0;
  // filling of track points starts from first point on track outwards, and
  // then from that point inwards. That's why the lower padrows might be in
  // reverse order at the end of the track point array. If the last element
  // is bigger than the first element, only trackpoints in ascending order
  // are in the array
  vector<AliHLTTrackPoint>::const_iterator clrow=fRawTrackPoints.end();
  if (clrow!=fRawTrackPoints.begin()) {
    clrow--;
    AliHLTUInt32_t partition=AliHLTTPCGeometry::CluID2Partition(clrow->GetId());
    AliHLTUInt32_t partitionrow=AliHLTTPCGeometry::CluID2Index(clrow->GetId());
    partitionrow+=AliHLTTPCGeometry::GetFirstRow(partition);
    AliHLTUInt32_t firstpartition=AliHLTTPCGeometry::CluID2Partition(fRawTrackPoints.begin()->GetId());
    AliHLTUInt32_t firstpartitionrow=AliHLTTPCGeometry::CluID2Index(fRawTrackPoints.begin()->GetId());
    firstpartitionrow+=AliHLTTPCGeometry::GetFirstRow(firstpartition);
    if (partitionrow>=firstpartitionrow) {
      bReverse=false;
      clrow=fRawTrackPoints.begin();
    }
  }
  const AliHLTUInt32_t clusterCountBitLength=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kClusterCount].fBitLength;
  unsigned long dataPosition=pDeflater->GetCurrentByteOutputPosition();
  for (unsigned row=0; row<159 && bWriteSuccess; row++) {
    if (clrow!=fRawTrackPoints.end()) {
      AliHLTUInt32_t thisPartition=AliHLTTPCGeometry::CluID2Partition(clrow->GetId());
      AliHLTUInt32_t thisTrackRow=AliHLTTPCGeometry::CluID2Index(clrow->GetId());
      thisTrackRow+=AliHLTTPCGeometry::GetFirstRow(thisPartition);
      if (thisTrackRow==row) {
	// write clusters
	const vector<AliHLTTrackSpacepoint>&  clusters=clrow->GetSpacepoints();
	AliHLTUInt32_t haveClusters=clusters.size()>0;
	// 1 bit for clusters on that padrow
	bWriteSuccess=bWriteSuccess && pDeflater->OutputBit(haveClusters);
	if (haveClusters) {
	  bWriteSuccess=bWriteSuccess && pDeflater->OutputBits(clusters.size(), clusterCountBitLength);
	  for (vector<AliHLTTrackSpacepoint>::const_iterator clid=clusters.begin();
	       clid!=clusters.end() && bWriteSuccess; clid++) {
	    if (!pSpacePoints->Check(clid->fId)) {
	      HLTError("can not find spacepoint 0x%08x", clid->fId);
	      continue;
	    }
	    if (writtenClusterIds) {
	      writtenClusterIds->push_back(clid->fId);
	    }

	    // FIXME: there is a bug in the calculation of the residuals stored with the
	    // assiciated space point, calculate again, but needs to be fixed
	    float deltapad =pSpacePoints->GetY(clid->fId)-clrow->GetU();//clid->fdU;
	    float deltatime =pSpacePoints->GetZ(clid->fId)-clrow->GetV();//clid->fdV;
	    if (TMath::Abs(deltapad)>=AliHLTTPCDefinitions::fgkMaxClusterDeltaPad ||
		TMath::Abs(deltatime)>=AliHLTTPCDefinitions::fgkMaxClusterDeltaTime) {
	      AliHLTUInt8_t slice = AliHLTTPCGeometry::CluID2Slice(clid->fId);
	      AliHLTUInt8_t partition = AliHLTTPCGeometry::CluID2Partition(clid->fId);
	      HLTFatal("cluster 0x%08x slice %d partition %d: residual out of range - pad %f (max %d), time %f (max %d)", clid->fId, slice, partition, deltapad, AliHLTTPCDefinitions::fgkMaxClusterDeltaPad, deltatime, AliHLTTPCDefinitions::fgkMaxClusterDeltaTime);
	    }
	    float sigmaY2=pSpacePoints->GetYWidth(clid->fId);
	    float sigmaZ2=pSpacePoints->GetZWidth(clid->fId);
	    AliHLTUInt64_t charge=(AliHLTUInt64_t)pSpacePoints->GetCharge(clid->fId);
	    AliHLTUInt64_t qmax=(AliHLTUInt64_t)pTPCRawSpacePoints->GetQMax(clid->fId);
	    // cout << "  row "    << setfill(' ') << setw(3) << fixed << right << row
	    // 	 << "  pad "    << setfill(' ') << setw(7) << fixed << right << setprecision (4) << pSpacePoints->GetY(clid->fId)
	    // 	 << "  dpad "   << setfill(' ') << setw(7) << fixed << right << setprecision (4) << deltapad
	    // 	 << "  time "   << setfill(' ') << setw(7) << fixed << right << setprecision (4) << pSpacePoints->GetZ(clid->fId)
	    // 	 << "  dtime "  << setfill(' ') << setw(7) << fixed << right << setprecision (4) << deltatime
	    // 	 << "  charge " << setfill(' ') << setw(5) << fixed << right << charge
	    // 	 << "  qmax "   << setfill(' ') << setw(4) << fixed << right << qmax
	    // 	 << endl;

	    // time and pad coordinates are scaled and transformed to integer values for
	    // both cluster and track point before calculating the residual. this makes
	    // the compression lossless with respect to the format without track model
	    // compression
	    AliHLTUInt64_t deltapad64=0;
	    AliHLTUInt32_t signDeltaPad=0;
	    if (!isnan(deltapad)) {
	      double clusterpad=pSpacePoints->GetY(clid->fId);
	      double trackpad=clrow->GetU();
	      if (clusterpad<0.) {
		HLTError("cluster 0x%08x has negative pad position", clid->fId, clusterpad);
	      }
	      clusterpad*=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kResidualPad].fScale;
	      trackpad*=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kResidualPad].fScale;
	      AliHLTUInt64_t clusterpad64=(AliHLTUInt64_t)round(clusterpad);
	      AliHLTUInt64_t trackpad64=0;
	      if (trackpad>0.) trackpad64=(AliHLTUInt64_t)round(trackpad);
	      if (clusterpad64<trackpad64) {
		deltapad64=trackpad64-clusterpad64;
		signDeltaPad=1;
	      } else {
		deltapad64=clusterpad64-trackpad64;
		signDeltaPad=0;
	      }
	    }
	    AliHLTUInt64_t deltatime64=0;
	    AliHLTUInt32_t signDeltaTime=0;
	    if (!isnan(deltatime)) {
	      double clustertime=pSpacePoints->GetZ(clid->fId);
	      double tracktime=clrow->GetV();
	      if (clustertime<0.) {
		HLTError("cluster 0x%08x has negative time position", clid->fId, clustertime);
	      }
	      clustertime*=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kResidualTime].fScale;
	      tracktime*=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kResidualTime].fScale;
	      AliHLTUInt64_t clustertime64=(AliHLTUInt64_t)round(clustertime);
	      AliHLTUInt64_t tracktime64=0;
	      if (tracktime>0.) tracktime64=(AliHLTUInt64_t)round(tracktime);
	      if (clustertime64<tracktime64) {
		deltatime64=tracktime64-clustertime64;
		signDeltaTime=1;
	      } else {
		deltatime64=clustertime64-tracktime64;
		signDeltaTime=0;
	      }
	    }
	    AliHLTUInt64_t sigmaY264=0;
	    if (!isnan(sigmaY2)) sigmaY264=(AliHLTUInt64_t)round(sigmaY2*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fScale);
	    // we can safely use the upper limit as this is an unphysical cluster, no impact to physics
	    if (sigmaY264 >= (unsigned)1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fBitLength) {
	      sigmaY264 = (1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fBitLength)-1;
	    }
	    AliHLTUInt64_t sigmaZ264=0;
	    if (!isnan(sigmaZ2)) sigmaZ264=(AliHLTUInt64_t)round(sigmaZ2*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fScale);
	    // we can safely use the upper limit as this is an unphysical cluster, no impact to physics
	    if (sigmaZ264 >= (unsigned)1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fBitLength) {
	      sigmaZ264 = (1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fBitLength)-1;
	    }
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kResidualPad    , deltapad64);  
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputBit(signDeltaPad);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kResidualTime   , deltatime64);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputBit(signDeltaTime);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kSigmaY2        , sigmaY264);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kSigmaZ2        , sigmaZ264);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kCharge         , charge);
	    bWriteSuccess=bWriteSuccess && pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kQMax           , qmax);
	    if (bWriteSuccess) writtenClusters++;
	  }
	}

	// set to next trackpoint
	if (bReverse) {
	  if (clrow!=fRawTrackPoints.begin()) {
	    AliHLTUInt32_t nextPartition=AliHLTTPCGeometry::CluID2Partition((clrow-1)->GetId());
	    AliHLTUInt32_t nextTrackRow=AliHLTTPCGeometry::CluID2Index((clrow-1)->GetId());
	    nextTrackRow+=AliHLTTPCGeometry::GetFirstRow(nextPartition);
	    if (thisTrackRow+1==nextTrackRow) {
	      clrow--;
	    } else {
	      // switch direction start from beginning
	      clrow=fRawTrackPoints.begin();
	      bReverse=false;
	    }
	  } else {
	    // all trackpoints processed
	    clrow=fRawTrackPoints.end();
	  }
	} else {
	  clrow++;
	}
	continue;
      } else {
	// sequence not ordered, search
	// this has been fixed and the search is no longer necessary
	// for (clrow=fRawTrackPoints.begin(); clrow!=fRawTrackPoints.end(); clrow++) {
	//   if ((AliHLTTPCGeometry::CluID2Index(clrow->GetId())+AliHLTTPCGeometry::GetFirstRow(AliHLTTPCGeometry::CluID2Partition(clrow->GetId())))==row) break;
	// }
	// if (clrow==fRawTrackPoints.end()) {
	//   clrow=fRawTrackPoints.begin();
	//   HLTWarning("no trackpoint on row %d, current point %d", row, thisTrackRow);
	// }
      }
    }
    // no cluster on that padrow
    AliHLTUInt32_t haveClusters=0;
    bWriteSuccess=bWriteSuccess && pDeflater->OutputBit(haveClusters);
  }

  if (!bWriteSuccess) {
    // TODO: code review 2015-02-10 Matthias.Richter@scieq.net
    // misleading error code, there are two reasons for failed write operation
    // 1) target buffer overflow -> -ENOSPC
    // 2) value range excess -> -ERANGE
    return -ENOSPC;
  }

  int allClusters=0;
  for (clrow=fRawTrackPoints.begin(); clrow!=fRawTrackPoints.end(); clrow++) {
    allClusters+=clrow->GetSpacepoints().size();
  }
  if (allClusters!=writtenClusters) {
    HLTError("track %d mismatch in written clusters: %d but expected %d", GetTrackId(), writtenClusters, allClusters);
  }

  pDeflater->Pad8Bits();
  return pDeflater->GetCurrentByteOutputPosition()-dataPosition;
}

int AliHLTTPCTrackGeometry::Read(const AliHLTUInt8_t* buffer,
				 AliHLTUInt32_t size,
				 float bz,
				 AliHLTUInt32_t& clusterBlockSize,
				 const char* /*option*/)
{
  // read track block from buffer
  int iResult=0;
  if (!buffer) return -EINVAL;
  if (size<sizeof(AliHLTTPCTrackBlock)) {
    HLTError("buffer does not contain valid data of track model clusters");
    return -ENODATA;
  }
  const AliHLTTPCTrackBlock* pTrackBlock=reinterpret_cast<const AliHLTTPCTrackBlock*>(buffer);
  if (pTrackBlock->fSize>size) {
    HLTError("inconsistent track data block of size %d exceeds available buffer of size %d", pTrackBlock->fSize, size);
    return -ENODATA;
  }
  if (pTrackBlock->fSize<sizeof(AliHLTTPCTrackBlock)) {
    HLTError("inconsistent size of track data block specified in the header: %d", pTrackBlock->fSize);
    return -ENODATA;
  }
  AliHLTExternalTrackParam param;
  memset(&param, 0, sizeof(param));
  param.fAlpha   =( pTrackBlock->fSlice + 0.5 ) * TMath::Pi() / 9.0;
  if (param.fAlpha>TMath::TwoPi()) param.fAlpha-=TMath::TwoPi();
  param.fX       = pTrackBlock->fX;
  param.fY       = pTrackBlock->fY;
  param.fZ       = pTrackBlock->fZ;
  param.fSinPhi  = pTrackBlock->fSinPhi;
  param.fTgl     = pTrackBlock->fTgl;
  param.fq1Pt    = pTrackBlock->fq1Pt;
  AliHLTGlobalBarrelTrack track(param);
  if ((iResult=track.CalculateHelixParams(bz))<0) {
    HLTError("failed to calculate helix params: %d", iResult);
    return iResult;
  }
  if ((iResult=CalculateTrackPoints(track))<0) {
    HLTError("failed to calculate track points: %d", iResult);
    return iResult;
  }
  clusterBlockSize=pTrackBlock->fSize-sizeof(AliHLTTPCTrackBlock);
  return sizeof(AliHLTTPCTrackBlock);
}
