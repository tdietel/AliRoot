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


// Class to read events from external (TNtuple) file
// Events -> neutron removal by EM dissociation of Pb nuclei
// Data from RELDIS code (by I. Pshenichnov)
// I. A. Pshenichnov, J. P. Bondorf, I. N. Mishustin, A. Ventura, and S. Masetti
// Phys. Rev. C 64, 024903 – Published 13 July 2001
// 
#include <TFile.h>
#include <TParticle.h>
#include <TTree.h>
#include <TVirtualMC.h>
#include <TDatabasePDG.h>
#include <TPDGCode.h>
#include "AliGenReadersEMD.h"
#include "AliStack.h"


ClassImp(AliGenReadersEMD)

AliGenReadersEMD::AliGenReadersEMD():
    fStartEvent(0),
    fNcurrent(0),
    fNparticle(0),
    fTreeNtuple(0),
    fPcToTrack(2),
    fNneu(0),
    fEneu(0),
    fPxfrag(0),
    fPyfrag(0),
    fPzfrag(0),
    fAfrag(0),
    fZfrag(0),
    fNpro(0),
    fEpro(0),
    fNtupleName(0),
    fInvertPz(kFALSE)
{
// Std constructor
    for(int i=0; i<70; i++){
       fPxneu[i] = fPyneu[i] = fPzneu[i] = 0.;
       if(i<50){
         fPxpro[i] = fPypro[i] = fPzpro[i] = 0.;
       }
    }
    //
    if(fPcToTrack==kAll) printf("\n\t   *** AliGenReadersEMD will track n, p and fragments \n\n");
    else if(fPcToTrack==kNucleons) printf("\n\t   *** AliGenReadersEMD will track all nucleons\n\n");
    else if(fPcToTrack==kOnlyNeutrons) printf("\n\t   *** AliGenReadersEMD will track only neutrons\n\n");
}


AliGenReadersEMD::AliGenReadersEMD(const AliGenReadersEMD &reader):
    AliGenReader(reader),
    fStartEvent(0),
    fNcurrent(0),
    fNparticle(0),
    fTreeNtuple(0),
    fPcToTrack(2),
    fNneu(0),
    fEneu(0),
    fPxfrag(0),
    fPyfrag(0),
    fPzfrag(0),
    fAfrag(0),
    fZfrag(0),
    fNpro(0),
    fEpro(0),
    fNtupleName(0),
    fInvertPz(kFALSE)
{
    // Copy Constructor
    for(int i=0; i<70; i++){
       fPxneu[i] = fPyneu[i] = fPzneu[i] = 0.;
       if(i<50){
         fPxpro[i] = fPypro[i] = fPzpro[i] = 0.;
       }
    }
    reader.Copy(*this);
}
  // -----------------------------------------------------------------------------------
AliGenReadersEMD::~AliGenReadersEMD()
{
    delete fTreeNtuple;
}

// -----------------------------------------------------------------------------------
AliGenReadersEMD& AliGenReadersEMD::operator=(const  AliGenReadersEMD& rhs)
{
// Assignment operator
    rhs.Copy(*this);
    return *this;
}

// -----------------------------------------------------------------------------------
void AliGenReadersEMD::Copy(TObject&) const
{
    //
    // Copy
    //
    Fatal("Copy","Not implemented!\n");
}

// -----------------------------------------------------------------------------------
void AliGenReadersEMD::Init()
{
//
// Reset the existing file environment and open a new root file

    TFile *pFile=0;
    if (!pFile) {
            pFile = TFile::Open(fFileName);
	    pFile->cd();
	    printf("\n %s file opened to read RELDIS EMD events\n\n", fFileName);
    }
    fTreeNtuple = (TTree*)gDirectory->Get(fNtupleName.Data());
    if (!fTreeNtuple) {
       printf("\n AliGenReadersEMD -> no ntuple named %s found in file %s\n\n", fNtupleName.Data(), fFileName);
       return;
    }
    fNcurrent = fStartEvent;

    TTree *Ntu=fTreeNtuple;
    //
    // Set branch addresses
    // **** neutrons
    Ntu->SetBranchAddress("N_na50",&fNneu);
    Ntu->SetBranchAddress("E_na50",&fEneu);
    Ntu->SetBranchAddress("Pxna50",  fPxneu);
    Ntu->SetBranchAddress("Pyna50",  fPyneu);
    Ntu->SetBranchAddress("Pzna50",  fPzneu);
    Ntu->SetBranchAddress("Px_mhfrag", &fPxfrag);
    Ntu->SetBranchAddress("Py_mhfrag", &fPyfrag);
    Ntu->SetBranchAddress("Pz_mhfrag", &fPzfrag);
    Ntu->SetBranchAddress("A_mhfrag", &fAfrag);
    Ntu->SetBranchAddress("Z_mhfrag", &fZfrag);
    Ntu->SetBranchAddress("N_na50_p",&fNpro);
    Ntu->SetBranchAddress("E_na50_p",&fEpro);
    Ntu->SetBranchAddress("Pxna50_p",  fPxpro);
    Ntu->SetBranchAddress("Pyna50_p",  fPypro);
    Ntu->SetBranchAddress("Pzna50_p",  fPzpro);
}

// -----------------------------------------------------------------------------------
Int_t AliGenReadersEMD::NextEvent()
{
    // Read the next event
    Int_t nTracks=0;
    fNparticle = 0;

    TFile* pFile = fTreeNtuple->GetCurrentFile();
    pFile->cd();

    Int_t nentries = (Int_t) fTreeNtuple->GetEntries();
    if(fNcurrent < nentries) {
	fTreeNtuple->GetEvent(fNcurrent);
	if(fNcurrent%100 == 0) printf("\n *** Reading event %d ***\n",fNcurrent);
	//
	if(fPcToTrack==kAll){ // all
	   nTracks = fNneu+fNpro+1;
	}
	else if(fPcToTrack==kNucleons){ // nucleons
	   nTracks = fNneu+fNpro;
	}
	else if(fPcToTrack==kOnlyNeutrons){ // neutrons
	   nTracks = fNneu;
	}
	fNcurrent++;
	printf("\t #### Putting %d particles in the stack\n", nTracks);
	return nTracks;
    }

    return 0;
}

// -----------------------------------------------------------------------------------
TParticle* AliGenReadersEMD::NextParticle()
{
    // Read the next particle
    Double_t p[4]={0.,0.,0.,0.};
    int pdgCode=0;

    if(fNparticle<fNneu){
        p[0] = fPxneu[fNparticle];
        p[1] = fPyneu[fNparticle];
        p[2] = fPzneu[fNparticle];
	pdgCode = 2112;
//    printf(" pc%d n: PDG code %d,  momentum (%f, %f, %f) \n", fNparticle, pdgCode, p[0],p[1],p[2]);
    }

    if(fPcToTrack==kAll || fPcToTrack==kNucleons){
      if(fNparticle>=fNneu && fNparticle<(fNneu+fNpro)){
        p[0] = fPxpro[fNparticle-fNneu];
        p[1] = fPypro[fNparticle-fNneu];
        p[2] = fPzpro[fNparticle-fNneu];
	pdgCode = 2212;
//    printf(" pc%d p: PDG code %d,  momentum (%f, %f, %f) \n", fNparticle, pdgCode, p[0],p[1],p[2]);
      }
    }
    if(fPcToTrack==kAll){
      if(fNparticle>=(fNneu+fNpro)){
        p[0] = fPxfrag;
        p[1] = fPyfrag;
        p[2] = fPzfrag;
	pdgCode = 1e10+1e6*fZfrag+10.*fAfrag;
//    printf(" pc%d fragment: PDG code %d,  momentum (%f, %f, %f) \n", fNparticle, pdgCode, p[0],p[1],p[2]);
      }
    }

    if(fInvertPz){
      p[2] = - p[2];
    }

    Double_t ptot = TMath::Sqrt(p[0]*p[0]+p[1]*p[1]+p[2]*p[2]);
    Double_t amass = TDatabasePDG::Instance()->GetParticle(pdgCode)->Mass();
    p[3] = TMath::Sqrt(ptot*ptot+amass*amass);

    if(p[3]<=amass){
       Warning("Generate","Particle %d  E = %f GeV mass = %f GeV ",pdgCode,p[3],amass);
    }

    // printf("  Pc %d:  PDGcode %d  p(%e, %e, %e, %e) M = %e\n",
    //	   fNparticle,pdgCode,p[0], p[1], p[2], p[3], amass );
    TParticle* particle = new TParticle(pdgCode, 0, -1, -1, -1, -1,
    	p[0], p[1], p[2], p[3], 0., 0., 0., 0.);
    if((p[0]*p[0]+p[1]*p[1]+p[2]*p[2])>1e-5) particle->SetBit(kTransportBit);

    //    printf("Check: %e\n", TMath::Sqrt( particle->Energy()*particle->Energy() - particle->P()*particle->P() ));

    
    fNparticle++;
    return particle;
}

//___________________________________________________________
void AliGenReadersEMD::RewindEvent()
{
  // Go back to the first particle of the event
  fNparticle = 0;
}
