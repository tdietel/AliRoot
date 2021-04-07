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




//-----------------------------------------------------------------
//
//           Implementation of the TPC seed class
//        This class is used by the AliTPCtracker class
//      Origin: Marian Ivanov, CERN, Marian.Ivanov@cern.ch
//-----------------------------------------------------------------
#include <TVectorF.h>
#include "TClonesArray.h"
#include "TGraphErrors.h"
#include "AliTPCseed.h"
#include "AliTPCReconstructor.h"
#include "AliTPCtracker.h"
#include "AliTPCClusterParam.h"
#include "AliTPCCalPad.h"
#include "AliTPCCalROC.h"
#include "AliTPCcalibDB.h"
#include "AliTPCParam.h"
#include "AliExternalTrackParam.h"
#include "AliTPCTransform.h"
#include "AliSplineFit.h"
#include "AliCDBManager.h"
#include "AliTPCcalibDButil.h"
#include <AliCTPTimeParams.h>


ClassImp(AliTPCseed)



AliTPCseed::AliTPCseed():
  AliTPCtrack(),
  fEsd(0x0),
  fNClStore(0),
  fClusterPointer(0),
  fClusterOwner(kFALSE),
  fRow(-1),
  fSector(-1),
  fRelativeSector(-1),
  fCurrentSigmaY2(1e10),
  fCurrentSigmaZ2(1e10),
  fCMeanSigmaY2p30(-1.),   //! current mean sigma Y2 - mean30%
  fCMeanSigmaZ2p30(-1.),   //! current mean sigma Z2 - mean30%
  fCMeanSigmaY2p30R(-1.),   //! current mean sigma Y2 - mean2%
  fCMeanSigmaZ2p30R(-1.),   //! current mean sigma Z2 - mean2%
  //
  fErrorY2(1e10),
  fErrorZ2(1e10),
  fErrorY2Syst(0.),
  fErrorZ2Syst(0.),
  fCurrentCluster(0x0),
  fCurrentClusterIndex1(-1),
  fNoCluster(0),
  fSort(0),
  fSeedType(0),
  fSeed1(-1),
  fSeed2(-1),
  fCircular(0),
  fMAngular(0),
  fPoolID(-1),
  fTrackPointsArr()
{
  //
  for (Int_t i=0;i<kMaxRow;i++) SetClusterIndex2(i,-3);
  for (Int_t i=0;i<3;i++)   fKinkIndexes[i]=0;
  for (Int_t i=0;i<AliPID::kSPECIES;i++)   fTPCr[i]=0.2;
  for (Int_t i=0;i<4;i++) {
    fDEDX[i] = 0.;
    fSDEDX[i] = 1e10;
    fNCDEDX[i] = 0;
    fNCDEDXInclThres[i] = 0;
  }
  for (Int_t i=0;i<9;i++) fDEDX[i] = 0;
  for (Int_t i=0;i<12;i++) fOverlapLabels[i] = -1;
  for (int i=5;i--;) fLSCovY[i] = 0;
  for (int i=3;i--;) fLSCovZ[i] = 0;

}

AliTPCseed::AliTPCseed(const AliTPCseed &s, Bool_t clusterOwner):
  AliTPCtrack(s),
  fEsd(0x0),
  fNClStore(0),
  fClusterPointer(0),
  fClusterOwner(clusterOwner),
  fRow(-1),
  fSector(-1),
  fRelativeSector(-1),
  fCurrentSigmaY2(-1),
  fCurrentSigmaZ2(-1),
  fCMeanSigmaY2p30(-1.),   //! current mean sigma Y2 - mean30%
  fCMeanSigmaZ2p30(-1.),   //! current mean sigma Z2 - mean30%
  fCMeanSigmaY2p30R(-1.),   //! current mean sigma Y2 - mean2%
  fCMeanSigmaZ2p30R(-1.),   //! current mean sigma Z2 - mean2%
  fErrorY2(1e10),
  fErrorZ2(1e10),
  fErrorY2Syst(0.),
  fErrorZ2Syst(0.),
  fCurrentCluster(0x0),
  fCurrentClusterIndex1(-1),
  fNoCluster(0),
  fSort(0),
  fSeedType(0),
  fSeed1(-1),
  fSeed2(-1),
  fCircular(0),
  fMAngular(0),
  fPoolID(-1),
  fTrackPointsArr(s.fTrackPointsArr)
{
  //---------------------
  // dummy copy constructor
  //-------------------------
  if (s.fClusterPointer) {
    fNClStore = kMaxRow;
    fClusterPointer = new AliTPCclusterMI*[kMaxRow];
    for (Int_t i=0;i<kMaxRow;i++) {
      fClusterPointer[i]=0;
      if (fClusterOwner){
	if (s.fClusterPointer[i])
	  fClusterPointer[i] = new AliTPCclusterMI(*(s.fClusterPointer[i]));
      }else{
	fClusterPointer[i] = s.fClusterPointer[i];
      }
    }
  }
  for (Int_t i=0;i<kMaxRow;i++) fIndex[i] = s.fIndex[i];
  for (Int_t i=0;i<AliPID::kSPECIES;i++)   fTPCr[i]=s.fTPCr[i];
  for (Int_t i=0;i<4;i++) {
    fDEDX[i] = s.fDEDX[i];
    fSDEDX[i] = s.fSDEDX[i];
    fNCDEDX[i] = s.fNCDEDX[i];
    fNCDEDXInclThres[i] = s.fNCDEDXInclThres[i];
  }
  for (Int_t i=0;i<9;i++) fDEDX[i] = 0;

  for (Int_t i=0;i<12;i++) fOverlapLabels[i] = s.fOverlapLabels[i];
  
  for (int i=5;i--;) fLSCovY[i] = s.fLSCovY[i];
  for (int i=3;i--;) fLSCovZ[i] = s.fLSCovZ[i];

}


AliTPCseed::AliTPCseed(const AliTPCtrack &t):
  AliTPCtrack(t),
  fEsd(0x0),
  fNClStore(0),
  fClusterPointer(0),
  fClusterOwner(kFALSE),
  fRow(-1),
  fSector(-1),
  fRelativeSector(-1),
  fCurrentSigmaY2(-1),
  fCurrentSigmaZ2(-1),
  fCMeanSigmaY2p30(-1.),   //! current mean sigma Y2 - mean30%
  fCMeanSigmaZ2p30(-1.),   //! current mean sigma Z2 - mean30%
  fCMeanSigmaY2p30R(-1.),   //! current mean sigma Y2 - mean2%
  fCMeanSigmaZ2p30R(-1.),   //! current mean sigma Z2 - mean2%
  fErrorY2(1e10),
  fErrorZ2(1e10),
  fErrorY2Syst(0.),
  fErrorZ2Syst(0.),
  fCurrentCluster(0x0),
  fCurrentClusterIndex1(-1),
  fNoCluster(0),
  fSort(0),
  fSeedType(0),
  fSeed1(-1),
  fSeed2(-1),
  fCircular(0),
  fMAngular(0),
  fPoolID(-1),
  fTrackPointsArr()
{
  //
  // Constructor from AliTPCtrack
  //
  fFirstPoint =0;
  for (Int_t i=0;i<5;i++)   fTPCr[i]=0.2;
  for (Int_t i=0;i<kMaxRow;i++) {
    Int_t index = t.GetClusterIndex(i);
    if (index>=-1){ 
      SetClusterIndex2(i,index);
    }
    else{
      SetClusterIndex2(i,-3); 
    }    
  }
  for (Int_t i=0;i<4;i++) {
    fDEDX[i] = 0.;
    fSDEDX[i] = 1e10;
    fNCDEDX[i] = 0;
    fNCDEDXInclThres[i] = 0;
  }
  for (Int_t i=0;i<9;i++) fDEDX[i] = fDEDX[i];

  for (Int_t i=0;i<12;i++) fOverlapLabels[i] = -1;
  for (int i=5;i--;) fLSCovY[i] = 0;
  for (int i=3;i--;) fLSCovZ[i] = 0;
}

AliTPCseed::AliTPCseed(Double_t xr, Double_t alpha, const Double_t xx[5],
		       const Double_t cc[15], Int_t index):      
  AliTPCtrack(xr, alpha, xx, cc, index),
  fEsd(0x0),
  fNClStore(0),
  fClusterPointer(0),
  fClusterOwner(kFALSE),
  fRow(-1),
  fSector(-1),
  fRelativeSector(-1),
  fCurrentSigmaY2(-1),
  fCurrentSigmaZ2(-1),
  fCMeanSigmaY2p30(-1.),   //! current mean sigma Y2 - mean30%
  fCMeanSigmaZ2p30(-1.),   //! current mean sigma Z2 - mean30%
  fCMeanSigmaY2p30R(-1.),   //! current mean sigma Y2 - mean2%
  fCMeanSigmaZ2p30R(-1.),   //! current mean sigma Z2 - mean2%
  fErrorY2(1e10),
  fErrorZ2(1e10),
  fErrorY2Syst(0.),
  fErrorZ2Syst(0.),
  fCurrentCluster(0x0),
  fCurrentClusterIndex1(-1),
  fNoCluster(0),
  fSort(0),
  fSeedType(0),
  fSeed1(-1),
  fSeed2(-1),
  fCircular(0),
  fMAngular(0),
  fPoolID(-1),
  fTrackPointsArr()
{
  //
  // Constructor
  //
  fFirstPoint =0;
  for (Int_t i=0;i<kMaxRow;i++) SetClusterIndex2(i,-3);
  for (Int_t i=0;i<5;i++)   fTPCr[i]=0.2;
  for (Int_t i=0;i<4;i++) {
    fDEDX[i] = 0.;
    fSDEDX[i] = 1e10;
    fNCDEDX[i] = 0;
    fNCDEDXInclThres[i] = 0;
  }
  for (Int_t i=0;i<9;i++) fDEDX[i] = 0;

  for (Int_t i=0;i<12;i++) fOverlapLabels[i] = -1;

  for (int i=5;i--;) fLSCovY[i] = 0;
  for (int i=3;i--;) fLSCovZ[i] = 0;

}

AliTPCseed::~AliTPCseed(){
  //
  // destructor
  AliDebug(5,"Destruct AliTPCseed - Begin");
  fNoCluster =0;
  if (fClusterPointer) {
    if (fClusterOwner){
      for (Int_t icluster=0; icluster<kMaxRow; icluster++){
	if(fClusterPointer[icluster]) { // shared clusters might be already deleted!
	  if (fClusterPointer[icluster]->TestBit(TObject::kNotDeleted)) delete fClusterPointer[icluster];
	  fClusterPointer[icluster]=0;
	}
      }
    }
    delete[] fClusterPointer;
  }
  AliDebug(5,"Destruct AliTPCseed - End");
}
//_________________________________________________
AliTPCseed & AliTPCseed::operator=(const AliTPCseed &param)
{
  //
  // assignment operator 
  // don't touch pool ID
  //
  if(this!=&param){
    AliTPCtrack::operator=(param);
    fTrackPointsArr = param.fTrackPointsArr;
    fEsd =param.fEsd; 
    fClusterOwner = param.fClusterOwner;
    if (param.fClusterPointer) {
      if (!fClusterPointer) {
	fClusterPointer = new AliTPCclusterMI*[kMaxRow];
	memset(fClusterPointer,0,kMaxRow*sizeof(AliTPCclusterMI*));
	fNClStore = kMaxRow;
      }
      if (!fClusterOwner) for(Int_t i = 0;i<kMaxRow;++i) fClusterPointer[i] = param.fClusterPointer[i];
      else                for(Int_t i = 0;i<kMaxRow;++i) {
	  delete fClusterPointer[i];
	  if (param.fClusterPointer[i]) fClusterPointer[i] = new AliTPCclusterMI(*(param.fClusterPointer[i]));
	  else fClusterPointer[i] = 0x0;
	}
    }
    // leave out fPoint, they are also not copied in the copy ctor...
    // but deleted in the dtor... strange...
    fRow            = param.fRow;
    fSector         = param.fSector;
    fRelativeSector = param.fRelativeSector;
    fCurrentSigmaY2 = param.fCurrentSigmaY2;
    fCurrentSigmaZ2 = param.fCurrentSigmaZ2;
    fCMeanSigmaY2p30 = param.fCMeanSigmaY2p30;
    fCMeanSigmaZ2p30 = param.fCMeanSigmaZ2p30;
    fCMeanSigmaY2p30R = param.fCMeanSigmaY2p30R;
    fCMeanSigmaZ2p30R = param.fCMeanSigmaZ2p30R;
    fErrorY2        = param.fErrorY2;
    fErrorZ2        = param.fErrorZ2;
    fErrorY2Syst    = param.fErrorY2Syst;
    fErrorZ2Syst    = param.fErrorZ2Syst;
    fCurrentCluster = param.fCurrentCluster; // this is not allocated by AliTPCSeed
    fCurrentClusterIndex1 = param.fCurrentClusterIndex1; 
    fNoCluster      = param.fNoCluster;
    fSort           = param.fSort;
    for(Int_t i = 0;i<4;++i){
      fSDEDX[i]  = param.fSDEDX[i];
      fNCDEDX[i] = param.fNCDEDX[i];
      fNCDEDXInclThres[i] = param.fNCDEDXInclThres[i];
    }

    for(Int_t i = 0;i<AliPID::kSPECIES;++i)fTPCr[i] = param.fTPCr[i];
    
    fSeedType = param.fSeedType;
    fSeed1    = param.fSeed1;
    fSeed2    = param.fSeed2;
    for(Int_t i = 0;i<12;++i)fOverlapLabels[i] = param.fOverlapLabels[i];
    fMAngular = param.fMAngular;
    fCircular = param.fCircular;

    for (int i=5;i--;) fLSCovY[i] = param.fLSCovY[i];
    for (int i=3;i--;) fLSCovZ[i] = param.fLSCovZ[i];

  }
  return (*this);
}

Double_t AliTPCseed::GetDensityFirst(Int_t n)
{
  //
  //
  // return cluster for n rows bellow first point
  Int_t nfoundable = 1;
  Int_t nfound      = 1;
  for (Int_t i=fLastPoint-1;i>0&&nfoundable<n; i--){
    Int_t index = GetClusterIndex2(i);
    if (index!=-1) nfoundable++;
    if (index>0) nfound++;
  }
  if (nfoundable<n) return 0;
  return Double_t(nfound)/Double_t(nfoundable);

}


void AliTPCseed::GetClusterStatistic(Int_t first, Int_t last, Int_t &found, Int_t &foundable, Int_t &shared, Bool_t plus2)
{
  // get cluster stat.  on given region
  //
  found       = 0;
  foundable   = 0;
  shared      =0;
  for (Int_t i=first;i<last; i++){
    Int_t index = GetClusterIndex2(i);
    if (index!=-1) foundable++;
    if (index&0x8000) continue;
    if (fClusterPointer && fClusterPointer[i]) {
      found++;
    }
    else 
      continue;

    if (fClusterPointer && fClusterPointer[i]->IsUsed(10)) {
      shared++;
      continue;
    }
    if (!plus2) continue; //take also neighborhoud
    //
    if ( (i>0) && fClusterPointer && fClusterPointer[i-1]){
      if (fClusterPointer[i-1]->IsUsed(10)) {
	shared++;
	continue;
      }
    }
    if ( fClusterPointer && fClusterPointer[i+1]){
      if (fClusterPointer[i+1]->IsUsed(10)) {
	shared++;
	continue;
      }
    }
    
  }
  //if (shared>found){
    //Error("AliTPCseed::GetClusterStatistic","problem\n");
  //}
}


void AliTPCseed::GetClusterStatistic(Int_t first, Int_t last, Int_t &found, Int_t &foundable)
{
  // get cluster stat.  on given region, faster than the version providing also shared info
  //
  found = foundable = 0;
  for (Int_t i=first;i<last; i++){
    Int_t index = GetClusterIndex2(i);
    if (index!=-1) foundable++;
    if (index<0 || index&0x8000) continue;
    found++;
  }
}

void AliTPCseed::Reset(Bool_t all)
{
  //
  //
  SetNumberOfClusters(0);
  fNFoundable = 0;
  SetChi2(0);
  ResetCovariance(10.);
  for (int i=5;i--;) fLSCovY[i] = 0;
  for (int i=3;i--;) fLSCovZ[i] = 0;
  //
  if (all){   
    for (Int_t i=kMaxRow;i--;) SetClusterIndex2(i,-3);
    if (fClusterPointer) {
      if (!fClusterOwner) for (Int_t i=kMaxRow;i--;) fClusterPointer[i]=0;
      else                for (Int_t i=kMaxRow;i--;) {delete fClusterPointer[i]; fClusterPointer[i]=0;}
    }
  }

}


void AliTPCseed::Modify(Double_t factor)
{

  //------------------------------------------------------------------
  //This function makes a track forget its history :)  
  //------------------------------------------------------------------
  if (factor<=0) {
    ResetCovariance(10.);
    return;
  }
  ResetCovariance(factor);

  SetNumberOfClusters(0);
  fNFoundable =0;
  SetChi2(0);
  fRemoval = 0;
  fCurrentSigmaY2 = 0.000005;
  fCurrentSigmaZ2 = 0.000005;
  fNoCluster     = 0;
  //fFirstPoint = kMaxRow;
  //fLastPoint  = 0;
}




Int_t  AliTPCseed::GetProlongation(Double_t xk, Double_t &y, Double_t & z) const
{
  //-----------------------------------------------------------------
  // This function find proloncation of a track to a reference plane x=xk.
  // doesn't change internal state of the track
  //-----------------------------------------------------------------
  
  Double_t x1=GetX(), x2=x1+(xk-x1), dx=x2-x1;

  if (TMath::Abs(GetSnp()+GetC()*dx) >= AliTPCReconstructor::GetMaxSnpTrack()) {   
    return 0;
  }

  //  Double_t y1=fP0, z1=fP1;
  Double_t c1=GetSnp(), r1=sqrt((1.-c1)*(1.+c1));
  Double_t c2=c1 + GetC()*dx, r2=sqrt((1.-c2)*(1.+c2));
  
  y = GetY();
  z = GetZ();
  //y += dx*(c1+c2)/(r1+r2);
  //z += dx*(c1+c2)/(c1*r2 + c2*r1)*fP3;
  
  Double_t dy = dx*(c1+c2)/(r1+r2);
  Double_t dz = 0;
  //
  Double_t delta = GetC()*dx*(c1+c2)/(c1*r2 + c2*r1);
  /*
  if (TMath::Abs(delta)>0.0001){
    dz = fP3*TMath::ASin(delta)/fP4;
  }else{
    dz = dx*fP3*(c1+c2)/(c1*r2 + c2*r1);
  }
  */
  //  dz =  fP3*AliTPCFastMath::FastAsin(delta)/fP4;
  dz =  GetTgl()*TMath::ASin(delta)/GetC();
  //
  y+=dy;
  z+=dz;
  

  return 1;  
}


//_____________________________________________________________________________
Double_t AliTPCseed::GetPredictedChi2(const AliCluster *c) const 
{
  //-----------------------------------------------------------------
  // This function calculates a predicted chi2 increment.
  //-----------------------------------------------------------------
  Double_t p[2]={c->GetY(), c->GetZ()};
  Double_t cov[3]={fErrorY2, 0., fErrorZ2};

  Float_t dx = ((AliTPCclusterMI*)c)->GetX()-GetX();
  if (TMath::Abs(dx)>0){
    Float_t ty = TMath::Tan(TMath::ASin(GetSnp()));
    Float_t dy = dx*ty;
    Float_t dz = dx*TMath::Sqrt(1.+ty*ty)*GetTgl();
    p[0] = c->GetY()-dy;  
    p[1] = c->GetZ()-dz;  
  }
  return AliExternalTrackParam::GetPredictedChi2(p,cov);
}

//_________________________________________________________________________________________


Int_t AliTPCseed::Compare(const TObject *o) const {
  //-----------------------------------------------------------------
  // This function compares tracks according to the sector - for given sector according z
  //-----------------------------------------------------------------
  AliTPCseed *t=(AliTPCseed*)o;

  if (fSort == 0){
    if (t->fRelativeSector>fRelativeSector) return -1;
    if (t->fRelativeSector<fRelativeSector) return 1;
    Double_t z2 = t->GetZ();
    Double_t z1 = GetZ();
    if (z2>z1) return 1;
    if (z2<z1) return -1;
    return 0;
  }
  else {
    Float_t f2 =1;
    f2 = 1-20*TMath::Sqrt(t->GetSigma1Pt2())/(t->OneOverPt()+0.0066);
    if (t->fBConstrain) f2=1.2;

    Float_t f1 =1;
    f1 = 1-20*TMath::Sqrt(GetSigma1Pt2())/(OneOverPt()+0.0066);

    if (fBConstrain)   f1=1.2;
 
    if (t->GetNumberOfClusters()*f2 <GetNumberOfClusters()*f1) return -1;
    else return +1;
  }
}




//_____________________________________________________________________________
Bool_t AliTPCseed::Update(const AliCluster *c, Double_t chisq, Int_t index)
{
  //-----------------------------------------------------------------
  // This function associates a cluster with this track.
  //-----------------------------------------------------------------
  Int_t n=GetNumberOfClusters();
  Int_t idx=GetClusterIndex(n);    // save the current cluster index

  AliTPCclusterMI cl(*(AliTPCclusterMI*)c);  cl.SetSigmaY2(fErrorY2); cl.SetSigmaZ2(fErrorZ2);

  AliTPCClusterParam * parcl = AliTPCcalibDB::Instance()->GetClusterParam();
  
  Float_t ty = TMath::Tan(TMath::ASin(GetSnp()));
  
  if(  parcl ){
    Int_t padSize = 0;                          // short pads
    if (cl.GetDetector() >= 36) {
      padSize = 1;                              // medium pads 
      if (cl.GetRow() > 63) padSize = 2; // long pads
    }
    Float_t waveCorr = parcl->GetWaveCorrection( padSize, cl.GetZ(), cl.GetMax(),cl.GetPad(), ty );
    cl.SetY( cl.GetY() - waveCorr ); 
  }

  Float_t dx = ((AliTPCclusterMI*)c)->GetX()-GetX();
  if (TMath::Abs(dx)>0){
    Float_t dy = dx*ty;
    Float_t dz = dx*TMath::Sqrt(1.+ty*ty)*GetTgl();
    cl.SetY(cl.GetY()-dy);  
    cl.SetZ(cl.GetZ()-dz);  
  }  
 

  if (!AliTPCtrack::Update(&cl,chisq,index)) return kFALSE;
  // increment moments for LSM cov.matrix
  {
    double wy = fErrorY2>0 ? 1./fErrorY2 : 1e-6;
    double wz = fErrorZ2>0 ? 1./fErrorZ2 : 1e-6;
    double x1 = cl.GetX(), x2 = x1*x1, x3 = x2*x1, x4 = x2*x2;
    fLSCovY[0] += wy;
    fLSCovY[1] += wy*x1;
    fLSCovY[2] += wy*x2;
    fLSCovY[3] += wy*x3;
    fLSCovY[4] += wy*x4;
    //
    fLSCovZ[0] += wz;
    fLSCovZ[1] += wz*x1;
    fLSCovZ[2] += wz*x2;
  }
  //
  if (fCMeanSigmaY2p30<0){
    fCMeanSigmaY2p30= c->GetSigmaY2();   //! current mean sigma Y2 - mean30%
    fCMeanSigmaZ2p30= c->GetSigmaZ2();   //! current mean sigma Z2 - mean30%    
    fCMeanSigmaY2p30R = 1;   //! current mean sigma Y2 - mean5%
    fCMeanSigmaZ2p30R = 1;   //! current mean sigma Z2 - mean5%
  }
  //
  fCMeanSigmaY2p30= 0.70*fCMeanSigmaY2p30 +0.30*c->GetSigmaY2();   
  fCMeanSigmaZ2p30= 0.70*fCMeanSigmaZ2p30 +0.30*c->GetSigmaZ2();  
  if (fCurrentSigmaY2>0){
    fCMeanSigmaY2p30R = 0.7*fCMeanSigmaY2p30R  +0.3*c->GetSigmaY2()/fCurrentSigmaY2;  
    fCMeanSigmaZ2p30R = 0.7*fCMeanSigmaZ2p30R  +0.3*c->GetSigmaZ2()/fCurrentSigmaZ2;   
  }


  SetClusterIndex(n,idx);          // restore the current cluster index
  return kTRUE;
}



//_____________________________________________________________________________
Float_t AliTPCseed::CookdEdx(Double_t low, Double_t up,Int_t i1, Int_t i2, Bool_t /* onlyused */) {
  //-----------------------------------------------------------------
  // This funtion calculates dE/dX within the "low" and "up" cuts.
  //-----------------------------------------------------------------
  // CookdEdxAnalytical(Double_t low, Double_t up, Int_t type, Int_t i1, Int_t i2, Int_t returnVal)
  AliTPCParam *param = AliTPCcalibDB::Instance()->GetParameters();
  
  Int_t row0 = param->GetNRowLow();
  Int_t row1 = row0+param->GetNRowUp1();
  Int_t row2 = row1+param->GetNRowUp2();
  const AliTPCRecoParam * recoParam = AliTPCcalibDB::Instance()->GetTransform()->GetCurrentRecoParam();
  Int_t useTot = 0;
  if (recoParam) useTot = (recoParam->GetUseTotCharge())? 0:1;
  //
  //
  TVectorF i1i2;
  TVectorF  irocTot;
  TVectorF oroc1Tot;
  TVectorF oroc2Tot;
  TVectorF forocTot;
  //
  TVectorF  irocMax;
  TVectorF oroc1Max;
  TVectorF oroc2Max;
  TVectorF forocMax;

  CookdEdxAnalytical(low,up,useTot ,i1  ,i2,   0, 2, 0, &i1i2);
  //
  CookdEdxAnalytical(low,up,kTRUE ,0   ,row0, 0, 2, 0, &irocTot);
  CookdEdxAnalytical(low,up,kTRUE ,row0,row1, 0, 2, 0, &oroc1Tot);
  CookdEdxAnalytical(low,up,kTRUE ,row1,row2, 0, 2, 0, &oroc2Tot);
  CookdEdxAnalytical(low,up,kTRUE ,row0,row2, 0, 2, 0, &forocTot); // full OROC truncated mean
  //
  CookdEdxAnalytical(low,up,kFALSE ,0   ,row0, 0, 2, 0, &irocMax);
  CookdEdxAnalytical(low,up,kFALSE ,row0,row1, 0, 2, 0, &oroc1Max);
  CookdEdxAnalytical(low,up,kFALSE ,row1,row2, 0, 2, 0, &oroc2Max);
  CookdEdxAnalytical(low,up,kFALSE ,row0,row2, 0, 2, 0, &forocMax); // full OROC truncated mean

  fDEDX[0]      = i1i2(0);
  //
  fDEDX[1]      =  irocTot(0);
  fDEDX[2]      = oroc1Tot(0);
  fDEDX[3]      = oroc2Tot(0);
  fDEDX[4]      = forocTot(0); // full OROC truncated mean
  fDEDX[5]      =  irocMax(0);
  fDEDX[6]      = oroc1Max(0);
  fDEDX[7]      = oroc2Max(0);
  fDEDX[8]      = forocMax(0); // full OROC truncated mean
  //
  fSDEDX[0]     = i1i2(1);
  fSDEDX[1]     =  irocTot(1);
  fSDEDX[2]     = oroc1Tot(1);
  fSDEDX[3]     = oroc2Tot(1);
  //
  fNCDEDX[0]    = TMath::Nint(i1i2(2));

  fNCDEDX[1]    = TMath::Nint( irocTot(2));
  fNCDEDX[2]    = TMath::Nint(oroc1Tot(2));
  fNCDEDX[3]    = TMath::Nint(oroc2Tot(2));
  //
  fNCDEDXInclThres[0]    = TMath::Nint(i1i2(2)+i1i2(9));
  fNCDEDXInclThres[1]    = TMath::Nint( irocTot(2)+ irocTot(9));
  fNCDEDXInclThres[2]    = TMath::Nint(oroc1Tot(2)+oroc1Tot(9));
  fNCDEDXInclThres[3]    = TMath::Nint(oroc2Tot(2)+oroc2Tot(9));
  //
  SetdEdx(fDEDX[0]);
  return fDEDX[0];

//  return CookdEdxNorm(low,up,0,i1,i2,1,0,2);


//   Float_t amp[200];
//   Float_t angular[200];
//   Float_t weight[200];
//   Int_t index[200];
//   //Int_t nc = 0;
//   Float_t meanlog = 100.;
  
//   Float_t mean[4]  = {0,0,0,0};
//   Float_t sigma[4] = {1000,1000,1000,1000};
//   Int_t nc[4]      = {0,0,0,0};
//   Float_t norm[4]    = {1000,1000,1000,1000};
//   //
//   //
//   fNShared =0;

//   Float_t gainGG = 1;
//   if (AliTPCcalibDB::Instance()->GetParameters()){
//     gainGG= AliTPCcalibDB::Instance()->GetParameters()->GetGasGain()/20000.;  //relative gas gain
//   }


//   for (Int_t of =0; of<4; of++){    
//     for (Int_t i=of+i1;i<i2;i+=4)
//       {
// 	Int_t clindex = fIndex[i];
// 	if (clindex<0||clindex&0x8000) continue;

// 	//AliTPCTrackPoint * point = (AliTPCTrackPoint *) arr.At(i);
// 	AliTPCTrackerPoint * point = GetTrackPoint(i);
// 	//AliTPCTrackerPoint * pointm = GetTrackPoint(i-1);
// 	//AliTPCTrackerPoint * pointp = 0;
// 	//if (i<kMaxRow) pointp = GetTrackPoint(i+1);

// 	if (point==0) continue;
// 	AliTPCclusterMI * cl = fClusterPointer[i];
// 	if (cl==0) continue;	
// 	if (onlyused && (!cl->IsUsed(10))) continue;
// 	if (cl->IsUsed(11)) {
// 	  fNShared++;
// 	  continue;
// 	}
// 	Int_t   type   = cl->GetType();
// 	//if (point->fIsShared){
// 	//  fNShared++;
// 	//  continue;
// 	//}
// 	//if (pointm) 
// 	//  if (pointm->fIsShared) continue;
// 	//if (pointp) 
// 	//  if (pointp->fIsShared) continue;

// 	if (type<0) continue;
// 	//if (type>10) continue;       
// 	//if (point->GetErrY()==0) continue;
// 	//if (point->GetErrZ()==0) continue;

// 	//Float_t ddy = (point->GetY()-cl->GetY())/point->GetErrY();
// 	//Float_t ddz = (point->GetZ()-cl->GetZ())/point->GetErrZ();
// 	//if ((ddy*ddy+ddz*ddz)>10) continue; 


// 	//	if (point->GetCPoint().GetMax()<5) continue;
// 	if (cl->GetMax()<5) continue;
// 	Float_t angley = point->GetAngleY();
// 	Float_t anglez = point->GetAngleZ();

// 	Float_t rsigmay2 =  point->GetSigmaY();
// 	Float_t rsigmaz2 =  point->GetSigmaZ();
// 	/*
// 	Float_t ns = 1.;
// 	if (pointm){
// 	  rsigmay +=  pointm->GetTPoint().GetSigmaY();
// 	  rsigmaz +=  pointm->GetTPoint().GetSigmaZ();
// 	  ns+=1.;
// 	}
// 	if (pointp){
// 	  rsigmay +=  pointp->GetTPoint().GetSigmaY();
// 	  rsigmaz +=  pointp->GetTPoint().GetSigmaZ();
// 	  ns+=1.;
// 	}
// 	rsigmay/=ns;
// 	rsigmaz/=ns;
// 	*/

// 	Float_t rsigma = TMath::Sqrt(rsigmay2*rsigmaz2);

// 	Float_t ampc   = 0;     // normalization to the number of electrons
// 	if (i>64){
// 	  //	  ampc = 1.*point->GetCPoint().GetMax();
// 	  ampc = 1.*cl->GetMax();
// 	  //ampc = 1.*point->GetCPoint().GetQ();	  
// 	  //	  AliTPCClusterPoint & p = point->GetCPoint();
// 	  //	  Float_t dy = TMath::Abs(Int_t( TMath::Abs(p.GetY()/0.6)) - TMath::Abs(p.GetY()/0.6)+0.5);
// 	  // Float_t iz =  (250.0-TMath::Abs(p.GetZ())+0.11)/0.566;
// 	  //Float_t dz = 
// 	  //  TMath::Abs( Int_t(iz) - iz + 0.5);
// 	  //ampc *= 1.15*(1-0.3*dy);
// 	  //ampc *= 1.15*(1-0.3*dz);
// 	  //	  Float_t zfactor = (AliTPCReconstructor::GetCtgRange()-0.0004*TMath::Abs(point->GetCPoint().GetZ()));
// 	  //ampc               *=zfactor; 
// 	}
// 	else{ 
// 	  //ampc = 1.0*point->GetCPoint().GetMax(); 
// 	  ampc = 1.0*cl->GetMax(); 
// 	  //ampc = 1.0*point->GetCPoint().GetQ(); 
// 	  //AliTPCClusterPoint & p = point->GetCPoint();
// 	  // Float_t dy = TMath::Abs(Int_t( TMath::Abs(p.GetY()/0.4)) - TMath::Abs(p.GetY()/0.4)+0.5);
// 	  //Float_t iz =  (250.0-TMath::Abs(p.GetZ())+0.11)/0.566;
// 	  //Float_t dz = 
// 	  //  TMath::Abs( Int_t(iz) - iz + 0.5);

// 	  //ampc *= 1.15*(1-0.3*dy);
// 	  //ampc *= 1.15*(1-0.3*dz);
// 	  //	Float_t zfactor = (1.02-0.000*TMath::Abs(point->GetCPoint().GetZ()));
// 	  //ampc               *=zfactor; 

// 	}
// 	ampc *= 2.0;     // put mean value to channel 50
// 	//ampc *= 0.58;     // put mean value to channel 50
// 	Float_t w      =  1.;
// 	//	if (type>0)  w =  1./(type/2.-0.5); 
// 	//	Float_t z = TMath::Abs(cl->GetZ());
// 	if (i<64) {
// 	  ampc /= 0.6;
// 	  //ampc /= (1+0.0008*z);
// 	} else
// 	  if (i>128){
// 	    ampc /=1.5;
// 	    //ampc /= (1+0.0008*z);
// 	  }else{
// 	    //ampc /= (1+0.0008*z);
// 	  }
	
// 	if (type<0) {  //amp at the border - lower weight
// 	  // w*= 2.;
	  
// 	  continue;
// 	}
// 	if (rsigma>1.5) ampc/=1.3;  // if big backround
// 	amp[nc[of]]        = ampc;
// 	amp[nc[of]]       /=gainGG;
// 	angular[nc[of]]    = TMath::Sqrt(1.+angley*angley+anglez*anglez);
// 	weight[nc[of]]     = w;
// 	nc[of]++;
//       }
    
//     TMath::Sort(nc[of],amp,index,kFALSE);
//     Float_t sumamp=0;
//     Float_t sumamp2=0;
//     Float_t sumw=0;
//     //meanlog = amp[index[Int_t(nc[of]*0.33)]];
//     meanlog = 50;
//     for (Int_t i=int(nc[of]*low+0.5);i<int(nc[of]*up+0.5);i++){
//       Float_t ampl      = amp[index[i]]/angular[index[i]];
//       ampl              = meanlog*TMath::Log(1.+ampl/meanlog);
//       //
//       sumw    += weight[index[i]]; 
//       sumamp  += weight[index[i]]*ampl;
//       sumamp2 += weight[index[i]]*ampl*ampl;
//       norm[of]    += angular[index[i]]*weight[index[i]];
//     }
//     if (sumw<1){ 
//       SetdEdx(0);  
//     }
//     else {
//       norm[of] /= sumw;
//       mean[of]  = sumamp/sumw;
//       sigma[of] = sumamp2/sumw-mean[of]*mean[of];
//       if (sigma[of]>0.1) 
// 	sigma[of] = TMath::Sqrt(sigma[of]);
//       else
// 	sigma[of] = 1000;
      
//     mean[of] = (TMath::Exp(mean[of]/meanlog)-1)*meanlog;
//     //mean  *=(1-0.02*(sigma/(mean*0.17)-1.));
//     //mean *=(1-0.1*(norm-1.));
//     }
//   }

//   Float_t dedx =0;
//   fSdEdx =0;
//   fMAngular =0;
//   //  mean[0]*= (1-0.05*(sigma[0]/(0.01+mean[1]*0.18)-1));
//   //  mean[1]*= (1-0.05*(sigma[1]/(0.01+mean[0]*0.18)-1));

  
//   //  dedx = (mean[0]* TMath::Sqrt((1.+nc[0]))+ mean[1]* TMath::Sqrt((1.+nc[1])) )/ 
//   //  (  TMath::Sqrt((1.+nc[0]))+TMath::Sqrt((1.+nc[1])));

//   Int_t norm2 = 0;
//   Int_t norm3 = 0;
//   for (Int_t i =0;i<4;i++){
//     if (nc[i]>2&&nc[i]<1000){
//       dedx      += mean[i] *nc[i];
//       fSdEdx    += sigma[i]*(nc[i]-2);
//       fMAngular += norm[i] *nc[i];    
//       norm2     += nc[i];
//       norm3     += nc[i]-2;
//     }
//     fDEDX[i]  = mean[i];             
//     fSDEDX[i] = sigma[i];            
//     fNCDEDX[i]= nc[i]; 
//   }

//   if (norm3>0){
//     dedx   /=norm2;
//     fSdEdx /=norm3;
//     fMAngular/=norm2;
//   }
//   else{
//     SetdEdx(0);
//     return 0;
//   }
//   //  Float_t dedx1 =dedx;
//   /*
//   dedx =0;
//   for (Int_t i =0;i<4;i++){
//     if (nc[i]>2&&nc[i]<1000){
//       mean[i]   = mean[i]*(1-0.12*(sigma[i]/(fSdEdx)-1.));
//       dedx      += mean[i] *nc[i];
//     }
//     fDEDX[i]  = mean[i];                
//   }
//   dedx /= norm2;
//   */

  
//   SetdEdx(dedx);
//   return dedx;
}

void AliTPCseed::CookPID()
{
  //
  // cook PID information according dEdx
  //
  Double_t fRange = 10.;
  Double_t fRes   = 0.1;
  Double_t fMIP   = 47.;
  //
  Int_t ns=AliPID::kSPECIES;
  Double_t sumr =0;
  for (Int_t j=0; j<ns; j++) {
    Double_t mass=AliPID::ParticleMass(j);
    Double_t mom=GetP();
    Double_t dedx=fdEdx/fMIP;
    Double_t bethe=AliExternalTrackParam::BetheBlochAleph(mom/mass); 
    Double_t sigma=fRes*bethe;
    if (sigma>0.001){
      if (TMath::Abs(dedx-bethe) > fRange*sigma) {
	fTPCr[j]=TMath::Exp(-0.5*fRange*fRange)/sigma;
	sumr+=fTPCr[j];
	continue;
      }
      fTPCr[j]=TMath::Exp(-0.5*(dedx-bethe)*(dedx-bethe)/(sigma*sigma))/sigma;
      sumr+=fTPCr[j];
    }
    else{
      fTPCr[j]=1.;
      sumr+=fTPCr[j];
    }
  }
  for (Int_t j=0; j<ns; j++) {
    fTPCr[j]/=sumr;           //normalize
  }
}

Double_t AliTPCseed::GetYat(Double_t xk) const {
//-----------------------------------------------------------------
// This function calculates the Y-coordinate of a track at the plane x=xk.
//-----------------------------------------------------------------
  if (TMath::Abs(GetSnp())>AliTPCReconstructor::GetMaxSnpTrack()) return 0.; //patch 01 jan 06
    Double_t c1=GetSnp(), r1=TMath::Sqrt((1.-c1)*(1.+c1));
    Double_t c2=c1+GetC()*(xk-GetX());
    if (TMath::Abs(c2)>AliTPCReconstructor::GetMaxSnpTrack()) return 0;
    Double_t r2=TMath::Sqrt((1.-c2)*(1.+c2));
    return GetY() + (xk-GetX())*(c1+c2)/(r1+r2);
}



Float_t  AliTPCseed::CookdEdxNorm(Double_t low, Double_t up, Int_t type, Int_t i1, Int_t i2, Bool_t shapeNorm,Int_t posNorm, Int_t padNorm, Int_t returnVal){
 
  //
  // calculates dedx using the cluster
  // low    -  up specify trunc mean range  - default form 0-0.7
  // type   -  1 - max charge  or 0- total charge in cluster 
  //           //2- max no corr 3- total+ correction
  // i1-i2  -  the pad-row range used for calculation
  // shapeNorm - kTRUE  -taken from OCDB
  //           
  // posNorm   - usage of pos normalization 
  // padNorm   - pad type normalization
  // returnVal - 0 return mean
  //           - 1 return RMS
  //           - 2 return number of clusters
  //           
  // normalization parametrization taken from AliTPCClusterParam
  //
  AliTPCClusterParam * parcl = AliTPCcalibDB::Instance()->GetClusterParam();
  AliTPCParam * param = AliTPCcalibDB::Instance()->GetParameters();
  if (!parcl)  return 0;
  if (!param) return 0;
  Int_t row0 = param->GetNRowLow();
  Int_t row1 = row0+param->GetNRowUp1();

  Float_t amp[kMaxRow];
  Int_t   indexes[kMaxRow];
  Int_t   ncl=0;
  //
  //
  Float_t gainGG      = 1;  // gas gain factor -always enabled
  Float_t gainPad     = 1;  // gain map  - used always
  Float_t corrShape   = 1;  // correction due angular effect, diffusion and electron attachment
  Float_t corrPos     = 1;  // local position correction - if posNorm enabled
  Float_t corrPadType = 1;  // pad type correction - if padNorm enabled
  Float_t corrNorm    = 1;  // normalization factor - set Q to channel 50
  //   
  //
  //
  if (AliTPCcalibDB::Instance()->GetParameters()){
    gainGG= AliTPCcalibDB::Instance()->GetParameters()->GetGasGain()/20000;  //relative gas gain
    gainGG *= AliTPCcalibDB::Instance()->GetParameters()->GetNtot()/36.82;//correction for the ionisation
  }

  const Float_t ktany = TMath::Tan(TMath::DegToRad()*10);
  const Float_t kedgey =3.;
  //
  //
  for (Int_t irow=i1; irow<i2; irow++){
    AliTPCclusterMI* cluster = GetClusterPointer(irow);
    if (!cluster) continue;
    if (TMath::Abs(cluster->GetY())>cluster->GetX()*ktany-kedgey) continue; // edge cluster
    Float_t charge= (type%2)? cluster->GetMax():cluster->GetQ();
    Int_t  ipad= 0;
    if (irow>=row0) ipad=1;
    if (irow>=row1) ipad=2;    
    //
    //
    //
    AliTPCCalPad * gainMap =  AliTPCcalibDB::Instance()->GetDedxGainFactor();
    if (gainMap) {
      //
      // Get gainPad - pad by pad calibration
      //
      Float_t factor = 1;      
      AliTPCCalROC * roc = gainMap->GetCalROC(cluster->GetDetector());
      if (irow < row0) { // IROC
	factor = roc->GetValue(irow, TMath::Nint(cluster->GetPad()));
      } else {         // OROC
	factor = roc->GetValue(irow - row0, TMath::Nint(cluster->GetPad()));
      }
      if (factor>0.5) gainPad=factor;
    }
    //
    //do position and angular normalization
    //
    if (shapeNorm){
      if (type<=1){
	//	
	const AliTPCTrackerPoints::Point * point = GetTrackPoint(irow);
	Float_t              ty = TMath::Abs(point->GetAngleY());
	Float_t              tz = TMath::Abs(point->GetAngleZ()*TMath::Sqrt(1+ty*ty));
	
	Float_t dr    = (250.-TMath::Abs(cluster->GetZ()))/250.;
	corrShape  = parcl->Qnorm(ipad,type,dr,ty,tz);
      }
    }
    
    if (posNorm>0){
      //
      // Do position normalization - relative distance to 
      // center of pad- time bin
      // Work in progress
      //      corrPos = parcl->QnormPos(ipad,type, cluster->GetPad(),
      // 				cluster->GetTimeBin(), cluster->GetZ(),
      // 				cluster->GetSigmaY2(),cluster->GetSigmaZ2(),
      // 				cluster->GetMax(),cluster->GetQ());
      // scaled response function
      Float_t yres0 = parcl->GetRMS0(0,ipad,0,0)/param->GetPadPitchWidth(cluster->GetDetector());
      Float_t zres0 = parcl->GetRMS0(1,ipad,0,0)/param->GetZWidth();
      //
      
      const AliTPCTrackerPoints::Point * point = GetTrackPoint(irow);
      Float_t              ty = TMath::Abs(point->GetAngleY());
      Float_t              tz = TMath::Abs(point->GetAngleZ()*TMath::Sqrt(1+ty*ty));
      
      if (type==1) corrPos = 
	parcl->QmaxCorrection(cluster->GetDetector(), cluster->GetRow(),cluster->GetPad(), 
			      cluster->GetTimeBin(),ty,tz,yres0,zres0,0.4);
      if (type==0) corrPos = 
	parcl->QtotCorrection(cluster->GetDetector(), cluster->GetRow(),cluster->GetPad(), 
			      cluster->GetTimeBin(),ty,tz,yres0,zres0,cluster->GetQ(),2.5,0.4);
      if (posNorm==3){
	Float_t dr    = (250.-TMath::Abs(cluster->GetZ()))/250.;
	Double_t signtgl = (cluster->GetZ()*point->GetAngleZ()>0)? 1:-1;
	Double_t p2 = TMath::Abs(TMath::Sin(TMath::ATan(ty)));
	Float_t corrHis = parcl->QnormHis(ipad,type,dr,p2,TMath::Abs(point->GetAngleZ())*signtgl);
	if (corrHis>0) corrPos*=corrHis;
      }

    }

    if (padNorm==1){
      //taken from OCDB
      if (type==0 && parcl->QpadTnorm()) corrPadType = (*parcl->QpadTnorm())[ipad];
      if (type==1 && parcl->QpadMnorm()) corrPadType = (*parcl->QpadMnorm())[ipad];

    }
    if (padNorm==2){
      corrPadType  =param->GetPadPitchLength(cluster->GetDetector(),cluster->GetRow());
      //use hardwired - temp fix
      if (type==0) corrNorm=3.;
      if (type==1) corrNorm=1.;
    }
    //
    //
    amp[ncl]=charge;
    amp[ncl]/=gainGG;                 // normalized gas gain
    amp[ncl]/=gainPad;                // 
    amp[ncl]/=corrShape;
    amp[ncl]/=corrPadType;
    amp[ncl]/=corrPos;
    amp[ncl]/=corrNorm; 
    //
    ncl++;
  }

  if (type>3) return ncl; 
  TMath::Sort(ncl,amp, indexes, kFALSE);

  if (ncl<10) return 0;
  
  Float_t suma=0;
  Float_t suma2=0;  
  Float_t sumn=0;
  Int_t icl0=TMath::Nint(ncl*low);
  Int_t icl1=TMath::Nint(ncl*up);
  for (Int_t icl=icl0; icl<icl1;icl++){
    suma+=amp[indexes[icl]];
    suma2+=amp[indexes[icl]]*amp[indexes[icl]];
    sumn++;
  }
  Float_t mean =suma/sumn;
  Float_t rms  =TMath::Sqrt(TMath::Abs(suma2/sumn-mean*mean));
  //
  // do time-dependent correction for pressure and temperature variations
  UInt_t runNumber = 1;
  Float_t corrTimeGain = 1;
  AliTPCTransform * trans = AliTPCcalibDB::Instance()->GetTransform();
  const AliTPCRecoParam * recoParam = AliTPCcalibDB::Instance()->GetTransform()->GetCurrentRecoParam();
  if (trans && recoParam->GetUseGainCorrectionTime()>0) {
    runNumber = trans->GetCurrentRunNumber();
    //AliTPCcalibDB::Instance()->SetRun(runNumber);
    TObjArray * timeGainSplines = AliTPCcalibDB::Instance()->GetTimeGainSplinesRun(runNumber);
    if (timeGainSplines) {
      UInt_t time = trans->GetCurrentTimeStamp();
      AliSplineFit * fitMIP = (AliSplineFit *) timeGainSplines->At(0);
      AliSplineFit * fitFPcosmic = (AliSplineFit *) timeGainSplines->At(1);
      if (fitMIP) {
	corrTimeGain = AliTPCcalibDButil::EvalGraphConst(fitMIP, time);/*fitMIP->Eval(time);*/
      } else {
	if (fitFPcosmic) corrTimeGain = AliTPCcalibDButil::EvalGraphConst(fitFPcosmic, time);/*fitFPcosmic->Eval(time);*/ 
      }
    }
  }
  mean /= corrTimeGain;
  rms /= corrTimeGain;
  //
  if (returnVal==1) return rms;
  if (returnVal==2) return ncl;
  return mean;
}

Float_t  AliTPCseed::CookdEdxAnalytical(Double_t low, Double_t up, Int_t type, Int_t i1, Int_t i2, Int_t returnVal, Int_t rowThres, Int_t mode, TVectorT<float> *returnVec){
 
  //
  // calculates dedx using the cluster
  // low    -  up specify trunc mean range  - default form 0-0.7
  // type   -  1 - max charge  or 0- total charge in cluster 
  //           //2- max no corr 3- total+ correction
  // i1-i2  -  the pad-row range used for calculation
  //           
  // posNorm   - usage of pos normalization 
  // returnVal - 0  return mean
  //           - 1  return RMS
  //           - 2  return number of clusters
  //           - 3  ratio
  //           - 4  mean upper half
  //           - 5  mean  - lower half
  //           - 6  third moment
  // mode      - 0 - linear
  //           - 1 - logatithmic
  // rowThres  - number of rows before and after given pad row to check for clusters below threshold
  //           
  // normalization parametrization taken from AliTPCClusterParam
  //
  if (returnVec) returnVec->ResizeTo(10);

  AliTPCClusterParam * parcl = AliTPCcalibDB::Instance()->GetClusterParam();
  AliTPCParam * param = AliTPCcalibDB::Instance()->GetParameters();
  AliTPCTransform * trans = AliTPCcalibDB::Instance()->GetTransform();
  const AliTPCRecoParam * recoParam = AliTPCcalibDB::Instance()->GetTransform()->GetCurrentRecoParam();
  if (!parcl)  return 0;
  if (!param) return 0;
  Int_t row0 = param->GetNRowLow();
  Int_t row1 = row0+param->GetNRowUp1();

  Float_t amp[kMaxRow];
  Int_t   indexes[kMaxRow];
  Int_t   ncl=0;
  Int_t   nclBelowThr = 0; // counts number of clusters below threshold
  //
  //
  Float_t gainGG      = 1;  // gas gain factor -always enabled
  Float_t gainPad     = 1;  // gain map  - used always
  Float_t corrPos     = 1;  // local position correction - if posNorm enabled
  //   
  //
  //
  if (AliTPCcalibDB::Instance()->GetParameters()){
    gainGG= AliTPCcalibDB::Instance()->GetParameters()->GetGasGain()/20000;  //relative gas gain
    gainGG *= AliTPCcalibDB::Instance()->GetParameters()->GetNtot()/36.82;//correction for the ionisation
  }
  Double_t timeCut=0;
  if (AliTPCcalibDB::Instance()->IsTrgL0()){
    // by defualt we assume L1 trigger is used - make a correction in case of  L0
    AliCTPTimeParams* ctp = AliTPCcalibDB::Instance()->GetCTPTimeParams();
    Double_t delay = ctp->GetDelayL1L0()*0.000000025;
    delay/=param->GetTSample();    
    timeCut=delay;
  }
  timeCut += recoParam->GetSkipTimeBins();

  //
  // extract time-dependent correction for pressure and temperature variations
  //
  UInt_t runNumber = 1;
  Float_t corrTimeGain = 1;
  TObjArray * timeGainSplines = 0x0;
  TGraphErrors * grPadEqual = 0x0;
  TGraphErrors*  grChamberGain[4]={0x0,0x0,0x0,0x0};
  TF1*  funDipAngle[4]={0x0,0x0,0x0,0x0};
  //
  //
  if (recoParam->GetNeighborRowsDedx() == 0) rowThres = 0;	
  UInt_t time = 1;//
  //
  if (trans) {
      runNumber = trans->GetCurrentRunNumber();
      time = trans->GetCurrentTimeStamp();
      //AliTPCcalibDB::Instance()->SetRun(runNumber);
      timeGainSplines = AliTPCcalibDB::Instance()->GetTimeGainSplinesRun(runNumber);
      if (timeGainSplines && recoParam->GetUseGainCorrectionTime()>0) {
	AliSplineFit * fitMIP = (AliSplineFit *) timeGainSplines->At(0);
	AliSplineFit * fitFPcosmic = (AliSplineFit *) timeGainSplines->At(1);
	if (fitMIP) {
	  corrTimeGain =  AliTPCcalibDButil::EvalGraphConst(fitMIP, time); /*fitMIP->Eval(time);*/
	} else {
	  if (fitFPcosmic) corrTimeGain = AliTPCcalibDButil::EvalGraphConst(fitFPcosmic, time); /*fitFPcosmic->Eval(time); */
	}
	//
	if (type==1) grPadEqual = (TGraphErrors * ) timeGainSplines->FindObject("TGRAPHERRORS_MEANQMAX_PADREGIONGAIN_BEAM_ALL");
	if (type==0) grPadEqual = (TGraphErrors * ) timeGainSplines->FindObject("TGRAPHERRORS_MEANQTOT_PADREGIONGAIN_BEAM_ALL");
        const char* names[4]={"SHORT","MEDIUM","LONG","ABSOLUTE"};
        for (Int_t iPadRegion=0; iPadRegion<4; ++iPadRegion) {
          grChamberGain[iPadRegion]=(TGraphErrors*)timeGainSplines->FindObject(Form("TGRAPHERRORS_MEAN_CHAMBERGAIN_%s_BEAM_ALL",names[iPadRegion]));
	  if (type==1) funDipAngle[iPadRegion]=(TF1*)timeGainSplines->FindObject(Form("TF1_QMAX_DIPANGLE_%s_BEAM_ALL",names[iPadRegion]));
	  if (type==0) funDipAngle[iPadRegion]=(TF1*)timeGainSplines->FindObject(Form("TF1_QTOT_DIPANGLE_%s_BEAM_ALL",names[iPadRegion]));
	}
      }
  }
  
  const Float_t kClusterShapeCut = 1.5; // IMPPRTANT TO DO: move value to AliTPCRecoParam
  const Float_t ktany = TMath::Tan(TMath::DegToRad()*10);
  const Float_t kedgey =3.;
  //
  //
  for (Int_t irow=i1; irow<i2; irow++){
    AliTPCclusterMI* cluster = GetClusterPointer(irow);
    if (!cluster && irow > 1 && irow < 157) {
      Bool_t isClBefore = kFALSE;
      Bool_t isClAfter  = kFALSE;
      for(Int_t ithres = 1; ithres <= rowThres; ithres++) {
	AliTPCclusterMI * clusterBefore = GetClusterPointer(irow - ithres);
	if (clusterBefore) isClBefore = kTRUE;
	AliTPCclusterMI * clusterAfter  = GetClusterPointer(irow + ithres);
	if (clusterAfter) isClAfter = kTRUE;
      }
      if (isClBefore && isClAfter) nclBelowThr++;
    }
    if (!cluster) continue;
    if (cluster->GetTimeBin()<timeCut) continue; //reject  clusters at the gating grid opening
    //
    //
    if (TMath::Abs(cluster->GetY())>cluster->GetX()*ktany-kedgey) continue; // edge cluster
    //
    const AliTPCTrackerPoints::Point * point = GetTrackPoint(irow);
    if (point==0) continue;    
    Float_t rsigmay = TMath::Sqrt(point->GetSigmaY());
    if (rsigmay > kClusterShapeCut) continue;
    //
    if (cluster->IsUsed(11)) continue; // remove shared clusters for PbPb
    //
    Float_t charge= (type%2)? cluster->GetMax():cluster->GetQ();
    Int_t  ipad= 0;
    if (irow>=row0) ipad=1;
    if (irow>=row1) ipad=2;    
    //
    //
    //
    AliTPCCalPad * gainMap =  AliTPCcalibDB::Instance()->GetDedxGainFactor();
    if (gainMap) {
      //
      // Get gainPad - pad by pad calibration
      //
      Float_t factor = 1;      
      AliTPCCalROC * roc = gainMap->GetCalROC(cluster->GetDetector());
      if (irow < row0) { // IROC
	factor = roc->GetValue(irow, TMath::Nint(cluster->GetPad()));
      } else {         // OROC
	factor = roc->GetValue(irow - row0, TMath::Nint(cluster->GetPad()));
      }
      if (factor>0.3) gainPad=factor;
    }
    //
    // Do position normalization - relative distance to 
    // center of pad- time bin
    
    Float_t              ty = TMath::Abs(point->GetAngleY());
    Float_t              tz = TMath::Abs(point->GetAngleZ()*TMath::Sqrt(1+ty*ty));
    Float_t yres0 = parcl->GetRMS0(0,ipad,0,0)/param->GetPadPitchWidth(cluster->GetDetector());
    Float_t zres0 = parcl->GetRMS0(1,ipad,0,0)/param->GetZWidth();

    yres0 *=parcl->GetQnormCorr(ipad, type,0);
    zres0 *=parcl->GetQnormCorr(ipad, type,1);
    Float_t effLength=parcl->GetQnormCorr(ipad, type,4)*0.5;
    Float_t effDiff  =(parcl->GetQnormCorr(ipad, type,2)+parcl->GetQnormCorr(ipad, type,3))*0.5;
    Float_t corrThr=0;
    Float_t corrThrMax=0;
    //
    if (type==1) {
      corrThr =	parcl->QmaxCorrection(cluster->GetDetector(), cluster->GetRow(),cluster->GetPad(), 
				      cluster->GetTimeBin(),ty,tz,yres0,zres0,effLength,effDiff);
      corrPos= parcl->GetQnormCorr(ipad, type,5)*corrThr;
      Float_t drm   = 0.5-TMath::Abs(cluster->GetZ()/250.);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,0)*drm);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,1)*ty*ty);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,2)*tz*tz);
      //
    }
    if (type==0) {
      corrThr = parcl->QtotCorrection(cluster->GetDetector(), cluster->GetRow(),cluster->GetPad(), 
				      cluster->GetTimeBin(),ty,tz,yres0,zres0,cluster->GetQ(),2.5,effLength,effDiff);
      corrPos=parcl->GetQnormCorr(ipad, type,5)*corrThr;      
      Float_t drm   = 0.5-TMath::Abs(cluster->GetZ()/250.);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,0)*drm);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,1)*ty*ty);
      corrPos*=(1+parcl->GetQnormCorr(ipad, type+2,2)*tz*tz);
      //
    }
    //
    // pad region equalization outside of cluster param
    //
    Float_t gainEqualPadRegion = 1;
    if (grPadEqual && recoParam->GetUseGainCorrectionTime()>0) gainEqualPadRegion = grPadEqual->Eval(ipad);
    //
    // chamber-by-chamber equalization outside gain map
    //
    Float_t gainChamber = 1;
    if (grChamberGain[ipad] && recoParam->GetUseGainCorrectionTime()>0) {
      gainChamber = grChamberGain[ipad]->Eval(cluster->GetDetector());
      if (gainChamber==0) gainChamber=1; // in case old calibation was used before use no correction
    }
    //
    // dip angle correction
    //
    Float_t corrDipAngle = 1;
    Float_t corrDipAngleAbs = 1;
    //    if (grDipAngle[ipad]) corrDipAngle = grDipAngle[ipad]->Eval(GetTgl());
    Double_t tgl=GetTgl();
    if (funDipAngle[ipad]) corrDipAngle = funDipAngle[ipad]->Eval(tgl);
    if (funDipAngle[3]) corrDipAngleAbs = funDipAngle[3]->Eval(tgl);
    //
    // pressure temperature and high voltage correction
    //
    Double_t correctionHVandPT = AliTPCcalibDB::Instance()->GetGainCorrectionHVandPT(time, runNumber,cluster->GetDetector(), 5 , recoParam->GetGainCorrectionHVandPTMode());
    //
    if ((AliTPCReconstructor::StreamLevel()&AliTPCtracker::kStreamSeeddEdx)){  // this part of the code is for the test purposes only  
      TTreeSRedirector *pcstream = AliTPCReconstructor:: GetDebugStreamer();
      TVectorF vecDEDX(9,fDEDX);
      TVectorF vecSDEDX(4,fSDEDX);
      TVectorF vecRDEDX(4);
      corrThrMax = parcl->QmaxCorrection(cluster->GetDetector(), cluster->GetRow(),cluster->GetPad(), 
					 cluster->GetTimeBin(),ty,tz,yres0,zres0,effLength,effDiff);

      for (Int_t i=0; i<4; i++) vecRDEDX[i]=(fNCDEDXInclThres[i]>0)?Float_t(fNCDEDX[i])/Float_t(fNCDEDXInclThres[i]):0;
      if (pcstream){
	(*pcstream)<<"dEdxCorrDump"<<    // streamer to check dEdx correction calibration
	  //
	  "cl.="<<cluster<<
	  "ipad="<<ipad<<
	  "time="<<time<<
	  "runNumber="<<runNumber<<
	  "vecDEDX.="<<&vecDEDX<<
	  "vecSDEDX.="<<&vecSDEDX<<
	  "vecRDEDX.="<<&vecRDEDX<<
	  "ty="<<ty<<
	  "tz="<<tz<<
	  "yres0="<<yres0<<
	  "zres0="<<zres0<<
	  "qpt="<<fP[4]<<
	  "type="<<type<<
	  "effLength="<<effLength<<
	  "effDiff="<<effDiff<<
	  "corrThr="<<corrThr<<
	  "corrThrMax="<<corrThrMax<<
	  "gainGG="<<gainGG<<
	  "correctionHVandPT="<<correctionHVandPT<<
	  "gainPad="<<gainPad<<
	  "corrPos="<<corrPos<<
	  "gainEqualPadRegion="<<gainEqualPadRegion<<
	  "gainChamber="<<gainChamber<<
	  "corrDipAngle="<<corrDipAngle<<
	  "corrDipAngleAbs="<<corrDipAngleAbs<<
	  "\n";
      }
    }
    amp[ncl]=charge;
    amp[ncl]/=gainGG;               // nominal gas gain
    amp[ncl]/=correctionHVandPT;    // correction for the HV and P/T - time dependent
    amp[ncl]/=gainPad;              // 
    amp[ncl]/=corrPos;
    amp[ncl]/=gainEqualPadRegion;
    amp[ncl]/=gainChamber;
    amp[ncl]/=corrDipAngle;
    amp[ncl]/=corrDipAngleAbs;
    amp[ncl]/=corrTimeGain;
    //
    ncl++;
  }

  if (type==2) return ncl; 
  TMath::Sort(ncl,amp, indexes, kFALSE);
  //
  if (ncl<10) return 0;
  //
  Double_t ampWithBelow[ncl + nclBelowThr];
  for(Int_t iCl = 0; iCl < ncl + nclBelowThr; iCl++) {
    if (iCl < nclBelowThr) {
      ampWithBelow[iCl] = amp[indexes[0]];
    } else {
      ampWithBelow[iCl] = amp[indexes[iCl - nclBelowThr]];
    }
  }
  Double_t factor=recoParam->GetMissingClusterdEdxFraction();
  if (factor>0) {
    if (factor>1) factor=1.;
    Double_t missingChargeReplacement = 0;
    Int_t nClForReplacement = TMath::Min(int(nclBelowThr+1), int(ncl * up));
    missingChargeReplacement = TMath::Mean(nClForReplacement, &(ampWithBelow[nclBelowThr]));
    for(Int_t iCl = 0; iCl < nclBelowThr; iCl++)  ampWithBelow[iCl]=missingChargeReplacement;
  }

  //printf("DEBUG: %i shit %f", nclBelowThr, amp[indexes[0]]);
  //
  Float_t suma=0;
  Float_t suma2=0;  
  Float_t suma3=0;  
  Float_t sumaS=0;  
  Float_t sumn=0;
  // upper,and lower part statistic
  Float_t sumL=0, sumL2=0, sumLN=0;
  Float_t sumD=0, sumD2=0, sumDN=0;

  Int_t iclN=TMath::Nint((ncl + nclBelowThr*(1-factor))*(up-low));
  Int_t icl0=TMath::Nint((ncl + nclBelowThr*(1-factor))*low+ nclBelowThr*factor);
  Int_t icl1=icl0+iclN;
  Int_t iclm=icl0+iclN/2;
  //
  for (Int_t icl=icl0; icl<icl1;icl++){
    if (ampWithBelow[icl]<0.1) continue;
    Double_t camp=ampWithBelow[icl];  // CHANGE 02.12.2019 - make time gain correction together with other corrections
    if (mode==1) camp= TMath::Log(camp);
    if (icl<icl1){
      suma+=camp;
      suma2+=camp*camp;
      suma3+=camp*camp*camp;
      sumaS+=TMath::Power(TMath::Abs(camp),1./3.);
      sumn++;
    }
    if (icl>iclm){
      sumL+=camp;
      sumL2+=camp*camp;
      sumLN++;
      }
    if (icl<=iclm){
      sumD+=camp;
      sumD2+=camp*camp;
      sumDN++;
    }
  }
  //
  Float_t mean = 0;
  Float_t meanL = 0;  
  Float_t meanD = 0;           // lower half mean
  if (sumn > 1e-30)   mean =suma/sumn;
  if (sumLN > 1e-30)  meanL =sumL/sumLN;
  if (sumDN > 1e-30)  meanD =(sumD/sumDN);
  /*
  Float_t mean =suma/sumn;
  Float_t meanL = sumL/sumLN;  
  Float_t meanD =(sumD/sumDN);           // lower half mean
  */

  Float_t rms = 0;
  Float_t mean2=0;
  Float_t mean3=0;
  Float_t meanS=0;

  if(sumn>0){
    rms = TMath::Sqrt(TMath::Abs(suma2/sumn-mean*mean));
    mean2=suma2/sumn;
    mean3=suma3/sumn;
    meanS=sumaS/sumn;
  }

  if (mean2>0) mean2=TMath::Power(TMath::Abs(mean2),1./2.);
  if (mean3>0) mean3=TMath::Power(TMath::Abs(mean3),1./3.);
  if (meanS>0) meanS=TMath::Power(TMath::Abs(meanS),3.);
  //
  if (mode==1) mean=TMath::Exp(mean);
  if (mode==1) meanL=TMath::Exp(meanL);  // upper truncation
  if (mode==1) meanD=TMath::Exp(meanD);  // lower truncation
  //
  //delete [] ampWithBelow; //return? // RS made on stack
  if ((AliTPCReconstructor::StreamLevel()&AliTPCtracker::kStreamSeeddEdx)) {
    Float_t dEdx0=0,dEdx1=0, dEdxF=0;
    if (1){ /// this is to check with debugger
      dEdx0= TMath::Mean(TMath::Nint((ncl + nclBelowThr)*(up-low)),&ampWithBelow[TMath::Nint((ncl+nclBelowThr)*low)]);
      dEdx1=TMath::Mean(TMath::Nint(ncl*(up-low)),&ampWithBelow[TMath::Nint(ncl*low+nclBelowThr)]);
      dEdxF=TMath::Mean(iclN,&ampWithBelow[icl0]);
      // if (gRandom->Rndm()<0.01 && nclBelowThr>2) printf("%f\t%f\t%f\t%d\n",dEdx0, dEdx1,dEdxF,nclBelowThr);
    }
    if (gRandom->Rndm() < 0.01 && nclBelowThr > 2) printf("dEdx %f\t%f\t%f\t%f\t%d\n", dEdx0, dEdx1, dEdxF, mean, nclBelowThr);   // to be transformed to the DebugStreamer
  }
  //
  if(returnVec){
      (*returnVec)(0) = mean;
      (*returnVec)(1) = rms;
      (*returnVec)(2) = ncl;
      (*returnVec)(3) = Double_t(nclBelowThr)/Double_t(nclBelowThr+ncl);
      (*returnVec)(4) = meanL;
      (*returnVec)(5) = meanD;
      (*returnVec)(6) = mean2;
      (*returnVec)(7) = mean3;
      (*returnVec)(8) = meanS;
      (*returnVec)(9) = nclBelowThr;
  }

  if (returnVal==1) return rms;
  if (returnVal==2) return ncl;
  if (returnVal==3) return Double_t(nclBelowThr)/Double_t(nclBelowThr+ncl);
  if (returnVal==4) return meanL;
  if (returnVal==5) return meanD;
  if (returnVal==6) return mean2;
  if (returnVal==7) return mean3;
  if (returnVal==8) return meanS;
  if (returnVal==9) return nclBelowThr;
  return mean;
}




Float_t  AliTPCseed::CookShape(Int_t type){
  //
  //
  //
 //-----------------------------------------------------------------
  // This funtion calculates dE/dX within the "low" and "up" cuts.
  //-----------------------------------------------------------------
  Float_t means=0;
  Float_t meanc=0;
  for (Int_t i =0; i<kMaxRow;i++)    {
    //
    //AliTPCclusterMI * cl = GetClusterPointer(i);
    //if (cl==0) continue;
    if (GetClusterIndex2(i)<0) continue;	
    //
    const AliTPCTrackerPoints::Point * point = GetTrackPoint(i);
    if (point==0) continue;    
    Float_t rsigmay =  TMath::Sqrt(point->GetSigmaY());
    Float_t rsigmaz =  TMath::Sqrt(point->GetSigmaZ());
    Float_t rsigma =   (rsigmay+rsigmaz)*0.5;
    if (type==0) means+=rsigma;
    if (type==1) means+=rsigmay;
    if (type==2) means+=rsigmaz;
    meanc++;
  }
  Float_t mean = (meanc>0)? means/meanc:0;
  return mean;
}



Int_t  AliTPCseed::RefitTrack(AliTPCseed *seed, AliExternalTrackParam * parin, AliExternalTrackParam * parout){
  //
  // Refit the track
  // return value - number of used clusters
  // 
  //
  const Int_t kMinNcl =10;
  static AliTPCseed trackTmp;
  AliTPCseed* track = &trackTmp;
  track->AliExternalTrackParam::operator=(*seed);
  Int_t sector=-1;
  // reset covariance
  //
  Double_t covar[15];
  for (Int_t i=0;i<15;i++) covar[i]=0;
  covar[0]=10.*10.;
  covar[2]=10.*10.;
  covar[5]=10.*10./(64.*64.);
  covar[9]=10.*10./(64.*64.);
  covar[14]=1*1;
  //

  Float_t xmin=1000, xmax=-10000;
  Int_t imin=158, imax=0;
  for (Int_t i=0;i<kMaxRow;i++) {
    AliTPCclusterMI *c=seed->GetClusterPointer(i);
    if (!c || (seed->GetClusterIndex(i) & 0x8000)) continue; 
    if (sector<0) sector = c->GetDetector();
    if (c->GetX()<xmin) xmin=c->GetX();
    if (c->GetX()>xmax) xmax=c->GetX();
    if (i<imin) imin=i;
    if (i>imax) imax=i;
  }
  if(imax-imin<kMinNcl) {
    //RS    delete track;
    return 0 ;
  }
  // Not succes to rotate
  if (!track->Rotate(TMath::DegToRad()*(sector%18*20.+10.)-track->GetAlpha())) {
    //    delete track;
    return 0;
  }
  //
  //
  // fit from inner to outer row
  //
  AliExternalTrackParam paramIn;
  AliExternalTrackParam paramOut;
  Bool_t isOK=kTRUE;
  Int_t ncl=0;
  //
  //
  //
  for (Int_t i=imin; i<=imax; i++){
    AliTPCclusterMI *c=seed->GetClusterPointer(i);
    if (!c || (seed->GetClusterIndex(i) & 0x8000)) continue; 
    //    if (RejectCluster(c,track)) continue;
    sector = (c->GetDetector()%18);
    if (!track->Rotate(TMath::DegToRad()*(sector%18*20.+10.)-track->GetAlpha())) {
      //continue;
    }
    Double_t r[3]={c->GetX(),c->GetY(),c->GetZ()};
    Double_t cov[3]={0.01,0.,0.01}; //TODO: correct error parametrisation
    if (!track->PropagateTo(r[0])) {
      isOK=kFALSE;
    }
    if ( !((static_cast<AliExternalTrackParam*>(track)->Update(&r[1],cov)))) isOK=kFALSE;
  }
  //RS  if (!isOK) { delete track; return 0;}
  if (!isOK) { return 0;}
  track->AddCovariance(covar);
  //
  //
  //
  for (Int_t i=imax; i>=imin; i--){
    AliTPCclusterMI *c=seed->GetClusterPointer(i);
    if (!c || (seed->GetClusterIndex(i) & 0x8000)) continue;
    //if (RejectCluster(c,track)) continue;
    sector = (c->GetDetector()%18);
    if (!track->Rotate(TMath::DegToRad()*(sector%18*20.+10.)-track->GetAlpha())) {
      //continue;
    }
    Double_t r[3]={c->GetX(),c->GetY(),c->GetZ()};
    Double_t cov[3]={0.01,0.,0.01}; //TODO: correct error parametrisation
    if (!track->PropagateTo(r[0])) {
      isOK=kFALSE;
    }
    if ( !((static_cast<AliExternalTrackParam*>(track)->Update(&r[1],cov)))) isOK=kFALSE;
  }
  //if (!isOK) { delete track; return 0;}
  paramIn = *track;
  track->AddCovariance(covar);
  //
  //
  for (Int_t i=imin; i<=imax; i++){
    AliTPCclusterMI *c=seed->GetClusterPointer(i);
    if (!c || (seed->GetClusterIndex(i) & 0x8000)) continue; 
    sector = (c->GetDetector()%18);
    if (!track->Rotate(TMath::DegToRad()*(sector%18*20.+10.)-track->GetAlpha())) {
      //continue;
    }
    ncl++;
    //if (RejectCluster(c,track)) continue;
    Double_t r[3]={c->GetX(),c->GetY(),c->GetZ()};
    Double_t cov[3]={0.01,0.,0.01}; //TODO: correct error parametrisation
    if (!track->PropagateTo(r[0])) {
      isOK=kFALSE;
    }
    if ( !((static_cast<AliExternalTrackParam*>(track)->Update(&r[1],cov)))) isOK=kFALSE;
  }
  //if (!isOK) { delete track; return 0;}
  paramOut=*track;
  //
  //
  //
  if (parin) (*parin)=paramIn;
  if (parout) (*parout)=paramOut;
  //RS  delete track;
  return ncl;
}



Bool_t AliTPCseed::RefitTrack(AliTPCseed* /*seed*/, Bool_t /*out*/){
  //
  //
  //
  return kFALSE;
}






void  AliTPCseed::GetError(AliTPCclusterMI* cluster, AliExternalTrackParam * param, 
				  Double_t& erry, Double_t &errz)
{
  //
  // Get cluster error at given position
  //
  AliTPCClusterParam *clusterParam = AliTPCcalibDB::Instance()->GetClusterParam();
  Double_t tany,tanz;  
  Double_t snp1=param->GetSnp();
  tany=snp1/TMath::Sqrt((1.-snp1)*(1.+snp1));
  //
  Double_t tgl1=param->GetTgl();
  tanz=tgl1/TMath::Sqrt((1.-snp1)*(1.+snp1));
  //
  Int_t padSize = 0;                          // short pads
  if (cluster->GetDetector() >= 36) {
    padSize = 1;                              // medium pads 
    if (cluster->GetRow() > 63) padSize = 2; // long pads
  }

  erry  = clusterParam->GetError0Par( 0, padSize, (250.0 - TMath::Abs(cluster->GetZ())), TMath::Abs(tany) );
  errz  = clusterParam->GetError0Par( 1, padSize, (250.0 - TMath::Abs(cluster->GetZ())), TMath::Abs(tanz) );
}


void  AliTPCseed::GetShape(AliTPCclusterMI* cluster, AliExternalTrackParam * param, 
				  Double_t& rmsy, Double_t &rmsz)
{
  //
  // Get cluster error at given position
  //
  AliTPCClusterParam *clusterParam = AliTPCcalibDB::Instance()->GetClusterParam();
  Double_t tany,tanz;  
  Double_t snp1=param->GetSnp();
  tany=snp1/TMath::Sqrt((1.-snp1)*(1.+snp1));
  //
  Double_t tgl1=param->GetTgl();
  tanz=tgl1/TMath::Sqrt((1.-snp1)*(1.+snp1));
  //
  Int_t padSize = 0;                          // short pads
  if (cluster->GetDetector() >= 36) {
    padSize = 1;                              // medium pads 
    if (cluster->GetRow() > 63) padSize = 2; // long pads
  }

  rmsy  = clusterParam->GetRMSQ( 0, padSize, (250.0 - TMath::Abs(cluster->GetZ())), TMath::Abs(tany), TMath::Abs(cluster->GetMax()) );
  rmsz  = clusterParam->GetRMSQ( 1, padSize, (250.0 - TMath::Abs(cluster->GetZ())), TMath::Abs(tanz) ,TMath::Abs(cluster->GetMax()));
}



Double_t AliTPCseed::GetQCorrGeom(Float_t ty, Float_t tz){
  //Geoetrical
  //ty    - tangent in local y direction
  //tz    - tangent 
  //
  Float_t norm=TMath::Sqrt(1+ty*ty+tz*tz);
  return norm;
}

Double_t AliTPCseed::GetQCorrShape(Int_t ipad, Int_t type,Float_t z, Float_t ty, Float_t tz, Float_t /*q*/, Float_t /*thr*/){
  //
  // Q normalization
  //
  // return value =  Q Normalization factor
  // Normalization - 1 - shape factor part for full drift          
  //                 1 - electron attachment for 0 drift

  // Input parameters:
  //
  // ipad - 0 short pad
  //        1 medium pad
  //        2 long pad
  //
  // type - 0 qmax
  //      - 1 qtot
  //
  //z     - z position (-250,250 cm)
  //ty    - tangent in local y direction
  //tz    - tangent 
  //

  AliTPCClusterParam * paramCl = AliTPCcalibDB::Instance()->GetClusterParam();
  AliTPCParam   * paramTPC = AliTPCcalibDB::Instance()->GetParameters();
 
  if (!paramCl) return 1;
  //
  Double_t dr =  250.-TMath::Abs(z); 
  Double_t sy =  paramCl->GetRMS0( 0,ipad, dr, TMath::Abs(ty));
  Double_t sy0=  paramCl->GetRMS0(0,ipad, 250, 0);
  Double_t sz =  paramCl->GetRMS0( 1,ipad, dr, TMath::Abs(tz));
  Double_t sz0=  paramCl->GetRMS0(1,ipad, 250, 0);

  Double_t sfactorMax = TMath::Sqrt(sy0*sz0/(sy*sz));

 
  Double_t dt = 1000000*(dr/paramTPC->GetDriftV());  //time in microsecond
  Double_t attProb = TMath::Exp(-paramTPC->GetAttCoef()*paramTPC->GetOxyCont()*dt);
  //
  //
  if (type==0) return sfactorMax*attProb;

  return attProb;


}


/*
//_______________________________________________________________________
Float_t AliTPCseed::GetTPCClustInfo(Int_t nNeighbours, Int_t type, Int_t row0, Int_t row1, TVectorT<float> *returnVec)
{
  //
  // TPC cluster information
  // type 0: get fraction of found/findable clusters with neighbourhood definition
  //      1: found clusters
  //      2: findable (number of clusters above and below threshold)
  //
  // definition of findable clusters:
  //            a cluster is defined as findable if there is another cluster
  //           within +- nNeighbours pad rows. The idea is to overcome threshold
  //           effects with a very simple algorithm.
  //

  const Float_t kClusterShapeCut = 1.5; // IMPPRTANT TO DO: move value to AliTPCRecoParam
  const Float_t ktany = TMath::Tan(TMath::DegToRad()*10);
  const Float_t kedgey =3.;
  
  Float_t ncl = 0;
  Float_t nclBelowThr = 0; // counts number of clusters below threshold

  for (Int_t irow=row0; irow<row1; irow++){
    AliTPCclusterMI* cluster = GetClusterPointer(irow);

    if (!cluster && irow > 1 && irow < 157) {
      Bool_t isClBefore = kFALSE;
      Bool_t isClAfter  = kFALSE;
      for(Int_t ithres = 1; ithres <= nNeighbours; ithres++) {
	AliTPCclusterMI * clusterBefore = GetClusterPointer(irow - ithres);
	if (clusterBefore) isClBefore = kTRUE;
	AliTPCclusterMI * clusterAfter  = GetClusterPointer(irow + ithres);
	if (clusterAfter) isClAfter = kTRUE;
      }
      if (isClBefore && isClAfter) nclBelowThr++;
    }
    if (!cluster) continue;
    //
    //
    if (TMath::Abs(cluster->GetY())>cluster->GetX()*ktany-kedgey) continue; // edge cluster
    //
    const AliTPCTrackerPoints::Point * point = GetTrackPoint(irow);
    if (point==0) continue;    
    Float_t rsigmay = TMath::Sqrt(point->GetSigmaY());
    if (rsigmay > kClusterShapeCut) continue;
    //
    if (cluster->IsUsed(11)) continue; // remove shared clusters for PbPb
    ncl++;
  }
  if(returnVec->GetNoElements != 3){
      returnVec->ResizeTo(3);
  }
  Float_t nclAll = nclBelowThr+ncl;
  returnVec(0) = nclAll>0?ncl/nclAll:0;
  returnVec(1) = ncl;
  returnVec(2) = nclAll;

  if(ncl<10)
    return 0;
  if(type==0) 
    if(nclAll>0)
      return ncl/nclAll;
  if(type==1)
    return ncl;
  if(type==2)
    return nclAll;
  return 0;
}
*/
//_______________________________________________________________________
Int_t AliTPCseed::GetNumberOfClustersIndices() {
  Int_t ncls = 0;
  for (int i=0; i < kMaxRow; i++) {
    if ((fIndex[i] & 0x8000) == 0)
      ncls++;
  }
  return ncls;
}

//_______________________________________________________________________
void AliTPCseed::Clear(Option_t*)
{
  // formally seed may allocate memory for clusters (althought this should not happen for 
  // the seeds in the pool). Hence we need this method for fwd. compatibility
  if (fClusterPointer) {
    if (fClusterOwner) for (int i=kMaxRow;i--;) {delete fClusterPointer[i]; fClusterPointer[i] = 0;}
    delete[] fClusterPointer;
    fNClStore = 0;
  }
  fTrackPointsArr.Clear();
  ResetBit(0xffffffff);
}

//_______________________________________________________________________
void AliTPCseed::SetClusterPointer(Int_t irow, AliTPCclusterMI* cl) 
{
  // if needed, create array and set the pointer
  if (!fClusterPointer) {
    fClusterPointer = new AliTPCclusterMI*[kMaxRow];
    fNClStore = kMaxRow;
    memset(fClusterPointer,0,kMaxRow*sizeof(AliTPCclusterMI*));
  }
  fClusterPointer[irow]=cl;
}


TObject* AliTPCseed::Clone(const char* /*newname*/) const
{
  // temporary override TObject::Clone to avoid crashes in reco
  AliTPCseed* src = (AliTPCseed*)this;
  AliTPCseed* dst = new AliTPCseed(*src,fClusterOwner);
  return dst;
}                                                                                                                                                                    

//__________________________________________________
void   AliTPCseed::TagSuppressSharedClusters()
{
  // RS: special function to flag the cluster if not flagged yet or remove its pointer if flagged
  // This is needed for deletion of the TPCseeds stored in the ESDfriendTracks in a safe way, avoiding
  // deletion of shared clusters. DO not use this method except from 
  // AliESDfriend->DeleteTracksSafe->TagSuppressSharedObjectsBeforeDeletion(): the clusters are DISABLED
  // 
  if (!fClusterOwner || !fClusterPointer) return;
  AliTPCclusterMI* cl=0;
  for (int i=kMaxRow;i--;) {
    if (!(cl=fClusterPointer[i])) continue;
    if (cl->IsDisabled()) fClusterPointer[i] = 0;
    else cl->Disable();
  }
}
