/**************************************************************************
 * Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

/* $Id: AliAnalysisTaskPIDResponse.cxx 43811 2010-09-23 14:13:31Z wiechula $ */
#include <TFile.h>
#include <TChain.h>

#include <AliAnalysisManager.h>
#include <AliInputEventHandler.h>
#include <AliVEvent.h>
#include <AliVTrack.h>
#include <AliLog.h>
#include <AliPIDResponse.h>
#include <AliESDpid.h>
#include <AliProdInfo.h>
#include <TPRegexp.h>
#include <TRandom3.h>

#include "AliAnalysisTaskPIDResponse.h"

ClassImp(AliAnalysisTaskPIDResponse)

/// \file AliAnalysisTaskPIDResponse.cxx

AliAnalysisTaskPIDResponse::AliAnalysisTaskPIDResponse():
AliAnalysisTaskSE(),
fIsMC(kFALSE),
fCachePID(kTRUE),
fOADBPath(),
fSpecialDetResponse(),
fRecoPassName(),
fRecoPassNameTuned(),
fPIDResponse(0x0),
fRun(-1),
fOldRun(-1),
fRecoPass(0),
fIsTunedOnData(kFALSE),
fTunedOnDataMask(0),
fRecoPassTuned(0),
fUseTPCEtaCorrection(kTRUE),
fUseTPCMultiplicityCorrection(kTRUE),
fUseTPCPileupCorrection(kTRUE),
fUseTRDEtaCorrection(kTRUE),
fUseTRDClusterCorrection(kTRUE),
fUseTRDCentralityCorrection(kTRUE),
fUserDataRecoPass(-1),
fRandomSeed(0)
{
  //
  // Dummy constructor
  //
}

//______________________________________________________________________________
AliAnalysisTaskPIDResponse::AliAnalysisTaskPIDResponse(const char* name):
AliAnalysisTaskSE(name),
fIsMC(kFALSE),
fCachePID(kTRUE),
fOADBPath(),
fSpecialDetResponse(),
fRecoPassName(),
fRecoPassNameTuned(),
fPIDResponse(0x0),
fRun(-1),
fOldRun(-1),
fRecoPass(0),
fIsTunedOnData(kFALSE),
fTunedOnDataMask(0),
fRecoPassTuned(0),
fResetTuneOnDataTOF(kFALSE),
fUseTPCEtaCorrection(kTRUE),
fUseTPCMultiplicityCorrection(kTRUE),
fUseTPCPileupCorrection(kTRUE),
fUseTRDEtaCorrection(kTRUE),
fUseTRDClusterCorrection(kTRUE),
fUseTRDCentralityCorrection(kTRUE),
fUserDataRecoPass(-1),
fRandomSeed(0)
{
  /// Default constructor

  DefineInput(0,TChain::Class());
}

//______________________________________________________________________________
AliAnalysisTaskPIDResponse::~AliAnalysisTaskPIDResponse()
{
  /// Destructor

}

//______________________________________________________________________________
void AliAnalysisTaskPIDResponse::UserCreateOutputObjects()
{
  /// Create the output QA objects

  AliLog::SetClassDebugLevel("AliAnalysisTaskPIDResponse",10);

  //input hander
  AliAnalysisManager *man=AliAnalysisManager::GetAnalysisManager();
  AliInputEventHandler *inputHandler=dynamic_cast<AliInputEventHandler*>(man->GetInputEventHandler());
  if (!inputHandler) AliFatal("Input handler needed");

  //pid response object
  inputHandler->CreatePIDResponse(fIsMC);
  fPIDResponse=inputHandler->GetPIDResponse();
  if (!fPIDResponse) AliFatal("PIDResponse object was not created");

  fPIDResponse->SetOADBPath(AliAnalysisManager::GetOADBPath());
  fPIDResponse->SetCachePID(fCachePID);
  if (!fOADBPath.IsNull()) fPIDResponse->SetOADBPath(fOADBPath.Data());

  if(fTunedOnDataMask != 0) fPIDResponse->SetTunedOnDataMask(fTunedOnDataMask);

  if (!fSpecialDetResponse.IsNull()){
    TObjArray *arr=fSpecialDetResponse.Tokenize("; ");
    for (Int_t i=0; i<arr->GetEntriesFast();++i){
      TString resp(arr->At(i)->GetName());
      if (resp.BeginsWith("TPC:")){
        resp.ReplaceAll("TPC:","");
        fPIDResponse->SetCustomTPCpidResponse(resp.Data());
        AliInfo(Form("Setting custom TPC response file: '%s'",resp.Data()));
      }
      else if (resp.BeginsWith("TPC-OADB:")){
        resp.ReplaceAll("TPC-OADB:","");
        fPIDResponse->SetCustomTPCpidResponseOADBFile(resp.Data());
        AliInfo(Form("Setting custom TPC response OADB file: '%s'",resp.Data()));
      }
      else if (resp.BeginsWith("TPC-Maps:")){
        resp.ReplaceAll("TPC-Maps:","");
        fPIDResponse->SetCustomTPCetaMaps(resp.Data());
        AliInfo(Form("Setting custom TPC eta maps file: '%s'",resp.Data()));
      }
      else if (resp.BeginsWith("TPC-dEdxType:")){
        resp.ReplaceAll("TPC-dEdxType:","");
        fPIDResponse->GetTPCResponse().SetdEdxTypeFromString(resp);
      }
      else if (resp.BeginsWith("RecoPassName:")){
        resp.ReplaceAll("RecoPassName:","");
        fRecoPassName=resp;
      }
    }
    delete arr;
  }

  // initialize the random seed needed for tune on data
  if (fRandomSeed != 1 && fIsTunedOnData) {
    AliInfoF("Tune on data was requested. Initializing the random seed with %lu", fRandomSeed);
    gRandom->SetSeed(fRandomSeed);
  }
}

//______________________________________________________________________________
void AliAnalysisTaskPIDResponse::UserExec(Option_t */*option*/)
{
  /// Setup the PID response functions and fill the QA histograms

  AliVEvent *event=InputEvent();
  if (!event) return;
  fRun=event->GetRunNumber();

  if (fRun!=fOldRun){
    SetRecoInfo();
    if(fIsTunedOnData) {
      fPIDResponse->SetTunedOnData(kTRUE,fRecoPassTuned, fRecoPassNameTuned);
      if(fResetTuneOnDataTOF) fPIDResponse->ResetTuneOnDataTOF();
    }

    fOldRun=fRun;

    fPIDResponse->SetUseTPCEtaCorrection(fUseTPCEtaCorrection);
    fPIDResponse->SetUseTPCMultiplicityCorrection(fUseTPCMultiplicityCorrection);
    fPIDResponse->SetUseTPCPileupCorrection(fUseTPCPileupCorrection);

    fPIDResponse->SetUseTRDEtaCorrection(fUseTRDEtaCorrection);
    fPIDResponse->SetUseTRDClusterCorrection(fUseTRDClusterCorrection);
    fPIDResponse->SetUseTRDCentralityCorrection(fUseTRDCentralityCorrection);
  }

  fPIDResponse->InitialiseEvent(event,fRecoPass, fRecoPassName);
    
  fPIDResponse->SetCurrentMCEvent(MCEvent());
  AliESDpid *pidresp = dynamic_cast<AliESDpid*>(fPIDResponse);
  if(pidresp && AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler()){
      pidresp->SetEventHandler(AliAnalysisManager::GetAnalysisManager()->GetMCtruthEventHandler());
  }
  //create and attach transient PID object
//   if (fCachePID) {
//     fPIDResponse->FillTrackDetectorPID();
//   }
}

//______________________________________________________________________________
void AliAnalysisTaskPIDResponse::SetRecoInfo()
{
  /// Set reconstruction information

  //reset information
  fRecoPass=0;

  //Get UserInfo from the current tree
  AliAnalysisManager *mgr=AliAnalysisManager::GetAnalysisManager();
  AliVEventHandler *inputHandler=mgr->GetInputEventHandler();
  if (!inputHandler) return;

  TList *uiList = inputHandler->GetUserInfo();
  AliProdInfo prodInfo(uiList);
  prodInfo.List();

  TTree *tree = inputHandler->GetTree();
  TFile *file = tree->GetCurrentFile();
  if (!file) {
    AliError("Current file not found, cannot set reconstruction information");
    return;
  } else {
    TString fileName(file->GetName());
    fPIDResponse->SetCurrentFile(fileName.Data());
  }

  fPIDResponse->SetCurrentAliRootRev(prodInfo.GetAlirootSvnVersion());

  if (prodInfo.IsMC() == kTRUE) fIsMC=kTRUE;         // protection if user didn't use macro switch
  if ( (prodInfo.IsMC() == kFALSE) && (fIsMC == kFALSE) ) {      // reco pass is needed only for data

    if (fUserDataRecoPass > -1) {
      AliInfo(Form("Data reconstruction pass is user specified. Setting pass #: %d",fUserDataRecoPass));
      fRecoPass = fUserDataRecoPass;
    } else {
      fRecoPass = prodInfo.GetRecoPass();

      // fRecoPassName might have been set as a user requirement in fSpecialDetResponse
      if (fRecoPassName.IsNull()) {
        fRecoPassName = prodInfo.GetRecoPassName();
      }
      if (fRecoPass < 0) {   // as last resort we find pass from file name (UGLY, but not stored in ESDs/AODs before LHC12d )
        TString fileName(file->GetName());
        if (fileName.Contains("pass1") ) {
          fRecoPass=1;
        } else if (fileName.Contains("pass2") ) {
          fRecoPass=2;
        } else if (fileName.Contains("pass3") ) {
          fRecoPass=3;
        } else if (fileName.Contains("pass4") ) {
          fRecoPass=4;
        } else if (fileName.Contains("pass5") ) {
          fRecoPass=5;
        }

        if (fRecoPassName.IsNull()) {
          // try to get the pass name from the file name
          TPRegexp reg(".*/(LHC.*)/000([0-9]+)/([a-zA-Z0-9_-]+)/.*");
          TObjArray *arrPassName=reg.MatchS(file->GetName());
          if (arrPassName->At(3)) fRecoPassName=arrPassName->At(3)->GetName();
          delete arrPassName;
        }
      }
    }
    if (fRecoPass <= 0) {
      AliError(" ******** Failed to find reconstruction pass number *********");
      AliError(" ******** PID information loaded for 'pass 0': parameters unreliable ******");
      AliError("      --> If these are MC data: please set kTRUE first argument of AddTaskPIDResponse");
      AliError("      --> If these are real data: ");
      AliError("          (a) please insert pass number inside the path of your local file  OR");
      AliError("          (b) specify reconstruction pass number when adding PIDResponse task");
      AliError("      !!! Note that this will lead to untrustable PID !!!");
//       AliFatal(" no pass number specified for this data file, abort. Asserting AliFatal");
    }
  } else {
    // we have MC

    if (fIsTunedOnData) {
      if (prodInfo.HasLPMPass() && prodInfo.GetRecoPass()!=fRecoPassTuned) {
        AliWarning ("******* Tuned on data reco pass mismatch *******");
        AliWarning ("        MC with tune on data is requested,");
        AliWarningF("        but the anchored reco pass from the 'LPMRawPass=' (%d)",prodInfo.GetRecoPass());
        AliWarningF("        does not match the reco pass set manually by the user (%d)", fRecoPassTuned);
        AliWarning ("        falling back to the reco pass from the 'LPMRawPass=' tag");
        fRecoPassTuned = prodInfo.GetRecoPass();
      }

      if (!prodInfo.GetAnchorPassName().IsNull()) {
        AliInfoF("The anchored reco pass name will be used from AliProdInfo (%s)",prodInfo.GetAnchorPassName().Data());
        fRecoPassTuned = prodInfo.GetRecoPass();
        fRecoPassNameTuned = prodInfo.GetAnchorPassName();
      }
    }
    else {
      AliWarning("You are running on MC but didn't request tune on data");
    }
  }

}
