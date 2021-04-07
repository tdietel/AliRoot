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

/// \file GPUReconstructionCUDArtc.cu
/// \author David Rohr

#include "GPUReconstructionCUDADef.h"
#include "GPUReconstructionIncludesDevice.h"

#ifndef GPUCA_GPUCODE_DEVICE
#error RTC Preprocessing must run on device code
#endif
#ifdef GPUCA_NO_CONSTANT_MEMORY
#error CUDA RTC does not support processing without constant memory
#endif

extern "C" {
#undef GPUCA_KRNL_REG
#define GPUCA_KRNL_REG(args) __launch_bounds__(GPUCA_M_MAX2_3(GPUCA_M_STRIP(args)))
#define GPUCA_KRNL(x_class, x_attributes, x_arguments, x_forward) GPUCA_KRNL_WRAP(GPUCA_KRNL_LOAD_, x_class, x_attributes, x_arguments, x_forward)
#define GPUCA_KRNL_LOAD_single(x_class, x_attributes, x_arguments, x_forward) GPUCA_KRNLGPU_SINGLE(x_class, x_attributes, x_arguments, x_forward)
#define GPUCA_KRNL_LOAD_multi(x_class, x_attributes, x_arguments, x_forward) GPUCA_KRNLGPU_MULTI(x_class, x_attributes, x_arguments, x_forward)
#include "GPUReconstructionKernels.h"
#undef GPUCA_KRNL
}
