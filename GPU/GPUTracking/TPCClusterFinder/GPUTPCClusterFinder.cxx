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

/// \file GPUTPCClusterFinder.cxx
/// \author David Rohr

#include "GPUTPCClusterFinder.h"
#include "GPUReconstruction.h"
#include "GPUMemorySizeScalers.h"
#include "GPUHostDataTypes.h"

#include "DataFormatsTPC/ZeroSuppression.h"
#include "DataFormatsTPC/Digit.h"

#include "ChargePos.h"
#include "Array2D.h"

using namespace GPUCA_NAMESPACE::gpu;
using namespace o2::tpc;

void GPUTPCClusterFinder::InitializeProcessor()
{
  mMinMaxCN = new MinMaxCN[GPUTrackingInOutZS::NENDPOINTS];
}

GPUTPCClusterFinder::~GPUTPCClusterFinder()
{
  delete[] mMinMaxCN;
  clearMCMemory();
}

void* GPUTPCClusterFinder::SetPointersMemory(void* mem)
{
  computePointerWithAlignment(mem, mPmemory, 1);
  return mem;
}

void* GPUTPCClusterFinder::SetPointersInput(void* mem)
{
  if (mNMaxPages == 0 && (mRec->GetRecoStepsGPU() & GPUDataTypes::RecoStep::TPCClusterFinding)) {
    computePointerWithAlignment(mem, mPdigits, mNMaxDigits);
  }
  return mem;
}

void* GPUTPCClusterFinder::SetPointersZSOffset(void* mem)
{
  const int n = (mRec->GetRecoStepsGPU() & GPUDataTypes::RecoStep::TPCClusterFinding) ? mNMaxPages : GPUTrackingInOutZS::NENDPOINTS;
  if (n) {
    computePointerWithAlignment(mem, mPzsOffsets, n);
  }
  return mem;
}

void* GPUTPCClusterFinder::SetPointersZS(void* mem)
{
  if (mNMaxPages && (mRec->GetRecoStepsGPU() & GPUDataTypes::RecoStep::TPCClusterFinding)) {
    computePointerWithAlignment(mem, mPzs, mNMaxPages * TPCZSHDR::TPC_ZS_PAGE_SIZE);
  }
  return mem;
}

void* GPUTPCClusterFinder::SetPointersOutput(void* mem)
{
  computePointerWithAlignment(mem, mPclusterInRow, GPUCA_ROW_COUNT);
  return mem;
}

void* GPUTPCClusterFinder::SetPointersScratch(void* mem)
{
  computePointerWithAlignment(mem, mPpadHasLostBaseline, TPC_PADS_IN_SECTOR);
  computePointerWithAlignment(mem, mPpositions, mNMaxDigitsFragment);
  computePointerWithAlignment(mem, mPpeakPositions, mNMaxPeaks);
  computePointerWithAlignment(mem, mPfilteredPeakPositions, mNMaxClusters);
  if (mRec->GetProcessingSettings().runMC) {
    computePointerWithAlignment(mem, mPclusterPosInRow, mNMaxClusters);
  }
  computePointerWithAlignment(mem, mPisPeak, mNMaxDigitsFragment);
  computePointerWithAlignment(mem, mPchargeMap, TPCMapMemoryLayout<decltype(*mPchargeMap)>::items());
  computePointerWithAlignment(mem, mPpeakMap, TPCMapMemoryLayout<decltype(*mPpeakMap)>::items());
  computePointerWithAlignment(mem, mPbuf, mBufSize * mNBufs);
  computePointerWithAlignment(mem, mPclusterByRow, GPUCA_ROW_COUNT * mNMaxClusterPerRow);
  return mem;
}

void GPUTPCClusterFinder::RegisterMemoryAllocation()
{
  AllocateAndInitializeLate();
  mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersInput, GPUMemoryResource::MEMORY_INPUT | GPUMemoryResource::MEMORY_GPU | GPUMemoryResource::MEMORY_STACK, "TPCClustererInput");

  int scratchType = GPUMemoryResource::MEMORY_SCRATCH | GPUMemoryResource::MEMORY_STACK;
  if (mRec->GetProcessingSettings().runMC) {
    scratchType |= GPUMemoryResource::MEMORY_HOST | GPUMemoryResource::MEMORY_GPU;
  }
  mScratchId = mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersScratch, scratchType, "TPCClustererScratch", GPUMemoryReuse{GPUMemoryReuse::REUSE_1TO1, GPUMemoryReuse::ClustererScratch, (unsigned short)(mISlice % mRec->GetProcessingSettings().nTPCClustererLanes)});

  mMemoryId = mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersMemory, GPUMemoryResource::MEMORY_PERMANENT, "TPCClustererMemory");
  mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersOutput, GPUMemoryResource::MEMORY_OUTPUT | GPUMemoryResource::MEMORY_STACK, "TPCClustererOutput");
  mZSId = mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersZS, GPUMemoryResource::MEMORY_CUSTOM | GPUMemoryResource::MEMORY_CUSTOM_TRANSFER | GPUMemoryResource::MEMORY_GPU | GPUMemoryResource::MEMORY_STACK, "TPCClustererZSData", GPUMemoryReuse{GPUMemoryReuse::REUSE_1TO1, GPUMemoryReuse::ClustererZS, (unsigned short)(mISlice % mRec->GetProcessingSettings().nTPCClustererLanes)});
  mZSOffsetId = mRec->RegisterMemoryAllocation(this, &GPUTPCClusterFinder::SetPointersZSOffset, GPUMemoryResource::MEMORY_CUSTOM | GPUMemoryResource::MEMORY_CUSTOM_TRANSFER | GPUMemoryResource::MEMORY_INPUT | GPUMemoryResource::MEMORY_STACK, "TPCClustererZSOffsets");
}

void GPUTPCClusterFinder::SetMaxData(const GPUTrackingInOutPointers& io)
{
  mNMaxPeaks = mRec->MemoryScalers()->NTPCPeaks(mNMaxDigitsFragment);
  mNMaxClusters = mRec->MemoryScalers()->NTPCClusters(mNMaxDigitsFragment);
  mNMaxClusterPerRow = 0.01f * mRec->MemoryScalers()->NTPCClusters(mNMaxDigits); // TODO: Can save some memory hery by using mNMaxClusters, and copying the computed clusters out after every fragment
  mBufSize = nextMultipleOf<std::max<int>(GPUCA_MEMALIGN, mScanWorkGroupSize)>(mNMaxDigitsFragment);
  mNBufs = getNSteps(mBufSize);
}

void GPUTPCClusterFinder::SetNMaxDigits(size_t nDigits, size_t nPages, size_t nDigitsFragment)
{
  mNMaxDigits = nextMultipleOf<std::max<int>(GPUCA_MEMALIGN, mScanWorkGroupSize)>(nDigits);
  mNMaxPages = nPages;
  mNMaxDigitsFragment = nDigitsFragment;
}

unsigned int GPUTPCClusterFinder::getNSteps(size_t items) const
{
  if (items == 0) {
    return 0;
  }
  unsigned int c = 1;
  size_t capacity = mScanWorkGroupSize;
  while (items > capacity) {
    capacity *= mScanWorkGroupSize;
    c++;
  }
  return c;
}

void GPUTPCClusterFinder::PrepareMC()
{
  assert(mNMaxClusterPerRow > 0);

  clearMCMemory();
  mPindexMap = new uint[TPCMapMemoryLayout<decltype(*mPindexMap)>::items()];
  mPlabelsByRow = new GPUTPCClusterMCInterim[GPUCA_ROW_COUNT * mNMaxClusterPerRow];
  mPlabelsInRow = new uint[GPUCA_ROW_COUNT];
}

void GPUTPCClusterFinder::clearMCMemory()
{
  delete[] mPindexMap;
  mPindexMap = nullptr;
  delete[] mPlabelsByRow;
  mPlabelsByRow = nullptr;
  delete[] mPlabelsInRow;
  mPlabelsInRow = nullptr;
}
