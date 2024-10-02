# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Wed Jan 05 10:25:13 PST 2010
#   Add FOLDER property.
#
#   Kathleen Bonnell, Mon Jan 10 07:59:28 MST 2010
#   Remove PROGRAM9 (xml2projectfile), and PROGRAM5 (xml2makefile).
#
#   Cyrus Harrison, Mon Dec  9 13:39:19 PST 2019
#   Rename main files to correspond to their actual use.
#
#   Kathleen Biagas, Wed Sep 18, 2024 
#   Add VTKM version to xml2cmake compile definitions.
#
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${VISIT_SOURCE_DIR}/common/utility
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

set(RELEVANT_QT_LIBS ${QT_QTXML_LIBRARY} ${QT_QTCORE_LIBRARY})

if(VISIT_STATIC)
    list(APPEND RELEVANT_QT_LIBS
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
endif()

foreach(tool xml2atts xml2avt xml2cmake xml2info xml2java xml2python xml2sim
             xmltest xml2window)
    add_executable(${tool} ${tool}.C)
    target_link_libraries(${tool}  ${RELEVANT_QT_LIBS})
    if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
        qt6_disable_unicode_defines(${tool})
    endif()
endforeach()


target_compile_definitions(xml2cmake PRIVATE VTK_MAJ=${VTK_MAJOR_VERSION} VTK_MIN=${VTK_MINOR_VERSION} VTKM_SMALL=${VTKm_VERSION_MAJOR}.${VTKm_VERSION_MINOR})

# Install the targets
VISIT_INSTALL_TARGETS(xmltest xml2atts xml2window xml2info xml2avt xml2python xml2java xml2cmake)
VISIT_TOOLS_ADD_FOLDER(xml xmltest xml2atts xml2window xml2info xml2avt xml2python xml2java xml2cmake xml2sim)
