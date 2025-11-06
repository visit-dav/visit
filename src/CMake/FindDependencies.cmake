
###############################################################################
# FindDependencies
# This file defines:
###############################################################################

#include(CMakeFindDependencyMacro)


# Compute the installation prefix relative to this file.
get_filename_component(_TP_PREFIX ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component(_TP_PREFIX ${_TP_PREFIX} PATH)
message("TP PREFIX: ${_TP_PREFIX}")

# need conditionals on this for platform?
find_package(Threads REQUIRED)

if(LINUX)
    find_package(X11 REQUIRED)
endif()


# some things needed by VTK that aren't automatically setup yet:

#add_library(ZLIB::ZLIB SHARED IMPORTED)
#set_target_properties(ZLIB::ZLIB PROPERTIES
#        INTERFACE_INCLUDE_DIRECTORIES ${_TP_PREFIX}/include/zlib
#        IMPORTED_LOCATION ${_TP_PREFIX}/lib/libzlib.so)

#add_library(OSMesa::OSMesa SHARED IMPORTED)
#set_target_properties(OSMesa::OSMesa PROPERTIES
#    INTERFACE_INCLUDE_DIRECTORIES ${_TP_PREFIX}/include/mesagl
#    IMPORTED_LOCATION ${_TP_PREFIX}/lib/libOSMesa.so.8)

#if(HAVE_OSPRAY)
#    add_library(ospray::ospray SHARED IMPORTED)
#    set_target_properties(ospray::ospray PROPERTIES
#        INTERFACE_INCLUDE_DIRECTORIES ${_TP_PREFIX}/include/ospray
#        IMPORTED_LOCATION ${_TP_PREFIX}/lib/libospray.so)
#endif()

###############################################################################
# Import Qt's CMake targets
###############################################################################

include(${CMAKE_CURRENT_LIST_DIR}/SetupQT.cmake)

# satisfy VTK
if(HAVE_OSMESA)
    add_library(OSMesa::OSMesa SHARED IMPORTED)
endif()

###############################################################################
# Import Python's CMake targets
###############################################################################

include(${CMAKE_CURRENT_LIST_DIR}/SetupPYTHON.cmake)

###############################################################################
# Import VTK's CMake targets
###############################################################################

include(${CMAKE_CURRENT_LIST_DIR}/SetupVTK.cmake)

###############################################################################
# Import VTKm's CMake targets
###############################################################################

if(HAVE_VTKM)
    include(${CMAKE_CURRENT_LIST_DIR}/SetupVTKM.cmake)
endif()

###############################################################################
# Import ANARI's CMake targets
###############################################################################

if(HAVE_ANARI)
    include(${CMAKE_CURRENT_LIST_DIR}/SetupANARI.cmake)
endif()


