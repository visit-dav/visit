# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

project(VISIT_BUILD_THIRDPARTY)
SET(CMAKE_SUPPRESS_REGENERATION TRUE)
SET(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)
set(CMAKE_CONFIGURATION_TYPES Release Debug)

if(COMMAND cmake_policy)
    if(WIN32)
        # Tell CMake to always use /MD, even for debug builds.
        # This is done so that Release versions of libraries can
        # be in the same build as Debug versions of other libraries.
        # Don't always want to debug every TP lib!
        cmake_policy(SET CMP0091 NEW) # MSVC_RUNTIME_LIBRARY
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
endif()

if(NOT VISIT_PATCH_COMMAND)
    set(visit_patch_command "C:/Program Files/Git/usr/bin/patch.exe")
else()
    set(visit_patch_command "${VISIT_PATCH_COMMAND}")
endif()


if(NOT visit_install_root)
    cmake_path(SET visit_install_root NORMALIZE ${CMAKE_CURRENT_BINARY_DIR}/../INSTALL)
endif()

set(CMAKE_INSTALL_PREFIX ${visit_install_root})

set(VISIT_VERSION "3.6.0" CACHE STRING
    "VisIt version whose third-party release provides the tarballs")
string(REGEX REPLACE "^v" "" visit_version "${VISIT_VERSION}")
if(visit_version STREQUAL "")
    message(FATAL_ERROR "VISIT_VERSION must specify a VisIt release version")
endif()



# Library options
set(VISIT_REQUIRED_LIBRARIES
    JPEG
    OPENSSL
    PNG
    PYTHON
    QT
    TIFF
    VTK
    ZLIB)

set(VISIT_OPTIONAL_LIBRARIES
    ADIOS2
    ANARI
    BLOSC2
    CFITSIO
    CGNS
    CONDUIT
    GDAL
    H5PART
    HDF5
    ICET
    MFEM
    MILI
    NETCDF
    OPENEXR
    OSPRAY
    QWT
    SILO
    VTKM
    XDMF)

option(BUILD_OPTIONAL
       "Build all optional third-party libraries by default"
       OFF)
option(BUILD_PARALLEL "Enable/Disable parallel options in libraries that support them." ON )

foreach(library IN LISTS VISIT_REQUIRED_LIBRARIES)
    option(ENABLE_${library} "Enable building required library ${library}" ON)
endforeach()

# Optional library selections are tri-state. AUTO follows BUILD_OPTIONAL on
# every configure; ON and OFF are explicit per-library overrides.
foreach(library IN LISTS VISIT_OPTIONAL_LIBRARIES)
    set(option_name ENABLE_${library})
    set(${option_name} AUTO CACHE STRING
        "Build optional library ${library}: AUTO, ON, or OFF")
    set_property(CACHE ${option_name} PROPERTY TYPE STRING)
    set_property(CACHE ${option_name} PROPERTY STRINGS AUTO ON OFF)

    string(TOUPPER "${${option_name}}" option_value)
    if(option_value STREQUAL "AUTO")
        set(${option_name} ${BUILD_OPTIONAL})
    elseif(option_value STREQUAL "ON" OR option_value STREQUAL "OFF")
        set(${option_name} ${option_value})
    else()
        message(FATAL_ERROR
                "${option_name} must be AUTO, ON, or OFF; got '${${option_name}}'")
    endif()
endforeach()
unset(option_name)
unset(option_value)

if(BUILD_PARALLEL)
    find_package(MPI)
    if(NOT MPI_FOUND)
        message(STATUS "MPI not found, disabling parallel")
        set(BUILD_PARALLEL OFF)
    endif()
endif()

# set up some necessary  variables, and include the .cmake file for each library.
include(ExternalProject)

set(visit_tp_url
    "https://github.com/visit-dav/third-party/releases/download/v${visit_version}")
set(visit_tp_patches ${VISIT_BUILD_THIRDPARTY_SOURCE_DIR}/patches)

set(visit_build_shared ON)

set(LOGGING_OPTIONS)
if(VISIT_LOGGING)
    set(LOGGING_OPTIONS
            LOG_DOWNLOAD ON
            LOG_PATCH ON
            LOG_CONFIGURE ON
            LOG_BUILD ON
            LOG_INSTALL ON
            LOG_MERGED_STDOUTERR ON)
endif()

include(zlib.cmake)
include(jpeg.cmake)
include(png.cmake)
include(tiff.cmake)
include(python.cmake)
include(ospray.cmake)
include(anari.cmake)
include(openssl.cmake)
include(qt.cmake)
include(vtk.cmake)
include(hdf5.cmake)
include(silo.cmake)
include(conduit.cmake)
include(blosc2.cmake)
include(adios2.cmake)
include(cgns.cmake)
include(cfitsio.cmake)
include(gdal.cmake)
include(h5part.cmake)
include(icet.cmake)
include(mili.cmake)
include(mfem.cmake)
include(netcdf.cmake)
include(openexr.cmake)
include(qwt.cmake)
include(vtkm.cmake)
include(xdmf.cmake)
