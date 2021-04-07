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

/// \file GPUTPCGMMergedTrack.h
/// \author Sergey Gorbunov, David Rohr

#ifndef GPUTPCGMMERGEDTRACK_H
#define GPUTPCGMMERGEDTRACK_H

#include "GPUTPCGMTrackParam.h"
#include "GPUTPCGMMergedTrackHit.h"
#include "GPUdEdxInfo.h"

namespace GPUCA_NAMESPACE
{
namespace gpu
{
/**
 * @class GPUTPCGMMergedTrack
 *
 * The class is used to store merged tracks in GPUTPCGMMerger
 */
class GPUTPCGMMergedTrack
{
 public:
  GPUd() unsigned int NClusters() const { return mNClusters; }
  GPUd() unsigned int NClustersFitted() const { return mNClustersFitted; }
  GPUd() unsigned int FirstClusterRef() const { return mFirstClusterRef; }
  GPUd() const GPUTPCGMTrackParam& GetParam() const { return mParam; }
  GPUd() float GetAlpha() const { return mAlpha; }
  GPUd() GPUTPCGMTrackParam& Param()
  {
    return mParam;
  }
  GPUd() float& Alpha()
  {
    return mAlpha;
  }
  GPUd() float LastX() const { return mLastX; }
  GPUd() float LastY() const { return mLastY; }
  GPUd() float LastZ() const { return mLastZ; }
  GPUd() bool OK() const { return mFlags & 0x01; }
  GPUd() bool Looper() const { return mFlags & 0x02; }
  GPUd() bool CSide() const { return mFlags & 0x04; }
  GPUd() bool CCE() const { return mFlags & 0x08; }
  GPUd() bool MergedLooper() const { return mFlags & 0x10; }

  GPUd() void SetNClusters(int v) { mNClusters = v; }
  GPUd() void SetNClustersFitted(int v) { mNClustersFitted = v; }
  GPUd() void SetFirstClusterRef(int v) { mFirstClusterRef = v; }
  GPUd() void SetParam(const GPUTPCGMTrackParam& v) { mParam = v; }
  GPUd() void SetAlpha(float v) { mAlpha = v; }
  GPUd() void SetLastX(float v) { mLastX = v; }
  GPUd() void SetLastY(float v) { mLastY = v; }
  GPUd() void SetLastZ(float v) { mLastZ = v; }
  GPUd() void SetOK(bool v)
  {
    if (v) {
      mFlags |= 0x01;
    } else {
      mFlags &= 0xFE;
    }
  }
  GPUd() void SetLooper(bool v)
  {
    if (v) {
      mFlags |= 0x02;
    } else {
      mFlags &= 0xFD;
    }
  }
  GPUd() void SetCSide(bool v)
  {
    if (v) {
      mFlags |= 0x04;
    } else {
      mFlags &= 0xFB;
    }
  }
  GPUd() void SetCCE(bool v)
  {
    if (v) {
      mFlags |= 0x08;
    } else {
      mFlags &= 0xF7;
    }
  }
  GPUd() void SetMergedLooper(bool v)
  {
    if (v) {
      mFlags |= 0x10;
    } else {
      mFlags &= 0xEF;
    }
  }
  GPUd() void SetFlags(unsigned char v) { mFlags = v; }
  GPUd() void SetLegs(unsigned char v) { mLegs = v; }
  GPUd() unsigned char Legs() const { return mLegs; }

  GPUd() const GPUTPCGMTrackParam::GPUTPCOuterParam& OuterParam() const { return mOuterParam; }
  GPUd() GPUTPCGMTrackParam::GPUTPCOuterParam& OuterParam()
  {
    return mOuterParam;
  }
  GPUd() const GPUdEdxInfo& dEdxInfo() const { return mdEdxInfo; }
  GPUd() GPUdEdxInfo& dEdxInfo()
  {
    return mdEdxInfo;
  }

 private:
  GPUTPCGMTrackParam mParam;                        //* fitted track parameters
  GPUTPCGMTrackParam::GPUTPCOuterParam mOuterParam; //* outer param
  GPUdEdxInfo mdEdxInfo;                            //* dEdx information

  float mAlpha;                  //* alpha angle
  float mLastX;                  //* outer X
  float mLastY;                  //* outer Y
  float mLastZ;                  //* outer Z
  unsigned int mFirstClusterRef; //* index of the first track cluster in corresponding cluster arrays
  unsigned int mNClusters;       //* number of track clusters
  unsigned int mNClustersFitted; //* number of clusters used in fit
  unsigned char mFlags;
  unsigned char mLegs;
};
} // namespace gpu
} // namespace GPUCA_NAMESPACE

#endif
