# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#
#****************************************************************************

SET(AVTWRITER_SOURCES
avtImageFileWriter.C
avtDatasetFileWriter.C
avtFileWriter.C
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/visit_vtk/full
${VTK_INCLUDE_DIRS}
)

if(VTK_VERSION VERSION_EQUAL "8.1.0")
    set(vtklibs vtkImagingCore vtkIOPLY)
else()
    set(vtklibs VTK::ImagingCore VTK::IOPLY)
endif()

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

#********************************* SERIAL ************************************
ADD_LIBRARY(avtwriter_ser ${AVTWRITER_SOURCES})
TARGET_LINK_LIBRARIES(avtwriter_ser visitcommon avtplotter_ser avtpipeline_ser visit_vtk ${vtklibs})
VISIT_INSTALL_TARGETS(avtwriter_ser)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtwriter_par ${AVTWRITER_SOURCES})
    TARGET_LINK_LIBRARIES(avtwriter_par visitcommon avtplotter_par avtpipeline_par visit_vtk ${vtklibs})
    VISIT_INSTALL_TARGETS(avtwriter_par)
ENDIF(VISIT_PARALLEL)
