#ifndef ALIVCALOTRIGGER_H
#define ALIVCALOTRIGGER_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

#include <TNamed.h>

/**
 * @class AliVCaloTrigger
 * @brief  Virtual class to access calorimeter  (EMCAL, PHOS, PMD, FMD) trigger data
 * @author Salvatore Aiola
 */
class AliVCaloTrigger : public TNamed 
{
public:
  /**
   * @brief Dummy onstructor
   */
  AliVCaloTrigger(): TNamed() {;}

  /**
   * @brief Named constructor
   * @param name Name of the new object
   * @param title Title of the new object
   */
  AliVCaloTrigger(const char* name, const char* title) : TNamed(name, title) {;}

  /**
   * @brief Copy constructor
   * @param ctrig Reference for copy
   * 
   * Copy is performed as deep copy so the new
   * object manages its own entries. Content handled
   * by object previously is destroyed.
   */
  AliVCaloTrigger(const AliVCaloTrigger& ctrig);

  /**
   * @brief Destructor
   */
  virtual ~AliVCaloTrigger() {;}

  /**
   * @brief Assignemtn operator
   * @param ctrig Reference for assignment
   * @return Calo trigger object after assignment 
   * 
   * Assignment is performed as deep copy so the new
   * object manages its own entries. Content handled
   * by object previously is destroyed.
   */
  AliVCaloTrigger& operator=(const AliVCaloTrigger& ctrig);
  
  /**
   * @brief Check whether calo trigger object carries entries
   * @return True if object does not carry data (is empty), false otherwise
   */
  virtual Bool_t       IsEmpty()                                             = 0;

  /**
   * @brief Reset internal iterator
   * 
   * Next iteration (via function Next()) will start 
   * from first entry in object
   */
  virtual void         Reset()                                               = 0;

  /**
   * @brief Allocate memory for new entries
   * @param size Number of entries to be handled by the calo trigger object
   */
  virtual void         Allocate(Int_t /*size*/)                              = 0;

  /**
   * @brief Delete entries handled by the object and free memory
   */
  virtual void         DeAllocate()                                          = 0;
  
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
  virtual Bool_t       Add(Int_t /*col*/, Int_t /*row*/, 
                           Float_t /*amp*/, Float_t /*time*/, 
                           Int_t* /*trgtimes*/, Int_t /*ntrgtimes*/, 
                           Int_t /*trgtimesum*/, Int_t /*trgbits*/)               = 0;

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
  virtual Bool_t       Add(Int_t /*col*/, Int_t /*row*/, 
                           Float_t /*amp*/, Float_t /*time*/, 
                           Int_t* /*trgtimes*/, Int_t /*ntrgtimes*/, 
                           Int_t /*trgtimesum*/, Int_t /*subr*/, Int_t /*trgbit*/)= 0;

  
  virtual void         SetL1Threshold(Int_t /*i*/, Int_t /*thr*/)            = 0;
  virtual void         SetL1Threshold(Int_t /*i*/, Int_t /*j*/, Int_t /*th*/)= 0;
  
  virtual void         SetL1V0(const Int_t* /*v*/)                           = 0;
  virtual void         SetL1V0(Int_t /*i*/, const Int_t* /*v*/)              = 0;
  
  virtual void         SetL1FrameMask(Int_t /*m*/)                           = 0;
  virtual void         SetL1FrameMask(Int_t /*i*/, Int_t /*m*/)              = 0;

  /**
   * @brief Access to position of the current fastor channel
   * @param[out] col Column of the current fastor in the detector plane
   * @param[out] row Row of the current fastor in the detector plane
   */
  virtual void         GetPosition(Int_t& /*col*/, Int_t& /*row*/)    const  = 0;

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
  virtual void         GetAmplitude(Float_t& /*amp*/)                 const  = 0;
  
  /**
   * @brief Get time of the peak amplitude
   * @param time Time of the peak amplitude
   */
  virtual void         GetTime(Float_t& /*time*/)                     const  = 0;
  
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
  virtual void         GetTriggerBits(Int_t& /*bits*/)                const  = 0;

  /**
   * @brief Get the number of L0 times for the current patch
   *
   * Level0 times are handled per L0 patch. Indexing is different
   * with respect to the fastor indexing.
   *
   * @param[out] ntimes Number of level0
   */
  virtual void         GetNL0Times(Int_t& /*ntimes*/)                 const  = 0;

  /**
   * @brief Get level0 times for the current L0 patch
   * @param times L0 times for the current L0 patch
   */
  virtual void         GetL0Times(Int_t* /*times*/)                   const  = 0;

  /**
   * @brief Get the number of entries in the trigger data
   * @return Number of entries
   */
  virtual Int_t        GetEntries()                                   const  = 0;

  /**
   * @brief Get the L1 time sums (L1 ADC values) for the current fastor
   * @param[out] timesum L1 timesums for the current fastor
   */
  virtual void         GetL1TimeSum(Int_t& /*timesum*/)               const  = 0;

  /**
   * @brief Get the L1 time sums (L1 ADC values) for the current fastor
   * @return L1 timesums for the current fastor
   */
  virtual Int_t        GetL1TimeSum()                                 const  = 0;
  
  /**
   * @brief Get PHOS subregion value stored in current position
   * @param[out] subreg PHOS subregion value for the current postion
   * 
   * PHOS subregions, used for the combined DCAL-PHOS jet trigger,
   * are stored in as extra entries for the current position in column-
   * row space. In the trigger patch calculation they are to be used
   * as normal L1 timesums.
   */
  virtual void         GetL1SubRegion(  Int_t& /*subreg*/)            const  = 0;

  /**
   * @brief Get PHOS subregion value stored in current position
   * @return PHOS subregion value for the current postion 
   * 
   * PHOS subregions, used for the combined DCAL-PHOS jet trigger,
   * are stored in as extra entries for the current position in column-
   * row space. In the trigger patch calculation they are to be used
   * as normal L1 timesums.
   */
  virtual Int_t        GetL1SubRegion()                               const  = 0;
  
  virtual Int_t        GetL1Threshold(Int_t /*i*/)                    const  = 0;
  virtual Int_t        GetL1Threshold(Int_t /*i*/, Int_t /*j*/)       const  = 0;
  
  virtual Int_t        GetL1V0(Int_t /*i*/)                           const  = 0;
  virtual Int_t        GetL1V0(Int_t /*i*/, Int_t /*j*/)              const  = 0;
  
  virtual Int_t        GetL1FrameMask()                               const  = 0;
  virtual Int_t        GetL1FrameMask(Int_t /*i*/)                    const  = 0;
 
  /**
   * @brief Get Median for background subtraction
   * @param i Detector index (0 - EMCAL, 1 - DCAL)
   * @return Median measured for subtraction in the detector by the opposite detector
   * 
   * The median value is stored for the detector, calulated by the opposite
   * side detector. I.e. median value 0 has to be subtracted from EMCAL patches
   * scaled with the patch size and median value 1 has to be subtracted from 
   * DCAL patches scaled by the patch size. The median is not normalized by the
   * area and must be divided by the area of background patches (8x8) before scaling
   * with the area of gamma/jet patches and subtracting from the patches.
   */
  virtual Int_t        GetMedian(Int_t /*detectorIndex*/)             const  = 0;
  
  virtual Int_t        GetTriggerBitWord()                            const  = 0;
  virtual void         GetTriggerBitWord(Int_t& /*bw*/ )              const  = 0;

  /**
   * @brief Forward to next trigger entry (fastor / L0 patch)
   * @return True if successful (next entry existing), false otherwise (was already at the end of the buffer)
   * 
   * After forwarding the internal iterator the getters will access the entries 
   * of the next FastOR. Event-related observables are not affected. The result
   * of the forwarding must be checked for true in order to determine whether the
   * iterator reached the end.
   */
  virtual Bool_t       Next()                                                = 0;

  /**
   * @brief Copy information of this object into another calo trigger object
   * @param obj Target object the information is copied to
   * 
   * Performing a deep copy, so the object the
   * information is copied to will manage its own
   * memory. Used in copy constructor and assignment
   * operator.
   */
  virtual void         Copy(TObject& obj)                             const     ;

  /**
   * @brief Print information about current trigger object
   * @param opt Optional settings (not used)
   */
  virtual void         Print(const Option_t* /*opt*/)                 const  = 0;
  
private:

  ClassDef(AliVCaloTrigger, 0)
};
#endif //ALIVCALOTRIGGER_H

