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

/// \file GPUO2InterfaceRefit.h
/// \author David Rohr

#ifndef GPUO2INTERFACEREFIT_H
#define GPUO2INTERFACEREFIT_H

// Some defines denoting that we are compiling for O2
#ifndef HAVE_O2HEADERS
#define HAVE_O2HEADERS
#endif
#ifndef GPUCA_TPC_GEOMETRY_O2
#define GPUCA_TPC_GEOMETRY_O2
#endif
#ifndef GPUCA_O2_INTERFACE
#define GPUCA_O2_INTERFACE
#endif

#include "GPUTrackingRefit.h"
#include <memory>
#include <vector>
#include <gsl/span>

namespace o2::tpc
{
using TPCClRefElem = uint32_t;
}

namespace o2::gpu
{
class GPUParam;
class GPUTPCO2InterfaceRefit
{
 public:
  // Must initialize with:
  // - In any case: Cluster Native access structure (cl), TPC Fast Transformation instance (trans), solenoid field (bz), TPC Track hit references (trackRef)
  // - Either the shared cluster map (sharedmap) or the vector of tpc tracks (trks) to build the shared cluster map internally
  // - o2::base::Propagator (p) in case RefitTrackAsTrackParCov is to be used

  GPUTPCO2InterfaceRefit(const o2::tpc::ClusterNativeAccess* cl, const TPCFastTransform* trans, float bz, const o2::tpc::TPCClRefElem* trackRef, const unsigned char* sharedmap = nullptr, const std::vector<o2::tpc::TrackTPC>* trks = nullptr, o2::base::Propagator* p = nullptr);
  ~GPUTPCO2InterfaceRefit();

  int RefitTrackAsGPU(o2::tpc::TrackTPC& trk, bool outward = false, bool resetCov = false) { return mRefit.RefitTrackAsGPU(trk, outward, resetCov); }
  int RefitTrackAsTrackParCov(o2::tpc::TrackTPC& trk, bool outward = false, bool resetCov = false) { return mRefit.RefitTrackAsTrackParCov(trk, outward, resetCov); }
  int RefitTrackAsGPU(o2::track::TrackParCov& trk, const o2::tpc::TrackTPCClusRef& clusRef, float time0, float* chi2 = nullptr, bool outward = false, bool resetCov = false) { return mRefit.RefitTrackAsGPU(trk, clusRef, time0, chi2, outward, resetCov); }
  int RefitTrackAsTrackParCov(o2::track::TrackParCov& trk, const o2::tpc::TrackTPCClusRef& clusRef, float time0, float* chi2 = nullptr, bool outward = false, bool resetCov = false) { return mRefit.RefitTrackAsTrackParCov(trk, clusRef, time0, chi2, outward, resetCov); }
  void setGPUTrackFitInProjections(bool v = true);
  void setTrackReferenceX(float v);
  void setIgnoreErrorsAtTrackEnds(bool v) { mRefit.mIgnoreErrorsOnTrackEnds = v; }

  static void fillSharedClustersMap(const o2::tpc::ClusterNativeAccess* cl, const gsl::span<const o2::tpc::TrackTPC> trks, const o2::tpc::TPCClRefElem* trackRef, unsigned char* shmap);

 private:
  GPUTrackingRefit mRefit;
  std::unique_ptr<GPUParam> mParam;
  std::vector<unsigned char> mSharedMap;
};
} // namespace o2::gpu

#endif
