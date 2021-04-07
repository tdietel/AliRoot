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

/// @file   AliHLTCompStatCollector.cxx
/// @author Matthias Richter
/// @date   
/// @brief  Collector component for the component statistics information.
///

#include "AliHLTCompStatCollector.h"
#include "AliHLTErrorGuard.h"
#include "TFile.h"
#include "TStopwatch.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TH2C.h"
#include "TTree.h"
#include "TFolder.h"
#include "TNamed.h"
#include "TString.h"
#include "TObjString.h"
#include <cassert>
#include <algorithm>
using std::sort;

#define HLTSTAT_FOLDER_NAME               "HLTstat"
#define HLTSTAT_FOLDER_DESC               "ALICE HLT component statistics"
#define HLTSTAT_ENTRY_PARENT_FOLDER_NAME  "parents"
#define HLTSTAT_ENTRY_PARENT_FOLDER_DESC  "parent components"
#define HLTSTAT_ENTRY_PROPS_FOLDER_NAME   "props"
#define HLTSTAT_ENTRY_PROPS_FOLDER_DESC   "component properties"
#define HLTSTAT_ENTRY_PROPS_IDOBJ_NAME    "id"
#define HLTSTAT_ENTRY_PROPS_IDOBJ_DESC    "numerical id calculated from chain id"

/** ROOT macro for the implementation of ROOT specific class methods */
ClassImp(AliHLTCompStatCollector)

AliHLTCompStatCollector::AliHLTCompStatCollector()
  :
  AliHLTProcessor(),
  fpTimer(NULL),
  fpFolder(NULL),
  fInstances(),
  fpStatTree(NULL),
  fCycleTime(0),
  fNofSets(0),
  fArraySize(0),
  fPosition(0),
  fpLevelArray(NULL),
  fpSpecArray(NULL),
  fpBlockNoArray(NULL),
  fpIdArray(NULL),
  fpTimeArray(NULL),
  fpCTimeArray(NULL),
  fpInputBlockCountArray(NULL),
  fpTotalInputSizeArray(NULL),
  fpNormalizedInputSizeArray(NULL),
  fpOutputBlockCountArray(NULL),
  fpTotalOutputSizeArray(NULL)
  , fpInputOutputRatioArray(NULL)
  , fpNormalizedInputOutputRatioArray(NULL)
  , fpComponentCycleTimeArray(NULL)
  , fpEventTypeArray(NULL)
  , fpEventCountArray(NULL)
  , fSizeEstimator(1000)
  , fMode(kPublishObjects)
  , fFileName()
  , fFile(NULL)
  , fLastTime(time(NULL))
  , fPeriod(0)
  , fEventModulo(0)
{
  // see header file for class documentation
  // or
  // refer to README to build package
  // or
  // visit http://web.ift.uib.no/~kjeks/doc/alice-hlt
}

AliHLTCompStatCollector::~AliHLTCompStatCollector()
{
  // see header file for class documentation
  ClearAll();
}

void AliHLTCompStatCollector::GetInputDataTypes( vector<AliHLTComponentDataType>& list)
{
  // see header file for class documentation
  list.push_back(kAliHLTDataTypeComponentStatistics);
}

AliHLTComponentDataType AliHLTCompStatCollector::GetOutputDataType()
{
  // see header file for class documentation
  return kAliHLTMultipleDataType;
}

int AliHLTCompStatCollector::GetOutputDataTypes(AliHLTComponentDataTypeList& tgtList)
{
  // see header file for class documentation
  tgtList.clear();
  tgtList.push_back(kAliHLTDataTypeHistogram);
  tgtList.push_back(kAliHLTDataTypeTTree);
  return tgtList.size();
}

void AliHLTCompStatCollector::GetOutputDataSize( unsigned long& constBase, double& inputMultiplier )
{
  // see header file for class documentation
  constBase=fSizeEstimator;
  inputMultiplier=100.0;
}

int AliHLTCompStatCollector::DoInit( int argc, const char** argv )
{
  // see header file for class documentation
  int iResult=0;
  TString argument="";
  int bMissingParam=0;
  for (int i=0; i<argc && iResult>=0; i++) {
    argument=argv[i];
    if (argument.IsNull()) continue;

    // -file
    if (argument.CompareTo("-file")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      fFileName=argv[i];
      fMode|=kSaveObjects;

    // -modulo
    } else if (argument.CompareTo("-modulo")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString param=argv[i];
      if (param.IsDigit()) {
	fEventModulo=param.Atoi();
      } else {
	HLTError("expecting number as parameter for option %s", argument.Data());
	iResult=-EINVAL;
      }

    // -period
    } else if (argument.CompareTo("-period")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString param=argv[i];
      if (param.IsDigit()) {
	fPeriod=param.Atoi();
      } else {
	HLTError("expecting number as parameter for option %s", argument.Data());
	iResult=-EINVAL;
      }

    // -publish
    } else if (argument.CompareTo("-publish")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString param=argv[i];
      if (param.IsDigit()) {
	if (param.Atoi()==1) fMode|=kPublishObjects;
	else if (param.Atoi()==0) fMode&=~kPublishObjects;
	else {
	  HLTError("expecting 0 or 1 as parameter for option %s", argument.Data());
	  iResult=-EINVAL;
	}
      } else {
	HLTError("expecting number as parameter for option %s", argument.Data());
	iResult=-EINVAL;
      }

    // -arraysize
    } else if (argument.CompareTo("-arraysize")==0) {
      if ((bMissingParam=(++i>=argc))) break;
      TString param=argv[i];
      if (param.IsDigit()) {
	//fArraySize=param.Atoi();
	HLTWarning("argument -arraysize is deprecated, array size adjusted dynamically");
      } else {
	HLTError("expecting number as parameter for option %s", argument.Data());
	iResult=-EINVAL;
      }

    } else {
      HLTError("unknown argument %s", argument.Data());
      iResult=-EINVAL;
    }
  }
  if (bMissingParam) {
    HLTError("missing parameter for argument %s", argument.Data());
    iResult=-EINVAL;
  }

  if (!fFileName.empty()) {
    fFile=new TFile(fFileName.c_str(), "RECREATE");
  }
  return iResult;
}

int AliHLTCompStatCollector::DoDeinit( )
{
  // see header file for class documentation
  ClearAll();

  if (fFile) {
    fFile->Close();
    delete fFile;
    fFile=NULL;
  }
  return 0;
}

int AliHLTCompStatCollector::DoEvent( const AliHLTComponentEventData& /*evtData*/, AliHLTComponentTriggerData& /*trigData*/)
{
  // see header file for class documentation
  int iResult=0;

  AliHLTUInt32_t eventType=gkAliEventTypeUnknown;
  IsDataEvent(&eventType);

  ResetFillingVariables();
  if (fpTimer) {
    fCycleTime=fpTimer->RealTime()*1000000;
  }

  // only if the map of instances is empty it can be initialized from the
  // component table entries
  // the logical check fo eventType==gkAliEventTypeStartOfRun does not work
  // for simulated data where the blocks of the SOR are stored inside the
  // first event
  bool bMapInitialization=fInstances.empty();
  vector<AliHLTCompStatCollector::AliHLTCompStatInstance> sortedInstances;

  bool bEmbeddedTree=false;
  bool bFolderCreated=false;
  if ((bFolderCreated=(fpFolder==NULL))) {
    fpFolder=new TFolder(HLTSTAT_FOLDER_NAME, HLTSTAT_FOLDER_DESC);
    if (bEmbeddedTree) fpFolder->Add(fpStatTree);
  }
  if (!fpFolder) return -ENOMEM;
  vector<TFolder*> newFolders;

  for (const AliHLTComponentBlockData* pBlock=GetFirstInputBlock(kAliHLTDataTypeComponentTable);
       pBlock && iResult>=0;
       pBlock=GetNextInputBlock()) {
    string chainId, compId, compArgs;
    vector<AliHLTUInt32_t> parents;
    int level=0;
    iResult=ExtractComponentTableEntry((const AliHLTUInt8_t*)pBlock->fPtr, pBlock->fSize,
				       chainId, compId, compArgs,
				       parents, level);
    if (iResult>0) {
      HLTDebug("%s(%s) 0x%08x", chainId.c_str(), compId.c_str(), pBlock->fSpecification);
      if (bMapInitialization) {
	map<AliHLTUInt32_t, AliHLTCompStatInstance>::const_iterator element=fInstances.find(pBlock->fSpecification);
	AliHLTCompStatInstance newInstance(pBlock->fSpecification, chainId, compId, compArgs, parents, level);
	if (element==fInstances.end()) {
	  // new instance
	  fInstances[pBlock->fSpecification]=newInstance;
	  sortedInstances.push_back(newInstance);
	} else {
	  // check existing instance
	  if (element->second!=newInstance) {
	    HLTWarning("component table entries have identical CRC ids but different content\n  in list: %s\n  skipping: %s",	       
		       element->second.Description().c_str(), newInstance.Description().c_str());
	  }
	}
      }

      TObject* pObj=NULL;
      TFolder* pEntry=NULL;
      if ((pObj=fpFolder->FindObjectAny(chainId.c_str()))!=NULL &&
	  (pEntry=dynamic_cast<TFolder*>(pObj))!=NULL ) {
	
      } else if (pObj) {
	HLTError("entry %s exists in folder, but is not a sub-folder", chainId.c_str());
      } else if (chainId.size()>0) {
	pEntry=new TFolder(chainId.c_str(), chainId.c_str());
	if (pEntry) {
	  pEntry->SetOwner();
	  TFolder* pProps=pEntry->AddFolder(HLTSTAT_ENTRY_PROPS_FOLDER_NAME, HLTSTAT_ENTRY_PROPS_FOLDER_DESC);
	  if (pProps) {
	    pProps->Add(new TObjString(compId.c_str()));
	    if (!compArgs.empty())
	      pProps->Add(new TObjString(compArgs.c_str()));
	    TNamed* pCRC=new TNamed(HLTSTAT_ENTRY_PROPS_IDOBJ_NAME, HLTSTAT_ENTRY_PROPS_IDOBJ_DESC);
	    if (pCRC) {
	      pCRC->SetUniqueID(pBlock->fSpecification);
	      pProps->Add(pCRC);
	    }
	  }
	  TFolder* pParents=pEntry->AddFolder(HLTSTAT_ENTRY_PARENT_FOLDER_NAME, HLTSTAT_ENTRY_PARENT_FOLDER_DESC);
	  if (pParents) {
	    for (vector<AliHLTUInt32_t>::iterator parent=parents.begin();
		 parent!=parents.end(); parent++) {
	      TString name; name.Form("0x%08x", *parent);
	      pParents->Add(new TObjString(name));
	    }
	  }
	  if (parents.size()==0) {
	    newFolders.push_back(pEntry);
	  } else {
	    vector<TFolder*>::iterator iter=newFolders.begin();
	    vector<AliHLTUInt32_t>::iterator parent=parents.begin();
	    while (iter!=newFolders.end() && parent!=parents.end()) {
	      TObject* idobj=(*iter)->FindObjectAny(HLTSTAT_ENTRY_PROPS_IDOBJ_NAME);
	      AliHLTUInt32_t crcid=0;
	      if (idobj) crcid=idobj->GetUniqueID();
	      HLTDebug("check: %s 0x%08x", (*iter)->GetName(), crcid);
	      if (idobj && crcid==*parent) break;
	      if ((++parent!=parents.end())) continue;
	      parent=parents.begin();
	      iter++;
	    }
	    newFolders.insert(iter,pEntry);
	  }
	}
      } else {
	HLTError("missing chain id for table entry 0x%08x (%p %d), skipping ...", pBlock->fSpecification, pBlock->fPtr, pBlock->fSize);
      }
    } else if (iResult!=0) {
      HLTError("extraction of table entry 0x%08x (%p %d) failed with %d", pBlock->fSpecification, pBlock->fPtr, pBlock->fSize, iResult);
    }
    iResult=0;
  }

  if (bMapInitialization) {
    // assign tags to all instances in the map
    int level=-1;
    int tag=-1;
    TString componentId;
    TObjArray* taglist=NULL;
    sort(sortedInstances.begin(), sortedInstances.end(), AliHLTCompStatInstance::SortByLevelAndComponentId);
    for (vector< AliHLTCompStatInstance>::const_iterator element=sortedInstances.begin();
	 element!=sortedInstances.end();
	 element++) {
      if (level!=element->GetLevel() ||
	  componentId.CompareTo(element->GetComponentId().c_str())!=0) {
	tag++;
	level=element->GetLevel();
	componentId=element->GetComponentId().c_str();
	if (fpFolder) {
	  if (!taglist) {
	    taglist=new TObjArray;
	    taglist->SetName("CompStatMap");
	    fpFolder->Add(taglist);
	  }
	  if (taglist) {
	    TString entry;
	    entry.Form("%02d ", tag); entry+=componentId;
	    taglist->Add(new TObjString(entry));
	  }
	}
      }
      fInstances[element->GetCRCId()].SetTag(tag);
    }

    if (fpStatTree==NULL && !fInstances.empty()) {
      iResult=AllocateStatTree(fInstances.size());
      if (!fpStatTree || iResult<0) {
	ALIHLTERRORGUARD(1, "failed to allocate tree for statistics collection, error %d", iResult);
      }
    }
  }

  if (newFolders.size()>0) {
    vector<TFolder*> revert;
    vector<TFolder*>::iterator iter=newFolders.begin();
    while (iter!=newFolders.end()) {
      revert.insert(revert.begin(), *iter);
      HLTDebug("%s", (*iter)->GetName());
      iter++;
    }
    newFolders.empty();
    newFolders.assign(revert.begin(), revert.end());

    vector<TFolder*>::iterator publisher=newFolders.begin();
    while (publisher!=newFolders.end()) {
      bool bRemove=false;
      HLTDebug("checking %s for parents", (*publisher)->GetName());
      TFolder* propsFolder=dynamic_cast<TFolder*>((*publisher)->FindObject(HLTSTAT_ENTRY_PROPS_FOLDER_NAME));
      assert(propsFolder);
      TObject* idobj=NULL;
      if (propsFolder) idobj=propsFolder->FindObject(HLTSTAT_ENTRY_PROPS_IDOBJ_NAME);
      assert(idobj);
      if (idobj) {
	AliHLTUInt32_t crcid=idobj->GetUniqueID();
	TString idstr; idstr.Form("0x%08x", crcid);
	for (vector<TFolder*>::iterator consumer=publisher+1;
	     consumer!=newFolders.end(); consumer++) {
	  HLTDebug("   checking %s", (*consumer)->GetName());
	  TFolder* parentFolder=dynamic_cast<TFolder*>((*consumer)->FindObject(HLTSTAT_ENTRY_PARENT_FOLDER_NAME));
	  assert(parentFolder);
	  if (parentFolder) {
#ifdef __DEBUG
	    TIter entries(parentFolder->GetListOfFolders());
	    while (TObject* entry=entries.Next())
	      if (entry) {
		HLTDebug("   searching %s in %s: %s", idstr.Data(), (*consumer)->GetName(), entry->GetName());
	      }
#endif
	    TObject* parent=parentFolder->FindObjectAny(idstr);
	    if (parent) {
	      parentFolder->Add(*publisher);
	      parentFolder->Remove(parent);
	      bRemove=true;
	    }
	  }
	}
      }
      if (bRemove) publisher=newFolders.erase(publisher);
      else publisher++;
    }

    for (publisher=newFolders.begin();
	 publisher!=newFolders.end(); publisher++) {
      RemoveRecurrence(*publisher);
      fpFolder->Add(*publisher);
    }
  }

  int blockNo=0;
  for (const AliHLTComponentBlockData* pBlock=GetFirstInputBlock(kAliHLTDataTypeComponentStatistics);
       pBlock && iResult>=0;
       pBlock=GetNextInputBlock(), blockNo++) {
    unsigned int current=fPosition;
    iResult=FillVariablesSorted(pBlock->fPtr, pBlock->fSize, eventType);
    for (; current<fPosition; current++) {
      fpSpecArray[current]=pBlock->fSpecification;
      fpBlockNoArray[current]=blockNo;
    }
    // indicate availability of component statistic block
    iResult=1;
  }

  int totalOutputSize=0;
  if (iResult>0 && eventType) {
    fNofSets=fPosition;
    if (fpStatTree) fpStatTree->Fill();

    // init the timer for the next cycle
    if (!fpTimer)  fpTimer=new TStopwatch;
    if (fpTimer) {
      fpTimer->Reset();
      fpTimer->Start();
    }
  }

  if (eventType==gkAliEventTypeEndOfRun ||
      (iResult>=0 && CheckPeriod())) {

    // publish objects to component output
    if ((fMode&kPublishObjects)!=0) {
      if (!bEmbeddedTree && fpStatTree) {
	iResult=PushBack(fpStatTree, kAliHLTDataTypeTTree|kAliHLTDataOriginOut);
	totalOutputSize+=GetLastObjectSize();
      }
      iResult=PushBack(fpFolder, kAliHLTDataTypeTObject|kAliHLTDataOriginOut);
      totalOutputSize+=GetLastObjectSize();
    }

    // save objects to file
    if ((fMode&kSaveObjects)!=0 && fFile!=NULL) {
      HLTDebug("saving objects to file %s", fFileName.c_str());
      fFile->cd();
      if (!bEmbeddedTree && fpStatTree) {
	fpStatTree->Write("", TObject::kOverwrite);
      }
      fpFolder->Write("", TObject::kOverwrite);
    }
  }

  if (iResult==-ENOSPC) {
    fSizeEstimator+=totalOutputSize;
  }

  if (iResult>0) iResult=0;
  return iResult;
}

void AliHLTCompStatCollector::ResetFillingVariables()
{
  // see header file for class documentation

  for (map<AliHLTUInt32_t, AliHLTCompStatInstance>::iterator element=fInstances.begin();
       element!=fInstances.end();
       element++) {
    element->second.ResetProcessed();
  }

  fCycleTime=0;
  fNofSets=0;
  fPosition=0;
  if (fArraySize==0) return;
  memset(fpLevelArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpSpecArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpBlockNoArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpIdArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpTimeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpCTimeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpInputBlockCountArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpTotalInputSizeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpNormalizedInputSizeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpOutputBlockCountArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpTotalOutputSizeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpInputOutputRatioArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpNormalizedInputOutputRatioArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpComponentCycleTimeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpEventTypeArray, 0, sizeof(UInt_t)*fArraySize);
  memset(fpEventCountArray, 0, sizeof(UInt_t)*fArraySize);
}

int AliHLTCompStatCollector::FillVariablesSorted(void* ptr, int size, AliHLTUInt32_t eventType)
{
  // see header file for class documentation
  int iResult=0;
  if (size%sizeof(AliHLTComponentStatistics)) {
    // older or invalid structure
    HLTError("data block is not aligned to the size of the AliHLTComponentStatistics struct");
    return -EINVAL;
  }
  
  AliHLTComponentStatistics* pStat=reinterpret_cast<AliHLTComponentStatistics*>(ptr);
  UInt_t nofStats=size/sizeof(AliHLTComponentStatistics);
  vector<int> indexList;
  UInt_t i=0;
  for (i=0; i<nofStats; i++) {
    vector<int>::iterator element=indexList.begin();
    for (; element!=indexList.end(); element++) {
      if (pStat[i].fLevel>pStat[*element].fLevel) {
	break;
      }
    }
    indexList.insert(element, i);
  }

  i=fPosition;
  for (vector<int>::iterator element=indexList.begin();
       element!=indexList.end();
       element++) {
    map<AliHLTUInt32_t, AliHLTCompStatInstance>::iterator instance=fInstances.find(pStat[*element].fId);
    if (i<fArraySize && instance!=fInstances.end()) {
      if (instance->second.IsProcessed()) {
	//HLTWarning("already processed instance %s, skip", instance->second.Description().c_str());
	continue;
      }
      instance->second.MarkProcessed();
      if ((int)pStat[*element].fLevel!=instance->second.GetLevel()) {
	// TODO: there is currently a mismatch of the level set in the statistics entry and
	// the one in the component table entries. However this does not matter for
	// archiving because the CRC id is related to a tag/level
	//HLTWarning("level does not match for instance %s, expected %d", instance->second.Description().c_str(), pStat[*element].fLevel);
      }
      fpLevelArray[i]=pStat[*element].fLevel;
      fpIdArray[i]=pStat[*element].fId;
      fpTimeArray[i]=pStat[*element].fTime;
      fpCTimeArray[i]=pStat[*element].fCTime;
      fpInputBlockCountArray[i]=pStat[*element].fInputBlockCount;
      fpTotalInputSizeArray[i]=pStat[*element].fTotalInputSize;
      fpNormalizedInputSizeArray[i]=pStat[*element].fTotalInputSize;
      if (pStat[*element].fInputBlockCount>0) 
	fpNormalizedInputSizeArray[i]/=pStat[*element].fInputBlockCount;
      fpOutputBlockCountArray[i]=pStat[*element].fOutputBlockCount;
      fpTotalOutputSizeArray[i]=pStat[*element].fTotalOutputSize;
      if (pStat[*element].fTotalOutputSize>0)
	fpInputOutputRatioArray[i]=pStat[*element].fTotalInputSize/pStat[*element].fTotalOutputSize;
      if (pStat[*element].fInputBlockCount>0)
	fpNormalizedInputOutputRatioArray[i]=fpInputOutputRatioArray[i]*pStat[*element].fOutputBlockCount/pStat[*element].fInputBlockCount;
      fpComponentCycleTimeArray[i]=pStat[*element].fComponentCycleTime;
      fpEventTypeArray[i]=eventType;
      fpEventCountArray[i]=GetEventCount();
      i++;
    } else if (instance==fInstances.end()) {
      HLTWarning("can not find instance of CRC id 0x%08x", pStat[*element].fId);
    }
  }

  if (i>fArraySize) {
    HLTWarning("too little space in branch variables to fill %d statistics blocks, total available %d, current position %d", i, fArraySize, fPosition);
    fPosition=fArraySize;
  } else {
    fPosition=i;
  }
  
  return iResult;
}

int AliHLTCompStatCollector::AllocateStatTree(AliHLTUInt32_t size)
{
  // allocate the statistics tree and the branch arrays
  if (fArraySize>0) {
    ClearStatTree();
  }
  fArraySize=size;
  if (fArraySize==0) return 0;

  fpLevelArray=new UInt_t[fArraySize];
  fpSpecArray=new UInt_t[fArraySize];
  fpBlockNoArray=new UInt_t[fArraySize];
  fpIdArray=new UInt_t[fArraySize];
  fpTimeArray=new UInt_t[fArraySize];
  fpCTimeArray=new UInt_t[fArraySize];
  fpInputBlockCountArray=new UInt_t[fArraySize];
  fpTotalInputSizeArray=new UInt_t[fArraySize];
  fpNormalizedInputSizeArray=new UInt_t[fArraySize];
  fpOutputBlockCountArray=new UInt_t[fArraySize];
  fpTotalOutputSizeArray=new UInt_t[fArraySize];
  fpInputOutputRatioArray=new UInt_t[fArraySize];
  fpNormalizedInputOutputRatioArray=new UInt_t[fArraySize];
  fpComponentCycleTimeArray=new UInt_t[fArraySize];
  fpEventTypeArray=new UInt_t[fArraySize];
  fpEventCountArray=new UInt_t[fArraySize];

  fpStatTree=new TTree("CompStat", "HLT component statistics");
  if (fpStatTree) {
    fpStatTree->SetDirectory(0);
    fpStatTree->Branch("cycleTime",        &fCycleTime, "cycleTime/F");
    fpStatTree->Branch("nofSets",          &fNofSets, "nofSets/I");
    fpStatTree->Branch("Level",            fpLevelArray, "Level[nofSets]/i");
    fpStatTree->Branch("Specification",    fpSpecArray, "Specification[nofSets]/i");
    fpStatTree->Branch("BlockNo",          fpBlockNoArray, "BlockNo[nofSets]/i");
    fpStatTree->Branch("Id",               fpIdArray, "Id[nofSets]/i");
    fpStatTree->Branch("Time",             fpTimeArray, "Time[nofSets]/i");
    fpStatTree->Branch("CTime",            fpCTimeArray, "CTime[nofSets]/i");
    fpStatTree->Branch("InputBlockCount",  fpInputBlockCountArray, "InputBlockCount[nofSets]/i");
    fpStatTree->Branch("TotalInputSize",   fpTotalInputSizeArray, "TotalInputSize[nofSets]/i");
    fpStatTree->Branch("NormalizedInputSize",   fpNormalizedInputSizeArray, "NormalizedInputSize[nofSets]/i");
    fpStatTree->Branch("OutputBlockCount", fpOutputBlockCountArray, "OutputBlockCount[nofSets]/i");
    fpStatTree->Branch("TotalOutputSize",  fpTotalOutputSizeArray, "TotalOutputSize[nofSets]/i");
    fpStatTree->Branch("InputOutputRatio",  fpInputOutputRatioArray, "InputOutputRatio[nofSets]/i");
    fpStatTree->Branch("NormalizedInputOutputRatio",  fpNormalizedInputOutputRatioArray, "NormalizedInputOutputRatio[nofSets]/i");
    fpStatTree->Branch("ComponentCycleTime",fpComponentCycleTimeArray, "ComponentCycleTime[nofSets]/i");
    fpStatTree->Branch("EventType",fpEventTypeArray, "EventType[nofSets]/i");
    fpStatTree->Branch("EventCount",fpEventCountArray, "EventCount[nofSets]/i");
  }

  return 0;
}

void AliHLTCompStatCollector::ClearAll()
{
  // see header file for class documentation
  if (fpTimer) delete fpTimer; fpTimer=NULL;
  if (fpFolder) delete fpFolder; fpFolder=NULL;
  ClearStatTree();
}

void AliHLTCompStatCollector::ClearStatTree()
{
  // clear the statistics tree and the branch arrays
  if (fpStatTree) delete fpStatTree; fpStatTree=NULL;
  if (fpLevelArray) delete fpLevelArray; fpLevelArray=NULL;
  if (fpSpecArray) delete fpSpecArray; fpSpecArray=NULL;
  if (fpBlockNoArray) delete fpBlockNoArray; fpBlockNoArray=NULL;
  if (fpIdArray) delete fpIdArray; fpIdArray=NULL;
  if (fpTimeArray) delete fpTimeArray; fpTimeArray=NULL;
  if (fpCTimeArray) delete fpCTimeArray; fpCTimeArray=NULL;
  if (fpInputBlockCountArray) delete fpInputBlockCountArray; fpInputBlockCountArray=NULL;
  if (fpTotalInputSizeArray) delete fpTotalInputSizeArray; fpTotalInputSizeArray=NULL;
  if (fpNormalizedInputSizeArray) delete fpNormalizedInputSizeArray; fpNormalizedInputSizeArray=NULL;
  if (fpOutputBlockCountArray) delete fpOutputBlockCountArray; fpOutputBlockCountArray=NULL;
  if (fpTotalOutputSizeArray) delete fpTotalOutputSizeArray; fpTotalOutputSizeArray=NULL;
  if (fpInputOutputRatioArray) delete fpInputOutputRatioArray; fpInputOutputRatioArray=NULL;
  if (fpNormalizedInputOutputRatioArray) delete fpNormalizedInputOutputRatioArray; fpNormalizedInputOutputRatioArray=NULL;
  if (fpComponentCycleTimeArray) delete fpComponentCycleTimeArray; fpComponentCycleTimeArray=NULL;
  if (fpEventTypeArray) delete fpEventTypeArray; fpEventTypeArray=NULL;
  if (fpEventCountArray) delete fpEventCountArray; fpEventCountArray=NULL;
  fArraySize=0;
}

int AliHLTCompStatCollector::RemoveRecurrence(TFolder* pRoot) const
{
  // see header file for class documentation
  int iResult=0;
  if (!pRoot) return -EINVAL;
  TFolder* parentFolder=dynamic_cast<TFolder*>(pRoot->FindObject(HLTSTAT_ENTRY_PARENT_FOLDER_NAME));
  assert(parentFolder);
  vector<TFolder*> listRemove;
  if (parentFolder) {
    TIter entries(parentFolder->GetListOfFolders());
    TFolder* entry=NULL;
    TObject* obj=NULL;
    while ((obj=entries.Next())!=NULL && (entry=dynamic_cast<TFolder*>(obj))!=NULL) {
      TString name=entry->GetName();
      HLTDebug("checking %s for recurrence", name.Data());
      TIter tokens(parentFolder->GetListOfFolders());
      TFolder* token=NULL;
      while ((obj=tokens.Next())!=NULL && (token=dynamic_cast<TFolder*>(obj))!=NULL) {
	if (name.CompareTo(token->GetName())==0) continue;
	if (token->FindObjectAny(name)!=NULL) {
	  listRemove.push_back(entry);
	  HLTDebug("found recurrence in %s", token->GetName());
	  break;
	} else {
	  HLTDebug("no recurrence found in %s", token->GetName());
	}
      }
      RemoveRecurrence(entry);
    }
    for (vector<TFolder*>::iterator removeElement=listRemove.begin();
	 removeElement!=listRemove.end(); removeElement++) {
      parentFolder->Remove(*removeElement);
    }
  }
  
  return iResult;  
}

bool AliHLTCompStatCollector::CheckPeriod(bool bUpdate)
{
  // see header file for class documentation
  bool result=true;
  if (fEventModulo>0) {
    if ((result=((GetEventCount()+1)%fEventModulo)==0)) {
      return true;
    }
  }
  if (fPeriod>0) {
    if ((result=((difftime(time(NULL), fLastTime)>(double)fPeriod))) &&
	bUpdate) {
      fLastTime=time(NULL);
    }
  }
  return result;
}

void AliHLTCompStatCollector::Print(const char* option) const
{
  // print information
  if (strcmp(option, "instances")==0) {
    vector<AliHLTCompStatInstance> sortedInstances;
    for (map<AliHLTUInt32_t, AliHLTCompStatInstance>::const_iterator element=fInstances.begin();
	 element!=fInstances.end();
	 element++) {
      sortedInstances.push_back(element->second);
    }
    sort(sortedInstances.begin(), sortedInstances.end(), AliHLTCompStatInstance::SortByLevelAndComponentId);
    for (vector<AliHLTCompStatInstance>::const_iterator element=sortedInstances.begin();
	 element!=sortedInstances.end();
	 element++) {
      element->Print("");
    }
  }
}

AliHLTCompStatCollector::AliHLTCompStatInstance::AliHLTCompStatInstance()
  : fCRCId(0)
  , fChainId()
  , fComponentId()
  , fComponentParam()
  , fParents()
  , fLevel(-1)
  , fTag(-1)
  , fProcessed(false)
{
  /// default constructor
}

AliHLTCompStatCollector::AliHLTCompStatInstance::AliHLTCompStatInstance(AliHLTUInt32_t CRCId,
									const char* chainId,
									const char* componentId,
									const char* componentParam,
									const vector<AliHLTUInt32_t>& parents,
									int level,
									int tag)
  : fCRCId(CRCId)
  , fChainId(chainId)
  , fComponentId(componentId)
  , fComponentParam(componentParam)
  , fParents(parents)
  , fLevel(level)
  , fTag(tag)
  , fProcessed(false)
{
}

AliHLTCompStatCollector::AliHLTCompStatInstance::AliHLTCompStatInstance(AliHLTUInt32_t CRCId,
									const string& chainId,
									const string& componentId,
									const string& componentParam,
									const vector<AliHLTUInt32_t>& parents,
									int level,
									int tag)
  : fCRCId(CRCId)
  , fChainId(chainId)
  , fComponentId(componentId)
  , fComponentParam(componentParam)
  , fParents(parents)
  , fLevel(level)
  , fTag(tag)
  , fProcessed(false)
{
}

AliHLTCompStatCollector::AliHLTCompStatInstance::AliHLTCompStatInstance(const AliHLTCompStatCollector::AliHLTCompStatInstance& src)
  : fCRCId(src.fCRCId)
  , fChainId(src.fChainId)
  , fComponentId(src.fComponentId)
  , fComponentParam(src.fComponentParam)
  , fParents(src.fParents)
  , fLevel(src.fLevel)
  , fTag(src.fTag)
  , fProcessed(src.fProcessed)
{
  /// copy constructor
}

AliHLTCompStatCollector::AliHLTCompStatInstance& AliHLTCompStatCollector::AliHLTCompStatInstance::operator=(const AliHLTCompStatCollector::AliHLTCompStatInstance& src)
{
  /// assignment operator
  if (&src==this) return *this;

  fCRCId=src.fCRCId;
  fChainId=src.fChainId;
  fComponentId=src.fComponentId;
  fComponentParam=src.fComponentParam;
  fParents.assign(src.fParents.begin(), src.fParents.end());
  fLevel=src.fLevel;
  fTag=src.fTag;

  return *this;
}

AliHLTCompStatCollector::AliHLTCompStatInstance::~AliHLTCompStatInstance()
{
  /// destructor
}

string AliHLTCompStatCollector::AliHLTCompStatInstance::Description(const char* /*option*/) const
{
  // get a description string
  TString description;
  description.Form("0x%08x level %d tag %d: %s (%s) %s",
		   fCRCId, fLevel, fTag, fChainId.c_str(), fComponentId.c_str(), fComponentParam.c_str());
  string ret=description.Data();
  return ret;
}

void AliHLTCompStatCollector::AliHLTCompStatInstance::Print(const char* /*option*/) const
{
  // print info to cout
  cout << Description() << endl;
}

bool AliHLTCompStatCollector::AliHLTCompStatInstance::operator==(const AliHLTCompStatInstance &b) const
{
  // check if two instances are equal
  if (fChainId       !=b.fChainId)          return false;
  if (fComponentId   !=b.fComponentId)      return false;
  if (fComponentParam!=b.fComponentParam)   return false;
  if (fCRCId         !=b.fCRCId)            return false;
  if (fLevel         !=b.fLevel)            return false;
  if (fTag>=0 && fTag!=b.fTag && b.fTag>=0) return false;

  return true;
}

bool AliHLTCompStatCollector::AliHLTCompStatInstance::SortByLevelAndComponentId(const AliHLTCompStatInstance &a,
										const AliHLTCompStatInstance &b)
{
  // helper function for std::sort
  if ( a.fLevel       != b.fLevel )       return ( a.fLevel       < b.fLevel );
  if ( a.fComponentId != b.fComponentId ) return ( a.fComponentId < b.fComponentId );
  if ( a.fChainId     != b.fChainId )     return ( a.fChainId     < b.fChainId );
  return 0;
}
