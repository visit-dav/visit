# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

project(VTK VERSION 9.5.0)

set(visit_vtk_root ${visit_install_root}/vtk/${VTK_VERSION})
set(visit_vtk_include_dir ${visit_vtk_root}/include/vtk-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR})
set(visit_vtk_library_dir ${visit_vtk_root}/lib)

# has this already been built
if(EXISTS ${visit_vtk_include_dir}/vtkActor.h AND
   EXISTS ${visit_vtk_library_dir}/vtkCommonCore-${VTK_VERSION_MAJOR}.${VTK_VERSION_MINOR}.lib)
    if(BUILD_ALL OR ENABLE_VTK)
        message(STATUS "VTK already installed.")
    endif()
    set(VTK_PREBUILT TRUE)
    return()
endif()

# if not requested, return
if(NOT (BUILD_ALL OR ENABLE_VTK))
    return()
endif()

message(STATUS "VTK being added to the build")

set(VTK_CMAKE_OPTIONS
    -DBUILD_SHARED_LIBS:BOOL=${visit_build_shared}
    -DCMAKE_BUILD_TYPE:STRING=Release
    -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING=MultiThreadedDLL
    -DBUILD_TESTING:STRING=OFF
    -DBUILD_DOCUMENTATION:BOOL=false
    -DVTK_FORBID_DOWNLOADS:BOOL=true
    -DVTK_ENABLE_REMOTE_MODULES:BOOL=OFF
    -DVTK_ALL_NEW_OBJECT_FACTORY:BOOL=true
    -DVTK_GROUP_ENABLE_Imaging:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_MPI:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Qt:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_StandAlone:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_STANDALONE:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Views:STRING=DONT_WANT
    -DVTK_GROUP_ENABLE_Web:STRING=DONT_WANT
    -DVTK_REPORT_OPENGL_ERRORS:BOOL=false
    -DVTK_MODULE_ENABLE_VTK_CommonCore:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_FiltersFlowPaths:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_FiltersGeometry:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_FiltersHybrid:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_FiltersModeling:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_GeovisCore:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOEnSight:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOGeometry:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOLegacy:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOPLY:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_InteractionStyle:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingFreeType:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_libxml2:STRING=YES
    -DCMAKE_INSTALL_PREFIX:PATH=${visit_vtk_root}
    -DVTK_WRAP_PYTHON:BOOL=true
    -DPython3_EXECUTABLE:FILEPATH=${visit_python_executable}
    -DPython3_INCLUDE_DIR:PATH=${visit_python_include}
    -DPython3_LIBRARY:FILEPATH=${visit_python_library}
    -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=YES
    -Dospray_DIR:PATH=${visit_ospray_config_dir}
    -DVTK_MODULE_ENABLE_VTK_RenderingAnari:STRING=YES
    -DVTK_MODULE_ENABLE_VTK_FiltersTexture:STRING=YES
    -Danari_DIR:PATH=${visit_anari_config_dir}
    -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:STRING=YES
    -DZLIB_BUILD_EXAMPLES:BOOL=OFF
    -DZLIB_INCLUDE_DIR:PATH=${visit_zlib_include}
    -DZLIB_LIBRARY:PATH=${visit_zlib_library}
    -DVTK_MODULE_USE_EXTERNAL_VTK_tiff:STRING=YES
    -DTIFF_INCLUDE_DIR:PATH=${visit_tiff_include}
    -DTIFF_LIBRARY:PATH=${visit_tiff_library}
    -DVTK_MODULE_USE_EXTERNAL_VTK_jpeg:STRING=YES
    -DJPEG_INCLUDE_DIR:PATH=${visit_jpeg_include}
    -DJPEG_LIBRARY:PATH=${visit_jpeg_library}
    -DVTK_MODULE_ENABLE_VTK_GUISupportQt:STRING=YES
    -DVTK_QT_VERSION:STRING=6
    -DQt6_DIR:PATH=${visit_qt_config_dir})

set(VTK_DEPENDS)

if(NOT PYTHON_PREBUILT)
    list(APPEND VTK_DEPENDS PYTHON)
endif()

if(NOT OSPRAY_PREBUILT)
    list(APPEND VTK_DEPENDS OSPRAY)
endif()

if(NOT ANARI_PREBUILT)
    list(APPEND VTK_DEPENDS ANARI)
endif()

if(NOT ZLIB_PREBUILT)
    list(APPEND VTK_DEPENDS ZLIB)
endif()

if(NOT TIFF_PREBUILT)
    list(APPEND VTK_DEPENDS TIFF)
endif()

if(NOT JPEG_PREBUILT)
    list(APPEND VTK_DEPENDS JPEG)
endif()

if(NOT QT_PREBUILT)
    list(APPEND VTK_DEPENDS QT)
endif()

set(VTK_ZIP_NAME "VTK-${VTK_VERSION}.tar.gz")

ExternalProject_Add(VTK
    PREFIX           VTK
    URL              ${visit_tp_url}/${VTK_ZIP_NAME}
    URL_HASH         SHA256=04ae86246b9557c6b61afbc534a6df099244fbc8f3937f82e6bc0570953af87d
    INSTALL_DIR      ${visit_vtk_root}
    CMAKE_CACHE_ARGS ${VTK_CMAKE_OPTIONS}
    DEPENDS          ${VTK_DEPENDS}
    ${LOGGING_OPTIONS}
    PATCH_COMMAND    ${CMAKE_COMMAND} -DVTK_VERSION:STRING=${VTK_VERSION}
                                      -DPATCH_CMD:PATH=${visit_patch_command}
                                      -DPATCH_DIR:PATH=${visit_tp_patches}
                                      -P ${visit_tp_patches}/vtk-patch.cmake)

