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

/// \file GPUTrackingLinkDef_O2.h
/// \author David Rohr

#ifdef __CLING__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;

#pragma link C++ class o2::gpu::GPUTPCO2Interface + ;
#pragma link C++ class o2::gpu::GPUTPCO2InterfaceRefit + ;
#pragma link C++ class o2::gpu::GPUO2InterfaceQA + ;
#pragma link C++ class o2::gpu::TPCdEdxCalibrationSplines + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsO2 + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsRec + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsProcessing + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsDisplay + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsDisplayLight + ;
#pragma link C++ class o2::gpu::GPUConfigurableParamGPUSettingsQA + ;
#pragma link C++ class o2::gpu::trackInterface < o2::dataformats::TrackTPCITS> + ;
#pragma link C++ class o2::gpu::GPUTRDTrack_t < o2::gpu::trackInterface < o2::dataformats::TrackTPCITS>> + ;
#pragma link C++ class std::vector < o2::gpu::GPUTRDTrack_t < o2::gpu::trackInterface < o2::dataformats::TrackTPCITS>>> + ;

#endif
