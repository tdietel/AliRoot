#ifndef ALIAODPIDUTIL_H
#define ALIAODPIDUTIL_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id: AliAODpidUtil.h 38493 2010-01-26 16:33:03Z hristov $ */

/// \class AliAODpidUtil
/// \brief Combined PID class
///
/// for the AOD class
/// Modified: Jens Wiechula, Uni Tuebingen, jens.wiechula@cern.ch
/// Modified: Pietro Antonioli, INFN BO, pietro.antonioli@bo.infn.it
///
/// \author Rosa Romita, GSI, r.romita@gsi.de

#include <Rtypes.h>
#include "AliPID.h" // Needed for inline functions

#include "AliPIDResponse.h"

class AliAODEvent;
class AliVParticle;

class AliAODpidUtil : public AliPIDResponse  {
public:
  //TODO: isMC???
  AliAODpidUtil(Bool_t isMC = kFALSE): AliPIDResponse(isMC) {;}
  virtual ~AliAODpidUtil() {;}
  virtual void SetEventPileupProperties(const AliVEvent* vevent);

protected:
  virtual Float_t GetSignalDeltaTOFold(const AliVParticle *track, AliPID::EParticleType type, Bool_t ratio=kFALSE) const;
  virtual Float_t GetNumberOfSigmasTOFold(const AliVParticle *vtrack, AliPID::EParticleType type) const;
  virtual Float_t GetExpectedSignalTOFold(const AliVParticle *vtrack, AliPID::EParticleType type) const;
  virtual Float_t GetExpectedSigmaTOFold(const AliVParticle *vtrack, AliPID::EParticleType type) const;

private:

  ClassDef(AliAODpidUtil,3)  // PID calculation class
};


#endif


