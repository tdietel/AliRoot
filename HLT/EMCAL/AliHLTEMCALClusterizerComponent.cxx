// $Id$

/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Authors: Oystein Djuvsland <oysteind@ift.uib.no>                       *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * `documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include <iostream>

#include "AliHLTEMCALClusterizerComponent.h"
#include "AliHLTCaloRecPointDataStruct.h"
#include "AliHLTCaloClusterDataStruct.h"
#include "AliHLTCaloDigitDataStruct.h"

#include "AliHLTCaloRecPointHeaderStruct.h"
#include "AliHLTEMCALGeometry.h"
#include "AliHLTEMCALRecoParamHandler.h"
#include "AliHLTCaloClusterAnalyser.h"




/** @file   AliHLTEMCALClusterizerComponent.cxx
    @author Oystein Djuvsland
    @date   
    @brief  A clusterizer component for EMCAL HLT
 */

// see header file for class documentation
// or
// refer to README to build package
// or
// visit http://web.ift.uib.no/~kjeks/doc/alice-hlt

#include "AliHLTCaloDefinitions.h"
#include "AliHLTEMCALDefinitions.h"
#include "AliHLTCaloClusterizer.h"

AliHLTEMCALClusterizerComponent::AliHLTEMCALClusterizerComponent(): 
AliHLTCaloClusterizerComponent("EMCAL")
{
  //See headerfile for documentation

  fDataOrigin = const_cast<char*>(kAliHLTDataOriginEMCAL);

  //AliHLTEMCALGeometry *geom = new AliHLTEMCALGeometry;


}

AliHLTEMCALClusterizerComponent::~AliHLTEMCALClusterizerComponent()
{
  //See headerfile for documentation
}

void
AliHLTEMCALClusterizerComponent::GetInputDataTypes( vector<AliHLTComponentDataType>& list)
{
  //See headerfile for documentation
  list.clear();
  //list.push_back(AliHLTCaloDefinitions::fgkDigitDataType|kAliHLTDataOriginEMCAL);
  list.push_back(AliHLTEMCALDefinitions::fgkDigitDataType);
}

AliHLTComponentDataType
AliHLTEMCALClusterizerComponent::GetOutputDataType()
{
  //See headerfile for documentation
  return kAliHLTDataTypeCaloCluster|kAliHLTDataOriginEMCAL;
}

void
AliHLTEMCALClusterizerComponent::GetOutputDataSize(unsigned long& constBase, double& inputMultiplier )
{
  //See headerfile for documentation
  constBase = 0;
  inputMultiplier = (double)sizeof(AliHLTCaloClusterDataStruct)/(double)sizeof(AliHLTCaloDigitDataStruct);
}


const Char_t*
AliHLTEMCALClusterizerComponent::GetComponentID()
{
  //See headerfile for documentation
  return "EmcalClusterizer";
}

AliHLTComponent*
AliHLTEMCALClusterizerComponent::Spawn()
{
  //See headerfile for documentation

  return new AliHLTEMCALClusterizerComponent();
}
int AliHLTEMCALClusterizerComponent::DoInit(int argc, const char** argv)
{
  fClusterizerPtr = new AliHLTCaloClusterizer("EMCAL");

  fRecoParamsPtr = new AliHLTEMCALRecoParamHandler();

  return AliHLTCaloClusterizerComponent::DoInit(argc, argv);
}

int AliHLTEMCALClusterizerComponent::DoDeinit()
{
  if(fRecoParamsPtr)
  {
    delete fRecoParamsPtr;
    fRecoParamsPtr = 0;
  }
  return AliHLTCaloClusterizerComponent::DoDeinit();
}


int AliHLTEMCALClusterizerComponent::InitialiseGeometry()
{
  fAnalyserPtr->SetGeometry(new AliHLTEMCALGeometry(GetRunNo()));

  return 0;
}
