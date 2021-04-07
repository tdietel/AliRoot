#ifndef ALITPCRECOPARAM_H
#define ALITPCRECOPARAM_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/// \class AliTPCRecoParam
/// \brief Class with TPC reconstruction parameters


#include "AliDetectorRecoParam.h"
#include "TVectorF.h"
#include "TVectorD.h"
#include  "TMatrixF.h"

class AliTPCRecoParam : public AliDetectorRecoParam
{
 public:
  enum {                       // methods used for correction maps time dependence
    kCorrMapInterpolation         // interpolate between 2 nearest timebins maps
    ,kCorrMapNoScaling            // no scaling, use just the single map matching to timestamp
    ,kCorrMapGlobalScalingLumi // scale current map by ratio of inst_lumi/<lumi_timebin>
  };
 public:
  AliTPCRecoParam();
  AliTPCRecoParam(const AliTPCRecoParam& src);
  AliTPCRecoParam& operator=(const AliTPCRecoParam& src);
  virtual ~AliTPCRecoParam();
  virtual void Print(const Option_t* option="") const;
  static   Bool_t  GetUseTimeCalibration();
  static   void    SetUseTimeCalibration(Bool_t useTimeCalibration);

  void     SetUseHLTClusters(Int_t useHLTClusters){fUseHLTClusters=useHLTClusters;}
  Int_t    GetUseHLTClusters() const {return fUseHLTClusters;}
  void     SetUseHLTPreSeeding(Int_t useHLTPreSeeding){fUseHLTPreSeeding=useHLTPreSeeding;}
  Int_t    GetUseHLTPreSeeding() const {return fUseHLTPreSeeding;}
  void     SetClusterSharing(Bool_t sharing){fBClusterSharing=sharing;}
  Bool_t   GetClusterSharing() const {return fBClusterSharing;}
  Double_t GetCtgRange() const     { return fCtgRange;}
  Double_t GetMaxSnpTracker() const{ return fMaxSnpTracker;}
  Double_t GetMaxSnpTrack() const  { return fMaxSnpTrack;}
  Bool_t   GetUseOuterDetectors() const { return fUseOuterDetectors;}
  void     SetUseOuterDetectors(Bool_t flag)  { fUseOuterDetectors=flag;}
  void     SetMaxChi2TPCTRD(Double_t maxChi2){fMaxChi2TPCTRD=maxChi2;}
  Double_t GetMaxChi2TPCTRD() const {return fMaxChi2TPCTRD;}
  void     SetMaxChi2TPCITS(Double_t maxChi2){fMaxChi2TPCITS=maxChi2;}
  Double_t GetMaxChi2TPCITS() const {return fMaxChi2TPCITS;}
  Double_t GetCutSharedClusters(Int_t index)const { return fCutSharedClusters[index];}
  void  SetCutSharedClusters(Int_t index, Float_t value){ fCutSharedClusters[index]=value;}
  Int_t GetClusterMaxRange(Int_t index)const { return fClusterMaxRange[index];}
  void     SetClusterMaxRange(Int_t index, Int_t value){ fClusterMaxRange[index]=value;}
  //
  Int_t    GetAccountDistortions()               const {return fAccountDistortions;}
  void     SetAccountDistortions(Int_t v)              {fAccountDistortions = v;}
  Bool_t   GetUseCorrectionMap()                 const {return fUseCorrectionMap;}
  void     SetUseCorrectionMap(Bool_t v=kTRUE)         {fUseCorrectionMap = v;}

  // accounting for systematic error on track cov.matrix level
  Float_t GetSystCovAmplitude()           const {return fSystCovAmplitude;}
  Float_t GetDistFluctCorrelation()       const {return fDistFluctCorrelation;}
  void    SetSystCovAmplitude(float v)          {fSystCovAmplitude = v;}
  void    SetDistFluctCorrelation(float v)      {fDistFluctCorrelation = v;}
  //
  // for simulation of distortions fluctuations 
  Float_t GetDistortionFluctMCAmp()           const {return fDistortionFluctMCAmp;}
  void    SetDistortionFluctMCAmp(float v=1.)   {fDistortionFluctMCAmp = v;}
  Float_t GetMinDistFluctMCRef()              const {return fMinDistFluctMCRef;}
  void    SetMinDistFluctMCRef(Float_t v=0)  {fMinDistFluctMCRef = v;}
  Float_t GetDistFluctUncorrFracMC()          const {return fDistFluctUncorrFracMC;}
  void    SetDistFluctUncorrFracMC(Float_t v=0)  {fDistFluctUncorrFracMC = v;}

  //
  // Outlier filtering configuration
  //
  Int_t   GetUseOulierClusterFilter() const { return fUseOulierClusterFilter;}  // swith to use outlier cluster filter
  void    SetUseOulierClusterFilter(Int_t value){ fUseOulierClusterFilter=value;}  // swith to use outlier cluster filter
  //
  Bool_t   DumpSignal()     const  { return fDumpSignal;}
  void     SetTimeInterval(Int_t first, Int_t last) { fFirstBin=first, fLastBin =last;}
  Int_t    GetFirstBin() const     { return fFirstBin;}
  Int_t    GetLastBin() const      { return fLastBin;}
  void     SetTimeBinRange(Int_t first, Int_t last){ fFirstBin = first; fLastBin = last;}
  Bool_t   GetCalcPedestal()       const  { return fBCalcPedestal;}
  Bool_t   GetDoUnfold()           const  { return fBDoUnfold;}
  void     SetDoUnfold(Bool_t unfold)     { fBDoUnfold = unfold;}
  Float_t  GetDumpAmplitudeMin()   const  { return fDumpAmplitudeMin;}
  Float_t  GetMaxNoise()           const  { return fMaxNoise;}
  //
  Int_t    GetUseOnePadCluster()   const  { return fUseOnePadCluster;}
  Bool_t   GetUseHLTOnePadCluster()const  { return fUseHLTOnePadCluster;}
  Float_t  GetMinMaxCutAbs()       const  { return fMinMaxCutAbs; }
  Float_t  GetMinLeftRightCutAbs() const  { return fMinLeftRightCutAbs;}  // minimal amplitude left right - PRF
  Float_t  GetMinUpDownCutAbs()    const  { return fMinUpDownCutAbs;}  // minimal amplitude up-down - TRF
  Float_t  GetMinMaxCutSigma()       const  { return fMinMaxCutSigma; }
  Float_t  GetMinLeftRightCutSigma() const  { return fMinLeftRightCutSigma;}  // minimal amplitude left right - PRF
  Float_t  GetMinUpDownCutSigma()    const  { return fMinUpDownCutSigma;}  // minimal amplitude up-down - TRF
  //
  void SetUseOnePadCluster(Int_t use)      {   fUseOnePadCluster = use;}
  void SetUseHLTOnePadCluster(Bool_t use)  {   fUseHLTOnePadCluster = use;}
  void SetMinMaxCutAbs(Float_t th)         {   fMinMaxCutAbs=th; }
  void SetMinLeftRightCutAbs(Float_t th)   {   fMinLeftRightCutAbs=th;}  // minimal amplitude left right - PRF
  void SetMinUpDownCutAbs(Float_t th)      {   fMinUpDownCutAbs=th;}  // minimal amplitude up-down - TRF
  void SetMinMaxCutSigma(Float_t th)       {   fMinMaxCutSigma=th; }
  void SetMinLeftRightCutSigma(Float_t th) {   fMinLeftRightCutSigma=th;}  // minimal amplitude left right - PRF
  void SetMinUpDownCutSigma(Float_t th)    {   fMinUpDownCutSigma=th;}  // minimal amplitude up-down - TRF
  void  SetUseTotCharge(Bool_t flag) {fUseTotCharge = flag;}
  void  SetCtgRange(Double_t ctgRange) {fCtgRange = ctgRange;}
  void  SetUseMultiplicityCorrectionDedx(Bool_t flag) {fUseMultiplicityCorrectionDedx = flag;}

  void  SetUseAlignmentTime(Bool_t flag) {fUseAlignmentTime = flag;}
  void  SetNeighborRowsDedx(Int_t nRows) {fNeighborRowsDedx = nRows;}
  void SetCorrectionHVandPTMode(Int_t value){ fGainCorrectionHVandPTMode =value;}
  void SetSkipTimeBins(Double_t value) {fSkipTimeBins=value;}
  //
  Int_t    GetLastSeedRowSec()       const  { return fLastSeedRowSec;}
  Int_t    GetSeedGapPrim()        const  { return fSeedGapPrim;}
  Int_t    GetSeedGapSec()         const  { return fSeedGapSec;}
  void     SetDoKinks(Bool_t on)   { fBKinkFinder=on; }
  Bool_t   GetDoKinks() const      { return fBKinkFinder;}
  Double_t GetKinkAngleCutChi2(Int_t index) const {return fKinkAngleCutChi2[index];}
  void     SetKinkAngleCutChi2(Int_t index,Double_t value) {fKinkAngleCutChi2[index]=value;}
  void     SetSeedGapPrim(Int_t seedGapPrim)         { fSeedGapPrim = seedGapPrim;}
  void     SetSeedGapSec(Int_t seedGapSec)          { fSeedGapSec  = seedGapSec;}
  Float_t  GetMaxC()    const      { return fMaxC;}
  Bool_t   GetSpecialSeeding() const { return fBSpecialSeeding;}
  //
  //

  //
  // Correction setup
  //
  void  SetUseFieldCorrection(Int_t flag){fUseFieldCorrection=flag;}
  void  SetUseComposedCorrection(Bool_t flag){fUseComposedCorrection=flag;}
  void  SetUseRPHICorrection(Int_t flag){fUseRPHICorrection=flag;}
  void  SetUseRadialCorrection(Int_t flag){fUseRadialCorrection=flag;}
  void  SetUseQuadrantAlignment(Int_t flag){fUseQuadrantAlignment=flag;}
  void  SetUseSectorAlignment(Int_t flag){fUseSectorAlignment=flag;}
  void  SetUseDriftCorrectionTime(Int_t flag){fUseDriftCorrectionTime=flag;}
  void  SetUseDriftCorrectionGY(Int_t flag){fUseDriftCorrectionGY=flag;}
  void  SetUseGainCorrectionTime(Int_t flag){fUseGainCorrectionTime=flag;}
  void  SetUseExBCorrection(Int_t flag){fUseExBCorrection=flag;}
  void  SetUseTOFCorrection(Bool_t flag) {fUseTOFCorrection = flag;}
  void  SetUseIonTailCorrection(Int_t flag) {fUseIonTailCorrection = flag;}
  void  SetIonTailCorrection(Float_t factor) {fIonTailCorrection = factor;}
  void  SetIonTailCorrectionTimeScale(Float_t timeScale) {fIonTailCorrectionTimeScale = timeScale;}
  void  SetCrosstalkCorrection(Float_t crosstalkCorrection) {fCrosstalkCorrection= crosstalkCorrection; }
  void  SetCrosstalkCorrectionMissingCharge(Float_t crosstalkCorrection) {fCrosstalkCorrectionMissingCharge= crosstalkCorrection; }
  void  SetCrosstalkIonTail(Bool_t crosstalkIonTail) {fCrosstalkIonTail= crosstalkIonTail; }
  //
  Int_t  GetCorrMapTimeDepMethod()      const {return fCorrMapTimeDepMethod;}
  void   SetCorrMapTimeDepMethod(int m)       {fCorrMapTimeDepMethod = m;}
  Int_t  GetUseLumiType()               const {return fUseLumiType;}
  void   SetUseLumiType(int tp)               {fUseLumiType  =tp;}
  //
  Bool_t GetUseMapLumiInfoCOG()         const {return fUseMapLumiInfoCOG;}
  void   SetUseMapLumiInfoCOG(Bool_t v=kTRUE) {fUseMapLumiInfoCOG = v;}
  //
  Int_t GetUseFieldCorrection() const {return fUseFieldCorrection;}
  Int_t GetUseComposedCorrection() const {return fUseComposedCorrection;}
  Int_t GetUseRPHICorrection() const {return fUseRPHICorrection;}
  Int_t GetUseRadialCorrection() const {return fUseRadialCorrection;}
  Int_t GetUseQuadrantAlignment() const {return fUseQuadrantAlignment;}
  Int_t GetUseSectorAlignment() const {return fUseSectorAlignment;}
  Int_t GetUseDriftCorrectionTime() const {return fUseDriftCorrectionTime;}
  Int_t GetUseDriftCorrectionGY() const {return fUseDriftCorrectionGY;}
  Int_t GetUseGainCorrectionTime() const {return fUseGainCorrectionTime;}
  Int_t GetUseExBCorrection() const {return fUseExBCorrection;}
  Bool_t GetUseTOFCorrection() {return fUseTOFCorrection;}
  Int_t GetUseIonTailCorrection() const {return fUseIonTailCorrection;}
  Float_t GetIonTailCorrection() const {return fIonTailCorrection;}
   Float_t GetIonTailCorrectionTimeScale() const {return fIonTailCorrectionTimeScale;}
  Double_t GetCrosstalkCorrection() const {return fCrosstalkCorrection;}
 Double_t GetCrosstalkCorrectionMissingCharge() const {return fCrosstalkCorrectionMissingCharge;}
 Bool_t   GetCrosstalkIonTail() const {return fCrosstalkIonTail;}

  Bool_t GetUseMultiplicityCorrectionDedx() const {return fUseMultiplicityCorrectionDedx;}
  Int_t  GetGainCorrectionHVandPTMode() const  { return   fGainCorrectionHVandPTMode;}
  Double_t  GetSkipTimeBins() const {return fSkipTimeBins;}

  Bool_t GetUseAlignmentTime() const {return fUseAlignmentTime;}
  //
  Bool_t   GetUseTotCharge() const {return fUseTotCharge;}          // switch use total or max charge
  Float_t  GetMinFraction() const {return fMinFraction;}           // truncated mean - lower threshold
  Float_t  GetMaxFraction() const {return fMaxFaction;}            // truncated mean - upper threshold
  Int_t    GetNeighborRowsDedx() const {return fNeighborRowsDedx;}
  void     SetMissingClusterdEdxFraction(Float_t fraction){fMissingClusterdEdxFraction=fraction;}
  Float_t   GetMissingClusterdEdxFraction() const {return fMissingClusterdEdxFraction;}
  //
  void     SetSystematicError(Double_t *systematic){ for (Int_t i=0; i<5;i++) fSystematicErrors[i]=systematic[i];}
  void     SetSystematicErrorCluster(Double_t *systematic){ for (Int_t i=0; i<2;i++) fSystematicErrorCluster[i]=systematic[i];}
  Double_t GetUseDistortionFractionAsErrorY() const {return fDistortionFractionAsErrorYZ[0];}
  Double_t GetUseDistortionFractionAsErrorZ() const {return fDistortionFractionAsErrorYZ[1];}
  Double_t GetUseDistDispFractionAsErrorY() const {return fDistDispFractionAsErrorYZ[0];}
  Double_t GetUseDistDispFractionAsErrorZ() const {return fDistDispFractionAsErrorYZ[1];}
  void     SetUseDistortionFractionAsErrorY(double v) {fDistortionFractionAsErrorYZ[0] = v;}
  void     SetUseDistortionFractionAsErrorZ(double v) {fDistortionFractionAsErrorYZ[1] = v;}
  void     SetUseDistDispFractionAsErrorY(double v) {fDistDispFractionAsErrorYZ[0] = v;}
  void     SetUseDistDispFractionAsErrorZ(double v) {fDistDispFractionAsErrorYZ[1] = v;}

  void     SetBadPadMaxDistXYZD(double x,double y,double z, double d)  {
    fBadPadMaxDistXYZD[0]=x;fBadPadMaxDistXYZD[1]=y;fBadPadMaxDistXYZD[2]=z;fBadPadMaxDistXYZD[3]=d;}
  void     SetBadClusMaxErrYZ(double y,double z)             {fBadClusMaxErrYZ[0]=y;fBadClusMaxErrYZ[1]=z;}

  const Double_t* GetBadPadMaxDistXYZD() const {return fBadPadMaxDistXYZD;}
  const Double_t* GetBadClusMaxErrYZ()   const {return fBadClusMaxErrYZ;}

  const Double_t * GetSystematicError() const { return fSystematicErrors;}
  const Double_t * GetSystematicErrorClusterInner() const { return fSystematicErrorClusterInner;}
  const Double_t * GetSystematicErrorCluster() const { return fSystematicErrorCluster;}

  const TVectorF* GetSystErrClInnerRegZ()       const {return fSystErrClInnerRegZ;}
  const TVectorF* GetSystErrClInnerRegZSigInv() const {return fSystErrClInnerRegZSigInv;}
  void SetSystErrClInnerRegZ(TVectorF* zc)         {fSystErrClInnerRegZ = zc;}
  void SetSystErrClInnerRegZSigInv(TVectorF* zs)   {fSystErrClInnerRegZSigInv = zs;}
  
  void    SetUseSystematicCorrelation(Bool_t useCorrelation)  {fUseSystematicCorrelation=useCorrelation;}
  Bool_t  GetUseSystematicCorrelation() const { return fUseSystematicCorrelation;}
  //
  Bool_t GetUseClusterErrordEdxCorrection() const {return fUseClusterErrordEdxCorrection;}
  void   SettUseClusterErrordEdxCorrection(Bool_t useClusterErrordEdxCorrection){ fUseClusterErrordEdxCorrection=useClusterErrordEdxCorrection;}
  Bool_t GetUseClusterErrordEdxMultCorrection() const {return fUseClusterErrordEdxMultCorrection;}
  void   SettUseClusterErrordEdxMultCorrection(Bool_t useClusterErrordEdxMultCorrection){ fUseClusterErrordEdxMultCorrection=useClusterErrordEdxMultCorrection;}
  const TMatrixF& GetClusterErrorMatrix() const {return fClusterErrorMatrix;}
  void  SetClusterErrorMatrix(TMatrixF* matrix) {fClusterErrorMatrix=*matrix;}
  void  SetClusterErrorMatrixElement(Int_t row, Int_t column, Float_t value) {fClusterErrorMatrix(row,column)=value;}
  void  SetClusterErrorParam();      // set default cluster error Param
  const TMatrixF& GetClusterNSigma2Cut() const {return fClusterNSigma2Cut;}
  void SetClusterNSigma2Cut(TMatrixF cuts){fClusterNSigma2Cut=cuts;}
  void SetClusterNSigma2Cut(Int_t row, Int_t column,Float_t cut){fClusterNSigma2Cut(row,column)=cut;}

  static   AliTPCRecoParam *GetLowFluxParam();        // make reco parameters for low  flux env.
  static   AliTPCRecoParam *GetHighFluxParam();       // make reco parameters for high flux env.
  static   AliTPCRecoParam *GetHLTParam(); // special setting for HLT
  static   AliTPCRecoParam *GetLaserTestParam(Bool_t bPedestal);  // special setting for laser
  static   AliTPCRecoParam *GetCosmicTestParam(Bool_t bPedestal); // special setting for cosmic
  //
  static  const Double_t * GetSystematicErrorClusterCustom() { return (fgSystErrClustCustom) ? fgSystErrClustCustom->GetMatrixArray():0;}
  static  const Double_t * GetPrimaryDCACut()                { return (fgPrimaryDCACut)? fgPrimaryDCACut->GetMatrixArray():0; }
  static  void  SetSystematicErrorClusterCustom( TVectorD *vec ) { fgSystErrClustCustom=vec;}
  static  void SetPrimaryDCACut( TVectorD *dcacut )              { fgPrimaryDCACut=dcacut;}
  //
  //
 protected:

  Int_t    fUseHLTClusters;  ///< allows usage of HLT clusters instead of RAW data
  Int_t    fUseHLTPreSeeding; ///< Usage of HLT pre-seeding
  Bool_t   fBClusterSharing; ///< allows or disable cluster sharing during tracking
  Double_t fCtgRange;        ///< +-fCtgRange is the ctg(Theta) window used for clusterization and tracking (MI)
  Double_t fMaxSnpTracker;   ///< max sin of local angle  - for TPC tracker
  Double_t fMaxSnpTrack;     ///< max sin of local angle  - for track
  Bool_t   fUseOuterDetectors; ///< switch - to use the outer detectors
  Double_t fMaxChi2TPCTRD;     ///< maximal allowed chi2 between the TRD in and TPC out to be accepted for refit
  Double_t fMaxChi2TPCITS;     ///< maximal allowed chi2 between the ITS in and TPC out to be accepted for backpropagation
  //
  // Outlier filtering configuration
  //
  Int_t   fUseOulierClusterFilter;  ///< swith to use outlier cluster filter

  Double_t fCutSharedClusters[2]; ///< cut value - maximal amount  of shared clusters
  Int_t fClusterMaxRange[2];   ///< neighborhood  - to define local maxima for cluster
  //
  //   clusterer parameters
  //
  Bool_t   fDumpSignal;      ///< Dump Signal information flag
  Int_t    fFirstBin;        ///< first time bin used by cluster finder
  Int_t    fLastBin;         ///< last time bin  used by cluster finder
  Bool_t   fBCalcPedestal;   ///< calculate Pedestal
  Bool_t   fBDoUnfold;       ///< do unfolding of clusters
  Float_t  fDumpAmplitudeMin; ///< minimal amplitude of signal to be dumped
  Float_t  fMaxNoise;        ///< maximal noise sigma on pad to be used in cluster finder
  Int_t    fUseOnePadCluster; ///< flag - use one pad cluster -0 not use >0 use
  Bool_t   fUseHLTOnePadCluster; ///< flag - use one HLT pad cluster for tracking
  Float_t  fMinMaxCutAbs;    ///< minimal amplitude at cluster maxima
  Float_t  fMinLeftRightCutAbs;  ///< minimal amplitude left right - PRF
  Float_t  fMinUpDownCutAbs;  ///< minimal amplitude up-down - TRF
  Float_t  fMinMaxCutSigma;    ///< minimal amplitude at cluster maxima
  Float_t  fMinLeftRightCutSigma;  ///< minimal amplitude left right - PRF
  Float_t  fMinUpDownCutSigma;  ///< minimal amplitude up-down - TRF
  //
  //
  Float_t  fMaxC;            ///< maximal curvature for tracking
  Bool_t   fBSpecialSeeding; ///< special seeding with big inclination angles allowed (for Cosmic and laser)
  Bool_t   fBKinkFinder;       ///< do kink finder reconstruction
  Double_t fKinkAngleCutChi2[2];   ///< angular cut for kinks
  Int_t    fLastSeedRowSec;     ///< Most Inner Row to make seeding for secondaries
  Int_t    fSeedGapPrim;   ///< seeding gap for primary tracks
  Int_t    fSeedGapSec;   ///< seeding gap for secondary tracks

  //
  // Correction switches
  //
  Int_t fUseFieldCorrection;     ///< use field correction
  Bool_t fUseComposedCorrection; ///< flag to use composed correction
  Int_t fUseRPHICorrection;      ///< use rphi correction
  Int_t fUseRadialCorrection;    ///< use radial correction
  Int_t fUseQuadrantAlignment;   ///< use quadrant alignment
  Int_t fUseSectorAlignment;     ///< use sector alignment
  Int_t fUseDriftCorrectionTime; ///< use drift correction time
  Int_t fUseDriftCorrectionGY;   ///< use drif correction global y
  Int_t fUseGainCorrectionTime;  ///< use gain correction time
  Int_t fUseExBCorrection;       ///< use ExB correction
  Bool_t fUseMultiplicityCorrectionDedx; ///< use Dedx multiplicity correction
  Bool_t fUseAlignmentTime;              ///< use time dependent alignment correction
  Int_t fUseIonTailCorrection;   ///< use ion tail correction
  Float_t fIonTailCorrection;      ///< ion tail tail correction factor - NEW SINCE 2018- additonal scaling correcting for imperfect knowledge of the integral of ion tail - shoudl be ~ 1
  Float_t fIonTailCorrectionTimeScale;      ///< ion tail tail correction factor time stretching - new Since 2019 - default value=1
  Double_t fCrosstalkCorrection;   ///< crosstalk correction factor (fro each signal substracted by (mean signal in wite patch)xfCrosstalkCorrection) - Effect important only after removing oc capacitors in 2012
  Double_t fCrosstalkCorrectionMissingCharge;   ///< crosstalk correction factor - missing charge factor (from each signal substracted by (mean signal in wite patch)xfCrosstalkCorrection) - Effect important only after removing  capacitors in 2012
  Bool_t   fCrosstalkIonTail;            /// < flag calculate crosstalk for ion tail
  //
  // dEdx switches
  //
  Bool_t   fUseTotCharge;          ///< switch use total or max charge
  Float_t fMinFraction;           ///< truncated mean - lower threshold
  Float_t fMaxFaction;            ///< truncated mean - upper threshold
  Int_t   fNeighborRowsDedx;      ///< number of neighboring rows to identify cluster below thres in dEdx calculation 0 -> switch off
  Float_t fMissingClusterdEdxFraction; /// mising cluster will be replaced by truncated mean of observed clusters 1+ Nclmissing*fMissingClusterdEdxFraction;
  Int_t   fGainCorrectionHVandPTMode; ///< switch for the usage of GainCorrectionHVandPT (see AliTPCcalibDB::GetGainCorrectionHVandPT
  Int_t   fAccountDistortions;        ///< account for distortions in tracking
  Double_t fSkipTimeBins;        ///< number of time bins to be skiiped (corrupted signal druing gating opening)

  Bool_t fUseTOFCorrection;  ///< switch - kTRUE use TOF correction kFALSE - do not use
  //
  Bool_t fUseCorrectionMap;  ///< flag to use parameterized correction map (AliTPCChebCorr)
  Int_t  fCorrMapTimeDepMethod; ///< method used for correction time dependence
  Int_t  fUseLumiType;          ///< luminosity graph to be used for different lumi scalings
  Bool_t fUseMapLumiInfoCOG;    ///< if true, try to use lumi COG record stored in Cheb.param (GetLumiInfo() if >0) 
  Float_t fSystCovAmplitude;    ///< apply syst correction to cov.matrix with this amplitude
  Float_t  fDistFluctCorrelation; ///< assumed correlation between fluctuating points
  Float_t fDistortionFluctMCAmp; ///< mult. amplitude for distortions fluctuation sigma 
  Float_t fMinDistFluctMCRef;    ///< min.fluctuation sigma at reference 850kHz pp@13TeV (==7.5kHz PbPb@5.02TeV), for MC
  Float_t fDistFluctUncorrFracMC; ///< uncorrelated fraction of distortions fluctuations to impose in MC
  //  misscalibration
  //
  TVectorF* fSystErrClInnerRegZ;        //< center of region in Z to apply extra systematic error
  TVectorF* fSystErrClInnerRegZSigInv;  //< inverse sigma forgaussian dumping aroung this center Z to apply extra syst error  
  Double_t fSystematicErrors[5];  ///< systematic errors in the track parameters - to be added to TPC covariance matrix
  Double_t fSystematicErrorClusterInner[2];  ///< systematic error of the cluster - used to downscale the information

  Double_t fSystematicErrorCluster[2];        ///< systematic error of the cluster - used e.g in OpenGG run to provide better cluster to track association efficiency
  Double_t fDistortionFractionAsErrorYZ[2];   ///< use fraction of distortion as additional error
  Double_t fDistDispFractionAsErrorYZ[2];   ///< use fraction of distortion dispersion as additional error
  Double_t fBadPadMaxDistXYZD[4];            ///< pad considered bad if abs distortion / dispersion exceeds this value
  Double_t fBadClusMaxErrYZ[2];              ///< pad considered bad if syst.error on cluster exceeds this value
  Bool_t fUseSystematicCorrelation;         ///< switch to use the correlation for the sys
  //
  Bool_t fUseClusterErrordEdxCorrection;     ///< switch to use the dEdx correction
  Bool_t fUseClusterErrordEdxMultCorrection;     ///< switch to use the dEdx, multiplicity  correction
  TMatrixF fClusterErrorMatrix;                  ///< cluster error matrix
  TMatrixF fClusterNSigma2Cut;                    /// < n sigma cluster/trac cut
  static TVectorD* fgSystErrClustCustom;  //< custom systematic errors for the TPC clusters overriding persistent data member
  static TVectorD* fgPrimaryDCACut;       //< only primaries passing DCAYZ cut are reconstructed

public:
  static Bool_t fgUseTimeCalibration; ///< flag usage the time dependent calibration
                                      // to be switched off for pass 0 reconstruction
                                      // Use static function, other option will be to use
                                      // additional specific storage ?
  /// \cond CLASSIMP
  ClassDef(AliTPCRecoParam, 38)
  /// \endcond
};


#endif
