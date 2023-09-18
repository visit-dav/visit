# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Hank Childs, Thu Jul 22 09:55:03 PDT 2010
#   Add attributes for multi-window saves.
#
#   Brad Whitlock, Fri Jul 23 11:14:02 PDT 2010
#   I added SelectionProperties and SelectionList.
#
#   Hank Childs, Thu Aug 19 12:26:29 PDT 2010
#   Move DDF to DataBinning.
#
#   Mark C. Miller, Fri Oct 29 10:02:16 PDT 2010
#   Added DBYieldedNoDataException
#
#   Brad Whitlock, Fri Dec 10 15:42:46 PST 2010
#   I added ThresholdOpAttributes, the bulk of the ThresholdAttributes. I also
#   added SelectionSummary and SelectionVariableSummary.
#
#   Mark C. Miller, Tue Feb  8 16:28:32 PST 2011
#   Added custom 'init' target for pareser re-gen. It was lost in transition
#   from autoconf to CMake.
#
#   Kathleen Biagas, Tue Aug 23 10:16:30 PDT 2011
#   Added QueryArgumentException.
#
#   Brad Whitlock, Mon Nov 28 16:20:55 PST 2011
#   I moved LaunchRPC to this directory from the launcher/rpc directory.
#
#   Cameron Christensen, Thursday, May 22, 2014
#   I added InvalidConversionException.
#
#   Kathleen Biagas, Mon Oct 31 15:23:31 PDT 2016
#   Add ZLIB_LIB to link libraries for visitcommon on Windows.
#
#   Kathleen Biagas, Tue Dec 20 15:43:20 PST 2016
#   Added GlyphTypes.
#
#   Cyrus Harrison, Tue Dec 10 08:51:21 PST 2019
#   Add xml tools code gen targets
#
#   Kathleen Biagas, Thu Jun  4 16:07:40 PDT 2020
#   Add LineGlyphAttributes and PointGlyphAttributes.
#
#   Kathleen Biagas, Thu Jun  4 16:07:40 PDT 2020
#   Enable mrucache, namescheme, utility unit tests on Windows.
#   Wrap unit tests in if-test for VISIT_ENABLE_UNIT_TESTS.
#   Add folder for unit tests.
#
#   Kathleen Biagas, Thu Nov 12, 2020
#   Enable StringHelpers_test on Windows.
#
#   Kathleen Biagas, Tue Jul 13 14:57:13 PDT 2021
#   Remove ExpressionList from code-gen target list. It cannot be auto-gen'd.
#
#   Eric Brugger, Thu Aug  5 11:21:21 PDT 2021
#   Removed support for SeedMe.
#
#   Kathleen Biagas, Wed Dec 7, 2022
#   Moved bulk of code to subdirectories.
#
#   Eric Brugger, Tue Jul 25 11:23:55 EDT 2023
#   Change CMAKE_THREAD_LIBS to Threads::Threads.
#
#****************************************************************************

add_subdirectory(comm)
add_subdirectory(Exceptions)
add_subdirectory(expr)
add_subdirectory(misc)
add_subdirectory(parser)
add_subdirectory(plugin)
add_subdirectory(proxybase)
add_subdirectory(state)
add_subdirectory(utility)

visit_append_list(
    NAME  visitcommon_INCLUDES
    ITEMS $<BUILD_INTERFACE:${VISIT_BINARY_DIR}/include>
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/icons>
          $<BUILD_INTERFACE:${ZLIB_INCLUDE_DIR}>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/zlib/include>
          $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/include>)

if(WIN32)
    visit_append_list(NAME visitcommon_DEPENDS ITEMS ws2_32 shlwapi)
else()
    visit_append_list(NAME visitcommon_DEPENDS ITEMS Threads::Threads ${DL_LIB})
endif()

visit_append_list(NAME visitcommon_DEPENDS ITEMS ${ZLIB_LIBRARY})

blt_add_library(
    NAME       visitcommon
    SOURCES    ${visitcommon_SOURCES}
    INCLUDES   ${visitcommon_INCLUDES}
    DEPENDS_ON ${visitcommon_DEPENDS})

if(visitcommon_FEATURES)
    target_compile_features(visitcommon PRIVATE ${visitcommon_FEATURES})
endif()

visit_install_export_targets(visitcommon)

unset(visitcommon_SOURCES CACHE)
unset(visitcommon_INCLUDES CACHE)
unset(visitcommon_DEPENDS CACHE)
unset(visitcommon_FEATURES CACHE)
