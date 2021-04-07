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

/** @file   AliHLTGlobalAgent.cxx
    @author Matthias Richter
    @date   
    @brief  Agent of the libAliHLTGlobal library
*/

#include <cassert>
#include "AliHLTGlobalAgent.h"
#include "AliHLTConfigurationHandler.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "AliHLTGlobalFlatEsdTestComponent.h"

// header files of library components
#include "AliHLTGlobalTrackMergerComponent.h"
#include "AliHLTGlobalCompareFlatComponent.h"
#include "AliHLTGlobalEsdToFlatConverterComponent.h"
#include "AliHLTGlobalFlatEsdConverterComponent.h"
#include "AliHLTGlobalEsdConverterComponent.h"
#include "AliHLTGlobalVertexerComponent.h"
#include "AliHLTGlobalOfflineVertexerComponent.h"
#include "AliHLTV0HistoComponent.h"
#include "AliHLTdNdPtAnalysisComponent.h"
#include "AliHLTdNdPtAnalysisMergerComponent.h"
#include "AliHLTGlobalVertexerHistoComponent.h"
#include "AliHLTGlobalHistoComponent.h"
#include "AliHLTGlobalHistoCollector.h"
#include "AliHLTGlobalDCSPublisherComponent.h"
#include "AliHLTMultiplicityCorrelationsComponent.h"
#include "AliHLTPrimaryVertexFinderComponent.h"
#include "AliHLTV0FinderComponent.h"
#include "AliHLTAnalysisManagerComponent.h"
#include "AliHLTAsyncTestComponent.h"
#include "AliHLTLumiRegComponent.h"
#include "AliHLTRootObjectMergerComponent.h"
#include "AliHLTAsyncCalibrationComponent.h"
#include "AliHLTZeroComponent.h"
#ifdef HAVE_ALIGPU
#include "AliHLTGlobalPromptRecoQAComponent.h"
#endif
#ifdef ZMQ
#include "AliHLTZMQsink.h"
#include "AliHLTZMQsource.h"
#endif

// header file for preprocessor plugin
#include "AliHLTGlobalPreprocessor.h"

#define NOCOMPRESS

/** global instance for agent registration */
AliHLTGlobalAgent gAliHLTGlobalAgent;

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTGlobalAgent)

AliHLTGlobalAgent::AliHLTGlobalAgent()
  :
  AliHLTModuleAgent("Global")
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

AliHLTGlobalAgent::~AliHLTGlobalAgent()
{
  // see header file for class documentation
}

int AliHLTGlobalAgent::RegisterComponents(AliHLTComponentHandler* pHandler) const
{
  // see header file for class documentation
  assert(pHandler);
  if (!pHandler) return -EINVAL;
  pHandler->AddComponent(new AliHLTGlobalEsdToFlatConverterComponent);
  pHandler->AddComponent(new AliHLTGlobalCompareFlatComponent);
  pHandler->AddComponent(new AliHLTGlobalFlatEsdTestComponent);
  pHandler->AddComponent(new AliHLTGlobalTrackMergerComponent);
  pHandler->AddComponent(new AliHLTGlobalFlatEsdConverterComponent);
  pHandler->AddComponent(new AliHLTGlobalEsdConverterComponent);
  pHandler->AddComponent(new AliHLTGlobalVertexerComponent);
  pHandler->AddComponent(new AliHLTGlobalOfflineVertexerComponent);
  pHandler->AddComponent(new AliHLTGlobalVertexerHistoComponent);
  pHandler->AddComponent(new AliHLTGlobalHistoComponent);
  pHandler->AddComponent(new AliHLTV0HistoComponent );
  //pHandler->AddComponent(new AliHLTdNdPtAnalysisComponent );
  //pHandler->AddComponent(new AliHLTdNdPtAnalysisMergerComponent );
  pHandler->AddComponent(new AliHLTMultiplicityCorrelationsComponent );
  pHandler->AddComponent(new AliHLTPrimaryVertexFinderComponent);
  pHandler->AddComponent(new AliHLTV0FinderComponent);
  pHandler->AddComponent(new AliHLTGlobalHistoCollector );
  pHandler->AddComponent(new AliHLTGlobalDCSPublisherComponent );
  pHandler->AddComponent(new AliHLTAnalysisManagerComponent);
  pHandler->AddComponent(new AliHLTAsyncTestComponent);
  pHandler->AddComponent(new AliHLTAsyncCalibrationComponent);
  pHandler->AddComponent(new AliHLTZeroComponent);
  pHandler->AddComponent(new AliHLTLumiRegComponent);
  pHandler->AddComponent(new AliHLTRootObjectMergerComponent);
#ifdef HAVE_ALIGPU
  pHandler->AddComponent(new AliHLTGlobalPromptRecoQAComponent);
#endif
#ifdef ZMQ
  pHandler->AddComponent(new AliHLTZMQsink);
  pHandler->AddComponent(new AliHLTZMQsource);
#endif
  return 0;
}

int AliHLTGlobalAgent::CreateConfigurations(AliHLTConfigurationHandler* pHandler,
					    AliRawReader* /*rawReader*/,
					    AliRunLoader* /*runloader*/) const
{
  // see header file for class documentation
  if (!pHandler) return -EINVAL;
  TObjArray* pTokens=NULL;

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // global vertexer component
  //
  // define the inputs 
  TString vertexerInputs="TPC-globalmerger ITS-tracker";

  // check for the availibility
  pTokens=vertexerInputs.Tokenize(" ");
  vertexerInputs="";
  if (pTokens) {
    for (int n=0; n<pTokens->GetEntriesFast(); n++) {
      TString module=((TObjString*)pTokens->At(n))->GetString();
      if (pHandler->FindConfiguration(module.Data())) {
	vertexerInputs+=module;
	vertexerInputs+=" ";
      }
    }
    delete pTokens;
    pTokens=NULL;
  }
  if (!vertexerInputs.IsNull()) {
    HLTInfo("\n\nConfiguring inputs to global HLT Vertexer: %s\n\n", vertexerInputs.Data());
    pHandler->CreateConfiguration("GLOBAL-vertexer","GlobalVertexer",vertexerInputs,"");
  } else {
    HLTWarning("\n\nNo inputs to global HLT Vertexer found\n\n");
  }

  /////////////////////////////////////////////////////////////////////////////////////
  //
  // assembly of the global ESD

  // define the inputs to the global ESD
  //TString esdInputs="TPC-globalmerger TPC-mcTrackMarker ITS-tracker TPC-HWCFDecoder TPC-ClusterTransformation GLOBAL-vertexer ITS-SPD-vertexer TPC-FastdEdx VZERO-RECO ITS-SAPtracker TRD-tracker"; // TRD-tracker removed temporarily while it is still developed, DISABLE GLOBAL VERTEXER TEMPORARILY
  TString esdInputs="TPC-globalmerger TPC-mcTrackMarker ITS-tracker TPC-HWCFDecoder TPC-ClusterTransformation ITS-SPD-vertexer TPC-FastdEdx VZERO-RECO ITS-SAPtracker";

  // check for the availibility
  pTokens=esdInputs.Tokenize(" ");
  esdInputs="";
  if (pTokens) {
    for (int n=0; n<pTokens->GetEntriesFast(); n++) {
      TString module=((TObjString*)pTokens->At(n))->GetString();
      if (pHandler->FindConfiguration(module.Data())) {
	esdInputs+=module;
	esdInputs+=" ";
      }
    }
    delete pTokens;
    pTokens=NULL;
  }

  if (esdInputs.Length()>0) {
    esdInputs+=" TPC-HWCFDecoder TPC-ClusterTransformation";
    HLTInfo("Configuring inputs to global HLT ESD: %s", esdInputs.Data());
  } else {
    HLTWarning("No inputs to global HLT ESD found");
  }
#ifdef NOCOMPRESS  
  pHandler->CreateConfiguration("GLOBAL-flat-esd-converter", "GlobalFlatEsdConverter", esdInputs.Data(), "-object-compression=0");
  pHandler->CreateConfiguration("GLOBAL-esd-converter", "GlobalEsdConverter", esdInputs.Data(), "-object-compression=0");
  pHandler->CreateConfiguration("GLOBAL-esd-converter-friends", "GlobalEsdConverter", esdInputs.Data(), "-object-compression=0 -make-friends");
  pHandler->CreateConfiguration("GLOBAL-esd-converter-schema", "GlobalEsdConverter", esdInputs.Data(), "-object-compression=0 -schema");
#endif
	
	
#ifndef NOCOMPRESS  
  pHandler->CreateConfiguration("GLOBAL-flat-esd-converter", "GlobalFlatEsdConverter", esdInputs.Data(), "");
  pHandler->CreateConfiguration("GLOBAL-esd-converter", "GlobalEsdConverter", esdInputs.Data(), "");
  pHandler->CreateConfiguration("GLOBAL-esd-converter-friends", "GlobalEsdConverter", esdInputs.Data(), "-make-friends");
#endif
  pHandler->CreateConfiguration("GLOBAL-flat-esd-test", "GlobalFlatEsdTest", "GLOBAL-esd-converter GLOBAL-flat-esd-converter", "");
  pHandler->CreateConfiguration("esd-to-flat-conversion", "GlobalEsdToFlatConverter", "GLOBAL-esd-converter-friends", "");
  pHandler->CreateConfiguration("compare-flat", "GlobalCompareFlat", "GLOBAL-flat-esd-converter esd-to-flat-conversion", "");

	

  ///////////////////////////////////////////////////////////////////////////////////////////////////
  //
  // global histograms
  //
  TString vertexhistoInput="GLOBAL-vertexer ITS-SPD-vertexer";

  // check for the availibility
  pTokens=vertexhistoInput.Tokenize(" ");
  vertexhistoInput="";
  if (pTokens) {
    for (int n=0; n<pTokens->GetEntriesFast(); n++) {
      TString module=((TObjString*)pTokens->At(n))->GetString();
      if (pHandler->FindConfiguration(module.Data())) {
	vertexhistoInput+=module;
	vertexhistoInput+=" ";
      }
    }
    delete pTokens;
    pTokens=NULL;
  }

  if (!vertexhistoInput.IsNull()) {
    HLTInfo("Configuring inputs to global HLT Vertex histogram component: %s", vertexhistoInput.Data());
    pHandler->CreateConfiguration("GLOBAL-vertexhisto","GlobalVertexerHisto", vertexhistoInput.Data(),"");
  } else {
    HLTWarning("No inputs to global HLT Vertex histogram component found");
  }
  
  //pHandler->CreateConfiguration("GLOBAL-dNdPtAnalysis", "dNdPtAnalysis", "GLOBAL-esd-converter", "");
 
  pHandler->CreateConfiguration("GLOBAL-MultCorr", "MultiplicityCorrelations", "GLOBAL-esd-converter", "");

  return 0;
}

const char* AliHLTGlobalAgent::GetReconstructionChains(AliRawReader* /*rawReader*/,
						    AliRunLoader* runloader) const
{
  // see header file for class documentation
  if (runloader) {
    // reconstruction chains for AliRoot simulation
    // Note: run loader is only available while running embedded into
    // AliRoot simulation
    return "GLOBAL-esd-converter";    
  }
  return NULL;
}

const char* AliHLTGlobalAgent::GetRequiredComponentLibraries() const
{
  // see header file for class documentation

  return "";
}

int AliHLTGlobalAgent::GetHandlerDescription(AliHLTComponentDataType /*dt*/,
					     AliHLTUInt32_t /*spec*/,
					     AliHLTOUTHandlerDesc& /*desc*/) const
{
  // see header file for class documentation

  return 0;
}

AliHLTOUTHandler* AliHLTGlobalAgent::GetOutputHandler(AliHLTComponentDataType /*dt*/,
						      AliHLTUInt32_t /*spec*/)
{
  // see header file for class documentation

  return NULL;
}

int AliHLTGlobalAgent::DeleteOutputHandler(AliHLTOUTHandler* pInstance)
{
  // see header file for class documentation
  if (pInstance==NULL) return -EINVAL;

  return 0;
}

AliHLTModulePreprocessor* AliHLTGlobalAgent::GetPreprocessor()
{
  // create the preprocessor plugin
  return new AliHLTGlobalPreprocessor;
}
