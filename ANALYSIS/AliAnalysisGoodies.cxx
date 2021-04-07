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
//_________________________________________________________________________
// Various utilities usefull for analysis
//
//*-- Yves Schutz 
//////////////////////////////////////////////////////////////////////////////

#include "AliAnalysisDataContainer.h" 
#include "AliTagAnalysis.h" 
#include "AliEventTagCuts.h" 
#include "AliRunTagCuts.h" 
#include "AliXMLCollection.h" 
#include "AliAnalysisGoodies.h" 
#include "AliAnalysisManager.h" 
#include "AliAODHandler.h"
#include "AliAnalysisTask.h" 
#include "AliLog.h" 

#include <Riostream.h>
#ifdef WITHALIEN
#include <TGridResult.h>
#include <TFileMerger.h>
#include <TFile.h> 
#endif
#include <TChain.h>
#include <TGrid.h>
#include <TROOT.h> 
#include <TSystem.h>
#include <TEntryList.h>

//______________________________________________________________________________
AliAnalysisGoodies::AliAnalysisGoodies() :
  TObject(),
  fTimer(), 
  fESDTreeName("esdTree"), 
  fAmgr(0)
{
    //ctor
    // connects to alien and creates an analysis manager

  fTimer.Reset() ; 
   
  TString token = gSystem->Getenv("GRID_TOKEN") ; 
  
  if ( token == "OK" ) 
    TGrid::Connect("alien://");
  else 
    AliInfo("You are not connected to the GRID") ; 
 // Make the analysis manager
  fAmgr = new AliAnalysisManager("Goodies Manager", "Analysis manager created by AliAnalysisGoodies") ;
}

//______________________________________________________________________________
AliAnalysisGoodies::AliAnalysisGoodies(const AliAnalysisGoodies& ag) :
  TObject(),
  fTimer(), 
  fESDTreeName(""), 
  fAmgr(0x0)  
{
    //copy ctor
    // connects to alien and creates an analysis manager

  fESDTreeName = ag.fESDTreeName ;  
  TString token = gSystem->Getenv("GRID_TOKEN") ; 
  
  if ( token == "OK" ) 
    TGrid::Connect("alien://");
  else 
    AliInfo("You are not connected to the GRID") ; 
   
  // Make the analysis manager
  fAmgr = new AliAnalysisManager("Goodies Manager", "Analysis manager created by AliAnalysisGoodies") ;
}

//______________________________________________________________________________
AliAnalysisGoodies& AliAnalysisGoodies::operator=(const AliAnalysisGoodies& ag)
{
  // Assignment operator
  if(this!=&ag) {
  }

  return *this;
}

//______________________________________________________________________________
void AliAnalysisGoodies::Help() const  
{
    // helper
    // display all available utilities
  AliInfo("Analysis utilities:\n") ; 
  printf("                ***  Alien2Local  : copy files ESD files listed in an xml collection from AliEn catalog to local storage and creates a local xml collection  \n") ; 
  printf("                                        usage: Alien2Local(in, out)\n") ; 
  printf("                                                in: a xml esd collection file name    \n") ;  
  printf("                                                ou: the local directory where to save the esd root files   \n") ;  
  printf("                ***  Make  : makes esd collection from tags  \n") ; 
  printf("                                        usage: Make(tags, esds)\n") ; 
  printf("                                                tags: is either a tag root file or an xml tag collection   \n") ;  
  printf("                                                esds: is an esd collection     \n") ;  
  printf("                ***  Merge  : merges files listed in a xml collection \n") ; 
  printf("                                        usage Merge(collection, outputDile)\n") ; 
  printf("                                               collection: is a xml collection \n") ;  
  printf("                ***  Process : process the events with an Analysis Task \n") ;
  printf("                                        usage: Process(esdFile, tagCuts) \n") ;
  printf("                                                esdFile: can be a root file with the ESD Tree ( ex: esd?AliESDs.root) \n") ;
  printf("                                                        or a root file with the Tag Tree ( ex: tag?Run100.Event0_100.ESD.tag.root) \n") ;
  printf("                                                        or a local or alien xml file with the ESD collection ( ex: esd?esdCollection.xml) \n") ;
  printf("                                                        or a local or alien xml file with the TAG collection ( ex: tag?tagCollection.xml) \n") ;
  printf("                                                        or a TChain of esd TTrees \n") ;
  printf("                                               tagCuts: is the AliEventTagCuts (needed only for tag? cases \n") ;
  printf("                ***  Register: register files already stored in a MSS into the AliEn catalog\n") ;
  printf("                                        usage: Register(lfndir, pfndir, pfnFileName) \n") ; 
  printf("                                                lfndir : AliEn directory ( ex:  /alice/data/2006/LHC06c/PHOS_TestBeam/\n") ;  
  printf("                                                pfndir : MSS directory   ( ex: /castor/cern.ch/alice/testbeam/phos/2006 \n") ;
  printf("                                                file   : text file with a list of the file names to be registered\n ") ; 

}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Alien2Local(const TString collectionNameIn, const TString localDir)
{
  // copy files ESD files listed in an xml collection from AliEn catalog to local storage and creates a local xml collection
  // usage: Alien2Local(in, out)
  //        in: a xml esd collection file name 
  //        ou: the local directory where to save the esd root files          

#ifdef WITHALIEN
  Bool_t rv = kTRUE ; 

  fTimer.Start() ; 

  AliXMLCollection * collectionIn = AliXMLCollection::Open(collectionNameIn) ;
  collectionIn->Reset() ; 

  AliXMLCollection * collectionOu = new AliXMLCollection() ; 
  TString collectionNameOu(collectionIn->GetCollectionName()) ; 
  collectionNameOu.Append("Local") ; 
  collectionOu->SetCollectionName(collectionNameOu) ; 
  collectionOu->WriteHeader() ; 

  const char* ocwd = gSystem->WorkingDirectory();

  Int_t counter = 1 ;  
  while ( collectionIn->Next() ) {
    gSystem->ChangeDirectory(localDir) ; 
    TString fileTURL = collectionIn->GetTURL("") ; 

    TString tempo(fileTURL) ; 
    tempo.Remove(tempo.Last('/'), tempo.Length()) ; 
    TString evtsNumber = tempo(tempo.Last('/')+1, tempo.Length())+"/";
    tempo.Remove(tempo.Last('/'), tempo.Length()) ; 
    TString runNumber = tempo(tempo.Last('/')+1, tempo.Length())+"/" ; 
    TString dir = localDir + runNumber ; 
    dir += evtsNumber ;
    char line[1024] ; 
    sprintf(line, ".! mkdir -p %s", dir.Data()) ; 
    gROOT->ProcessLine(line) ; 
    printf("***************************%s\n", line) ; 
    TEntryList * list = collectionIn->GetEventList("") ; 
    if (!list) 
     list = new TEntryList() ; 
    tempo = fileTURL ; 
    TString filename = tempo(tempo.Last('/')+1, tempo.Length()) ;  
    dir += filename ; 
    AliInfo(Form("Copying %s to %s\n", fileTURL.Data(), dir.Data())) ;  
    collectionOu->WriteBody(counter, collectionIn->GetGUID(""), collectionIn->GetLFN(""), dir, list) ;
    counter++ ;
    TFile::Cp(fileTURL, dir) ;
  }
  collectionOu->Export() ;
  gSystem->ChangeDirectory(ocwd) ; 
  
  fTimer.Stop();
  fTimer.Print();

  return rv ;
#else
  return kFALSE;
#endif
}

//______________________________________________________________________
AliAnalysisDataContainer * AliAnalysisGoodies::ConnectInput(AliAnalysisTask * task, TClass * classin, UShort_t index) 
{
  // connect a task to the input

  if ( ! fAmgr->GetTask(task->GetName()) ) 
    fAmgr->AddTask(task) ;
  else 
    AliFatal(Form("Task %s already exists", task->GetName())) ; 

  AliAnalysisDataContainer * taskInput = 0x0 ; 
  if ( fAmgr->GetInputs() ) 
    taskInput = dynamic_cast<AliAnalysisDataContainer *>(fAmgr->GetInputs()->FindObject(Form("InputContainer_%s_%d", task->GetName(), index))) ; 
  if ( ! taskInput ) {
    taskInput  = fAmgr->CreateContainer(Form("InputContainer_%s_%d", task->GetName(), index), classin, AliAnalysisManager::kInputContainer) ;
    fAmgr->ConnectInput (task, index, taskInput);
  }
  else 
    AliFatal(Form("Input %s already exists", taskInput->GetName())) ; 

  return taskInput ; 
} 

//______________________________________________________________________
void AliAnalysisGoodies::ConnectInput(AliAnalysisTask * task, AliAnalysisDataContainer * taskInput, UShort_t index) 
{
  // connect a task to the input

  if ( ! fAmgr->GetTask(task->GetName()) ) 
    fAmgr->AddTask(task) ;
  else 
    AliFatal(Form("Task %s already exists", task->GetName())) ; 

  fAmgr->ConnectInput (task, index, taskInput);
} 

//______________________________________________________________________
AliAnalysisDataContainer *  AliAnalysisGoodies::ConnectOuput(AliAnalysisTask * task, TClass * classou, UShort_t index, TString opt ) 
{
  // connect a task to the output

  char filename[20] ; 

    if (opt == "AOD" ) {    
      if ( fAmgr->GetOutputEventHandler() == 0x0) {
	AliAODHandler * aodHandler = new AliAODHandler() ; 
	aodHandler->SetOutputFileName(Form("%s_0.root",task->GetName())) ; 
	fAmgr->SetOutputEventHandler(aodHandler) ;
      } 
      sprintf(filename, "default") ; 
    } 
    else 
      sprintf(filename, "%s_%d.root",task->GetName(), index) ; 
    
    AliAnalysisDataContainer * taskOuput = 0x0 ;
    if ( fAmgr->GetOutputs() ) 
      taskOuput = dynamic_cast<AliAnalysisDataContainer *>(fAmgr->GetOutputs()->FindObject(Form("OutputContainer_%s_%d", task->GetName(), index))) ; 
    if ( ! taskOuput )
      taskOuput = fAmgr->CreateContainer(Form("OutputContainer_%s_%d", task->GetName(), index), classou, AliAnalysisManager::kOutputContainer, filename) ;
    fAmgr->ConnectOutput(task, index, taskOuput);
    
    return taskOuput ; 
} 

//______________________________________________________________________
void AliAnalysisGoodies::ConnectOuput(AliAnalysisTask * task, AliAnalysisDataContainer * taskOuput, UShort_t index) 
{
  // connect a task to the output

  fAmgr->ConnectInput (task, index, taskOuput);
} 

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Make(AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts *evtCuts, const char * in, const char * out) const  
{
  // makes esd collection from tags 
  // usage Make(tags, esds)
  //              tags: is either a tag root file or an xml tag collection  
  //              esds: is an esd collection  

  Bool_t rv = kTRUE ; 

  if ( !evtCuts && !runCuts ) {
    AliError("No Tag cuts provided") ; 
    return kFALSE ; 
  }
 
  TString file(in) ; 
  if ( file.Contains(".root") ) 
    rv = MakeEsdCollectionFromTagFile(runCuts, lhcCuts, detCuts, evtCuts, file.Data(), out) ; 
  else  if ( file.Contains(".xml") ) 
    rv = MakeEsdCollectionFromTagCollection(runCuts, lhcCuts, detCuts, evtCuts, file.Data(), out) ;
  else {
    AliError(Form("%s is not a valid file format", in)) ; 
    rv = kFALSE ; 
  }

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::MakeEsdCollectionFromTagFile(AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts *evtCuts, const char * in, const char * out) const 
{
  // Makes an esd collection from a root tag file 
  Bool_t rv = kTRUE ; 
    // Open the file collection 
  printf("*** Create Collection       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  file   = |%s|             \n",in);              	
 
  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  rv = tagAna->AddTagsFile(in);
  if ( ! rv ) 
    return rv ; 
 
  tagAna->CreateXMLCollection(out, runCuts, lhcCuts, detCuts, evtCuts) ;
 
  return rv ; 

}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::MakeEsdCollectionFromTagCollection(AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts *evtCuts, const char * in, const char * out) const 
{
  // Makes an esd collection from a xml tag collection 
  Bool_t rv = kTRUE ; 
   // Open the file collection 
  printf("*** Create Collection       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",in);              	

#ifdef WITHALIEN
  
  TGridCollection* collection = gGrid->OpenCollection(in, 0);
  TGridResult* result = collection->GetGridResult("", 0, 0);
  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  tagAna->ChainGridTags(result);

  tagAna->CreateXMLCollection(out, runCuts, lhcCuts, detCuts, evtCuts) ;

#else
  rv =  kFALSE;
#endif
  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::MakeEsdCollectionFromTagCollection(const char * runCuts, const char *lhcCuts, const char *detCuts, const char * evtCuts, const char * in, const char * out) const 
{
  // Makes an esd collection from a xml tag collection 
  
  Bool_t rv = kTRUE ; 
 
  // Open the file collection 
  printf("*** Create Collection       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",in);              	
  
#ifdef WITHALIEN

  TGridCollection* collection = gGrid->OpenCollection(in, 0);
  TGridResult* result = collection->GetGridResult("", 0, 0);
  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  tagAna->ChainGridTags(result);
  
  tagAna->CreateXMLCollection(out, runCuts, lhcCuts, detCuts, evtCuts) ;

#else
  rv = kFALSE;
#endif
  return rv ;
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Merge(const char * collectionFile, const char * subFile, const char * outFile) 
{
  // merges files listed in a xml collection 
  // usage Merge(collection, outputFile))
  //              collection: is a xml collection  
  
  Bool_t rv = kFALSE ; 

  if ( strstr(collectionFile, ".xml") == 0 ) {
    AliError("Input collection file must be an \".xml\" file\n") ; 
    return kFALSE ; 
  }

  fTimer.Start() ;

  // Open the file collection 
  printf("*** Create Collection       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",collectionFile);              	
  
#ifdef WITHALIEN

  TGridCollection* collection = gGrid->OpenCollection(collectionFile, 0);
  TGridResult* result = collection->GetGridResult("", 0, 0);
  
  Int_t index = 0  ;
  const char * turl ;
  TFileMerger merger ; 
  if (!outFile) {
    TString tempo(collectionFile) ; 
    if ( subFile) 
      tempo.ReplaceAll(".xml", subFile) ; 
    else 
      tempo.ReplaceAll(".xml", "_Merged.root") ; 
    outFile = tempo.Data() ; 
  }
  merger.OutputFile(outFile) ; 

  while ( (turl = result->GetKey(index, "turl")) ) {
    char file[2048] ;
    if ( subFile )
      sprintf(file, "%s#%s", turl, subFile) ; 
    else 
      sprintf(file, "%s", turl) ; 
      
    printf("%s\n", file) ; 
    merger.AddFile(file) ; 
    index++ ;  
  }

  if (index) 
    merger.Merge() ; 
  
  AliInfo(Form("Files merged into %s\n", outFile)) ;
 
  fTimer.Stop();
  fTimer.Print();
  
#else
  rv = kFALSE;
#endif
  return rv ;
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Process(TChain * chain) 
{
  // process events starting from a chain of esd Trees
  Bool_t rv = kFALSE ; 
  fTimer.Start() ;

  rv = ProcessChain(chain) ; 

  fTimer.Stop();
  fTimer.Print();

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Process(const char * inFile) 
{ 
  // process the events with an Analysis Task 
  // usage Process(esdFile)
  //              esdFile: is of the form opt?file_lfn 
  Bool_t rv = kFALSE ; 
  AliRunTagCuts      * runCuts = 0x0 ; 
  AliLHCTagCuts      * lhcCuts = 0x0 ;
  AliDetectorTagCuts * detCuts = 0x0 ; 
  AliEventTagCuts    * evtCuts = 0x0 ;

  rv = Process(inFile, runCuts, lhcCuts, detCuts, evtCuts) ; 

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Process(const char * inFile, AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts * evtCuts ) 
{
  // process the events with an Analysis Task 
  // usage Process(esdFile, runtagCuts, evtTagCuts)
  //              esdFile: is of the form opt?file_lfn 
  
  Bool_t rv = kFALSE ; 

  fTimer.Start() ;

  TString file(inFile) ; 
  if ( file.Contains("esd?") && file.Contains(".root") ) {
    file.ReplaceAll("esd?", "") ; 
    rv = ProcessEsdFile(file.Data()) ; 

  } else if ( file.Contains("esd?") && file.Contains(".xml") ) { 
    file.ReplaceAll("esd?", "") ; 
    rv = ProcessEsdXmlCollection(file.Data()) ; 

  } else if (file.Contains("tag?") && file.Contains(".root") ) {
    file.ReplaceAll("tag?", "") ; 
    rv = ProcessTagFile(file.Data(), runCuts, lhcCuts, detCuts, evtCuts) ; 

  } else if (file.Contains("tag?") && file.Contains(".xml") ) {
    file.ReplaceAll("tag?", "") ; 
    rv = ProcessTagXmlCollection(file.Data(), runCuts, lhcCuts, detCuts, evtCuts) ; 

  } else { 
    AliError(Form("%s is not a valid file format", inFile)) ; 
    rv = kFALSE ;
  }
  
  fTimer.Stop();
  fTimer.Print();

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Process(const char * inFile, const char * runCuts, const char * lhcCuts, const char * detCuts, const char * evtCuts) 
{
  // process the events with an Analysis Task 
  // usage Process(esdFile, runtagCuts, evtTagCuts)
  //              esdFile: is of the form opt?file_lfn 
  
  Bool_t rv = kFALSE ; 

  fTimer.Start() ;

  TString file(inFile) ; 
  if ( file.Contains("esd?") && file.Contains(".root") ) {
    file.ReplaceAll("esd?", "") ; 
    rv = ProcessEsdFile(file.Data()) ; 

  } else if ( file.Contains("esd?") && file.Contains(".xml") ) { 
    file.ReplaceAll("esd?", "") ; 
    rv = ProcessEsdXmlCollection(file.Data()) ; 

  } else if (file.Contains("tag?") && file.Contains(".root") ) {
    file.ReplaceAll("tag?", "") ; 
    rv = ProcessTagFile(file.Data(), runCuts, lhcCuts, detCuts, evtCuts) ; 

  } else if (file.Contains("tag?") && file.Contains(".xml") ) {
    file.ReplaceAll("tag?", "") ; 
    rv = ProcessTagXmlCollection(file.Data(), runCuts, lhcCuts, detCuts, evtCuts) ; 

  } else { 
    AliError(Form("%s is not a valid file format", inFile)) ; 
    rv = kFALSE ;
  }
  
  fTimer.Stop();
  fTimer.Print();

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessChain(TChain * chain) const
{
  // Procees a TChain. 

  Bool_t rv = kTRUE ;

  // start processing 
  if (fAmgr->InitAnalysis()) {
    fAmgr->PrintStatus();
    fAmgr->StartAnalysis("local",chain);
  } else 
    rv = kFALSE ; 
  
  return rv ; 
}
 
//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessEsdFile(const char * esdFile) const   
{
  // process the events in a single ESD file with an Analysis Task 
  // usage ProcessLocalEsdFile(esdFile)
  //              esdFile: is the root file (local or in alien) with the ESD Tree ( ex: AliESDs.root) 
 
  Bool_t rv = kTRUE ;  
  
  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",esdFile);              	

  // Makes the ESD chain 
  printf("*** Getting the Chain       ***\n");
  TChain* analysisChain = new TChain(fESDTreeName) ;
  analysisChain->AddFile(esdFile);
 
  // Process the events
  rv = ProcessChain(analysisChain) ; 

  return rv;
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessTagFile(const char * tagFile, AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts *evtCuts) const   
{
  // process the events in a single Tag file with an Analysis Task 
  // usage ProcessLocalEsdFile(tagFile)
  //              tagFile: is the root file (local or in alien) with the Tag Tree (ex: Run102.Event0_100.ESD.tag.root) 
 
  Bool_t rv = kTRUE ;  
  
  if ( !evtCuts && !runCuts ) {
    AliError("No Tag cuts provided") ; 
    return kFALSE ; 
  }
  
  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",tagFile);              	

  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  rv = tagAna->AddTagsFile(tagFile);
  if ( ! rv ) 
    return rv ; 

  // Query the tag file and make the analysis chain
  TChain * analysisChain = new TChain(fESDTreeName)  ;
  analysisChain = tagAna->QueryTags(runCuts, lhcCuts, detCuts, evtCuts);
  
  // Process the events
  rv = ProcessChain(analysisChain) ; 

  return rv;
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessTagFile(const char * tagFile, const char * runCuts, const char * lhcCuts, const char * detCuts, const char * evtCuts) const   
{
  // process the events in a single Tag file with an Analysis Task 
  // usage ProcessLocalEsdFile(tagFile)
  //              tagFile: is the root file (local or in alien) with the Tag Tree (ex: Run102.Event0_100.ESD.tag.root) 
 
  Bool_t rv = kTRUE ;  
  

  if ( !evtCuts && !runCuts ) {
    AliError("No Tag cuts provided") ; 
    return kFALSE ; 
  }
  
  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",tagFile);              	

  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  rv = tagAna->AddTagsFile(tagFile);
  if ( ! rv ) 
    return rv ; 

  // Query the tag file and make the analysis chain
  TChain * analysisChain = new TChain(fESDTreeName)  ;
  analysisChain = tagAna->QueryTags(runCuts, lhcCuts, detCuts, evtCuts);
  
  // Process the events
 rv = ProcessChain(analysisChain) ; 

  return rv;
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessEsdXmlCollection(const char * xmlFile) const   
{
  // process the events in a xml ESD collection  with an Analysis Task 
  // usage ProcessLocalEsdFile(xmlFile)
  //              xmlFile: is the local xml file with the ESD collection ( ex: esdCollection.xml) 
 
  Bool_t rv = kTRUE ;  
  
  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",xmlFile);              	

#ifdef WITHALIEN
  //AliXMLCollection * collection = AliXMLCollection::Open(xmlFile,0) ;
  TGridCollection* collection = gGrid->OpenCollection(xmlFile, 0);
  if (! collection) {
    AliError(Form("%s not found", xmlFile)) ; 
    return kFALSE ; 
  }

  TGridResult* result = collection->GetGridResult("",0 ,0);
  TList* analysisfilelist = result->GetFileInfoList();
  
  // Makes the ESD chain 
  printf("*** Getting the Chain       ***\n");
  TChain* analysisChain = new TChain(fESDTreeName);
  analysisChain->AddFileInfoList(analysisfilelist);

  // Process the events
  rv = ProcessChain(analysisChain) ; 

#else
  rv = kFALSE;

#endif

  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessTagXmlCollection(const char * xmlFile, AliRunTagCuts *runCuts, AliLHCTagCuts *lhcCuts, AliDetectorTagCuts *detCuts, AliEventTagCuts * evtCuts) const   
{
  // process the events in a xml ESD collection  with an Analysis Task 
  // usage ProcessLocalEsdFile(xmlFile)
  //              xmlFile: is the local xml file with the tag collection ( ex: tagCollection.xml) 
 
  Bool_t rv = kTRUE ;  
  
  if ( !evtCuts && !runCuts ) {
    AliError("No Tag cuts provided") ; 
    return kFALSE ; 
  }

  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|            \n",xmlFile);              	
 
  // check if file is local or alien
  if ( gSystem->AccessPathName(xmlFile) ) 
    TGrid::Connect("alien://"); 

#ifdef WITHALIEN

  TGridCollection* collection = gGrid->OpenCollection(xmlFile, 0);
  if (! collection) {
    AliError(Form("%s not found", xmlFile)) ; 
    return kFALSE ; 
  }

  TGridResult* result = collection->GetGridResult("", 0, 0);
  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  tagAna->ChainGridTags(result);
  
  // Query the tag file and make the analysis chain
  TChain * analysisChain = new TChain(fESDTreeName)  ;
  analysisChain = tagAna->QueryTags(runCuts, lhcCuts, detCuts, evtCuts);

  // Process the events
  rv = ProcessChain(analysisChain) ; 

#else
  rv = kFALSE;
#endif
  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::ProcessTagXmlCollection(const char * xmlFile, const char * runCuts, const char * lhcCuts, const char * detCuts, const char * evtCuts) const   
{
  // process the events in a xml ESD collection  with an Analysis Task 
  // usage ProcessLocalEsdFile(xmlFile)
  //              xmlFile: is the local xml file with the tag collection ( ex: tagCollection.xml) 
 
  Bool_t rv = kTRUE ;  

 if ( !evtCuts && !runCuts ) {
    AliError("No Tag cuts provided") ; 
    return kFALSE ; 
  }

  printf("*** Process       ***\n");
  printf("***  Wk-Dir = |%s|             \n",gSystem->WorkingDirectory());
  printf("***  Coll   = |%s|             \n",xmlFile);              	
 
#ifdef WITHALIEN

  // check if file is local or alien
  if ( gSystem->AccessPathName(xmlFile) ) 
    TGrid::Connect("alien://"); 

  TGridCollection* collection = gGrid->OpenCollection(xmlFile, 0);
  if (! collection) {
    AliError(Form("%s not found", xmlFile)) ; 
    return kFALSE ; 
  }

  TGridResult* result = collection->GetGridResult("", 0, 0);
  AliTagAnalysis * tagAna = new AliTagAnalysis(); 
  tagAna->ChainGridTags(result);
  
  // Query the tag file and make the analysis chain
  TChain * analysisChain = new TChain(fESDTreeName)  ;
  analysisChain = tagAna->QueryTags(runCuts, lhcCuts, detCuts, evtCuts);

  // Process the events
  rv = ProcessChain(analysisChain) ; 

#else
  rv = kFALSE;
#endif
  return rv ; 
}

//______________________________________________________________________
Bool_t AliAnalysisGoodies::Register( const char * lfndir, const char * pfndir, const char * file) 
{
  // register files already stored in a MSS into the AliEn catalog
  // usage: Register(lfndir, pfndir, pfnFileName)
  //         lfndir : AliEn directory ( ex:  /alice/data/2006/LHC06c/PHOS_TestBeam/ ) 
  //         pfndir : MSS directory   ( ex: /castor/cern.ch/alice/testbeam/phos/2006 )
  //         file   : text file with a list of the file names to be registered

  Bool_t rv = kTRUE ;  
  fTimer.Start() ; 

  ifstream in;
  in.open(file);
  if ( in.bad() ) {
    AliError(Form("Cannot open file %s\n", file)) ; 
    return kFALSE ; 
  }

  TGrid::Connect("alien://");

  char fileName[1024] ;

  while (1) {
    in >> fileName ;
    if (!in.good()) 
      break;
    char lfn[1024] ; 
    
    sprintf(lfn, "%s/%s", lfndir, fileName) ; 
    
    char pfn[1024] ; 
    
    sprintf(pfn, "castor://Alice::CERN::Castor2/%s/%s", pfndir, fileName) ;  
        
    printf("Register %s as %s\n", pfn, lfn) ; 
    
    gGrid->Register(lfn, pfn) ;
  }
  
  fTimer.Stop();
  fTimer.Print();
  
  return rv;
}
 
