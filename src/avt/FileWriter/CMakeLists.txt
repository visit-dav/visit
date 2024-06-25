# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

set(avtwriter_SOURCES
    avtDatasetFileWriter.C
    avtFileWriter.C
    avtImageFileWriter.C)

set(avtwriter_HEADERS
    avtDatasetFileWriter.h
    avtFileWriter.h
    avtImageFileWriter.h
    file_writer_exports.h)

set(avtwriter_INCLUDES
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERISON_INCLUDE}/visit/avt/FileWriter>)

set(avtwriter_DEPENDS
    visitcommon
    visit_vtk
    VTK::ImagingCore
    VTK::IOPLY)

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtwriter_ser
    SOURCES    ${avtwriter_SOURCES}
    HEADERS    ${avtwriter_HEADERS}
    INCLUDES   ${avtwriter_INCLUDES}
    DEPENDS_ON ${avtwriter_DEPENDS}
               avtplotter_ser
               avtpipeline_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtwriter_par
        SOURCES    ${avtwriter_SOURCES}
        HEADERS    ${avtwriter_HEADERS}
        INCLUDES   ${avtwriter_INCLUDES}
        DEPENDS_ON ${avtwriter_DEPENDS}
                   avtplotter_par
                   avtpipeline_par)
endif()

