// -*- mode: C++ -*- 
#ifndef ALIESDUTILS_H
#define ALIESDUTILS_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */


/* $Id$ */

//-------------------------------------------------------------------------
//   AliESDUtils - This is a namespace that temporary provides general 
//                 purpose ESD utilities to avoid unnecessary dependencies
//                 between PWG libraries. To be removed/replaced by AODB
//                 framework.
//      
//-------------------------------------------------------------------------
// Author: Andrei Gheata

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include <TObjArray.h>
#include "TVectorFfwd.h"

class AliVEvent;
class AliESDEvent;
class AliVertexerTracks;
class TClonesArray;

namespace AliESDUtils {

  /********************************/
  /* Centrality-related corrections */
  /********************************/

  Float_t GetCorrV0(const AliVEvent* esd, Float_t &v0CorrResc, Float_t *v0multChCorr = NULL, Float_t *v0multChCorrResc = NULL);
  Float_t GetCorrSPD2(Float_t spd2raw,Float_t zv);
  TObjArray*  RefitESDVertexTracks(AliESDEvent* esdEv, Int_t algo=6, const Double_t* cuts=0, TClonesArray* extDest=0);
  Float_t GetCorrV0A0(Float_t v0araw,Float_t zv);
  Float_t GetCorrV0A(Float_t v0araw,Float_t zv);
  Float_t GetCorrV0C(Float_t v0craw,Float_t zv);
  void GetTPCPileupVertexInfo(const AliESDEvent* event, TVectorF& vertexInfo);
  void GetITSPileupVertexInfo(const AliESDEvent* event, TVectorF& vertexInfo, Double_t dcaCut=0.05,  Double_t dcaZcut=0.15);
}  

#endif
