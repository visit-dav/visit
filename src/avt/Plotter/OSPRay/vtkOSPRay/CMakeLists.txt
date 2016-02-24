## ======================================================================================= ##
## Copyright 2014-2015 Texas Advanced Computing Center, The University of Texas at Austin  ##
## All rights reserved.                                                                    ##
##                                                                                         ##
## Licensed under the BSD 3-Clause License, (the "License"); you may not use this file     ##
## except in compliance with the License.                                                  ##
## A copy of the License is included with this software in the file LICENSE.               ##
## If your copy does not contain the License, you may obtain a copy of the License at:     ##
##                                                                                         ##
##     http://opensource.org/licenses/BSD-3-Clause                                         ##
##                                                                                         ##
## Unless required by applicable law or agreed to in writing, software distributed under   ##
## the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY ##
## KIND, either express or implied.                                                        ##
## See the License for the specific language governing permissions and limitations under   ##
## limitations under the License.                                                          ##
## ======================================================================================= ##

# vtkOSPRay is an interface from VTK's rendering classes to OSPRay,
# a "real time" Ray Tracer.

# It creates a library that includes OSPRay specific subclasses
# of VTK's rendering classes, Renderer, Camera, etc.
# These can be instantiated by name, or by using object factory overloading
# at post compile time (ie, set VTK_AUTOLOAD_PATH to vtkOSPRay's build
# directory and run a normal vtk app)

project(vtkOSPRay)



#Configure vtkOSPRay source proper ############################################

# Define a flag
if (BUILD_AGAINST_PARAVIEW)
    add_definitions(-DVTKOSPRAY_FOR_PARAVIEW)
endif(BUILD_AGAINST_PARAVIEW)

# Set vtkOSPRay source files.
SET ( OSPRAY_CLASSES
    vtkOSPRayActor.cxx
    #vtkOSPRayRenderable.cxx
    vtkOSPRayCamera.cxx
    vtkOSPRayLight.cxx
    vtkOSPRayManager.cxx
    vtkOSPRayPolyDataMapper.cxx
    vtkOSPRayProperty.cxx
    vtkOSPRayRenderer.cxx
    vtkOSPRayTestSource.cxx
    vtkOSPRayTexture.cxx
    vtkOSPRayVolumeRayCastMapper.h
    vtkOSPRayVolumeRayCastMapper.cxx
    vtkOSPRayPVLODVolume.cxx
    )

IF (BUILD_AGAINST_PARAVIEW)
    add_definitions(-DVTKOSPRAY_FOR_PARAVIEW)
    # paraview uses its own LOD actor, put it in here
    SET ( OSPRAY_CLASSES ${OSPRAY_CLASSES}
        vtkOSPRayLODActor.cxx
        vtkPVOSPRayRepresentation.cxx
        vtkPVOSPRayRepresentation.h
        vtkPVOSPRayOutlineRepresentation.cxx
        vtkPVOSPRayOutlineRepresentation.h
        # pnav - ParaView 4.3 is unhappy with some code in these, disabling for now
        #        something changed between the VTK in PV 4.1 and PV 4.3
        vtkPVOSPRayImageVolumeRepresentation.cxx
        vtkPVOSPRayImageVolumeRepresentation.h
        vtkOSPRayCompositeMapper.cxx)
ENDIF (BUILD_AGAINST_PARAVIEW)

add_definitions(
    -DvtkCommonCore_EXPORTS
    -DvtkPVClientServerCoreRendering_EXPORTS
    -DvtkRenderingVolume_EXPORTS
    -DvtkPVVTKExtensionsRendering_EXPORTS
    )

SET ( OSPRAY_SRCS
    ${OSPRAY_CLASSES}
    #   vtkOSPRayObjectFactory.cxx
    )

set (${vtk-module}_NO_HeaderTest 1)

vtk_module_library(${vtk-module} ${OSPRAY_SRCS} ${MOCSrcs} ${UISrcs})
find_package(OSPRay)


# setup include paths
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/vtkOSPRay)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/vtkOSPRay)
INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR}/Base)
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/Base)


# if(PARAVIEW_BUILD_QT_GUI)
#   INCLUDE(${QT_USE_FILE})
# endif(PARAVIEW_BUILD_QT_GUI)
FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
INCLUDE_DIRECTORIES(
    ${QT_INCLUDE_DIR}
    )

if(VTK_USE_X)
    vtk_module_link_libraries(${vtk-module}
        LINK_PRIVATE ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} ${X11_LIBRARIES} ${X11_Xt_LIB})
elseif(VTK_USE_COCOA)
    vtk_module_link_libraries(${vtk-module} ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} "-framework OpenGL -framework Cocoa")
    target_link_libraries(${vtk-module} ${OSPRAY_LIBRARIES} ${QT_LIBRARIES} ${VTK_LIBRARIES})
endif()

# OSPRay_Core uses MMTime which is in it's own special library.
if(WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} LINK_PRIVATE Winmm)
endif()

#Examples ####################################################################
#IF ( BUILD_EXAMPLES )
#ADD_SUBDIRECTORY(Examples)
#ENDIF ( BUILD_EXAMPLES )
