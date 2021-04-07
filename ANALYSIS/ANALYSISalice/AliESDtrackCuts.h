/// \class AliESDtrackCuts
/// \brief Class for handling of ESD track cuts.
///
/// The class manages a number of track quality cuts, a
/// track-to-vertex cut and a number of kinematic cuts. Two methods
/// can be used to figure out if an ESD track survives the cuts:
/// AcceptTrack which takes a single AliESDtrack as argument and
/// returns kTRUE/kFALSE or GetAcceptedTracks which takes an AliESDEvent
/// object and returns an TObjArray (of AliESDtracks) with the tracks
/// in the ESD that survived the cuts.
///
/// TODO:
/// - add functionality to save and load cuts
/// - add histograms for kinematic cut variables?
/// - upper and lower cuts for all (non-boolean) cuts
/// - update print method
/// - put comments to each variable

#ifndef ALIESDTRACKCUTS_H
#define ALIESDTRACKCUTS_H

#include <TString.h>

#include "AliAnalysisCuts.h"

class AliESDEvent;
class AliESDtrack;
class AliVTrack;
class AliVEvent;
class AliLog;
class TTree;
class TH1;
class TH1F;
class TH2F;
class TF1;
class TCollection;
class TFormula;

class AliESDtrackCuts : public AliAnalysisCuts
{
public:
  enum ITSClusterRequirement { kOff = 0, kNone, kAny, kFirst, kOnlyFirst, kSecond, kOnlySecond, kBoth };
  enum Detector { kSPD = 0, kSDD, kSSD };
  enum ITSULayers { kITSU012 = 0, kITSU34, kITSU56 };
  enum MultEstTrackCuts { kMultEstTrackCutGlobal = 0, kMultEstTrackCutITSSA, kMultEstTrackCutDCAwSPD, kMultEstTrackCutDCAwoSPD, kNMultEstTrackCuts /* this must always be the last */};
  enum MultEstTrackType { kTrackletsITSTPC = 0, kTrackletsITSSA, kTracklets };
  enum VertexType { kVertexTracks = 0x1, kVertexSPD = 0x2, kVertexTPC = 0x4 };
  
  AliESDtrackCuts(const Char_t* name = "AliESDtrackCuts", const Char_t* title = "");
  virtual ~AliESDtrackCuts();

  virtual Bool_t IsSelected(TObject* obj);
  virtual Bool_t IsSelected(TList* /*list*/) {return kTRUE;}

  Bool_t AcceptTrack(const AliESDtrack* esdTrack);
  Bool_t AcceptVTrack(const AliVTrack* vTrack);
  TObjArray* GetAcceptedTracks(const AliESDEvent* esd, Bool_t bTPC = kFALSE);
  Int_t CountAcceptedTracks(const AliESDEvent* const esd);
  
  static Int_t GetReferenceMultiplicity(const AliESDEvent* esd, Bool_t tpcOnly);
  static Int_t GetReferenceMultiplicity(const AliESDEvent* esd, MultEstTrackType trackType = kTrackletsITSTPC, Float_t etaRange = 0.5, Float_t etaCent=0.);
  static AliESDtrackCuts* GetMultEstTrackCuts(MultEstTrackCuts cut);

  static AliESDtrack* GetTPCOnlyTrack(const AliESDEvent* esd, Int_t iTrack);
  static AliESDtrack* GetTPCOnlyTrackFromVEvent(const AliVEvent* vEvent, Int_t iTrack);
  
  // Standard cut definitions
  static AliESDtrackCuts* GetStandardTPCOnlyTrackCuts();
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2009(Bool_t selPrimaries=kTRUE);
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2010(Bool_t selPrimaries=kTRUE, Int_t clusterCut=0);
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2011(Bool_t selPrimaries=kTRUE, Int_t clusterCut=1);
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2011TighterChi2(Bool_t selPrimaries=kTRUE, Int_t clusterCut=1);
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2015PbPb(Bool_t selPrimaries=kTRUE, Int_t clusterCut=1, Bool_t cutAcceptanceEdges = kTRUE, Bool_t removeDistortedRegions = kFALSE);
  static AliESDtrackCuts* GetStandardITSTPCTrackCuts2015PbPbTighterChi2(Bool_t selPrimaries=kTRUE, Int_t clusterCut=1, Bool_t cutAcceptanceEdges = kTRUE, Bool_t removeDistortedRegions = kFALSE);
  static AliESDtrackCuts* GetStandardITSSATrackCuts2009(Bool_t selPrimaries=kTRUE, Bool_t useForPid=kTRUE);
  static AliESDtrackCuts* GetStandardITSSATrackCuts2010(Bool_t selPrimaries=kTRUE, Bool_t useForPid=kTRUE);
  static AliESDtrackCuts* GetStandardITSSATrackCutsPbPb2010(Bool_t selPrimaries=kTRUE, Bool_t useForPid=kTRUE);
  static AliESDtrackCuts* GetStandardITSPureSATrackCuts2009(Bool_t selPrimaries=kTRUE, Bool_t useForPid=kTRUE);
  static AliESDtrackCuts* GetStandardITSPureSATrackCuts2010(Bool_t selPrimaries=kTRUE, Bool_t useForPid=kTRUE);
  // Standard cuts for daughter tracks
  static AliESDtrackCuts* GetStandardV0DaughterCuts();
  
  // Standard cuts equivalent to the ones used in Run3
  static AliESDtrackCuts* GetStandardRun3NoTrackCuts();
  static AliESDtrackCuts* GetStandardRun3GlobalTrackCuts();
  static AliESDtrackCuts* GetStandardRun3GlobalSDDTrackCuts();

  // static function to determine if the track crosses a distorted region in the TPC
  static Bool_t IsTrackInDistortedTpcRegion(const AliESDtrack * esdTrack);

  virtual Long64_t Merge(TCollection* list);
  virtual void Copy(TObject &c) const;
  AliESDtrackCuts(const AliESDtrackCuts& pd);  // Copy Constructor
  AliESDtrackCuts &operator=(const AliESDtrackCuts &c);

  //######################################################
  // track quality cut setters  
  void SetMinNClustersTPC(Int_t min=-1)          {fCutMinNClusterTPC=min;}
  void SetMinNClustersTPCPtDep(TFormula *f1=0x0, Float_t ptmax=0.);
  void SetMinNClustersITS(Int_t min=-1)          {fCutMinNClusterITS=min;}
  void SetMinNCrossedRowsTPC(Float_t min=-1) { fCutMinNCrossedRowsTPC=min;}
  void SetMinRatioCrossedRowsOverFindableClustersTPC(Float_t min = -1) { fCutMinRatioCrossedRowsOverFindableClustersTPC=min;}
  void SetMinLengthActiveVolumeTPC(Float_t min = 120., Float_t width=2) {fCutMinLengthActiveVolumeTPC=min; fDeadZoneWidth=width;}
  void SetCutGeoNcrNcl(Float_t deadZoneWidth=2,Float_t cutGeoNcrNclLength=130, Float_t cutGeoNcrNclGeom1Pt=1.5, Float_t cutGeoNcrNclFractionNcr=0.9,  Float_t cutGeoNcrNclFractionNcl=0.70);
  void SetCutOutDistortedRegionsTPC(Bool_t cutOutDistortedRegionTPC = kTRUE) {fCutOutDistortedRegionTPC=cutOutDistortedRegionTPC;}

  void SetClusterRequirementITS(Detector det, ITSClusterRequirement req = kOff) { fCutClusterRequirementITS[det] = req; }
  void SetClusterRequirementITS(ITSULayers det, ITSClusterRequirement req = kOff) { fCutClusterRequirementITS[det] = req; }
  void SetMaxChi2PerClusterTPC(Float_t max=1e10) {fCutMaxChi2PerClusterTPC=max;}
  void SetMaxChi2PerClusterITS(Float_t max=1e10) {fCutMaxChi2PerClusterITS=max;}
  void SetMaxChi2TPCConstrainedGlobal(Float_t max=1e10) {fCutMaxChi2TPCConstrainedVsGlobal = max; }
  void SetMaxChi2TPCConstrainedGlobalVertexType(Int_t vertexType = kVertexTracks | kVertexSPD) { fCutMaxChi2TPCConstrainedVsGlobalVertexType = vertexType; }
  void SetMaxNOfMissingITSPoints(Int_t max=6)    {fCutMaxMissingITSPoints=max;}
  void SetRequireTPCRefit(Bool_t b=kFALSE)       {fCutRequireTPCRefit=b;}
  void SetRequireTPCStandAlone(Bool_t b=kFALSE)  {fCutRequireTPCStandAlone=b;}
  void SetRequireITSRefit(Bool_t b=kFALSE)       {fCutRequireITSRefit=b;}
  void SetRequireITSPid(Bool_t b=kFALSE)         {fCutRequireITSPid=b;}
  void SetRequireITSStandAlone(Bool_t b=kFALSE)    {fCutRequireITSStandAlone = b;} 
  void SetRequireITSPureStandAlone(Bool_t b=kFALSE){fCutRequireITSpureSA = b;}


  void SetAcceptKinkDaughters(Bool_t b=kTRUE)    {fCutAcceptKinkDaughters=b;}
  void SetAcceptSharedTPCClusters(Bool_t b=kTRUE){fCutAcceptSharedTPCClusters=b;}
  void SetMaxFractionSharedTPCClusters(Float_t max=1e10) {fCutMaxFractionSharedTPCClusters=max;}
  void SetMaxCovDiagonalElements(Float_t c1=1e10, Float_t c2=1e10, Float_t c3=1e10, Float_t c4=1e10, Float_t c5=1e10) 
    {fCutMaxC11=c1; fCutMaxC22=c2; fCutMaxC33=c3; fCutMaxC44=c4; fCutMaxC55=c5;}
  void SetMaxRel1PtUncertainty(Float_t max=1e10)      {fCutMaxRel1PtUncertainty=max;}
  void SetMaxRel1PtUncertaintyPtDep(const char *dist="");


  // track to vertex cut setters
  void SetMaxNsigmaToVertex(Float_t sigma=1e10)       {fCutNsigmaToVertex = sigma; SetRequireSigmaToVertex(kTRUE);}
  void SetRequireSigmaToVertex(Bool_t b=kTRUE)        {fCutSigmaToVertexRequired = b;}
  void SetMaxDCAToVertexXY(Float_t dist=1e10)         {fCutMaxDCAToVertexXY = dist;}
  void SetMaxDCAToVertexZ(Float_t dist=1e10)          {fCutMaxDCAToVertexZ = dist;}
  void SetMinDCAToVertexXY(Float_t dist=0.)           {fCutMinDCAToVertexXY = dist;}
  void SetMinDCAToVertexZ(Float_t dist=0.)            {fCutMinDCAToVertexZ = dist;}
  void SetMaxDCAToVertexXYPtDep(const char *dist="");
  void SetMaxDCAToVertexZPtDep(const char *dist=""); 
  void SetMinDCAToVertexXYPtDep(const char *dist="");
  void SetMinDCAToVertexZPtDep(const char *dist=""); 
  void SetDCAToVertex2D(Bool_t b=kFALSE)              {fCutDCAToVertex2D = b;}


  // getters  
  Int_t   GetMinNClusterTPC()        const   { return fCutMinNClusterTPC;}
  Float_t GetMinNCrossedRowsTPC()    const   { return fCutMinNCrossedRowsTPC;}
  Float_t GetMinRatioCrossedRowsOverFindableClustersTPC()    const   { return fCutMinRatioCrossedRowsOverFindableClustersTPC;}
  Float_t GetMinLengthActiveVolumeTPC() const { return fCutMinLengthActiveVolumeTPC;}
  Int_t   GetMinNClustersITS()       const   { return fCutMinNClusterITS;}
  TFormula *GetMinNClustersTPCPtDep() const  { return f1CutMinNClustersTPCPtDep;}
  ITSClusterRequirement GetClusterRequirementITS(Detector det) const { return fCutClusterRequirementITS[det]; }
  ITSClusterRequirement GetClusterRequirementITS(ITSULayers det) const { return fCutClusterRequirementITS[det]; }
  Float_t GetMaxChi2PerClusterTPC()  const   { return fCutMaxChi2PerClusterTPC;}
  Float_t GetMaxChi2PerClusterITS()  const   { return fCutMaxChi2PerClusterITS;}
  Float_t GetMaxChi2TPCConstrainedGlobal() const { return fCutMaxChi2TPCConstrainedVsGlobal; }
  Int_t GetMaxChi2TPCConstrainedGlobalVertexType() const { return fCutMaxChi2TPCConstrainedVsGlobalVertexType; }
  Int_t   GetMaxNOfMissingITSPoints() const   { return  fCutMaxMissingITSPoints;}
  Bool_t  GetRequireTPCRefit()       const   { return fCutRequireTPCRefit;}
  Bool_t  GetRequireTPCStandAlone()  const   { return fCutRequireTPCStandAlone;}
  Bool_t  GetRequireITSRefit()       const   { return fCutRequireITSRefit;}
  Bool_t  GetRequireITSStandAlone()  const   { return fCutRequireITSStandAlone; }
  Bool_t  GetRequireITSpureSA()      const   { return fCutRequireITSpureSA; }
  Bool_t  GetAcceptKinkDaughters()   const   { return fCutAcceptKinkDaughters;}
  Bool_t  GetAcceptSharedTPCClusters()        const   {return fCutAcceptSharedTPCClusters;}
  Float_t GetMaxFractionSharedTPCClusters()   const   {return fCutMaxFractionSharedTPCClusters;}
  void    GetMaxCovDiagonalElements(Float_t& c1, Float_t& c2, Float_t& c3, Float_t& c4, Float_t& c5) const
      {c1 = fCutMaxC11; c2 = fCutMaxC22; c3 = fCutMaxC33; c4 = fCutMaxC44; c5 = fCutMaxC55;}
  Float_t GetMaxRel1PtUncertainty()  const   { return fCutMaxRel1PtUncertainty;}
  const char* GetMaxRel1PtUncertaintyPtDep() const   { return fCutMaxRel1PtUncertaintyPtDep;}
  Float_t GetMaxNsigmaToVertex()     const   { return fCutNsigmaToVertex;}
  Float_t GetMaxDCAToVertexXY()      const   { return fCutMaxDCAToVertexXY;}
  Float_t GetMaxDCAToVertexZ()       const   { return fCutMaxDCAToVertexZ;}
  Float_t GetMinDCAToVertexXY()      const   { return fCutMinDCAToVertexXY;}
  Float_t GetMinDCAToVertexZ()       const   { return fCutMinDCAToVertexZ;}
  const char* GetMaxDCAToVertexXYPtDep() const   { return fCutMaxDCAToVertexXYPtDep;}
  const char* GetMaxDCAToVertexZPtDep()  const   { return fCutMaxDCAToVertexZPtDep;}
  const char* GetMinDCAToVertexXYPtDep() const   { return fCutMinDCAToVertexXYPtDep;}
  const char* GetMinDCAToVertexZPtDep()  const   { return fCutMinDCAToVertexZPtDep;}
  Bool_t  GetDCAToVertex2D()         const   { return fCutDCAToVertex2D;}
  Bool_t  GetRequireSigmaToVertex( ) const   { return fCutSigmaToVertexRequired;}

  void GetPRange(Float_t& r1, Float_t& r2)  const {r1=fPMin;   r2=fPMax;}
  void GetPtRange(Float_t& r1, Float_t& r2) const {r1=fPtMin;  r2=fPtMax;}
  void GetPxRange(Float_t& r1, Float_t& r2) const {r1=fPxMin;  r2=fPxMax;}
  void GetPyRange(Float_t& r1, Float_t& r2) const {r1=fPyMin;  r2=fPyMax;}
  void GetPzRange(Float_t& r1, Float_t& r2) const {r1=fPzMin;  r2=fPzMax;}
  void GetEtaRange(Float_t& r1, Float_t& r2) const {r1=fEtaMin; r2=fEtaMax;}
  void GetRapRange(Float_t& r1, Float_t& r2) const {r1=fRapMin; r2=fRapMax;}

  // track kinmatic cut setters
  void SetPRange(Float_t r1=0, Float_t r2=1e10)       {fPMin=r1;   fPMax=r2;}
  void SetPtRange(Float_t r1=0, Float_t r2=1e10)      {fPtMin=r1;  fPtMax=r2;}
  void SetPxRange(Float_t r1=-1e10, Float_t r2=1e10)  {fPxMin=r1;  fPxMax=r2;}
  void SetPyRange(Float_t r1=-1e10, Float_t r2=1e10)  {fPyMin=r1;  fPyMax=r2;}
  void SetPzRange(Float_t r1=-1e10, Float_t r2=1e10)  {fPzMin=r1;  fPzMax=r2;}
  void SetEtaRange(Float_t r1=-1e10, Float_t r2=1e10) {fEtaMin=r1; fEtaMax=r2;}
  void SetRapRange(Float_t r1=-1e10, Float_t r2=1e10) {fRapMin=r1; fRapMax=r2;}

  //######################################################
  void SetHistogramsOn(Bool_t b=kFALSE) {fHistogramsOn = b;}
  void DefineHistograms(Int_t color=1);
  virtual Bool_t LoadHistograms(const Char_t* dir = 0);
  void SaveHistograms(const Char_t* dir = 0);
  void DrawHistograms();

  static Float_t GetSigmaToVertex(const AliESDtrack* const esdTrack);
  static Float_t GetSigmaToVertexVTrack(const AliVTrack* const vTrack);

  static void EnableNeededBranches(TTree* tree);

  // void SaveQualityCuts(Char_t* file)
  // void LoadQualityCuts(Char_t* file)

  TH1F* GetDZNormalized(Int_t i) const { return fhDZNormalized[i]; }
  TH1F* GetNClustersTPC(Int_t i) const { return fhNClustersTPC[i]; }
  TH1F* GetPtHist(Int_t i) const { return fhPt[i]; }
  
  // TOF cuts
  void SetFlagCutTOFdistance(Bool_t flagTOFcut) { fFlagCutTOFdistance = flagTOFcut;}
  Bool_t GetFlagCutTOFdistance() const { return fFlagCutTOFdistance;}
  void SetCutTOFdistance(Float_t cut) { fCutTOFdistance = cut;}
  Float_t GetCutTOFdistance() const { return fCutTOFdistance;}
  void SetRequireTOFout(Bool_t b = kFALSE) {fCutRequireTOFout = b;} 
  void SetRequireStandardTOFmatchCuts();

protected:
  void Init(); // sets everything to 0
  Bool_t CheckITSClusterRequirement(ITSClusterRequirement req, Bool_t clusterL1, Bool_t clusterL2);
  Bool_t CheckPtDepDCA(TString dist,Bool_t print=kFALSE) const;
  void SetPtDepDCACuts(Double_t pt);
  
  Bool_t CheckPtDepUncertainty(TString dist,Bool_t print=kFALSE) const;
  void SetPtDepUncertaintyCuts(Double_t pt);
  
  enum { kNCuts = 45 }; 

  //######################################################
  // esd track quality cuts
  static const Char_t* fgkCutNames[kNCuts]; //!<! names of cuts (for internal use)
  static AliESDtrackCuts* fgMultEstTrackCuts[kNMultEstTrackCuts]; //!<! track cuts used for the multiplicity estimate

  Int_t   fCutMinNClusterTPC;         ///< min number of tpc clusters
  Int_t   fCutMinNClusterITS;         ///< min number of its clusters
  Float_t   fCutMinNCrossedRowsTPC;     ///< min number of tpc crossed rows
  Float_t fCutMinRatioCrossedRowsOverFindableClustersTPC; ///< min ratio crossed rows / findable clusters
  TFormula *f1CutMinNClustersTPCPtDep; ///< pt dependent tpc clusters cut
  Float_t fCutMaxPtDepNClustersTPC;     ///< maximum pt for pt dependend TPC cluster cut. For pt=>ptmax NClusterMin = f1CutMinNClustersTPCPtDep->Eval(fCutMaxPtDepNClustersTPC).
  Float_t fCutMinLengthActiveVolumeTPC; ///< mininum length (in cm) over which the track is sampled in the active volume of the TPC (outside boundaries)
  Float_t fDeadZoneWidth;             ///< width of the TPC dead zone (missing pads + PRF +ExB)
  // TPC geometrical cut combined with cut on crossed rows and number of clusters    
  Float_t fCutGeoNcrNclLength;        ///< cut on the geometical length  condition Ngeom(cm)>cutGeoNcrNclLength default=130
  Float_t fCutGeoNcrNclGeom1Pt;       ///< 1/pt dependence slope  cutGeoNcrNclLength:=fCutGeoNcrNclLength-abs(1/pt)^fCutGeoNcrNclGeom1Pt
  Float_t fCutGeoNcrNclFractionNcr;   ///< relative fraction cut Ncr  condition Ncr>cutGeoNcrNclFractionNcr*fCutGeoNcrNclLength
  Float_t fCutGeoNcrNclFractionNcl;   ///< ralative fraction cut Ncr  condition Ncl>cutGeoNcrNclFractionNcl
  // TPC distorted regions
  Bool_t  fCutOutDistortedRegionTPC;  ///< flag if distorted regions in the TPC should be cut out


  ITSClusterRequirement fCutClusterRequirementITS[3];  ///< detailed ITS cluster requirements for (SPD, SDD, SSD)

  Float_t fCutMaxChi2PerClusterTPC;   ///< max tpc fit chi2 per tpc cluster
  Float_t fCutMaxChi2PerClusterITS;   ///< max its fit chi2 per its cluster
  Float_t fCutMaxChi2TPCConstrainedVsGlobal; ///< max chi2 TPC track constrained with vtx vs. global track
  Int_t fCutMaxChi2TPCConstrainedVsGlobalVertexType; ///< vertex type for max chi2 TPC track constrained with vtx vs. global track (can be configured to accept several vertex types)
  Int_t   fCutMaxMissingITSPoints;    ///< max n. of missing ITS points

  Float_t fCutMaxC11;                 ///< max cov. matrix diag. elements (res. y^2)
  Float_t fCutMaxC22;                 ///< max cov. matrix diag. elements (res. z^2)
  Float_t fCutMaxC33;                 ///< max cov. matrix diag. elements (res. sin(phi)^2)
  Float_t fCutMaxC44;                 ///< max cov. matrix diag. elements (res. tan(theta_dip)^2)
  Float_t fCutMaxC55;                 ///< max cov. matrix diag. elements (res. 1/pt^2)
  
  //cut on relative pt resolution
  Float_t fCutMaxRel1PtUncertainty;           ///< max relative uncertainty of 1/pt
  TString fCutMaxRel1PtUncertaintyPtDep;      ///< pt-dep max relative uncertainty of 1/pt
  TFormula *f1CutMaxRel1PtUncertaintyPtDep;   ///< only internal use (pt-dep max relative uncertainty of 1/pt)

  Bool_t  fCutAcceptKinkDaughters;    ///< accepting kink daughters?
  Bool_t  fCutAcceptSharedTPCClusters;///< accepting shared clusters in TPC?
  Float_t fCutMaxFractionSharedTPCClusters; ///< Maximum fraction of shared clusters in TPC
  Bool_t  fCutRequireTPCRefit;        ///< require TPC refit
  Bool_t  fCutRequireTPCStandAlone;   ///< require TPC standalone tracks
  Bool_t  fCutRequireITSRefit;        ///< require ITS refit
  Bool_t  fCutRequireITSPid;          ///< require ITS pid
  Bool_t  fCutRequireITSStandAlone;   ///< require ITS standalone tracks (remove pure SA)
  Bool_t  fCutRequireITSpureSA;       ///< require ITS pure standalone tracks (found using all ITS clusters)


  // track to vertex cut
  Float_t fCutNsigmaToVertex;         ///< max number of estimated sigma from track-to-vertex
  Bool_t  fCutSigmaToVertexRequired;  ///< cut track if sigma from track-to-vertex could not be calculated
  Float_t fCutMaxDCAToVertexXY;       ///< track-to-vertex cut in max absolute distance in xy-plane
  Float_t fCutMaxDCAToVertexZ;        ///< track-to-vertex cut in max absolute distance in z-plane
  Float_t fCutMinDCAToVertexXY;       ///< track-to-vertex cut on min absolute distance in xy-plane
  Float_t fCutMinDCAToVertexZ;        ///< track-to-vertex cut on min absolute distance in z-plane
  // 
  TString fCutMaxDCAToVertexXYPtDep;  ///< pt-dep track-to-vertex cut in max absolute distance in xy-plane
  TString fCutMaxDCAToVertexZPtDep;   ///< pt-dep track-to-vertex cut in max absolute distance in z-plane
  TString fCutMinDCAToVertexXYPtDep;  ///< pt-dep track-to-vertex cut on min absolute distance in xy-plane
  TString fCutMinDCAToVertexZPtDep;   ///< pt-dep track-to-vertex cut on min absolute distance in z-plane

  // only internal use, set via strings above
  TFormula *f1CutMaxDCAToVertexXYPtDep;  ///< pt-dep track-to-vertex cut in max absolute distance in xy-plane
  TFormula *f1CutMaxDCAToVertexZPtDep;   ///< pt-dep track-to-vertex cut in max absolute distance in z-plane
  TFormula *f1CutMinDCAToVertexXYPtDep;  ///< pt-dep track-to-vertex cut on min absolute distance in xy-plane
  TFormula *f1CutMinDCAToVertexZPtDep;   ///< pt-dep track-to-vertex cut on min absolute distance in z-plane

  Bool_t  fCutDCAToVertex2D;          ///< if true a 2D DCA cut is made. Tracks are accepted if sqrt((DCAXY / fCutMaxDCAToVertexXY)^2 + (DCAZ / fCutMaxDCAToVertexZ)^2) < 1 AND sqrt((DCAXY / fCutMinDCAToVertexXY)^2 + (DCAZ / fCutMinDCAToVertexZ)^2) > 1

  // esd kinematics cuts
  Float_t fPMin,   fPMax;             ///< definition of the range of the P
  Float_t fPtMin,  fPtMax;            ///< definition of the range of the Pt
  Float_t fPxMin,  fPxMax;            ///< definition of the range of the Px
  Float_t fPyMin,  fPyMax;            ///< definition of the range of the Py
  Float_t fPzMin,  fPzMax;            ///< definition of the range of the Pz
  Float_t fEtaMin, fEtaMax;           ///< definition of the range of the eta
  Float_t fRapMin, fRapMax;           ///< definition of the range of the y

  Bool_t  fCutRequireTOFout;        ///< require TOF out
  Bool_t  fFlagCutTOFdistance;       ///< cut on TOFdistance? --> yes by default!
  Float_t fCutTOFdistance;           ///< value of the cut on TOFdistance
  static Char_t fgBeamTypeFlag;      ///< -1 --> no check done on the beam type yet
                                     // 0 --> beam type != "A-A"
                                     // 1  --> beam type == "A-A"
   
  //######################################################
  // diagnostics histograms
  Bool_t fHistogramsOn;               ///< histograms on/off

  /// 
  TH1F* fhNClustersITS[2];            //->
  /// 
  TH1F* fhNClustersTPC[2];            //->
  /// 
  TH1F* fhNSharedClustersTPC[2];      //->
  /// 
  TH1F* fhNCrossedRowsTPC[2];         //->
  TH1F* fhRatioCrossedRowsOverFindableClustersTPC[2]; //->

  /// 
  TH1F* fhChi2PerClusterITS[2];       //->
  /// 
  TH1F* fhChi2PerClusterTPC[2];       //->
  /// 
  TH1F* fhChi2TPCConstrainedVsGlobal[2];       //->
  /// number of points in SDD+SSD (ITS PID selection)
  TH1F* fhNClustersForITSPID[2];      //->
  /// number of missing ITS points
  TH1F* fhNMissingITSPoints[2];       //->

  /// 
  TH1F* fhC11[2];                     //->
  /// 
  TH1F* fhC22[2];                     //->
  /// 
  TH1F* fhC33[2];                     //->
  /// 
  TH1F* fhC44[2];                     //->
  /// 
  TH1F* fhC55[2];                     //->

  /// rel. uncertainty of 1/pt
  TH1F* fhRel1PtUncertainty[2];       //->

  /// 
  TH1F* fhDXY[2];                     //->
  /// 
  TH1F* fhDZ[2];                      //->
  /// absolute distance sqrt(dxy**2 + dz**2) to vertex; if 2D cut is set, normalized to given values
  TH1F* fhDXYDZ[2];                   //->
  /// 
  TH2F* fhDXYvsDZ[2];                 //->

  /// 
  TH1F* fhDXYNormalized[2];           //->
  /// 
  TH1F* fhDZNormalized[2];            //->
  /// 
  TH2F* fhDXYvsDZNormalized[2];       //->
  /// 
  TH1F* fhNSigmaToVertex[2];          //->

  /// pt of esd tracks
  TH1F* fhPt[2];                      //->
  /// eta of esd tracks
  TH1F* fhEta[2];                     //->

  /// theoretical distance to vertex normalized (2d gauss)
  TF1*  ffDTheoretical;               //->

  /// statistics of what cuts the tracks did not survive
  TH1F*  fhCutStatistics;             //->
  /// 2d statistics plot
  TH2F*  fhCutCorrelation;            //->

  /// TOF signal distance dx vs dz
  TH2F* fhTOFdistance[2];            //->

  ClassDef(AliESDtrackCuts, 24)
};


#endif
