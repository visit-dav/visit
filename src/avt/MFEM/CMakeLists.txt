# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.


visit_add_library(
    NAME       avtmfem
    SOURCES    avtMFEMDataAdaptor.C
    HEADERS    avtMFEMDataAdaptor.h
               avtMFEMLogging.h
               avtmfem_exports.h
    INCLUDES   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/avt/MFEM>
    DEPENDS_ON visitcommon
               VTK::CommonCore
               VTK::FiltersCore
               mfem)

