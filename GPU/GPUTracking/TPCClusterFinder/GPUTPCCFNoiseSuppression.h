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

/// \file NoiseSuppression.h
/// \author Felix Weiglhofer

#ifndef O2_GPU_NOISE_SUPPRESSION_H
#define O2_GPU_NOISE_SUPPRESSION_H

#include "clusterFinderDefs.h"
#include "GPUGeneralKernels.h"
#include "GPUConstantMem.h"
#include "GPUTPCClusterFinder.h"
#include "Array2D.h"
#include "PackedCharge.h"

namespace GPUCA_NAMESPACE::gpu
{

struct ChargePos;

class GPUTPCCFNoiseSuppression : public GPUKernelTemplate
{

 public:
  enum K : int {
    noiseSuppression = 0,
    updatePeaks = 1,
  };

  struct GPUSharedMemory {
    ChargePos posBcast[SCRATCH_PAD_WORK_GROUP_SIZE];
    PackedCharge buf[SCRATCH_PAD_WORK_GROUP_SIZE * SCRATCH_PAD_NOISE_N];
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

  template <int iKernel = defaultKernel, typename... Args>
  GPUd() static void Thread(int nBlocks, int nThreads, int iBlock, int iThread, GPUSharedMemory& smem, processorType& clusterer, Args... args);

 private:
  static GPUd() void noiseSuppressionImpl(int, int, int, int, GPUSharedMemory&, const GPUSettingsRec&, const Array2D<PackedCharge>&, const Array2D<uchar>&, const ChargePos*, const uint, uchar*);

  static GPUd() void updatePeaksImpl(int, int, int, int, const ChargePos*, const uchar*, const uint, Array2D<uchar>&);

  static GPUdi() void checkForMinima(float, float, PackedCharge, int, ulong*, ulong*);

  static GPUdi() void findMinima(const PackedCharge*, const ushort, const int, int, const float, const float, ulong*, ulong*);

  static GPUdi() void findPeaks(const uchar*, const ushort, const int, int, ulong*);

  static GPUdi() bool keepPeak(ulong, ulong);

  static GPUd() void findMinimaAndPeaks(const Array2D<PackedCharge>&, const Array2D<uchar>&, const GPUSettingsRec&, float, const ChargePos&, ChargePos*, PackedCharge*, ulong*, ulong*, ulong*);
};

} // namespace GPUCA_NAMESPACE::gpu

#endif
