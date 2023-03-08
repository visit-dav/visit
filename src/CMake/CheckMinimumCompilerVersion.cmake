# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Checks for minimum compiler version that is supported by VisIt
# Borrowed from VTK.
#-----------------------------------------------------------------------------



#-----------------------------------------------------------------------------
# Minimum compiler version check: GCC >= 7.3
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.3)
  message(FATAL_ERROR "GCC 7.3 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: LLVM Clang >= 3.3
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 3.3)
  message(FATAL_ERROR "LLVM Clang 3.3 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: Apple Clang >= 5.0 (Xcode 5.0)
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.0)
  message(FATAL_ERROR "Apple Clang 5.0 or later is required.")
endif ()

#-----------------------------------------------------------------------------

# Minimum compiler version check: Microsoft C/C++ >= 19.1 (aka VS 2017)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.1)
  message(FATAL_ERROR "Microsoft Visual Studio 2017 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: Intel C++ (ICC) >= 14
if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
  message(FATAL_ERROR "Intel C++ (ICC) 14.0 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# VTKm requires C++14. Turning off extensions for maximum compatability.
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_EXTENSIONS OFF)

set(BLT_CXX_STD "c++14" CACHE STRING "")

