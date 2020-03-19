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
#****************************************************************************

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${VISIT_SOURCE_DIR}/common/utility
)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(xmltest         xmltest.C)
ADD_EXECUTABLE(xml2atts        xml2atts.C)
ADD_EXECUTABLE(xml2window      xml2window.C)
ADD_EXECUTABLE(xml2info        xml2info.C)
ADD_EXECUTABLE(xml2avt         xml2avt.C)
ADD_EXECUTABLE(xml2python      xml2python.C)
ADD_EXECUTABLE(xml2java        xml2java.C)
ADD_EXECUTABLE(xml2cmake       xml2cmake.C)
ADD_EXECUTABLE(xml2sim         xml2sim.C)

SET(RELEVANT_QT_LIBS 
    ${QT_QTXML_LIBRARY}
    ${QT_QTCORE_LIBRARY}
)

IF(VISIT_STATIC)
    SET(RELEVANT_QT_LIBS
        ${RELEVANT_QT_LIBS}
        ${QT_QTXML_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

# Specify the libraries to link against on Windows & Mac
TARGET_LINK_LIBRARIES(xmltest    ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2atts   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2window ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2info   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2avt    ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2python ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2java   ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2cmake  ${RELEVANT_QT_LIBS})
TARGET_LINK_LIBRARIES(xml2sim    ${RELEVANT_QT_LIBS})

target_compile_definitions(xml2cmake PRIVATE VTK_MAJ=${VTK_MAJOR_VERSION} VTK_MIN=${VTK_MINOR_VERSION})

# Install the targets
VISIT_INSTALL_TARGETS(xmltest xml2atts xml2window xml2info xml2avt xml2python xml2java xml2cmake)
VISIT_TOOLS_ADD_FOLDER(xml xmltest xml2atts xml2window xml2info xml2avt xml2python xml2java xml2cmake xml2sim)
