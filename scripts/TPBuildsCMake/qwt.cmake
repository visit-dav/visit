# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(QWT VERSION 6.3.0)

set(visit_qwt_root        ${visit_install_root}/qwt/${QWT_VERSION})
set(visit_qwt_include     ${visit_qwt_root}/include)
set(visit_qwt_library_dir ${visit_qwt_root}/lib)
set(visit_qwt_library     ${visit_qwt_library_dir}/qwt.lib)

# has this already been built
if(EXISTS ${visit_qwt_include}/qwt.h AND
   EXISTS ${visit_qwt_library})
    if(BUILD_ALL OR ENABLE_QWT)
       message(STATUS "QWT already installed.")
    endif()
    set(QWT_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_QWT))
    return()
endif()

message(STATUS "QWT being added to the build")

set(QWT_ZIP_NAME "qwt-git-d3706f6e7f0351d278be2d989a4caaf92b399bbd.tar.xz")

set(QWT_DEPENDS)
if(NOT QT_PREBUILT)
    set(QWT_DEPENDS QT)
endif()

ExternalProject_Add(QWT
    PREFIX            QWT
    URL               ${visit_tp_url}/${QWT_ZIP_NAME}
    URL_HASH          SHA256=39839f3aa83f41d09109296d41659e04bb234d9e41ab551af9f4e9b4fceed251
    INSTALL_DIR       ${visit_qwt_root}
    BUILD_IN_SOURCE   TRUE
    CONFIGURE_COMMAND ${visit_qt_qmake} "QWT_INSTALL_PREFIX = ${visit_qwt_root}" -config release qwt.pro 
    BUILD_COMMAND     nmake
    INSTALL_COMMAND   nmake install
    DEPENDS           ${QWT_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND     ${CMAKE_COMMAND} -DQWT_VERSION:STRING=${QWT_VERSION}
                                       -DPATCH_DIR:PATH=${visit_tp_patches}
                                       -DPATCH_CMD:PATH=${visit_patch_command}
                                       -P ${visit_tp_patches}/qwt-patch.cmake)


