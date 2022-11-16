# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#****************************************************************************
# Modifications:
#   Kathleen Bonnell, Thu Dec 10 17:53:36 MT 2009
#   Use the same find routines whether on Windows or not.
#
#   Cyrus Harrison, Fri Feb 19 15:41:04 PST 2010
#   Added install of vtk python wrappers (if they exist).
#
#   Cyrus Harrison, Tue Mar  9 07:51:00 PST 2010
#   Added install_name_tool patch of of vtk python wrappers (if they exist)
#
#   Kathleen Bonnell,  Wed Mar 17 10:03:52 MST 2010
#   Prevent '.svn' from being included when installing directories. 
#   Change how python wrappers are handled on windows, due to different
#   VTK directory structure.
#
#   Kathleen Bonnell,  Thu Dec 2 15:30:05 MST 2010
#   Add .lib files for Install on Windows.
# 
#   Tom Fogal, Fri Jan 28 13:35:29 MST 2011
#   Account for new VTK installation structure.
#
#   Kathleen Bonnell, Tue Jun  7 11:10:51 PDT 2011
#   Don't add VTK's MangledMesa directory to VTK_INCLUDE_DIRS unless VTK
#   was built with MangledMesa.
#
#   Brad Whitlock, Mon Nov 21 10:22:56 PST 2011
#   Print out some variables that were supposed to have been set by the
#   FindVTK.cmake routine so we know better what's going on. Also deal with
#   the case where VTK has been installed normally (with vtk-5.8 subdirs)
#   and change how Python filters are located on Windows.
#
#   Cyrus Harrison, Tue Sep 25 12:09:39 PDT 2012
#   Added Geoviz libs
#
#   Kathleen Biagas, Thu Dec 6 10:26:54 PST 2012 
#   Use VTK_LIBRARIES and others in determining what to install, rather than
#   listing individually, which is prone to omissions.
#
#   Kathleen Biagas, Wed Jan 30 17:54:21 MST 2013
#   Preliminary changes for VTK-6.
#
#   Kathleen Biagas, Thu Feb 21 16:04:58 MST 2013
#   Set VTK_LIBRARY_DIRS manually.
#
#   Kathleen Biagas, Thu May  2 10:11:44 PDT 2013
#   Don't attempt the python wrappers if we don't have PYTHON_VERSION.
# 
#   Kathleen Biagas, Fri May  3 17:00:03 MST 2013
#   Ensure that the vtk install has all the libraries we require.
# 
#   Alister Maguire, Thu Sep 14 14:26:07 PDT 2017
#   Added vtkRenderingVolumeOpenGL2
#
#   Kathleen Biagas, Fri Jan  17 2020
#   Install vtkTestOpenGLVersion.exe on Windows when needed.
# 
#   Kathleen Biagas, Sat July 17, 2022 
#   Renamed from FindVisItVTK.cmake.
# 
#****************************************************************************/

INCLUDE(${VISIT_SOURCE_DIR}/CMake/ThirdPartyInstallLibrary.cmake)

# Use the VTK_DIR hint from the config-site .cmake file 

IF(EXISTS ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}/VTKConfig.cmake)
    SET(VTK_DIR ${VISIT_VTK_DIR}/lib/cmake/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION})
ENDIF()

MESSAGE(STATUS "Checking for VTK in ${VTK_DIR}")

# Set up our list of required and optional vtk modules
SET(REQ_VTK_MODS 
        vtkCommonCore
        vtkCommonDataModel
        vtkFiltersCore
        vtkFiltersFlowPaths
        vtkFiltersHybrid
        vtkFiltersModeling
        vtkIOLegacy
        vtkIOPLY
        vtkIOXML
        vtkInteractionStyle
        vtkRenderingAnnotation
        vtkRenderingOpenGL2
        vtkRenderingVolumeOpenGL2
        vtkglew)

IF(NOT VISIT_SERVER_COMPONENTS_ONLY AND NOT VISIT_ENGINE_ONLY AND NOT VISIT_DBIO_ONLY)
    LIST(APPEND REQ_VTK_MODS vtkGUISupportQtOpenGL)
ENDIF()

# Optional
#SET(OPT_VTK_MODS
#       vtkGeovisCore # Cartographic Projection
#       vtkIOEnSight  # EnSight
#       vtklibxml2    # Xdmf
#   )

# We don't list our required modules in the find_package call because it
# does funny things with VTK_INCLUDES, and the OPTIONAL_COMPONENTS arg
# causes an error if anything in the optional list is not found, would be better
# if it issued a warning instead. Perhaps one day it will be fixed, and we can 
# use this: find_package(VTK 6.0 REQUIRED ${REQ_VTK_MODS} OPTIONAL_COMPONENTS ${OPT_VTK_MODS} NO_MODULE PATHS ${VTK_DIR})

set(Qt5_DIR ${VISIT_QT_DIR}/lib/cmake/Qt5)
find_package(VTK ${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} REQUIRED NO_MODULE PATHS ${VTK_DIR})

# Ensure we have all the required modules:
FOREACH(module ${REQ_VTK_MODS})
    IF(NOT TARGET ${module})
        MESSAGE(ERROR "VisIt requires ${module}")
    ENDIF()
ENDFOREACH()


# due to cmake policy 0126, created in cmake 3.21
# if a var is already defined, calling set even with FORCE will not overwrite
# the value so we unset it here to make it clear we want to replace the old
# value with something new
unset(VTK_LIBRARY_DIRS)

SET(VTK_LIBRARY_DIRS ${VTK_INSTALL_PREFIX}/lib CACHE PATH "Path to vtk libraries" FORCE)
MESSAGE(STATUS "  VTK_FOUND=${VTK_FOUND}")
MESSAGE(STATUS "  VTK_MAJOR_VERSION=${VTK_MAJOR_VERSION}")
MESSAGE(STATUS "  VTK_MINOR_VERSION=${VTK_MINOR_VERSION}")
MESSAGE(STATUS "  VTK_BUILD_VERSION=${VTK_BUILD_VERSION}")
MESSAGE(STATUS "  VTK_INCLUDE_DIRS=${VTK_INCLUDE_DIRS}")
MESSAGE(STATUS "  VTK_DEFINITIONS=${VTK_DEFINITIONS}")
MESSAGE(STATUS "  VTK_LIBRARIES=${VTK_LIBRARIES}")
MESSAGE(STATUS "  VTK_INSTALL_PREFIX=${VTK_INSTALL_PREFIX}")
MESSAGE(STATUS "  VTK_LIBRARY_DIRS=${VTK_LIBRARY_DIRS}")

# Add install commands for all of the VTK libraries. Is there a better way?
IF(APPLE)
    SET(SO_EXT "dylib")
ELSE(APPLE)
    IF(WIN32)
        SET(SO_EXT "dll")
    ELSE(WIN32)
        SET(SO_EXT "so")
    ENDIF(WIN32)
ENDIF(APPLE)

IF(VISIT_VTK_SKIP_INSTALL)
    MESSAGE(STATUS "Skipping installation of VTK libraries")
ELSE(VISIT_VTK_SKIP_INSTALL)
    IF(NOT WIN32)
        SET(pathnameandprefix "${VTK_INSTALL_PREFIX}/lib/lib")
    ELSE()
        SET(pathnameandprefix "${VTK_INSTALL_PREFIX}/bin/")
        SET(pathnameandprefixlib "${VTK_INSTALL_PREFIX}/lib/")
    ENDIF(NOT WIN32)
    MACRO(SETUP_INSTALL vtklib)
        SET(LIBNAME   ${pathnameandprefix}${vtklib}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.${SO_EXT})
        IF(EXISTS ${LIBNAME})
            THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
        ENDIF(EXISTS ${LIBNAME})

        IF(WIN32)
            # install .lib versions, too
            SET(LIBNAME   ${pathnameandprefixlib}${vtklib}-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}.lib)
            IF(EXISTS ${LIBNAME})
                THIRD_PARTY_INSTALL_LIBRARY(${LIBNAME})
            ENDIF(EXISTS ${LIBNAME})
        ENDIF(WIN32)
    ENDMACRO(SETUP_INSTALL vtklib)
  
    # Base libs and their python wrappings
    FOREACH(VTKLIB ${VTK_LIBRARIES})
        SETUP_INSTALL("${VTKLIB}")
    ENDFOREACH(VTKLIB)  

    # Python
    IF(PYTHON_VERSION)
        # different naming convention.
        # Remove '.' from python version for use in vtk python library naming. 
        STRING(REPLACE "." "" PYVER ${PYTHON_VERSION})
        FOREACH(VTKLIB ${VTK_LIBRARIES})
            SETUP_INSTALL("${VTKLIB}Python${PYVER}D")
        ENDFOREACH(VTKLIB)  
        SETUP_INSTALL(vtkWrappingPython${PYVER}Core)
    ENDIF(PYTHON_VERSION)

    # Add install targets for VTK headers too -- but just the vtk-5.0 dir.
    # The VTK_INCLUDE_DIRS may contain stuff like /usr/include or the
    # Python directory and we just want VTK here.
    IF(VISIT_HEADERS_SKIP_INSTALL)
        MESSAGE(STATUS "Skipping vtk headers installation")
    ELSE(VISIT_HEADERS_SKIP_INSTALL)
        FOREACH(X ${VTK_INCLUDE_DIRS})
            IF(EXISTS ${X}/vtkActor.h)
                #MESSAGE("Install ${X} to ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk")
                INSTALL(DIRECTORY ${X}
                    DESTINATION ${VISIT_INSTALLED_VERSION_INCLUDE}/vtk
                    FILE_PERMISSIONS OWNER_WRITE OWNER_READ 
                                     GROUP_WRITE GROUP_READ 
                                     WORLD_READ
                    DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE 
                                          GROUP_WRITE GROUP_READ GROUP_EXECUTE 
                                          WORLD_READ WORLD_EXECUTE
                    PATTERN ".svn" EXCLUDE
                )
            ENDIF(EXISTS ${X}/vtkActor.h)
        ENDFOREACH(X)
    ENDIF(VISIT_HEADERS_SKIP_INSTALL)
ENDIF(VISIT_VTK_SKIP_INSTALL)

# check for python wrappers
if(NOT WIN32)
    file(GLOB VTK_PY_WRAPPERS_DIR ${VTK_LIBRARY_DIRS}/python*/)
else ()
    file(GLOB VTK_PY_WRAPPERS_DIR ${VISIT_VTK_DIR}/lib/python*)
endif()
MESSAGE(STATUS "  VTK_PY_WRAPPERS_DIR=${VTK_PY_WRAPPERS_DIR}")

IF(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    MESSAGE(STATUS "Found VTK Python Wrappers - ${VTK_PY_WRAPPERS_DIR}")
    FILE(GLOB VTK_PY_EGG ${VTK_PY_WRAPPERS_DIR}/site-packages/*.egg*)
    FILE(GLOB VTK_PY_MODULE ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)

    IF(VISIT_VTK_SKIP_INSTALL)
        MESSAGE(STATUS "Skipping installation of VTK Python bindings")
    ELSE(VISIT_VTK_SKIP_INSTALL)
        INSTALL(FILES ${VTK_PY_EGG}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
            )

        INSTALL(DIRECTORY ${VTK_PY_MODULE}
                DESTINATION ${VISIT_INSTALLED_VERSION_LIB}/site-packages/
                FILE_PERMISSIONS OWNER_WRITE OWNER_READ GROUP_WRITE GROUP_READ WORLD_READ
                DIRECTORY_PERMISSIONS OWNER_WRITE OWNER_READ OWNER_EXECUTE GROUP_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
                PATTERN ".svn" EXCLUDE
            )
    ENDIF(VISIT_VTK_SKIP_INSTALL)

    SET(VTK_PYTHON_WRAPPERS_FOUND TRUE)
ELSE(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)
    SET(VTK_PYTHON_WRAPPERS_FOUND FALSE)
ENDIF(EXISTS ${VTK_PY_WRAPPERS_DIR}/site-packages/vtk)

MARK_AS_ADVANCED(VTK_PYTHON_WRAPPERS_FOUND)

# prepare for drop-in replacement of mesa with opengl, if all pieces are in place
if(WIN32 AND VISIT_MESA_REPLACE_OPENGL AND VISIT_MESAGL_DIR)
    if(EXISTS ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe)
        install(FILES ${VISIT_VTK_DIR}/bin/vtkTestOpenGLVersion.exe
                DESTINATION ${VISIT_INSTALLED_VERSION_BIN}
                PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                            GROUP_READ GROUP_WRITE GROUP_EXECUTE
                            WORLD_READ WORLD_EXECUTE
                CONFIGURATIONS "" None Debug Release RelWithDebInfo MinSizeRel
                )
    endif()
endif()

IF(NOT ${VTK_FOUND})
    MESSAGE(FATAL_ERROR "VTK is required to build VisIt.")
ENDIF(NOT ${VTK_FOUND})
