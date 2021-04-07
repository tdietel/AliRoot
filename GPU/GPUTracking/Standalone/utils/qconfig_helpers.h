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

/// \file qconfig_helpers.h
/// \author David Rohr

#ifndef QCONFIG_HELPERS_H
#define QCONFIG_HELPERS_H

#include <string>
#include <sstream>

namespace qConfig
{
template <class T>
std::string print_type(T val)
{
  std::ostringstream s;
  s << val;
  return s.str();
};
template <>
std::string print_type<char>(char val)
{
  return std::to_string(val);
};
template <>
std::string print_type<unsigned char>(unsigned char val)
{
  return std::to_string(val);
};
template <>
std::string print_type<bool>(bool val)
{
  return val ? "true" : "false";
};
} // namespace qConfig

#endif
