# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Brad Whitlock, Wed Apr 10 18:00:15 PDT 2013
#   Fix for static on Mac.
#
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#****************************************************************************

SET(CLIPEDITOR_SOURCES
    ClipEditor.C
    DataSet.C
    Font.C
    main.C
    Matrix.C
    Shape.C
    Transforms.C
    Vector.C
    Viewer.C
)

ADD_EXECUTABLE(genhexcases genhexcases.C)
ADD_EXECUTABLE(gentetcases gentetcases.C)
ADD_EXECUTABLE(genvoxcases genvoxcases.C)
ADD_EXECUTABLE(genpolycases genpolycases.C)

SET(RELEVANT_QT_LIBS 
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWIDGETS_LIBRARY}
    ${QT_QTOPENGLWIDGETS_LIBRARY}
)
IF(VISIT_STATIC)
    MAC_NIB_INSTALL(clipeditor)
    SET(RELEVANT_QT_LIBS
        ${RELEVANT_QT_LIBS}
        ${QT_QTCORE_LIB_DEPENDENCIES}
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
    )
ENDIF(VISIT_STATIC)

INCLUDE_DIRECTORIES(${VISIT_SOURCE_DIR}/visit_vtk/lightweight)

LINK_DIRECTORIES(${LIBRARY_OUTPUT_DIRECTORY})

ADD_EXECUTABLE(clipeditor ${VISIT_APPLICATION_STYLE} ${CLIPEDITOR_SOURCES})

set_target_properties(clipeditor PROPERTIES AUTOMOC ON)

TARGET_LINK_LIBRARIES(clipeditor  ${RELEVANT_QT_LIBS} ${OPENGL_LIBRARIES})

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(clipeditor)
endif()

