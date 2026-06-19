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

include(CMakeDependentOption)

# Library options
option(BUILD_ALL "Build all supported Third Party libs. If off, individual libraries can be built via ENABLE_XXX where XXX is the capitalized name of the library" ON )

cmake_dependent_option(ENABLE_ADIOS2 "Enable building adios2" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_ANARI "Enable building anari" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_CFITSIO "Enable building cfitsio" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_CGNS "Enable building cgns" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_CONDUIT "Enable building conduit" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_HDF5 "Enable building hdf5" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_H5PART "Enable building H5PART" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_ICET "Enable building ICET" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_JPEG "Enable building jpeg" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_MFEM "Enable building mfem" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_MILI "Enable building mili" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_NETCDF "Enable building NETCDF" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_OPENEXR "Enable building OpenEXR" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_OSPRAY "Enable building ospray" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_PYTHON "Enable building python" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_OPENSSL "Enable building openssl" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_QT "Enable building qt" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_TIFF "Enable building tiff" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_PNG "Enable building png" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_VTK "Enable building vtk" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_VTKM "Enable building vtk-m" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_ZLIB "Enable building zlib" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_XDMF "Enable building Xdmf" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_GDAL "Enable building GDAL" OFF "NOT BUILD_ALL" ON)
cmake_dependent_option(ENABLE_QWT "Enable building QWT" OFF "NOT BUILD_ALL" ON)


# set up some necessary  variables, and include the .cmake file for each library.
include(ExternalProject)

set(visit_tp_url "https://github.com/visit-dav/third-party/releases/download/v3.5.0")
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

