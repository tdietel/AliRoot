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

/// \file GPUCommonArray.h
/// \author David Rohr

#ifndef GPUCOMMONFAIRARRAY_H
#define GPUCOMMONFAIRARRAY_H

#ifndef GPUCA_GPUCODE_DEVICE
#include <array>
#endif

#include "GPUCommonDef.h"
namespace o2::gpu::gpustd
{
#ifdef GPUCA_GPUCODE_DEVICE
template <typename T, size_t N>
struct array {
  GPUd() T& operator[](size_t i) { return m_internal_V__[i]; }
  GPUd() const T& operator[](size_t i) const { return m_internal_V__[i]; }
  T m_internal_V__[N];
};
#else
template <typename T, size_t N>
using array = std::array<T, N>;
#endif
} // namespace o2::gpu::gpustd

#endif
