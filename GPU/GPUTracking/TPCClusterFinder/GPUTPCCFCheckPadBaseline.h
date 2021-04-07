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

/// \file GPUTPCCFCheckPadBaseline.h
/// \author Felix Weiglhofer

#ifndef O2_GPU_GPU_TPC_CF_CHECK_PAD_BASELINE_H
#define O2_GPU_GPU_TPC_CF_CHECK_PAD_BASELINE_H

#include "GPUGeneralKernels.h"
#include "GPUConstantMem.h"

#include "clusterFinderDefs.h"

namespace GPUCA_NAMESPACE::gpu
{

class GPUTPCCFCheckPadBaseline : public GPUKernelTemplate
{

 private:
  // Only use these constants on device side...
  // Use getPadsPerBlock() for host side
  enum {
    PadsPerBlockGPU = 4, // Number of pads in a single cache line
    PadsPerBlockCPU = 1,
#ifdef GPUCA_GPUCODE
    PadsPerBlock = PadsPerBlockGPU,
#else
    PadsPerBlock = PadsPerBlockCPU,
#endif
    NumOfCachedTimebins = GPUCA_GET_THREAD_COUNT(GPUCA_LB_GPUTPCCFCheckPadBaseline) / PadsPerBlock,
  };

 public:
  struct GPUSharedMemory {
    tpccf::Charge charges[PadsPerBlock][NumOfCachedTimebins];
  };

#ifdef HAVE_O2HEADERS
  typedef GPUTPCClusterFinder processorType;
  GPUhdi() static processorType* Processor(GPUConstantMem& processors)
  {
    return processors.tpcClusterer;
  }
#endif

  GPUhdi() CONSTEXPR static GPUDataTypes::RecoStep GetRecoStep()
  {
    return GPUDataTypes::RecoStep::TPCClusterFinding;
  }

  // Use this to get num of pads per block on host side. Can't use constant there.
  static int getPadsPerBlock(bool isGPU)
  {
    return (isGPU) ? PadsPerBlockGPU : PadsPerBlockCPU;
  }

  template <int iKernel = defaultKernel>
  GPUd() static void Thread(int nBlocks, int nThreads, int iBlock, int iThread, GPUSharedMemory& smem, processorType& clusterer);

 private:
  GPUd() static ChargePos padToChargePos(int pad, const GPUTPCClusterFinder&);
};

} // namespace GPUCA_NAMESPACE::gpu

#endif
