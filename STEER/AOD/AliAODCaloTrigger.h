#ifndef ALIAODCALOTRIGGER_H
#define ALIAODCALOTRIGGER_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

#include "AliVCaloTrigger.h"

class TArrayI;

/**
 * @class AliAODCaloTrigger
 * @brief Container with calorimeter trigger information in the AOD event
 *
 * @author  R. Guernane, LPSC Grenoble CNRS/IN2P3
 */
class AliAODCaloTrigger : public AliVCaloTrigger 
{
public:
  /**
   * @brief Dummy constructor
   */
  AliAODCaloTrigger();

  /**
   * @brief Named constructor
   * @param name Name of the new object
   * @param title Title of the new object
   */
  AliAODCaloTrigger(const char* name, const char* title);

  /**
   * @brief Copy constructor
   * @param ctrig Reference for copy
   * 
   * Copy is performed as deep copy so the new
   * object manages its own entries. Content handled
   * by object previously is destroyed.
   */
  AliAODCaloTrigger(const AliAODCaloTrigger& ctrig);

  /**
   * @brief Destructor
   */
  virtual ~AliAODCaloTrigger();
    
  /**
   * @brief Assignemtn operator
   * @param ctrig Reference for assignment
   * @return Calo trigger object after assignment 
   * 
   * Assignment is performed as deep copy so the new
   * object manages its own entries. Content handled
   * by object previously is destroyed.
   */
  AliAODCaloTrigger& operator=(const AliAODCaloTrigger& ctrig);

  /**
   * @brief Check whether calo trigger object carries entries
   * @return True if object does not carry data (is empty), false otherwise
   */
  Bool_t  IsEmpty() {return (fNEntries == 0);}

  /**
   * @brief Reset internal iterator
   * 
   * Next iteration (via function Next()) will start 
   * from first entry in object
   */
  virtual void Reset() {fCurrent = -1;}

  /**
   * @brief Allocate memory for new entries
   * @param size Number of entries to be handled by the calo trigger object
   */
  void    Allocate(Int_t size);

  /**
   * @brief Delete entries handled by the object and free memory
   */
  void    DeAllocate(        ); 

  /**
   * @brief Add new module entry to the Calo Trigger object
   * @param col Colunm of the module
   * @param row Row of the module
   * @param amp Peak amplitude
   * @param time Time at peak
   * @param trgtimes Trigger times
   * @param ntrgtimes Number of trigger times
   * @param trgtimesum Time sum (time integral over raw ADCs)
   * @param trgbits Trigger bit 
   * @return True if adding the object was successful, false otherwise
   * 
   * The object will be added at the current last entry of the internal memory. No 
   * check for overflow is performed. Users have to check that the object will not
   * exceed the internal memory
   */
  Bool_t  Add(Int_t col, Int_t row, Float_t amp, Float_t time, Int_t trgtimes[], Int_t ntrgtimes, Int_t trgtimesum, Int_t trgbits);

  /**
   * @brief Add new module entry to the Calo Trigger object
   * @param col Colunm of the module
   * @param row Row of the module
   * @param amp Peak amplitude
   * @param time Time at peak
   * @param trgtimes Trigger times
   * @param ntrgtimes Number of trigger times
   * @param trgtimesum Time sum (time integral over raw ADCs)
   * @param subra PHOS subregion (EMCAL only)
   * @param trgbits Trigger bit 
   * @return True if adding the object was successful, false otherwise
   * 
   * The object will be added at the current last entry of the internal memory. No 
   * check for overflow is performed. Users have to check that the object will not
   * exceed the internal memory
   */
  Bool_t  Add(Int_t col, Int_t row, Float_t amp, Float_t time, Int_t trgtimes[], Int_t ntrgtimes, Int_t trgtimesum, Int_t subra, Int_t trgbits);

  void    SetL1Threshold(Int_t i, Int_t thr) {fL1Threshold[i] = thr;}
  void    SetL1Threshold(Int_t i, Int_t j, Int_t thr) {if (i) fL1DCALThreshold[j] = thr; else fL1Threshold[j] = thr;}

  void    SetL1V0(const Int_t* v) {for (int i = 0; i < 2; i++) fL1V0[i] = v[i];}
  void    SetL1V0(Int_t i, const Int_t* v) {
    for (int j = 0; j < 2; j++) {
      if (i)
        { fL1DCALV0[j] = v[j]; }
      else
        { fL1V0[j] = v[j]; }
    }
  }
  
  void    SetL1FrameMask(Int_t m) {fL1FrameMask = m;}
  void    SetL1FrameMask(Int_t i, Int_t m) {if (i) fL1DCALFrameMask = m; else fL1FrameMask = m;}

  void    SetTriggerBitWord(Int_t w) {fTriggerBitWord = w;}

  /**
   * @brief Set the median value for the background subtraction in the detector 
   * @param detectorID Index of the subdetector (0 - EMCAL, 1 - DCAL)
   * @param medianValue Median value for the background subtraction in the detector
   * 
   * The median value is stored for the detector, calulated by the opposite
   * side detector. I.e. median value 0 has to be subtracted from EMCAL patches
   * scaled with the patch size and median value 1 has to be subtracted from 
   * DCAL patches scaled by the patch size.
   */
  void    SetMedian(Int_t detectorID, Int_t medianValue) {fMedian[detectorID] = medianValue;}

  /**
   * @brief Access to position of the current fastor channel
   * @param[out] col Column of the current fastor in the detector plane
   * @param[out] row Row of the current fastor in the detector plane
   */
  void    GetPosition(     Int_t& col, Int_t& row           ) const;

  /**
   * @brief Access to L0-amplitude of the current fastor channel
   * @param[out] amp L0-amplitude for the given fastor channel
   * 
   * The max. amplitude is the maximum value of the time series digitised
   * by the TRU at L0. The value is mostly used from monitoring while for
   * the trigger patches both at L0 and at L1 the time integrals over 4
   * bins around the peak time (L1 timesums) are used.
   * 
   * Attention: TRUs using the ALTRO raw stream with 10 bit words, while
   * STUs use a dedicated raw stream with 12 bit words. As L0 amplitudes
   * are calculated from the TRU payload the raw signals were right-shifted 
   * by 2 bits. Consequently the amplitude needs to be multiplied by 4 in 
   * order to compensate teh bit shift.
   */
  void    GetAmplitude(    Float_t& amp                     ) const;

  /**
   * @brief Get time of the peak amplitude
   * @param time Time of the peak amplitude
   */
  void    GetTime(         Float_t& time                    ) const;
    
  /**
   * @brief Get the trigger bits for a given fastor position
   *
   * Trigger bits define the starting position of online patches.
   * They are defined in AliEMCALTriggerTypes.h. Note that for
   * reconstructed patches an offset (MC offset) has to be taken
   * into account
   *
   * @param[out] bits Trigger bits connected to a given position
   */
  void    GetTriggerBits(  Int_t& bits                      ) const;

  /**
   * @brief Get the number of L0 times for the current patch
   *
   * Level0 times are handled per L0 patch. Indexing is different
   * with respect to the fastor indexing.
   *
   * @param[out] ntimes Number of level0
   */
  void    GetNL0Times(     Int_t& ntimes                    ) const;

  /**
   * @brief Get level0 times for the current L0 patch
   * @param times L0 times for the current L0 patch
   */
  void    GetL0Times(      Int_t  times[]                   ) const;

  /**
   * @brief Get the number of entries in the trigger data
   * @return Number of entries
   */
  Int_t   GetEntries(                                       ) const {return fNEntries;}

  /**
   * @brief Get the L1 time sums (L1 ADC values) for the current fastor
   * @param[out] timesum L1 timesums for the current fastor
   */
  void    GetL1TimeSum(    Int_t& timesum                   ) const;

  /**
   * @brief Get the L1 time sums (L1 ADC values) for the current fastor
   * @return L1 timesums for the current fastor
   */
  Int_t   GetL1TimeSum(                                     ) const;

  /**
   * @brief Get PHOS subregion value stored in current position
   * @param[out] subreg PHOS subregion value for the current postion
   * 
   * PHOS subregions, used for the combined DCAL-PHOS jet trigger,
   * are stored in as extra entries for the current position in column-
   * row space. In the trigger patch calculation they are to be used
   * as normal L1 timesums.
   */
  void    GetL1SubRegion(  Int_t& subreg                    ) const;

  /**
   * @brief Get PHOS subregion value stored in current position
   * @return PHOS subregion value for the current postion 
   * 
   * PHOS subregions, used for the combined DCAL-PHOS jet trigger,
   * are stored in as extra entries for the current position in column-
   * row space. In the trigger patch calculation they are to be used
   * as normal L1 timesums.
   */
  Int_t   GetL1SubRegion(                                   ) const;
  
  Int_t   GetL1Threshold(  Int_t  i                         ) const {return fL1Threshold[i];}
  Int_t   GetL1Threshold(  Int_t  i, Int_t j                ) const {return ((i)?fL1DCALThreshold[j]:fL1Threshold[j]);}

  Int_t   GetL1V0(         Int_t  i                         ) const {return fL1V0[i];}
  Int_t   GetL1V0(         Int_t  i, Int_t  j               ) const {return ((i)?fL1DCALV0[j]:fL1V0[j]);}
  
  Int_t   GetL1FrameMask(                                   ) const {return fL1FrameMask;}
  Int_t   GetL1FrameMask(  Int_t  i                         ) const {return ((i)?fL1DCALFrameMask:fL1FrameMask);}

  /**
   * @brief Get Median for background subtraction
   * @param detectorIndex Detector index (0 - EMCAL, 1 - DCAL)
   * @return Median measured for subtraction in the detector by the opposite detector
   * 
   * The median value is stored for the detector, calulated by the opposite
   * side detector. I.e. median value 0 has to be subtracted from EMCAL patches
   * scaled with the patch size and median value 1 has to be subtracted from 
   * DCAL patches scaled by the patch size. The median is not normalized by the
   * area and must be divided by the area of background patches (8x8) before scaling
   * with the area of gamma/jet patches and subtracting from the patches.
   */
  Int_t   GetMedian(       Int_t detectorIndex              ) const {return fMedian[detectorIndex];}
  
  Int_t   GetTriggerBitWord(                                ) const {return fTriggerBitWord;}
  void    GetTriggerBitWord( Int_t& bw                      ) const {bw = fTriggerBitWord;}

  /**
   * @brief Forward to next trigger entry (fastor / L0 patch)
   * @return True if successful (next entry existing), false otherwise (was already at the end of the buffer)
   * 
   * After forwarding the internal iterator the getters will access the entries 
   * of the next FastOR. Event-related observables are not affected. The result
   * of the forwarding must be checked for true in order to determine whether the
   * iterator reached the end.
   */
  virtual Bool_t Next();
   
  /**
   * @brief Copy information of this object into another calo trigger object
   * @param obj Target object the information is copied to
   * 
   * Performing a deep copy, so the object the
   * information is copied to will manage its own
   * memory. Used in copy constructor and assignment
   * operator.
   */ 
  virtual void Copy(TObject& obj) const;
  
  /**
   * @brief Print information about current trigger object
   * @param opt Optional settings (not used)
   */
  virtual void Print(const Option_t* opt = "") const;
    
private:

  Int_t    fNEntries;					///< Number of entries in the trigger object (usually mapped to fastor channels)
  Int_t    fCurrent;					///< Index of the current entry

  /** Array of col positions for a trigger entry, one entry corresponds to a certain fastor channel */
  Int_t*   fColumn;             // [fNEntries]

  /** Array of row positions for a trigger entry, one entry corresponds to a certain fastor channel */
  Int_t*   fRow;                // [fNEntries]

  /** Array with L0 amplitudes for a trigger entry, one entry corresponds to a certain fastor channel */
  Float_t* fAmplitude;          // [fNEntries]

  /** Array of trigger times, one entry corresponds to a certain fastor channel */
  Float_t* fTime;               // [fNEntries]

  /** Array of the number of Level0 times, one entry corresponds to a certain L0 patch */
  Int_t*   fNL0Times;           // [fNEntries]

  /** Array of Level0 times, one entry corresponds to a certain L0 patch */
  TArrayI* fL0Times;            //

    /** Array of the L1 time sums (L1 ADC values), one entry corresponds to a certain fastor channel */
  Int_t*   fL1TimeSum;          // [fNEntries]

  /** Array of trigger bits: Each bit position corresponds to a certain trigger (L0/L1) fired. Note
   * that there is a MC offset to be taken into account. Reconsturcted triggers start from the position
   * of the MC offset. Trigger bits are defined in AliEMCALTriggerTypes.h. One entry correspons to a
   * certain fastor position.
   */
  Int_t*   fTriggerBits;        // [fNEntries]
    
  Int_t    fL1Threshold[4];     ///< L1 thresholds from raw data
  Int_t    fL1V0[2];            ///< L1 threshold components
  Int_t    fL1FrameMask;        ///< Validation flag for L1 data
    
  Int_t    fL1DCALThreshold[4]; ///< L1 thresholds from raw data
  Int_t*   fL1SubRegion;        // [fNEntries]
  Int_t    fL1DCALFrameMask;    ///< Validation flag for L1 data
  Int_t    fMedian[2];          ///< Background median, measured for detector (0 - EMCAL, 1 - DCAL)
  Int_t    fTriggerBitWord;     ///< Trigger bit word
  Int_t    fL1DCALV0[2];        ///< L1 threshold components

  ClassDef(AliAODCaloTrigger, 5)
};
#endif

