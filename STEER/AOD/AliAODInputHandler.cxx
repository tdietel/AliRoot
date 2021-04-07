/**************************************************************************
 * Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
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

/* $Id$ */

#include <TSystem.h>
#include <TTree.h>
#include <TList.h>
#include <TNamed.h>
#include <TFile.h>
#include <TH2.h>

#include "AliAODInputHandler.h"
#include "AliAODEvent.h"
#include "AliVCuts.h"
#include "AliMCEvent.h"
#include "AliAODpidUtil.h"
#include "AliAODMCHeader.h"

ClassImp(AliAODInputHandler)

static Option_t *gAODDataType = "AOD";

//______________________________________________________________________________
AliAODInputHandler::AliAODInputHandler() :
    AliInputEventHandler(),
    fEvent(0),
    fMCEvent(0),
    fFriends(0),
    fAODpidUtil(0x0),
    fMergeEvents(kFALSE),
    fMergeTracks(kTRUE), 
    fMergeEMCALClusters(kTRUE), 
    fMergePHOSClusters(kTRUE), 
    fMergeEMCALCells(kTRUE), 
	fMergePHOSCells(kTRUE),	
	fMergeEMCALTrigger(kTRUE), 
	fMergePHOSTrigger(kTRUE),
   fMergeHMPIDrings(kTRUE),
    fFriendsConnected(kFALSE),
    fFileToMerge(0),
    fTreeToMerge(0),
    fAODEventToMerge(0),
    fMergeOffset(0)
{
  /// Default constructor

  fHistStatistics[0] = fHistStatistics[1] = NULL;
}

//______________________________________________________________________________
AliAODInputHandler::AliAODInputHandler(const char* name, const char* title):
  AliInputEventHandler(name, title),
  fEvent(0),
  fMCEvent(new AliMCEvent()),
  fFriends(new TList()),
  fAODpidUtil(0x0),
  fMergeEvents(kFALSE),
  fMergeTracks(kTRUE), 
  fMergeEMCALClusters(kTRUE), 
  fMergePHOSClusters(kTRUE), 
  fMergeEMCALCells(kTRUE), 
  fMergePHOSCells(kTRUE),
  fMergeEMCALTrigger(kTRUE), 
  fMergePHOSTrigger(kTRUE),
  fMergeHMPIDrings(kTRUE),
  fFriendsConnected(kFALSE),
  fFileToMerge(0),
  fTreeToMerge(0),
  fAODEventToMerge(0),
  fMergeOffset(0)
{
    /// Constructor

  fHistStatistics[0] = fHistStatistics[1] = NULL;
}

//______________________________________________________________________________
AliAODInputHandler::~AliAODInputHandler() 
{
/// Destructor

  if (fFriends) fFriends->Delete();
  delete fFriends;
  delete fHistStatistics[0];
  delete fHistStatistics[1];
  delete fAODpidUtil;
}

//______________________________________________________________________________
Bool_t AliAODInputHandler::Init(TTree* tree, Option_t* opt)
{
    /// Initialisation necessary for each new tree

    fTree = tree;
    if (!fTree) return kFALSE;
    GetEntries();

    ConnectFriends();

    SwitchOffBranches();
    SwitchOnBranches();
    
    // Get pointer to AOD event
    if (!fEvent) fEvent = new AliAODEvent();
    
    fEvent->ReadFromTree(fTree);
    
    if (fMixingHandler) fMixingHandler->Init(tree, opt);
    
    return kTRUE;
}

//______________________________________________________________________________
Bool_t AliAODInputHandler::BeginEvent(Long64_t entry)
{
    /// Begin event

    static Int_t prevRunNumber = -1;
    if (prevRunNumber != fEvent->GetRunNumber() && NeedField()) {
      fEvent->InitMagneticField();
      prevRunNumber = fEvent->GetRunNumber();
    } 

    AliAODMCHeader* mcHeader  = (AliAODMCHeader*) fEvent->GetList()->FindObject(AliAODMCHeader::StdBranchName());
    TClonesArray* mcParticles = (TClonesArray*)   (fEvent->FindListObject("mcparticles"));
    
    if (mcParticles && mcHeader) {
      if (!fMCEvent) fMCEvent = new AliMCEvent();
      fMCEvent->SetExternalHeader(mcHeader);
      fMCEvent->SetParticleArray(mcParticles);
      fMCEvent->SetBGEventReused(mcHeader->GetBGEventReused());
    }

    // When merging, get current event number from GetReadEntry(), 
    // entry gives the events in the current file
    if (fTreeToMerge) fTreeToMerge->GetEntry(GetReadEntry() + fMergeOffset);
  
    if (fEventCuts)
      fIsSelectedResult = fEventCuts->GetSelectionMask(fEvent);
    else
      fIsSelectedResult = static_cast<AliVAODHeader*>(fEvent->GetHeader())->GetOfflineTrigger();

    if (fMixingHandler) fMixingHandler->BeginEvent(entry);

    // set transient pointer to event inside tracks
    fEvent->ConnectTracks();

    return kTRUE;
}

//______________________________________________________________________________
Bool_t AliAODInputHandler::Notify(const char* path)
{
  /// Notifaction of directory change

  if (fMixingHandler) fMixingHandler->Notify(path);
  if (!fFriendsConnected) {
      ConnectFriends();
      fEvent->ReadFromTree(fTree, "reconnect");
  }
  fFriendsConnected = kFALSE;
  fUserInfo=fTree->GetTree()->GetUserInfo();
    
  TTree *ttree = fTree->GetTree();
  if (!ttree) ttree = fTree;
  TString statFname(ttree->GetCurrentFile()->GetName());
  AliInfo(Form("Moving to file %s", statFname.Data()));
  Int_t indarchive = statFname.Index("#");
  if (indarchive<0) {
     statFname = gSystem->DirName(statFname);
     statFname += "/";
  } else {
     statFname.Remove(indarchive+1);
  }   
  statFname += "EventStat_temp.root";
  TFile *statFile = 0;
  if (IsCheckStatistics()) statFile = TFile::Open(statFname, "READ");
  if (statFile) {
     TList *list = (TList*)statFile->Get("cstatsout");
     if (list) {
        AliVCuts *physSel = (AliVCuts*)list->At(0);
        if (physSel) {
           TH2F *hAll = dynamic_cast<TH2F*>(physSel->GetStatistics("ALL"));
           TH2F *hBin0 = dynamic_cast<TH2F*>(physSel->GetStatistics("BIN0"));
           if (fHistStatistics[0] && hAll) {
              TList tmplist;
              tmplist.Add(hAll);
              fHistStatistics[0]->Merge(&tmplist);
              tmplist.Clear();
              tmplist.Add(hBin0);
              if (fHistStatistics[1] && hBin0) fHistStatistics[1]->Merge(&tmplist);
           } else {
             if (hAll && hBin0) {
                fHistStatistics[0] = static_cast<TH2F*>(hAll->Clone());
                fHistStatistics[1] = static_cast<TH2F*>(hBin0->Clone());
                fHistStatistics[0]->SetDirectory(0);
                fHistStatistics[1]->SetDirectory(0);
             }   
           }   
        }
        delete list;
     }
     delete statFile;
  }
  return kTRUE;
}

//______________________________________________________________________________
Bool_t AliAODInputHandler::FinishEvent()
{
  /// Finish event

  if (fMixingHandler) fMixingHandler->FinishEvent();
  if (fEvent) fEvent->Reset();
  return kTRUE;
}

//______________________________________________________________________________
void AliAODInputHandler::AddFriend(char* filename)
{
    /// Add a friend tree

    TNamed* obj = new TNamed(filename, filename);
    if (!fFriends) fFriends = new TList();
    fFriends->Add(obj);
}

//______________________________________________________________________________
Option_t *AliAODInputHandler::GetDataType() const
{
/// Returns handled data type.

   return gAODDataType;
}

//______________________________________________________________________________
TObject *AliAODInputHandler::GetStatistics(Option_t *option) const
{
/// Get the statistics histogram(s) from the physics selection object. This
/// should be called during FinishTaskOutput(). Option can be empty (default
/// statistics histogram) or BIN0.

   TString opt(option);
   opt.ToUpper();
   if (opt=="BIN0") return fHistStatistics[1];
   return fHistStatistics[0];
}   

void AliAODInputHandler::ConnectFriends()
{
    /// Connect the friend trees

    if (!fFriends) return;
    if (!fMergeEvents) {
	TIter next(fFriends);
	TNamed* obj;
	TString aodTreeFName,aodFriendTreeFName;
	TTree *ttree = fTree->GetTree();
	if (!ttree) ttree = fTree;
   if(!ttree->GetCurrentFile()){
     AliWarning("Couldn't get current AOD file, not connecting friends");
     return;
   }
	aodTreeFName = ttree->GetCurrentFile()->GetName();
	
	while((obj = (TNamed*)next())) {
	    aodFriendTreeFName = aodTreeFName;
       if (strlen(GetInputFileName())) aodFriendTreeFName.ReplaceAll(GetInputFileName(),obj->GetName());
	    aodFriendTreeFName.ReplaceAll("AliAOD.root",obj->GetName());
	    aodFriendTreeFName.ReplaceAll("AliAODs.root",obj->GetName());
	    ttree->AddFriend("aodTree", aodFriendTreeFName.Data());
	}
    } else {
	// Friends have to be merged
	TNamed* filename = (TNamed*) (fFriends->At(0));
        fFileToMerge = TFile::Open(filename->GetName());
	if (fFileToMerge) {
	    fFileToMerge->GetObject("aodTree", fTreeToMerge);
	    if (!fAODEventToMerge) fAODEventToMerge = new AliAODEvent();
	    fAODEventToMerge->ReadFromTree(fTreeToMerge);
	}
    }
    fFriendsConnected = kTRUE;
}

//______________________________________________________________________________
void AliAODInputHandler::CreatePIDResponse(Bool_t isMC/*=kFALSE*/)
{
  /// create the pid response object if it does not exist yet

  if (fAODpidUtil) return;
  fAODpidUtil=new AliAODpidUtil(isMC);
  
}

