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

/// @file   AliHLTTPCDataCompressionMonitorComponent.cxx
/// @author Matthias Richter
/// @date   2011-09-12
/// @brief  TPC component for monitoring of data compression
///

#include "AliHLTTPCDataCompressionMonitorComponent.h"
#include "AliHLTTPCDataCompressionComponent.h"
#include "AliHLTTPCDataCompressionDecoder.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCHWCFData.h"
#include "AliHLTTPCDefinitions.h"
#include "AliHLTTPCClusterDataFormat.h"
#include "AliHLTTPCRawCluster.h"
#include "AliHLTTPCGeometry.h"
#include "AliHLTTPCTrackGeometry.h"
#include "AliHLTTPCHWCFSpacePointContainer.h"
#include "AliHLTTPCRawSpacePointContainer.h"
#include "AliHLTErrorGuard.h"
#include "AliHLTComponentBenchmark.h"
#include "AliHLTCDHWrapper.h"
#include "AliTPCclusterMI.h"
#include "AliTPCROC.h"
#include "TH1I.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2I.h"
#include "TH2F.h"
#include "TH2D.h"
#include "TH3I.h"
#include "TH3F.h"
#include "TH3D.h"
#include "TProfile.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TList.h"
#include <memory>

#if __cplusplus > 201402L
#define AUTO_PTR std::unique_ptr
#else
#define AUTO_PTR std::auto_ptr
#endif

ClassImp(AliHLTTPCDataCompressionMonitorComponent)

AliHLTTPCDataCompressionMonitorComponent::AliHLTTPCDataCompressionMonitorComponent()
  : AliHLTProcessor()
  , fpBenchmark(NULL)
  , fpDecoder(NULL)
  , fpHWClusterDecoder(NULL)
  , fHistoHWCFDataSize(NULL)
  , fHistoHWCFReductionFactor(NULL)
  , fHistoTotalReductionFactor(NULL)
  , fHistoNofClusters(NULL)
  , fHistoNofClustersReductionFactor(NULL)
  , fHistogramFile()
  , fHuffmanTableFile()
  , fMonitoringContainer(NULL)
  , fVerbosity(0)
  , fFlags(0)
  , fPublishingMode(kPublishSeparate)
{
}

AliHLTTPCDataCompressionMonitorComponent::~AliHLTTPCDataCompressionMonitorComponent()
{
  /// destructor
}


const char* AliHLTTPCDataCompressionMonitorComponent::GetComponentID()
{
  /// inherited from AliHLTComponent: id of the component
  return "TPCDataCompressorMonitor";
}


void AliHLTTPCDataCompressionMonitorComponent::GetInputDataTypes( AliHLTComponentDataTypeList& tgtList)
{
  /// inherited from AliHLTComponent: list of data types in the vector reference
  tgtList.clear();
  tgtList.push_back(AliHLTTPCDefinitions::fgkHWClustersDataType);
  tgtList.push_back(kAliHLTDataTypeTrack|kAliHLTDataOriginTPC);
  tgtList.push_back(AliHLTTPCDefinitions::fgkRawClustersDataType);
  tgtList.push_back(AliHLTTPCDefinitions::RemainingClustersCompressedDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClusterTracksCompressedDataType());  
  tgtList.push_back(AliHLTTPCDefinitions::RemainingClusterIdsDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClusterIdTracksDataType());
  tgtList.push_back(AliHLTTPCDefinitions::ClustersFlagsDataType());
}

AliHLTComponentDataType AliHLTTPCDataCompressionMonitorComponent::GetOutputDataType()
{
  /// inherited from AliHLTComponent: output data type of the component.
  return kAliHLTMultipleDataType;
}

int AliHLTTPCDataCompressionMonitorComponent::GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList)
{
  /// inherited from AliHLTComponent: multiple output data types of the component.
  tgtList.clear();
  tgtList.push_back(kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
  return tgtList.size();
}

void AliHLTTPCDataCompressionMonitorComponent::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
{
  /// inherited from AliHLTComponent: output data size estimator
  constBase=10000000;
  inputMultiplier=1.0;
}

AliHLTComponent* AliHLTTPCDataCompressionMonitorComponent::Spawn()
{
  /// inherited from AliHLTComponent: spawn function.
  return new AliHLTTPCDataCompressionMonitorComponent;
}

int AliHLTTPCDataCompressionMonitorComponent::DoEvent( const AliHLTComponentEventData& /*evtData*/, 
						       const AliHLTComponentBlockData* /*inputBlocks*/, 
						       AliHLTComponentTriggerData& /*trigData*/,
						       AliHLTUInt8_t* /*outputPtr*/,
						       AliHLTUInt32_t& /*size*/,
						       AliHLTComponentBlockDataList& /*outputBlocks*/ )
{
  /// inherited from AliHLTProcessor: data processing
  int iResult=0;

  AliHLTUInt32_t eventType=gkAliEventTypeUnknown;
  if (!IsDataEvent(&eventType)) {
    if (eventType==gkAliEventTypeEndOfRun && fPublishingMode!=kPublishOff) {
      iResult=Publish(fPublishingMode);
    }
    return iResult;
  }

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->StartNewEvent();
    GetBenchmarkInstance()->Start(0);
  }

  const AliHLTComponentBlockData* pDesc=NULL;
  unsigned rawDataSize=0;
  unsigned rawEventSizeFromRCUtrailer=0;
  unsigned hwclustersDataSize=0;
  unsigned rawclustersDataSize=0;
  unsigned nofCompressedClusters=0;
  unsigned nofClusters=0;
  unsigned compDataSize=0; 
  
  // check size of TPC raw data
  for (pDesc=GetFirstInputBlock(kAliHLTDataTypeDDLRaw | kAliHLTDataOriginTPC);
       pDesc!=NULL; pDesc=GetNextInputBlock()) {
    fFlags|=kHaveRawData;
    rawDataSize+=pDesc->fSize;
  }

  bool bUseHWCFDataForClusterCalculations=true;
  for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RawClustersDataType());
       pDesc!=NULL; pDesc=GetNextInputBlock()) {
    // first check the type of input, there has been a change in the input type
    // of the compression component in Jun 2013, see below
    // use a local variable here, which in general should do the job because
    // if there are is data available always HWCF and raw clusters should be
    // there is in the configuration. There shouldn't be any difference on
    // event basis.
    if (pDesc->fSize<sizeof(AliHLTTPCRawClusterData)) continue;
    const AliHLTTPCRawClusterData* clusterData = reinterpret_cast<const AliHLTTPCRawClusterData*>(pDesc->fPtr);
    if (!clusterData) continue;
    if (clusterData->fVersion==1) {
      bUseHWCFDataForClusterCalculations=false;
      break;
    }
  }

  // check size of HWCF data and add to the MonitoringContainer
  for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::fgkHWClustersDataType);
       pDesc!=NULL; pDesc=GetNextInputBlock()) {
    fFlags|=kHaveHWClusters;
    // FIXME: the decoding can now be handled via the data container
    AliHLTCDHWrapper header(pDesc->fPtr);
    if (pDesc->fSize<=header.GetHeaderSize()) continue;
    if (fpHWClusterDecoder) {
      hwclustersDataSize+=pDesc->fSize;
      AliHLTUInt8_t* pData=reinterpret_cast<AliHLTUInt8_t*>(pDesc->fPtr);
      pData+=header.GetHeaderSize();
      if (fpHWClusterDecoder->Init(pData, pDesc->fSize-header.GetHeaderSize())<0 ||
	  (fpHWClusterDecoder->CheckVersion()<0 && (int)(pDesc->fSize-header.GetHeaderSize())>fpHWClusterDecoder->GetRCUTrailerSize())) {
	HLTError("data block of type %s corrupted: can not decode format",
		 AliHLTComponent::DataType2Text(pDesc->fDataType).c_str());
      } else {
	if (bUseHWCFDataForClusterCalculations) nofClusters+=fpHWClusterDecoder->GetNumberOfClusters();
	if (fpHWClusterDecoder->GetRCUTrailer()) {
	  // first word of the RCU trailer contains the payload size in 32bit words
	  const AliHLTUInt32_t*  pRCUTrailer=reinterpret_cast<const AliHLTUInt32_t*>(fpHWClusterDecoder->GetRCUTrailer());
	  AliHLTUInt32_t payloadSize=(*pRCUTrailer)&0x00ffffff;
	  rawEventSizeFromRCUtrailer+=header.GetHeaderSize()
	    + payloadSize*sizeof(AliHLTUInt32_t)
	    + fpHWClusterDecoder->GetRCUTrailerSize();
	}
      }
    }
    if (fMonitoringContainer && bUseHWCFDataForClusterCalculations) {
      fMonitoringContainer->AddRawData(pDesc);
    }
  }

  if (fMonitoringContainer && fpDecoder) {
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Start(1);
    }

    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RemainingClusterIdsDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      iResult=fMonitoringContainer->AddClusterIds(pDesc);
    }

    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::ClusterIdTracksDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      iResult=fMonitoringContainer->AddClusterIds(pDesc);
    }

    // read data
    AliHLTTPCDataCompressionDecoder& decoder=*fpDecoder;
    decoder.Clear();

    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::ClustersFlagsDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      iResult=decoder.AddClusterFlags(pDesc);
    }
    
    if (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RawClustersDescriptorDataType())) {
        if ((iResult=decoder.AddRawClustersDescriptor(pDesc))<0) {
            return iResult;
        }
    }
    //CompressionDescriptor should have priority over rawcluster descriptor in case both are present, because this describes the actual compressed data.
    if (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::DataCompressionDescriptorDataType())) {
        if ((iResult=decoder.AddCompressionDescriptor(pDesc))<0) {
            return iResult;
        }
    }

    bool bHaveRawClusters=false;
    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RawClustersDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      // Note: until r51411 and v5-01-Rev-03 the compressed cluster format was sent with data
      // type {CLUSTRAW,TPC }, the version member indicated the actual type of data
      // These data do not include the 0.5 shift in pad position, that's why it has
      // to be added in the unpacking. This is a very special case, this data type and
      // data version==1 only occured in the early TPC data compression test runs with
      // v5-01-Rev-01
      // Additional correction 2015-02-26: the input data type of the compression component
      // has been changed to raw clusters instead of HWCF clusters on Jun 27 2013 in commit
      // 49bdc4660e6b95428c4c1fb9403fc17fad34dc9d. This has implications to the input of the
      // monitoring coponent. Raw clusters have now to be used to monitor the differences of
      // original and compressed cluster parameters. The case of reading compressed clusters
      // with data type {CLUSTRAW,TPC } and version!=1 has been removed as data with
      // this format have never been recorded.
      if (pDesc->fSize<sizeof(AliHLTTPCRawClusterData)) continue;
      const AliHLTTPCRawClusterData* clusterData = reinterpret_cast<const AliHLTTPCRawClusterData*>(pDesc->fPtr);
      if (!clusterData) continue;
      if (clusterData->fVersion==1) {
	// compressed clusters without the pad shift
	// data type {CLUSTRAW,TPC } with version==1
	decoder.SetPadShift(0.5);
        bHaveRawClusters=true;
        iResult=decoder.ReadClustersPartition(fMonitoringContainer->BeginRemainingClusterBlock(0, pDesc->fSpecification),
                                              reinterpret_cast<AliHLTUInt8_t*>(pDesc->fPtr),
                                              pDesc->fSize,
                                              pDesc->fSpecification);
        if (iResult<0) {
        HLTError("reading of partition clusters failed with error %d", iResult);
        }
      } else {
	rawclustersDataSize+=pDesc->fSize;
	if (sizeof(AliHLTTPCRawClusterData)+clusterData->fCount*sizeof(AliHLTTPCRawCluster)==pDesc->fSize) {
	  nofClusters+=clusterData->fCount;
	  if (fMonitoringContainer) {
	    fMonitoringContainer->AddRawData(pDesc);
	  }
	} else {
	  ALIHLTERRORGUARD(5, "inconsistent data block of raw clusters");
	}
      }
    }

    decoder.SetPadShift(0.0);

    if (!bHaveRawClusters) {
    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::RemainingClustersCompressedDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      iResult=decoder.ReadClustersPartition(fMonitoringContainer->BeginRemainingClusterBlock(0, pDesc->fSpecification),
					    reinterpret_cast<AliHLTUInt8_t*>(pDesc->fPtr),
					    pDesc->fSize,
					    pDesc->fSpecification);
      if (iResult>=0) {
	compDataSize+=pDesc->fSize;
	nofCompressedClusters+=iResult;
      }
    }

    for (pDesc=GetFirstInputBlock(AliHLTTPCDefinitions::ClusterTracksCompressedDataType());
	 pDesc!=NULL; pDesc=GetNextInputBlock()) {
      iResult=decoder.ReadTrackModelClustersCompressed(fMonitoringContainer->BeginTrackModelClusterBlock(0),
					       reinterpret_cast<AliHLTUInt8_t*>(pDesc->fPtr),
					       pDesc->fSize,
					       pDesc->fSpecification);
      if (iResult>=0) {
	compDataSize+=pDesc->fSize;
	nofCompressedClusters+=iResult;
      }
    }
    } else {
      if (GetFirstInputBlock(AliHLTTPCDefinitions::RemainingClustersCompressedDataType()) ||
	  GetFirstInputBlock(AliHLTTPCDefinitions::ClusterTracksCompressedDataType())) {
	ALIHLTERRORGUARD(5, "conflicting data blocks, monitoring histograms already filled from raw cluster data, ignoring blocks of compressed partition and track clusters");
      }		     
    }
    if (GetBenchmarkInstance()) {
      GetBenchmarkInstance()->Stop(1);
    }

    fMonitoringContainer->Clear();
  }

  if ((fFlags&kHaveHWClusters)!=0 && (fFlags&kHaveRawData)!=0) {
    if (rawDataSize!=rawEventSizeFromRCUtrailer && rawEventSizeFromRCUtrailer>0) {
      HLTError("got different raw event size from raw data and rcu trailer: raw %d, rcu trailer %d", rawDataSize, rawEventSizeFromRCUtrailer);
    }
  }
  if (rawDataSize==0)
    rawDataSize=rawEventSizeFromRCUtrailer;

  float hwcfratio=0;
  float ratio=0;
  float totalratio=0;
  if (nofClusters==0 && nofCompressedClusters>0) {
    // no information from original data, skip calculations and print a short message
    HLTInfo("comp data %d, %d clusters\n", compDataSize, nofCompressedClusters);
  } else {
  // the monitoring component can now handle both cases of AliHLTTPCDataCompressionComponent input
  // 1) HWCF clusters: the original implementation
  // 2) RAW clusters: changed in commit 49bdc4660e6b95428c4c1fb9403fc17fad34dc9d Jun 27 2013
  // if there are no raw clusters as input we take the HWCF clusters as reference size
  if (hwclustersDataSize) {hwcfratio=(float)rawDataSize; hwcfratio/=hwclustersDataSize;}
  if (rawclustersDataSize==0) rawclustersDataSize=hwclustersDataSize;
  if (compDataSize) {ratio=(float)rawclustersDataSize; ratio/=compDataSize;}
  if (compDataSize) {totalratio=(float)rawDataSize; totalratio/=compDataSize;}
  if (fHistoHWCFDataSize)        fHistoHWCFDataSize       ->Fill(rawDataSize/1024, hwclustersDataSize/1024);
  if (fHistoHWCFReductionFactor) fHistoHWCFReductionFactor->Fill(rawDataSize/1024, hwcfratio);
  if (fHistoTotalReductionFactor && nofClusters>0)
    fHistoTotalReductionFactor->Fill(rawDataSize/1024, totalratio);
  if (fHistoNofClusters)         fHistoNofClusters        ->Fill(rawDataSize/1024, nofClusters);
  if (fHistoNofClustersReductionFactor && nofClusters>0)
    fHistoNofClustersReductionFactor ->Fill(nofClusters, ratio);
  HLTInfo("raw data %d, raw/hwcf cluster data %d, comp data %d, ratio %.2f, %d clusters, total compression ratio %.2f\n", rawDataSize, rawclustersDataSize, compDataSize, ratio, nofClusters, totalratio);
  }

  if (iResult>=0 && fPublishingMode!=kPublishOff) {
    iResult=Publish(fPublishingMode);
  }

  if (GetBenchmarkInstance()) {
    GetBenchmarkInstance()->Stop(0);
    GetBenchmarkInstance()->AddInput(compDataSize);
    HLTBenchmark("%s", GetBenchmarkInstance()->GetStatistics());
  }

  return iResult;
}

int AliHLTTPCDataCompressionMonitorComponent::Publish(int mode)
{
  /// publish to output
  // additional histograms derived from the main ones to publish
  TObjArray *derivedHistos = new TObjArray();
  derivedHistos->SetOwner(kTRUE);

  // FIXME: code needs to be optimized, maybe a bit to much new and delete for the
  // moment, the data type might need adjustment
  int iResult=0;
  TObjArray* pArray=mode==kPublishArray?(new TObjArray):NULL;
  TList* pList=mode==kPublishList?(new TList):NULL;
  if (mode==kPublishSeparate) {
    if (fHistoHWCFDataSize && fHistoHWCFDataSize->GetEntries())        PushBack(fHistoHWCFDataSize       , kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
    if (fHistoHWCFReductionFactor && fHistoHWCFReductionFactor->GetEntries()) PushBack(fHistoHWCFReductionFactor, kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
    if (fHistoNofClusters && fHistoNofClusters->GetEntries())         PushBack(fHistoNofClusters        , kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
    if (fHistoNofClustersReductionFactor && fHistoNofClusters->GetEntries()) PushBack(fHistoNofClustersReductionFactor, kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
    if (fHistoTotalReductionFactor && fHistoNofClusters->GetEntries()) PushBack(fHistoTotalReductionFactor, kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
  } else if (pList) {
    if (fHistoHWCFDataSize)        pList->Add(fHistoHWCFDataSize->Clone());
    if (fHistoHWCFReductionFactor) pList->Add(fHistoHWCFReductionFactor->Clone());
    if (fHistoNofClusters)         pList->Add(fHistoNofClusters->Clone());
    if (fHistoNofClustersReductionFactor) pList->Add(fHistoNofClustersReductionFactor->Clone());
    if (fHistoTotalReductionFactor) pList->Add(fHistoTotalReductionFactor->Clone());
  } else if (pArray) {
    if (fHistoHWCFDataSize)        pArray->Add(fHistoHWCFDataSize->Clone());
    if (fHistoHWCFReductionFactor) pArray->Add(fHistoHWCFReductionFactor->Clone());
    if (fHistoNofClusters)         pArray->Add(fHistoNofClusters->Clone());
    if (fHistoNofClustersReductionFactor) pArray->Add(fHistoNofClustersReductionFactor->Clone());
    if (fHistoTotalReductionFactor) pArray->Add(fHistoTotalReductionFactor->Clone());
  }


  if (fMonitoringContainer) {
    static const char* searchIds[] = {"fHistograms", "fHistograms2D", "fHistograms3D", NULL};
    const char** searchId=searchIds;
    while (*searchId && iResult>=0) {
      const TObject* o=fMonitoringContainer->FindObject(*searchId);
      if (o) {
	const TObjArray* histograms=dynamic_cast<const TObjArray*>(o);
	if (histograms) {
	  for (int i=0; i<histograms->GetEntriesFast() && iResult>=0; i++) {
	    if (!histograms->At(i)) continue;
	    ///
	    TString name=histograms->At(i)->GetName();
	    if( (name.CompareTo(fgkHistogramDefinitions2D[kHistogramQMaxSector].fName)==0) ||
		(name.CompareTo(fgkHistogramDefinitions2D[kHistogramSigmaY2Sector].fName)==0) ||
		(name.CompareTo(fgkHistogramDefinitions2D[kHistogramSigmaZ2Sector].fName)==0) ){
	      TH2F *h1=(TH2F*)histograms->At(i);
	      TProfile *h2 = (TProfile*)(h1->ProfileX());
	      derivedHistos->Add(h2);
	    }
	    if( name.CompareTo(fgkHistogramDefinitions3D[kHistogramPadrowPadSector].fName)==0) {
	      TH3F *h1=(TH3F*)histograms->At(i);
	      for (int j=1; j<=72; j++) {
	      h1->GetXaxis()->SetRange(j,j);
	      TString histoname = Form("zy_%d",j);
	      TH2F *h2 = (TH2F*)h1->Project3D(histoname.Data());
	      derivedHistos->Add(h2);
	      }
	    }
	    ///
	    if (mode==kPublishSeparate) {
        if (histograms->At(i) && ((TH1*)histograms->At(i))->GetEntries())
          iResult=PushBack(histograms->At(i), kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
	    } else if (pList) {
	      pList->Add(histograms->At(i)->Clone());
	    } else if (pArray) {
	      pArray->Add(histograms->At(i)->Clone());
	    }
	  }
	  for (int i=0; i<derivedHistos->GetEntriesFast() && iResult>=0; i++) {
	    if (mode==kPublishSeparate) {
        if (derivedHistos->At(i) && ((TH1*)derivedHistos->At(i))->GetEntries())
          iResult=PushBack(derivedHistos->At(i), kAliHLTDataTypeHistogram|kAliHLTDataOriginTPC);
	    } else if (pList) {
	      pList->Add(derivedHistos->At(i)->Clone());
	    } else if (pArray) {
	      pArray->Add(derivedHistos->At(i)->Clone());
	    }	    
	  }
	}
      } else {
	HLTError("failed to find object \"%s\"", *searchId);
      }
      searchId++;
    }
  }

  if (pArray) {
    iResult=PushBack(pArray, kAliHLTDataTypeTObjArray|kAliHLTDataOriginTPC);
    pArray->SetOwner(kTRUE);
    delete pArray;
    pArray=NULL;
  }
  if (pList) {
    iResult=PushBack(pList, kAliHLTDataTypeTObject|kAliHLTDataOriginTPC);
    pList->SetOwner(kTRUE);
    delete pList;
    pList=NULL;
  }
  return iResult;
}

int AliHLTTPCDataCompressionMonitorComponent::DoInit( int argc, const char** argv )
{
  /// inherited from AliHLTComponent: component initialisation and argument scan.
  int iResult=0;

  // component configuration
  //Stage 1: default initialization.
  //Default values.
  fFlags=0;

  //Stage 2: OCDB.
  TString cdbPath("HLT/ConfigTPC/");
  cdbPath += GetComponentID();
  //
  // iResult = ConfigureFromCDBTObjString(cdbPath);
  // if (iResult < 0) 
  //   return iResult;

  //Stage 3: command line arguments.
  if (argc && (iResult = ConfigureFromArgumentString(argc, argv)) < 0)
    return iResult;

  AUTO_PTR<AliHLTTPCHWCFData> hwClusterDecoder(new AliHLTTPCHWCFData);
  AUTO_PTR<AliDataContainer> dataContainer(new AliDataContainer);

  AUTO_PTR<TH2I> histoHWCFDataSize(new TH2I("HWCFDataSize",
						 "HW ClusterFinder Size",
						 100, 0., 80000., 100, 0., 80000.));
  if (histoHWCFDataSize.get()) {
    TAxis* xaxis=histoHWCFDataSize->GetXaxis();
    if (xaxis) xaxis->SetTitle("raw data size [kB]");
    TAxis* yaxis=histoHWCFDataSize->GetYaxis();
    if (yaxis) yaxis->SetTitle("compressed data size [kb]");
  }

  AUTO_PTR<TH2I> histoHWCFReductionFactor(new TH2I("HWCFReductionFactor",
							"Data reduction HW ClusterFinder vs. raw data size",
							100, 0., 80000., 30, 0., 3.));
  if (histoHWCFReductionFactor.get()) {
    TAxis* xaxis=histoHWCFReductionFactor->GetXaxis();
    if (xaxis) xaxis->SetTitle("raw data size [kB]");
    TAxis* yaxis=histoHWCFReductionFactor->GetYaxis();
    if (yaxis) yaxis->SetTitle("reduction factor");
  }

  AUTO_PTR<TH2I> histoTotalReductionFactor(new TH2I("TotalReductionFactor",
							 "Total reduction Factor vs. raw data size",
							 100, 0., 80000., 100, 0., 10.));
  if (histoTotalReductionFactor.get()) {
    TAxis* xaxis=histoTotalReductionFactor->GetXaxis();
    if (xaxis) xaxis->SetTitle("raw data size [kB]");
    TAxis* yaxis=histoTotalReductionFactor->GetYaxis();
    if (yaxis) yaxis->SetTitle("reduction factor");
  }

  AUTO_PTR<TH2I> histoNofClusters(new TH2I("NofClusters",
					       "Number of HLT TPC clusters",
					       100, 0., 80000., 500, 0., 3000000.));
  if (histoNofClusters.get()) {
    TAxis* xaxis=histoNofClusters->GetXaxis();
    if (xaxis) xaxis->SetTitle("raw data size [kB]");
    TAxis* yaxis=histoNofClusters->GetYaxis();
    if (yaxis) yaxis->SetTitle("N. of clusters");
  }

  AUTO_PTR<TH2I> histoNofClustersReductionFactor(new TH2I("ReductionFactorVsNofClusters",
							       "Reduction Factor vs. Number of HLT TPC clusters",
							       500, 0., 3000000., 100, 0., 10.));
  if (histoNofClustersReductionFactor.get()) {
    TAxis* xaxis=histoNofClustersReductionFactor->GetXaxis();
    if (xaxis) xaxis->SetTitle("N. of clusters");
    TAxis* yaxis=histoNofClustersReductionFactor->GetYaxis();
    if (yaxis) yaxis->SetTitle("reduction factor");
  }

  AUTO_PTR<AliHLTComponentBenchmark> benchmark(new AliHLTComponentBenchmark);
  if (benchmark.get()) {
    benchmark->SetTimer(0,"total");
    benchmark->SetTimer(1,"clusterdecoding");
  } else {
    return -ENOMEM;
  }

  AUTO_PTR<AliHLTTPCDataCompressionDecoder> decoder(new AliHLTTPCDataCompressionDecoder);
  if (!decoder.get()) {
    return -ENOMEM;
  }


  fHistoHWCFDataSize=histoHWCFDataSize.release();
  fHistoHWCFReductionFactor=histoHWCFReductionFactor.release();
  fHistoTotalReductionFactor=histoTotalReductionFactor.release();
  fHistoNofClusters=histoNofClusters.release();
  fHistoNofClustersReductionFactor=histoNofClustersReductionFactor.release();

  fpHWClusterDecoder=hwClusterDecoder.release();
  fMonitoringContainer=dataContainer.release();
  fpBenchmark=benchmark.release();
  fpDecoder=decoder.release();
  if (!fHuffmanTableFile.IsNull()) {
    fpDecoder->SetHuffmanTableConfiguration(fHuffmanTableFile.Data());
  }

  return iResult;
}

int AliHLTTPCDataCompressionMonitorComponent::DoDeinit()
{
  /// inherited from AliHLTComponent: component cleanup
  int iResult=0;

  if (fpBenchmark) delete fpBenchmark; fpBenchmark=NULL;
  if (fpDecoder) delete fpDecoder;
  fpDecoder=NULL;
  if (fpHWClusterDecoder) delete fpHWClusterDecoder;
  fpHWClusterDecoder=NULL;

  if (!fHistogramFile.IsNull()) {
    TFile out(fHistogramFile, "RECREATE");
    if (!out.IsZombie()) {
      out.cd();
      if (fHistoHWCFDataSize) fHistoHWCFDataSize->Write();
      if (fHistoHWCFReductionFactor) fHistoHWCFReductionFactor->Write();
      if (fHistoTotalReductionFactor) fHistoTotalReductionFactor->Write();
      if (fHistoNofClusters) fHistoNofClusters->Write();
      if (fHistoNofClustersReductionFactor) fHistoNofClustersReductionFactor->Write();
      if (fMonitoringContainer) {
	const TObject* o1=fMonitoringContainer->FindObject("fHistograms");
	const TObject* o2=fMonitoringContainer->FindObject("fHistograms2D");
	const TObject* o3=fMonitoringContainer->FindObject("fHistograms3D");
	if (o1) o1->Write();
	if (o2) o2->Write();
	if (o3) o3->Write();
      }
      out.Close();
    }
  }
  if (fHistoHWCFDataSize) delete fHistoHWCFDataSize;
  fHistoHWCFDataSize=NULL;
  if (fHistoHWCFReductionFactor) delete fHistoHWCFReductionFactor;
  fHistoHWCFReductionFactor=NULL;
  if (fHistoTotalReductionFactor) delete fHistoTotalReductionFactor;
  fHistoTotalReductionFactor=NULL;
  if (fHistoNofClusters) delete fHistoNofClusters;
  fHistoNofClusters=NULL;
  if (fHistoNofClustersReductionFactor) delete fHistoNofClustersReductionFactor;
  fHistoNofClustersReductionFactor=NULL;
  if (fMonitoringContainer) {
    fMonitoringContainer->Clear();
    delete fMonitoringContainer;
  }
  fMonitoringContainer=NULL;


  return iResult;
}

int AliHLTTPCDataCompressionMonitorComponent::ScanConfigurationArgument(int argc, const char** argv)
{
  /// inherited from AliHLTComponent: argument scan
  int iResult=0;
  if (argc<1) return 0;
  int bMissingParam=0;
  int i=0;
  TString argument=argv[i];

  do {
    // -histogram-file
    if (argument.CompareTo("-histogram-file")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      fHistogramFile=argv[i++];
      return i;
    }
    // -load-huffman-table
    if (argument.CompareTo("-load-huffman-table")==0) {
      // load huffman table from file instead OCDB
      if ((bMissingParam=(++i>=argc))) break;
      fHuffmanTableFile=argv[i++];
      return 2;
    }
    // -publishing-mode
    if (argument.CompareTo("-publishing-mode")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString option=argv[i++];
      if (option.CompareTo("off")==0)           fPublishingMode=kPublishOff     ;
      else if (option.CompareTo("separate")==0) fPublishingMode=kPublishSeparate;
      else if (option.CompareTo("list")==0)     fPublishingMode=kPublishList    ;
      else if (option.CompareTo("array")==0)    fPublishingMode=kPublishArray   ;
      else {
	HLTError("invalid option \"%s\" for argument \"%s\", expecting 'off', 'separate', 'list', or 'array'", option.Data(), argument.Data());
	return -EPROTO;
      }
      return i;
    }
  } while (0); // using do-while only to have break available

  if (bMissingParam) {
    HLTError("missing parameter for argument %s", argument.Data());
    iResult=-EPROTO;
  }

  return iResult;
}

AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::AliDataContainer()
  : fHistograms(new TObjArray)  
  , fHistograms2D(new TObjArray)  
  , fHistograms3D(new TObjArray)    
  , fHistogramPointers()
  , fHistogram2DPointers()
  , fHistogram3DPointers()
  , fRemainingClusterIds()
  , fTrackModelClusterIds()
  , fCurrentClusterIds(NULL)
  , fRawData(NULL)
  , fCurrentCluster()
  , fSector(-1)
  , fBegin()
  , fMaxSigmaY2Scaled((0x1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fBitLength)-1)
  , fMaxSigmaZ2Scaled((0x1<<AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fBitLength)-1)
{
  /// constructor
  memset(&fCurrentCluster, 0, sizeof(AliHLTTPCRawCluster));
  if (fHistograms) {
    fHistograms->SetOwner(kTRUE);
    fHistogramPointers.resize(kNumberOfHistograms, NULL);
    for (const AliHistogramDefinition* definition=fgkHistogramDefinitions;
	 definition->fName!=NULL; definition++) {
      fHistogramPointers[definition->fId]=new TH1D(definition->fName,
						  definition->fTitle,
						  definition->fBins,
						  definition->fLowerBound,
						  definition->fUpperBound
						  );
      if (fHistogramPointers[definition->fId]) {
	fHistogramPointers[definition->fId]->SetOption(definition->fDrawOptions);
      fHistograms->AddAt(fHistogramPointers[definition->fId], definition->fId);
      }
    }
  }
  ///
  if (fHistograms2D) {
    fHistograms2D->SetOwner(kTRUE);
    fHistogram2DPointers.resize(kNumberOfHistograms2D, NULL);
    for (const AliHistogramDefinition2D* definition=fgkHistogramDefinitions2D;
  	 definition->fName!=NULL; definition++) {
      fHistogram2DPointers[definition->fId]=new TH2D(definition->fName,
						     definition->fTitle,
						     definition->fBinsX,
						     definition->fLowerBoundX,
						     definition->fUpperBoundX,
						     definition->fBinsY,
						     definition->fLowerBoundY,
						     definition->fUpperBoundY
						     );
      if (fHistogram2DPointers[definition->fId]) {
	fHistogram2DPointers[definition->fId]->SetOption(definition->fDrawOptions);
      fHistograms2D->AddAt(fHistogram2DPointers[definition->fId], definition->fId);
      }
    }
  }
  ///
  if (fHistograms3D) {
    fHistograms3D->SetOwner(kTRUE);
    fHistogram3DPointers.resize(kNumberOfHistograms3D, NULL);
    for (const AliHistogramDefinition3D* definition=fgkHistogramDefinitions3D;
  	 definition->fName!=NULL; definition++) {
      fHistogram3DPointers[definition->fId]=new TH3D(definition->fName,
						     definition->fTitle,
						     definition->fBinsX,
						     definition->fLowerBoundX,
						     definition->fUpperBoundX,
						     definition->fBinsY,
						     definition->fLowerBoundY,
						     definition->fUpperBoundY,
						     definition->fBinsZ,
						     definition->fLowerBoundZ,
						     definition->fUpperBoundZ
						     );
      if (fHistogram3DPointers[definition->fId]) {
	fHistogram3DPointers[definition->fId]->SetOption(definition->fDrawOptions);
      fHistograms3D->AddAt(fHistogram3DPointers[definition->fId], definition->fId);
      }
    }
  }
  
}

const AliHLTTPCDataCompressionMonitorComponent::AliHistogramDefinition AliHLTTPCDataCompressionMonitorComponent::fgkHistogramDefinitions[] = {
  {kHistogramPadrow,        "padrow"   , "TPC clusters (all) padrow; padrow; counts"                  ,  160,   0.,   160., ""},
  {kHistogramHWCFPad,       "hwcfpad"  , "TPC clusters (all) hwcfpad; pad; counts"                    ,  280,   0.,   140., ""},
  {kHistogramPad,           "pad"      , "TPC clusters (all) pad; pad; counts"                        ,  280,   0.,   140., ""},
  {kHistogramTime,          "timebin"  , "TPC clusters (all) timebin; time; counts"                   , 1024,   0.,  1024., ""},
  {kHistogramSigmaY2,       "sigmaY2"  , "TPC clusters (all) sigmaY2; #sigma_{Y}^{2}; counts"         ,  100,   0.,     1., ""},
  {kHistogramSigmaZ2,       "sigmaZ2"  , "TPC clusters (all) sigmaZ2; #sigma_{Z}^{2}; counts"         ,  100,   0.,     1., ""},
  {kHistogramCharge,        "charge"   , "TPC clusters (all) charge; charge; counts"                  , 1024,   0., 65536., ""},
  {kHistogramQMax,          "qmax"     , "TPC clusters (all) qmax; Q_{max}; counts"                   ,  128,   0.,  1024., ""},
  {kHistogramDeltaPadrow,   "d_padrow" , "TPC compression QA delta_padrow; #Delta padrow; counts"         , 1000,  -1.,     1., ""},
  {kHistogramDeltaPad,      "d_pad"    , "TPC compression QA delta_pad; #Delta pad; counts"               , 1000,  -1.,     1., ""},
  {kHistogramDeltaTime,     "d_time"   , "TPC compression QA delta_time; #Delta time; counts"             , 1000,  -1.,     1., ""},
  {kHistogramDeltaSigmaY2,  "d_sigmaY2", "TPC compression QA delta_sigmaY2; #Delta #sigma_{Y}^{2}; counts", 1000,  -1.,     1., ""},
  {kHistogramDeltaSigmaZ2,  "d_sigmaZ2", "TPC compression QA delta_sigmaZ2; #Delta #sigma_{Z}^{2}; counts", 1000,  -1.,     1., ""},
  {kHistogramDeltaCharge,   "d_charge" , "TPC compression QA delta_charge; #Delta charge"                 , 1000,  -1.,     1., ""},
  {kHistogramDeltaQMax,     "d_qmax"   , "TPC compression QA delta_qmax; #Delta Q_{max}"                  , 1000,  -1.,     1., ""},
  {kHistogramOutOfRange,    "ResError" , "TPC Compression Residual Error; padrow; counts"          ,  159,   0.,   159., ""},
  {kNumberOfHistograms, NULL, NULL, 0,0.,0., NULL}
};

const AliHLTTPCDataCompressionMonitorComponent::AliHistogramDefinition2D AliHLTTPCDataCompressionMonitorComponent::fgkHistogramDefinitions2D[] = {
  {kHistogramQMaxSector,    "qmaxsector"   , "TPC cluster charge QMax v.s. TPC sector; sector; Q_{max}"           ,  72,   0.,  72., 1024,   0., 1024., "colz"},
  {kHistogramSigmaY2Sector, "sigmaY2sector", "TPC cluster width sigmaY2 v.s. TPC sector; sector; #sigma_{Y}^{2}" ,  72,   0.,  72.,  100,   0.,    1., "colz"},
  {kHistogramSigmaZ2Sector, "sigmaZ2sector", "TPC cluster width sigmaZ2 v.s. TPC sector; sector; #sigma_{Z}^{2}" ,  72,   0.,  72.,  100,   0.,    1., "colz"},
  {kHistogramXYA,            "XYA", "TPC clusters (all) XY positions - A side; X[cm]; Y[cm]"                      , 100,-300., 300.,  100,-300.,  300., "colz"},
  {kHistogramXYC,            "XYC", "TPC clusters (all) XY positions - C side; X[cm]; Y[cm]"                      , 100,-300., 300.,  100,-300.,  300., "colz"},
  {kNumberOfHistograms2D, NULL, NULL, 0,0.,0., 0,0.,0., NULL}
};

const AliHLTTPCDataCompressionMonitorComponent::AliHistogramDefinition3D AliHLTTPCDataCompressionMonitorComponent::fgkHistogramDefinitions3D[] = {
  {kHistogramPadrowPadSector,"padrowpadsector","padrowpadsector; sector; pad;padrow", 72,0.,72., 140,0.,140., 159,0.,159., ""},
  {kNumberOfHistograms3D, NULL, NULL, 0,0.,0., 0,0.,0., 0,0.,0., NULL}
};

AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::~AliDataContainer()
{
  /// dectructor
  if (fRawData) delete fRawData;
  if (fHistograms) delete fHistograms;
  if (fHistograms2D) delete fHistograms2D;
  if (fHistograms3D) delete fHistograms3D;
}

AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::iterator& AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::BeginPartitionClusterBlock(int /*count*/, AliHLTUInt32_t specification)
{
  /// iterator of remaining clusters block of specification
  AliHLTUInt8_t slice=AliHLTTPCDefinitions::GetMinSliceNr(specification);
  AliHLTUInt8_t partition=AliHLTTPCDefinitions::GetMinPatchNr(specification);
  unsigned index=slice*AliHLTTPCGeometry::GetNumberOfPatches()+partition;
  if (index<fRemainingClusterIds.size())
    fCurrentClusterIds=&fRemainingClusterIds[index];
  else
    fCurrentClusterIds=NULL;
  fBegin=iterator(this);
  return fBegin;
}

AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::iterator& AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::BeginTrackModelClusterBlock(int /*count*/)
{
  /// iterator of track model clusters
  if (fTrackModelClusterIds.fIds && fTrackModelClusterIds.fSize>0)
    fCurrentClusterIds=&fTrackModelClusterIds;
  else
    fCurrentClusterIds=NULL;
  fBegin=iterator(this);
  return fBegin;
}

int AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::AddRawData(const AliHLTComponentBlockData* pDesc)
{
  /// add raw data block
  int iResult=0;
  AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pSpacePointGrid=NULL;
  if (!fRawData && pDesc->fDataType==AliHLTTPCDefinitions::HWClustersDataType()) {
    fRawData=new AliHLTTPCHWCFSpacePointContainer(AliHLTTPCHWCFSpacePointContainer::kModeCreateMap);
    pSpacePointGrid=AliHLTTPCHWCFSpacePointContainer::AllocateIndexGrid();
  } else if (!fRawData && pDesc->fDataType==AliHLTTPCDefinitions::RawClustersDataType()) {
    fRawData=new AliHLTTPCRawSpacePointContainer(AliHLTTPCRawSpacePointContainer::kModeCreateMap);
    pSpacePointGrid=AliHLTTPCRawSpacePointContainer::AllocateIndexGrid();
  }
  {
    if (!fRawData) return -ENOMEM;
    if ((iResult=fRawData->AddInputBlock(pDesc))<0) return iResult;
    if (pSpacePointGrid) {
      fRawData->PopulateAccessGrid(pSpacePointGrid, pDesc->fSpecification);
      fRawData->SetSpacePointPropertyGrid(pDesc->fSpecification, pSpacePointGrid);
    }
    return 0;
  }
  return -ENODATA;  
}

int AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::AddClusterIds(const AliHLTComponentBlockData* pDesc)
{
  /// add cluster id block for remaining or track model clusters
  if (!pDesc) return -EINVAL;
  if (pDesc->fDataType==AliHLTTPCDefinitions::ClusterIdTracksDataType()) {
    fTrackModelClusterIds.fIds=reinterpret_cast<AliHLTUInt32_t*>(pDesc->fPtr);
    fTrackModelClusterIds.fSize=pDesc->fSize/sizeof(AliHLTUInt32_t);
    return 0;
  }
  if (pDesc->fDataType==AliHLTTPCDefinitions::RemainingClusterIdsDataType()) {
    AliHLTUInt8_t slice=AliHLTTPCDefinitions::GetMinSliceNr(pDesc->fSpecification);
    AliHLTUInt8_t partition=AliHLTTPCDefinitions::GetMinPatchNr(pDesc->fSpecification);
    unsigned index=slice*AliHLTTPCGeometry::GetNumberOfPatches()+partition;
    if (fRemainingClusterIds.size()<=index) {
      if ((int)fRemainingClusterIds.size()<AliHLTTPCGeometry::GetNSlice()*AliHLTTPCGeometry::GetNumberOfPatches()) {
	fRemainingClusterIds.resize(AliHLTTPCGeometry::GetNSlice()*AliHLTTPCGeometry::GetNumberOfPatches());
      } else {
	fRemainingClusterIds.resize(index+1);
      }
    }
    fRemainingClusterIds[index].fIds=reinterpret_cast<AliHLTUInt32_t*>(pDesc->fPtr);
    fRemainingClusterIds[index].fSize=pDesc->fSize/sizeof(AliHLTUInt32_t);
    return 0;
  }
  return -ENODATA;
}

AliHLTUInt32_t AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::GetClusterId(int clusterNo) const
{
  /// get the cluster id from the current cluster id block (optional)
  if (!fCurrentClusterIds ||
      clusterNo<0 ||
      (int)fCurrentClusterIds->fSize<=clusterNo)
    return kAliHLTVoidDataSpec;
  return fCurrentClusterIds->fIds[clusterNo];
}

AliHLTUInt32_t AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FindNearestCluster(int slice, int partition, const AliHLTTPCRawCluster& cluster) const
{
  /// get the cluster id of the nearest original cluster
  if (!fRawData) return kAliHLTVoidDataSpec;
  AliHLTUInt32_t key=AliHLTTPCDefinitions::EncodeDataSpecification(slice, slice, partition, partition);
  // FIXME: AliHLTIndexGrid::Index is not declared const
  AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid* pGrid=const_cast<AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid*>(GetClusterSpacePointPropertyGrid(key));
  if (!pGrid) return kAliHLTVoidDataSpec;
  AliHLTUInt32_t clusterId=kAliHLTVoidDataSpec;
  // search a 4x4 matrix out of the 9x9 matrix around the cell addressed by
  // pad and time
  float padrow=(float)cluster.GetPadRow()-AliHLTTPCGeometry::GetFirstRow(partition);
  float pad=cluster.GetPad();
  float time=cluster.GetTime();
  float minr2=-1.;
  const float padpitch=AliHLTTPCGeometry::GetPadPitchWidth(partition);
  const float zwidth=AliHLTTPCGeometry::GetZWidth();
  float maxDeltaPad=AliHLTTPCDefinitions::GetMaxClusterDeltaPad();
  float maxDeltaTime=AliHLTTPCDefinitions::GetMaxClusterDeltaTime();
  int rowindex=pGrid->GetXIndex(padrow);
  int padstartindex=pGrid->GetYIndex(pad);
  int timestartindex=pGrid->GetZIndex(time);
  int cellindex=pGrid->Index(rowindex, padstartindex, timestartindex);
  float centerpad=pGrid->GetCenterY(cellindex);
  float centertime=pGrid->GetCenterZ(cellindex);
  if ((TMath::Abs(centerpad-pad)>maxDeltaPad && pad>0.) ||
      (TMath::Abs(centertime-time)>maxDeltaTime && time>0.)) {
    ALIHLTERRORGUARD(20, "invalid pad center calculation, please check dimensions if dimensions of index grid match the maximum possible deviation");
  }

  int paddirection=1;
  int timedirection=1;
  if (centerpad>pad) paddirection=-1;
  if (centertime>time) timedirection=-1;
  for (int padcount=0, padindex=padstartindex; padcount<2; padcount++, padindex+=paddirection) {
    if (padindex<0) continue;
    if (padindex>=pGrid->GetDimensionY()) break;
    for (int timecount=0, timeindex=timestartindex; timecount<2; timecount++, timeindex+=timedirection) {
      if (timeindex<0) continue;
      if (timeindex>=pGrid->GetDimensionZ()) break;
      cellindex=pGrid->Index(rowindex, padindex, timeindex);
      float cellpad=pGrid->GetCenterY(cellindex);
      float celltime=pGrid->GetCenterZ(cellindex);
      for (AliHLTSpacePointContainer::AliHLTSpacePointPropertyGrid::iterator& cl=pGrid->begin((float)padrow, cellpad, celltime);
	   cl!=pGrid->end(); cl++) {
	if (cl.Data().fTrackId>=0) continue;
	if (GetClusterCharge(cl.Data().fId)!=cluster.GetCharge() ||
	    GetClusterQMax(cl.Data().fId)!=cluster.GetQMax()) continue;
	if (TMath::Abs(padrow-GetClusterX(cl.Data().fId))>=1.) {
	  HLTError("slice %d, partition %d, cluster 0x%08x: mismatch on padrow: %f  vs. cluster %f", slice, partition, cl.Data().fId, padrow, GetClusterX(cl.Data().fId));
	  continue;
	}
	float clusterpad=GetClusterY(cl.Data().fId);
	float clustertime=GetClusterZ(cl.Data().fId);
	clusterpad-=pad;
	clusterpad*=padpitch;
	clustertime-=time;
	clustertime*=zwidth;
	float r2=clusterpad*clusterpad+clustertime*clustertime;
	if (minr2<0. || r2<minr2) {
	  clusterId=cl.Data().fId;
	  cl.Data().fTrackId=1;
	  minr2=r2;
	}
      }
    }
  }
  return clusterId;
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillPadRow(int row, int slice, AliHLTUInt32_t /*clusterId*/)
{
  /// fill padrow histogram
  unsigned index=kHistogramPadrow;
  fCurrentCluster.SetPadRow(row);
  // the inner sectors consist of readout partitions 0 and 1, if the row
  // is smaller than first row of readout partition 2, its an inner sector
  if (row<AliHLTTPCGeometry::GetFirstRow(2)) {
    fSector = slice;
  } else {
    fSector = slice+AliHLTTPCGeometry::GetNSlice();
  }
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(row);
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillPad(float pad, AliHLTUInt32_t /*clusterId*/)
{
  /// fill pad histogram
  fCurrentCluster.SetPad(pad);
  int currentRow=fCurrentCluster.GetPadRow();
  unsigned index=kHistogramPad;
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(pad);

  index=kHistogramPadrowPadSector;
  if (index<fHistogram3DPointers.size() && fHistogram3DPointers[index]!=NULL)
    fHistogram3DPointers[index]->Fill(fSector,pad,currentRow);
  
  AliTPCROC *roc=AliTPCROC::Instance();
  if (roc) {
  Float_t pos[3]={0.,0.,0.};
  roc->GetPositionGlobal(fSector, fSector>35?currentRow-63:currentRow, (int)pad, pos); 
  if (fSector<=17 || (fSector>=36&&fSector<=53))
    //Sectors 0 to 17 and 36 to 53 are on the A side, sectors 18 to 35 and 54 to 71 are on the C side. 
    { 
      index=kHistogramXYA;
      if (index<fHistogram2DPointers.size() && fHistogram2DPointers[index]!=NULL)
	fHistogram2DPointers[index]->Fill(pos[0],pos[1]);
    } else {
    index=kHistogramXYC;
    if (index<fHistogram2DPointers.size() && fHistogram2DPointers[index]!=NULL)
      fHistogram2DPointers[index]->Fill(pos[0],pos[1]);
  }
  }

}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillTime(float time, AliHLTUInt32_t /*clusterId*/)
{
  /// fill pad histogram
  fCurrentCluster.SetTime(time);
  unsigned index=kHistogramTime;
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(time);
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillSigmaY2(float sigmaY2, AliHLTUInt32_t /*clusterId*/, int partition)
{
  /// fill sigmaY2 histogram
  fCurrentCluster.SetSigmaPad2(sigmaY2);
  unsigned index=kHistogramSigmaY2;
  /// take account for different pad widths
  float weight=AliHLTTPCGeometry::GetPadPitchWidth(partition);
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(sigmaY2*weight*weight);

  index=kHistogramSigmaY2Sector;
  if (index<fHistogram2DPointers.size() && fHistogram2DPointers[index]!=NULL)
    fHistogram2DPointers[index]->Fill(fSector,sigmaY2*weight*weight);

}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillSigmaZ2(float sigmaZ2, AliHLTUInt32_t /*clusterId*/)
{
  /// fill sigmaZ2 histogram
  fCurrentCluster.SetSigmaTime2(sigmaZ2);
  unsigned index=kHistogramSigmaZ2;
  // FIXME: this is just a fixed value, to be correct the values from the global
  // parameter block has to be used
  float weight=AliHLTTPCGeometry::GetZWidth();
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(sigmaZ2*weight*weight);

  index=kHistogramSigmaZ2Sector;
  if (index<fHistogram2DPointers.size() && fHistogram2DPointers[index]!=NULL)
    fHistogram2DPointers[index]->Fill(fSector,sigmaZ2*weight*weight);

}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillCharge(unsigned charge, AliHLTUInt32_t /*clusterId*/)
{
  /// fill charge histogram
  fCurrentCluster.SetCharge(charge);
  unsigned index=kHistogramCharge;
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(charge);
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillFlags(unsigned short flags, AliHLTUInt32_t /*clusterId*/)
{
  fCurrentCluster.SetFlags(flags);
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FillQMax(unsigned qmax, AliHLTUInt32_t /*clusterId*/)
{
  /// fill qmax histogram
  fCurrentCluster.SetQMax(qmax);
  unsigned index=kHistogramQMax;
  if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
    fHistogramPointers[index]->Fill(qmax);

  index=kHistogramQMaxSector;
  if (index<fHistogram2DPointers.size() && fHistogram2DPointers[index]!=NULL)
    fHistogram2DPointers[index]->Fill(fSector,qmax);
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::Fill(int slice, int partition, AliHLTUInt32_t clusterId)
{
  /// fill cluster histograms requiring the full cluster information
  
  // TODO: the complete filling of histograms can be moved to this function
  // and the cluster struct be filled in the iterator
  // The delta histograms are filled here either by using the specified
  // cluster, or the nearest cluster on the padrow with identical charge
  // and qmax is searched for comparison.
  if (clusterId==kAliHLTVoidDataSpec) {
    clusterId=FindNearestCluster(slice, partition, fCurrentCluster);
  }
  if (clusterId==kAliHLTVoidDataSpec) return;
  bool bResidualError=false;
  int currentRow=fCurrentCluster.GetPadRow();

  if (fRawData) {
    unsigned index=kHistogramDeltaPadrow;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	fHistogramPointers[index]->Fill(fCurrentCluster.GetPadRow()-GetClusterX(clusterId));
      }
    }

    index=kHistogramDeltaPad;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	float dPad=fCurrentCluster.GetPad()-GetClusterY(clusterId);
	fHistogramPointers[index]->Fill(dPad);
	static const float maxdPad=0.015; // better 100um for 4 and 6mm pad width
	if (TMath::Abs(dPad)>maxdPad) {
	  //HLTError("cluster 0x%08x slice %d partition %d: pad difference %f - max %f", clusterId, slice, partition, dPad, maxdPad);
	  bResidualError=true;
	}
      }
    }

    index=kHistogramDeltaTime;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	float dTime=fCurrentCluster.GetTime()-GetClusterZ(clusterId);
	fHistogramPointers[index]->Fill(dTime);
	static const float maxdTime=0.04; // corresponds to 100um
	if (TMath::Abs(dTime)>maxdTime) {
	  //HLTError("cluster 0x%08x slice %d partition %d: time difference %f - max %f", clusterId, slice, partition, dTime, maxdTime);
	  bResidualError=true;
	}
      }
    }

    index=kHistogramDeltaSigmaY2;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	float factor=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaY2].fScale;
	float sigma=GetClusterYWidth(clusterId)*factor;
	if (sigma>fMaxSigmaY2Scaled) sigma=fMaxSigmaY2Scaled;
	sigma/=factor;
	fHistogramPointers[index]->Fill(fCurrentCluster.GetSigmaPad2()-sigma);
      }
    }

    index=kHistogramDeltaSigmaZ2;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	float factor=AliHLTTPCDefinitions::fgkClusterParameterDefinitions[AliHLTTPCDefinitions::kSigmaZ2].fScale;
	float sigma=GetClusterZWidth(clusterId)*factor;
	if (sigma>fMaxSigmaZ2Scaled) sigma=fMaxSigmaZ2Scaled;
	sigma/=factor;
	fHistogramPointers[index]->Fill(fCurrentCluster.GetSigmaTime2()-sigma);
      }
    }

    index=kHistogramDeltaCharge;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	fHistogramPointers[index]->Fill(fCurrentCluster.GetCharge()-GetClusterCharge(clusterId));
      }
    }

    index=kHistogramDeltaQMax;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      if (CheckClusterID(clusterId)) {
	fHistogramPointers[index]->Fill(fCurrentCluster.GetQMax()-GetClusterQMax(clusterId));
      }
    }

    if (bResidualError) {
    index=kHistogramOutOfRange;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL) {
      fHistogramPointers[index]->Fill(currentRow>=0?currentRow:0);
    }
    }

    index=kHistogramHWCFPad;
    if (index<fHistogramPointers.size() && fHistogramPointers[index]!=NULL)
      fHistogramPointers[index]->Fill(GetClusterY(clusterId));
  }
}

void AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::Clear(Option_t * option)
{
  /// internal cleanup
  if (fRawData) fRawData->Clear(option);
}

TObject* AliHLTTPCDataCompressionMonitorComponent::AliDataContainer::FindObject(const char *name) const
{
  /// get histogram object  
  if (!name) return NULL;
  if ( strcmp(name,"fHistograms")   == 0 )
    return fHistograms;
  if ( strcmp(name,"fHistograms2D") == 0 )
    return fHistograms2D;
  if ( strcmp(name,"fHistograms3D") == 0 )
    return fHistograms3D;

  return NULL;
}
