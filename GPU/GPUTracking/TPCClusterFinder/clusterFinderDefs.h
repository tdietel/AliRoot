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

/// \file clusterFinderDefs.h
/// \author David Rohr

#ifndef O2_GPU_CLUSTERFINDERDEFS_H
#define O2_GPU_CLUSTERFINDERDEFS_H

#include "GPUDef.h"

#ifndef __OPENCL__
using uchar = unsigned char;
#endif
#ifdef __APPLE__
using ulong = unsigned long;
#endif

#define SCRATCH_PAD_WORK_GROUP_SIZE GPUCA_GET_THREAD_COUNT(GPUCA_LB_CLUSTER_FINDER)

/* #define CHARGEMAP_TIME_MAJOR_LAYOUT */
#define CHARGEMAP_TILING_LAYOUT

#define SCRATCH_PAD_SEARCH_N 8
#define SCRATCH_PAD_COUNT_N 16
#if defined(GPUCA_GPUCODE)
#define SCRATCH_PAD_BUILD_N 8
#define SCRATCH_PAD_NOISE_N 8
#else
// Double shared memory on cpu as we can't reuse the memory from other threads
#define SCRATCH_PAD_BUILD_N 16
#define SCRATCH_PAD_NOISE_N 16
#endif

#define PADDING_PAD 2
#define PADDING_TIME 3
#define TPC_SECTORS 36
#define TPC_ROWS_PER_CRU 18
#define TPC_NUM_OF_ROWS 152
#define TPC_PADS_PER_ROW 138
#define TPC_PADS_PER_ROW_PADDED (TPC_PADS_PER_ROW + PADDING_PAD)
#define TPC_NUM_OF_PADS (TPC_NUM_OF_ROWS * TPC_PADS_PER_ROW_PADDED + PADDING_PAD)
#define TPC_PADS_IN_SECTOR 14560
#define TPC_MAX_FRAGMENT_LEN 4000
#define TPC_MAX_FRAGMENT_LEN_PADDED (TPC_MAX_FRAGMENT_LEN + 2 * PADDING_TIME)
#define TPC_MAX_TIME_BIN_TRIGGERED 600

#if 0
#define DBG_PRINT(msg, ...) printf(msg "\n", __VA_ARGS__)
#else
#define DBG_PRINT(msg, ...) static_cast<void>(0)
#endif

#ifdef GPUCA_GPUCODE
#define CPU_ONLY(x) static_cast<void>(0)
#define CPU_PTR(x) nullptr
#else
#define CPU_ONLY(x) x
#define CPU_PTR(x) x
#endif

namespace GPUCA_NAMESPACE::gpu::tpccf
{

using SizeT = size_t;
using TPCTime = int;
using TPCFragmentTime = short;
using Pad = unsigned char;
using GlobalPad = short;
using Row = unsigned char;
using Cru = unsigned char;

using Charge = float;

using Delta = short;
using Delta2 = short2;

using local_id = short2;

} // namespace GPUCA_NAMESPACE::gpu::tpccf

#endif
