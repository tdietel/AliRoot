#ifndef ALIEMCALTRIGGERELECTRONICS_H
#define ALIEMCALTRIGGERELECTRONICS_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

//________________________________________________
/// \class AliEMCALTriggerElectronics
/// \ingroup EMCALsim
/// \brief EMCal trigger electronics manager L0/L1
///
/// EMCal trigger electronics manager L0/L1
/// can handle both simulated digits and raw data
///
/// \author: R. GUERNANE LPSC Grenoble CNRS/IN2P3
//________________________________________________

#ifndef ROOT_TObject
#  include "TObject.h"
#endif
#include "TClonesArray.h"

class AliRawReader;
class AliEMCALTriggerDCSConfig;
class AliEMCALTriggerData;
class AliEMCALTriggerSTU;
class AliESDVZERO;
class AliEMCALTriggerTRU;
class TTree;
class AliEMCALGeometry;

class AliEMCALTriggerElectronics : public TObject 
{
public:
                 AliEMCALTriggerElectronics(const AliEMCALTriggerDCSConfig* dcsConfig = 0x0); // ctor
  virtual       ~AliEMCALTriggerElectronics();                                   // dtor
  
  virtual void   Digits2Trigger(TClonesArray* digits, const Int_t V0M[], AliEMCALTriggerData* data);	
  virtual void   Reset();  

  Float_t GetADCscaleMC() { return fADCscaleMC; }
  void SetADCscaleMC(Float_t input) { fADCscaleMC = input; }
  
  virtual AliEMCALTriggerTRU* GetTRU( Int_t iTRU ) {return (AliEMCALTriggerTRU*)fTRU->At(iTRU);}
  virtual AliEMCALTriggerSTU* GetSTU( Bool_t isDCAL = false ) {return isDCAL ? fSTUDCAL : fSTU;}
  
private:
  
  AliEMCALTriggerElectronics(const AliEMCALTriggerElectronics& other);            // Not implemented
  AliEMCALTriggerElectronics& operator=(const AliEMCALTriggerElectronics& other); // Not implemented
  
  Int_t                 fNTRU;     //< Total number of TRUs
  TClonesArray*         fTRU;      ///< 32 TRU
  AliEMCALTriggerSTU*   fSTU;      ///< 1 STU for EMCAL
  AliEMCALGeometry     *fGeometry; ///< EMCal geometry
 
  Int_t                fMedianMode; // 0 for no median subtraction, 1 for median sub.
  Float_t              fADCscaleMC; //< Scaling up MC raw digits so samples match total energy
  Float_t              fL1ADCNoise; //< Sigma of noise added to the L1 Timesums in MC
  TClonesArray*        fTRUDCAL;  //< 14 TRU
  AliEMCALTriggerSTU*  fSTUDCAL;  //< 1 STU for DCAL
 
  /// \cond CLASSIMP
  ClassDef(AliEMCALTriggerElectronics,1) ;
  /// \endcond
  
};

#endif //ALIEMCALTRIGGERELECTRONICS_H
