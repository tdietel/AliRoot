// -*- mode: c++ -*-

/**************************************************************************
 * This file is property of and copyright by the Experimental Nuclear     *
 * Physics Group, Dep. of Physics                                         *
 * University of Oslo, Norway, 2007                                       *
 *                                                                        *
 * Author: Per Thomas Hille <perthi@fys.uio.no> for the ALICE HLT Project.*
 * Contributors are mentioned in the code where appropriate.              *
 * Please report bugs to perthi@fys.uio.no                                *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "AliCaloRawAnalyzerFactory.h"
#include "AliCaloRawAnalyzerFastFit.h"
#include "AliCaloRawAnalyzerNN.h"
#include "AliCaloRawAnalyzerPeakFinder.h"
#include "AliCaloRawAnalyzerCrude.h"
#include "AliCaloRawAnalyzerKStandard.h"
#include "AliCaloRawAnalyzerKStandardFast.h"
#include "AliCaloRawAnalyzerFakeALTRO.h"
#include "AliCaloRawAnalyzerGamma2.h"

///
/// Default constructor.
//_______________________________________________________________________
AliCaloRawAnalyzerFactory::AliCaloRawAnalyzerFactory()
{ }

///
/// \return the requested algorithm.
//_______________________________________________________________________
AliCaloRawAnalyzer*
AliCaloRawAnalyzerFactory::CreateAnalyzer( const int algo )
{  
  switch ( algo)
  {
    case  kFastFit:
      return new  AliCaloRawAnalyzerFastFit();
      break;
    case kNeuralNet:
      return new AliCaloRawAnalyzerNN();
      break;
    case kPeakFinder:
      return new AliCaloRawAnalyzerPeakFinder();
      break;
    case kCrude:
      return  new AliCaloRawAnalyzerCrude();
      break;
    case kStandard:
      return new AliCaloRawAnalyzerKStandard();
      break;
    case kStandardFast:
      return new AliCaloRawAnalyzerKStandardFast();
      break;
    case kFakeAltro:
      return  new AliCaloRawAnalyzerFakeALTRO();
      break;
    case kGamma2:
      return  new AliCaloRawAnalyzerGamma2();
      break;
    default:
      return  new AliCaloRawAnalyzerCrude();
      break;
  }
}





