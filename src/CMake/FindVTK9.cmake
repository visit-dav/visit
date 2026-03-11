# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Notes: Copied from FindVisItVTK.cmake and reworked for VTK-9.
#
# Modifications:
#  Kathleen Biagas, Jun 29, 2023
#  WrappingPythonCore needs special logic, has Python version as part of name.
#
#  Kathleen Biagas, Wed Nov  8 10:00:28 PST 2023
#  Test for lib vs lib64.
#  Use QT_MAJOR_VERSION instead of '5' or '6'.
#
#  Kathleen Biagas, Wed Feb 28, 2024
#  Removed install logic for python modules.  Now handled in
#  lib/CMakeLists.txt with build/lib/site-packages/ directory install.
#
#  Kathleen Biagas, Thu May 2, 2024
#  On Windows, ensure 'vtksys' and 'WrappingPythonCore' are handled correctly
#  when installing the .libs.
#
#  Kathleen Biagas, Thu Jun 19, 2025
#  Remove support for VTK < 9.5. Simplify some logic.
#
#  Kathleen Biagas, Mon Oct 20, 2025 
#  Change ospray check to 'ospray_FOUND' instead of 'OSPRAY_FOUND'.
#
#*****************************************************************************

# Use the VTK_DIR hint from the config-site .cmake file

set(vtk_lib_path lib)
if(EXISTS ${VISIT_VTK_DIR}/lib64)
    set(vtk_lib_path lib64)
endif()
set(VTK_DIR ${VISIT_VTK_DIR}/${vtk_lib_path}/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION})

message(STATUS "Checking for VTK in ${VTK_DIR}")

# Set up our list of required and optional vtk modules
set(REQ_VTK_MODS
    CommonCore
    CommonDataModel
    FiltersCore
    FiltersFlowPaths
    FiltersHybrid
    FiltersModeling
    IOLegacy
    IOPLY
    IOXML
    InteractionStyle
    RenderingAnnotation
    RenderingOpenGL2
    RenderingVolumeOpenGL2
    glad)


if(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    list(APPEND REQ_VTK_MODS GUISupportQt)
    set(Qt${QT_VERSION_MAJOR}_DIR ${VISIT_QT_DIR}/lib/cmake/Qt${QT_VERSION_MAJOR})
endif()

if(ospray_FOUND)
    list(APPEND REQ_VTK_MODS RenderingRayTracing)
endif()

# Optional
set(OPT_VTK_MODS
    GeovisCore # Cartographic Projection Operator
    IOEnSight  # EnSight Database
    libxml2    # Xdmf Database
    )


find_package(VTK ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}
    REQUIRED ${REQ_VTK_MODS}
    OPTIONAL_COMPONENTS ${OPT_VTK_MODS}
    NO_MODULE
    PATHS ${VTK_DIR})

# Ensure we have all the required modules:
foreach(module ${REQ_VTK_MODS})
    if(NOT TARGET VTK::${module})
        message(ERROR " VisIt requires VTK::${module}")
    endif()
endforeach()


message(STATUS "  VTK_FOUND=${VTK_FOUND}")
message(STATUS "  VTK_MAJOR_VERSION=${VTK_MAJOR_VERSION}")
message(STATUS "  VTK_MINOR_VERSION=${VTK_MINOR_VERSION}")
message(STATUS "  VTK_BUILD_VERSION=${VTK_BUILD_VERSION}")
message(STATUS "  VTK_PREFIX_PATH=${VTK_PREFIX_PATH}")
message(STATUS "  VTK_PYTHON_VERSION=${VTK_PYTHON_VERSION}")
message(STATUS "  VTK_PYTHONPATH=${VTK_PYTHONPATH}")
message(STATUS "  VTK_LIBRARIES=${VTK_LIBRARIES}")
message(STATUS "  VTK_AVAILABLE_COMPONENTS=${VTK_AVAILABLE_COMPONENTS}")


# Add install commands for all of the VTK libraries. Is there a better way?
if(APPLE)
    set(SO_EXT "dylib")
elseif(WIN32)
    # use lib instead of dll so that both will be installed
    # correctly with single call to THIRD_PARTY_INSTALL_LIBRARY
    set(SO_EXT "lib")
else()
    set(SO_EXT "so")
endif()

if(VISIT_VTK_SKIP_INSTALL)
    message(STATUS "Skipping installation of VTK libraries")
else(VISIT_VTK_SKIP_INSTALL)
    if(NOT WIN32)
        set(pathnameandprefix "${VTK_PREFIX_PATH}/${vtk_lib_path}/lib")
    else()
        set(pathnameandprefix "${VTK_PREFIX_PATH}/lib/")
    endif()

    macro(SETUP_INSTALL vtk_component)
        set(sepchar "-")
        if(${vtk_component} MATCHES "vtksys")
            set(LIBNAME   ${pathnameandprefix}${vtk_component}${sepchar}${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        elseif(${vtk_component} MATCHES "WrappingPythonCore")
            # also needs PYTHON_VERSION
            set(LIBNAME   ${pathnameandprefix}vtk${vtk_component}${PYTHON_VERSION}${sepchar}${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        else()
            set(LIBNAME   ${pathnameandprefix}vtk${vtk_component}${sepchar}${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        endif()
        if(EXISTS ${LIBNAME})
            THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
        endif()
    endmacro()

    # Base libs and their python wrappings
    foreach(VTKLIB ${VTK_AVAILABLE_COMPONENTS})
        SETUP_INSTALL("${VTKLIB}")
    endforeach()

    if(VISIT_HEADERS_SKIP_INSTALL)
        message(STATUS "Skipping vtk headers installation")
    else()
        install(DIRECTORY ${VTK_PREFIX_PATH}/include/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}
            DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk
            FILE_PERMISSIONS OWNER_WRITE OWNER_READ
                             GROUP_WRITE GROUP_READ
                             WORLD_READ
            DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE
                                  GROUP_WRITE GROUP_READ GROUP_EXECUTE
                                  WORLD_READ WORLD_EXECUTE)
    endif()
endif()

# check for python wrappers
set(VTK_PY_WRAPPERS_DIR ${VTK_PREFIX_PATH}/${VTK_PYTHONPATH})
message(STATUS "  VTK_PY_WRAPPERS_DIR=${VTK_PY_WRAPPERS_DIR}")

if(EXISTS ${VTK_PY_WRAPPERS_DIR}/vtkmodules)
    message(STATUS "Found VTK Python Wrappers - ${VTK_PY_WRAPPERS_DIR}")

    if(VISIT_VTK_SKIP_INSTALL)
        message(STATUS "Skipping installation of VTK Python bindings")
    endif()

    set(VTK_PYTHON_WRAPPERS_FOUND TRUE)
else()
    set(VTK_PYTHON_WRAPPERS_FOUND FALSE)
endif()

mark_as_advanced(VTK_PYTHON_WRAPPERS_FOUND)

if(NOT ${VTK_FOUND})
    message(FATAL_ERROR "VTK is required to build VisIt.")
endif()
unset(VTK_DIR)

