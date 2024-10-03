# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Biagas, Thu Mar 30, 2023
#   Use AUTOMOC target property instead of QT_WRAP_CPP macro.
#
#   Kathleen Biagas, Tue May 5, 2023
#   Disable qt6 forcing of defines for UNICODE.
#
#   Kathleen Biagas, Thu Oct 3, 2024
#   Utilize visit_add_library.
#
#****************************************************************************

set(VIEWER_QT_LIBS
    ${QT_QTGUI_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTCORE_LIBRARY})

if(VISIT_STATIC)
    list(APPEND VIEWER_QT_LIBS
        ${QT_QTGUI_LIB_DEPENDENCIES}
        ${QT_QTOPENGL_LIB_DEPENDENCIES}
        ${QT_QTCORE_LIB_DEPENDENCIES})
endif()

visit_add_library(
    NAME        viewersubjectproxy
    SOURCES     ViewerSubjectProxy.C
                ViewerSubjectProxyFactory.C
    HEADERS     ViewerSubjectProxy.h
                ViewerSubjectProxyFactory.h
                viewersubjectproxy_exports.h
    INCLUDES    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                $<INSTALL_INTERFACE:${VISIT_INSTALLED_VERSION_INCLUDE}/visit/viewer/subjectproxy>
    DEPENDS_ON  viewer
                viewerrpc
                viewerproxy
                visitcommon
                mdserverrpc
                mdserverproxy
                vclrpc
                vclproxy
                enginerpc
                engineproxy
                avtdbatts
                avtview
                avtmath
                avtpipeline_ser
                avtplotter_ser
                avtfilters_ser
                avtviswindow_ser
                avtwriter_ser
                avtqtviswindow
                winutil
                ${VIEWER_QT_LIBS})

set_target_properties(viewersubjectproxy PROPERTIES AUTOMOC ON)

if(QT_VERSION VERSION_GREATER_EQUAL "6.2.0")
    qt6_disable_unicode_defines(viewersubjectproxy)
endif()

