# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#[[ NOTES:
   Ninja.exe must be in your PATH environment variable
#]]

project(QT VERSION 6.4.2)

set(visit_qt_root       ${visit_install_root}/qt/${QT_VERSION})
set(visit_qt_qmake      ${visit_install_root}/qt/${QT_VERSION}/bin/qmake.exe)
set(visit_qt_config_dir ${visit_qt_root}/lib/cmake/Qt${QT_VERSION_MAJOR})

# has this already been built
if(EXISTS ${visit_qt_root}/include/QtWidgets/qapplication.h AND
   EXISTS ${visit_qt_root}/lib/Qt6Widgets.lib)
    if(BUILD_ALL OR ENABLE_QT)
        message(STATUS "QT already installed.")
    endif()
    set(QT_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_QT))
    return()
endif()

message(STATUS "QT being added to the build.")

# tarball names for the qt submodules that will be built
set(QTBASE_ZIP_NAME  "qtbase-everywhere-src-${QT_VERSION}.tar.xz")
set(QTTOOLS_ZIP_NAME "qttools-everywhere-src-${QT_VERSION}.tar.xz")
set(QTSVG_ZIP_NAME   "qtsvg-everywhere-src-${QT_VERSION}.tar.xz")

# CMake options for building qtbase
set(QTBASE_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_qt_root}
    -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DFEATURE_androiddeployqt:BOOL=OFF
    -DFEATURE_dbus:BOOL=OFF
    -DFEATURE_debug:BOOL=OFF
    -DFEATURE_directwrite:BOOL=OFF
    -DFEATURE_directwrite3:BOOL=OFF
    -DFEATURE_LTCG:BOOL=ON
    -DFEATURE_OPENSSL:BOOL=ON
    -DFEATURE_sql:BOOL=OFF
    -DFEATURE_sql_db2:BOOL=OFF
    -DFEATURE_sql_ibase:BOOL=OFF
    -DFEATURE_sql_mysql:BOOL=OFF
    -DFEATURE_sql_oci:BOOL=OFF
    -DFEATURE_sql_odbc:BOOL=OFF
    -DFEATURE_sql_psql:BOOL=OFF
    -DFEATURE_sql_sqlite:BOOL=OFF
    -DFEATURE_sqlmodel:BOOL=OFF
    -DFEATURE_testlib:BOOL=OFF
    -DOPENSSL_ROOT_DIR:PATH=${visit_openssl_root}
    -DQT_BUILD_EXAMPLES_BY_DEFAULT:BOOL=OFF
    -DQT_BUILD_TESTS_BY_DEFAULT:BOOL=OFF)

set(QT_DEPENDS)
if(NOT OPENSSL_PREBUILT)
    list(APPEND QT_DEPENDS OPENSSL)
endif()

# Command to build qtbase
ExternalProject_Add(QT
    PREFIX QT
    URL              ${visit_tp_url}/${QTBASE_ZIP_NAME}
    URL_HASH         SHA256=a88bc6cedbb34878a49a622baa79cace78cfbad4f95fdbd3656ddb21c705525d
    CMAKE_GENERATOR  Ninja
    CMAKE_CACHE_ARGS ${QTBASE_CMAKE_OPTIONS}
    DEPENDS          ${QT_DEPENDS}
    ${LOGGING_OPTIONS})



# build qttools
ExternalProject_Add(QTTOOLS
    PREFIX            QT
    URL               ${visit_tp_url}/${QTTOOLS_ZIP_NAME}
    CONFIGURE_COMMAND ${visit_qt_root}/bin/qt-configure-module.bat ../QTTOOLS
    URL_HASH          SHA256=a31387916184e4a5ef522d3ea841e8e931cc0f88be0824a7a354a572d5826c68
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --parallel
    INSTALL_COMMAND   ${CMAKE_COMMAND} --install .
    DEPENDS           QT
    ${LOGGING_OPTIONS})

# build qtsvg
ExternalProject_Add(QTSVG
    PREFIX            QT
    URL               ${visit_tp_url}/${QTSVG_ZIP_NAME}
    CONFIGURE_COMMAND ${visit_qt_root}/bin/qt-configure-module.bat ../QTSVG
    URL_HASH          SHA256=b746af3cb1793621d8ed7eae38d9ad5a15541dc2742031069f2ae3fe87590314
    BUILD_COMMAND     ${CMAKE_COMMAND} --build . --parallel
    INSTALL_COMMAND   ${CMAKE_COMMAND} --install .
    DEPENDS           QT
    ${LOGGING_OPTIONS})

