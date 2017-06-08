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

/* $Id: AliTRDCalDCSv2.cxx 18952 2007-06-08 11:36:12Z cblume $ */

///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  TRD calibration class v2 for TRD DCS parameters                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <AliLog.h>
#include "AliTRDCalDCSv2.h"
#include "AliTRDCalDCSFEEv2.h"
#include "AliTRDCalDCSGTU.h"

ClassImp(AliTRDCalDCSv2)
  
//_____________________________________________________________________________
AliTRDCalDCSv2::AliTRDCalDCSv2()
  :TNamed()
  ,fGNumberOfTimeBins(-1)
  ,fGConfigTag(-1)
  ,fGSingleHitThres(-1)
  ,fGThreePadClustThres(-1)
  ,fGSelNoZS(-1)
  ,fGTCFilterWeight(-1)
  ,fGTCFilterShortDecPar(-1)
  ,fGTCFilterLongDecPar(-1)
  ,fGFastStatNoise(-1)
  ,fGConfigVersion(0)
  ,fGConfigName(0)
  ,fGFilterType(0)
  ,fGReadoutParam(0)
  ,fGTestPattern(0)
  ,fGTrackletMode(0)
  ,fGTrackletDef(0)
  ,fGTriggerSetup(0)
  ,fGAddOptions(0)
  ,fRunType("")
  ,fStartTime(0)
  ,fEndTime(0)
  ,fFEEArr(new TObjArray(540))
  ,fPTRArr(new TObjArray(6))
  ,fGTUObj(new AliTRDCalDCSGTU())
{
  //
  // AliTRDCalDCSv2 default constructor
  //
}

//_____________________________________________________________________________
AliTRDCalDCSv2::AliTRDCalDCSv2(const Text_t *name, const Text_t *title)
  :TNamed(name,title)
  ,fGNumberOfTimeBins(-1)
  ,fGConfigTag(-1)
  ,fGSingleHitThres(-1)
  ,fGThreePadClustThres(-1)
  ,fGSelNoZS(-1)
  ,fGTCFilterWeight(-1)
  ,fGTCFilterShortDecPar(-1)
  ,fGTCFilterLongDecPar(-1)
  ,fGFastStatNoise(-1)
  ,fGConfigVersion(0)
  ,fGConfigName(0)
  ,fGFilterType(0)
  ,fGReadoutParam(0)
  ,fGTestPattern(0)
  ,fGTrackletMode(0)
  ,fGTrackletDef(0)
  ,fGTriggerSetup(0)
  ,fGAddOptions(0)
  ,fRunType("")
  ,fStartTime(0)
  ,fEndTime(0)
  ,fFEEArr(new TObjArray(540))
  ,fPTRArr(new TObjArray(6))
  ,fGTUObj(new AliTRDCalDCSGTU())
{
  //
  // AliTRDCalDCSv2 constructor
  //
}

//_____________________________________________________________________________
AliTRDCalDCSv2::AliTRDCalDCSv2(const AliTRDCalDCSv2 &cd)
  :TNamed(cd)
  ,fGNumberOfTimeBins(-1)
  ,fGConfigTag(-1)
  ,fGSingleHitThres(-1)
  ,fGThreePadClustThres(-1)
  ,fGSelNoZS(-1)
  ,fGTCFilterWeight(-1)
  ,fGTCFilterShortDecPar(-1)
  ,fGTCFilterLongDecPar(-1)
  ,fGFastStatNoise(-1)
  ,fGConfigVersion(0)
  ,fGConfigName(0)
  ,fGFilterType(0)
  ,fGReadoutParam(0)
  ,fGTestPattern(0)
  ,fGTrackletMode(0)
  ,fGTrackletDef(0)
  ,fGTriggerSetup(0)
  ,fGAddOptions(0)
  ,fRunType("")
  ,fStartTime(0)
  ,fEndTime(0)
  ,fFEEArr(new TObjArray(540))
  ,fPTRArr(new TObjArray(6))
  ,fGTUObj(new AliTRDCalDCSGTU())
{
  //
  // AliTRDCalDCSv2 copy constructor
  //
}

//_____________________________________________________________________________
AliTRDCalDCSv2 &AliTRDCalDCSv2::operator=(const AliTRDCalDCSv2 &cd)
{
  //
  // Assignment operator
  //
  if (&cd == this) return *this;

  new (this) AliTRDCalDCSv2(cd);
  return *this;
}

//_____________________________________________________________________________
void AliTRDCalDCSv2::EvaluateGlobalParameters()
{
  //
  // Do an evaluation of all global parameters
  //

  for(Int_t i=0; i<540; i++) {
    AliTRDCalDCSFEEv2 *iDCSFEEObj;
    iDCSFEEObj = GetCalDCSFEEObj(i);
    if(iDCSFEEObj != NULL) {
      if(iDCSFEEObj->GetStatusBit() == 0) {
	// first, set the parameters of the first good ROC as global
	fGNumberOfTimeBins    = iDCSFEEObj->GetNumberOfTimeBins();
	fGConfigTag           = iDCSFEEObj->GetConfigTag();
	fGSingleHitThres      = iDCSFEEObj->GetSingleHitThres();
	fGThreePadClustThres  = iDCSFEEObj->GetThreePadClustThres();
	fGSelNoZS             = iDCSFEEObj->GetSelectiveNoZS();
	fGTCFilterWeight      = iDCSFEEObj->GetTCFilterWeight();
	fGTCFilterShortDecPar = iDCSFEEObj->GetTCFilterShortDecPar();
	fGTCFilterLongDecPar  = iDCSFEEObj->GetTCFilterLongDecPar();
	fGFastStatNoise       = iDCSFEEObj->GetFastStatNoise();
	fGConfigVersion       = iDCSFEEObj->GetConfigVersion();
	fGConfigName          = iDCSFEEObj->GetConfigName();
	fGFilterType          = iDCSFEEObj->GetFilterType();
	fGReadoutParam        = iDCSFEEObj->GetReadoutParam();
	fGTestPattern         = iDCSFEEObj->GetTestPattern();
	fGTrackletMode        = iDCSFEEObj->GetTrackletMode();
	fGTrackletDef         = iDCSFEEObj->GetTrackletDef();
	fGTriggerSetup        = iDCSFEEObj->GetTriggerSetup();
	fGAddOptions          = iDCSFEEObj->GetAddOptions();
	break;
      }
    }
  }

  for(Int_t i=0; i<540; i++) {
    AliTRDCalDCSFEEv2 *iDCSFEEObj;
    iDCSFEEObj = GetCalDCSFEEObj(i);
    if(iDCSFEEObj != NULL) {
      if(iDCSFEEObj->GetStatusBit() == 0) {
	// second, if any of the other good chambers differ, set the global value to -1/""
	if(fGNumberOfTimeBins    != iDCSFEEObj->GetNumberOfTimeBins())    fGNumberOfTimeBins    = -2;
	if(fGConfigTag           != iDCSFEEObj->GetConfigTag())           fGConfigTag           = -2;
	if(fGSingleHitThres      != iDCSFEEObj->GetSingleHitThres())      fGSingleHitThres      = -2;
	if(fGThreePadClustThres  != iDCSFEEObj->GetThreePadClustThres())  fGThreePadClustThres  = -2;
	if(fGSelNoZS             != iDCSFEEObj->GetSelectiveNoZS())       fGSelNoZS             = -2;
	if(fGTCFilterWeight      != iDCSFEEObj->GetTCFilterWeight())      fGTCFilterWeight      = -2;
	if(fGTCFilterShortDecPar != iDCSFEEObj->GetTCFilterShortDecPar()) fGTCFilterShortDecPar = -2;
	if(fGTCFilterLongDecPar  != iDCSFEEObj->GetTCFilterLongDecPar())  fGTCFilterLongDecPar  = -2;
	if(fGFastStatNoise       != iDCSFEEObj->GetFastStatNoise())       fGFastStatNoise       = -2;
	if(fGConfigVersion       != iDCSFEEObj->GetConfigVersion())       fGConfigVersion       = "mixed";
	if(fGConfigName          != iDCSFEEObj->GetConfigName())          fGConfigName          = "mixed";
	if(fGFilterType          != iDCSFEEObj->GetFilterType())          fGFilterType          = "mixed";
	if(fGReadoutParam        != iDCSFEEObj->GetReadoutParam())        fGReadoutParam        = "mixed";
	if(fGTestPattern         != iDCSFEEObj->GetTestPattern())         fGTestPattern         = "mixed";
	if(fGTrackletMode        != iDCSFEEObj->GetTrackletMode())        fGTrackletMode        = "mixed";
	if(fGTrackletDef         != iDCSFEEObj->GetTrackletDef())         fGTrackletDef         = "mixed";
	if(fGTriggerSetup        != iDCSFEEObj->GetTriggerSetup())        fGTriggerSetup        = "mixed";
	if(fGAddOptions          != iDCSFEEObj->GetAddOptions())          fGAddOptions          = "mixed";
      }
    }
  }
}
//_________________________________________________________
template <typename T>
T AliTRDCalDCSv2::GetPluralityOf(T (AliTRDCalDCSFEEv2::*fct)() const){
  //
  // Calls the function fct on each element of the FEE array and
  // determines the plurality of the return values of type T
  //
  std::map<const T,Int_t> map;
  // Loop over all chambers
  const Int_t ndcs = TMath::Min(GetFEEArr()?GetFEEArr()->GetSize():0,540);
  for(Int_t idcs=0;idcs<ndcs;idcs++) {
    // Get the config name of this FEE
    const AliTRDCalDCSFEEv2 *fee = GetCalDCSFEEObj(idcs);
    if(!fee)continue;
    const T cfg = (fee->*fct)();
    // We try to insert the cfg with one occurence
    auto p = map.insert(std::make_pair(cfg,1));
    // p is a pair of <element, bool> where bool indicates
    //  whether insertion was a) successful - nothing to do
    //  or b) unsuccessful - we increment the counter of the
    //  returned object which is the object which prevented
    //  insertion (because it has the same key/cfg value)
    if(!p.second){
      p.first->second++;
    }
  } // chambers

  // Get the config value with the plurality, i.e. the config
  //  which is applied the most numerous. For this we create
  //  a multimap with number of occurrences - config value. A
  //  multimap (instead of a map) is needed as multiple con-
  //  fig values might have the same number of occurrence.
  std::multimap<Int_t, const T,std::greater<Int_t>> rmap;
  std::transform(map.cbegin(), map.cend(),
                 std::inserter(rmap, rmap.begin()),
                 [](const std::pair<const T,Int_t> &org)
                   ->std::pair<Int_t,const T>{
                   return std::make_pair(org.second,org.first);} );
  // Print
  AliInfoStream() << "# occurences" << " - " << "config value" << "\n";
  for(auto e:rmap){
    AliInfoStream() <<  std::setw(12) << std::setfill(' ') << // pad integer with ' ' to twelve characters
      e.first << " - " << e.second << "\n";
  }
  // Return most numerous config value
  return rmap.cbegin()->second;
  //rslt = rmap.cbegin()->second;

}
//_________________________________________________________
void AliTRDCalDCSv2::ForcePluralityConfig() {
  //
  // WARNING DO NOT USE THIS FUNCTION LIGHTLY
  //  This sets the global configuration to
  //  the configuration of the plurality of
  //  chambers. This means chambers which do
  //  not belong to the plurality are recon-
  //  structed with a WRONG configuration
  //

  AliInfoStream()
    << " ---- Global Configuration Before Fixing ---- \n"
    << "fGNumberOfTimeBins    - " << fGNumberOfTimeBins << "\n"
    << "fGConfigTag           - " << fGConfigTag << "\n"
    << "fGSingleHitThres      - " << fGSingleHitThres << "\n"
    << "fGThreePadClustThres  - " << fGThreePadClustThres << "\n"
    << "fGSelNoZS             - " << fGSelNoZS << "\n"
    << "fGTCFilterWeight      - " << fGTCFilterWeight << "\n"
    << "fGTCFilterShortDecPar - " << fGTCFilterShortDecPar << "\n"
    << "fGTCFilterLongDecPar  - " << fGTCFilterLongDecPar << "\n"
    << "fGFastStatNoise       - " << fGFastStatNoise << "\n"
    << "fGConfigVersion       - " << fGConfigVersion << "\n"
    << "fGConfigName          - " << fGConfigName << "\n"
    << "fGFilterType          - " << fGFilterType << "\n"
    << "fGReadoutParam        - " << fGReadoutParam << "\n"
    << "fGTestPattern         - " << fGTestPattern << "\n"
    << "fGTrackletMode        - " << fGTrackletMode << "\n"
    << "fGTrackletDef         - " << fGTrackletDef << "\n"
    << "fGTriggerSetup        - " << fGTriggerSetup << "\n"
    << "fGAddOptions          - " << fGAddOptions << "\n"
    << " -------------------------------------------- \n";
  // bal
 
  fGNumberOfTimeBins    = GetPluralityOf(&AliTRDCalDCSFEEv2::GetNumberOfTimeBins);
  fGNumberOfTimeBins    = GetPluralityOf(&AliTRDCalDCSFEEv2::GetNumberOfTimeBins);
  fGConfigTag           = GetPluralityOf(&AliTRDCalDCSFEEv2::GetConfigTag);
  fGSingleHitThres      = GetPluralityOf(&AliTRDCalDCSFEEv2::GetSingleHitThres);
  fGThreePadClustThres  = GetPluralityOf(&AliTRDCalDCSFEEv2::GetThreePadClustThres);
  fGSelNoZS             = GetPluralityOf(&AliTRDCalDCSFEEv2::GetSelectiveNoZS);
  fGTCFilterWeight      = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTCFilterWeight);
  fGTCFilterShortDecPar = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTCFilterShortDecPar);
  fGTCFilterLongDecPar  = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTCFilterLongDecPar);
  fGFastStatNoise       = GetPluralityOf(&AliTRDCalDCSFEEv2::GetFastStatNoise);
  fGConfigVersion       = GetPluralityOf(&AliTRDCalDCSFEEv2::GetConfigVersion);
  fGConfigName          = GetPluralityOf(&AliTRDCalDCSFEEv2::GetConfigName);
  fGFilterType          = GetPluralityOf(&AliTRDCalDCSFEEv2::GetFilterType);
  fGReadoutParam        = GetPluralityOf(&AliTRDCalDCSFEEv2::GetReadoutParam);
  fGTestPattern         = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTestPattern);
  fGTrackletMode        = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTrackletMode);
  fGTrackletDef         = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTrackletDef);
  fGTriggerSetup        = GetPluralityOf(&AliTRDCalDCSFEEv2::GetTriggerSetup);
  fGAddOptions          = GetPluralityOf(&AliTRDCalDCSFEEv2::GetAddOptions);

       
  AliInfoStream()
    << " ---- Global Configuration After Fixing  ---- \n"
    << "fGNumberOfTimeBins    - " << fGNumberOfTimeBins << "\n"
    << "fGConfigTag           - " << fGConfigTag << "\n"
    << "fGSingleHitThres      - " << fGSingleHitThres << "\n"
    << "fGThreePadClustThres  - " << fGThreePadClustThres << "\n"
    << "fGSelNoZS             - " << fGSelNoZS << "\n"
    << "fGTCFilterWeight      - " << fGTCFilterWeight << "\n"
    << "fGTCFilterShortDecPar - " << fGTCFilterShortDecPar << "\n"
    << "fGTCFilterLongDecPar  - " << fGTCFilterLongDecPar << "\n"
    << "fGFastStatNoise       - " << fGFastStatNoise << "\n"
    << "fGConfigVersion       - " << fGConfigVersion << "\n"
    << "fGConfigName          - " << fGConfigName << "\n"
    << "fGFilterType          - " << fGFilterType << "\n"
    << "fGReadoutParam        - " << fGReadoutParam << "\n"
    << "fGTestPattern         - " << fGTestPattern << "\n"
    << "fGTrackletMode        - " << fGTrackletMode << "\n"
    << "fGTrackletDef         - " << fGTrackletDef << "\n"
    << "fGTriggerSetup        - " << fGTriggerSetup << "\n"
    << "fGAddOptions          - " << fGAddOptions << "\n"
    << " -------------------------------------------- \n";
  
}

