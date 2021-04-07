// $Id: AliHLTCaloClusterizerComponent.cxx 36709 2009-11-12 16:57:55Z odjuvsla $

/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "AliHLTCaloClusterizerComponent.h"
#include "AliHLTCaloClusterizer.h"
#include "AliHLTCaloClusterAnalyser.h"
#include "AliHLTCaloRecPointDataStruct.h"
#include "AliHLTCaloRecPointHeaderStruct.h"
#include "AliHLTCaloDigitDataStruct.h"
#include "AliHLTCaloDefinitions.h"
#include "AliHLTCaloClusterDataStruct.h"
#include "AliHLTCaloRecoParamHandler.h"
#include "AliHLTEMCALGeometry.h"
#include "TString.h"

/** @file   AliHLTCaloClusterizerComponent.cxx
    @author Oystein Djuvsland
    @author Rudiger Haake (Yale), adapted to EMCAL/DCAL clusterizer
    @date
    @brief  A clusterizer component for EMCAL/DCAL HLT
 */

ClassImp(AliHLTCaloClusterizerComponent);

AliHLTCaloClusterizerComponent::AliHLTCaloClusterizerComponent(TString det):
            AliHLTCaloProcessor(),
            AliHLTCaloConstantsHandler(det),
            fDataOrigin(0),
            fAnalyserPtr(0),
            fRecoParamsPtr(0),
            fClusterizerPtr(0),
            fGeometry(0),
            fDigitsPointerArray(0),
            fOutputDigitsArray(0),
            fDigitCount(0),
            fCopyDigitsToOuput(kTRUE),
            fUseInputSpec(kFALSE)
{
  //See headerfile for documentation
  fUseInputSpec = det=="EMCAL";


}

AliHLTCaloClusterizerComponent::~AliHLTCaloClusterizerComponent()
{
  //See headerfile for documentation
  if(fAnalyserPtr)
  {
    delete fAnalyserPtr;
    fAnalyserPtr = 0;
  }
}

int
AliHLTCaloClusterizerComponent::DoEvent(const AliHLTComponentEventData& evtData, const AliHLTComponentBlockData* blocks,
    AliHLTComponentTriggerData& /*trigData*/, AliHLTUInt8_t* outputPtr, AliHLTUInt32_t& size,
    std::vector<AliHLTComponentBlockData>& outputBlocks)
{
  //See headerfile for documentation

  if (blocks == 0) return 0;

  UInt_t offset           = 0;
  UInt_t mysize           = 0;
  Int_t nRecPoints        = 0;
  Int_t nDigits           = 0;
  Int_t digCount          = 0;

  UInt_t availableSize = size;
  AliHLTUInt8_t* outBPtr;
  outBPtr = outputPtr;
  const AliHLTComponentBlockData* iter = 0;
  unsigned long ndx;

  UInt_t specification = 0;

  AliHLTCaloDigitDataStruct *digitDataPtr = 0;

  // Adding together all the digits, should be put in standalone method
  for ( ndx = 0; ndx < evtData.fBlockCnt; ndx++ )
  {
    iter = blocks+ndx;

    if (iter->fDataType == (AliHLTCaloDefinitions::fgkDigitDataType|fDataOrigin))
    {

      // Update the number of digits
      nDigits = iter->fSize/sizeof(AliHLTCaloDigitDataStruct);
      availableSize -= iter->fSize;

      if(fUseInputSpec)
        specification = iter->fSpecification;
      else
        specification = specification|iter->fSpecification;

      digitDataPtr = reinterpret_cast<AliHLTCaloDigitDataStruct*>(iter->fPtr);

      // We copy the digits to the digit buffer used by the clusterizer
      // This is convinient since we want the digits from all DDLs before starting
      // Could be changed if this is a bottle neck.
      for (Int_t i = 0; i < nDigits; i++)
      {
        // If we have a digit based on a low gain channel, but there has been no high gain channel,
        // we shouldn't use it since we are then very sensitive to noise (e.g. for PHOS 1 LG ADC count = 40 MeV)
        if(digitDataPtr->fHgPresent)
        {
          fDigitsPointerArray[digCount] = digitDataPtr;
          digCount++;
          digitDataPtr++;
        }
      }
    }
  }

  if (digCount > 0)
  {
    // Do the clusterisation
    nRecPoints = fClusterizerPtr->ClusterizeEvent(digCount);

    HLTDebug("Number of rec points found: %d", nRecPoints);

    // Give the storage pointer to the analyser
    fAnalyserPtr->SetCaloClusterData(reinterpret_cast<AliHLTCaloClusterDataStruct*>(outBPtr));

    // Give the rec points to the analyser (input)
    fAnalyserPtr->SetRecPointArray(fClusterizerPtr->GetRecPoints(), nRecPoints);

    // Give the digits to the analyser
    fAnalyserPtr->SetDigitDataArray(fDigitsPointerArray);

    // Then we create the clusters
    Int_t nClusters = fAnalyserPtr->CreateClusters(nRecPoints, size, mysize);

    if (nClusters < 0)
      HLTError("Error in clusterisation");

    HLTDebug("Number of clusters: %d", nRecPoints);

    AliHLTComponentBlockData bd;
    FillBlockData( bd );
    bd.fOffset = offset;
    bd.fSize = mysize;
    bd.fDataType = kAliHLTDataTypeCaloCluster | fDataOrigin;
    bd.fSpecification = specification;
    outputBlocks.push_back( bd );
  }

  size = mysize;

  return 0;
}

int
AliHLTCaloClusterizerComponent::Reconfigure(const char* cdbEntry, const char* /*chainId*/)
{
  // see header file for class documentation

  const char* path="HLT/ConfigPHOS/ClusterizerComponent";

  if (cdbEntry) path = cdbEntry;

  return ConfigureFromCDBTObjString(path);
}

int
AliHLTCaloClusterizerComponent::ScanConfigurationArgument(int argc, const char **argv)
{
  //See header file for documentation

  if (argc <= 0) return 0;

  for(int i=0;i<argc;i++){
    TString argument=argv[i];

    if (argument.CompareTo("-digitthreshold") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetEmcMinEnergyThreshold(argument.Atof());
    }

    if (argument.CompareTo("-recpointthreshold") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetEmcClusteringThreshold(argument.Atof());
    }

    if (argument.CompareTo("-cutonsinglecell") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fAnalyserPtr->SetCutOnSingleCellClusters(true, argument.Atof());
    }

    if (argument.CompareTo("-emctimegate") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetEmcTimeGate(argument.Atof());
    }

    if (argument.CompareTo("-celltimemin") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetCellTimeMin(argument.Atof());
    }

    if (argument.CompareTo("-celltimemax") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetCellTimeMax(argument.Atof());
    }

    if (argument.CompareTo("-usegradientcut") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetUseGradientCut((Bool_t)argument.Atoi());
    }

    if (argument.CompareTo("-gradientcut") == 0)
    {
      if (++i >= argc) return -EPROTO;
      argument = argv[i];
      fClusterizerPtr->SetGradientCut(argument.Atof());
    }

  }

  return 0;
}

int
AliHLTCaloClusterizerComponent::DoInit(int argc, const char** argv )
{
  fGeometry = new AliHLTEMCALGeometry(GetRunNo());

  //See headerfile for documentation

  if (fCaloConstants->GetDETNAME() == "EMCAL") {
    //std::cout << "Allocating " << 2*fCaloConstants->GetNXCOLUMNSMOD()*fCaloConstants->GetNZROWSMOD() << std::endl;
    fDigitsPointerArray = new AliHLTCaloDigitDataStruct*[20*fCaloConstants->GetNXCOLUMNSMOD()*fCaloConstants->GetNZROWSMOD()];
  }
  else 
    fDigitsPointerArray = new AliHLTCaloDigitDataStruct*[fCaloConstants->GetNXCOLUMNSMOD()*fCaloConstants->GetNZROWSMOD()];

  fClusterizerPtr->SetGeometry(fGeometry);
  fClusterizerPtr->SetDigitArray(fDigitsPointerArray);

  fAnalyserPtr = new AliHLTCaloClusterAnalyser();

  if (fCaloConstants->GetDETNAME() == "PHOS")
  {
    fAnalyserPtr->SetClusterType(kPHOSCluster);
  }
  else if (fCaloConstants->GetDETNAME() == "EMCAL")
  {
    fAnalyserPtr->SetClusterType(kEMCALClusterv1);
  }
  else
  {
    fAnalyserPtr->SetClusterType(kUndef);
  }
  //InitialiseGeometry();
  fAnalyserPtr->SetGeometry(fGeometry);
  if (fRecoParamsPtr)
  {
    if (!fRecoParamsPtr->GetParametersFromCDB())
    {
      fAnalyserPtr->SetRecoParamHandler(fRecoParamsPtr);
      fClusterizerPtr->SetEmcClusteringThreshold(fRecoParamsPtr->GetRecPointThreshold());
      fClusterizerPtr->SetEmcMinEnergyThreshold(fRecoParamsPtr->GetRecPointMemberThreshold());
      HLTInfo("Setting thresholds for clusterizer: %f, %f", fRecoParamsPtr->GetRecPointThreshold(), fRecoParamsPtr->GetRecPointMemberThreshold());
    }
  }
  //

  //  const char *path = "HLT/ConfigPHOS/ClusterizerComponent";

  //  ConfigureFromCDBTObjString(path);

  ScanConfigurationArgument(argc, argv);

  return 0;
}
int AliHLTCaloClusterizerComponent::DoDeinit()
{
  // See header file for documentation
  if (fDigitsPointerArray)
  {
    delete []  fDigitsPointerArray;
    fDigitsPointerArray = 0;
  }
  if (fClusterizerPtr)
  {
    delete fClusterizerPtr;
    fClusterizerPtr = 0;
  }
  if (fAnalyserPtr)
  {
    delete fAnalyserPtr;
    fAnalyserPtr = 0;
  }
  if(fGeometry){
    delete fGeometry;
    fGeometry = 0;
  }
  return 0;
}

Int_t
AliHLTCaloClusterizerComponent::CompareDigits(const void *dig0, const void *dig1)
{
  // See header file for documentation
  return (*((AliHLTCaloDigitDataStruct**)(dig0)))->fID - (*((AliHLTCaloDigitDataStruct**)(dig1)))->fID;

  //return (*((AliHLTCaloDigitDataStruct**)(dig0)))->fID - (*((AliHLTCaloDigitDataStruct**)(dig1)))->fID;
}
