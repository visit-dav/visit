# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(VTKM VERSION 2.3.0)

set(visit_vtkm_dir ${visit_install_root}/vtkm/${VTKM_VERSION})

# has this already been built
if(EXISTS ${visit_vtkm_dir}/include/vtkm-${VTKM_VERSION_MAJOR}.${VTKM_VERSION_MINOR}/vtkm/Version.h AND
   EXISTS ${visit_vtkm_dir}/lib/vtkm_cont-${VTKM_VERSION_MAJOR}.${VTKM_VERSION_MINOR}.lib)
    if(BUILD_ALL OR ENABLE_VTKM)
        message(STATUS "VTKM already installed.")
    endif()
    set(PREBUILT_VTKM TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_VTKM))
    return()
endif()

message(STATUS "VTKM being added to the build.")

set(VTKM_ZIP_NAME "vtk-m-v${VTKM_VERSION}.tar.gz")

set(VTKM_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=OFF
    -DVTKm_ENABLE_TESTING:BOOL=OFF
    -DVTKm_ENABLE_TESTING_LIBRARY:BOOL=OFF
    -DVTKm_ENABLE_RENDERING:BOOL=ON
    -DVTKm_USE_64BIT_IDS:BOOL=OFF
    -DVTKm_USE_DOUBLE_PRECISION:BOOL=ON
    -DVTKm_USE_DEFAULT_TYPES_FOR_VTK:BOOL=ON
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_install_root}/vtkm/${VTKM_VERSION})

ExternalProject_Add(
    VTKM
    PREFIX            VTKM
    URL               ${visit_tp_url}/${VTKM_ZIP_NAME}
    URL_HASH          SHA256=63f1a0273227463a1be38c046630ea6f606ed958543fe6ff64eb6bb6513a6013
    CMAKE_CACHE_ARGS  ${VTKM_CMAKE_OPTIONS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND     ${CMAKE_COMMAND}
        -DVTKM_VERSION:STRING=${VTKM_VERSION}
        -DPATCH_DIR:PATH=${visit_tp_patches}
        -DPATCH_CMD:FILEPATH=${visit_patch_command}
        -P ${visit_tp_patches}/vtkm-patch.cmake)

