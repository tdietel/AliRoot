#ifndef ALIAODHEADER_H
#define ALIAODHEADER_H
/* Copyright(c) 1998-2007, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

/// \class AliAODHeader
/// \brief AOD event header class
///
/// \author Markus Oldenburg, CERN

#include <TVector2.h>

#include "AliVHeader.h"
#include "AliVAODHeader.h"
#include "AliAODVertex.h"
#include <TString.h>
#include <TBits.h>
#include "AliCentrality.h"
#include "AliEventplane.h"
#include <TVectorF.h>

class TGeoHMatrix;
class TString;

class AliAODHeader : public AliVAODHeader {

 public :
  AliAODHeader();
 
  AliAODHeader(Int_t nRun, UShort_t nBunchX, UInt_t nOrbit, UInt_t nPeriod, UInt_t tStamp=0, const Char_t *title="");
  AliAODHeader(Int_t nRun, 
	       UShort_t nBunchX,
	       UInt_t nOrbit,
	       UInt_t nPeriod,
	       UInt_t tStamp,
	       Int_t refMult,
	       Int_t refMultPos,
	       Int_t refMultNeg,
	       Int_t refMultComb05,
	       Int_t refMultComb08,
	       Int_t refMultComb10,
	       Double_t magField,
	       Double_t muonMagFieldScale,
	       Double_t cent,
	       Double_t eventplane,
	       Double_t n1Energy,
	       Double_t p1Energy,
	       Double_t n2Energy,
	       Double_t p2Energy,
	       Double_t *emEnergy,
	       ULong64_t triggerMask,ULong64_t triggerMaskNext50,
	       UChar_t   triggerCluster,
	       UInt_t    fEventType,
	       const Float_t *vzeroEqFactors,
	       const Char_t *title="",
	       Int_t nMuons=0,
	       Int_t nDimuons=0,
	       Int_t nGlobalMuons=0,             // AU
	       Int_t nGlobalDimuons=0,          // AU
	       UInt_t daqAttrib=0);

  virtual ~AliAODHeader();
  AliAODHeader(const AliAODHeader& evt); 
  AliAODHeader& operator=(const AliAODHeader& evt);
  Bool_t    InitMagneticField() const;

  Int_t     GetRunNumber()          const { return fRunNumber;}
  Int_t     GetEventNumberESDFile() const { return fEventNumberESDFile;}
  Int_t     GetNumberOfESDTracks()   const { return fNumberESDTracks;}
  Int_t     GetNumberOfTPCClusters() const {return fNumberTPCClusters;}
  Int_t     GetNumberOfTPCTracks()   const {return fNumberTPCTracks;}
  void      SetNumberOfTPCClusters(int n)  {fNumberTPCClusters = n;}
  void      SetNumberOfTPCTracks(int n)    {fNumberTPCTracks = n;}
  UShort_t  GetBunchCrossNumber()   const { return fBunchCrossNumber; }
  UInt_t    GetOrbitNumber()        const { return fOrbitNumber; }
  UInt_t    GetPeriodNumber()       const { return fPeriodNumber; }
  void      SetTimeStamp(UInt_t timeStamp){fTimeStamp = timeStamp;}
  UInt_t    GetTimeStamp()          const { return fTimeStamp;}
  ULong64_t GetTriggerMask()        const { return fTriggerMask; }
  ULong64_t GetTriggerMaskNext50()  const { return fTriggerMaskNext50; }
  UChar_t   GetTriggerCluster()     const { return fTriggerCluster; }
  TString   GetFiredTriggerClasses()const { return fFiredTriggers;}
  UInt_t    GetEventType()          const { return fEventType; }
  Double_t  GetMagneticField()      const { return fMagneticField; }
  Double_t  GetMuonMagFieldScale()  const { return fMuonMagFieldScale; }
  
  Double_t  GetCentrality()         const { return fCentrality; }
  Double_t  GetEventplane()         const { return fEventplane; }
  Double_t  GetEventplaneMag()      const { return fEventplaneMag; }
  Double_t  GetEventplaneQx()       const { return fEventplaneQx; }
  Double_t  GetEventplaneQy()       const { return fEventplaneQy; }
  Double_t  GetZDCN1Energy()        const { return fZDCN1Energy; }
  Double_t  GetZDCP1Energy()        const { return fZDCP1Energy; }
  Double_t  GetZDCN2Energy()        const { return fZDCN2Energy; }
  Double_t  GetZDCP2Energy()        const { return fZDCP2Energy; }
  Double_t  GetZDCEMEnergy(Int_t i) const { return fZDCEMEnergy[i]; }
  Int_t     GetRefMultiplicity()    const { return fRefMult; }
  Int_t     GetRefMultiplicityPos() const { return fRefMultPos; }
  Int_t     GetRefMultiplicityNeg() const { return fRefMultNeg; }
  Int_t     GetNumberOfMuons()      const { return fNMuons; }
  Int_t     GetNumberOfDimuons()    const { return fNDimuons; }
  Int_t     GetNumberOfGlobalMuons()   const { return fNGlobalMuons; }      // AU
  Int_t     GetNumberOfGlobalDimuons() const { return fNGlobalDimuons; }    // AU
  Int_t     GetRefMultiplicityComb05() const { return fRefMultComb05; }
  Int_t     GetRefMultiplicityComb08() const { return fRefMultComb08; }
  Int_t     GetRefMultiplicityComb10() const { return fRefMultComb10; }

  void SetDetectorStatusMask(UInt_t detStatus) { fDetectorStatus=detStatus;          }
  void SetDetectorStatus(UInt_t detMask)       { fDetectorStatus|=detMask;           }
  void ResetDetectorStatus(UInt_t detMask)     { fDetectorStatus&=~detMask;          }
  UInt_t GetDetectorStatus() const             { return fDetectorStatus;             }
  Bool_t IsDetectorOn(ULong_t detMask) const    { return (fDetectorStatus&detMask)>0; }


  UInt_t    GetDAQAttributes()         const {return fDAQAttributes;}
  void      SetDAQAttributes(UInt_t v)       {fDAQAttributes = v;}

  Double_t  GetQTheta(UInt_t i) const;
  UInt_t    GetNQTheta() const { return (UInt_t)fNQTheta; }

  Double_t GetDiamondX() const {return fDiamondXY[0];}
  Double_t GetDiamondY() const {return fDiamondXY[1];}
  Double_t GetDiamondZ() const {return fDiamondZ;}
  Double_t GetSigma2DiamondX() const {return fDiamondCovXY[0];}
  Double_t GetSigma2DiamondY() const {return fDiamondCovXY[2];}
  Double_t GetSigma2DiamondZ() const {return fDiamondSig2Z;}
  void GetDiamondCovXY(Float_t cov[3]) const {
    for(Int_t i=0;i<3;i++) { cov[i]=fDiamondCovXY[i]; }
    return;
  }
  UInt_t   GetL0TriggerInputs() const {return fL0TriggerInputs;}  
  UInt_t   GetL1TriggerInputs() const {return fL1TriggerInputs;} 
  UShort_t GetL2TriggerInputs() const {return fL2TriggerInputs;} 
  AliCentrality* GetCentralityP()  const { return fCentralityP; }
  AliEventplane* GetEventplaneP()  const { return fEventplaneP; }

  
  void SetRunNumber(Int_t nRun)                { fRunNumber = nRun; }
  void SetEventNumberESDFile(Int_t n)          { fEventNumberESDFile=n; }
  void SetNumberOfESDTracks(Int_t n)           { fNumberESDTracks=n; }
  void SetBunchCrossNumber(UShort_t nBx)       { fBunchCrossNumber = nBx; }
  void SetOrbitNumber(UInt_t nOr)              { fOrbitNumber = nOr; }
  void SetPeriodNumber(UInt_t nPer)            { fPeriodNumber = nPer; }
  void SetTriggerMask(ULong64_t trigMsk)       { fTriggerMask = trigMsk; }
  void SetTriggerMaskNext50(ULong64_t trigMsk) { fTriggerMaskNext50 = trigMsk; }
  void SetFiredTriggerClasses(TString trig)    { fFiredTriggers = trig;}
  void SetTriggerCluster(UChar_t trigClus)     { fTriggerCluster = trigClus; }
  void SetEventType(UInt_t evttype)            { fEventType = evttype; }
  void SetMagneticField(Double_t magFld)       { fMagneticField = magFld; }
  void SetMuonMagFieldScale(Double_t magFldScl){ fMuonMagFieldScale = magFldScl; }
  void SetCentrality(const AliCentrality* cent);
  void SetEventplane(AliEventplane* eventplane);
  void SetZDCN1Energy(Double_t n1Energy)       { fZDCN1Energy = n1Energy; }
  void SetZDCP1Energy(Double_t p1Energy)       { fZDCP1Energy = p1Energy; }
  void SetZDCN2Energy(Double_t n2Energy)       { fZDCN2Energy = n2Energy; }
  void SetZDCP2Energy(Double_t p2Energy)       { fZDCP2Energy = p2Energy; }
  void SetZDCEMEnergy(Double_t emEnergy1, Double_t emEnergy2)      
  	{ fZDCEMEnergy[0] = emEnergy1; fZDCEMEnergy[1] = emEnergy2;}
  void SetRefMultiplicity(Int_t refMult)       { fRefMult = refMult; }
  void SetRefMultiplicityPos(Int_t refMultPos) { fRefMultPos = refMultPos; }
  void SetRefMultiplicityNeg(Int_t refMultNeg) { fRefMultNeg = refMultNeg; }
  void SetNumberOfMuons(Int_t nMuons) { fNMuons = nMuons; }
  void SetNumberOfDimuons(Int_t nDimuons) { fNDimuons = nDimuons; }
  void SetNumberOfGlobalMuons(Int_t nGlobalMuons) { fNGlobalMuons = nGlobalMuons; }            // AU
  void SetNumberOfGlobalDimuons(Int_t nGlobalDimuons) { fNGlobalDimuons = nGlobalDimuons; }    // AU
  void SetRefMultiplicityComb05(Int_t refMult)   { fRefMultComb05 = refMult; }
  void SetRefMultiplicityComb08(Int_t refMult)   { fRefMultComb08 = refMult; }  
  void SetRefMultiplicityComb10(Int_t refMult)   { fRefMultComb10 = refMult; }
  
  void SetTPCTrackBeforeClean(int n) {fNTPCTrackBeforeClean = n;}
  Int_t GetNTPCTrackBeforeClean() const {return fNTPCTrackBeforeClean;}
  
  void SetQTheta(Double_t *QTheta, UInt_t size = 5);  
  void RemoveQTheta();

  void ResetEventplanePointer();
  
  void SetDiamond(Float_t xy[2],Float_t cov[3]) { 
    for(Int_t i=0;i<3;i++) {fDiamondCovXY[i] = cov[i];}
    for(Int_t i=0;i<2;i++) {fDiamondXY[i]    = xy[i] ;}
  }
  void SetDiamondZ(Float_t z, Float_t sig2z){
    fDiamondZ=z; fDiamondSig2Z=sig2z;
  }
  void SetL0TriggerInputs(UInt_t n)   {fL0TriggerInputs=n;}
  void SetL1TriggerInputs(UInt_t n)   {fL1TriggerInputs=n;}
  void SetL2TriggerInputs(UShort_t n) {fL2TriggerInputs=n;}
  void SetESDFileName(TString name)   {fESDFileName = name;}
  void Print(Option_t* option = "") const;

  void    SetPHOSMatrix(TGeoHMatrix*matrix, Int_t i) {
      if ((i >= 0) && (i < kNPHOSMatrix)) fPHOSMatrix[i] = matrix;
  }
  const TGeoHMatrix* GetPHOSMatrix(Int_t i) const {
      return ((i >= 0) && (i < kNPHOSMatrix)) ? fPHOSMatrix[i] : NULL;
  }
  
  void    SetEMCALMatrix(TGeoHMatrix*matrix, Int_t i) {
      if ((i >= 0) && (i < kNEMCALMatrix)) fEMCALMatrix[i] = matrix;
  }
  const TGeoHMatrix* GetEMCALMatrix(Int_t i) const {
      return ((i >= 0) && (i < kNEMCALMatrix)) ? fEMCALMatrix[i] : NULL;
  }
  
  UInt_t GetOfflineTrigger() { return fOfflineTrigger; }
  void   SetOfflineTrigger(UInt_t trigger) { fOfflineTrigger = trigger; }
  UInt_t GetNumberOfITSClusters(Int_t ilay) const {return fITSClusters[ilay];}
  void   SetITSClusters(Int_t ilay, UInt_t nclus);
  Int_t  GetTPConlyRefMultiplicity() const {return fTPConlyRefMult;}
  void   SetTPConlyRefMultiplicity(Int_t mult) {fTPConlyRefMult = mult;} 
  
  TString GetESDFileName() const  {return fESDFileName;}
  void Clear(Option_t* = "");
  enum {kNPHOSMatrix = 5};
  enum {kNEMCALMatrix = 22}; 
  enum {kT0SpreadSize = 4};

  void           SetVZEROEqFactors(const Float_t* factors) {
    if (factors)
      for (Int_t i = 0; i < 64; ++i) fVZEROEqFactors[i] = factors[i];}
  const Float_t* GetVZEROEqFactors() const {return fVZEROEqFactors;}
  Float_t        GetVZEROEqFactors(Int_t i) const {return fVZEROEqFactors[i];}
  Float_t    GetT0spread(Int_t i) const {
    return ((i >= 0)  && (i<kT0SpreadSize)) ? fT0spread[i] : 0;}
  void       SetT0spread(Int_t i, Float_t t) {
    if ((i>=0)&&(i<kT0SpreadSize)) fT0spread[i]=t;}

  const TVectorF* GetTPCPileUpInfo() const {return fTPCPileUpInfo;}
  const TVectorF* GetITSPileUpInfo() const {return fITSPileUpInfo;}
  void SetTPCPileUpInfo(const TVectorF* src);
  void SetITSPileUpInfo(const TVectorF* src);
  
  Int_t  FindIRIntInteractionsBXMap(Int_t difference) const;
  void   SetIRInt2InteractionMap(TBits bits) { fIRInt2InteractionsMap = bits; }
  void   SetIRInt1InteractionMap(TBits bits) { fIRInt1InteractionsMap = bits; }
  TBits  GetIRInt2InteractionMap() const { return fIRInt2InteractionsMap; }
  TBits  GetIRInt1InteractionMap() const { return fIRInt1InteractionsMap; }
  Int_t  GetIRInt2ClosestInteractionMap() const;
  Int_t  GetIRInt1ClosestInteractionMap(Int_t gap = 3) const;
  Int_t  GetIRInt2LastInteractionMap() const;
  
 private :

  Double32_t  fMagneticField;       ///< Solenoid Magnetic Field in kG
  Double32_t  fMuonMagFieldScale;   ///< magnetic field scale of muon arm magnet
  Double32_t  fCentrality;          ///< Centrality
  Double32_t  fEventplane;          ///< Event plane angle
  Double32_t  fEventplaneMag;       ///< Length of Q vector from TPC event plance
  Double32_t  fEventplaneQx;        ///< Q vector component x from TPC event plance
  Double32_t  fEventplaneQy;        ///< Q vector component y from TPC event plance
  Double32_t  fZDCN1Energy;         ///< reconstructed energy in the neutron1 ZDC
  Double32_t  fZDCP1Energy;         ///< reconstructed energy in the proton1 ZDC
  Double32_t  fZDCN2Energy;         ///< reconstructed energy in the neutron2 ZDC
  Double32_t  fZDCP2Energy;         ///< reconstructed energy in the proton2 ZDC
  Double32_t  fZDCEMEnergy[2];      ///< reconstructed energy in the electromagnetic ZDCs
  Int_t       fNQTheta;             ///< number of QTheta elements
  ///< values to store Lee-Yang-Zeros
  Double32_t *fQTheta;              //[fNQTheta]
  ULong64_t   fTriggerMask;         ///< Trigger Type (mask)
  ULong64_t   fTriggerMaskNext50;   ///< Trigger Type (mask) for upper 50 slots
  TString     fFiredTriggers;       ///< String with fired triggers
  Int_t       fRunNumber;           ///< Run Number
  Int_t       fRefMult;             ///< reference multiplicity
  Int_t       fRefMultPos;          ///< reference multiplicity of positive particles
  Int_t       fRefMultNeg;          ///< reference multiplicity of negative particles
  Int_t       fNMuons;              ///< number of muons in the forward spectrometer
  Int_t       fNDimuons;            ///< number of dimuons in the forward spectrometer
  Int_t       fNGlobalMuons;        ///< number of muons in the forward spectrometer + MFT       // AU
  Int_t       fNGlobalDimuons;      ///< number of dimuons in the forward spectrometer + MFT     // AU
  UInt_t      fDetectorStatus;      ///< set detector event status bit for good event selection
  UInt_t      fDAQAttributes;       ///< DAQ attibutes
  UInt_t      fEventType;           ///< Type of Event
  UInt_t      fOrbitNumber;         ///< Orbit Number
  UInt_t      fPeriodNumber;        ///< Period Number
  UShort_t    fBunchCrossNumber;    ///< BunchCrossingNumber
  UInt_t      fTimeStamp;           ///< Time stamp
  Short_t     fRefMultComb05;       ///< combined reference multiplicity (tracklets + ITSTPC) in |eta|<0.5
  Short_t     fRefMultComb08;       ///< combined reference multiplicity (tracklets + ITSTPC) in |eta|<0.8
  Short_t     fRefMultComb10;       ///< combined reference multiplicity (tracklets + ITSTPC) in |eta|<1.0
  UChar_t     fTriggerCluster;      ///< Trigger cluster (mask)
  Double32_t      fDiamondXY[2];    ///< Interaction diamond (x,y) in RUN
  Double32_t      fDiamondCovXY[3]; ///< Interaction diamond covariance (x,y) in RUN
  Double32_t      fDiamondZ;        ///< Interaction diamond (z) in RUN
  Double32_t      fDiamondSig2Z;    ///< Interaction diamond sigma^2 (z) in RUN
  TGeoHMatrix*    fPHOSMatrix[kNPHOSMatrix];   ///< PHOS module position and orientation matrices
  TGeoHMatrix*    fEMCALMatrix[kNEMCALMatrix]; ///< EMCAL supermodule position and orientation matrices
  UInt_t      fOfflineTrigger;      ///< fired offline triggers for this event
  TString     fESDFileName;         ///< ESD file name to which this event belongs
  Int_t       fEventNumberESDFile;  ///< Event number in ESD file
  Int_t       fNumberESDTracks;     ///< Number of tracks in origingal ESD event
  Int_t       fNumberTPCTracks;     ///< Number of TPCrefit tracks in origingal ESD event
  Int_t       fNTPCTrackBeforeClean; ///< unumber of TPC tracks in the ESD before Clean (if any)
  Int_t       fNumberTPCClusters;   ///< total number of TPC clusters
  UInt_t      fL0TriggerInputs;     ///< L0 Trigger Inputs (mask)
  UInt_t      fL1TriggerInputs;     ///< L1 Trigger Inputs (mask)
  UShort_t    fL2TriggerInputs;     ///< L2 Trigger Inputs (mask)
  UInt_t      fITSClusters[6];      ///< Number of ITS cluster per layer
  Int_t       fTPConlyRefMult;      ///< Reference multiplicty for standard TPC only tracks
  AliCentrality* fCentralityP;      ///< Pointer to full centrality information
  AliEventplane* fEventplaneP;	    ///< Pointer to full event plane information
  Float_t     fVZEROEqFactors[64];  ///< V0 channel equalization factors for event-plane reconstruction
  Float_t     fT0spread[kT0SpreadSize]; ///< spread of time distributions: (TOA+T0C/2), T0A, T0C, (T0A-T0C)/2
  TVectorF*   fTPCPileUpInfo;           ///< custom info about TPC pileup used by TPC PID
  TVectorF*   fITSPileUpInfo;           ///< custom info about TPC pileup used by TPC PID
  TBits   fIRInt2InteractionsMap;  ///< map of the Int2 events (normally 0TVX) near the event, that's Int2Id-EventId in a -90 to 90 window
  TBits   fIRInt1InteractionsMap;  ///< map of the Int1 events (normally V0A&V0C) near the event, that's Int1Id-EventId in a -90 to 90 window
  ClassDef(AliAODHeader, 33);
};
inline
void AliAODHeader::SetCentrality(const AliCentrality* cent)      { 
    if(cent){
	if(fCentralityP)*fCentralityP = *cent;
	else fCentralityP = new AliCentrality(*cent);
	fCentrality = cent->GetCentralityPercentile("V0M");
    }
    else{
	fCentrality = -999;
    }
}
inline
void AliAODHeader::SetEventplane(AliEventplane* eventplane)      { 
    if(eventplane){
	if(fEventplaneP)*fEventplaneP = *eventplane;
	else fEventplaneP = new AliEventplane(*eventplane);
	fEventplane = eventplane->GetEventplane("Q");
        const TVector2* qvect=eventplane->GetQVector();
        fEventplaneMag = -999;
	fEventplaneQx = -999;
	fEventplaneQy = -999;
        if (qvect) {
	  fEventplaneMag=qvect->Mod();
	  fEventplaneQx=qvect->X();
	  fEventplaneQy=qvect->Y();
	}
    }
    else{
	fEventplane = -999;
        fEventplaneMag = -999;
	fEventplaneQx = -999;
	fEventplaneQy = -999;
    }
}
inline
void AliAODHeader::ResetEventplanePointer()      {
  delete fEventplaneP;
  fEventplaneP = 0x0;
}

inline
void AliAODHeader::SetITSClusters(Int_t ilay, UInt_t nclus)
{
    if (ilay >= 0 && ilay < 6) fITSClusters[ilay] = nclus;
}


#endif
