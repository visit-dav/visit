
###############################################################################
# FindDependencies
#
###############################################################################


# Compute the installation prefix relative to this file.
get_filename_component(_TP_PREFIX ${CMAKE_CURRENT_LIST_FILE} PATH)
get_filename_component(_TP_PREFIX ${_TP_PREFIX} PATH)
message("TP PREFIX: ${_TP_PREFIX}")

# need conditionals on this for platform?
find_package(Threads REQUIRED)

if(LINUX)
    find_package(X11 REQUIRED)
endif()

if(VISIT_PARALLEL)
    include(${CMAKE_CURRENT_LIST_DIR}/VisItParallel.cmake)
endif()


###############################################################################
# Import Qt's CMake targets
###############################################################################

include(${CMAKE_CURRENT_LIST_DIR}/SetupQT.cmake)


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


