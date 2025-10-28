# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Conduit.

################################################################
# if BLT_SOURCE_DIR is not set - use "blt" as default
################################################################
# Note: this is relative to the root CMakeLists.txt file
# so relative to the git repo root, this is actually `src/blt`
if(NOT BLT_SOURCE_DIR)
    set(BLT_SOURCE_DIR "blt")
endif()

################################################################
# if not set, prefer c++11 lang standard
################################################################
if(NOT BLT_CXX_STD)
    set(BLT_CXX_STD "c++11" CACHE STRING "")
endif()

################################################################
# don't use BLT's all warnings feature
set(ENABLE_ALL_WARNINGS OFF CACHE BOOL "")
################################################################

################################################################
################################################################
# Google Test will look or python
# If PYTHON_EXECUTABLE is not set, it will go on to find
# the wrong python, so this logic protects us when using
# PYTHON_DIR, as VisIt has for some time
################################################################
################################################################
set(CMAKE_DISABLE_FIND_PACKAGE_PythonInterp ON)


################################################################
# Turn off BLT testing until needed
################################################################
set(ENABLE_TESTS OFF CACHE BOOL "")

################################################################
# Enable FOLDERS
################################################################
set(ENABLE_FOLDERS ON CACHE BOOL "")


################################################################
# init blt using BLT_SOURCE_DIR
################################################################

message(STATUS "Setting up BLT")
include(${BLT_SOURCE_DIR}/SetupBLT.cmake)

#
# Override some options related to RPATHS.
#

# BLT sets CMAKE_INSTALL_RPATH_USE_LINK_PATH to TRUE, which
# turns on saving automatically generated RPATHS in installations.
# We don't want RPATHS in installations.
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

# BLT sets CMAKE_INSTALL_RPATH to "${CMAKE_INSTALL_PREFIX}/lib",
# which usually results in /usr/local/lib, which we don't want.
set(CMAKE_INSTALL_RPATH "")


# next message helps bracket messages sent by BLT.
message(STATUS "Done setting up BLT")

# allow us to find python again
set(CMAKE_DISABLE_FIND_PACKAGE_PythonInterp OFF)
