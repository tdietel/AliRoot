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
#include <TMath.h> 
#include <TMinuit.h>
#include <TTree.h> 
#include <TBenchmark.h>
#include <TBrowser.h>
#include <TROOT.h>
#include <TClonesArray.h>

// --- Standard library ---
#include <cassert>

// --- AliRoot header files ---
#include "AliLog.h"
#include "AliEMCALClusterizerNxN.h"
#include "AliEMCALRecPoint.h"
#include "AliEMCALDigit.h"
#include "AliEMCALGeometry.h"
#include "AliCaloCalibPedestal.h"
#include "AliEMCALCalibData.h"
#include "AliEMCALCalibTime.h"
#include "AliESDCaloCluster.h"
#include "AliEMCALUnfolding.h"

/// \cond CLASSIMP
ClassImp(AliEMCALClusterizerNxN) ;
/// \endcond

///
/// Default constructor
//____________________________________________________________________________
AliEMCALClusterizerNxN::AliEMCALClusterizerNxN()
  : AliEMCALClusterizer(), fNRowDiff(1), fNColDiff(1), fEnergyGrad(0)
{ }

///
/// Constructor 
/// 
/// \param geometry: EMCal geometry pointer
//____________________________________________________________________________
AliEMCALClusterizerNxN::AliEMCALClusterizerNxN(AliEMCALGeometry* geometry)
  : AliEMCALClusterizer(geometry), fNRowDiff(1), fNColDiff(1), fEnergyGrad(0)
{ }

///
/// Constructor, geometry and calibration are initialized elsewhere.
///
/// \param geometry: EMCal geometry pointer
/// \param calib: EMCal energy calibration container
/// \param calibt: EMCal time calibration container
/// \param caloped: EMCal bad map container
//____________________________________________________________________________
AliEMCALClusterizerNxN::AliEMCALClusterizerNxN(AliEMCALGeometry* geometry, 
                                               AliEMCALCalibData * calib, 
                                               AliEMCALCalibTime * calibt, 
                                               AliCaloCalibPedestal * caloped)
: AliEMCALClusterizer(geometry, calib, calibt, caloped), 
fNRowDiff(1), fNColDiff(1), fEnergyGrad(0)
{ }

///
/// Destructtor
//____________________________________________________________________________
AliEMCALClusterizerNxN::~AliEMCALClusterizerNxN()
{ }

///
/// Steering method to perform clusterization for the current event 
/// in AliEMCALLoader
//____________________________________________________________________________
void AliEMCALClusterizerNxN::Digits2Clusters(Option_t * option)
{  
  if (strstr(option,"tim"))
    gBenchmark->Start("EMCALClusterizer"); 
  
  if (strstr(option,"print"))
    Print(""); 
  
  // Get calibration parameters from file or digitizer default values.
  GetCalibrationParameters();
  
  //Get dead channel map from file or digitizer default values.
  GetCaloCalibPedestal();
  
  MakeClusters();  //only the real clusters
  
  if (fToUnfold)
  {
    fClusterUnfolding->SetInput(fNumberOfECAClusters,fRecPoints,fDigitsArr);
    fClusterUnfolding->MakeUnfolding();
  }
  
  // Evaluate position, dispersion and other RecPoint properties for EC section 
  for (Int_t index = 0; index < fRecPoints->GetEntries(); index++) 
  { 
    AliEMCALRecPoint * rp = dynamic_cast<AliEMCALRecPoint *>(fRecPoints->At(index));
    if (rp) {
      rp->EvalAll(fECAW0,fDigitsArr,fJustClusters);
      AliDebug(5, Form("MAX INDEX %d ", rp->GetMaximalEnergyIndex()));
      //For each rec.point set the distance to the nearest bad crystal
      if (fCaloPed)
        rp->EvalDistanceToBadChannels(fCaloPed);
    }
  }
  
  fRecPoints->Sort();
  
  for (Int_t index = 0; index < fRecPoints->GetEntries(); index++) 
  {
    AliEMCALRecPoint *rp = dynamic_cast<AliEMCALRecPoint *>(fRecPoints->At(index));
    if (rp) 
    {
      rp->SetIndexInList(index);
    }
    else AliFatal("RecPoint NULL!!");
  }
  
  if (fTreeR)
    fTreeR->Fill();
  
  if (strstr(option,"deb") || strstr(option,"all"))  
    PrintRecPoints(option);
  
  AliDebug(1,Form("EMCAL Clusterizer found %d Rec Points",fRecPoints->GetEntriesFast()));
  
  if (strstr(option,"tim")) 
  {
    gBenchmark->Stop("EMCALClusterizer");
    printf("Exec took %f seconds for Clusterizing", 
           gBenchmark->GetCpuTime("EMCALClusterizer"));
  }    
}

///
/// Gives the neighbourness of two digits = 0 are not neighbour ; continue searching 
///                                       = 1 are neighbour
///                                       = 2 is in different SM; continue searching 
/// In case it is in different SM, but same phi rack, check if neigbours at eta=0
/// neighbours are defined as digits having at least a common side 
/// The order of d1 and d2 is important: first (d1) should be a digit already in a cluster 
///                                      which is compared to a digit (d2)  not yet in a cluster  
//____________________________________________________________________________
Int_t AliEMCALClusterizerNxN::AreNeighbours(AliEMCALDigit * d1, AliEMCALDigit * d2, Bool_t & shared) const
{  
  if (fEnergyGrad) 
  { //false by default
    if (d2->GetCalibAmp()>d1->GetCalibAmp())
      return 3; // energy of neighboring cell should be smaller in order to become a neighbor
  }
  
  Int_t nSupMod1=0, nModule1=0, nIphi1=0, nIeta1=0, iphi1=0, ieta1=0;
  Int_t nSupMod2=0, nModule2=0, nIphi2=0, nIeta2=0, iphi2=0, ieta2=0;
  Int_t rowdiff=0, coldiff=0;
  
  shared = kFALSE;
  
  fGeom->GetCellIndex(d1->GetId(), nSupMod1,nModule1,nIphi1,nIeta1);
  fGeom->GetCellIndex(d2->GetId(), nSupMod2,nModule2,nIphi2,nIeta2);
  fGeom->GetCellPhiEtaIndexInSModule(nSupMod1,nModule1,nIphi1,nIeta1, iphi1,ieta1);
  fGeom->GetCellPhiEtaIndexInSModule(nSupMod2,nModule2,nIphi2,nIeta2, iphi2,ieta2);
  
  //If different SM, check if they are in the same phi, then consider cells close to eta=0 as neighbours; May 2010
  if (nSupMod1 != nSupMod2 ) 
  {
    //Check if the 2 SM are in the same PHI position (0,1), (2,3), ...
    Float_t smPhi1 = fGeom->GetEMCGeometry()->GetPhiCenterOfSM(nSupMod1);
    Float_t smPhi2 = fGeom->GetEMCGeometry()->GetPhiCenterOfSM(nSupMod2);
    
    if (!TMath::AreEqualAbs(smPhi1, smPhi2, 1e-3)) return 2; //Not phi rack equal, not neighbours
    
    // In case of a shared cluster, index of SM in C side, columns start at 48 and ends at 48*2
    // C Side impair SM, nSupMod%2=1; A side pair SM nSupMod%2=0
    if (nSupMod1%2) ieta1+=AliEMCALGeoParams::fgkEMCALCols;
    else           ieta2+=AliEMCALGeoParams::fgkEMCALCols;
    
    shared = kTRUE; // maybe a shared cluster, we know this later, set it for the moment.
    
  }//Different SM, same phi
  
  rowdiff = TMath::Abs(iphi1 - iphi2);  
  coldiff = TMath::Abs(ieta1 - ieta2);  
  
  // neighbours +-1 in col and row
  if ( TMath::Abs(coldiff) <= fNColDiff && TMath::Abs(rowdiff) <= fNRowDiff)
  {
    
    AliDebug(9, Form("AliEMCALClusterizerNxN::AreNeighbours(): id1=%d, (row %d, col %d) ; id2=%d, (row %d, col %d), shared %d \n",
                     d1->GetId(), iphi1,ieta1, d2->GetId(), iphi2,ieta2, shared));
    
    return 1;
  }//Neighbours
  else 
  {
    AliDebug(9, Form("NOT AliEMCALClusterizerNxN::AreNeighbours(): id1=%d, (row %d, col %d) ; id2=%d, (row %d, col %d), shared %d \n",
                     d1->GetId(), iphi1,ieta1, d2->GetId(), iphi2,ieta2, shared));
    shared = kFALSE;
    return 2; 
  }//Not neighbours
}

///
/// Make clusters
//____________________________________________________________________________
void AliEMCALClusterizerNxN::MakeClusters()
{  
  if (fGeom==0) 
    AliFatal("Did not get geometry from EMCALLoader");
  
  fNumberOfECAClusters = 0;
  fRecPoints->Delete();
  
  // Set up TObjArray with pointers to digits to work on, calibrate digits 
  TObjArray digitsC;
  TIter nextdigit(fDigitsArr);
  AliEMCALDigit *digit = 0;
  while ( (digit = static_cast<AliEMCALDigit*>(nextdigit())) ) 
  {
    Float_t dEnergyCalibrated = digit->GetAmplitude();
    Float_t time              = digit->GetTime();
    Calibrate(dEnergyCalibrated,time ,digit->GetId());
    digit->SetCalibAmp(dEnergyCalibrated);
    digit->SetTime(time);    
    if (dEnergyCalibrated > fMinECut)
        digitsC.AddLast(digit);
  }
  
  TIter nextdigitC(&digitsC);
  
  AliDebug(1,Form("MakeClusters: Number of digits %d  -> (e %f)\n",
                  fDigitsArr->GetEntries(),fMinECut));
  
  Bool_t bDone = kFALSE;
  while ( bDone != kTRUE )
  {
    // first sort the digits:
    Int_t iMaxEnergyDigit = -1;
    Float_t dMaxEnergyDigit = -1;
    AliEMCALDigit *pMaxEnergyDigit = 0;
    
    nextdigitC.Reset();
    while ( (digit = static_cast<AliEMCALDigit *>(nextdigitC())) ) 
    { // scan over the list of digitsC
      Float_t dEnergyCalibrated = digit->GetCalibAmp();
      Float_t time              = digit->GetTime();
      if (fGeom->CheckAbsCellId(digit->GetId()) &&
          dEnergyCalibrated > fECAClusteringThreshold &&
          time              < fTimeMax          &&
          time              > fTimeMin             ) // no threshold by default!
      {                                              // needs to be set in OCDB!
        if (dEnergyCalibrated > dMaxEnergyDigit) 
        {
          dMaxEnergyDigit = dEnergyCalibrated;
          iMaxEnergyDigit = digit->GetId();
          pMaxEnergyDigit = digit;
        }
      }
    }
    
    if (iMaxEnergyDigit < 0 || digitsC.GetEntries() <= 0) 
    {
      bDone = kTRUE;
      continue;
    }
    
    AliDebug (2, Form("Max digit found: %1.5f AbsId: %d", dMaxEnergyDigit, iMaxEnergyDigit));
    
    // keep the candidate digits in a list
    TList clusterDigitList;
    clusterDigitList.SetOwner(kFALSE);
    clusterDigitList.AddLast(pMaxEnergyDigit);	 
    
    Double_t clusterCandidateEnergy = dMaxEnergyDigit;
    
    // now loop over the rest of the digits and cluster into NxN cluster 
    // we do not actually cluster yet: we keep them in the list clusterDigitList
    nextdigitC.Reset();
    while ( (digit = static_cast<AliEMCALDigit *>(nextdigitC())) ) 
    { // scan over the list of digitsC
      if (digit == pMaxEnergyDigit) continue;
      Float_t dEnergyCalibrated = digit->GetCalibAmp();
      
      AliDebug(5, Form("-> Digit ENERGY: %1.5f", dEnergyCalibrated));
      
      if (fGeom->CheckAbsCellId(digit->GetId()) && dEnergyCalibrated > 0.0  )
      {
        Float_t time = pMaxEnergyDigit->GetTime(); //Time or TimeR?
        if (TMath::Abs(time - digit->GetTime()) > fTimeCut ) continue; //Time or TimeR?
        Bool_t shared = kFALSE; //cluster shared by 2 SuperModules?
        if (AreNeighbours(pMaxEnergyDigit, digit, shared) == 1) // call (digit,digitN) in THAT order !!!!! 
        {      
          clusterDigitList.AddLast(digit);
          clusterCandidateEnergy += dEnergyCalibrated;
        }
      }
    }// loop over the next digits
    
    // start a cluster here only if a cluster energy is larger than clustering threshold
    AliDebug(5, Form("Clusterization threshold is %f MeV", fECAClusteringThreshold));
    
    if (fNumberOfECAClusters >= fRecPoints->GetSize()) 
      fRecPoints->Expand(2*fNumberOfECAClusters+1);
    
    (*fRecPoints)[fNumberOfECAClusters] = new AliEMCALRecPoint("") ;
    AliEMCALRecPoint *recPoint = dynamic_cast<AliEMCALRecPoint *>( fRecPoints->At(fNumberOfECAClusters) ) ;
    
    // AliEMCALRecPoint *recPoint = new  AliEMCALRecPoint(""); 
    // fRecPoints->AddAt(recPoint, fNumberOfECAClusters);
    // recPoint = static_cast<AliEMCALRecPoint *>(fRecPoints->At(fNumberOfECAClusters)); 
    if (recPoint) 
    {
      fNumberOfECAClusters++;       
      recPoint->SetClusterType(AliVCluster::kEMCALClusterv1);
      
      AliDebug(9, Form("Number of cells per cluster (max is 9!): %d", clusterDigitList.GetEntries()));
      
      for (Int_t idig = 0; idig < clusterDigitList.GetEntries(); idig++)
      {
        digit = (AliEMCALDigit*)clusterDigitList.At(idig);
        AliDebug(5, Form(" Adding digit %d", digit->GetId()));
        // note: this way the sharing info is lost!
        recPoint->AddDigit(*digit, digit->GetCalibAmp(), kFALSE); //Time or TimeR?
        digitsC.Remove(digit); 		  
      }
    }// recpoint

    
    AliDebug (2, Form("Number of digits left: %d", digitsC.GetEntries()));      
  } // while ! done 
  
  AliDebug(1,Form("total no of clusters %d from %d digits",fNumberOfECAClusters,fDigitsArr->GetEntriesFast())); 
}
