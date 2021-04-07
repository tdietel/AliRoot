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

/// @file   AliHLTTPCHWCFSpacePointContainer.cxx
/// @author Matthias Richter
/// @date   2011-08-08
/// @brief  Helper class for handling of HLT TPC cluster data blocks from the
///         HW ClusterFinder
/// @note   Class is a duplicate of AliHLTTPCHWCFSpacePointContainer and should
///         be merged with it in a generic way

#include "AliHLTTPCHWCFSpacePointContainer.h"
#include "AliHLTErrorGuard.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCRawCluster.h"
#include "AliHLTTPCGeometry.h"
#include "AliHLTComponent.h"
#include "AliHLTTemplates.h"
#include "AliHLTDataDeflater.h"
#include "AliHLTCDHWrapper.h"
#include "AliLog.h"
#include "TMath.h"
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

#if __cplusplus > 201402L
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTTPCHWCFSpacePointContainer)

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointContainer(int mode)
  : AliHLTSpacePointContainer()
  , fClusters()
  , fSelections()
  , fBlocks()
  , fSingleBlock()
  , fMode(mode)
  , fWrittenClusterIds(NULL)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
  if (fMode&kModeSingle) {
    fSingleBlock.SetDecoder(new AliHLTTPCHWCFData);
    fSingleBlock.SetGrid(AllocateIndexGrid());
  }
}

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointContainer(const AliHLTTPCHWCFSpacePointContainer& c)
  : AliHLTSpacePointContainer(c)
  , fClusters(c.fClusters.begin(), c.fClusters.end())
  , fSelections()
  , fBlocks()
  , fSingleBlock()
  , fMode(c.fMode)
  , fWrittenClusterIds(NULL)
{
  /// copy constructor
}

AliHLTTPCHWCFSpacePointContainer& AliHLTTPCHWCFSpacePointContainer::operator=(const AliHLTTPCHWCFSpacePointContainer& c)
{
  /// assignment operator
  if (&c==this) return *this;
  AliHLTSpacePointContainer::operator=(c);
  fClusters=c.fClusters;
  fMode=c.fMode;
  fWrittenClusterIds=NULL;

  return *this;
}

AliHLTTPCHWCFSpacePointContainer::~AliHLTTPCHWCFSpacePointContainer()
{
  // destructor
  Clear();
  if (fSingleBlock.GetDecoder()) delete fSingleBlock.GetDecoder();
  if (fSingleBlock.GetGrid()) delete fSingleBlock.GetGrid();
  if (fWrittenClusterIds) delete fWrittenClusterIds;
}

int AliHLTTPCHWCFSpacePointContainer::AddInputBlock(const AliHLTComponentBlockData* pDesc)
{
  // add input block to the collection
  if (!pDesc) return -EINVAL;
  int iResult=0;
  int count=0;
  if (pDesc->fDataType!=AliHLTTPCDefinitions::fgkHWClustersDataType) {
    HLTWarning("ignoring data block of type %s", AliHLTComponent::DataType2Text(pDesc->fDataType).c_str());
    return 0;
  }
  if (!pDesc->fPtr) return -ENODATA;
  AliHLTCDHWrapper header(pDesc->fPtr);
  if (pDesc->fSize<=header.GetHeaderSize()) return 0;

  AliHLTUInt8_t slice = AliHLTTPCDefinitions::GetMinSliceNr( pDesc->fSpecification );
  AliHLTUInt8_t part  = AliHLTTPCDefinitions::GetMinPatchNr( pDesc->fSpecification );

  AliHLTUInt32_t decoderIndex=AliHLTTPCGeometry::CreateClusterID(slice, part, 0);

  AliHLTUInt32_t *buffer=reinterpret_cast<AliHLTUInt32_t*>(pDesc->fPtr);
  // skip the first 8 or 10 CDH words
  buffer += header.GetHeaderSize()/sizeof(AliHLTUInt32_t);
  UInt_t bufferSize32 = ((Int_t)pDesc->fSize - header.GetHeaderSize() )/sizeof(AliHLTUInt32_t);

  AliHLTTPCHWCFData* pDecoder=NULL;
  AliHLTSpacePointPropertyGrid* pGrid=NULL;
  if (fMode&kModeSingle) {
    pDecoder=fSingleBlock.GetDecoder();
    pGrid=fSingleBlock.GetGrid();
  } else {
    if (fBlocks.find(decoderIndex)!=fBlocks.end()) {
      HLTError("data block of slice %d partition %d already added, skipping data block", slice, part);
      return -EEXIST;
    }
  }

  if (!pDecoder) {
    pDecoder=new AliHLTTPCHWCFData;
    if (!pDecoder) return -ENOMEM;
  }

  if (pDecoder->Init(reinterpret_cast<AliHLTUInt8_t*>(buffer), bufferSize32*sizeof(AliHLTUInt32_t))<0 ||
      (pDecoder->CheckVersion()<0 && (int)(bufferSize32*sizeof(AliHLTUInt32_t))>pDecoder->GetRCUTrailerSize())) {
    HLTError("data block of type %s corrupted: can not decode format",
	     AliHLTComponent::DataType2Text(pDesc->fDataType).c_str());
    return -EBADMSG;
  }

  if (fMode&kModeSingle && !pGrid) {
    pGrid=AllocateIndexGrid();
    if (!pGrid) {
      delete pDecoder;
      return -ENOMEM;
    }
  }

  if (fMode&kModeCreateMap) { // register immediately
  UInt_t nofClusters=pDecoder->GetNumberOfClusters();

  for (UInt_t i=0; i<nofClusters; i++) {
    AliHLTUInt32_t clusterID=~(AliHLTUInt32_t)0;
    // cluster ID from slice, partition and index
    clusterID=AliHLTTPCGeometry::CreateClusterID(slice, part, i);

    if (fClusters.find(clusterID)==fClusters.end()) {
      // new cluster
      fClusters[clusterID]=AliHLTTPCHWCFSpacePointProperties(pDecoder, i);
      count++;
    } else {
      HLTError("cluster with ID 0x%08x already existing, skipping cluster %d of data block 0x%08x",
	       clusterID, i, pDesc->fSpecification);
    }
  }
  }

  if (pGrid && (iResult=PopulateAccessGrid(pGrid, pDecoder, slice, part))<0) {
    HLTError("failed to populate access grid for block %s 0x%09x: %d",
	     AliHLTComponent::DataType2Text(pDesc->fDataType).c_str(), pDesc->fSpecification, iResult);
    return iResult;
  }

  if (fMode&kModeSingle) {
    fSingleBlock.SetDecoder(pDecoder);
    fSingleBlock.SetGrid(pGrid);
    fSingleBlock.SetId(decoderIndex);
  } else {
    fBlocks[decoderIndex]=AliHLTTPCHWCFSpacePointBlock(decoderIndex, pDecoder, pGrid);
  }
  return count;
}

AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* AliHLTTPCHWCFSpacePointContainer::AllocateIndexGrid()
{
  // allocate index grid, one single point to define the dimensions
  
  // max 33 padrows, step 1 padrow
  // max 140 pads, step 2x max delta pad
  // max 1024 time bins, step 2x max delta time
  return new AliHLTSpacePointPropertyGrid(33, 1.0,
					  140, 2*AliHLTTPCDefinitions::GetMaxClusterDeltaPad(),
					  1024, 2*AliHLTTPCDefinitions::GetMaxClusterDeltaTime()
					  );
}

int AliHLTTPCHWCFSpacePointContainer::PopulateAccessGrid(AliHLTSpacePointPropertyGrid* pGrid, AliHLTUInt32_t mask) const
{
  // populate an access grid
  if (!pGrid) return -EINVAL;

  pGrid->Clear();
  
  AliHLTUInt8_t slice = AliHLTTPCDefinitions::GetMinSliceNr(mask);
  AliHLTUInt8_t partition  = AliHLTTPCDefinitions::GetMinPatchNr(mask);
  AliHLTUInt32_t decoderIndex=AliHLTTPCGeometry::CreateClusterID(slice, partition, 0);
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointBlock>::const_iterator block=fBlocks.find(decoderIndex);
  if (block==fBlocks.end()) {
    HLTError("can not find data block of id 0x%08x", mask);
    return -ENOENT;
  }
  return PopulateAccessGrid(pGrid, block->second.GetDecoder(), slice, partition);
}

int AliHLTTPCHWCFSpacePointContainer::PopulateAccessGrid(AliHLTSpacePointPropertyGrid* pGrid, AliHLTTPCHWCFData* pDecoder,
							 int slice, int partition) const
{
  // populate an access grid
  if (!pDecoder) return -EINVAL;
  int iResult=0;

  if (pDecoder->GetNumberOfClusters()==0) return 0;
  AliHLTTPCHWCFData::iterator cl=pDecoder->begin();
  for (; cl!=pDecoder->end(); ++cl) {
    iResult=pGrid->CountSpacePoint(cl.GetPadRow(), cl.GetPad(), cl.GetTime());
    if (iResult<0)
      HLTError("CountSpacePoint %f %f %f failed: %d", cl.GetPadRow(), cl.GetPad(), cl.GetTime(), iResult);
  }
  
  int count=0;
  cl=pDecoder->begin();
  for (; cl!=pDecoder->end(); ++cl, count++) {
    AliHLTUInt32_t id=AliHLTTPCGeometry::CreateClusterID(slice, partition, count);
    iResult=pGrid->AddSpacePoint(AliHLTSpacePointProperties(id), cl.GetPadRow(), cl.GetPad(), cl.GetTime());
    if (iResult<0)
      HLTError("AddSpacePoint 0x%08x %f %f %f failed: %d", id, cl.GetPadRow(), cl.GetPad(), cl.GetTime(), iResult);
  }

  return 0;
}

const AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* AliHLTTPCHWCFSpacePointContainer::GetSpacePointPropertyGrid(AliHLTUInt32_t mask) const
{
  // get the access grid for a data block
  AliHLTUInt8_t slice = AliHLTTPCDefinitions::GetMinSliceNr(mask);
  AliHLTUInt8_t part  = AliHLTTPCDefinitions::GetMinPatchNr(mask);
  AliHLTUInt32_t decoderIndex=AliHLTTPCGeometry::CreateClusterID(slice, part, 0);
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointBlock>::const_iterator block=fBlocks.find(decoderIndex);
  if (block==fBlocks.end()) {
    HLTError("can not find data block of id 0x%08x", mask);
    return NULL;
  }
  return block->second.GetGrid();
}

int AliHLTTPCHWCFSpacePointContainer::SetSpacePointPropertyGrid(AliHLTUInt32_t mask, AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pGrid)
{
  // set the access grid for a data block
  AliHLTUInt8_t slice = AliHLTTPCDefinitions::GetMinSliceNr(mask);
  AliHLTUInt8_t part  = AliHLTTPCDefinitions::GetMinPatchNr(mask);
  AliHLTUInt32_t decoderIndex=AliHLTTPCGeometry::CreateClusterID(slice, part, 0);
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointBlock>::iterator block=fBlocks.find(decoderIndex);
  if (block==fBlocks.end()) {
    HLTError("can not find data block of id 0x%08x", mask);
    return -ENOENT;
  }
  if (block->second.GetGrid()!=NULL && pGrid!=NULL && block->second.GetGrid()!=pGrid) {
    // there is trouble ahead because this will delete the index grid instance
    // but it might be an external pointer supposed to be deleted by someone else
    ALIHLTERRORGUARD(1, "overriding previous instance of index grid, potential memory leak or invalid deallocation ahead");
  }
  block->second.SetGrid(pGrid);
  return 0;
}

int AliHLTTPCHWCFSpacePointContainer::GetClusterIDs(vector<AliHLTUInt32_t>& tgt) const
{
  // get array of cluster IDs
  tgt.clear();
  transform(fClusters.begin(), fClusters.end(), back_inserter(tgt), HLT::AliGetKey());
  return tgt.size();
}

bool AliHLTTPCHWCFSpacePointContainer::Check(AliHLTUInt32_t clusterID) const
{
  // check if the cluster is available
  return fClusters.find(clusterID)!=fClusters.end();
}

const vector<AliHLTUInt32_t>* AliHLTTPCHWCFSpacePointContainer::GetClusterIDs(AliHLTUInt32_t mask)
{
  // get array of cluster IDs filtered by mask
  if (fSelections.find(mask)!=fSelections.end()) {
    // return existing selection
    return fSelections.find(mask)->second;
  }
  // create new collection
  vector<AliHLTUInt32_t>* selected=new vector<AliHLTUInt32_t>;
  if (!selected) return NULL;
  UInt_t slice=AliHLTTPCGeometry::CluID2Slice(mask);
  UInt_t partition=AliHLTTPCGeometry::CluID2Partition(mask);
  //HLTInfo("creating collection 0x%08x", mask);

  // the first cluster with number 0 has equal ID to mask unless
  // the mask selects multiple partitions/slices
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(mask);
  bool bAll=false;
  if (slice>=(unsigned)AliHLTTPCGeometry::GetNSlice() ||
      partition>=(unsigned)AliHLTTPCGeometry::GetNumberOfPatches()) {
    cl=fClusters.begin();
    bAll=true;
  }
  for (; cl!=fClusters.end(); cl++) {
    UInt_t s=AliHLTTPCGeometry::CluID2Slice(cl->first);
    UInt_t p=AliHLTTPCGeometry::CluID2Partition(cl->first);
    if ((slice>=(unsigned)AliHLTTPCGeometry::GetNSlice() || s==slice) && 
	(partition>=(unsigned)AliHLTTPCGeometry::GetNumberOfPatches() || p==partition)) {
      selected->push_back(cl->first);
    } else if (!bAll) {
      // no need to continue, we are out of the range
      break;
    }
  }
  //HLTInfo("collection 0x%08x with %d spacepoints", mask, selected->size());
  fSelections[mask]=selected;
  return selected;
}

float AliHLTTPCHWCFSpacePointContainer::GetX(AliHLTUInt32_t clusterID) const
{
  // get X coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  // FIXME: understand deviation from the nominal x value
  // there is a small deviation in the x coordinate - padrow number correlation
  // in principle, the clusterfinder only uses the mapping to set the x parameter.
  // now extracting the x value from the padrow no.
  //return cl->second.Decoder()->fX;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetPadRow(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetXWidth(AliHLTUInt32_t clusterID) const
{
  // get error for X coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  return 0.0; // fixed in padrow number
}

float AliHLTTPCHWCFSpacePointContainer::GetY(AliHLTUInt32_t clusterID) const
{
  // get Y coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetPad(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetYWidth(AliHLTUInt32_t clusterID) const
{
  // get error for Y coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetSigmaY2(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetZ(AliHLTUInt32_t clusterID) const
{
  // get Z coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetTime(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetZWidth(AliHLTUInt32_t clusterID) const
{
  // get error for Z coordinate
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetSigmaZ2(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetCharge(AliHLTUInt32_t clusterID) const
{
  // get charge
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetCharge(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetQMax(AliHLTUInt32_t clusterID) const
{
  // get charge
  std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.find(clusterID);
  if (cl==fClusters.end() ||
      cl->second.Decoder()==NULL) return 0.0;
  int index=AliHLTTPCGeometry::CluID2Index(cl->first);
  return cl->second.Decoder()->GetQMax(index);
}

float AliHLTTPCHWCFSpacePointContainer::GetPhi(AliHLTUInt32_t clusterID) const
{
  // get charge

  // phi can be derived directly from the id, no need to search
  // for existing cluster
  int slice=AliHLTTPCGeometry::CluID2Slice(clusterID);
  return ( slice + 0.5 ) * TMath::Pi() / 9.0;
}

void AliHLTTPCHWCFSpacePointContainer::Clear(Option_t * option)
{
  // clear the object and reset pointer references
  fClusters.clear();

  for (std::map<AliHLTUInt32_t, vector<AliHLTUInt32_t>*>::iterator selection=fSelections.begin();
       selection!=fSelections.end(); selection++) {
    if (selection->second) delete selection->second;
  }
  fSelections.clear();

  for (std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointBlock>::iterator block=fBlocks.begin();
       block!=fBlocks.end(); block++) {
    if (block->second.GetDecoder()) delete block->second.GetDecoder();
    if (block->second.GetGrid()) delete block->second.GetGrid();
  }
  fBlocks.clear();

  if (fSingleBlock.GetDecoder()) fSingleBlock.GetDecoder()->Reset();
  if (fSingleBlock.GetGrid()) fSingleBlock.GetGrid()->Clear();

  AliHLTSpacePointContainer::Clear(option);
}

void AliHLTTPCHWCFSpacePointContainer::Print(ostream& out, Option_t */*option*/) const
{
  // print to stream
  std::stringstream str;
  str << "AliHLTTPCHWCFSpacePointContainer::Print" << endl;
  str << "n clusters: " << fClusters.size() << endl;
  for (std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.begin();
       cl!=fClusters.end(); cl++) {
    str << " 0x" << hex << setw(8) << setfill('0') << cl->first << dec << cl->second << endl;
  }
  out << str.str();
}

AliHLTSpacePointContainer* AliHLTTPCHWCFSpacePointContainer::SelectByMask(AliHLTUInt32_t mask, bool /*bAlloc*/) const
{
  /// create a collection of clusters for a space point mask
  AUTO_PTR<AliHLTTPCHWCFSpacePointContainer> c(new AliHLTTPCHWCFSpacePointContainer);
  if (!c.get()) return NULL;

  UInt_t slice=AliHLTTPCGeometry::CluID2Slice(mask);
  UInt_t partition=AliHLTTPCGeometry::CluID2Partition(mask);
  for (std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator cl=fClusters.begin();
       cl!=fClusters.end(); cl++) {
    UInt_t s=AliHLTTPCGeometry::CluID2Slice(cl->first);
    UInt_t p=AliHLTTPCGeometry::CluID2Partition(cl->first);
    if ((slice>=(unsigned)AliHLTTPCGeometry::GetNSlice() || s==slice) && 
	(partition>=(unsigned)AliHLTTPCGeometry::GetNumberOfPatches() || p==partition)) {
      c->fClusters[cl->first]=cl->second;
    }
  }
  return c.release();
}

AliHLTSpacePointContainer* AliHLTTPCHWCFSpacePointContainer::SelectByTrack(int trackId, bool /*bAlloc*/) const
{
  /// create a collection of clusters for a specific track
  AUTO_PTR<AliHLTTPCHWCFSpacePointContainer> c(new AliHLTTPCHWCFSpacePointContainer);
  if (!c.get()) return NULL;

  HLT::copy_map_if(fClusters.begin(), fClusters.end(), c->fClusters, HLT::AliHLTUnaryPredicate<AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties, int>(&AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::GetTrackId,trackId));
  return c.release();
}

AliHLTSpacePointContainer* AliHLTTPCHWCFSpacePointContainer::SelectByMC(int mcId, bool /*bAlloc*/) const
{
  /// create a collection of clusters for a specific MC track
  AUTO_PTR<AliHLTTPCHWCFSpacePointContainer> c(new AliHLTTPCHWCFSpacePointContainer);
  if (!c.get()) return NULL;

  HLT::copy_map_if(fClusters.begin(), fClusters.end(), c->fClusters, HLT::AliHLTUnaryPredicate<AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties, int>(&AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::GetMCId,mcId));
  return c.release();
}

AliHLTSpacePointContainer* AliHLTTPCHWCFSpacePointContainer::UsedClusters(bool /*bAlloc*/) const
{
  /// create a collection of all used clusters
  AUTO_PTR<AliHLTTPCHWCFSpacePointContainer> c(new AliHLTTPCHWCFSpacePointContainer);
  if (!c.get()) return NULL;

  HLT::copy_map_if(fClusters.begin(), fClusters.end(), c->fClusters, HLT::AliHLTUnaryPredicate<AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties, bool>(&AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::IsUsed,true));
  return c.release();
}

AliHLTSpacePointContainer* AliHLTTPCHWCFSpacePointContainer::UnusedClusters(bool /*bAlloc*/) const
{
  /// create a collection of all unused clusters
  AUTO_PTR<AliHLTTPCHWCFSpacePointContainer> c(new AliHLTTPCHWCFSpacePointContainer);
  if (!c.get()) return NULL;

  HLT::copy_map_if(fClusters.begin(), fClusters.end(), c->fClusters, HLT::AliHLTUnaryPredicate<AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties, bool>(&AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::IsUsed,false));
  return c.release();
}

int AliHLTTPCHWCFSpacePointContainer::MarkUsed(const AliHLTUInt32_t* clusterIDs, int arraySize)
{
  /// mark the clusters with specified IDs as used
  if (!clusterIDs) return -EINVAL;
  int iCount=0;
  for (int i=0; i<arraySize; i++) {
    if (fClusters.find(clusterIDs[i])==fClusters.end()) continue;
    fClusters[clusterIDs[i]].MarkUsed();
    iCount++;
  }
  return iCount;
}

int AliHLTTPCHWCFSpacePointContainer::SetTrackID(int trackID, const AliHLTUInt32_t* clusterIDs, int arraySize)
{
  /// set track id for specified clusters
  if (!clusterIDs) return -EINVAL;
  int iCount=0;
  for (int i=0; i<arraySize; i++) {
    if (fClusters.find(clusterIDs[i])==fClusters.end()) continue;
    fClusters[clusterIDs[i]].SetTrackId(trackID);
    iCount++;
  }
  return iCount;
}

int AliHLTTPCHWCFSpacePointContainer::GetTrackID(AliHLTUInt32_t clusterID) const
{
  /// get track id for specified cluster
  map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointProperties>::const_iterator element=fClusters.find(clusterID);
  if (element==fClusters.end()) return -1;
  return element->second.GetTrackId();
}

int AliHLTTPCHWCFSpacePointContainer::SetMCID(int mcID, const AliHLTUInt32_t* clusterIDs, int arraySize)
{
  /// set mc id for specified clusters
  if (!clusterIDs) return -EINVAL;
  int iCount=0;
  for (int i=0; i<arraySize; i++) {
    if (fClusters.find(clusterIDs[i])==fClusters.end()) continue;
    fClusters[clusterIDs[i]].SetMCId(mcID);
    iCount++;
  }
  return iCount;
}

int AliHLTTPCHWCFSpacePointContainer::Write(AliHLTUInt8_t* outputPtr,
					    AliHLTUInt32_t size,
					    AliHLTComponentBlockDataList&
					    outputBlocks,
					    AliHLTDataDeflater* pDeflater,
					    const char* option) const
{
  /// write blocks to HLT component output
  AliHLTUInt32_t offset=0;
  if (outputBlocks.size()>0) {
    offset=outputBlocks.back().fOffset+outputBlocks.back().fSize;
  }
  return Write(outputPtr, size, offset, outputBlocks, pDeflater, option);
}

int AliHLTTPCHWCFSpacePointContainer::Write(AliHLTUInt8_t* outputPtr,
					    AliHLTUInt32_t size,
					    AliHLTUInt32_t offset,
					    AliHLTComponentBlockDataList&
					    outputBlocks,
					    AliHLTDataDeflater* pDeflater,
					    const char* option) const
{
  /// write blocks to HLT component output
  return WriteSorted(outputPtr, size, offset, outputBlocks, pDeflater, option);
}

int AliHLTTPCHWCFSpacePointContainer::WriteSorted(AliHLTUInt8_t* outputPtr,
						  AliHLTUInt32_t size,
						  AliHLTUInt32_t offset,
						  AliHLTComponentBlockDataList&
						  outputBlocks,
						  AliHLTDataDeflater* pDeflater,
						  const char* option) const
{
  /// write blocks to HLT component output
  int iResult=0;

  if (fMode&kModeSingle) {
    iResult=WriteSorted(outputPtr, size, offset, fSingleBlock.GetDecoder(), fSingleBlock.GetGrid(), fSingleBlock.GetId(), outputBlocks, pDeflater, option);
  } else {
    iResult=-ENOENT;
    for (std::map<AliHLTUInt32_t, AliHLTTPCHWCFSpacePointBlock>::const_iterator block=fBlocks.begin();
	 block!=fBlocks.end(); block++) {
      AliHLTTPCHWCFData* pDecoder=block->second.GetDecoder();
      AliHLTSpacePointPropertyGrid* pGrid=block->second.GetGrid();
      AliHLTUInt32_t mask=block->first;
      // FIXME: have to propagate the parameter which block is currently to be written
      // for now the index grid is only set for that one
      if (!pGrid) continue;
      iResult=WriteSorted(outputPtr, size, offset, pDecoder, pGrid, mask, outputBlocks, pDeflater, option);
      break; // only one is supposed to be written
    }
    if (iResult==-ENOENT) {
      HLTError("could not find the index grid of the partition to be written");
    }
  }
  return iResult;
}

int AliHLTTPCHWCFSpacePointContainer::WriteSorted(AliHLTUInt8_t* outputPtr,
						  AliHLTUInt32_t size,
						  AliHLTUInt32_t offset,
						  AliHLTTPCHWCFData* pDecoder,
						  AliHLTSpacePointPropertyGrid* pGrid,
						  AliHLTUInt32_t mask,
						  AliHLTComponentBlockDataList&
						  outputBlocks,
						  AliHLTDataDeflater* pDeflater,
						  const char* option) const
{
  /// write blocks to HLT component output
  if (!outputPtr || !pDecoder || !pGrid) return -EINVAL;
  if (pDecoder->GetNumberOfClusters()==0) return 0;
  if (option) {
    // this is only for sending mc labels in simulation and testing
    // no impact to real running
    if (!fWrittenClusterIds && strcmp(option, "write-cluster-ids")==0) {
      const_cast<AliHLTTPCHWCFSpacePointContainer*>(this)->fWrittenClusterIds=new vector<AliHLTUInt32_t>;
    }
  }
  int iResult=0;
  AliHLTUInt32_t capacity=size;
  size=0;

  int slice=AliHLTTPCGeometry::CluID2Slice(mask);
  int part=AliHLTTPCGeometry::CluID2Partition(mask);

  // Note: the offset parameter is only for the block descriptors, output pointer and size
  // consider already the offset
  AliHLTTPCRawClusterData* blockout=reinterpret_cast<AliHLTTPCRawClusterData*>(outputPtr+size);
  blockout->fVersion=0;
  blockout->fCount=0;

  if (pDeflater) {
    pDeflater->Clear();
    pDeflater->InitBitDataOutput(reinterpret_cast<AliHLTUInt8_t*>(blockout->fClusters), capacity-size-sizeof(AliHLTTPCRawClusterData));
    blockout->fVersion=pDeflater->GetDeflaterVersion();
    if (fMode&kModeDifferentialPadTime) blockout->fVersion+=2;
  }

  unsigned lastPadRow=0;
  AliHLTUInt64_t lastPad64=0;
  AliHLTUInt64_t lastTime64=0;
  pDeflater->StartEncoder();
  AliHLTSpacePointPropertyGrid::iterator clusterID=pGrid->begin();
  if (clusterID!=pGrid->end()) {
    for (; clusterID!=pGrid->end(); clusterID++) {
      if (clusterID.Data().fTrackId>-1) {
	// this is an assigned cluster, skip
	// TODO: introduce selectors into AliHLTIndexGrid::begin to loop
	// consistently over entries, e.g. this check has to be done also
	// in the forwarding of MC labels in
	// AliHLTTPCDataCompressionComponent::ForwardMCLabels
	continue;
      }
      if ((unsigned)slice!=AliHLTTPCGeometry::CluID2Slice(clusterID.Data().fId) ||
	  (unsigned)part!=AliHLTTPCGeometry::CluID2Partition(clusterID.Data().fId)) {
	HLTError("cluster index 0x%08x out of slice %d partition %d", clusterID.Data().fId, slice, part);
      }
      int index=AliHLTTPCGeometry::CluID2Index(clusterID.Data().fId);
      const AliHLTTPCHWCFData::iterator& input=pDecoder->find(index);
      if (!(input!=pDecoder->end())) continue;
      if (fWrittenClusterIds) fWrittenClusterIds->push_back(clusterID.Data().fId);
      int padrow=input.GetPadRow();
      if (padrow<0) {
	// something wrong here, padrow is stored in the cluster header
	// word which has bit pattern 0x3 in bits bit 30 and 31 which was
	// not recognized
	ALIHLTERRORGUARD(1, "can not read cluster header word");
	iResult=-EBADF;
	break;
      }

      float pad =input.GetPad();
      float time =input.GetTime();
      float sigmaY2=input.GetSigmaY2();
      float sigmaZ2=input.GetSigmaZ2();
      if (!pDeflater) {
	AliHLTTPCRawCluster& c=blockout->fClusters[blockout->fCount];
	padrow+=AliHLTTPCGeometry::GetFirstRow(part);
	c.SetPadRow(padrow);
	c.SetCharge(input.GetCharge());
	c.SetPad(pad);  
	c.SetTime(time);
	c.SetSigmaPad2(sigmaY2);
	c.SetSigmaTime2(sigmaZ2);
	c.SetQMax(input.GetQMax());
      } else {
	AliHLTUInt64_t padrow64=input.GetPadRow();
	if (padrow64==lastPadRow) {
	  padrow64-=lastPadRow;
	} else if (padrow64>lastPadRow) {
	  padrow64-=lastPadRow;
	  lastPadRow+=padrow64;
	} else {
	  AliFatal("padrows not ordered");
	}

	AliHLTUInt32_t padType=0;
	AliHLTUInt32_t signdPad=0;
	AliHLTUInt64_t pad64=0;
	if ((fMode&kModeDifferentialPadTime)!=0 && sigmaY2<.00001) {
	  // single pad cluster
	  // use twice the pad position to take account for the 0.5 offset
	  // added in the AliHLTTPCHWCFData decoder in accordance with the
	  // offline definition. Using the factor 2, this offset is not
	  // cut off by rounding
	  pad64=(AliHLTUInt64_t)round(2*pad);
	  padType=1;
	} else {
	if (!isnan(pad)) pad64=(AliHLTUInt64_t)round(pad*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kPad].fScale);
	}
	if (fMode&kModeDifferentialPadTime && padType==0) {
	  AliHLTUInt64_t dpad64=0;
	  if (pad64<lastPad64) {
	    dpad64=lastPad64-pad64;
	    signdPad=1;
	  } else {
	    dpad64=pad64-lastPad64;
	    signdPad=0;
	  }
	  lastPad64=pad64;
	  pad64=dpad64;
	}
	AliHLTUInt32_t signdTime=0;
	AliHLTUInt64_t time64=0;
	if (!isnan(time)) time64=(AliHLTUInt64_t)round(time*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kTime].fScale);
	if (fMode&kModeDifferentialPadTime) {
	  AliHLTUInt64_t dtime64=0;
	  if (time64<lastTime64) {
	    dtime64=lastTime64-time64;
	    signdTime=1;
	  } else {
	    dtime64=time64-lastTime64;
	    signdTime=0;
	  }
	  lastTime64=time64;
	  time64=dtime64;
	}
	AliHLTUInt64_t sigmaY264=0;
	if (!isnan(sigmaY2)) sigmaY264=(AliHLTUInt64_t)round(sigmaY2*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fScale);
	AliHLTUInt64_t sigmaZ264=0;
	if (!isnan(sigmaZ2)) sigmaZ264=(AliHLTUInt64_t)round(sigmaZ2*AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fScale);
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kPadRow , padrow64);
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kPad    , pad64);
	if (fMode&kModeDifferentialPadTime) pDeflater->OutputBit(padType);
	if (fMode&kModeDifferentialPadTime && padType==0) pDeflater->OutputBit(signdPad);
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kTime   , time64);
	if (fMode&kModeDifferentialPadTime) pDeflater->OutputBit(signdTime);
	if (padType==0) pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kSigmaY2, sigmaY264);
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kSigmaZ2, sigmaZ264);
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kCharge , input.GetCharge());
	pDeflater->OutputParameterBits(AliHLTTPCDefinitions::kQMax   , input.GetQMax());
      }
      blockout->fCount++;
    }
  }
  pDeflater->StopEncoder();
  AliHLTComponent_BlockData bd;
  AliHLTComponent::FillBlockData(bd);
  bd.fOffset        = size+offset;
  if (!pDeflater) {
    bd.fSize        = sizeof(AliHLTTPCRawClusterData)+blockout->fCount*sizeof(AliHLTTPCRawCluster);
    bd.fDataType    = AliHLTTPCDefinitions::fgkRawClustersDataType;
  } else {
    pDeflater->Pad8Bits();
    bd.fSize        = sizeof(AliHLTTPCRawClusterData)+pDeflater->GetBitDataOutputSizeBytes();
    pDeflater->CloseBitDataOutput();
    bd.fDataType    = AliHLTTPCDefinitions::RemainingClustersCompressedDataType();
  }
  bd.fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification(slice, slice, part, part);
  outputBlocks.push_back(bd);

  size += bd.fSize;

  if (fWrittenClusterIds && fWrittenClusterIds->size()>0) {
    AliHLTComponent::FillBlockData(bd);
    bd.fOffset        = size+offset;
    bd.fSize        = fWrittenClusterIds->size()*sizeof(vector<AliHLTUInt32_t>::value_type);
    if (bd.fSize+size<=capacity) {
      memcpy(outputPtr+size, &(*fWrittenClusterIds)[0], bd.fSize);
      bd.fDataType    = AliHLTTPCDefinitions::RemainingClusterIdsDataType();
      bd.fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification(slice, slice, part, part);
      outputBlocks.push_back(bd);    
      size += bd.fSize;
    } else {
      iResult=-ENOSPC;
    }
    fWrittenClusterIds->clear();
  }

  if (iResult<0) return iResult;
  return size;
}

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::AliHLTTPCHWCFSpacePointProperties()
  : fDecoder(NULL)
  , fIndex(-1)
  , fUsed(false)
  , fTrackId(-1)
  , fMCId(-1)
{
  // constructor
}

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::AliHLTTPCHWCFSpacePointProperties(const AliHLTTPCHWCFData* pDecoder, int index)
  : fDecoder(pDecoder)
  , fIndex(index)
  , fUsed(false)
  , fTrackId(-1)
  , fMCId(-1)
{
  // constructor
}

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::AliHLTTPCHWCFSpacePointProperties(const AliHLTTPCHWCFSpacePointProperties& src)
  : fDecoder(src.fDecoder)
  , fIndex(src.fIndex)
  , fUsed(src.fUsed)
  , fTrackId(src.fTrackId)
  , fMCId(src.fMCId)
{
  // copy constructor
}

AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties& AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::operator=(const AliHLTTPCHWCFSpacePointProperties& src)
{
  // assignment operator
  if (&src==this) return *this;
  fDecoder=src.fDecoder;
  fIndex=src.fIndex;
  fUsed=src.fUsed;
  fTrackId=src.fTrackId;
  fMCId=src.fMCId;

  return *this;
}

void AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties::Print(ostream& out, Option_t */*option*/) const
{
  // print info
  if (!Decoder()) {
    out << "no data";
    return;
  }
  std::stringstream str;
  const AliHLTTPCHWCFData* decoder=Decoder();
  str.setf(ios::fixed,ios::floatfield);
  str << " " << setfill(' ') << setw(3) << decoder->GetPadRow(fIndex) 
      << " " << setw(8) << setprecision(3) << decoder->GetPad(fIndex)
      << " " << setw(8) << setprecision(3) << decoder->GetTime(fIndex)
      << " " << setw(8) << setprecision(1) << decoder->GetSigmaY2(fIndex) 
      << " " << setw(9) << setprecision(1) << decoder->GetSigmaZ2(fIndex)
      << " " << setw(5) << decoder->GetCharge(fIndex) 
      << " " << setw(5) << decoder->GetQMax(fIndex)
      << " " << fTrackId << " " << fMCId << " " << fUsed;
  out << str.str();
}

ostream& operator<<(ostream &out, const AliHLTTPCHWCFSpacePointContainer::AliHLTTPCHWCFSpacePointProperties& p)
{
  p.Print(out);
  return out;
}
