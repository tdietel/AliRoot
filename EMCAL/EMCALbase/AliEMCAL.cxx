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

// --- ROOT system ---
class TFile;
#include <TFolder.h> 
#include <TGeoGlobalMagField.h>
#include <TGraph.h> 
#include <TH1F.h> 
#include <TRandom.h> 
#include <TTree.h>
#include <TVirtualMC.h> 

// --- AliRoot header files ---
#include "AliMagF.h"
#include "AliLog.h"
#include "AliEMCAL.h"
#include "AliRun.h"
#include "AliRunLoader.h"
#include "AliCDBManager.h"
#include "AliEMCALLoader.h"
#include "AliEMCALSDigitizer.h"
#include "AliEMCALDigitizer.h"
#include "AliEMCALDigit.h"
#include "AliEMCALRawUtils.h"
#include "AliCDBManager.h"
#include "AliCDBEntry.h"
#include "AliEMCALRawUtils.h"
#include "AliRawReader.h"
#include "AliEMCALTriggerData.h"
#include "AliEMCALRecParam.h"
#include "AliRawEventHeaderBase.h"

/// \cond CLASSIMP
ClassImp(AliEMCAL) ;
/// \endcond

// Needed for embedding
AliEMCALRawUtils*        AliEMCAL::fgRawUtils   = 0;   ///< EMCAL raw utilities class

///
/// Default Constructor
//____________________________________________________________________________
AliEMCAL::AliEMCAL()
  : AliDetector(),
    fBirkC0(0),
    fBirkC1(0.),
    fBirkC2(0.),
    fGeometry(0), 
    fCheckRunNumberAndGeoVersion(kTRUE),
    fTriggerData(0x0)
{
  fName = "EMCAL" ;
  InitConstants();
  
  // Should call  AliEMCALGeometry::GetInstance(EMCAL->GetTitle(),"") for getting EMCAL geometry
}

///
/// Constructor : title is used to identify the layout
/// 
/// \param name: detector name "EMCAL"
/// \param title: geometry name, see AliEMCALGeometry for options, see AliEMCAL::GetGeometry()
/// \param checkGeoAndRun: Request automatic setting of geometry depending on run number, see AliEMCAL::GetGeometry()
//____________________________________________________________________________
AliEMCAL::AliEMCAL(const char* name, const char* title, 
                   const Bool_t checkGeoAndRun)
  : AliDetector(name,title),
    fBirkC0(0),
    fBirkC1(0.),
    fBirkC2(0.),
    fGeometry(0), 
    fCheckRunNumberAndGeoVersion(checkGeoAndRun),
    fTriggerData(0x0)
{
  InitConstants();  
}

///
/// Destructor
//____________________________________________________________________________
AliEMCAL::~AliEMCAL()
{
  delete fgRawUtils;
  delete fTriggerData;   
  //fTriggerData->Delete(); // RS: if TClonesArray is created, Delete will be called from ~TClonesArray
}

///
/// Initialize EMCAL values, only Birk's law constants
/// but they are really initialized in AliEMCAL::CreateMaterials()
//____________________________________________________________________________
void AliEMCAL::InitConstants()
{
  fBirkC0 = 1;
  fBirkC1 = 0.013/1.032;
  fBirkC2 = 9.6e-6/(1.032 * 1.032);
}

//Not needed, modify $ALICE_ROOT/data/galice.cuts instead.
//Load the modified one in the configuration file with SetTransPar
// //____________________________________________________________________________
// void AliEMCAL::DefineMediumParameters()
// {
//   //
//   // EMCAL cuts (Geant3) 
//   // 
//   Int_t * idtmed = fIdtmed->GetArray() - 1599 ; 
// // --- Set decent energy thresholds for gamma and electron tracking

//   // Tracking threshold for photons and electrons in Lead 
//   Float_t cutgam=10.e-5; // 100 kev;
//   Float_t cutele=10.e-5; // 100 kev;
//   TString ntmp(GetTitle()); 
//   ntmp.ToUpper();
//   if(ntmp.Contains("10KEV")) {
//     cutele = cutgam = 1.e-5;
//   } else if(ntmp.Contains("50KEV")) {
//     cutele = cutgam = 5.e-5;
//   } else if(ntmp.Contains("100KEV")) {
//     cutele = cutgam = 1.e-4;
//   } else if(ntmp.Contains("200KEV")) {
//     cutele = cutgam = 2.e-4;
//   } else if(ntmp.Contains("500KEV")) {
//     cutele = cutgam = 5.e-4;
//   }

//   TVirtualMC::GetMC()->Gstpar(idtmed[1600],"CUTGAM", cutgam);
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600],"CUTELE", cutele); // 1MEV -> 0.1MEV; 15-aug-05
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600],"BCUTE",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600],"BCUTM",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   // --- Generate explicitly delta rays in Lead ---
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600], "LOSS", 3) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600], "DRAY", 1) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600], "DCUTE", cutele) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1600], "DCUTM", cutele) ;

// // --- in aluminium parts ---
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602],"CUTGAM", cutgam) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602],"CUTELE", cutele) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602],"BCUTE",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602],"BCUTM",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602], "LOSS",3.) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602], "DRAY",1.) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602], "DCUTE", cutele) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1602], "DCUTM", cutele) ;

// // --- and finally thresholds for photons and electrons in the scintillator ---
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601],"CUTGAM", cutgam) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601],"CUTELE", cutele) ;// 1MEV -> 0.1MEV; 15-aug-05
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601],"BCUTE",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601],"BCUTM",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601], "LOSS",3) ; // generate delta rays 
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601], "DRAY",1) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601], "DCUTE", cutele) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1601], "DCUTM", cutele) ;

//   // S steel - 
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603],"CUTGAM", cutgam);
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603],"CUTELE", cutele);
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603],"BCUTE",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603],"BCUTM",  cutgam);  // BCUTE and BCUTM start from GUTGUM
//   // --- Generate explicitly delta rays 
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603], "LOSS",3);
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603], "DRAY",1);
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603], "DCUTE", cutele) ;
//   TVirtualMC::GetMC()->Gstpar(idtmed[1603], "DCUTM", cutele) ;

//   AliEMCALGeometry* geom = GetGeometry();
//   if(geom->GetILOSS()>=0) {
//     for(int i=1600; i<=1603; i++) TVirtualMC::GetMC()->Gstpar(idtmed[i], "LOSS", geom->GetILOSS()) ; 
//   } 
//   if(geom->GetIHADR()>=0) {
//     for(int i=1600; i<=1603; i++) TVirtualMC::GetMC()->Gstpar(idtmed[i], "HADR", geom->GetIHADR()) ; 
//   }
// }

/// Create and return the digitizer.
//____________________________________________________________________________
AliDigitizer* AliEMCAL::CreateDigitizer(AliDigitizationInput* digInput) const
{
  return new AliEMCALDigitizer(digInput);
}

///
/// Definitions of materials to build EMCAL and associated tracking media.
//____________________________________________________________________________
void AliEMCAL::CreateMaterials()
{
  // media number in idtmed are 1599 to 1698.
  // --- Air ---               
  Float_t aAir[4]={12.0107,14.0067,15.9994,39.948};
  Float_t zAir[4]={6.,7.,8.,18.};
  Float_t wAir[4]={0.000124,0.755267,0.231781,0.012827};
  Float_t dAir = 1.20479E-3;
  AliMixture(0, "Air$", aAir, zAir, dAir, 4, wAir) ;
  
  // --- Lead ---                                                                     
  AliMaterial(1, "Pb$", 207.2, 82, 11.35, 0.56, 0., 0, 0) ;
  
  
  // --- The polysterene scintillator (CH) ---
  Float_t aP[2] = {12.011, 1.00794} ;
  Float_t zP[2] = {6.0, 1.0} ;
  Float_t wP[2] = {1.0, 1.0} ;
  Float_t dP = 1.032 ;
  
  AliMixture(2, "Polystyrene$", aP, zP, dP, -2, wP) ;
  
  // --- Aluminium ---
  AliMaterial(3, "Al$", 26.98, 13., 2.7, 8.9, 999., 0, 0) ;
  // ---         Absorption length is ignored ^
  
  // 25-aug-04 by PAI - see  PMD/AliPMDv0.cxx for STEEL definition
  Float_t asteel[4] = { 55.847,51.9961,58.6934,28.0855 };
  Float_t zsteel[4] = { 26.,24.,28.,14. };
  Float_t wsteel[4] = { .715,.18,.1,.005 };
  AliMixture(4, "STAINLESS STEEL$", asteel, zsteel, 7.88, 4, wsteel);
  
  // Oct 26,2010 : Multipurpose Copy Paper UNV-21200), weiht 75 g/m**2. 
  // *Cellulose C6H10O5
  //    Component C  A=12.01   Z=6.    W=6./21.
  //    Component H  A=1.      Z=1.    W=10./21.
  //    Component O  A=16.     Z=8.    W=5./21.
  Float_t apaper[3] = { 12.01, 1.0, 16.0};
  Float_t zpaper[3] = {  6.0,  1.0,  8.0};
  Float_t wpaper[3] = {6./21., 10./21., 5./21.};
  AliMixture(5, "BondPaper$", apaper, zpaper, 0.75, 3, wpaper);
  
  // DEFINITION OF THE TRACKING MEDIA
  // Look to the $ALICE_ROOT/data/galice.cuts for particular values
  // of cuts.
  // Don't forget to add a new tracking medium with non-default cuts
  
  // for EMCAL: idtmed[1599->1698] equivalent to fIdtmed[0->100]
  Int_t   isxfld = ((AliMagF*)TGeoGlobalMagField::Instance()->GetField())->Integ() ;
  Float_t sxmgmx = ((AliMagF*)TGeoGlobalMagField::Instance()->GetField())->Max() ;
  
  // Air                                                                         -> idtmed[1599]
  AliMedium(0, "Air$", 0, 0,
            isxfld, sxmgmx, 10.0, 1.0, 0.1, 0.1, 10.0, 0, 0) ;
  
  // The Lead                                                                      -> idtmed[1600]
  
  AliMedium(1, "Lead$", 1, 0,
            isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.1, 0.1, 0, 0) ;
  
  // The scintillator of the CPV made of Polystyrene scintillator                   -> idtmed[1601]
  float deemax = 0.1; // maximum fractional energy loss in one step (0 < DEEMAX < deemax )
  AliMedium(2, "Scintillator$", 2, 1,
            isxfld, sxmgmx, 10.0, 0.001, deemax, 0.001, 0.001, 0, 0) ;
  
  // Various Aluminium parts made of Al                                            -> idtmed[1602]
  AliMedium(3, "Al$", 3, 0,
            isxfld, sxmgmx, 10.0, 0.1, 0.1, 0.001, 0.001, 0, 0) ;
  
  // 25-aug-04 by PAI : see  PMD/AliPMDv0.cxx for STEEL definition                 -> idtmed[1603]
  AliMedium(4, "S steel$", 4, 0, 
            isxfld, sxmgmx, 10.0, 0.01, 0.1, 0.001, 0.001, 0, 0) ;
  
  // Oct 26,2010; Nov 24,2010                                                      -> idtmed[1604]
  deemax = 0.01;
  AliMedium(5, "Paper$", 5, 0, 
            isxfld, sxmgmx, 10.0, deemax, 0.1, 0.001, 0.001, 0, 0) ;
  
  
  // Set constants for Birk's Law implentation
  fBirkC0 =  1;
  fBirkC1 =  0.013/dP;
  fBirkC2 =  9.6e-6/(dP * dP);
}

///
/// Init (not needed)
//____________________________________________________________________________
void  AliEMCAL::Init()
{ 
  //Not needed, modify $ALICE_ROOT/data/galice.cuts instead.
  //Load the modified one in the configuration file with SetTransPar
  //DefineMediumParameters(); 
}  

///
/// Create raw samples from digits
//____________________________________________________________________________
void AliEMCAL::Digits2Raw() 
{
  //  static AliEMCALRawUtils rawUtils;
  // RS why this should be static? This makes root to die produce "double delete"
  // since the TF1 created in the rawUtils is garbage-collected by root before the 
  // destructor is called during garbage collecting of the static object
  AliEMCALRawUtils rawUtils;
  rawUtils.Digits2Raw();
  
}

///
/// Create summable digits from hits
//____________________________________________________________________________
void AliEMCAL::Hits2SDigits()  
{ 
  GetGeometry();
  
  AliEMCALSDigitizer emcalDigitizer(fLoader->GetRunLoader()->GetFileName().Data()) ;
  emcalDigitizer.SetEventRange(0, -1) ; // do all the events
  emcalDigitizer.Digitize() ;
}

///
/// Conversion from raw data to EMCAL sdigits. 
/// Does the same as AliEMCALReconstructor::ConvertDigits()
/// Needed to embed real data and simulation.
/// Works on a single-event basis.
///
/// \param rawReader: raw data stream pointer
//______________________________________________________________________
Bool_t AliEMCAL::Raw2SDigits(AliRawReader* rawReader)
{  
  rawReader->Reset() ; 
  
  // Get/create the sdigits tree and array
  AliRunLoader *rl = AliRunLoader::Instance();
  AliEMCALLoader *emcalLoader = dynamic_cast<AliEMCALLoader*>(rl->GetDetectorLoader("EMCAL")); 
  
  if(!emcalLoader)
  {
    AliFatal("NULL loader");
    return kFALSE;
  }
  
  emcalLoader->GetEvent();
  emcalLoader->LoadSDigits("UPDATE");
  
  TTree * treeS = emcalLoader->TreeS();
  if ( !treeS ) 
  { 
    emcalLoader->MakeSDigitsContainer();
    treeS = emcalLoader->TreeS();
  }
  
  if(!emcalLoader->SDigits()) 
  {
    AliFatal("No sdigits array available\n");
    return kFALSE;
  }
  
  TClonesArray * sdigits = emcalLoader->SDigits();
  sdigits->Clear("C");  
  
  // Trigger sdigits
  if (!fTriggerData) 
  {
    int dsize = (GetGeometry()->GetTriggerMappingVersion() == 2) ? 2 : 1;
    fTriggerData = new TClonesArray("AliEMCALTriggerData",dsize);
    for (int i=0;i<dsize;i++) 
    {
      new((*fTriggerData)[i]) AliEMCALTriggerData();
    }
  }
  
  for (int i=0;i<fTriggerData->GetEntriesFast();i++)
  {
    ((AliEMCALTriggerData*)fTriggerData->At(i))->SetMode(AliEMCALTriggerData::ETriggerDataMode_t::kRawData);   
  }
  
  const Int_t nTRU = GetGeometry()->GetNTotalTRU();
  TClonesArray *digitsTrg = new TClonesArray("AliEMCALTriggerRawDigit", nTRU * 96);    
  Int_t bufsize = 32000;
  treeS->Branch("EMTRG", &digitsTrg, bufsize);
  
  // Only physics events
  if (rawReader->GetType()== AliRawEventHeaderBase::kPhysicsEvent)
  {
    if(!fgRawUtils)  fgRawUtils   = new AliEMCALRawUtils;
    
    // must be done here because, in constructor, option is not yet known
    fgRawUtils->SetOption(GetOption());
    
    // Set parameters from OCDB to raw utils
    AliEMCALRecParam* recpar = emcalLoader->ReconstructionParameters(0);
    // fgRawUtils->SetRawFormatHighLowGainFactor(recpar->GetHighLowGainFactor());
    // fgRawUtils->SetRawFormatOrder(recpar->GetOrderParameter());
    // fgRawUtils->SetRawFormatTau(recpar->GetTau());
    fgRawUtils->SetNoiseThreshold(recpar->GetNoiseThreshold());
    fgRawUtils->SetNPedSamples(recpar->GetNPedSamples());
    fgRawUtils->SetRemoveBadChannels(recpar->GetRemoveBadChannels());
    fgRawUtils->SetFittingAlgorithm(recpar->GetFittingAlgorithm());
    fgRawUtils->SetFALTROUsage(recpar->UseFALTRO());
    // fgRawUtils->SetTimeMin(recpar->GetTimeMin());
    // fgRawUtils->SetTimeMax(recpar->GetTimeMax());
    
    // Fit
    fgRawUtils->Raw2Digits(rawReader,sdigits,emcalLoader->PedestalData(),digitsTrg,fTriggerData);
    
  }// skip calibration event
  else
  {
    AliDebug(1," Calibration Event, skip!");
  }
  
  // Final arrangements of the array, set all sdigits as embedded
  sdigits->Sort() ;
  for (Int_t iSDigit = 0 ; iSDigit < sdigits->GetEntriesFast() ; iSDigit++)
  { 
    AliEMCALDigit * sdigit = dynamic_cast<AliEMCALDigit *>(sdigits->At(iSDigit)) ;
    if(sdigit)
    {
      sdigit->SetIndexInList(iSDigit) ;
      sdigit->SetType(AliEMCALDigit::kEmbedded);
    }
    else 
    {
      AliFatal("sdigit is NULL!");
    }
  }	
  
  AliDebug(1,Form("Embedded sdigits entries %d \n",sdigits->GetEntriesFast()));
  
  // Write array, clean arrays, unload ..
  
  Int_t bufferSize = 32000 ;    
  TBranch * sdigitsBranch = treeS->GetBranch("EMCAL");
  if (sdigitsBranch)
    sdigitsBranch->SetAddress(&sdigits);
  else
    treeS->Branch("EMCAL",&sdigits,bufferSize);
  
  treeS->Fill();
  emcalLoader->WriteSDigits("OVERWRITE");
  emcalLoader->UnloadSDigits();
  
  digitsTrg->Delete();
  delete digitsTrg;
  
  return kTRUE;
}


///
/// Create/recover EMCal Loader
//____________________________________________________________________________
AliLoader* AliEMCAL::MakeLoader(const char* topfoldername)
{
  // different behaviour than standard (singleton getter)
  // --> to be discussed and made eventually coherent
  fLoader = new AliEMCALLoader(GetName(),topfoldername);
  return fLoader;
}

///
/// Initializes and returns EMCAL geometry.
/// Get it from run number if fCheckRunNumberAndGeoVersion
//____________________________________________________________________________
AliEMCALGeometry* AliEMCAL::GetGeometry() const
{  
  // Pass the transpor model name (Geant3, Geant4, Fluka) and title to the geometry
  // Check the transport model name and option in the geometry, 
  // set sampling fraction depending on it (AliEMCALEMCGeometry::DefineSamplingFraction)
  
  TString mcname   = "";
  TString mctitle  = "";
  if(TVirtualMC::GetMC())
  {
    mcname  = TVirtualMC::GetMC()->GetName()  ;
    mctitle = TVirtualMC::GetMC()->GetTitle() ;
  }
  
  TString geoName(GetTitle());
  
  // Check if run number and requested geometry correspond to the same geometry as
  // in real data taking. To prevent errors in official simulation productions
  if(!(AliEMCALGeometry::GetInstance()))
  {
    // Set geometry with the name used in the configuration file
    if(!fCheckRunNumberAndGeoVersion)
    {
      AliInfo(Form("Geometry name in use <<%s>>, requested via Config file", geoName.Data()));
      return AliEMCALGeometry::GetInstance(GetTitle(),"EMCAL",mcname,mctitle) ;
    }
    else
    {
      // Check run number and version and set the corresponding one.
      // Get run number
      // AliRunLoader *rl = AliRunLoader::Instance();
      // Int_t runNumber = rl->GetRunNumber();
      
      AliCDBManager* man = AliCDBManager::Instance();
      Int_t runNumber = man->GetRun();
      
      return AliEMCALGeometry::GetInstanceFromRunNumber(runNumber,geoName,mcname,mctitle);
    }
  }// Init geometry for the first time
  
  
  return AliEMCALGeometry::GetInstance();
  
}
