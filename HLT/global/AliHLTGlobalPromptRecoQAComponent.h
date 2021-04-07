#ifndef ALIHLTGLOBALPROMPTRECOQACOMPONENT_H
#define ALIHLTGLOBALPROMPTRECOQACOMPONENT_H
//* This file is property of and copyright by the ALICE HLT Project        *
//* ALICE Experiment at CERN, All rights reserved.                         *
//* See cxx source for full Copyright notice                               *

#include "AliHLTProcessor.h"
#include "AliHLTComponentBenchmark.h"
#include <vector>
#include <map>
#include "TH1.h"
#include "AliOptionParser.h"

// forward declarations
class AliESDEvent;
class AliESDfriend;
class TTree;
struct AliHLTTracksData;
class AliTPCclusterMI;
class TH1F;
class TH2F;
class TH1D;
class TH2D;
class AliHLTTPCHWCFData;

/**
 * @class AliHLTGlobalPromptRecoQAComponent
 * simple global data QA
 *
 */

struct axisStruct {
  int bins;
  double low;
  double high;
  double* value;
  std::map<std::string,bool> histograms;
  string description;
  axisStruct() : bins(0), low(0.), high(1.), value(NULL), histograms(), description() {}
  axisStruct(const axisStruct& s) : bins(s.bins), low(s.low), high(s.high), value(s.value), histograms(s.histograms), description(s.description) {}
  axisStruct& operator=(const axisStruct& s) {bins=s.bins; low=s.low; high=s.high; value=s.value; histograms=s.histograms; description=s.description; return *this;}
  void set( int b, double l, double h, double* v, string desc="" )
  { bins=b; low=l; high=h; value=v; description=desc; }
};

struct histStruct {
  TH1* hist;
  axisStruct x; //x data
  axisStruct y; //y data
  string trigger; //trigger name
  string config; //full config string
  bool triggerIsRegex;
  int Fill();
  histStruct() : hist(NULL), x(), y(), trigger(), config(), triggerIsRegex(false) {}
  histStruct( const histStruct& s) : hist(s.hist), x(s.x), y(s.y), trigger(s.trigger), config(s.config), triggerIsRegex(s.triggerIsRegex) {}
  histStruct& operator=(const histStruct& s) {hist=s.hist; x=s.x; y=s.y; trigger=s.trigger; config=s.config; triggerIsRegex = s.triggerIsRegex; return *this;}
};

class AliHLTGlobalPromptRecoQAComponent : public AliHLTProcessor, public AliOptionParser
{
 public:
  /** standard constructor */
  AliHLTGlobalPromptRecoQAComponent();
  /** destructor */
  virtual ~AliHLTGlobalPromptRecoQAComponent();

  // interface methods of base class
  const char* GetComponentID() {return "PromptRecoQA";};
  void GetInputDataTypes(AliHLTComponentDataTypeList& list);
  AliHLTComponentDataType GetOutputDataType();
  void GetOutputDataSize(unsigned long& constBase, double& inputMultiplier);
  AliHLTComponent* Spawn() {return new AliHLTGlobalPromptRecoQAComponent;}

 protected:
  // interface methods of base class
  int DoInit(int argc, const char** argv);
  int DoDeinit();
  int DoEvent( const AliHLTComponentEventData& evtData,
		       const AliHLTComponentBlockData* blocks,
		       AliHLTComponentTriggerData& trigData,
		       AliHLTUInt8_t* outputPtr,
		       AliHLTUInt32_t& size,
		       AliHLTComponentBlockDataList& outputBlocks );

  using AliHLTProcessor::DoEvent;

 private:
  /** copy constructor prohibited */
  AliHLTGlobalPromptRecoQAComponent(const AliHLTGlobalPromptRecoQAComponent&);
  /** assignment operator prohibited */
  AliHLTGlobalPromptRecoQAComponent& operator=(const AliHLTGlobalPromptRecoQAComponent&);

  //destroy all histograms
  int Reset(bool resetDownstream=false);

  /**
   * (Re)Configure from the CDB
   * Loads the following objects:
   */
  int Reconfigure(const char* cdbEntry, const char* chainId);

  /**
   * Configure the component.
   * overloaded from AliOptionParser
   * Parse a string for the configuration arguments and set the component
   * properties.
   */
  int ProcessOption(TString option, TString value);


  void NewAxis(string config);
  void NewAxis(string name, int bins, float low, float high, string desc="");
  void NewHistogram(string trigName, string histName, string histTitle, string xname, string yname, string config="" );
  void NewHistogram(std::string histConfig);
  void CreateFixedHistograms();
  void DeleteFixedHistograms();
  int FillHistograms();

protected:

  int fVerbosity; //!transient
  AliHLTComponentBenchmark fBenchmark; // benchmark

  AliHLTTPCHWCFData* fpHWCFData;

  Int_t fSkipEvents;
  Int_t fPrintStats; //print status messages: 0: never, 1: when pushing histograms (respect pushback-period), 2: always
  Int_t fPrintDownscale;
  Int_t fEventsSinceSkip;
  Bool_t fPushEmptyHistograms;
  Bool_t fResetAfterPush;

  Int_t fScaleDownClusterAttachHistos; //Scale down filling of histograms: 0 = disable histogramn, 1 = fill every histogram, n = fill every n-th histogram

  std::map<string,histStruct> fHistograms;
  std::map<string,axisStruct> fAxes;

  double fnClustersSPD;
  double frawSizeSPD;
  double fnClustersSDD;
  double frawSizeSDD;
  double fnClustersSSD;
  double frawSizeSSD;
  double fnClustersITS;
  double frawSizeITS;
  double frawSizeVZERO;
  double frawSizeEMCAL;
  double frawSizeZDC;
  double frawSizeTRD;
  double frawSizeFMD;
  double frawSizeTZERO;
  double frawSizeACORDE;
  double frawSizeCTP;
  double frawSizeAD;
  double frawSizeTOF;
  double frawSizePHOS;
  double frawSizeCPV;
  double frawSizeHMPID;
  double frawSizePMD;
  double frawSizeMUTK;
  double frawSizeMUTG;

  double fnClustersTPC;
  double frawSizeTPC;
  double fhwcfSizeTPC;
  double fclusterSizeTPCtransformed;
  double fclusterSizeTPC;
  double fcompressedSizeTPC;
  double fTPCSplitRatioPad;
  double fTPCSplitRatioTime;

  double fnITSSAPtracks;
  double fnTPCtracklets;
  double fnTPCtracks;
  double fnITSTracks;
  double fnITSOutTracks;

  double fvZEROMultiplicity;
  double fvZEROTriggerChargeA;
  double fvZEROTriggerChargeC;
  double fvZEROTriggerChargeAC;
  
  double ftZEROAmplitude;

  double fzdcZNC;
  double fzdcZNA;
  double fzdcZNAC;

  double fzdcRecoSize;
  double femcalRecoSize;
  double femcalTRU;
  double femcalSTU;
  
  double fnTRDTracklets;
  double fnTRDTracks;

  double fcompressionRatio;
  double fcompressionRatioFull;

  double fnESDSize;
  double fnESDFriendSize;
  double fnFlatESDSize;
  double fnFlatESDFriendSize;

  double fnHLTInSize;
  double fnHLTOutSize;
  double fhltRatio;
  
  long long int fTotalClusters;
  long long int fTotalCompressedBytes;

  double fITSSPDvertexZ;

  //Fixed histograms with track / cluster properties.
  //These are not created dynamically because that would require quite some CPU resources.
  TH1D* fHistClusterChargeTot;
  TH1D* fHistClusterChargeMax;
  TH1D* fHistTPCTrackPt;
  TH2F* fHistTPCAattachedClustersRowPhi;
  TH2F* fHistTPCAallClustersRowPhi;
  TH2F* fHistTPCCattachedClustersRowPhi;
  TH2F* fHistTPCCallClustersRowPhi;
  TH2F* fHistDeDxOffline;
  TH2F* fHistDeDxNew[10];
  TH1D* fHistTRDHCId;
  TH1D* fHistTPCClusterFlags;

  ClassDef(AliHLTGlobalPromptRecoQAComponent, 0)
};
#endif
