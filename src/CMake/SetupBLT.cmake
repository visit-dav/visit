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

message(STATUS "Setting up BLT")
################################################################
# init blt using BLT_SOURCE_DIR
################################################################
include(${BLT_SOURCE_DIR}/SetupBLT.cmake)

# allow us to find python again
set(CMAKE_DISABLE_FIND_PACKAGE_PythonInterp OFF)
