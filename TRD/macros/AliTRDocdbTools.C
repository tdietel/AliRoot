#include <AliCDBEntry.h>
#include <AliCDBManager.h>
#include <AliTRDCalDCS.h>
#include <AliTRDCalDCSv2.h>
#include <AliTRDCalDCSFEE.h>
#include <AliTRDCalDCSFEEv2.h>



Int_t year=2018;

void UseAlienCDB(Int_t runNr);
void UseCvmfsCDB(Int_t year);
void UseLocalCDB(TString path);
void runinfo(int run);


void AliTRDocdbTools()
{
  cout << endl
       << "LOADING AliTRDocdbTools.C..." << endl
       << endl
       << "This is a collection of utilities to interact with the "
       << "TRD part of the OCDB." << endl
       << endl;

  
  UseCvmfsCDB(2018);
  
}

void UseAlienCDB(Int_t runNr)
{
  cout << "I : Accessing grid storage for run number " << runNr << endl;
  cout << "I : Get CDBManager instance." << endl;
  AliCDBManager *man = AliCDBManager::Instance();
  cout << "I : SetDefaultStorage." << endl;
  man->SetDefaultStorageFromRun(runNr);
}

void UseCvmfsCDB(Int_t year)
{
  UseLocalCDB(Form("/cvmfs/alice-ocdb.cern.ch/calibration/data/%4d/OCDB",
		   year));
}

void UseLocalCDB(TString path)
{
  cout << "I : Accessing local CDB storage at " << path << endl;
  AliCDBManager *man = AliCDBManager::Instance();
  man->SetDefaultStorage("local://"+path);
}



void list_runs(Int_t year)
{
  const char* dirpattern = 
    Form("/cvmfs/alice-ocdb.cern.ch/calibration/data/%d/OCDB/TRD/Calib/DCS",
	 year);

  void* dir = gSystem->OpenDirectory(gSystem->ExpandPathName(dirpattern));

  const char* entry;
  while((entry = (char*)gSystem->GetDirEntry(dir))) {
    int firstrun=0, lastrun=0,v=0,s=0;
    sscanf(entry,"Run%d_%d_v%d_s%d.root",&firstrun,&lastrun,&v,&s);
    
    cout << firstrun << "-" << lastrun << endl;

    if (firstrun==0) continue;
    
    if ( lastrun != 999999999 ) {
      // this entry looks like a fix for a broken config -> skip
      continue;
    }
    
    runinfo(firstrun);
  }

}

void runinfo(int run)
{
  cout << "I : Get OCDB Entry." << endl;
  AliCDBEntry *entry = AliCDBManager::Instance()->Get("TRD/Calib/DCS",run);
  if (entry == NULL) {
    cout << endl << "ERROR: Unable to get the AliTRDCalDCS object"
	 << "from the OCDB for run number " << run << "." << endl;
    return;
  }

  entry->Print();
  
}


//// List all runs in a given year. 
//void list_runs(Int_t year) {
//
//  // Run ranges will have to be updated every year, refer to file
//  // OCDBFoldervsRunRange.xml in CVMVS OCDB for reference.
//
//  switch (year) {
//  case 2017: list_runs(267258, 282900); break;
//  case 2018: list_runs(282901, 999999); break;
//
//  default:
//    cout << "Unknown year '" << year
//	 << "', please update AliTRDocdbTools.C" << endl;
//    break;
//  }
//}
//
