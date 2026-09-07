# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
#
# Modifications:
#    Justin Privitera, Wed Mar 22 16:09:52 PDT 2023
#    Added visit_vtk to include dirs and target link libs.
#
#    Kathleen Biagas, Wed Sep 2, 2026
#    avtmfem and mfem are unconditional dependencies.
#
#****************************************************************************

visit_add_library(
    NAME       avtblueprint
    SOURCES    avtConduitBlueprintDataAdaptor.C
               avtConduitBlueprintInfoWarningHandler.C
    HEADERS    avtblueprint_exports.h
               avtConduitBlueprintDataAdaptor.h
               avtConduitBlueprintInfoWarningHandler.C
               avtConduitBlueprintLogging.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/Blueprint>
    DEPENDS_ON visitcommon
               lightweight_visit_vtk
               avtmfem
               VTK::CommonCore
               VTK::FiltersCore
               conduit
               mfem)

