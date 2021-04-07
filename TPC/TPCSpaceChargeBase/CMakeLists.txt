# Copyright CERN and copyright holders of ALICE O2. This software is distributed
# under the terms of the GNU General Public License v3 (GPL Version 3), copied
# verbatim in the file "COPYING".
#
# See http://alice-o2.web.cern.ch/license for full licensing information.
#
# In applying this license CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization or
# submit itself to any jurisdiction.

set(MODULE TPCSpaceChargeBase)

set(SRCS
    AliTPC3DCylindricalInterpolator.cxx
    AliTPC3DCylindricalInterpolatorIrregular.cxx
    AliTPCLookUpTable3DInterpolatorD.cxx
    AliTPCLookUpTable3DInterpolatorIrregularD.cxx
    AliTPCPoissonSolver.cxx
    AliTPCSpaceCharge3DCalc.cxx)
string(REPLACE ".cxx" ".h" HDRS_CINT "${SRCS}")

if(${ALIGPU_BUILD_TYPE} STREQUAL "ALIROOT")
  add_definitions(-DGPUCA_ALIROOT_LIB)
  include_directories(SYSTEM ${ROOT_INCLUDE_DIR})
  include_directories(${AliRoot_SOURCE_DIR}/TPC/TPCSpaceChargeBase)

  # Generate the dictionary
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  generate_dictionary("Ali${MODULE}" "TPCSpaceChargeBaseLinkDef.h"
                      "${HDRS_CINT}" "${incdirs}")

  set(ROOT_DEPENDENCIES Core Hist MathCore Matrix Physics)

  # Generate the ROOT map Dependecies
  set(LIBDEPS ${ROOT_DEPENDENCIES})
  generate_rootmap("Ali${MODULE}" "${LIBDEPS}"
                   "${CMAKE_CURRENT_SOURCE_DIR}/TPCSpaceChargeBaseLinkDef.h")

  # Add a library to the project using the specified source files
  add_library_tested(Ali${MODULE} SHARED ${SRCS} G__Ali${MODULE}.cxx)
  target_link_libraries(Ali${MODULE} ${LIBDEPS})

  # Additional compilation flags
  set_target_properties(Ali${MODULE} PROPERTIES COMPILE_FLAGS "")

  # System dependent: Modify the way the library is build
  if(${CMAKE_SYSTEM} MATCHES Darwin)
    set_target_properties(Ali${MODULE}
                          PROPERTIES LINK_FLAGS "-undefined dynamic_lookup")
  endif(${CMAKE_SYSTEM} MATCHES Darwin)

  # Installation
  install(TARGETS Ali${MODULE} ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

  install(FILES ${HDRS_CINT} DESTINATION include)
endif()
