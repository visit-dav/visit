# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#
#****************************************************************************


#-----------------------------------------------------------------------------
# Create common/misc/ConfigureInfo.C
#-----------------------------------------------------------------------------

# Store some configure information into a C file (like version, GIT revision
# and platform things that change).
configure_file(ConfigureInfo.C.in
               ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C @ONLY IMMEDIATE)

target_sources(visitcommon PRIVATE
    DebugStreamFull.C
    Environment.C
    FileFunctions.C
    InstallationFunctions.C
    PTY.C
    RuntimeSetting.C
    StackTimer.C
    TimingsManager.C
    VisItException.C
    VisItInit.C
    ${VISIT_BINARY_DIR}/common/misc/ConfigureInfo.C)

target_include_directories(visitcommon PUBLIC .)

if(WIN32)
    # for GetProfilesDirectory used in FileFunctions
    target_link_libraries(visitcommon PRIVATE userenv)
    # for std::filesystem::path used in FileFunctions
    target_compile_features(visitcommon PRIVATE cxx_std_17)
endif()

