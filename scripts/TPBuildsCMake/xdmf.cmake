# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(XDMF VERSION 2.1.1)

set(visit_xdmf_root ${visit_install_root}/xdmf/${XDMF_VERSION})
set(visit_xdmf_include_dir ${visit_xdmf_root}/include)
set(visit_xdmf_library_dir ${visit_xdmf_root}/lib)

# has this already been built
if(EXISTS ${visit_xdmf_include_dir}/Xdmf.h AND
   EXISTS ${visit_xdmf_library_dir}/Xdmf.lib)
    if(BUILD_ALL OR ENABLE_XDMF)
        message(STATUS "XDMF already installed.")
    endif()
    set(XDMF_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_XDMF))
    return()
endif()

message(STATUS "XDMF being added to the build")

set(XDMF_DEPENDS)

if(NOT HDF5_PREBUILT)
    list(APPEND XDMF_DEPENDS HDF5)
endif()

if(NOT VTK_PREBUILT)
    list(APPEND XDMF_DEPENDS VTK)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND XDMF_DEPENDS ZLIB)
endif()

set(XDMF_ZIP_NAME "Xdmf-${XDMF_VERSION}.tar.gz")

set(XDMF_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_xdmf_root}
    -DXDMF_BUILD_EXAMPLES:BOOL=OFF
    -DXDMF_BUILD_MPI:BOOL=OFF
    -DXDMF_BUILD_UTILS:BOOL=OFF
    -DXDMF_BUILD_VTK:BOOL=OFF
    -DXDMF_WRAP_PYTHON:BOOL=OFF

    -DXDMF_SYSTEM_HDF5:BOOL=ON
    -DHDF5_ENABLE_HSIZET:BOOL=ON
    -DHDF5_INCLUDE_PATH:PATH=${visit_hdf5_include}
    -DHDF5_LIBRARY:FILEPATH=${visit_hdf5_library}
    -DXDMF_SYSTEM_HDF5_IS_PARALLEL:BOOL=OFF

    -DXDMF_SYSTEM_LIBXML2:BOOL=ON
    -DLIBXML2_INCLUDE_PATH:PATH=${visit_vtk_include_dir}/vtklibxml2/include
    -DLIBXML2_LIBRARY:FILEPATH=${visit_vtk_library_dir}/vtklibxml2-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}.lib

    -DXDMF_SYSTEM_ZLIB:BOOL=ON
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY:FILEPATH=${visit_zlib_library})

ExternalProject_Add(XDMF
    PREFIX           XDMF
    URL              ${visit_tp_url}/${XDMF_ZIP_NAME}
    URL_HASH         SHA256=4f0c2011d1d6f86052b102b25b36276168a31e191b4206a8d0c9d716ebced7e1
    INSTALL_DIR      ${visit_install_root}/xdmf/${XDMF_VERSION}
    CMAKE_CACHE_ARGS ${XDMF_CMAKE_OPTIONS}
    DEPENDS          ${XDMF_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DXDMF_VERSION:STRING=${XDMF_VERSION}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -P ${visit_tp_patches}/xdmf-patch.cmake)

