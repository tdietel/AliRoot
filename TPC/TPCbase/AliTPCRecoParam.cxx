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


/// \class AliTPCRecoParam
/// \brief Class with TPC reconstruction parameters
///
/// The reconstruction parameters are used in the AliTPCclusterer and AliTPCtracker
///
/// They are retrieved:
/// 0. User speciefied it in reconstruction macro
/// 1. if (not 0) from OCDB  - AliTPCcalibDB::GetRecoParam(eventtype)
/// 2. if (not 0 or 1) default parameter - High flux enevironment used
///
/// Setting for systematic errors addition:
/// [0] - systematic RMSY
/// [1] - systematic RMSZ
/// [2] - systematic RMSSNP
/// [3] - systematic RMSTheta
/// [4] - systematic RMSCuravture -  systematic error in 1/cm not in 1/pt
///
/// How to add it example - 3 mm systematic error y, 3 cm systematic error z (drift)
/// ~~~{.cxx}
/// Double_t sysError[5]={0.3,3, 0.3/150., 3./150.,0.3/(150*150.)}
/// param->SetSystematicError(sysError);
/// ~~~


#include "AliTPCRecoParam.h"
#include "AliLumiTools.h"

/// \cond CLASSIMP
ClassImp(AliTPCRecoParam)
/// \endcond

TVectorD* AliTPCRecoParam::fgSystErrClustCustom = 0;  // normally will be set as AliTPCReconstructor::SetSystematicErrorCluster alias
TVectorD* AliTPCRecoParam::fgPrimaryDCACut = 0;       // normally will be set as AliTPCReconstructor::SetPrimaryDCACut alias

Bool_t AliTPCRecoParam::fgUseTimeCalibration=kTRUE; // flag usage the time dependent calibration
                                      // to be switched off for pass 0 reconstruction
                                      // Use static function, other option will be to use
                                      // additional specific storage ?

//_____________________________________________________________________________
AliTPCRecoParam::AliTPCRecoParam():
  AliDetectorRecoParam(),
  fUseHLTClusters(4),  // use HLTorRAW data
  fUseHLTPreSeeding(0), // no pre-seeding for now
  fBClusterSharing(kTRUE),
  fCtgRange(1.05),
  fMaxSnpTracker(0.95),
  fMaxSnpTrack(0.999),
  fUseOuterDetectors(kFALSE),
  fMaxChi2TPCTRD(36),     // maximal allowed chi2 between the TRD in and TPC out to be accepted for refit
  fMaxChi2TPCITS(36),     // maximal allowed chi2 between the ITS in and TPC out to be accepted for backpropagation
  fUseOulierClusterFilter(0),  // swith to use outlier cluster filter
  fDumpSignal(kFALSE),
  fFirstBin(0),
  fLastBin(-1),
  fBCalcPedestal(kFALSE),
  fBDoUnfold(kTRUE),
  fDumpAmplitudeMin(100),
  fMaxNoise(2.),
  //
  fUseOnePadCluster(kTRUE),
  fUseHLTOnePadCluster(kFALSE),
  fMinMaxCutAbs(4.),
  fMinLeftRightCutAbs(6.),
  fMinUpDownCutAbs(6.),
  //
  fMinMaxCutSigma(4.),
  fMinLeftRightCutSigma(7.),
  fMinUpDownCutSigma(8.),
  fMaxC(0.3),
  fBSpecialSeeding(kFALSE),
  fBKinkFinder(kTRUE),
  fLastSeedRowSec(120),
  fSeedGapPrim(6),
  fSeedGapSec(6),
  fUseFieldCorrection(0),      // use field correction
  fUseComposedCorrection(kFALSE),      // use field correction
  fUseRPHICorrection(0),      // use rphi correction
  fUseRadialCorrection(0),    // use radial correction
  fUseQuadrantAlignment(0),   // use quadrant alignment
  fUseSectorAlignment(0),     // use sector alignment
  fUseDriftCorrectionTime(1), // use drift correction time
  fUseDriftCorrectionGY(1),   // use drif correction global y
  fUseGainCorrectionTime(0),  // use gain correction time
  fUseExBCorrection(1),  // use ExB correction
  fUseMultiplicityCorrectionDedx(kTRUE), // use Dedx multiplicity correction
  fUseAlignmentTime(kTRUE),              // use time dependent alignment correction
  fUseIonTailCorrection(0),   // no ion tail correction for now
  fIonTailCorrection(1),      ///< ion tail tail correction factor - NEW SINCE 2018- additonal scaling correcting for imperfect knowledge of the integral of ion tail - shoudl be ~ 1
  fIonTailCorrectionTimeScale(1.0),      ///< ion tail tail correction factor time stretching - new Since 2019 - default value=1
  fCrosstalkCorrection(0),   // crosstalk correction factor (from each signal substracted by (mean signal in wite patch)xfCrosstalkCorrection) - Effect important only after removing oc capacitors in 2012
  fCrosstalkCorrectionMissingCharge(1),   // crosstalk correction factor - missing charge (from each signal substracted by (mean signal in wite patch)xfCrosstalkCorrection) - Effect important only after removing oc capacitors in 2012
  fCrosstalkIonTail(kFALSE),            /// < flag calculate crosstalk for ion tail
 //
  fUseTotCharge(kTRUE),          // switch use total or max charge
  fMinFraction(0.01),           // truncated mean - lower threshold
  fMaxFaction(0.7),            // truncated mean - upper threshold
  fNeighborRowsDedx(2),           // neighbour rows for below threshold dEdx calculation
  fMissingClusterdEdxFraction(0), /// missing cluster will be replaced by truncated mean of observed clusters 1+ Nclmissing*fMissingClusterdEdxFraction; - - default 0 to be back compatible
  fGainCorrectionHVandPTMode(0), // switch for the usage of GainCorrectionHVandPT (see AliTPCcalibDB::GetGainCorrectionHVandPT
  fAccountDistortions(kFALSE),
  fSkipTimeBins(5),              // number of time bins to be skipped (corrupted signal druing gating opening)
  fUseTOFCorrection(kTRUE),
  fUseCorrectionMap(kFALSE),
  fCorrMapTimeDepMethod(kCorrMapInterpolation),
  fUseLumiType(AliLumiTools::kLumiCTP),
  fUseMapLumiInfoCOG(kFALSE),
  fSystCovAmplitude(2.0),
  fDistFluctCorrelation(0.99),
  fDistortionFluctMCAmp(1.0),
  fMinDistFluctMCRef(0.0),
  fDistFluctUncorrFracMC(0.0),
  fSystErrClInnerRegZ(0),
  fSystErrClInnerRegZSigInv(0),
  fUseClusterErrordEdxCorrection(false),     ///< switch to use the dEdx correction  - default is false - for  back compatibility
  fUseClusterErrordEdxMultCorrection(false),     ///< switch to use the dEdx, multiplicity  correction  - for  back compatibility
  fClusterErrorMatrix(8,10),                     /// < cluster error matrix
  fClusterNSigma2Cut(3,3),                       /// < n sigma cluster/track cut
  fUseSystematicCorrelation(kTRUE)
{
  /// constructor

  SetName("TPC");
  SetTitle("TPC");
  for (Int_t i=0;i<5;i++) fSystematicErrors[i]=0;
  // systematic error parameterization at INNER wall of the TPC
  fSystematicErrorClusterInner[0]=0.5;   // 0.5 cm
  fSystematicErrorClusterInner[1]=5;     // 5 cm slope
  //
  fSystematicErrorCluster[0]=0;   // sy cluster error
  fSystematicErrorCluster[1]=0;   // sz cluster error
  //
  fDistortionFractionAsErrorYZ[0] = -1.; // fraction of used distortion correction is used as an error (if positive)
  fDistortionFractionAsErrorYZ[1] = -1.; // fraction of used distortion correction is used as an error (if positive)

  fDistDispFractionAsErrorYZ[0] = -1; // fraction of used distortion correction is used as an error (if positive)
  fDistDispFractionAsErrorYZ[1] = -1; // fraction of used distortion correction is used as an error (if positive)
  //
  fCutSharedClusters[0]=0.5; // maximal allowed fraction of shared clusters - shorter track
  fCutSharedClusters[1]=0.25; // maximal allowed fraction of shared clusters - longer  track
  fClusterMaxRange[0]=1;     // y - pad      range
  fClusterMaxRange[1]=1;     // z - time bin range
  fKinkAngleCutChi2[0]=9;    // angular cut for kink finder - to create a kink
                             // ~ about 5 % rate  for high pt kink finder
  fKinkAngleCutChi2[1]=12;    // angular cut for kink finder - to use the partial track                             // form kink
                             // ~ about 2 % rate  for high pt kink finder
  //
  SetBadPadMaxDistXYZD(999.,999.,999.,999.); // by default accept any distortions
  SetBadClusMaxErrYZ(999.,999.);        // by default accept any errors
  //
  fClusterNSigma2Cut(0,0)=1; fClusterNSigma2Cut(0,1)=6.25; fClusterNSigma2Cut(0,2)=2*6.25;         // edge n sigma cut
  fClusterNSigma2Cut(1,0)=9; fClusterNSigma2Cut(1,1)=9; fClusterNSigma2Cut(1,2)=32;                // normal clusters cut
  fClusterNSigma2Cut(2,0)=6.25; fClusterNSigma2Cut(2,1)=6.25; fClusterNSigma2Cut(2,2)=2*6.25;      // unfolded clusters cut
  SetClusterErrorParam();
}

//_____________________________________________________________________________
AliTPCRecoParam::AliTPCRecoParam(const AliTPCRecoParam& src)
{
  // copy c-tor
  memcpy(this,&src,sizeof(AliTPCRecoParam)); // make 1st a shallow copy
  // 
  // now treat the pointers
  if (fSystErrClInnerRegZ)       fSystErrClInnerRegZ       = new TVectorF(*fSystErrClInnerRegZ);
  if (fSystErrClInnerRegZSigInv) fSystErrClInnerRegZSigInv = new TVectorF(*fSystErrClInnerRegZSigInv);
}

//_____________________________________________________________________________
AliTPCRecoParam& AliTPCRecoParam::operator=(const AliTPCRecoParam& src)
{
  // assignment operator
  if (this!=&src) {
    if (fSystErrClInnerRegZ) delete fSystErrClInnerRegZ;
    if (fSystErrClInnerRegZSigInv) delete fSystErrClInnerRegZSigInv;
    memcpy(this,&src,sizeof(AliTPCRecoParam)); // make 1st a shallow copy
    // now treat the pointers
    if (fSystErrClInnerRegZ)       fSystErrClInnerRegZ       = new TVectorF(*fSystErrClInnerRegZ);
    if (fSystErrClInnerRegZSigInv) fSystErrClInnerRegZSigInv = new TVectorF(*fSystErrClInnerRegZSigInv);
  }
  return *this;
}

//_____________________________________________________________________________
AliTPCRecoParam::~AliTPCRecoParam()
{
  /// destructor
  delete fSystErrClInnerRegZ;
  delete fSystErrClInnerRegZSigInv;
}

void AliTPCRecoParam::Print(const Option_t* /*option*/) const{
  ///

  AliTPCRecoParam::Dump();
  printf("Systematic errors:\n");
  const char * cherrs[5]={"sy=","sz=","ssnp=","stheta=","s1pt="};
  for (Int_t i=0; i<5; i++){
    printf("%s%f\n",cherrs[i],fSystematicErrors[i]);
  }
}


AliTPCRecoParam *AliTPCRecoParam::GetLowFluxParam(){
  /// make default reconstruction  parameters for low  flux env.

  AliTPCRecoParam *param = new AliTPCRecoParam;
  param->fCtgRange = 10;
  param->fFirstBin = 0;
  param->fLastBin  = 1000;
  param->SetName("Low Flux");
  param->SetTitle("Low Flux");
  return param;
}

AliTPCRecoParam *AliTPCRecoParam::GetHighFluxParam(){
  /// make reco parameters for high flux env.

  AliTPCRecoParam *param = new AliTPCRecoParam;
  param->fCtgRange = 1.05;
  param->fFirstBin = 0;
  param->fLastBin  = 1000;
  param->fUseTotCharge=kFALSE;
  param->SetName("High Flux");
  param->SetTitle("High Flux");
  return param;
}

AliTPCRecoParam *AliTPCRecoParam::GetHLTParam(){
  /// make reco parameters for high flux env.

  AliTPCRecoParam *param = new AliTPCRecoParam;
  param->fCtgRange = 1.05;
  param->fFirstBin = 80;
  param->fLastBin  = 1000;
  param->fMaxSnpTracker = 0.9;
  param->fMaxC          = 0.06;
  //
  param->SetName("Hlt Param");
  param->SetTitle("Hlt Param");
  param->fBKinkFinder   = kFALSE;
  return param;
}

AliTPCRecoParam *AliTPCRecoParam::GetLaserTestParam(Bool_t bPedestal){
  /// special setting for laser

  AliTPCRecoParam *param = new AliTPCRecoParam;
  param->fDumpSignal=kTRUE;
  param->fCtgRange = 10.05;
  param->fFirstBin = 0;
  param->fLastBin  = 1000;
  param->fBCalcPedestal = bPedestal;
  param->fBDoUnfold     = kFALSE;
  param->fDumpAmplitudeMin = 150;
  param->fBKinkFinder   = kFALSE;
  param->fMaxSnpTracker = 0.98;
  param->fMaxC          = 0.02;
  param->fBSpecialSeeding = kTRUE;
  param->fUseTOFCorrection=kFALSE;
  param->fUseHLTClusters=1; // always RAW data
  //
  //
  param->SetName("Laser Flux");
  param->SetTitle("Laser Flux");
  return param;
}

AliTPCRecoParam *AliTPCRecoParam::GetCosmicTestParam(Bool_t bPedestal){
  /// special setting for cosmic

  AliTPCRecoParam *param = new AliTPCRecoParam;
  param->fDumpSignal=kTRUE;
  param->fCtgRange = 10.05;    // full TPC
  param->fFirstBin = 60;
  param->fLastBin  = 1000;
  param->fBCalcPedestal = bPedestal;
  param->fBDoUnfold     = kFALSE;
  param->fBSpecialSeeding = kTRUE;
  param->fMaxC          = 0.07;
  param->fBKinkFinder   = kFALSE;
  param->fUseTOFCorrection =kFALSE;
  param->SetName("Cosmic Flux");
  param->SetTitle("Cosmic Flux");

  return param;
}

/// Set default cluster error estimator - correcting for the dEdx and multiplicity
void AliTPCRecoParam::SetClusterErrorParam(){
  /*
  errParam matrix

  Low mutplicity:
  erry2LM=(sy2+0.04*tanPhi2)*(0.5+mdEdx+0.3*mQ)*0.25;
  errz2LM=(sz2)*(0.5+mdEdx+0.3*mQ)*0.5;
  -->
  errParam(0,i) = (p0+p1*mdEdx+p2*mQ)*p3*(sy2+p4*tanPhi2)      // erry2LM
  errParam(1,i) = (p0+p1*mdEdx+p2*mQ)*p3*(sz2+p4*tanTheta2)    // errz2LM

  High multiplicity:
  erry2OccudEdx = 0.12*0.12*occu6N*(mdEdx+0.3*mQ);
  erry2Ratio   = 0.07*negRatio+0.15*posRatio;
  erry2HM      = (erry2OccudEdx+erry2Ratio)*(0.3+tanPhi2);
  errz2OccudEdx = 0.09*0.09*occu6N*(mdEdx+0.3*mQ);
  errz2Ratio   = 0.01*negRatio+0.05*posRatio;
  errz2HM      = (errz2OccudEdx+errz2Ratio)*(1+tgl2)*0.5;

  errParam(2,i) = occu6N*(p0+p1*mdEdx+p2*mQ)*p3   // erry2OccudEdx
  errParam(3,i) = occu6N*(p0+p1*mdEdx+p2*mQ)*p3   // errz2OccudEdx
//
  errParam(4,i) = p0*negRatio+p1*posRatio         // erry2Ratio
  errParam(5,i) = p0*negRatio+p1*posRatio         // errz2Ratio
//
  errParam(6,i) = (erry2OccudEdx+erry2Ratio)*(p0+p1*tanPhi2); //erry2HM
  errParam(7,i) = (errz2OccudEdx+errz2Ratio)*(p0+p1*tgl2); //errz2HM
*/
  TMatrixF &e=fClusterErrorMatrix;
  e(0,0)=0.5; e(0,1)=1; e(0,2)=0.15; e(0,3)=0.25; e(0,4)=0.04;    /// (p0+p1*mdEdx+p2*mQ)*p3*(sy2+p4*tanPhi2)      // erry2LM
  e(1,0)=0.5; e(1,1)=1; e(1,2)=0.15; e(1,3)=0.5;  e(1,4)=0;    /// (p0+p1*mdEdx+p2*mQ)*p3*(sy2+p4*tgl2)         // errz2LM
  //
  e(2,0)=0; e(2,1)=1; e(2,2)=0.3; e(2,3)=0.12*0.12;          /// occu6N*(p0+p1*mdEdx+p2*mQ)*p3   // erry2OccudEdx 0.12*0.12*occu6N*(mdEdx+0.3*mQ);
  e(3,0)=0; e(3,1)=1; e(3,2)=0.3; e(3,3)=0.09*0.09;          /// occu6N*(p0+p1*mdEdx+p2*mQ)*p3   // errz2OccudEdx = 0.09*0.09*occu6N*(mdEdx+0.3*mQ);
  //
  e(4,0)= 0.07; e(4,1)=0.15;                                /// p0*negRatio+p1*posRatio         // erry2Ratio  =0.07*negRatio+0.15*posRatio;
  e(5,0)= 0.01; e(5,1)=0.05;                                /// p0*negRatio+p1*posRatio         // 0.01*negRatio+0.05*posRatio;
  //
  e(6,0)= 0.3; e(6,1)=1; e(6,2)=1;                         /// errParam(6,i) = (erry2OccudEdx+erry2Ratio)*(p0+p1*tanPhi2); //erry2HM      = (erry2OccudEdx+erry2Ratio)*(0.3+tanPhi2);
  e(7,0)= 1;   e(7,1)=1; e(7,2)=0.5;                         ///errParam(7,i) = (errz2OccudEdx+errz2Ratio)*(p0+p1*tgl2); //errz2HM (errz2OccudEdx+errz2Ratio)*(1+tgl2)*0.5;

}


Bool_t  AliTPCRecoParam::GetUseTimeCalibration(){
  /// get

  return fgUseTimeCalibration;
}
void    AliTPCRecoParam::SetUseTimeCalibration(Bool_t useTimeCalibration) {
  /// set

  fgUseTimeCalibration = useTimeCalibration;
}

