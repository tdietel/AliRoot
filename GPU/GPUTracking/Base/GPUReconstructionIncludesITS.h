//**************************************************************************\
//* This file is property of and copyright by the ALICE Project            *\
//* ALICE Experiment at CERN, All rights reserved.                         *\
//*                                                                        *\
//* Primary Authors: Matthias Richter <Matthias.Richter@ift.uib.no>        *\
//*                  for The ALICE HLT Project.                            *\
//*                                                                        *\
//* Permission to use, copy, modify and distribute this software and its   *\
//* documentation strictly for non-commercial purposes is hereby granted   *\
//* without fee, provided that the above copyright notice appears in all   *\
//* copies and that both the copyright notice and this permission notice   *\
//* appear in the supporting documentation. The authors make no claims     *\
//* about the suitability of this software for any purpose. It is          *\
//* provided "as is" without express or implied warranty.                  *\
//**************************************************************************

/// \file GPUReconstructionIncludesITS.h
/// \author David Rohr

#ifndef GPURECONSTRUCTIONINCLDUESITS_H
#define GPURECONSTRUCTIONINCLDUESITS_H

#if defined(HAVE_O2HEADERS) && !defined(GPUCA_NO_ITS_TRAITS)
#include "ITStracking/TrackerTraitsCPU.h"
#include "ITStracking/VertexerTraits.h"
#else
namespace o2
{
namespace its
{
class TrackerTraits
{
};
class TrackerTraitsCPU : public TrackerTraits
{
};
class VertexerTraits
{
};
} // namespace its
} // namespace o2
#if defined(HAVE_O2HEADERS)
#include "ITStracking/Road.h"
#include "ITStracking/Cluster.h"
#endif
#endif

#endif
