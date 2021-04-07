#ifndef ALIANALYSISTASKPIDRESPONSE_H
#define ALIANALYSISTASKPIDRESPONSE_H

/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: AliAnalysisTaskPIDResponse.h 43642 2010-09-17 15:50:04Z wiechula $ */
/// \class AliAnalysisTaskPIDResponse
/// \brief \author Jens Wiechula
///
/// \date 24/02/2011

#include <TVectorDfwd.h>
#include <TString.h>

#ifndef ALIANALYSISTASKSE_H
#include "AliAnalysisTaskSE.h"
#endif

class AliPIDResponse;
class AliVEvent;

class AliAnalysisTaskPIDResponse : public AliAnalysisTaskSE {
  
  
public:
  AliAnalysisTaskPIDResponse();
  AliAnalysisTaskPIDResponse(const char *name);
  virtual ~AliAnalysisTaskPIDResponse();

  void SetIsMC(Bool_t isMC=kTRUE)   { fIsMC=isMC; }
  void SetCachePID(Bool_t cachePID) { fCachePID=cachePID; }
  Bool_t GetCachePID() const { return fCachePID; }
  
  virtual void UserCreateOutputObjects();
  
  virtual void UserExec(Option_t */*option*/);

  void SetOADBPath(const char* path) {fOADBPath=path;}
  const char* GetOADBPath() const { return fOADBPath.Data(); }
  void SetTuneOnData(Bool_t flag, Int_t recopass, TString recoPassName){fIsTunedOnData=flag; fRecoPassTuned=recopass; fRecoPassNameTuned=recoPassName;}
  Bool_t GetTunedOnData() const { return fIsTunedOnData; };
  void SetTuneOnDataMask(Int_t mask){fTunedOnDataMask=mask;};
  void ResetTuneOnDataTOF(Bool_t flag=kTRUE) {fResetTuneOnDataTOF = flag;}

  void SetUseTPCEtaCorrection(Bool_t useTPCEtaCorrection) { fUseTPCEtaCorrection = useTPCEtaCorrection; };
  Bool_t UseTPCEtaCorrection() const { return fUseTPCEtaCorrection; };
  
  void SetUseTPCMultiplicityCorrection(Bool_t useMultiplicityCorrection = kTRUE) { fUseTPCMultiplicityCorrection = useMultiplicityCorrection; };
  Bool_t UseTPCMultiplicityCorrection() const { return fUseTPCMultiplicityCorrection; };

  void SetUseTPCPileupCorrection(Bool_t usePileupCorrection = kTRUE) { fUseTPCPileupCorrection = usePileupCorrection; };
  Bool_t UseTPCPileupCorrection() const { return fUseTPCPileupCorrection; };


  void SetUseTRDEtaCorrection(Bool_t useTRDEtaCorrection) { fUseTRDEtaCorrection = useTRDEtaCorrection; };
  Bool_t UseTRDEtaCorrection() const { return fUseTRDEtaCorrection; };
  void SetUseTRDClusterCorrection(Bool_t useTRDClusterCorrection) { fUseTRDClusterCorrection = useTRDClusterCorrection; };
  Bool_t UseTRDClusterCorrection() const { return fUseTRDClusterCorrection; };
  void SetUseTRDCentralityCorrection(Bool_t useTRDCentralityCorrection) { fUseTRDCentralityCorrection = useTRDCentralityCorrection; };
  Bool_t UseTRDCentralityCorrection() const { return fUseTRDCentralityCorrection; };


  void SetSpecialDetectorResponse(const char* det) { fSpecialDetResponse=det; }
  void SetUserDataRecoPass(Int_t pass){fUserDataRecoPass=pass;};

  void SetRandomSeed(const ULong_t randomSeed) { fRandomSeed = randomSeed; }
  ULong_t GetRandomSeed() const { return fRandomSeed; }

private:
  Bool_t fIsMC;                        ///< If we run on MC data
  Bool_t fCachePID;                    ///< Cache PID values in transient object
  TString fOADBPath;                   ///< OADB path to use
  TString fSpecialDetResponse;         ///< Special detector response files for debugging
  TString fRecoPassName;               //!<! Full name of the reco pass
  TString fRecoPassNameTuned;          ///< Full name of the reco pass used for tuning to MC
  
  AliPIDResponse *fPIDResponse;        //!<! PID response Handler
  Int_t   fRun;                        //!<! current run number
  Int_t   fOldRun;                     //!<! current run number
  Int_t   fRecoPass;                   //!<! reconstruction pass

  Bool_t  fIsTunedOnData;              ///< flag to tune MC on data (dE/dx)
  Int_t   fTunedOnDataMask;            ///< mask to activate tuning on data on specific detectors
  Int_t   fRecoPassTuned;              ///< Reco pass tuned on data for MC
  Bool_t  fResetTuneOnDataTOF;         ///< Flag to force recomputation of tune-on-data for TOF   

  Bool_t fUseTPCEtaCorrection;          ///< Use TPC eta correction
  Bool_t fUseTPCMultiplicityCorrection; ///< Use TPC multiplicity correction
  Bool_t fUseTPCPileupCorrection;       ///< Use TPC multiplicity correction

  Bool_t fUseTRDEtaCorrection;          ///< Use TRD eta correction
  Bool_t fUseTRDClusterCorrection;      ///< Use TRD cluster correction
  Bool_t fUseTRDCentralityCorrection;   ///< Use TRD centrality correction

  Int_t  fUserDataRecoPass;            ///< forced DATA reco pass
  ULong_t fRandomSeed;                 ///< random seed required for tune on data, 0: random, 1: off, otherwise use provided number
  
  //
  void SetRecoInfo();
    
  AliAnalysisTaskPIDResponse(const AliAnalysisTaskPIDResponse &other);
  AliAnalysisTaskPIDResponse& operator=(const AliAnalysisTaskPIDResponse &other);
  
  ClassDef(AliAnalysisTaskPIDResponse,12)  // Task to properly set the PID response functions of all detectors
};
#endif
