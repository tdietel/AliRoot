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

/* $Id$ */

// Generator using HIJING as an external generator
// The main HIJING options are accessable for the user through this interface.
// Uses the THijing implementation of TGenerator.
// Author:
// Andreas Morsch    (andreas.morsch@cern.ch)
//

#include <TClonesArray.h>
#include <TGraph.h>
#include <THijing.h>
#include <TLorentzVector.h>
#include <TPDGCode.h>
#include <TParticle.h>
#include <TF1.h>
#include <TFile.h>

#include "AliGenHijing.h"
#include "AliGenHijingEventHeader.h"
#include "AliHijingRndm.h"
#include "AliLog.h"
#include "AliRun.h"

ClassImp(AliGenHijing)

AliGenHijing::AliGenHijing()
    :AliGenMC(),
     fFrame("CMS"),
     fMinImpactParam(0.),
     fMaxImpactParam(5.),
     fKeep(0),
     fQuench(1),
     fShadowing(1),
     fDecaysOff(3),
     fTrigger(0),
     fEvaluate(0),
     fSelectAll(0),
     fFlavor(0),
     fKineBias(0.),
     fTrials(0),
     fXsection(0.),
     fHijing(0),
     fPtHardMin(2.0),
     fPtHardMax(-1),
     fSpectators(1),
     fDsigmaDb(0),
     fDnDb(0),
     fPtMinJet(-2.5),
     fEtaMinJet(-20.),
     fEtaMaxJet(+20.),
     fPhiMinJet(0.),
     fPhiMaxJet(2. * TMath::Pi()),
     fRadiation(3),
     fSimpleJet(kFALSE),
     fNoGammas(kFALSE),
     fProjectileSpecn(0),
     fProjectileSpecp(0),
     fTargetSpecn(0),
     fTargetSpecp(0),
     fLHC(kFALSE),
     fRandomPz(kFALSE),
     fNoHeavyQuarks(kFALSE),
     fHeader(AliGenHijingEventHeader("Hijing")),
     fSigmaNN(-1),
     fNoElas(1),
     fDataFragmentation(kTRUE),
     fFreeProjSpecn(0),
     fFreeProjSpecp(0),
     fFreeTargSpecn(0),
     fFreeTargSpecp(0),
     fFragmNeutrons(0x0),
     fFragmProtons(0x0),
     fFragmNWidth(0x0),
     fFragmPWidth(0x0),
     fEConv(1),
     fMissing(50),
     fFlipPzSign(kFALSE)
{
  // Constructor
  fEnergyCMS = 5500.;
  AliHijingRndm::SetHijingRandom(GetRandom());
}

AliGenHijing::AliGenHijing(Int_t npart)
    :AliGenMC(npart),
     fFrame("CMS"),
     fMinImpactParam(0.),
     fMaxImpactParam(5.),
     fKeep(0),
     fQuench(1),
     fShadowing(1),
     fDecaysOff(3),
     fTrigger(0),
     fEvaluate(0),
     fSelectAll(0),
     fFlavor(0),
     fKineBias(0.),
     fTrials(0),
     fXsection(0.),
     fHijing(0),
     fPtHardMin(2.0),
     fPtHardMax(-1),
     fSpectators(1),
     fDsigmaDb(0),
     fDnDb(0),
     fPtMinJet(-2.5),
     fEtaMinJet(-20.),
     fEtaMaxJet(+20.),
     fPhiMinJet(0.),
     fPhiMaxJet(2. * TMath::Pi()),
     fRadiation(3),
     fSimpleJet(kFALSE),
     fNoGammas(kFALSE),
     fProjectileSpecn(0),
     fProjectileSpecp(0),
     fTargetSpecn(0),
     fTargetSpecp(0),
     fLHC(kFALSE),
     fRandomPz(kFALSE),
     fNoHeavyQuarks(kFALSE),
     fHeader(AliGenHijingEventHeader("Hijing")),
     fSigmaNN(-1),
     fNoElas(1),
     fDataFragmentation(kTRUE),
     fFreeProjSpecn(0),
     fFreeProjSpecp(0),
     fFreeTargSpecn(0),
     fFreeTargSpecp(0),
     fFragmNeutrons(0x0),
     fFragmProtons(0x0),
     fFragmNWidth(0x0),
     fFragmPWidth(0x0),
     fEConv(1),
     fMissing(50),
     fFlipPzSign(kFALSE)     
{
// Default PbPb collisions at 5. 5 TeV
//
    fEnergyCMS = 5500.;
    fName = "Hijing";
    fTitle= "Particle Generator using HIJING";
//
//
// Set random number generator
    AliHijingRndm::SetHijingRandom(GetRandom());

}

AliGenHijing::~AliGenHijing()
{
// Destructor
    if ( fDsigmaDb) delete  fDsigmaDb;
    if ( fDnDb)     delete  fDnDb;
}

void AliGenHijing::Init()
{
// Initialisation

    // Coeffs to go from mm / mm to meter / second
    SetGeneratorUnitsForMeterSecond(1.e-3, 1e-3/TMath::C());

    fFrame.Resize(8);
    fTarget.Resize(8);
    fProjectile.Resize(8);

    SetMC(new THijing(fEnergyCMS, fFrame, fProjectile, fTarget,
		      fAProjectile, fZProjectile, fATarget, fZTarget,
		      fMinImpactParam, fMaxImpactParam));

    fHijing=(THijing*) fMCEvGen;
    fHijing->SetIHPR2(2,  fRadiation);
    fHijing->SetIHPR2(3,  fTrigger);
    fHijing->SetIHPR2(6,  fShadowing);
    fHijing->SetIHPR2(12, fDecaysOff);
    fHijing->SetIHPR2(21, fKeep);
    fHijing->SetHIPR1(8,  fPtHardMin);
    fHijing->SetHIPR1(9,  fPtHardMax);
    fHijing->SetHIPR1(10, fPtMinJet);
    if (fSigmaNN>0)
      fHijing->SetHIPR1(31, fSigmaNN/2.);
    fHijing->SetHIPR1(50, fSimpleJet);
    //
    // Switching off elastic scattering
    // By deafult it is off
    if (fNoElas) {
      fHijing->SetIHPR2(14, 0);
      printf("\n Elastic scattering between N-N pairs is switched off. \n");
    }
//
//  Quenching
//
//
//  fQuench = 0:  no quenching
//  fQuench = 1:  hijing default
//  fQuench = 2:  new LHC  parameters for HIPR1(11) and HIPR1(14)
//  fQuench = 3:  new RHIC parameters for HIPR1(11) and HIPR1(14)
//  fQuench = 4:  new LHC  parameters with log(e) dependence
//  fQuench = 5:  new RHIC parameters with log(e) dependence
    fHijing->SetIHPR2(50, 0);
    if (fQuench > 0)
	fHijing->SetIHPR2(4,  1);
    else
	fHijing->SetIHPR2(4,  0);
// New LHC parameters from Xin-Nian Wang
    if (fQuench == 2) {
	fHijing->SetHIPR1(14, 1.1);
	fHijing->SetHIPR1(11, 3.7);
    } else if (fQuench == 3) {
	fHijing->SetHIPR1(14, 0.20);
	fHijing->SetHIPR1(11, 2.5);
    } else if (fQuench == 4) {
	fHijing->SetIHPR2(50, 1);
	fHijing->SetHIPR1(14, 4.*0.34);
	fHijing->SetHIPR1(11, 3.7);
    } else if (fQuench == 5) {
	fHijing->SetIHPR2(50, 1);
	fHijing->SetHIPR1(14, 0.34);
	fHijing->SetHIPR1(11, 2.5);
    }

//
// Heavy quarks
//
    if (fNoHeavyQuarks) {
	fHijing->SetIHPR2(49, 1);
    } else {
	fHijing->SetIHPR2(49, 0);
    }

    if(fDataFragmentation){
      //TFile *file = TFile::Open("$ALICE_ROOT/ZDC/fragmSpecDataDriven.root","READ"); //old 2.76 TeV data
      TFile *file = TFile::Open("$ALICE_ROOT/ZDC/fragmentsDD5TeV.root","READ");
      if(!file->IsOpen()){
        AliError("Could not open file $ALICE_ROOT/ZDC/fragmentsDD5TeV.root");
      }
      //fFragmNeutrons = dynamic_cast<TF1*> (file->Get("funcorrn")); //old 2.76 TeV data
      //fFragmProtons = dynamic_cast<TF1*> (file->Get("funcorrp")); //old 2.76 TeV data
      fFragmNeutrons = dynamic_cast<TF1*> (file->Get("fneu"));
      fFragmProtons = dynamic_cast<TF1*> (file->Get("fitfunp"));
      fFragmNWidth = dynamic_cast<TF1*> (file->Get("ferrn"));
      fFragmPWidth = dynamic_cast<TF1*> (file->Get("ferrp"));
    }

    // deal with energy/momentun conservation and counting of errors in hijhrd
    fHijing->SetIHPR2(31,fEConv);
    if (fMissing>0)
      fHijing->SetIHPR2(30,fMissing);

    AliGenMC::Init();

//
//  Initialize Hijing
//
    fHijing->Initialize();
//
    if (fEvaluate) EvaluateCrossSections();
//
}

void AliGenHijing::SetSeed(UInt_t seed)
{
  AliHijingRndm::GetHijingRandom()->SetSeed(seed);
}

void AliGenHijing::Generate()
{
// Generate one event

  Float_t polar[3]    =   {0,0,0};
  Float_t origin[3]   =   {0,0,0};
  Float_t origin0[3]  =   {0,0,0};
  Float_t time0 = 0.;
  Float_t p[3];
  Float_t tof;

//  converts from mm/c to s
  const Float_t kconv = 0.001/2.99792458e8;
//
  Int_t nt  = 0;
  Int_t jev = 0;
  Int_t j=0, kf=0, ks=0, ksp=0, imo=0;
  kf = 0;



  fTrials = 0;

  for (j = 0;j < 3; j++) origin0[j] = fOrigin[j];
  time0 = fTimeOrigin;

  if(fVertexSmear == kPerEvent) {
      Vertex();
      for (j=0; j < 3; j++) origin0[j] = fVertex[j];
      time0 = fTime;
  }


  Float_t sign = (fRandomPz && (Rndm() < 0.5))? -1. : 1.;

  while(1)
  {
//    Generate one event
// --------------------------------------------------------------------------
      fProjectileSpecn    = 0;
      fProjectileSpecp    = 0;
      fTargetSpecn        = 0;
      fTargetSpecp        = 0;
// --------------------------------------------------------------------------
      fHijing->GenerateEvent();
      fTrials++;
      fNprimaries = 0;
      fHijing->ImportParticles(&fParticles,"All");
      if (fTrigger != kNoTrigger) {
	  if (!CheckTrigger()) continue;
      }
      if (fLHC) Boost();


      Int_t np = fParticles.GetEntriesFast();
      Int_t nc = 0;
      if (np == 0 ) continue;
      Int_t i;
      Int_t* newPos     = new Int_t[np];
      Int_t* pSelected  = new Int_t[np];

      for (i = 0; i < np; i++) {
	  newPos[i]    = i;
	  pSelected[i] = 0;
      }

//      Get event vertex
//
      fVertex[0] = origin0[0];
      fVertex[1] = origin0[1];
      fVertex[2] = origin0[2];
      fTime = time0;
//
//      First select parent particles
//
      TParticle *  iparticle = 0;
      for (i = 0; i < np; i++) {
	  iparticle = (TParticle *) fParticles.At(i);

// Is this a parent particle ?
	  if (Stable(iparticle)) continue;
//
	  Bool_t  selected             =  kTRUE;
	  Bool_t  hasSelectedDaughters =  kFALSE;


	  kf        = iparticle->GetPdgCode();
	  ks        = iparticle->GetStatusCode();
	  if (kf == 92) continue;

          if (!fSelectAll) selected = KinematicSelection(iparticle, 0) &&
	  		       SelectFlavor(kf);
	  hasSelectedDaughters = DaughtersSelection(iparticle);
//
// Put particle on the stack if it is either selected or
// it is the mother of at least one seleted particle
//
	  if (selected || hasSelectedDaughters) {
	      nc++;
	      pSelected[i] = 1;
	  } // selected
      } // particle loop parents
//
// Now select the final state particles
//

      for (i = 0; i<np; i++) {
	  iparticle = (TParticle *) fParticles.At(i);
// Is this a final state particle ?
	  if (!Stable(iparticle)) continue;

	  Bool_t  selected             =  kTRUE;
	  kf        = iparticle->GetPdgCode();
	  ks        = iparticle->GetStatusCode();
	  ksp       = iparticle->GetUniqueID();

// --------------------------------------------------------------------------
// Count spectator neutrons and protons
	  if(ksp == 0 || ksp == 1){
	      if(kf == kNeutron) fProjectileSpecn += 1;
	      if(kf == kProton)  fProjectileSpecp += 1;
	  }
	  else if(ksp == 10 || ksp == 11){
	      if(kf == kNeutron) fTargetSpecn += 1;
	      if(kf == kProton)  fTargetSpecp += 1;
	  }
// --------------------------------------------------------------------------
//
	  if (!fSelectAll) {
	      selected = KinematicSelection(iparticle,0)&&SelectFlavor(kf);
	      if (!fSpectators && selected) selected = (ksp != 0 && ksp != 1 && ksp != 10
							&& ksp != 11);
	  }
//
// Put particle on the stack if selected
//
	  if (selected) {
	      nc++;
	      pSelected[i] = 1;
	  } // selected
      } // particle loop final state

      if(fDataFragmentation){
         Float_t impPar = fHijing->GetBB();
         fFreeProjSpecn = FreeSpectatorsn(impPar);
         fFreeProjSpecp = FreeSpectatorsp(impPar);
         fFreeTargSpecn = FreeSpectatorsn(impPar);
         fFreeTargSpecp = FreeSpectatorsp(impPar);
	 //
	 printf("\n b %f fm - SPECTATORS: DATA DRIVEN:   %d  %d  %d  %d \n", impPar, fFreeProjSpecn, fFreeProjSpecp, fFreeTargSpecn, fFreeTargSpecp);
      }
//
// Write particles to stack
      Int_t countSpecPn=0, countSpecPp=0;
      Int_t countSpecTn=0, countSpecTp=0;
      for (i = 0; i<np; i++) {
	  iparticle = (TParticle *) fParticles.At(i);
	  Bool_t  hasMother   = (iparticle->GetFirstMother()     >=0);
	  Bool_t  hasDaughter = (iparticle->GetFirstDaughter()   >=0);
	  if (pSelected[i]) {
	      kf   = iparticle->GetPdgCode();
	      ks   = iparticle->GetStatusCode();
	      ksp  = iparticle->GetUniqueID();
	      p[0] = iparticle->Px();
	      p[1] = iparticle->Py();
	      p[2] = iparticle->Pz() * sign;
	      origin[0] = origin0[0]+iparticle->Vx()/10;
	      origin[1] = origin0[1]+iparticle->Vy()/10;
	      origin[2] = origin0[2]+iparticle->Vz()/10;
	      tof = time0+kconv * iparticle->T();

	      if(fFlipPzSign){
		p[2] = -p[2];
		origin[2] = origin0[2] - iparticle->Vz()/10;
	      }

	      
	      imo = -1;
	      TParticle* mother = 0;
	      if (hasMother) {
		  imo = iparticle->GetFirstMother();
		  mother = (TParticle *) fParticles.At(imo);
		  imo = (mother->GetPdgCode() != 92) ? newPos[imo] : -1;
	      } // if has mother
	      Bool_t tFlag = (fTrackIt && !hasDaughter);
	      // Don't put in the stack a no. of spectators larger than the number of free spectators
	      if(fDataFragmentation && (ksp==0 || ksp==1) && kf == kNeutron){ // Projectile neutrons
	         countSpecPn++;
		 if(countSpecPn>fFreeProjSpecn) continue;
		 //printf(" Putting spec.n from proj. %d into the stack\n", countSpecPn);
	      }
	      if(fDataFragmentation && (ksp==0 || ksp==1) && kf == kProton){ // Projectile protons
	         countSpecPp++;
		 if(countSpecPp>fFreeProjSpecp) continue;
		 //printf(" Putting spec.p from proj. %d into the stack\n", countSpecPp);
	      }
	      if(fDataFragmentation && (ksp==10 || ksp==11) && kf == kNeutron){ // Target neutrons
	         countSpecTn++;
		 if(countSpecTn>fFreeTargSpecn) continue;
		 //printf(" Putting spec.n from targ. %d into the stack\n", countSpecTn);
	      }
	      if(fDataFragmentation && (ksp==10 || ksp==11) && kf == kProton){ // Target protons
	         countSpecTp++;
		 if(countSpecTp>fFreeTargSpecp) continue;
		 //printf(" Putting spec.p from targ. %d into the stack\n", countSpecTp);
	      }
	      PushTrack(tFlag,imo,kf,p,origin,polar,tof,kPNoProcess,nt, 1., ks);
	      if (nt>=0) {
		fNprimaries++;
		KeepTrack(nt);
	      }
	      newPos[i] = nt;
	  } // if selected
      } // particle loop
      delete[] newPos;
      delete[] pSelected;

      AliInfo(Form("\n I've put %i particles on the stack \n",nc));
      if (nc > 0) {
	  jev += nc;
	  if (jev >= fNpart || fNpart == -1) {
	      fKineBias = Float_t(fNpart)/Float_t(fTrials);
	      AliInfo(Form("\n Trials: %i %i %i\n",fTrials, fNpart, jev));
	      break;
	  }
      }
  } // event loop

  MakeHeader();
  SetHighWaterMark(nt);
}

void AliGenHijing::KeepFullEvent()
{
    fKeep=1;
}

void AliGenHijing::EvaluateCrossSections()
{
//     Glauber Calculation of geometrical x-section
//
    Float_t xTot       = 0.;          // barn
    Float_t xTotHard   = 0.;          // barn
    Float_t xPart      = 0.;          // barn
    Float_t xPartHard  = 0.;          // barn
    Float_t sigmaHard  = 0.1;         // mbarn
    Float_t bMin       = 0.;
    Float_t bMax       = fHijing->GetHIPR1(34)+fHijing->GetHIPR1(35);
    const Float_t kdib = 0.2;
    Int_t   kMax       = Int_t((bMax-bMin)/kdib)+1;


    printf("\n Projectile Radius (fm): %f \n",fHijing->GetHIPR1(34));
    printf("\n Target     Radius (fm): %f \n",fHijing->GetHIPR1(35));
    printf("\n Inelastic and total cross section (mb) %f %f \n",fHijing->GetHINT1(12), fHijing->GetHINT1(13));
    Int_t i;
    Float_t oldvalue= 0.;

    Float_t* b   = new Float_t[kMax];
    Float_t* si1 = new Float_t[kMax];
    Float_t* si2 = new Float_t[kMax];
    for (i = 0; i < kMax; i++){
      b[i] = 0.;
      si1[i] = 0.;
      si2[i] = 0.;
    }

    for (i = 0; i < kMax; i++)
    {
	Float_t xb  = bMin+i*kdib;
	Float_t ov;
	ov=fHijing->Profile(xb);
	Float_t gb  =  2.*0.01*fHijing->GetHIPR1(40)*kdib*xb*(1.-TMath::Exp(-fHijing->GetHINT1(12)*ov));
	Float_t gbh =  2.*0.01*fHijing->GetHIPR1(40)*kdib*xb*sigmaHard*ov;
	xTot+=gb;
	xTotHard += gbh;
	printf("profile %f %f %f\n", xb, ov, fHijing->GetHINT1(12));

	if (xb > fMinImpactParam && xb < fMaxImpactParam)
	{
	    xPart += gb;
	    xPartHard += gbh;
	}

	if(oldvalue) if ((xTot-oldvalue)/oldvalue<0.0001) break;
	oldvalue = xTot;
	printf("\n Total cross section (barn): %d %f %f \n",i, xb, xTot);
	printf("\n Hard  cross section (barn): %d %f %f \n\n",i, xb, xTotHard);
	if (i>0) {
	    si1[i] = gb/kdib;
	    si2[i] = gbh/gb;
	    b[i]  = xb;
	}
    }

    printf("\n Total cross section (barn): %f \n",xTot);
    printf("\n Hard  cross section (barn): %f \n \n",xTotHard);
    printf("\n Partial       cross section (barn): %f %f \n",xPart, xPart/xTot*100.);
    printf("\n Partial  hard cross section (barn): %f %f \n",xPartHard, xPartHard/xTotHard*100.);

//  Store result as a graph
    b[0] = 0;
    si1[0] = 0;
    si2[0]=si2[1];

    fDsigmaDb  = new TGraph(i, b, si1);
    fDnDb      = new TGraph(i, b, si2);
}

Bool_t AliGenHijing::DaughtersSelection(const TParticle* iparticle)
{
//
// Looks recursively if one of the daughters has been selected
//
//    printf("\n Consider daughters %d:",iparticle->GetPdgCode());
    Int_t imin = -1;
    Int_t imax = -1;
    Int_t i;
    Bool_t hasDaughters = (iparticle->GetFirstDaughter() >=0);
    Bool_t selected = kFALSE;
    if (hasDaughters) {
	imin = iparticle->GetFirstDaughter();
	imax = iparticle->GetLastDaughter();
	for (i = imin; i <= imax; i++){
	    TParticle *  jparticle = (TParticle *) fParticles.At(i);
	    Int_t ip = jparticle->GetPdgCode();
	    if (KinematicSelection(jparticle,0)&&SelectFlavor(ip)) {
		selected=kTRUE; break;
	    }
	    if (DaughtersSelection(jparticle)) {selected=kTRUE; break; }
	}
    } else {
	return kFALSE;
    }
    return selected;
}


Bool_t AliGenHijing::SelectFlavor(Int_t pid)
{
// Select flavor of particle
// 0: all
// 4: charm and beauty
// 5: beauty
    Bool_t res = 0;

    if (fFlavor == 0) {
	res = kTRUE;
    } else {
	Int_t ifl = TMath::Abs(pid/100);
	if (ifl > 10) ifl/=10;
	res = (fFlavor == ifl);
    }
//
//  This part if gamma writing is inhibited
    if (fNoGammas)
	res = res && (pid != kGamma && pid != kPi0);
//
    return res;
}

Bool_t AliGenHijing::Stable(const TParticle*  particle) const
{
// Return true for a stable particle
//

    if (particle->GetFirstDaughter() < 0 )
    {
	return kTRUE;
    } else {
	return kFALSE;
    }
}



void AliGenHijing::MakeHeader()
{
// Builds the event header, to be called after each event
    fHeader.SetNProduced(fNprimaries);
    fHeader.SetImpactParameter(fHijing->GetHINT1(19));
    fHeader.SetTotalEnergy(fHijing->GetEATT());
    fHeader.SetHardScatters(fHijing->GetJATT());
    fHeader.SetParticipants(fHijing->GetNP(), fHijing->GetNT());
    fHeader.SetCollisions(fHijing->GetN0(),
			  fHijing->GetN01(),
			  fHijing->GetN10(),
			  fHijing->GetN11());
    fHeader.SetSpectators(fProjectileSpecn, fProjectileSpecp,
			  fTargetSpecn,fTargetSpecp);
    fHeader.SetReactionPlaneAngle(fHijing->GetHINT1(20));
    fHeader.SetTrueNPart(fHijing->GetNPART());

// 4-momentum vectors of the triggered jets.
//
// Before final state gluon radiation.
    TLorentzVector* jet1 = new TLorentzVector(fHijing->GetHINT1(21),
					      fHijing->GetHINT1(22),
					      fHijing->GetHINT1(23),
					      fHijing->GetHINT1(24));

    TLorentzVector* jet2 = new TLorentzVector(fHijing->GetHINT1(31),
					      fHijing->GetHINT1(32),
					      fHijing->GetHINT1(33),
					      fHijing->GetHINT1(34));
// After final state gluon radiation.
    TLorentzVector* jet3 = new TLorentzVector(fHijing->GetHINT1(26),
					      fHijing->GetHINT1(27),
					      fHijing->GetHINT1(28),
					      fHijing->GetHINT1(29));

    TLorentzVector* jet4 = new TLorentzVector(fHijing->GetHINT1(36),
					      fHijing->GetHINT1(37),
					      fHijing->GetHINT1(38),
					      fHijing->GetHINT1(39));
    fHeader.SetJets(jet1, jet2, jet3, jet4);
// Bookkeeping for kinematic bias
    fHeader.SetTrials(fTrials);
// Event Vertex
    fHeader.SetPrimaryVertex(fVertex);
    fHeader.SetInteractionTime(fTime);

    Int_t nsd1 = 0,nsd2 = 0,ndd = 0;
    Int_t nT = fHijing->GetNT();
    Int_t nP = fHijing->GetNP();
    for (Int_t i = 1; i <= nP; ++i) {
      for (Int_t j = 1; j <= nT; ++j) {
      Int_t tp = fHijing->GetNFP(i, 5);
      Int_t tt = fHijing->GetNFT(j, 5);
      if (tp == 2)
        nsd1++;
      if (tt == 2)
        nsd2++;
      if (tp == 2 && tt == 2)
        ndd++;
      }
    }
    fHeader.SetNDiffractive(nsd1, nsd2, ndd);
    fHeader.SetFreeSpectators(fFreeProjSpecn, fFreeProjSpecp, fFreeTargSpecn, fFreeTargSpecp);
    fHeader.SetSpectatorsInTheStack(fSpectators);
    fHeader.SetDataFromFragmentation(fDataFragmentation);

    AddHeader(&fHeader);
    fCollisionGeometry = &fHeader;
}


Bool_t AliGenHijing::CheckTrigger()
{
// Check the kinematic trigger condition
//
    Bool_t   triggered = kFALSE;

    if (fTrigger == 1) {
//
//  jet-jet Trigger

	TLorentzVector* jet1 = new TLorentzVector(fHijing->GetHINT1(26),
						  fHijing->GetHINT1(27),
						  fHijing->GetHINT1(28),
						  fHijing->GetHINT1(29));

	TLorentzVector* jet2 = new TLorentzVector(fHijing->GetHINT1(36),
						  fHijing->GetHINT1(37),
						  fHijing->GetHINT1(38),
						  fHijing->GetHINT1(39));
	Double_t eta1      = jet1->Eta();
	Double_t eta2      = jet2->Eta();
	Double_t phi1      = jet1->Phi();
	Double_t phi2      = jet2->Phi();
//    printf("\n Trigger: %f %f %f %f",
//	   fEtaMinJet, fEtaMaxJet, fPhiMinJet, fPhiMaxJet);
	if (
	    (eta1 < fEtaMaxJet && eta1 > fEtaMinJet &&
	     phi1 < fPhiMaxJet && phi1 > fPhiMinJet)
	    ||
	    (eta2 < fEtaMaxJet && eta2 > fEtaMinJet &&
	     phi2 < fPhiMaxJet && phi2 > fPhiMinJet)
	    )
	    triggered = kTRUE;
    } else if (fTrigger == 2) {
//  Gamma Jet
//
	Int_t np = fParticles.GetEntriesFast();
	for (Int_t i = 0; i < np; i++) {
	    TParticle* part = (TParticle*) fParticles.At(i);
	    Int_t kf = part->GetPdgCode();
	    Int_t ksp = part->GetUniqueID();
	    if (kf == 22 && ksp == 40) {
		Float_t phi = part->Phi();
		Float_t eta = part->Eta();
		if  (eta < fEtaMaxJet &&
		     eta > fEtaMinJet &&
		     phi < fPhiMaxJet &&
		     phi > fPhiMinJet) {
		    triggered = 1;
		    break;
		} // check phi,eta within limits
	    } // direct gamma ?
	} // particle loop
    } // fTrigger == 2
    return triggered;
}

Int_t AliGenHijing::FreeSpectatorsn(Float_t b)
{
// Select no. of spectator neutrons to put on the stack
  /*Float_t corr = fFragmNeutrons->Eval(b);
  Float_t nSpecInStack = corr*nSpecn;*/ //old approach

  float nave = fFragmNeutrons->Eval(b);
  float sigma = fFragmNWidth->Eval(b);
  float nSpecInStack = gRandom->Gaus(nave, 0.68*sigma*nave);
  if(nave<0 || nSpecInStack<0) nSpecInStack=0;

  return (int) nSpecInStack;
}

Int_t AliGenHijing::FreeSpectatorsp(Float_t b)
{
// Select no. of spectator protons to put on the stack
  /*Double_t corr = fFragmProtons->Eval(b);
  Float_t nSpecInStack = corr*nSpecp;*/ //old approach

  Float_t pave = fFragmProtons->Eval(b);
  Float_t sigma = fFragmPWidth->Eval(b);
  //take into account roughly a 70% acceptance for spectator protons
  float nSpecInStack = gRandom->Gaus(pave, 0.68*sigma*pave)/0.7;
  if(pave<0 || nSpecInStack<0) nSpecInStack=0;

  return (int) nSpecInStack;
}
