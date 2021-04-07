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

/// \file GPUTPCSliceOutput.cxx
/// \author Sergey Gorbunov, Ivan Kisel, David Rohr

#include "GPUOutputControl.h"
#include "GPUTPCSliceOutput.h"
#include "GPUCommonMath.h"
#include <atomic>

using namespace GPUCA_NAMESPACE::gpu;

unsigned int GPUTPCSliceOutput::EstimateSize(unsigned int nOfTracks, unsigned int nOfTrackClusters)
{
  // calculate the amount of memory [bytes] needed for the event
  return sizeof(GPUTPCSliceOutput) + sizeof(GPUTPCTrack) * nOfTracks + sizeof(GPUTPCSliceOutCluster) * nOfTrackClusters;
}

#ifndef GPUCA_GPUCODE
void GPUTPCSliceOutput::Allocate(GPUTPCSliceOutput*& ptrOutput, int nTracks, int nTrackHits, GPUOutputControl* outputControl, void*& internalMemory)
{
  // Allocate All memory needed for slice output
  const size_t memsize = EstimateSize(nTracks, nTrackHits);

  if (outputControl && outputControl->useExternal()) {
    static std::atomic_flag lock = ATOMIC_FLAG_INIT;
    while (lock.test_and_set(std::memory_order_acquire)) {
    }
    outputControl->checkCurrent();
    if (outputControl->size - ((char*)outputControl->ptrCurrent - (char*)outputControl->ptrBase) < memsize) {
      outputControl->size = 1;
      ptrOutput = nullptr;
      lock.clear(std::memory_order_release);
      return;
    }
    ptrOutput = reinterpret_cast<GPUTPCSliceOutput*>(outputControl->ptrCurrent);
    outputControl->ptrCurrent = (char*)outputControl->ptrCurrent + memsize;
    lock.clear(std::memory_order_release);
  } else {
    if (internalMemory) {
      free(internalMemory);
    }
    internalMemory = malloc(memsize);
    ptrOutput = reinterpret_cast<GPUTPCSliceOutput*>(internalMemory);
  }
  ptrOutput->SetMemorySize(memsize);
}
#endif
