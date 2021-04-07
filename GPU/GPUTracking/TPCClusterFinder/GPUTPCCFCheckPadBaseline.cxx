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

#include "GPUTPCCFCheckPadBaseline.h"
#include "Array2D.h"
#include "PackedCharge.h"
#include "clusterFinderDefs.h"

using namespace GPUCA_NAMESPACE::gpu;
using namespace GPUCA_NAMESPACE::gpu::tpccf;

template <>
GPUd() void GPUTPCCFCheckPadBaseline::Thread<0>(int nBlocks, int nThreads, int iBlock, int iThread, GPUSharedMemory& smem, processorType& clusterer)
{
  static_assert(TPC_MAX_FRAGMENT_LEN % NumOfCachedTimebins == 0);

  Array2D<PackedCharge> chargeMap(reinterpret_cast<PackedCharge*>(clusterer.mPchargeMap));

  int totalCharges = 0;
  int consecCharges = 0;
  int maxConsecCharges = 0;

  int localPadId = iThread / NumOfCachedTimebins;
  int localTimeBin = iThread % NumOfCachedTimebins;
  bool handlePad = localTimeBin == 0;
  int basePad = iBlock * PadsPerBlock;

  CfFragment& fragment = clusterer.mPmemory->fragment;

  ChargePos basePos = padToChargePos(basePad + localPadId, clusterer);

  for (tpccf::TPCFragmentTime t = localTimeBin + fragment.firstNonOverlapTimeBin(); t < fragment.lastNonOverlapTimeBin(); t += NumOfCachedTimebins) {
    ChargePos pos = basePos.delta({0, t});
    smem.charges[localPadId][localTimeBin] = (pos.valid()) ? chargeMap[pos].unpack() : 0;
    GPUbarrierWarp();
    if (handlePad) {
      for (int i = 0; i < NumOfCachedTimebins; i++) {
        Charge q = smem.charges[localPadId][i];
        totalCharges += (q > 0);
        consecCharges = (q > 0) ? consecCharges + 1 : 0;
        maxConsecCharges = CAMath::Max(consecCharges, maxConsecCharges);
      }
    }
  }

  GPUbarrierWarp();

  if (handlePad) {
    int totalChargesBaseline = clusterer.Param().rec.maxTimeBinAboveThresholdIn1000Bin * fragment.lengthWithoutOverlap() / 1000;
    int consecChargesBaseline = clusterer.Param().rec.maxConsecTimeBinAboveThreshold;
    bool hasLostBaseline = (totalChargesBaseline > 0 && totalCharges >= totalChargesBaseline) || (consecChargesBaseline > 0 && maxConsecCharges >= consecChargesBaseline);
    clusterer.mPpadHasLostBaseline[basePad + localPadId] |= hasLostBaseline;
  }
}

GPUd() ChargePos GPUTPCCFCheckPadBaseline::padToChargePos(int pad, const GPUTPCClusterFinder& clusterer)
{
  const GPUTPCGeometry& geo = clusterer.Param().tpcGeometry;

  int padOffset = 0;
  for (Row r = 0; r < TPC_NUM_OF_ROWS; r++) {
    int padInRow = pad - padOffset;
    if (0 <= padInRow && padInRow < geo.NPads(r)) {
      return ChargePos{r, Pad(padInRow), 0};
    }
    padOffset += geo.NPads(r);
  }

  return ChargePos{0, 0, INVALID_TIME_BIN};
}
