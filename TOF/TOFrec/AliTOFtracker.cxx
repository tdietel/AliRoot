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

//--------------------------------------------------------------------//
//                                                                    //
// AliTOFtracker Class                                                //
// Task: Perform association of the ESD tracks to TOF Clusters        //
// and Update ESD track with associated TOF Cluster parameters        //
//                                                                    //
// -- Authors : S. Arcelli, C. Zampolli (Bologna University and INFN) //
// -- Contacts: Annalisa.De.Caro@cern.ch                              //
// --         : Chiara.Zampolli@bo.infn.it                            //
// --         : Silvia.Arcelli@bo.infn.it                             //
//                                                                    //
//--------------------------------------------------------------------//

#include <Rtypes.h>
#include <TROOT.h>

#include <TSeqCollection.h>
#include <TClonesArray.h>
#include <TObjArray.h>
#include <TGeoManager.h>
#include <TTree.h>
#include <TFile.h>
#include <TH2F.h>

#include "AliGeomManager.h"
#include "AliESDtrack.h"
#include "AliESDEvent.h"
#include "AliESDpid.h"
#include "AliLog.h"
#include "AliTrackPointArray.h"
#include "AliCDBManager.h"

//#include "AliTOFpidESD.h"
#include "AliTOFRecoParam.h"
#include "AliTOFReconstructor.h"
#include "AliTOFcluster.h"
#include "AliTOFGeometry.h"
#include "AliTOFtracker.h"
#include "AliTOFtrack.h"
#include "TTreeStream.h"

extern TGeoManager *gGeoManager;
//extern TROOT *gROOT;


ClassImp(AliTOFtracker)

//_____________________________________________________________________________
AliTOFtracker::AliTOFtracker():
  fDebugStreamer(0x0),
  fkRecoParam(0x0),
  fGeom(0x0),
  fN(0),
  fNseeds(0),
  fNseedsTOF(0),
  fngoodmatch(0),
  fnbadmatch(0),
  fnunmatch(0),
  fnmatch(0),
  fESDEv(0),
  fTracks(new TClonesArray("AliTOFtrack")),
  fSeeds(new TObjArray(100)),
  fTOFtrackPoints(new TObjArray(10)),
  fHDigClusMap(0x0),
  fHDigNClus(0x0),
  fHDigClusTime(0x0),
  fHDigClusToT(0x0),
  fHRecNClus(0x0),
  fHRecDist(0x0),
  fHRecSigYVsP(0x0),
  fHRecSigZVsP(0x0),
  fHRecSigYVsPWin(0x0),
  fHRecSigZVsPWin(0x0),
  fCalTree(0x0),
  fIch(-1),
  fToT(-1.),
  fTime(-1.),
  fExpTimePi(-1.),
  fExpTimeKa(-1.),
  fExpTimePr(-1.),
  fNTOFmatched(0)
{
  //AliTOFtracker main Ctor

  for (Int_t ii=0; ii<kMaxCluster; ii++) fClusters[ii]=0x0;

  // Getting the geometry
  fGeom = new AliTOFGeometry();
  /* RS?
  for(Int_t i=0; i< 20000;i++){
    fClusterESD[i] = NULL;
    fHit[i] = NULL;
  }
  */
  InitCheckHists();
  if (AliTOFReconstructor::StreamLevel()>0) {
    fDebugStreamer = new TTreeSRedirector("TOFdebug.root","recreate");
    AliTOFReconstructor::SetDebugStreamer(fDebugStreamer);
  }

}
//_____________________________________________________________________________
AliTOFtracker::~AliTOFtracker() {
  //
  // Dtor
  //

  SaveCheckHists();

  if(!(AliCDBManager::Instance()->GetCacheFlag())){
    delete fkRecoParam;
  }
  delete fGeom; 
  delete fHDigClusMap;
  delete fHDigNClus;
  delete fHDigClusTime;
  delete fHDigClusToT;
  delete fHRecNClus;
  delete fHRecDist;
  delete fHRecSigYVsP;
  delete fHRecSigZVsP;
  delete fHRecSigYVsPWin;
  delete fHRecSigZVsPWin;
  delete fCalTree;
  if (fTracks){
    fTracks->Delete();
    delete fTracks;
    fTracks=0x0;
  }
  if (fSeeds){
    fSeeds->Delete();
    delete fSeeds;
    fSeeds=0x0;
  }
  if (fTOFtrackPoints){
    fTOFtrackPoints->Delete();
    delete fTOFtrackPoints;
    fTOFtrackPoints=0x0;
  }

  for (Int_t ii=0; ii<kMaxCluster; ii++)
    if (fClusters[ii]) fClusters[ii]->Delete();

  /* RS?
     for(Int_t i=0; i< 20000;i++){
     if(fClusterESD[i]){
     delete fClusterESD[i];
     fClusterESD[i] = NULL;
     }
     if(fHit[i]){
     delete fHit[i];
     fHit[i] = NULL;
     }
     }
  */
  if (fDebugStreamer) delete fDebugStreamer;

}
//_____________________________________________________________________________
void AliTOFtracker::GetPidSettings(AliESDpid *esdPID) {
  // 
  // Sets TOF resolution from RecoParams
  //
  if (fkRecoParam)
    esdPID->GetTOFResponse().SetTimeResolution(fkRecoParam->GetTimeResolution());
  else
    AliWarning("fkRecoParam not yet set; cannot set PID settings");
} 
//_____________________________________________________________________________
Int_t AliTOFtracker::PropagateBack(AliESDEvent * const event) {
  //
  // Gets seeds from ESD event and Match with TOF Clusters
  //
  fESDEv = event;
  //
  if (fN==0) {
    AliInfo("No TOF recPoints to be matched with reconstructed tracks");
    return 0;
  }

  // initialize RecoParam for current event
  AliDebug(1,"Initializing params for TOF");

  fkRecoParam = AliTOFReconstructor::GetRecoParam();  // instantiate reco param from STEER...

  if (fkRecoParam == 0x0) { 
    AliFatal("No Reco Param found for TOF!!!");
  }
  //fkRecoParam->Dump();
  //if(fkRecoParam->GetApplyPbPbCuts())fkRecoParam=fkRecoParam->GetPbPbparam();
  //fkRecoParam->PrintParameters();

  /* RS?
  // create clusters from hit
  for(Int_t i=0;i < fNTOFmatched;i++){
    fClusterESD[i] = new AliESDTOFCluster(fHit[i],event);
    fClusterESD[i]->SetStatus(fClusters[i]->GetStatus());
  }
  */
  //Initialise some counters

  fNseeds=0;
  fNseedsTOF=0;
  fngoodmatch=0;
  fnbadmatch=0;
  fnunmatch=0;
  fnmatch=0;

  Int_t ntrk=event->GetNumberOfTracks();
  fNseeds = ntrk;


  //Load ESD tracks into a local Array of ESD Seeds
  for (Int_t i=0; i<fNseeds; i++){
    fSeeds->AddLast(event->GetTrack(i));
    //    event->GetTrack(i)->SetESDEvent(event); // RS: Why this is needed? The event is already set
  }
  //Prepare ESD tracks candidates for TOF Matching
  CollectESD();

  if (fNseeds==0 || fNseedsTOF==0) {
    AliInfo("No seeds to try TOF match");
    fSeeds->Clear();
    fTracks->Clear();
    return 0 ;
  }

  //First Step with Strict Matching Criterion
  MatchTracks(0);

  //Second Step with Looser Matching Criterion
  MatchTracks(1);

  //Third Step without kTOFout flag (just to update clusters)
  MatchTracks(2);

  //RS?  event->SetTOFcluster(fNTOFmatched,fClusterESD); 
 
  if (fN==0) {
    AliInfo("No TOF recPoints to be matched with reconstructed tracks");
    fSeeds->Clear();
    fTracks->Clear();
    return 0;
  }

  AliInfo(Form("Number of matched tracks = %d (good = %d, bad = %d)",fnmatch,fngoodmatch,fnbadmatch));

  //Update the matched ESD tracks

  for (Int_t i=0; i<ntrk; i++) {
    // RS: This is a bogus code since t and seed are the same object
    //    AliESDtrack *t=event->GetTrack(i);
    AliESDtrack *seed =(AliESDtrack*)fSeeds->At(i);
    if ( (seed->GetStatus()&AliESDtrack::kTOFin)!=0 ) {
      //t->SetStatus(AliESDtrack::kTOFin);
      //if(seed->GetTOFsignal()>0){
      if ( (seed->GetStatus()&AliESDtrack::kTOFout)!=0 ) {
	//t->SetStatus(AliESDtrack::kTOFout);
	//t->SetTOFsignal(seed->GetTOFsignal());
	//t->SetTOFcluster(seed->GetTOFcluster());
	//t->SetTOFsignalToT(seed->GetTOFsignalToT());
	//t->SetTOFsignalRaw(seed->GetTOFsignalRaw());
	//t->SetTOFsignalDz(seed->GetTOFsignalDz());
	//t->SetTOFsignalDx(seed->GetTOFsignalDx());
	//t->SetTOFDeltaBC(seed->GetTOFDeltaBC());
	//t->SetTOFL0L1(seed->GetTOFL0L1());
	//t->SetTOFCalChannel(seed->GetTOFCalChannel());
	Int_t tlab[3]; seed->GetTOFLabel(tlab);    
	//t->SetTOFLabel(tlab);

	Double_t alphaA = (Double_t)seed->GetAlpha();
	Double_t xA = (Double_t)seed->GetX();
	Double_t yA = (Double_t)seed->GetY();
	Double_t zA = (Double_t)seed->GetZ();
	Double_t p1A = (Double_t)seed->GetSnp();
	Double_t p2A = (Double_t)seed->GetTgl();
	Double_t p3A = (Double_t)seed->GetSigned1Pt();
	const Double_t *covA = (Double_t*)seed->GetCovariance();

	// Make attention, please:
	//      AliESDtrack::fTOFInfo array does not be stored in the AliESDs.root file
	//      it is there only for a check during the reconstruction step.
	Float_t info[10]; seed->GetTOFInfo(info);
	//t->SetTOFInfo(info);
	AliDebug(3,Form(" distance=%f; residual in the pad reference frame: dX=%f, dZ=%f", info[0],info[1],info[2]));

	// Check done:
	//       by calling the AliESDtrack::UpdateTrackParams,
	//       the current track parameters are changed
	//       and it could cause refit problems.
	//       We need to update only the following track parameters:
        //            the track length and expected times.
	//       Removed AliESDtrack::UpdateTrackParams call
	//       Called AliESDtrack::SetIntegratedTimes(...) and
	//       AliESDtrack::SetIntegratedLength() routines.
	/*
	  AliTOFtrack *track = new AliTOFtrack(*seed);
	  t->UpdateTrackParams(track,AliESDtrack::kTOFout); // to be checked - AdC
	  delete track;
	  Double_t time[AliPID::kSPECIESC]; t->GetIntegratedTimes(time);
	*/

	Double_t time[AliPID::kSPECIESC]; seed->GetIntegratedTimes(time,AliPID::kSPECIESC);
	//t->SetIntegratedTimes(time);

	//Double_t length =  seed->GetIntegratedLength();
	//t->SetIntegratedLength(length);

	Double_t alphaB = (Double_t)seed->GetAlpha();
	Double_t xB = (Double_t)seed->GetX();
	Double_t yB = (Double_t)seed->GetY();
	Double_t zB = (Double_t)seed->GetZ();
	Double_t p1B = (Double_t)seed->GetSnp();
	Double_t p2B = (Double_t)seed->GetTgl();
	Double_t p3B = (Double_t)seed->GetSigned1Pt();
	const Double_t *covB = (Double_t*)seed->GetCovariance();
	AliDebug(3,"Track params -now(before)-:");
	AliDebug(3,Form("    X: %f(%f), Y: %f(%f), Z: %f(%f) --- alpha: %f(%f)",
			xB,xA,
			yB,yA,
			zB,zA,
			alphaB,alphaA));
	AliDebug(3,Form("    p1: %f(%f), p2: %f(%f), p3: %f(%f)",
			p1B,p1A,
			p2B,p2A,
			p3B,p3A));
	AliDebug(3,Form("    cov1: %f(%f), cov2: %f(%f), cov3: %f(%f)"
			" cov4: %f(%f), cov5: %f(%f), cov6: %f(%f)"
			" cov7: %f(%f), cov8: %f(%f), cov9: %f(%f)"
			" cov10: %f(%f), cov11: %f(%f), cov12: %f(%f)"
			" cov13: %f(%f), cov14: %f(%f), cov15: %f(%f)",
			covB[0],covA[0],
			covB[1],covA[1],
			covB[2],covA[2],
			covB[3],covA[3],
			covB[4],covA[4],
			covB[5],covA[5],
			covB[6],covA[6],
			covB[7],covA[7],
			covB[8],covA[8],
			covB[9],covA[9],
			covB[10],covA[10],
			covB[11],covA[11],
			covB[12],covA[12],
			covB[13],covA[13],
			covB[14],covA[14]
			));
	AliDebug(2,Form(" TOF params: %6d  %f %f %f %f %f %6d %3d  %f",
			i,
			seed->GetTOFsignalRaw(),
			seed->GetTOFsignal(),
			seed->GetTOFsignalToT(),
			seed->GetTOFsignalDz(),
			seed->GetTOFsignalDx(),
			seed->GetTOFCalChannel(),
			seed->GetTOFcluster(),
			seed->GetIntegratedLength()));
	AliDebug(2,Form(" %f %f %f %f %f %f %f %f %f",
			time[0], time[1], time[2], time[3], time[4], time[5], time[6], time[7], time[8]));
      }
    }
  }
  /* RS?
  if(fNTOFmatched){
    Int_t *matchmap = new Int_t[fNTOFmatched];
    event->SetTOFcluster(fNTOFmatched,fClusterESD,matchmap);
    for (Int_t i=0; i<ntrk; i++) { // remapping after TOF matching selection
      AliESDtrack *t=event->GetTrack(i);
      t->ReMapTOFcluster(fNTOFmatched,matchmap);
    }

    delete[] matchmap;
  }
  */

  //Make TOF PID
  // Now done in AliESDpid
  // fPid->MakePID(event,timeZero);
  MakeGammaSeed();
  fSeeds->Clear();
  //fTracks->Delete();
  fTracks->Clear();
  return 0;
  
}
//_________________________________________________________________________
void AliTOFtracker::CollectESD() {
   //prepare the set of ESD tracks to be matched to clusters in TOF

  Int_t seedsTOF1=0;
  Int_t seedsTOF3=0;
  Int_t seedsTOF2=0;
 
  TClonesArray &aTOFTrack = *fTracks;
  for (Int_t i=0; i<fNseeds; i++) {

    AliESDtrack *t =(AliESDtrack*)fSeeds->At(i);
    if ((t->GetStatus()&AliESDtrack::kTPCout)==0)continue;

    AliTOFtrack *track = new AliTOFtrack(*t); // New
    Float_t x = (Float_t)track->GetX(); //New

    // TRD 'good' tracks
    if ( ( (t->GetStatus()&AliESDtrack::kTRDout)!=0 ) ) {

      AliDebug(1,Form(" Before propagation till inner TOF radius, ESDtrackLength=%f, TOFtrackLength=%f",t->GetIntegratedLength(),track->GetIntegratedLength()));

      // TRD 'good' tracks, already propagated at 371 cm
      if( x >= AliTOFGeometry::Rmin() ) {

	if  ( track->PropagateToInnerTOF() ) {

	  AliDebug(1,Form(" TRD propagated track till rho = %fcm."
			  " And then the track has been propagated till rho = %fcm.",
			  x, (Float_t)track->GetX()));

	  track->SetSeedIndex(i);
	  t->UpdateTrackParams(track,AliESDtrack::kTOFin);
	  new(aTOFTrack[fNseedsTOF]) AliTOFtrack(*track);
	  fNseedsTOF++;
	  seedsTOF1++;

	  AliDebug(1,Form(" After propagation till inner TOF radius, ESDtrackLength=%f, TOFtrackLength=%f",t->GetIntegratedLength(),track->GetIntegratedLength()));
	}
	delete track;

      }
      else { // TRD 'good' tracks, propagated rho<371cm

	if  ( track->PropagateToInnerTOF() ) {

	  AliDebug(1,Form(" TRD propagated track till rho = %fcm."
			  " And then the track has been propagated till rho = %fcm.",
			  x, (Float_t)track->GetX()));

	  track->SetSeedIndex(i);
	  t->UpdateTrackParams(track,AliESDtrack::kTOFin);
	  new(aTOFTrack[fNseedsTOF]) AliTOFtrack(*track);
	  fNseedsTOF++;
	  seedsTOF3++;

	  AliDebug(1,Form(" After propagation till inner TOF radius, ESDtrackLength=%f, TOFtrackLength=%f",t->GetIntegratedLength(),track->GetIntegratedLength()));
	}
	delete track;

      }
      //delete track;
    }

    else { // Propagate the rest of TPCbp

      AliDebug(1,Form(" Before propagation till inner TOF radius, ESDtrackLength=%f, TOFtrackLength=%f",t->GetIntegratedLength(),track->GetIntegratedLength()));

      if ( track->PropagateToInnerTOF() ) { 

	AliDebug(1,Form(" TPC propagated track till rho = %fcm."
			" And then the track has been propagated till rho = %fcm.",
			x, (Float_t)track->GetX()));

      	track->SetSeedIndex(i);
	t->UpdateTrackParams(track,AliESDtrack::kTOFin);
 	new(aTOFTrack[fNseedsTOF]) AliTOFtrack(*track);
	fNseedsTOF++;
	seedsTOF2++;

	AliDebug(1,Form(" After propagation till inner TOF radius, ESDtrackLength=%f, TOFtrackLength=%f",t->GetIntegratedLength(),track->GetIntegratedLength()));
      }
      delete track;
    }
  }

  AliInfo(Form("Number of TOF seeds = %d (kTRDout371 = %d, kTRDoutLess371 = %d, !kTRDout = %d)",fNseedsTOF,seedsTOF1,seedsTOF3,seedsTOF2));

  // Sort according uncertainties on track position 
  fTracks->Sort();

}

//_________________________________________________________________________
void AliTOFtracker::MatchTracks( Int_t mLastStep){

  // Parameters used/regulating the reconstruction

  //static Float_t corrLen=0.;//0.75;
  static Float_t detDepth=18.;
  static Float_t padDepth=0.5;

  const Float_t kSpeedOfLight= 2.99792458e-2; // speed of light [cm/ps]
  const Float_t kTimeOffset = 0.; // time offset for tracking algorithm [ps]

  Float_t dY=AliTOFGeometry::XPad(); 
  Float_t dZ=AliTOFGeometry::ZPad(); 

  Float_t sensRadius = fkRecoParam->GetSensRadius();
  Float_t stepSize   = fkRecoParam->GetStepSize();
  Float_t scaleFact  = fkRecoParam->GetWindowScaleFact();
  Float_t dyMax=fkRecoParam->GetWindowSizeMaxY(); 
  Float_t dzMax=fkRecoParam->GetWindowSizeMaxZ();
  Float_t dCut=fkRecoParam->GetDistanceCut();
  if (dCut==3. && fNseedsTOF<=10) {
    dCut=10.;
    AliInfo(Form("Matching window=%f, since low multiplicity event (fNseedsTOF=%d)",
		 dCut, fNseedsTOF));
  }
  if(mLastStep == 2) dCut=10.;

  if (AliTOFReconstructor::GetExtraTolerance()>0) {
    dCut += AliTOFReconstructor::GetExtraTolerance();
    AliInfoF("Extra %.2f tolerance on distance is added: dCut=%.2f",AliTOFReconstructor::GetExtraTolerance(),dCut);
  }

  Double_t maxChi2=fkRecoParam->GetMaxChi2TRD();
  Bool_t timeWalkCorr    = fkRecoParam->GetTimeWalkCorr();
  if(!mLastStep){
    AliDebug(1,"++++++++++++++TOF Reconstruction Parameters:++++++++++++");
    AliDebug(1,Form("TOF sens radius: %f",sensRadius));
    AliDebug(1,Form("TOF step size: %f",stepSize));
    AliDebug(1,Form("TOF Window scale factor: %f",scaleFact));
    AliDebug(1,Form("TOF Window max dy: %f",dyMax));
    AliDebug(1,Form("TOF Window max dz: %f",dzMax));
    AliDebug(1,Form("TOF distance Cut: %f",dCut));
    AliDebug(1,Form("TOF Max Chi2: %f",maxChi2));
    AliDebug(1,Form("Time Walk Correction? : %d",timeWalkCorr));   
  }

  //Match ESD tracks to clusters in TOF

  // Get the number of propagation steps
  Int_t nSteps=(Int_t)(detDepth/stepSize);
  AliDebug(1,Form(" Number of steps to be done %d",nSteps));

  AliDebug(1,"++++++++++++++++++++++++++++++++++++++++++++++++++++++++");

  //PH Arrays (moved outside of the loop)
  Float_t * trackPos[4];
  for (Int_t ii=0; ii<4; ii++) trackPos[ii] = new Float_t[nSteps];
  Int_t * clind = new Int_t[fN];
  
  // Some init
  const Int_t kNclusterMax = 1000; // related to fN value
  TGeoHMatrix global[kNclusterMax];

  //The matching loop
  for (Int_t iseed=0; iseed<fNseedsTOF; iseed++) {

    fTOFtrackPoints->Delete();

    for (Int_t ii=0; ii<kNclusterMax; ii++)
      global[ii].Clear();
    AliTOFtrack *track =(AliTOFtrack*)fTracks->UncheckedAt(iseed);
    AliESDtrack *t =(AliESDtrack*)fSeeds->At(track->GetSeedIndex());
    //if ( t->GetTOFsignal()>0. ) continue;
    if ( ((t->GetStatus()&AliESDtrack::kTOFout)!=0 ) && mLastStep < 2) continue;
    AliTOFtrack *trackTOFin = new AliTOFtrack(*track);

    // Determine a window around the track
    Double_t x,par[5]; 
    trackTOFin->GetExternalParameters(x,par);
    Double_t cov[15]; 
    trackTOFin->GetExternalCovariance(cov);

    if (cov[0]<0. || cov[2]<0.) {
      AliWarning(Form("Very strange track (%d)! At least one of its covariance matrix diagonal elements is negative!",iseed));
      //delete trackTOFin;
      //continue;
    }

    Double_t dphi=
      scaleFact*
      ((5*TMath::Sqrt(TMath::Abs(cov[0])) + 0.5*dY + 2.5*TMath::Abs(par[2]))/sensRadius); 
    Double_t dz=
       scaleFact*
       (5*TMath::Sqrt(TMath::Abs(cov[2])) + 0.5*dZ + 2.5*TMath::Abs(par[3]));

    Double_t phi=TMath::ATan2(par[0],x) + trackTOFin->GetAlpha();
    if (phi<-TMath::Pi())phi+=2*TMath::Pi();
    if (phi>=TMath::Pi())phi-=2*TMath::Pi();
    Double_t z=par[1];   

    //upper limit on window's size.
    if (dz> dzMax) dz=dzMax;
    if (dphi*sensRadius> dyMax) dphi=dyMax/sensRadius;


    // find the clusters in the window of the track
    Int_t nc=0;
    for (Int_t k=FindClusterIndex(z-dz); k<fN; k++) {

      if (nc>=kNclusterMax) {
 	AliWarning("No more matchable clusters can be stored! Please, increase the corresponding vectors size.");
 	break;
      }

      AliTOFcluster *c=fClusters[k];
      if (c->GetZ() > z+dz) break;
      if (c->IsUsed() && mLastStep < 2) continue;
      if (!c->GetStatus()) {
	AliDebug(1,"Cluster in channel declared bad!");
	continue; // skip bad channels as declared in OCDB
      }

      Double_t dph=TMath::Abs(c->GetPhi()-phi);
      if (dph>TMath::Pi()) dph-=2.*TMath::Pi();
      if (TMath::Abs(dph)>dphi) continue;

      Double_t yc=(c->GetPhi() - trackTOFin->GetAlpha())*c->GetR();
      Double_t p[2]={yc, c->GetZ()};
      Double_t cov2[3]= {dY*dY/12., 0., dZ*dZ/12.};
      if (trackTOFin->AliExternalTrackParam::GetPredictedChi2(p,cov2) > maxChi2)continue;

      clind[nc] = k;      
      Char_t path[200];
      Int_t ind[5];
      ind[0]=c->GetDetInd(0);
      ind[1]=c->GetDetInd(1);
      ind[2]=c->GetDetInd(2);
      ind[3]=c->GetDetInd(3);
      ind[4]=c->GetDetInd(4);
      fGeom->GetVolumePath(ind,path);
      gGeoManager->cd(path);
      global[nc] = *gGeoManager->GetCurrentMatrix();
      nc++;
    }

    if (nc == 0 ) {
      AliDebug(1,Form("No available clusters for the track number %d",iseed));
      fnunmatch++;
      delete trackTOFin;
      continue;
    }

    AliDebug(1,Form(" Number of available TOF clusters for the track number %d: %d",iseed,nc));

    //start fine propagation 

    Int_t nStepsDone = 0;
    for( Int_t istep=0; istep<nSteps; istep++){ 

      // First of all, propagate the track...
      Float_t xs = AliTOFGeometry::RinTOF()+istep*stepSize;
      if (!(trackTOFin->PropagateTo(xs))) break;

      //  ...and then, if necessary, rotate the track
      Double_t ymax = xs*TMath::Tan(0.5*AliTOFGeometry::GetAlpha());
      Double_t ysect = trackTOFin->GetY();
      if (ysect > ymax) {
	if (!(trackTOFin->Rotate(AliTOFGeometry::GetAlpha()))) break;
      } else if (ysect <-ymax) {
	if (!(trackTOFin->Rotate(-AliTOFGeometry::GetAlpha()))) break;
      }

      nStepsDone++;
      AliDebug(3,Form(" current step %d (%d) - nStepsDone=%d",istep,nSteps,nStepsDone));

      // store the running point (Globalrf) - fine propagation     

      Double_t r[3];
      trackTOFin->GetXYZ(r);
      trackPos[0][nStepsDone-1]= (Float_t) r[0];
      trackPos[1][nStepsDone-1]= (Float_t) r[1];
      trackPos[2][nStepsDone-1]= (Float_t) r[2];   
      trackPos[3][nStepsDone-1]= trackTOFin->GetIntegratedLength();
    }


#if 0
    /*****************/
    /**** OLD CODE ***/
    /*****************/

    Int_t nfound = 0;
    Bool_t accept = kFALSE;
    Bool_t isInside = kFALSE;
    for (Int_t istep=0; istep<nStepsDone; istep++) {

      Float_t ctrackPos[3];	
      ctrackPos[0] = trackPos[0][istep];
      ctrackPos[1] = trackPos[1][istep];
      ctrackPos[2] = trackPos[2][istep];

      //now see whether the track matches any of the TOF clusters            

      Float_t dist3d[3];
      accept = kFALSE;
      for (Int_t i=0; i<nc; i++) {
        isInside = fGeom->IsInsideThePad((TGeoHMatrix*)(&global[i]),ctrackPos,dist3d);

        if ( mLastStep ) {
          Float_t yLoc = dist3d[1];
          Float_t rLoc = TMath::Sqrt(dist3d[0]*dist3d[0]+dist3d[2]*dist3d[2]);
	  accept = (TMath::Abs(yLoc)<padDepth*0.5 && rLoc<dCut);
	  AliDebug(3," I am in the case mLastStep==kTRUE ");
	}
	else {
	  accept = isInside;
	}
	if (accept) {

	  fTOFtrackPoints->AddLast(new AliTOFtrackPoint(clind[i],
							TMath::Sqrt(dist3d[0]*dist3d[0] + dist3d[1]*dist3d[1] + dist3d[2]*dist3d[2]),
							dist3d[2],dist3d[0],dist3d[1],
							AliTOFGeometry::RinTOF()+istep*stepSize,trackPos[3][istep]));

	  AliDebug(3,Form(" dist3dLoc[0] = %f, dist3dLoc[1] = %f, dist3dLoc[2] = %f ",dist3d[0],dist3d[1],dist3d[2]));
	  nfound++;
	  if(accept &&!mLastStep)break;
	}//end if accept

      } //end for on the clusters
      if(accept &&!mLastStep)break;
    } //end for on the steps     

    /*****************/
    /**** OLD CODE ***/
    /*****************/
#endif

    if ( nStepsDone == 0 ) {
      AliDebug(1,Form(" No track points for the track number %d",iseed));
      fnunmatch++;
      delete trackTOFin;
      continue;
    }

    AliDebug(2,Form(" Number of steps done for the track number %d: %d",iseed,nStepsDone));

    /*****************/
    /**** NEW CODE ***/
    /*****************/

    Int_t *isClusterMatchable = NULL;
    if(nc){
      isClusterMatchable = new Int_t[nc];
      for (Int_t i=0; i<nc; i++) isClusterMatchable[i] = kFALSE;	  	
    }

    Int_t nfound = 0;
    Bool_t accept = kFALSE;
    Bool_t isInside = kFALSE;
    for (Int_t istep=0; istep<nStepsDone; istep++) {

      Bool_t gotInsideCluster = kFALSE;
      Int_t trackInsideCluster = -1;

      Float_t ctrackPos[3];     
      ctrackPos[0] = trackPos[0][istep];
      ctrackPos[1] = trackPos[1][istep];
      ctrackPos[2] = trackPos[2][istep];

      //now see whether the track matches any of the TOF clusters            

      Float_t dist3d[3]={0.,0.,0.};
      accept = kFALSE;
      for (Int_t i=0; i<nc; i++) {

        // ***** NEW *****
        /* check whether track was inside another cluster
         * and in case inhibit this cluster.
         * this will allow to only go on and add track points for
         * that cluster where the track got inside first */
        if (gotInsideCluster && trackInsideCluster != i) {
	  AliDebug(3,Form(" A - istep=%d ~ %d %d ~ nfound=%d",istep,trackInsideCluster,i,nfound));
          continue;
	}
	AliDebug(3,Form(" B - istep=%d ~ %d %d ~ nfound=%d",istep,trackInsideCluster,i,nfound));

        /* check whether track is inside this cluster */
	for (Int_t hh=0; hh<3; hh++) dist3d[hh]=0.;
	isInside = fGeom->IsInsideThePad((TGeoHMatrix*)(&global[i]),ctrackPos,dist3d);

        // ***** NEW *****
        /* if track is inside this cluster set flags which will then
         * inhibit to add track points for the other clusters */
        if (isInside) {
          gotInsideCluster = kTRUE;
          trackInsideCluster = i;
        }

        if ( mLastStep ) {
          Float_t yLoc = dist3d[1];
          Float_t rLoc = TMath::Sqrt(dist3d[0]*dist3d[0]+dist3d[2]*dist3d[2]);
          accept = (TMath::Abs(yLoc)<padDepth*0.5 && rLoc<dCut);
          AliDebug(3," I am in the case mLastStep==kTRUE ");
        }

	//***** NEW *****
	/* add point everytime that:
	 * - the track is inside the cluster
	 * - the track got inside the cluster, even when it eventually exited the cluster
	 * - the tracks is within dCut from the cluster
	 */
        if (accept || isInside || gotInsideCluster) {

          fTOFtrackPoints->AddLast(new AliTOFtrackPoint(clind[i],
                                                        TMath::Sqrt(dist3d[0]*dist3d[0] + dist3d[1]*dist3d[1] + dist3d[2]*dist3d[2]),
                                                        dist3d[2],dist3d[0],dist3d[1],
                                                        AliTOFGeometry::RinTOF()+istep*stepSize,trackPos[3][istep]));

          AliDebug(2,Form(" dist3dLoc[0] = %f, dist3dLoc[1] = %f, dist3dLoc[2] = %f ",dist3d[0],dist3d[1],dist3d[2]));
          nfound++;

	  AliDebug(3,Form(" C - istep=%d ~ %d %d ~ nfound=%d",istep,trackInsideCluster,i,nfound));
	  
	  // store the match in the ESD
	  if (mLastStep==2 && !isClusterMatchable[i]) { // add TOF clusters to the track
	    //
	    isClusterMatchable[i] = kTRUE;
	    //Tracking info
	    Double_t mom=t->GetP();
	    AliDebug(3,Form(" Momentum for track %d -> %f", iseed,mom));
	    Double_t time[AliPID::kSPECIESC];
	    // read from old structure (the one used by TPC in reco)
	    for(Int_t isp=0;isp<AliPID::kSPECIESC;isp++){
	      time[isp] = t->GetIntegratedTimesOld(isp); // in ps
	      Double_t mass=AliPID::ParticleMass(isp);
	      Double_t momz = mom*AliPID::ParticleCharge(isp);
	      time[isp]+=(trackPos[3][istep]-trackPos[3][0])/kSpeedOfLight*TMath::Sqrt(momz*momz+mass*mass)/momz;
	      //time[isp]+=(trackPos[3][istep]-trackPos[3][0])/kSpeedOfLight*TMath::Sqrt(mom*mom+mass*mass)/mom;
	    }
	    //
	    AliESDTOFCluster* esdTOFCl = GetESDTOFCluster(clind[i]); 
	    if(!esdTOFCl->Update(t->GetID(),dist3d[0],dist3d[1],dist3d[2],trackPos[3][istep],time))//x,y,z -> tracking RF
	      t->AddTOFcluster(esdTOFCl->GetESDID());
	  }

          // ***** NEW *****
          /* do not break loop in any case
           * if the track got inside a cluster all other clusters
           * are inhibited */
          //      if(accept &&!mLastStep)break;
          
        }//end if accept
        
      } //end for on the clusters
      
      // ***** NEW *****
      /* do not break loop in any case
      * if the track got inside a cluster all other clusters
      * are inhibited but we want to go on adding track points */
      //      if(accept &&!mLastStep)break;
      
    } //end for on the steps     
    if(nc) delete[] isClusterMatchable;

    if (nfound == 0 ) {
      AliDebug(1,Form("No track points for the track number %d",iseed));
      fnunmatch++;
      delete trackTOFin;
      continue;
    }
    
    AliDebug(1,Form(" Number of track points for the track number %d: %d",iseed,nfound));

    // now choose the cluster to be matched with the track.

    Int_t idclus=-1;
    Float_t  recL = 0.;
    Float_t  xpos=0.;
    Float_t  mindist=1000.;
    Float_t  mindistZ=0.;
    Float_t  mindistY=0.;
    Float_t  mindistX=stepSize;
    for (Int_t iclus= 0; iclus<nfound;iclus++) {
      AliTOFtrackPoint *matchableTOFcluster = (AliTOFtrackPoint*)fTOFtrackPoints->At(iclus);
      if (fDebugStreamer && (AliTOFReconstructor::StreamLevel())>0){
        Float_t mindistNew = matchableTOFcluster->Distance();
        Float_t mindistYNew = matchableTOFcluster->DistanceY();
        Float_t mindistZNew = matchableTOFcluster->DistanceZ();
        Float_t mindistXNew = matchableTOFcluster->DistanceX();
        AliTOFcluster *c=fClusters[matchableTOFcluster->Index()];
        Int_t clindex=matchableTOFcluster->Index();
        (*fDebugStreamer)<<"matchNearest"<<
                          "mLastStep="<<mLastStep<<
                          "c.="<<c<<
                          "clindex="<<clindex<<
                         "nfound="<<nfound<<
                         "mindist="<<mindist<<
                         "iclus="<<iclus<<
                         "mindistNew="<<mindistNew<<
                         "mindist="<<mindist<<
                         "mindistX="<<mindistX<<
                         "mindistXNew="<<mindistXNew<<
                         "mindistY="<<mindistY<<
                         "mindistYNew="<<mindistYNew<<
                         "mindistZ="<<mindistZ<<
                         "mindistZNew="<<mindistZNew<<
                         "\n";
      }
      //if ( matchableTOFcluster->Distance()<mindist ) {

      //if ( TMath::Abs(matchableTOFcluster->DistanceX())<TMath::Abs(mindistX) &&
	   //TMath::Abs(matchableTOFcluster->DistanceX())<=stepSize ) {
  if ( TMath::Abs(matchableTOFcluster->Distance())<TMath::Abs(mindist) &&
	    TMath::Abs(matchableTOFcluster->DistanceX())<=stepSize ) {                     /// MI - Temporary BUG FIX - use 3D distance instead of the localX (radial) distance

	mindist = matchableTOFcluster->Distance();
	mindistZ = matchableTOFcluster->DistanceZ(); // Z distance in the
						     // RF of the hit pad
						     // closest to the
						     // reconstructed
						     // track
	mindistY = matchableTOFcluster->DistanceY(); // Y distance in the
						     // RF of the hit pad
						     // closest to the
						     // reconstructed
						     // track
	mindistX = matchableTOFcluster->DistanceX(); // X distance in the
						     // RF of the hit pad
						     // closest to the
						     // reconstructed
						     // track
	xpos = matchableTOFcluster->PropRadius();
        idclus = matchableTOFcluster->Index();
        recL = matchableTOFcluster->Length();// + corrLen*0.5;

	AliDebug(2,Form(" %d(%d) --- %f (%f, %f, %f), step=%f -- idclus=%d --- seed=%d, trackId=%d, trackLab=%d", iclus,nfound,
			mindist,mindistX,mindistY,mindistZ,stepSize,idclus,iseed,track->GetSeedIndex(),track->GetLabel()));

      }
    } // loop on found TOF track points

    if (TMath::Abs(mindistX)>stepSize && idclus!=-1) {
      AliInfo(Form(" %d - not matched --- but idclus=%d, trackId=%d, trackLab=%d",iseed,
		   idclus,track->GetSeedIndex(),track->GetLabel()));
      idclus=-1;
    }

    if (idclus==-1) {
      AliDebug(1,Form("Reconstructed track %d doesn't match any TOF cluster", iseed));
      fnunmatch++;
      delete trackTOFin;
      continue;
    }

    AliDebug(1,Form(" %d - matched",iseed));

    fnmatch++;

    AliTOFcluster *c=fClusters[idclus];

    AliDebug(3, Form("%7d     %7d     %10d     %10d  %10d  %10d      %7d",
		     iseed,
		     fnmatch-1,
		     TMath::Abs(trackTOFin->GetLabel()),
		     c->GetLabel(0), c->GetLabel(1), c->GetLabel(2),
		     idclus)); // AdC

    c->Use(); 

    // Track length correction for matching Step 2 
    /*
    if (mLastStep) {
      Float_t rc = TMath::Sqrt(c->GetR()*c->GetR() + c->GetZ()*c->GetZ());
      Float_t rt = TMath::Sqrt(trackPos[0][70]*trackPos[0][70]
			       +trackPos[1][70]*trackPos[1][70]
			       +trackPos[2][70]*trackPos[2][70]);
      Float_t dlt=rc-rt;
      recL=trackPos[3][70]+dlt;
    }
    */
    if (
	(c->GetLabel(0)==TMath::Abs(trackTOFin->GetLabel()))
	||
	(c->GetLabel(1)==TMath::Abs(trackTOFin->GetLabel()))
	||
	(c->GetLabel(2)==TMath::Abs(trackTOFin->GetLabel()))
	) {
      fngoodmatch++;

       AliDebug(2,Form(" track label good %5d",trackTOFin->GetLabel()));

    }
    else {
      fnbadmatch++;

      AliDebug(2,Form(" track label  bad %5d",trackTOFin->GetLabel()));

    }

    delete trackTOFin;

    //  Store quantities to be used in the TOF Calibration
    Float_t tToT=AliTOFGeometry::ToTBinWidth()*c->GetToT()*1E-3; // in ns
//     t->SetTOFsignalToT(tToT);
    Float_t rawTime=AliTOFGeometry::TdcBinWidth()*c->GetTDCRAW()+kTimeOffset; // RAW time,in ps
//     t->SetTOFsignalRaw(rawTime);
//     t->SetTOFsignalDz(mindistZ);
//     t->SetTOFsignalDx(mindistY);
//     t->SetTOFDeltaBC(c->GetDeltaBC());
//     t->SetTOFL0L1(c->GetL0L1Latency());

    Float_t info[10] = {mindist,mindistY,mindistZ,
			0.,0.,0.,0.,0.,0.,0.};
    t->SetTOFInfo(info);
    AliDebug(3,Form(" distance=%f; residual in the pad reference frame: dX=%f, dZ=%f", info[0],info[1],info[2]));


    Int_t ind[5];
    ind[0]=c->GetDetInd(0);
    ind[1]=c->GetDetInd(1);
    ind[2]=c->GetDetInd(2);
    ind[3]=c->GetDetInd(3);
    ind[4]=c->GetDetInd(4);
    Int_t calindex = AliTOFGeometry::GetIndex(ind);
//     t->SetTOFCalChannel(calindex);

    // keep track of the track labels in the matched cluster
    Int_t tlab[3];
    tlab[0]=c->GetLabel(0);
    tlab[1]=c->GetLabel(1);
    tlab[2]=c->GetLabel(2);
    AliDebug(3,Form(" tdc time of the matched track %6d = ",c->GetTDC()));    
    Double_t tof=AliTOFGeometry::TdcBinWidth()*c->GetTDC()+kTimeOffset; // in ps
    AliDebug(3,Form(" tof time of the matched track: %f = ",tof));    
    Double_t tofcorr=tof;
    if(timeWalkCorr)tofcorr=CorrectTimeWalk(mindistZ,tof);
    AliDebug(3,Form(" tof time of the matched track, after TW corr: %f = ",tofcorr));    
    //Set TOF time signal and pointer to the matched cluster
//     t->SetTOFsignal(tofcorr);
    t->SetTOFcluster(idclus); // pointing to the recPoints tree

    AliDebug(3,Form(" Setting TOF raw time: %f, z distance: %f  corrected time: %f ",rawTime,mindistZ,tofcorr));

    //Tracking info
    Double_t time[AliPID::kSPECIESC];
    // read from old structure (the one used by TPC in reco)
    for(Int_t isp=0;isp<AliPID::kSPECIESC;isp++){
      time[isp] = t->GetIntegratedTimesOld(isp); // in ps
    }
    Double_t mom=t->GetP();
    AliDebug(3,Form(" Momentum for track %d -> %f", iseed,mom));
    for (Int_t j=0;j<AliPID::kSPECIESC;j++) {
      Double_t mass=AliPID::ParticleMass(j);
      Double_t momz = mom*AliPID::ParticleCharge(j);
      time[j]+=(recL-trackPos[3][0])/kSpeedOfLight*TMath::Sqrt(momz*momz+mass*mass)/momz;
      //time[j]+=(recL-trackPos[3][0])/kSpeedOfLight*TMath::Sqrt(mom*mom+mass*mass)/mom;
    }

    AliTOFtrack *trackTOFout = new AliTOFtrack(*t); 
    if (!(trackTOFout->PropagateTo(xpos))) {
      delete trackTOFout;
      continue;
    }

    // If necessary, rotate the track
    Double_t yATxposMax=xpos*TMath::Tan(0.5*AliTOFGeometry::GetAlpha());
    Double_t yATxpos=trackTOFout->GetY();
    if (yATxpos > yATxposMax) {
      if (!(trackTOFout->Rotate(AliTOFGeometry::GetAlpha()))) {
	delete trackTOFout;
	continue;
      }
    } else if (yATxpos < -yATxposMax) {
      if (!(trackTOFout->Rotate(-AliTOFGeometry::GetAlpha()))) {
	delete trackTOFout;
	continue;
      }
    }

    // Fill the track residual histograms and update track only if in the first two step (0 and 1)
    if(mLastStep < 2){
      FillResiduals(trackTOFout,c,kFALSE);

      t->UpdateTrackParams(trackTOFout,AliESDtrack::kTOFout);

// don't update old structure with TOF info
//       t->SetIntegratedLength(recL);
//       t->SetIntegratedTimes(time);
//       t->SetTOFLabel(tlab);
 
      // add tof cluster to the track also for step 2
      AliESDTOFCluster* esdTOFCl = GetESDTOFCluster(idclus); 
      esdTOFCl->Update(t->GetID(),mindistY,mindist,mindistZ,recL,time);
      t->AddTOFcluster(esdTOFCl->GetESDID());
      /* RS?
      if(idclus < 20000){
	fClusterESD[idclus]->Update(t->GetID(),mindistY,mindist,mindistZ,recL,time);//x,y,z -> tracking RF
	
	t->AddTOFcluster(idclus);
      }
      else{
	AliInfo("Too many TOF clusters matched with tracks (> 20000)");
      }
      */
    }
    // Fill Reco-QA histos for Reconstruction
    fHRecNClus->Fill(nc);
    fHRecDist->Fill(mindist);
    if (cov[0]>=0.)
      fHRecSigYVsP->Fill(mom,TMath::Sqrt(cov[0]));
    else
      fHRecSigYVsP->Fill(mom,-TMath::Sqrt(-cov[0]));
    if (cov[2]>=0.)
      fHRecSigZVsP->Fill(mom,TMath::Sqrt(cov[2]));
    else
      fHRecSigZVsP->Fill(mom,-TMath::Sqrt(-cov[2]));
    fHRecSigYVsPWin->Fill(mom,dphi*sensRadius);
    fHRecSigZVsPWin->Fill(mom,dz);

    // Fill Tree for on-the-fly offline Calibration

    if ( !((t->GetStatus() & AliESDtrack::kTIME)==0 ) ) {
      fIch=calindex;
      fToT=tToT;
      fTime=rawTime;
      fExpTimePi=time[2];
      fExpTimeKa=time[3];
      fExpTimePr=time[4];
      fCalTree->Fill();
    }
    delete trackTOFout;
  }
  for (Int_t i=0; i<fNTOFmatched;i++) {
    AliESDTOFCluster *tofCl = GetESDTOFCluster(i);
    if (tofCl == NULL) {
      AliDebug(4, "No cluster associated");
    }
  }
  for (Int_t ii=0; ii<4; ii++) delete [] trackPos[ii];
  delete [] clind;
 
}
//_________________________________________________________________________
Int_t AliTOFtracker::LoadClusters(TTree *cTree) {
  //--------------------------------------------------------------------
  //This function loads the TOF clusters
  //--------------------------------------------------------------------

  Int_t npadX = AliTOFGeometry::NpadX();
  Int_t npadZ = AliTOFGeometry::NpadZ();
  Int_t nStripA = AliTOFGeometry::NStripA();
  Int_t nStripB = AliTOFGeometry::NStripB();
  Int_t nStripC = AliTOFGeometry::NStripC();

  TBranch *branch=cTree->GetBranch("TOF");
  if (!branch) { 
    AliError("can't get the branch with the TOF clusters !");
    return 1;
  }

  static TClonesArray dummy("AliTOFcluster",10000);
  dummy.Clear();
  TClonesArray *clusters=&dummy;
  branch->SetAddress(&clusters);

  cTree->GetEvent(0);
  Int_t nc=clusters->GetEntriesFast();
  fHDigNClus->Fill(nc);

  AliInfo(Form("Number of clusters: %d",nc));

  fN = 0;
  fNTOFmatched = 0;

  for (Int_t i=0; i<nc; i++) {
    AliTOFcluster *c=(AliTOFcluster*)clusters->UncheckedAt(i);
//PH    fClusters[i]=new AliTOFcluster(*c); fN++;

    if (!c->Misalign()) AliWarning("Can't misalign this cluster !"); // RS

    fClusters[i]=c; fN++;
    c->SetESDID(-1);
  // Fill Digits QA histos
 
    Int_t isector = c->GetDetInd(0);
    Int_t iplate = c->GetDetInd(1);
    Int_t istrip = c->GetDetInd(2);
    Int_t ipadX = c->GetDetInd(4);
    Int_t ipadZ = c->GetDetInd(3);

    Float_t time =(AliTOFGeometry::TdcBinWidth()*c->GetTDC())*1E-3; // in ns
    Float_t tot = (AliTOFGeometry::TdcBinWidth()*c->GetToT())*1E-3;//in ns

    /* RS?
    Int_t ind[5];
    ind[0]=isector;
    ind[1]=iplate;
    ind[2]=istrip;
    ind[3]=ipadZ;
    ind[4]=ipadX;
    Int_t calindex = AliTOFGeometry::GetIndex(ind);
    Int_t tofLabels[3]={c->GetLabel(0),c->GetLabel(1),c->GetLabel(2)};
    */
    Int_t stripOffset = 0;
    switch (iplate) {
    case 0:
      stripOffset = 0;
      break;
    case 1:
      stripOffset = nStripC;
      break;
    case 2:
      stripOffset = nStripC+nStripB;
      break;
    case 3:
      stripOffset = nStripC+nStripB+nStripA;
      break;
    case 4:
      stripOffset = nStripC+nStripB+nStripA+nStripB;
      break;
    default:
      AliError(Form("Wrong plate number in TOF (%d) !",iplate));
      break;
    };
    Int_t zindex=npadZ*(istrip+stripOffset)+(ipadZ+1);
    Int_t phiindex=npadX*isector+ipadX+1;
    fHDigClusMap->Fill(zindex,phiindex);
    fHDigClusTime->Fill(time);
    fHDigClusToT->Fill(tot);
    //AliESDTOFCluster * tofCl=GetESDTOFCluster(fNTOFmatched);
    //if (tofCl==NULL){
    //  AliDebug(4,"No cluster associated");
    //}
    fNTOFmatched++; // RS: Actually number of clusters
    /* RS?
    if(fNTOFmatched < 20000){
      fHit[fNTOFmatched] = new AliESDTOFHit(AliTOFGeometry::TdcBinWidth()*c->GetTDC(),
					    AliTOFGeometry::TdcBinWidth()*c->GetTDCRAW(),
					    AliTOFGeometry::ToTBinWidth()*c->GetToT()*1E-3,
					    calindex,tofLabels,c->GetL0L1Latency(),
					    c->GetDeltaBC(),i,c->GetZ(),c->GetR(),c->GetPhi());
      fNTOFmatched++;
    }
    */
  }

  return 0;
}
//_________________________________________________________________________
void AliTOFtracker::UnloadClusters() {
  //--------------------------------------------------------------------
  //This function unloads TOF clusters
  //--------------------------------------------------------------------
  for (Int_t i=0; i<fN; i++) {
//PH    delete fClusters[i];
    fClusters[i] = 0x0;
  }
  /* RS
  for(Int_t i=0; i< 20000;i++){
    if(fClusterESD[i]){
      delete fClusterESD[i];
      fClusterESD[i] = NULL;
    }
    if(fHit[i]){
      delete fHit[i];
      fHit[i] = NULL;
    }
  }
  */
  fN=0;
  fNTOFmatched = 0;
}

//_________________________________________________________________________
Int_t AliTOFtracker::FindClusterIndex(Double_t z) const {
  //--------------------------------------------------------------------
  // This function returns the index of the nearest cluster 
  //--------------------------------------------------------------------
  if (fN==0) return 0;
  if (z <= fClusters[0]->GetZ()) return 0;
  if (z > fClusters[fN-1]->GetZ()) return fN;
  Int_t b=0, e=fN-1, m=(b+e)/2;
  for (; b<e; m=(b+e)/2) {
    if (z > fClusters[m]->GetZ()) b=m+1;
    else e=m; 
  }
  return m;
}

//_________________________________________________________________________
Bool_t AliTOFtracker::GetTrackPoint(Int_t index, AliTrackPoint& p) const
{
  // Get track space point with index i
  // Coordinates are in the global system
  AliTOFcluster *cl = fClusters[index];
  Float_t xyz[3];
  xyz[0] = cl->GetR()*TMath::Cos(cl->GetPhi());
  xyz[1] = cl->GetR()*TMath::Sin(cl->GetPhi());
  xyz[2] = cl->GetZ();
  Float_t phiangle = (Int_t(cl->GetPhi()*TMath::RadToDeg()/20.)+0.5)*20.*TMath::DegToRad();
  Float_t sinphi = TMath::Sin(phiangle), cosphi = TMath::Cos(phiangle);
  Float_t tiltangle = AliTOFGeometry::GetAngles(cl->GetDetInd(1),cl->GetDetInd(2))*TMath::DegToRad();
  Float_t sinth = TMath::Sin(tiltangle), costh = TMath::Cos(tiltangle);
  Float_t sigmay2 = AliTOFGeometry::XPad()*AliTOFGeometry::XPad()/12.;
  Float_t sigmaz2 = AliTOFGeometry::ZPad()*AliTOFGeometry::ZPad()/12.;
  Float_t cov[6];
  cov[0] = sinphi*sinphi*sigmay2 + cosphi*cosphi*sinth*sinth*sigmaz2;
  cov[1] = -sinphi*cosphi*sigmay2 + sinphi*cosphi*sinth*sinth*sigmaz2;
  cov[2] = -cosphi*sinth*costh*sigmaz2;
  cov[3] = cosphi*cosphi*sigmay2 + sinphi*sinphi*sinth*sinth*sigmaz2;
  cov[4] = -sinphi*sinth*costh*sigmaz2;
  cov[5] = costh*costh*sigmaz2;
  p.SetXYZ(xyz[0],xyz[1],xyz[2],cov);

  // Detector numbering scheme
  Int_t nSector = AliTOFGeometry::NSectors();
  Int_t nPlate  = AliTOFGeometry::NPlates();
  Int_t nStripA = AliTOFGeometry::NStripA();
  Int_t nStripB = AliTOFGeometry::NStripB();
  Int_t nStripC = AliTOFGeometry::NStripC();

  Int_t isector = cl->GetDetInd(0);
  if (isector >= nSector)
    AliError(Form("Wrong sector number in TOF (%d) !",isector));
  Int_t iplate = cl->GetDetInd(1);
  if (iplate >= nPlate)
    AliError(Form("Wrong plate number in TOF (%d) !",iplate));
  Int_t istrip = cl->GetDetInd(2);

  Int_t stripOffset = 0;
  switch (iplate) {
  case 0:
    stripOffset = 0;
    break;
  case 1:
    stripOffset = nStripC;
    break;
  case 2:
    stripOffset = nStripC+nStripB;
    break;
  case 3:
    stripOffset = nStripC+nStripB+nStripA;
    break;
  case 4:
    stripOffset = nStripC+nStripB+nStripA+nStripB;
    break;
  default:
    AliError(Form("Wrong plate number in TOF (%d) !",iplate));
    break;
  };

  Int_t idet = (2*(nStripC+nStripB)+nStripA)*isector +
               stripOffset +
               istrip;
  UShort_t volid = AliGeomManager::LayerToVolUID(AliGeomManager::kTOF,idet);
  p.SetVolumeID((UShort_t)volid);
  return kTRUE;
}
//_________________________________________________________________________
void AliTOFtracker::InitCheckHists() {

  //Init histos for Digits/Reco QA and Calibration


  TDirectory *dir = gDirectory;
  TFile *logFileTOF = 0;

  TSeqCollection *list = gROOT->GetListOfFiles();
  int n = list->GetEntries();
  Bool_t isThere=kFALSE;
  for(int i=0; i<n; i++) {
    logFileTOF = (TFile*)list->At(i);
    if (strstr(logFileTOF->GetName(), "TOFQA.root")){
      isThere=kTRUE;
      break;
    } 
  }

  if(!isThere)logFileTOF = new TFile( "TOFQA.root","RECREATE");
  logFileTOF->cd(); 

  fCalTree = new TTree("CalTree", "Tree for TOF calibration");
  fCalTree->Branch("TOFchannelindex",&fIch,"iTOFch/I");
  fCalTree->Branch("ToT",&fToT,"TOFToT/F");
  fCalTree->Branch("TOFtime",&fTime,"TOFtime/F");
  fCalTree->Branch("PionExpTime",&fExpTimePi,"PiExpTime/F");
  fCalTree->Branch("KaonExpTime",&fExpTimeKa,"KaExpTime/F");
  fCalTree->Branch("ProtonExpTime",&fExpTimePr,"PrExpTime/F");

  //Digits "QA" 
  fHDigClusMap = new TH2F("TOFDig_ClusMap", "",182,0.5,182.5,864, 0.5,864.5);  
  fHDigNClus = new TH1F("TOFDig_NClus", "",200,0.5,200.5);  
  fHDigClusTime = new TH1F("TOFDig_ClusTime", "",2000,0.,200.);  
  fHDigClusToT = new TH1F("TOFDig_ClusToT", "",500,0.,100);  

  //Reco "QA"
  fHRecNClus =new TH1F("TOFRec_NClusW", "",50,0.5,50.5);
  fHRecDist=new TH1F("TOFRec_Dist", "",50,0.5,10.5);
  fHRecSigYVsP=new TH2F("TOFDig_SigYVsP", "",40,0.,4.,100, 0.,5.);
  fHRecSigZVsP=new TH2F("TOFDig_SigZVsP", "",40,0.,4.,100, 0.,5.);
  fHRecSigYVsPWin=new TH2F("TOFDig_SigYVsPWin", "",40,0.,4.,100, 0.,50.);
  fHRecSigZVsPWin=new TH2F("TOFDig_SigZVsPWin", "",40,0.,4.,100, 0.,50.);

  dir->cd();

}

//_________________________________________________________________________
void AliTOFtracker::SaveCheckHists() {

  //write histos for Digits/Reco QA and Calibration

  TDirectory *dir = gDirectory;
  TFile *logFileTOF = 0;

  TSeqCollection *list = gROOT->GetListOfFiles();
  int n = list->GetEntries();
  Bool_t isThere=kFALSE;
  for(int i=0; i<n; i++) {
    logFileTOF = (TFile*)list->At(i);
    if (strstr(logFileTOF->GetName(), "TOFQA.root")){
      isThere=kTRUE;
      break;
    } 
  }
   
  if(!isThere) {
	  AliError(Form("File TOFQA.root not found!! not wring histograms...."));
	  return;
  }
  logFileTOF->cd(); 
  fHDigClusMap->Write(fHDigClusMap->GetName(), TObject::kOverwrite);
  fHDigNClus->Write(fHDigNClus->GetName(), TObject::kOverwrite);
  fHDigClusTime->Write(fHDigClusTime->GetName(), TObject::kOverwrite);
  fHDigClusToT->Write(fHDigClusToT->GetName(), TObject::kOverwrite);
  fHRecNClus->Write(fHRecNClus->GetName(), TObject::kOverwrite);
  fHRecDist->Write(fHRecDist->GetName(), TObject::kOverwrite);
  fHRecSigYVsP->Write(fHRecSigYVsP->GetName(), TObject::kOverwrite);
  fHRecSigZVsP->Write(fHRecSigZVsP->GetName(), TObject::kOverwrite);
  fHRecSigYVsPWin->Write(fHRecSigYVsPWin->GetName(), TObject::kOverwrite);
  fHRecSigZVsPWin->Write(fHRecSigZVsPWin->GetName(), TObject::kOverwrite);
  fCalTree->Write(fCalTree->GetName(),TObject::kOverwrite);
  logFileTOF->Flush();  

  dir->cd();
  }
//_________________________________________________________________________
Float_t AliTOFtracker::CorrectTimeWalk( Float_t dist, Float_t tof) const {

  //dummy, for the moment
  Float_t tofcorr=0.;
  if(dist<AliTOFGeometry::ZPad()*0.5){
    tofcorr=tof;
    //place here the actual correction
  }else{
    tofcorr=tof; 
  } 
  return tofcorr;
}
//_________________________________________________________________________

void AliTOFtracker::FillClusterArray(TObjArray* arr) const
{
  //
  // Returns the TOF cluster array
  //

  if (fN==0)
    arr = 0x0;
  else
    for (Int_t i=0; i<fN; ++i) arr->Add(fClusters[i]);

}

//_________________________________________________________________________
AliESDTOFCluster* AliTOFtracker::GetESDTOFCluster(int clID)
{
  // get ESDTOFcluster corresponding to fClusters[clID]. If the original cluster
  // was not stored yet in the ESD, first do this
  AliTOFcluster *c = fClusters[clID];
  AliESDTOFCluster *clESD = 0;
  int esdID = c->GetESDID(); // was this cluster already stored in the ESD clusters?
  TClonesArray* esdTOFClArr = fESDEv->GetESDTOFClusters();
  if (esdID<0) { // cluster was not stored yet, do this
    esdID = esdTOFClArr->GetEntriesFast();
    c->SetESDID(esdID);
    // first store the hits of the cluster
    TClonesArray* esdTOFHitArr = fESDEv->GetESDTOFHits();
    int nh = esdTOFHitArr->GetEntriesFast();
    Int_t tofLabels[3]={c->GetLabel(0),c->GetLabel(1),c->GetLabel(2)};
    Int_t ind[5] = {c->GetDetInd(0), c->GetDetInd(1), c->GetDetInd(2), c->GetDetInd(3), c->GetDetInd(4) };
    Int_t calindex = AliTOFGeometry::GetIndex(ind);
    /*AliESDTOFHit* esdHit = */ 
    new ( (*esdTOFHitArr)[nh] ) 
      AliESDTOFHit( AliTOFGeometry::TdcBinWidth()*c->GetTDC(),
		    AliTOFGeometry::TdcBinWidth()*c->GetTDCRAW(),
		    AliTOFGeometry::ToTBinWidth()*c->GetToT()*1E-3,
		    calindex,tofLabels,c->GetL0L1Latency(),
		    c->GetDeltaBC(),esdID,c->GetZ(),c->GetR(),c->GetPhi());
    //
    clESD =  new( (*esdTOFClArr)[esdID] ) AliESDTOFCluster( clID );
    clESD->SetEvent(fESDEv);
    clESD->SetStatus( c->GetStatus() );
    clESD->SetESDID(esdID);
    // 
    // register hits in the cluster
    clESD->AddESDTOFHitIndex(nh);
  }
  else clESD =  (AliESDTOFCluster*)esdTOFClArr->At(esdID); // cluster is aready stored in the ESD
  //
  return clESD;
  //
}


void AliTOFtracker::MakeGammaSeed() {
    // find the gamma candidate clusters
    const Float_t kTimeOffset = 0.; // time offset for tracking algorithm [ps]
    const AliESDVertex * vertex = fESDEv->GetPrimaryVertex();
    Int_t run=fESDEv->GetRunNumber();
    Int_t event=fESDEv->GetEventNumberInFile();
    Double_t timeStamp= fESDEv->GetTimeStampCTPBCCorr();
    ULong64_t orbitID      = (ULong64_t)fESDEv->GetOrbitNumber();
    ULong64_t bunchCrossID = (ULong64_t)fESDEv->GetBunchCrossNumber();
    ULong64_t periodID     = (ULong64_t)fESDEv->GetPeriodNumber();
    ULong64_t gid = ((periodID << 36) | (orbitID << 12) | bunchCrossID);

    Double_t xyzVertex[3];
    vertex->GetXYZ(xyzVertex);
    Int_t nc=0;
    const Float_t dPhiCut = 0.34;
    const Float_t dZCut   = 5.;
    Float_t xyz0[3], xyz1[3];
    for (Int_t i0=0; i0<fN; i0++) {
      AliTOFcluster *c0 = fClusters[i0];
      if (!c0->GetStatus()) continue;
      c0->GetGlobalXYZ(xyz0);
      Float_t phi0= TMath::ATan2(xyz0[1],xyz0[0]);
      Float_t length0=(xyz0[0]-xyzVertex[0])*(xyz0[0]-xyzVertex[0])+(xyz0[1]-xyzVertex[1])*(xyz0[1]-xyzVertex[1])+(xyz0[2]-xyzVertex[2])*(xyz0[2]-xyzVertex[2]);
      length0=TMath::Sqrt(length0);
      Double_t tof0=AliTOFGeometry::TdcBinWidth()*c0->GetTDC()+kTimeOffset; // in ps
      if (fDebugStreamer && (AliTOFReconstructor::StreamLevel() & AliTOFReconstructor::kStreamSingle)>0){
        (*fDebugStreamer)<<"hit0"<<
        "run="<<run<<
        "event="<<event<<
        "timeStamp="<<timeStamp<<
        "gid="<<gid<<
        "fNTOFmatched="<<fNTOFmatched<<
        "nhits="<<fN<<
        "c0.="<<c0<<
        "lentgh0="<<length0<<
        "tof0="<<tof0<<
        "\n";
      }
      for (Int_t i1=FindClusterIndex(c0->GetZ()-dZCut); i1<fN; i1++) {
        if (i0==i1) continue;
        AliTOFcluster *c1 = fClusters[i1];
        if (!c1->GetStatus()) continue;
        if (c1->GetZ()-c0->GetZ()>dZCut) break;
        c1->GetGlobalXYZ(xyz1);
        Float_t phi1= TMath::ATan2(xyz1[1],xyz1[0]);
        Double_t dphi=phi0-phi1;
        if (TMath::Abs(dphi) > dPhiCut) continue;
        Float_t length1=(xyz1[0]-xyzVertex[0])*(xyz1[0]-xyzVertex[0])+(xyz1[1]-xyzVertex[1])*(xyz1[1]-xyzVertex[1])+(xyz1[2]-xyzVertex[2])*(xyz1[2]-xyzVertex[2]);
        length1=TMath::Sqrt(length1);
        Double_t tof1=AliTOFGeometry::TdcBinWidth()*c1->GetTDC()+kTimeOffset; // in ps
        Double_t dist3D=(xyz1[0]-xyz0[0])*(xyz1[0]-xyz0[0])+(xyz1[1]-xyz0[1])*(xyz1[1]-xyz0[1])+(xyz1[2]-xyz0[2])*(xyz1[2]-xyz0[2]);
        dist3D=TMath::Sqrt(dist3D);
        Float_t dTime=tof1-tof0;
        AliTOFcluster *h0, *h1;
        Float_t l0=length0, l1=length1;
        Float_t t0=tof0, t1=tof1;
        if (c0->GetR()<c1->GetR()){
          h0=c0; h1=c1;
        }else{
          h0=c1; h1=c0;
          dphi*=-1;
          dTime*=-1;
          l0=length1; l1=length0;
          t0=tof0; t1=tof1;
        }
        Float_t dR=h1->GetR()-h0->GetR();
        if (fDebugStreamer && (AliTOFReconstructor::StreamLevel() & AliTOFReconstructor::kStreamV0)>0){
          (*fDebugStreamer)<<"gammaSeed"<<
            "run="<<run<<
            "event="<<event<<
            "timeStamp="<<timeStamp<<
            "gid="<<gid<<
            "fNTOFmatched="<<fNTOFmatched<<
            "nhits="<<fN<<
            "dphi="<<dphi<<
            "dR="<<dR<<
            "dist3D="<<dist3D<<
            "fTime="<<dTime<<
            "h0.="<<h0<<
            "l0="<<l0<<
            "t0="<<t0<<
            "h1.="<<h1<<
            "l1="<<l1<<
            "t1="<<t1<<
          "\n";
        }
      }

      }

}

void AliTOFtracker::SetAliasStreamer(TTree *tree) {
  tree->SetAlias("v1","l1/t1/0.03");
  tree->SetAlias("v0","l0/t0/0.03");
  tree->SetAlias("dz","h0.fZ-h1.fZ");
  tree->SetAlias("dx","h0.fX-h1.fX");
  tree->SetAlias("dy","h0.fY-h1.fY");
  tree->SetAlias("dL","sqrt(dx**2+dy**2+dz**2)");
}