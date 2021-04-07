// $Id$
//**************************************************************************
//* This file is property of and copyright by the                          *
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

/// @file   AliHLTTPCDataCompressionComponent.cxx
/// @author Matthias Richter
/// @date   2011-08-08
/// @brief  TPC component for data compression
///

#include "AliHLTTPCDataCompressionComponent.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCDataCompressionDescriptor.h"
#include "AliHLTTPCRawClustersDescriptor.h"
#include "AliHLTTPCTrackGeometry.h"
#include "AliHLTTPCSpacePointContainer.h"
#include "AliHLTTPCRawSpacePointContainer.h"
#include "AliHLTGlobalBarrelTrack.h"
#include "AliHLTComponentBenchmark.h"
#include "AliHLTDataDeflaterSimple.h"
#include "AliHLTDataDeflaterHuffman.h"
#include "AliHLTTPCGeometry.h"
#include "AliHLTTPCClusterMCData.h"
#include "AliHLTTPCClusterTransformation.h"
#include "AliHLTErrorGuard.h"
#include "AliCDBManager.h"
#include "AliCDBPath.h"
#include "AliCDBId.h"
#include "AliCDBMetaData.h"
#include "AliCDBEntry.h"
#ifdef HAVE_ALIGPU
#include "AliHLTTPCClusterStatComponent.h"
#include "AliHLTTPCReverseTransformInfoV1.h"
#endif
#include "TH1F.h"
#include "TFile.h"
#include "TObjString.h"
#include <memory>

#if __cplusplus > 201402L
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

ClassImp(AliHLTTPCDataCompressionComponent)

AliHLTTPCDataCompressionComponent::AliHLTTPCDataCompressionComponent()
  : AliHLTProcessor()
  , fMode(kCompressionModeNone)
  , fDeflaterMode(kDeflaterModeNone)
  , fVerificationMode(0)
  , fProvideClusterIds(0)
  , fCreateFlags(0)
  , fMaxDeltaPad(AliHLTTPCDefinitions::GetMaxClusterDeltaPad())
  , fMaxDeltaTime(AliHLTTPCDefinitions::GetMaxClusterDeltaTime())
  , fRawInputClusters(NULL)
  , fInputClusters(NULL)
  , fTrackGrid(NULL)
  , fSpacePointGrid(NULL)
  , fpDataDeflater(NULL)
  , fHistoCompFactor(NULL)
  , fHistoResidualPad(NULL)
  , fHistoResidualTime(NULL)
  , fHistoClustersOnTracks(NULL)
  , fHistoClusterRatio(NULL)
  , fHistoTrackClusterRatio(NULL)
  , fHistogramFile()
  , fHuffmanTableFile()
  , fpBenchmark(NULL)
  , fpWrittenAssociatedClusterIds(NULL)
  , fDriftTimeFactorA(1.)
  , fDriftTimeOffsetA(0.)
  , fDriftTimeFactorC(1.)
  , fDriftTimeOffsetC(0.)
  , fVerbosity(0)
{
}

AliHLTTPCDataCompressionComponent::~AliHLTTPCDataCompressionComponent()
{
  /// destructor
  if (fpWrittenAssociatedClusterIds) delete fpWrittenAssociatedClusterIds;
}


const char* AliHLTTPCDataCompressionComponent::GetComponentID()
{
  /// inherited from AliHLTComponent: id of the component
  return "TPCDataCompressor";
}


void AliHLTTPCDataCompressionComponent::GetInputDataTypes( AliHLTComponentDataTypeList& tgtList)
{
  /// inherited from AliHLTComponent: list of data types in the vector reference
  tgtList.clear();
  tgtList.push_back(AliHLTTPCDefinitions::RawClustersDataType());
  tgtList.push_back(AliHLTTPCDefinitions::RawClustersDescriptorDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClustersDataType());
  tgtList.push_back(kAliHLTDataTypeTrack|kAliHLTDataOriginTPC);
}

AliHLTComponentDataType AliHLTTPCDataCompressionComponent::GetOutputDataType()
{
  /// inherited from AliHLTComponent: output data type of the component.
  return kAliHLTMultipleDataType;
}

int AliHLTTPCDataCompressionComponent::GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList)
{
  /// inherited from AliHLTComponent: multiple output data types of the component.
  tgtList.clear();
  tgtList.push_back(AliHLTTPCDefinitions::DataCompressionDescriptorDataType());
  tgtList.push_back(AliHLTTPCDefinitions::RawClustersDataTypeNotCompressed());
  tgtList.push_back(AliHLTTPCDefinitions::RemainingClustersCompressedDataType());
  tgtList.push_back(AliHLTTPCDefinitions::RemainingClusterIdsDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClusterTracksCompressedDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClusterIdTracksDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClustersFlagsDataType());
  return tgtList.size();
}

void AliHLTTPCDataCompressionComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
{
  /// inherited from AliHLTComponent: output data size estimator
  constBase=0;
  inputMultiplier=1.;  // there should not be more data than input
  inputMultiplier+=.3; // slightly more data when using the old HWCF data with 20 Byte and raw clusters 22 Byte
  if (fpWrittenAssociatedClusterIds) inputMultiplier+=.3; // space for optional cluster id array
}

AliHLTComponent* AliHLTTPCDataCompressionComponent::Spawn()
{
  /// inherited from AliHLTComponent: spawn function.
  return new AliHLTTPCDataCompressionComponent;
}

void AliHLTTPCDataCompressionComponent::GetOCDBObjectDescription(TMap* const targetMap)
{
  /// Get a list of OCDB object needed for the particular component
  if (!targetMap) return;

  targetMap->Add(new TObjString("HLT/ConfigTPC/TPCDataCompressor"),
		 new TObjString("component arguments"));
  if (fDeflaterMode==kDeflaterModeHuffman) {
    targetMap->Add(new TObjString("HLT/ConfigTPC/TPCDataCompressorHuffmanTables"),
		   new TObjString("huffman tables for deflater mode 'huffman'"));
  }
}

int AliHLTTPCDataCompressionComponent::DoEvent( const AliHLTComponentEventData& /*evtData*/,
						const AliHLTComponentBlockData* /*inputBlocks*/,
						AliHLTComponentTriggerData& /*trigData*/,
						AliHLTUInt8_t* outputPtr,
						AliHLTUInt32_t& size,
						AliHLTComponentBlockDataList& outputBlocks )
{
  /// inherited from AliHLTProcessor: data processing
  int iResult=0;
  AliHLTUInt32_t capacity=size;
  size=0;

  if (!IsDataEvent()) return 0;

  if (!fRawInputClusters) {
    return -ENODEV;
  }

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->StartNewEvent();
    GetBenchmarkInstance()->Start(0);
  }

  // Process an event

  // Loop over all input blocks in the event
  bool bHaveMC=(GetFirstInputBlock(AliHLTTPCDefinitions::fgkAliHLTDataTypeClusterMCInfo | kAliHLTDataOriginTPC))!=NULL;
  if ((bHaveMC || fVerificationMode>0 || fProvideClusterIds ) && fpWrittenAssociatedClusterIds==NULL) {
    fpWrittenAssociatedClusterIds=new vector<AliHLTUInt32_t>;
  }

  const AliHLTComponentBlockData* pDesc=NULL;

  AliHLTUInt8_t minSlice=0xFF, maxSlice=0xFF, minPatch=0xFF, maxPatch=0xFF;
  AliHLTUInt32_t inputRawClusterSize=0;
  AliHLTUInt32_t outputDataSize=0;
  int allClusters=0;
  int associatedClusters=0;
  float bz=GetBz();

  /// input track array
  vector<AliHLTGlobalBarrelTrack> inputTrackArray;

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->Start(2);
  }

  bool isInputPresent = kFALSE;

  // transformed clusters
  // the transformed clusters have not been used yet
  if (false) { // FIXME: condition to be adjusted
    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::fgkClustersDataType);
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      if (GetBenchmarkInstance()) {
	GetBenchmarkInstance()->AddInput(pDesc->fSize);
      }
      isInputPresent = kTRUE;
      AliHLTUInt8_t slice = 0;
      AliHLTUInt8_t patch = 0;
      slice = AliHLTTPCDefinitions::GetMinSliceNr( pDesc->fSpecification );
      patch = AliHLTTPCDefinitions::GetMinPatchNr( pDesc->fSpecification );
      if ( minSlice==0xFF || slice<minSlice )	minSlice = slice;
      if ( maxSlice==0xFF || slice>maxSlice )	maxSlice = slice;
      if ( minPatch==0xFF || patch<minPatch )	minPatch = patch;
      if ( maxPatch==0xFF || patch>maxPatch )	maxPatch = patch;
      if (fInputClusters) {
	fInputClusters->AddInputBlock(pDesc);
      }
    }
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Stop(2);
      GetBenchmarkInstance()->Start(3);
    }
  }

  vector<int> trackindexmap; // stores index for every track id

  // track data input
  if (fMode==kCompressionModeV1TrackModel || fMode==kCompressionModeV2TrackModel) {
    for (pDesc=GetFirstInputBlock(kAliHLTDataTypeTrack|kAliHLTDataOriginTPC);
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      if (GetBenchmarkInstance()) {
	GetBenchmarkInstance()->AddInput(pDesc->fSize);
      }
      AliHLTUInt8_t slice = 0;
      AliHLTUInt8_t patch = 0;
      slice = AliHLTTPCDefinitions::GetMinSliceNr( pDesc->fSpecification );
      patch = AliHLTTPCDefinitions::GetMinPatchNr( pDesc->fSpecification );
      if ( minSlice==0xFF || slice<minSlice )	minSlice = slice;
      if ( maxSlice==0xFF || slice>maxSlice )	maxSlice = slice;
      if ( minPatch==0xFF || patch<minPatch )	minPatch = patch;
      if ( maxPatch==0xFF || patch>maxPatch )	maxPatch = patch;
      const AliHLTTracksData* pTracks=reinterpret_cast<const AliHLTTracksData*>(pDesc->fPtr);
      if ((iResult=AliHLTGlobalBarrelTrack::ConvertTrackDataArray(pTracks, pDesc->fSize, inputTrackArray))<0) {
	return iResult;
      }
      trackindexmap.resize(inputTrackArray.size(), -1);
    }
  }

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->Stop(3);
    GetBenchmarkInstance()->Start(4);
  }

  // processing
  int trackindex=0;
  for (vector<AliHLTGlobalBarrelTrack>::iterator track=inputTrackArray.begin();
       track!=inputTrackArray.end();
       track++, trackindex++) {
    int trackID=track->GetID();
    if (trackID<0) {
      // FIXME: error guard
      HLTError("invalid track ID");
      continue;
    }
    if (trackID>=(int)trackindexmap.size())
      trackindexmap.resize(trackID+1, -1);
    trackindexmap[trackID]=trackindex;

    if (fVerbosity>0) {
      UInt_t nofPoints=track->GetNumberOfPoints();
      const UInt_t* points=track->GetPoints();
      for (unsigned i=0; i<nofPoints; i++) {
	int slice=AliHLTTPCGeometry::CluID2Slice(points[i]);
	int partition=AliHLTTPCGeometry::CluID2Partition(points[i]);
	int number=AliHLTTPCGeometry::CluID2Index(points[i]);
	HLTInfo("track %d point %d id 0x%08x slice %d partition %d number %d", track->GetID(), i, points[i], slice, partition, number);
      }
    }

    AliHLTTPCTrackGeometry* trackpoints=new AliHLTTPCTrackGeometry;
    if (!trackpoints) continue;
    HLTDebug("track %d id %d:", trackindex, trackID);

    // in order to avoid rounding errors the track points are
    // calculated in exactly the same way as in the decoding
    // Thats why the track instance can not be used directly
    // but a new instance is created from the values in the
    // storage format.
    // think about moving that to some common code used by
    // both compression and decoding
    AliHLTExternalTrackParam param;
    memset(&param, 0, sizeof(param));
    float alpha=track->GetAlpha();
    while (alpha<0.) alpha+=TMath::TwoPi();
    while (alpha>TMath::TwoPi()) alpha-=TMath::TwoPi();
    AliHLTUInt8_t tSlice=AliHLTUInt8_t(9*alpha/TMath::Pi());
    param.fAlpha   =( tSlice + 0.5 ) * TMath::Pi() / 9.0;
    if (param.fAlpha>TMath::TwoPi()) param.fAlpha-=TMath::TwoPi();
    param.fX       = track->GetX();
    param.fY       = track->GetY();
    param.fZ       = track->GetZ();
    param.fSinPhi  = track->GetSnp();
    param.fTgl     = track->GetTgl();
    param.fq1Pt    = track->GetSigned1Pt();
    AliHLTGlobalBarrelTrack ctrack(param);
    ctrack.CalculateHelixParams(bz);
    trackpoints->InitDriftTimeTransformation(fDriftTimeFactorA, fDriftTimeOffsetA, fDriftTimeFactorC, fDriftTimeOffsetC);
    trackpoints->SetTrackId(trackID);
    trackpoints->CalculateTrackPoints(ctrack);
    trackpoints->RegisterTrackPoints(fTrackGrid);
    track->SetTrackGeometry(trackpoints);
  }

  for (vector<AliHLTGlobalBarrelTrack>::const_iterator track=inputTrackArray.begin();
       track!=inputTrackArray.end();
       track++) {
    AliHLTTrackGeometry* trackpoints=track->GetTrackGeometry();
    if (!trackpoints) continue;
    trackpoints->FillTrackPoints(fTrackGrid);
  }
  if (fVerbosity>0) {
    fTrackGrid->Print();
  }

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->Stop(4);
    GetBenchmarkInstance()->Start(5);
  }

  // loop over raw cluster blocks, assign to tracks and write
  // unassigned clusters
  for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::fgkRawClustersDataType);
       pDesc!=NULL; pDesc=GetNextInputBlock()) {
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Start(1);
      GetBenchmarkInstance()->AddInput(pDesc->fSize);
    }
    isInputPresent = kTRUE;
    AliHLTUInt8_t slice = 0;
    AliHLTUInt8_t patch = 0;
    slice = AliHLTTPCDefinitions::GetMinSliceNr( pDesc->fSpecification );
    patch = AliHLTTPCDefinitions::GetMinPatchNr( pDesc->fSpecification );
    if ( minSlice==0xFF || slice<minSlice )	minSlice = slice;
    if ( maxSlice==0xFF || slice>maxSlice )	maxSlice = slice;
    if ( minPatch==0xFF || patch<minPatch )	minPatch = patch;
    if ( maxPatch==0xFF || patch>maxPatch )	maxPatch = patch;
    inputRawClusterSize+=pDesc->fSize;

    // add the data and populate the index grid
    fRawInputClusters->AddInputBlock(pDesc);
    fRawInputClusters->PopulateAccessGrid(fSpacePointGrid, pDesc->fSpecification);
    if (fVerbosity>0 && fSpacePointGrid->GetNumberOfSpacePoints()>0) {
      HLTInfo("index grid slice %d partition %d", slice, patch);
      fSpacePointGrid->Print();
      for (AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid::iterator& cl=fSpacePointGrid->begin();
	   cl!=fSpacePointGrid->end(); cl++) {
	AliHLTUInt32_t id=cl.Data().fId;
	float row=fRawInputClusters->GetX(id);
	float pad=fRawInputClusters->GetY(id);
	float time=fRawInputClusters->GetZ(id);
	HLTInfo("    cluster id 0x%08x: row %f  pad %f  time %f", id, row, pad, time);
      }
    }
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Stop(1);
      GetBenchmarkInstance()->Start(4);
    }

    // process the clusters per padrow and check the track grid
    // for tracks crossing that particular padrow
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Stop(4);
      GetBenchmarkInstance()->Start(5);
    }
    allClusters+=fSpacePointGrid->GetNumberOfSpacePoints();
    iResult=ProcessTrackClusters(&inputTrackArray[0], inputTrackArray.size(), fTrackGrid, trackindexmap, fSpacePointGrid, fRawInputClusters, slice, patch);
    if( iResult< 0 ) break;
    int assignedInThisPartition = iResult;
    associatedClusters+=iResult;
    
    iResult=ProcessRemainingClusters(&inputTrackArray[0], inputTrackArray.size(), fTrackGrid, trackindexmap, fSpacePointGrid, fRawInputClusters, slice, patch);
    if( iResult< 0 ) break;
    associatedClusters+=iResult;

    if (fSpacePointGrid->GetNumberOfSpacePoints()>0) {
      if (fVerbosity>0) HLTInfo("associated %d (%d) of %d clusters in slice %d partition %d", iResult+assignedInThisPartition, assignedInThisPartition, fSpacePointGrid->GetNumberOfSpacePoints(), slice, patch);
    }

    // write all remaining clusters not yet assigned to tracks
    // the index grid is used to write sorted in padrow
    // FIXME: decoder index instead of data specification to be used
    // use an external access grid to reduce allocated memory
    // set to NULL after writing the clusters
    const char* writeoptions="";
    if (fpWrittenAssociatedClusterIds) {
      writeoptions="write-cluster-ids";
    }
    fRawInputClusters->SetSpacePointPropertyGrid(pDesc->fSpecification, fSpacePointGrid);
    iResult=fRawInputClusters->Write(outputPtr+size, capacity-size, outputBlocks, fpDataDeflater, writeoptions);
    fRawInputClusters->SetSpacePointPropertyGrid(pDesc->fSpecification, NULL);
    if( iResult<0 ) break;
    
    size+=iResult;
    outputDataSize+=iResult;
    // the size of the optional cluster id array must be subtracted
    if (fpWrittenAssociatedClusterIds && outputBlocks.size()>0 &&
	outputBlocks.back().fDataType==AliHLTTPCDefinitions::RemainingClusterIdsDataType()) {
      outputDataSize-=outputBlocks.back().fSize;
    }
    if (GetBenchmarkInstance()) GetBenchmarkInstance()->AddOutput(iResult);
    
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Stop(5);
    }

    fSpacePointGrid->Clear();
  }
  if (fHistoClusterRatio && allClusters>0) {
    if (fVerbosity>0) HLTInfo("associated %d of %d clusters to tracks", associatedClusters, allClusters);
    float ratio=associatedClusters; ratio/=allClusters;
    fHistoClusterRatio->Fill(ratio);
  }

  // output of track model clusters
  if (iResult>=0) do {
    AliHLTUInt32_t tracksBufferOffset=sizeof(AliHLTTPCTrackModelBlock);
    if (capacity-size<tracksBufferOffset) {
      iResult=-ENOSPC;
      break;
    }
    if (fpWrittenAssociatedClusterIds) fpWrittenAssociatedClusterIds->clear();
    AliHLTTPCTrackModelBlock* trackModelBlock=reinterpret_cast<AliHLTTPCTrackModelBlock*>(outputPtr+size);
    trackModelBlock->fVersion=1;
    trackModelBlock->fDeflaterMode=fpDataDeflater?fpDataDeflater->GetDeflaterVersion():0;
    trackModelBlock->fTrackCount=inputTrackArray.size();
    trackModelBlock->fClusterCount=0;
    trackModelBlock->fGlobalParameterCnt=5;
    tracksBufferOffset+=trackModelBlock->fGlobalParameterCnt*sizeof(trackModelBlock->fGlobalParameters);
    if (capacity-size<tracksBufferOffset) {
      iResult=-ENOSPC;
      break;
    }

    AliHLTUInt32_t parameterIndex=0;
    trackModelBlock->fGlobalParameters[parameterIndex++]=bz;
    trackModelBlock->fGlobalParameters[parameterIndex++]=fDriftTimeFactorA;
    trackModelBlock->fGlobalParameters[parameterIndex++]=fDriftTimeOffsetA;
    trackModelBlock->fGlobalParameters[parameterIndex++]=fDriftTimeFactorC;
    trackModelBlock->fGlobalParameters[parameterIndex++]=fDriftTimeOffsetC;
    if (parameterIndex!=trackModelBlock->fGlobalParameterCnt) {
      HLTError("internal error, size of parameter array has changed without providing all values");
      iResult=-EFAULT;
      break;
    }

    if (trackindexmap.size()>0) {// condition for track model compression
      iResult=WriteTrackClusters(inputTrackArray, fRawInputClusters, fpDataDeflater, outputPtr+size+tracksBufferOffset, capacity-size-tracksBufferOffset);
      if (iResult<0) break;
      AliHLTComponent_BlockData bd;
      FillBlockData(bd);
      bd.fOffset        = size;
      bd.fSize          = tracksBufferOffset+iResult;
      bd.fDataType      = AliHLTTPCDefinitions::ClusterTracksCompressedDataType();
      bd.fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification(minSlice, maxSlice, minPatch, maxPatch);
      outputBlocks.push_back(bd);
      size += bd.fSize;
      outputDataSize+=bd.fSize;
      HLTBenchmark("track data block of %d tracks: size %d", inputTrackArray.size(), bd.fSize);

      if (fpWrittenAssociatedClusterIds && fpWrittenAssociatedClusterIds->size()>0) {
	AliHLTComponent::FillBlockData(bd);
	bd.fOffset        = size;
	bd.fSize        = fpWrittenAssociatedClusterIds->size()*sizeof(vector<AliHLTUInt32_t>::value_type);
	if (capacity-size>bd.fSize) {
	  memcpy(outputPtr+bd.fOffset, &(*fpWrittenAssociatedClusterIds)[0], bd.fSize);
	  bd.fDataType    = AliHLTTPCDefinitions::ClusterIdTracksDataType();
	  bd.fSpecification = AliHLTTPCDefinitions::EncodeDataSpecification(minSlice, maxSlice, minPatch, maxPatch);
	  outputBlocks.push_back(bd);
	  size += bd.fSize;
	} else {
	  iResult=-ENOSPC;
	}
	
	fpWrittenAssociatedClusterIds->clear();
      }
    }

  } while (0);

  fRawInputClusters->Clear();

  // Write header block
  
  if( iResult>=0 && isInputPresent ){
    pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RawClustersDescriptorDataType() );
    if( pDesc ){
      const AliHLTTPCRawClustersDescriptor &clDesc = *reinterpret_cast<const AliHLTTPCRawClustersDescriptor*>(pDesc->fPtr);
      if( !clDesc.CheckSize( pDesc->fSize ) ){
	HLTError("Corrupted cluster descriptor");
      }
     
      AliHLTComponent_BlockData bd;
      FillBlockData(bd);
      bd.fOffset        = size;
      bd.fSize          = sizeof(AliHLTTPCDataCompressionDescriptor);
      bd.fDataType      = AliHLTTPCDefinitions::DataCompressionDescriptorDataType();
      if( capacity < size + bd.fSize ){
	iResult = -ENOSPC;
      } else {
	AliHLTTPCDataCompressionDescriptor compDesc;
	compDesc.SetMergedClustersFlag( clDesc.GetMergedClustersFlag() );
	*(AliHLTTPCDataCompressionDescriptor*)(outputPtr + bd.fOffset ) = compDesc;
	outputBlocks.push_back(bd);
	size += bd.fSize;
	outputDataSize+=bd.fSize;
	//HLTBenchmark("header data block of size %d", bd.fSize);
      }
    }
  }

  float compressionFactor=(float)inputRawClusterSize;
  if ((outputDataSize)>0) compressionFactor/=outputDataSize;
  else compressionFactor=0.;
  if (fHistoCompFactor && compressionFactor > 0.) fHistoCompFactor->Fill(compressionFactor);

  if (GetBenchmarkInstance() && allClusters>0) {
    GetBenchmarkInstance()->Stop(0);
    if (fDeflaterMode!=kDeflaterModeHuffmanTrainer) {
      HLTBenchmark("%s - compression factor %.2f", GetBenchmarkInstance()->GetStatistics(), compressionFactor);
    } else {
      HLTBenchmark("%s", GetBenchmarkInstance()->GetStatistics());
    }
  }

  if (fInputClusters) {
    fInputClusters->Clear();
  }
  if (fRawInputClusters) {
    fRawInputClusters->Clear();
  }
  if (fTrackGrid) {
    fTrackGrid->Clear();
  }

  if( iResult>=0 ){ // forward MC labels
    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::fgkAliHLTDataTypeClusterMCInfo | kAliHLTDataOriginTPC);
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      outputBlocks.push_back(*pDesc);
    }
  }

  return iResult;
}

int AliHLTTPCDataCompressionComponent::ProcessTrackClusters(AliHLTGlobalBarrelTrack* pTracks, unsigned nofTracks,
							    AliHLTTrackGeometry::AliHLTTrackGrid* pTrackIndex,
							    const vector<int>& trackIndexMap,
							    AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pClusterIndex,
							    AliHLTSpacePointContainer* pClusters,
							    int slice, int partition) const
{
  // process to assigned track clusters
  int iResult=0;
  int assignedClusters=0;
  if (!pTracks || nofTracks==0) return 0;

  vector<int> processedTracks(nofTracks, -1);
  for (AliHLTTrackGeometry::AliHLTTrackGrid::iterator& trackId=pTrackIndex->begin(slice, partition, -1);
       trackId!=pTrackIndex->end(); trackId++) {
    if (trackId.Data()>=trackIndexMap.size()) {
      HLTError("can not find track id %d in index map of size %d", trackId.Data(), trackIndexMap.size());
      continue;
    }
    int trackindex=trackIndexMap[trackId.Data()];
    if (trackindex<0 || trackindex>=(int)nofTracks) {
      HLTError("invalid index %d found for track id %d", trackindex, trackId.Data());
      continue;
    }
    if (processedTracks[trackindex]>0) continue;
    processedTracks[trackindex]=1;
    AliHLTGlobalBarrelTrack& track=pTracks[trackindex];
    if (!track.GetTrackGeometry()) {
      HLTError("can not find track geometry for track %d", trackId.Data());
      continue;
    }
    AliHLTTPCTrackGeometry* pTrackPoints=dynamic_cast<AliHLTTPCTrackGeometry*>(track.GetTrackGeometry());
    if (!pTrackPoints) {
      HLTError("invalid track geometry type for track %d, expecting AliHLTTPCTrackGeometry", trackId.Data());
      continue;
    }

    UInt_t nofTrackPoints=track.GetNumberOfPoints();
    const UInt_t* trackPoints=track.GetPoints();
    for (unsigned i=0; i<nofTrackPoints; i++) {
      const AliHLTUInt32_t& clusterId=trackPoints[i];
      if (AliHLTTPCGeometry::CluID2Slice(clusterId)!=(unsigned)slice ||
	  AliHLTTPCGeometry::CluID2Partition(clusterId)!=(unsigned)partition) {
	// not in the current partition;
	continue;
      }

      int clusterrow=(int)pClusters->GetX(clusterId);
      AliHLTUInt32_t pointId=AliHLTTPCGeometry::CreateClusterID(slice, partition, clusterrow);
      AliHLTTrackGeometry::AliHLTTrackPoint* point=pTrackPoints->GetRawTrackPoint(pointId);
      if (!point) {
	//HLTError("can not find track point slice %d partition %d padrow %d (0x%08x) of track %d", slice, partition, clusterrow, pointId, trackId.Data());
	continue;
      }
      float pad=point->GetU();
      float time=point->GetV();

      iResult=FindCellClusters(trackId.Data(), clusterrow, pad, time, pClusterIndex, pClusters, point, clusterId);
      if (iResult>0) assignedClusters+=iResult;
    }
  }
  return assignedClusters;
}

int AliHLTTPCDataCompressionComponent::ProcessRemainingClusters(AliHLTGlobalBarrelTrack* pTracks, unsigned nofTracks,
								AliHLTTrackGeometry::AliHLTTrackGrid* pTrackIndex,
								const vector<int>& trackIndexMap,
								AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pClusterIndex,
								AliHLTSpacePointContainer* pClusters,
								int slice, int partition) const
{
  // assign remaining clusters to tracks
  int iResult=0;
  int associatedClusters=0;
  if (!pTracks || nofTracks==0) return 0;

  for (int padrow=0; padrow<AliHLTTPCGeometry::GetNRows(partition); padrow++) {
    for (AliHLTTrackGeometry::AliHLTTrackGrid::iterator& trackId=pTrackIndex->begin(slice, partition, padrow);
	 trackId!=pTrackIndex->end(); trackId++) {
      if (trackId.Data()>=trackIndexMap.size()) {
	HLTError("can not find track id %d in index map of size %d", trackId.Data(), trackIndexMap.size());
	continue;
      }
      int trackindex=trackIndexMap[trackId.Data()];
      if (trackindex<0 || trackindex>=(int)nofTracks) {
	HLTError("invalid index %d found for track id %d", trackindex, trackId.Data());
	continue;
      }
      AliHLTGlobalBarrelTrack& track=pTracks[trackindex];
      if (!track.GetTrackGeometry()) {
	HLTError("can not find track geometry for track %d", trackId.Data());
	continue;
      }
      AliHLTTPCTrackGeometry* pTrackPoints=dynamic_cast<AliHLTTPCTrackGeometry*>(track.GetTrackGeometry());
      if (!pTrackPoints) {
	HLTError("invalid track geometry type for track %d, expecting AliHLTTPCTrackGeometry", trackId.Data());
	continue;
      }
      AliHLTUInt32_t pointId=AliHLTTPCGeometry::CreateClusterID(slice, partition, padrow);
      AliHLTTrackGeometry::AliHLTTrackPoint* point=pTrackPoints->GetRawTrackPoint(pointId);
      if (!point) {
	//HLTError("can not find track point slice %d partition %d padrow %d (0x%08x) of track %d", slice, partition, padrow, pointId, trackId.Data());
	continue;
      }
      float pad=point->GetU();
      float time=point->GetV();

      iResult=FindCellClusters(trackId.Data(), padrow, pad, time, pClusterIndex, pClusters, point);
      if (iResult>0) associatedClusters+=iResult;
      if (fVerbosity>0) {
	HLTInfo("trackpoint track %d slice %d partition %d padrow %d: %.3f \t%.3f - associated %d", track.GetID(), slice, partition, padrow, pad, time, iResult);
      }
    }
  }
  if (iResult<0) return iResult;
  return associatedClusters;
}

int AliHLTTPCDataCompressionComponent::FindCellClusters(int trackId, int padrow, float pad, float time,
							AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pClusterIndex,
							AliHLTSpacePointContainer* pClusters,
							AliHLTTrackGeometry::AliHLTTrackPoint* pTrackPoint,
							AliHLTUInt32_t clusterId) const
{
  // check index cell for entries and assign to track
  int count=0;
  // search a 4x4 matrix out of the 9x9 matrix around the cell addressed by
  // pad and time
  int rowindex=pClusterIndex->GetXIndex((float)padrow);
  int padstartindex=pClusterIndex->GetYIndex(pad);
  int timestartindex=pClusterIndex->GetZIndex(time);
  int cellindex=pClusterIndex->Index(rowindex, padstartindex, timestartindex);
  float centerpad=pClusterIndex->GetCenterY(cellindex);
  float centertime=pClusterIndex->GetCenterZ(cellindex);
  if ((TMath::Abs(centerpad-pad)>fMaxDeltaPad && pad>0.) ||
      (TMath::Abs(centertime-time)>fMaxDeltaTime && time>0.)) {
    ALIHLTERRORGUARD(20, "invalid pad center calculation, please check dimensions if dimensions of index grid match the maximum possible deviation");
  }

  int paddirection=1;
  int timedirection=1;
  if (centerpad>pad) paddirection=-1;
  if (centertime>time) timedirection=-1;
  for (int padcount=0, padindex=padstartindex; padcount<2; padcount++, padindex+=paddirection) {
    if (padindex<0) continue;
    if (padindex>=pClusterIndex->GetDimensionY()) break;
    for (int timecount=0, timeindex=timestartindex; timecount<2; timecount++, timeindex+=timedirection) {
      if (timeindex<0) continue;
      if (timeindex>=pClusterIndex->GetDimensionZ()) break;
      cellindex=pClusterIndex->Index(rowindex, padindex, timeindex);
      float cellpad=pClusterIndex->GetCenterY(cellindex);
      float celltime=pClusterIndex->GetCenterZ(cellindex);
      for (AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid::iterator& cl=pClusterIndex->begin((float)padrow, cellpad, celltime);
	   cl!=pClusterIndex->end(); cl++) {
	if (cl.Data().fTrackId>=0) continue;
	if (clusterId!=(~(AliHLTUInt32_t)0) && clusterId!=cl.Data().fId) continue;
	if (TMath::Abs(padrow-pClusters->GetX(cl.Data().fId))>=1.) {
	  HLTError("cluster 0x%08x: mismatch on padrow: trackpoint %d  cluster %f", cl.Data().fId, padrow, pClusters->GetX(cl.Data().fId));
	  continue;
	}
	float clusterpad=pClusters->GetY(cl.Data().fId);
	float clustertime=pClusters->GetZ(cl.Data().fId);
	if (TMath::Abs(clusterpad-pad)<fMaxDeltaPad &&
	    TMath::Abs(clustertime-time)<fMaxDeltaTime) {
	  // add this cluster to the track point and mark in the index grid
	  cl.Data().fTrackId=trackId;
	  pTrackPoint->AddAssociatedSpacePoint(cl.Data().fId, clusterpad-pad, clustertime-time);
	  count++;
	}
	if (clusterId!=(~(AliHLTUInt32_t)0)) break;
      }
    }
  }
  return count;
}

int AliHLTTPCDataCompressionComponent::WriteTrackClusters(const vector<AliHLTGlobalBarrelTrack>& tracks,
							  AliHLTSpacePointContainer* pSpacePoints,
							  AliHLTDataDeflater* pDeflater,
							  AliHLTUInt8_t* outputPtr,
							  AliHLTUInt32_t capacity) const
{
  // write the track data block including all associated clusters
  AliHLTUInt32_t size=0;
  for (vector<AliHLTGlobalBarrelTrack>::const_iterator track=tracks.begin();
       track!=tracks.end();
       track++) {
    if (!track->GetTrackGeometry()) {
      HLTError("can not find track geometry for track %d", track->GetID());
      return -EBADF;
    }
    AliHLTTPCTrackGeometry* pTrackPoints=dynamic_cast<AliHLTTPCTrackGeometry*>(track->GetTrackGeometry());
    if (!pTrackPoints) {
      HLTError("invalid track geometry type for track %d, expecting AliHLTTPCTrackGeometry", track->GetID());
      return -EBADF;
    }

    int result=pTrackPoints->Write(*track, pSpacePoints, pDeflater, outputPtr+size, capacity-size, fpWrittenAssociatedClusterIds);
    if (result<0) {
      HLTError("failed to write track points for track %d with error code %d, aborting", track->GetID(), result);
      return result;
    }
    size+=result;

    UInt_t nofTrackPoints=track->GetNumberOfPoints();
    const UInt_t* trackPoints=track->GetPoints();

    int assignedPoints=0;
    int assignedTrackPoints=0;
    const vector<AliHLTTrackGeometry::AliHLTTrackPoint>& rawPoints=pTrackPoints->GetRawPoints();
    for (vector<AliHLTTrackGeometry::AliHLTTrackPoint>::const_iterator point=rawPoints.begin();
	 point!=rawPoints.end(); point++) {
      const vector<AliHLTTrackGeometry::AliHLTTrackSpacepoint>& spacePoints=point->GetSpacepoints();
      for (vector<AliHLTTrackGeometry::AliHLTTrackSpacepoint>::const_iterator spacePoint=spacePoints.begin();
	   spacePoint!=spacePoints.end(); spacePoint++) {
	float dpad=spacePoint->GetResidual(0);
	float dtime=spacePoint->GetResidual(1);
	if (dpad>-1000 && dtime>-1000 && fHistoResidualPad && fHistoResidualTime) {
	  fHistoResidualPad->Fill(dpad);
	  fHistoResidualTime->Fill(dtime);
	}
	assignedPoints++;
	for (unsigned i=0; i<nofTrackPoints; i++) {
	  if (trackPoints[i]==spacePoint->fId) {
	    assignedTrackPoints++;
	    break;
	  }
	}
      }
    }
    if (fHistoClustersOnTracks) {
      fHistoClustersOnTracks->Fill(assignedPoints);
    }
    if (fHistoTrackClusterRatio && nofTrackPoints>0) {
      float ratio=assignedTrackPoints; ratio/=nofTrackPoints;
      fHistoTrackClusterRatio->Fill(ratio);
    }
  }
  return size;
}

int AliHLTTPCDataCompressionComponent::DoInit( int argc, const char** argv )
{
  /// inherited from AliHLTComponent: component initialisation and argument scan.
  int iResult=0;

  // component configuration
  //Stage 1: default initialization.
  //Default values.

  //Stage 2: OCDB.
  TString cdbPath("HLT/ConfigTPC/");
  cdbPath += GetComponentID();
  //
  iResult = ConfigureFromCDBTObjString(cdbPath);
  if (iResult < 0)
    return iResult;

  //Stage 3: command line arguments.
  int mode=fMode; // just a backup for the info message below
  int deflaterMode=fDeflaterMode;
  if (argc && (iResult = ConfigureFromArgumentString(argc, argv)) < 0)
    return iResult;

  if (mode!=fMode || deflaterMode!=fDeflaterMode) {
    HLTInfo("configured from command line: mode %d, deflater mode %d", fMode, fDeflaterMode);
  }

  AUTO_PTR<AliHLTComponentBenchmark> benchmark(new AliHLTComponentBenchmark);
  if (benchmark.get()) {
    benchmark->SetTimer(0,"total");
    benchmark->SetTimer(1,"rawclusterinput");
    benchmark->SetTimer(2,"clusterinput");
    benchmark->SetTimer(3,"trackinput");
    benchmark->SetTimer(4,"processing");
    benchmark->SetTimer(5,"output");
  } else {
    return -ENOMEM;
  }

  unsigned spacePointContainerMode=0;
  if (fMode==kCompressionModeV1TrackModel || fMode==kCompressionModeV2TrackModel) {
    // initialize map data for cluster access in the track association loop
    spacePointContainerMode|=AliHLTTPCRawSpacePointContainer::kModeCreateMap;
  }
  if (fMode==kCompressionModeV2 || fMode==kCompressionModeV2TrackModel) {
    // optimized storage format: differential pad and time storage
    spacePointContainerMode|=AliHLTTPCRawSpacePointContainer::kModeDifferentialPadTime;
  }
  AUTO_PTR<AliHLTTPCRawSpacePointContainer> rawInputClusters(new AliHLTTPCRawSpacePointContainer(spacePointContainerMode, fCreateFlags));
  AUTO_PTR<AliHLTTPCSpacePointContainer> inputClusters(new AliHLTTPCSpacePointContainer);

  AUTO_PTR<TH1F> histoCompFactor(new TH1F("CompressionFactor",
					       "HLT TPC data compression factor",
					       100, 0., 10.));
  AUTO_PTR<TH1F> histoResidualPad(new TH1F("PadResidual",
						"HLT TPC pad residual",
						100, -fMaxDeltaPad, fMaxDeltaPad));
  AUTO_PTR<TH1F> histoResidualTime(new TH1F("TimeResidual",
						 "HLT TPC time residual",
						 100, -fMaxDeltaTime, fMaxDeltaTime));
  AUTO_PTR<TH1F> histoClustersOnTracks(new TH1F("ClustersOnTracks",
						 "Clusters in track model compression",
						 200, 0., 600));
  AUTO_PTR<TH1F> histoClusterRatio(new TH1F("ClusterRatio",
						 "Fraction of clusters in track model compression",
						 100, 0., 1.));
  AUTO_PTR<TH1F> histoTrackClusterRatio(new TH1F("UsedTrackClusters",
						 "Fraction of track clusters in track model compression",
						 100, 0., 1.));

  // track grid: 36 slices, each 6 partitions with max 33 rows
  fTrackGrid=new AliHLTTrackGeometry::AliHLTTrackGrid(36, 1, 6, 1, 33, 1, 20000);
  fSpacePointGrid=AliHLTTPCRawSpacePointContainer::AllocateIndexGrid();

  if (!rawInputClusters.get() ||
      !inputClusters.get() ||
      !fTrackGrid ||
      !fSpacePointGrid) {
    if (fTrackGrid) delete fTrackGrid; fTrackGrid=NULL;
    if (fSpacePointGrid) delete fSpacePointGrid; fSpacePointGrid=NULL;
    return -ENOMEM;
  }

  if (fDeflaterMode>0 && (iResult=InitDeflater(fDeflaterMode))<0)
    return iResult;

  fpBenchmark=benchmark.release();
  fRawInputClusters=rawInputClusters.release();
  fInputClusters=inputClusters.release();

  // initialize the histograms if stored at the end
  // condition might be extended
  if (!fHistogramFile.IsNull()) {
    fHistoCompFactor=histoCompFactor.release();
    fHistoResidualPad=histoResidualPad.release();
    fHistoResidualTime=histoResidualTime.release();
    fHistoClustersOnTracks=histoClustersOnTracks.release();
    fHistoClusterRatio=histoClusterRatio.release();
    fHistoTrackClusterRatio=histoTrackClusterRatio.release();
  }

  // only init drift time if actually needed
  // i.e. when using track model compression
  // transform init takes ages.....
  if (fMode==kCompressionModeV1TrackModel || fMode==kCompressionModeV2TrackModel) {
    if (iResult>=0 && (iResult=InitDriftTimeTransformation())<0) return iResult;
  }

  HLTInfo("TPC Cluster compression running in mode %d / deflaterMode %d", fMode, fDeflaterMode);

  return iResult;
}

int AliHLTTPCDataCompressionComponent::InitDeflater(int mode)
{
  /// init the data deflater
  int iResult=0;
  if (mode==kDeflaterModeHuffman || mode==kDeflaterModeHuffmanTrainer) {
    // huffman deflater
    AUTO_PTR<AliHLTDataDeflaterHuffman> deflater(new AliHLTDataDeflaterHuffman(mode==kDeflaterModeHuffmanTrainer));
    if (!deflater.get()) return -ENOMEM;

    if (!deflater->IsTrainingMode()) {
      TObject* pConf=NULL;
      if (fHuffmanTableFile.IsNull()) {
      TString cdbPath("HLT/ConfigTPC/");
      cdbPath += GetComponentID();
      cdbPath += "HuffmanTables";
      pConf=LoadAndExtractOCDBObject(cdbPath);
      } else {
	// load huffman table directly from file
	TFile* tablefile = TFile::Open(fHuffmanTableFile);
	if (!tablefile || tablefile->IsZombie()) return -EBADF;
	TObject* obj = NULL;
	AliCDBEntry* cdbentry = NULL;
	tablefile->GetObject("AliCDBEntry", obj);
	if (obj == NULL || (cdbentry = dynamic_cast<AliCDBEntry*>(obj))==NULL) {
	  HLTError("can not read configuration object from file %s", fHuffmanTableFile.Data());
	  return -ENOENT;
	}
	HLTInfo("reading huffman table configuration object from file %s", fHuffmanTableFile.Data());
	pConf = cdbentry->GetObject();
      }
      if (!pConf) return -ENOENT;
      if (dynamic_cast<TList*>(pConf)==NULL) {
	HLTError("huffman table configuration object of inconsistent type");
	return -EINVAL;
      }
      iResult=deflater->InitDecoders(dynamic_cast<TList*>(pConf));
      if (iResult<0) return iResult;
    }
    
    if (!fHistogramFile.IsNull())
      deflater->EnableStatistics();

    unsigned nofParameters=AliHLTTPCDefinitions::GetNumberOfClusterParameterDefinitions();
    unsigned p=0;
    for (; p<nofParameters; p++) {
      const AliHLTTPCDefinitions::AliClusterParameter& parameter=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[p];
      // use the pad/time length as reference for the calculation of ratio for residuals
      unsigned refLength=0;
      unsigned refLengthPad=0;
      unsigned refLengthTime=0;
      if (parameter.fId==AliHLTTPCDefinitions::kPad)               refLengthPad=parameter.fBitLength;
      else if (parameter.fId==AliHLTTPCDefinitions::kTime)         refLengthTime=parameter.fBitLength;
      else if (parameter.fId==AliHLTTPCDefinitions::kResidualPad)  refLength=refLengthPad;
      else if (parameter.fId==AliHLTTPCDefinitions::kResidualTime) refLength=refLengthTime;

      if (deflater->AddParameterDefinition(parameter.fName,
					   parameter.fBitLength,
					   refLength)!=(int)parameter.fId) {
	// for performance reason the parameter id is simply used as index in the array of
	// definitions, the position must match the id
	HLTFatal("mismatch between parameter id and position in array for parameter %s, rearrange definitions!", parameter.fName);
	return -EFAULT;
      }
    }
    fpDataDeflater=deflater.release();
    return 0;
  }
  if (mode==kDeflaterModeSimple) {
    AUTO_PTR<AliHLTDataDeflaterSimple> deflater(new AliHLTDataDeflaterSimple);
    if (!deflater.get()) return -ENOMEM;

    if (!fHistogramFile.IsNull())
      deflater->EnableStatistics();

    unsigned nofParameters=AliHLTTPCDefinitions::GetNumberOfClusterParameterDefinitions();
    unsigned p=0;
    for (; p<nofParameters; p++) {
      const AliHLTTPCDefinitions::AliClusterParameter& parameter=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[p];
      if (deflater->AddParameterDefinition(parameter.fName,
					   parameter.fBitLength,
					   parameter.fOptional)!=(int)parameter.fId) {
	// for performance reason the parameter id is simply used as index in the array of
	// definitions, the position must match the id
	HLTFatal("mismatch between parameter id and position in array for parameter %s, rearrange definitions!", parameter.fName);
	return -EFAULT;
      }
    }
    fpDataDeflater=deflater.release();
    return 0;
  }
  HLTError("invalid deflater mode %d, allowed 1=simple 2=huffman", mode);
  return -EINVAL;
}

int AliHLTTPCDataCompressionComponent::DoDeinit()
{
  /// inherited from AliHLTComponent: component cleanup
  int iResult=0;
  if (fpBenchmark) delete fpBenchmark; fpBenchmark=NULL;
  if (fRawInputClusters) delete fRawInputClusters; fRawInputClusters=NULL;
  if (fInputClusters) delete fInputClusters; fInputClusters=NULL;
  if (!fHistogramFile.IsNull()) {
    TFile out(fHistogramFile, "RECREATE");
    if (!out.IsZombie()) {
      out.cd();
      if (fHistoCompFactor) fHistoCompFactor->Write();
      if (fHistoResidualPad) fHistoResidualPad->Write();
      if (fHistoResidualTime) fHistoResidualTime->Write();
      if (fHistoClusterRatio) fHistoClusterRatio->Write();
      if (fHistoClustersOnTracks) fHistoClustersOnTracks->Write();
      if (fHistoTrackClusterRatio) fHistoTrackClusterRatio->Write();
      out.Close();
    }
  }
  if (fHistoCompFactor) delete fHistoCompFactor;
  fHistoCompFactor=NULL;
  if (fHistoResidualPad) delete fHistoResidualPad;
  fHistoResidualPad=NULL;
  if (fHistoResidualTime) delete fHistoResidualTime;
  fHistoResidualTime=NULL;
  if (fHistoClustersOnTracks) delete fHistoClustersOnTracks;
  fHistoClustersOnTracks=NULL;
  if (fHistoClusterRatio) delete fHistoClusterRatio;
  fHistoClusterRatio=NULL;
  if (fHistoTrackClusterRatio) delete fHistoTrackClusterRatio;
  fHistoTrackClusterRatio=NULL;

  if (fpDataDeflater) {
    if (!fHistogramFile.IsNull()) {
      TString filename=fHistogramFile;
      filename.ReplaceAll(".root", "-deflater.root");
      fpDataDeflater->SaveAs(filename);
    }
    if (fDeflaterMode==kDeflaterModeHuffmanTrainer) {
      if (fHuffmanTableFile.IsNull()) {
	fHuffmanTableFile=GetComponentID();
	fHuffmanTableFile+="-huffman.root";
      }
      // TODO: currently, the code tables are also calculated in FindObject
      // check if a different function is more appropriate
      TObject* pConf=fpDataDeflater->FindObject("DeflaterConfiguration");
      if (pConf) {
	TString cdbEntryPath("HLT/ConfigTPC/");
	cdbEntryPath += GetComponentID();
	cdbEntryPath += "HuffmanTables";
	AliCDBPath cdbPath(cdbEntryPath);
	AliCDBId cdbId(cdbPath, AliCDBManager::Instance()->GetRun(), AliCDBRunRange::Infinity(), 0, 0);
	AliCDBMetaData* cdbMetaData=new AliCDBMetaData;
	cdbMetaData->SetResponsible("ALICE HLT Matthias.Richter@cern.ch");
	cdbMetaData->SetComment("Huffman encoder configuration");
	AliCDBEntry* entry=new AliCDBEntry(pConf, cdbId, cdbMetaData, kTRUE);

	entry->SaveAs(fHuffmanTableFile);
	// this is a small memory leak
	// seg fault in ROOT object handling if the two objects are deleted
	// investigate later
	//delete entry;
	//delete cdbMetaData;
      }
    }
    delete fpDataDeflater;
  }
  fpDataDeflater=NULL;


  if (fTrackGrid) delete fTrackGrid; fTrackGrid=NULL;
  if (fSpacePointGrid) delete fSpacePointGrid; fSpacePointGrid=NULL;

  return iResult;
}

int AliHLTTPCDataCompressionComponent::ScanConfigurationArgument(int argc, const char** argv)
{
  /// inherited from AliHLTComponent: argument scan
  int iResult=0;
  if (argc<1) return 0;
  int bMissingParam=0;
  int i=0;
  TString argument=argv[i];

  do {
    // -mode
    if (argument.CompareTo("-mode")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString parameter=argv[i];
      if (parameter.IsDigit()) {
	fMode=parameter.Atoi();
	return 2;
      } else {
	HLTError("invalid parameter for argument %s, expecting number instead of %s", argument.Data(), parameter.Data());
	return -EPROTO;
      }
    }

    // -create-flags
    if (argument.CompareTo("-create-flags")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString parameter=argv[i];
      if (parameter.IsDigit()) {
	fCreateFlags=parameter.Atoi();
	return 2;
      } else {
	HLTError("invalid parameter for argument %s, expecting number instead of %s", argument.Data(), parameter.Data());
	return -EPROTO;
      }
    }

    // -deflater-mode
    if (argument.CompareTo("-deflater-mode")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString parameter=argv[i];
      if (parameter.IsDigit()) {
	fDeflaterMode=parameter.Atoi();
	return 2;
      } else {
	HLTError("invalid parameter for argument %s, expecting number instead of %s", argument.Data(), parameter.Data());
	return -EPROTO;
      }
    }

    // -histogram-file
    if (argument.CompareTo("-histogram-file")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      fHistogramFile=argv[i++];
      return 2;
    }
    // -save-huffman-table
    if (argument.CompareTo("-save-huffman-table")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      fHuffmanTableFile=argv[i++];
      return 2;
    }
    // -load-huffman-table
    if (argument.CompareTo("-load-huffman-table")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      fHuffmanTableFile=argv[i++];
      return 2;
    }
    // -cluster-verification
    if (argument.CompareTo("-cluster-verification")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString parameter=argv[i];
      if (parameter.IsDigit()) {
	fVerificationMode=parameter.Atoi();
	return 2;
      } else {
	HLTError("invalid parameter for argument %s, expecting number instead of %s", argument.Data(), parameter.Data());
	return -EPROTO;
      }
    }
    // -cluster-ids
    if (argument.CompareTo("-cluster-ids")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString parameter=argv[i];
      if (parameter.IsDigit()) {
	fProvideClusterIds = parameter.Atoi();
	return 2;
      } else {
	HLTError("invalid parameter for argument %s, expecting number instead of %s", argument.Data(), parameter.Data());
	return -EPROTO;
      }
    }
  } while (0); // using do-while only to have break available

  if (bMissingParam) {
    HLTError("missing parameter for argument %s", argument.Data());
    iResult=-EPROTO;
  }

  return iResult;
}

int AliHLTTPCDataCompressionComponent::InitDriftTimeTransformation()
{
  /// calculate correction factor and offset for a linear approximation of the
  /// drift time transformation, separately for A and C side
  int iResult=0;
  AliHLTTPCClusterTransformation transform;
  if ((iResult=transform.Init( GetBz(), GetTimeStamp(), false, 0))<0) {
    HLTError("failed to init AliHLTTPCClusterTransformation: %d", iResult);
    return iResult;
  }

#ifdef HAVE_ALIGPU
  /*if ((iResult=CalculateDriftTimeTransformation(transform, 0, 0, fDriftTimeFactorA, fDriftTimeOffsetA))<0) return iResult;
  if (fVerbosity>0) HLTInfo("drift time transformation A side: m=%f n=%f", fDriftTimeFactorA, fDriftTimeOffsetA);
  if ((iResult=CalculateDriftTimeTransformation(transform, 18, 0, fDriftTimeFactorC, fDriftTimeOffsetC))<0) return iResult;
  if (fVerbosity>0) HLTInfo("drift time transformation C side: m=%f n=%f", fDriftTimeFactorC, fDriftTimeOffsetC);*/
#endif

  return 0;
}

#ifdef HAVE_ALIGPU
/*template <class T> static inline int GenericCalculateDriftTimeTransformation(T& transform,
									int slice, int padrow,
									float& m, float& n,
									AliHLTTPCReverseTransformInfoV1* rev)
{
  /// calculate correction factor and offset for a linear approximation of the
  /// drift time transformation by just probing the range of timebins with
  /// AliHLTTPCClusterTransformation
  const int nofSteps=100;
  vector<float> zvalues;
  vector<float> tvalues;

  int nofTimebins=AliHLTTPCGeometry::GetNTimeBins();
  int stepWidth=nofTimebins/nofSteps;
  int time=0;
  int count=0;
  float meanT=0.;
  float meanZ=0.;
  for (time=0; time<nofTimebins; time+=stepWidth) {
    Float_t xyz[3];
    transform.Transform(slice, padrow, 0, time, xyz);
    float mytime = time;
    if (rev)
    {
	Float_t padtimeref[2];
	AliHLTTPCClusterStatComponent::TransformReverse(slice, padrow, xyz[1], xyz[2], padtimeref, rev);
	mytime -= padtimeref[1];
    }
    if ((slice < 18) ^ (xyz[2] > 0)) continue;
    count++;
    zvalues.push_back(xyz[2]);
    tvalues.push_back(mytime);
    meanT+=mytime;
    meanZ+=xyz[2];
  }
  if (count==0) count=1;
  meanT/=count;
  meanZ/=count;
  float sumTZ=.0;
  float sumT2=.0;
  vector<float>::const_iterator t=tvalues.begin();
  for (vector<float>::const_iterator z=zvalues.begin(); z!=zvalues.end(); z++, t++) {
    sumTZ+=(*t-meanT)*(*z-meanZ);
    sumT2+=(*t-meanT)*(*t-meanT);
  }
  m=sumTZ/sumT2;
  n=meanZ-m*meanT;

  for (float z = 0;fabs(z) < 250;z += slice < 18 ? 10 : -10)
  {
	float test = (z - n) / m;
  }

  return 0;
}

int AliHLTTPCDataCompressionComponent::CalculateDriftTimeTransformation(AliHLTTPCClusterTransformation& transform, int slice, int padrow, float& m, float& n, AliHLTTPCReverseTransformInfoV1* rev) {
    return GenericCalculateDriftTimeTransformation(transform, slice, padrow, m, n, rev);
}
int AliHLTTPCDataCompressionComponent::CalculateDriftTimeTransformation(AliHLTTPCFastTransform& transform, int slice, int padrow, float& m, float& n, AliHLTTPCReverseTransformInfoV1* rev) {
    return GenericCalculateDriftTimeTransformation(transform, slice, padrow, m, n, rev);
}*/
#endif
