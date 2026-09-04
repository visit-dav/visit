# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#-----------------------------------------------------------------------------
# Checks for minimum compiler version that is supported by VisIt
# Borrowed from VTK.
#
# Modifications;
#   Kathleen Biagas, Fri Sep 4, 2206
#   Update gcc to 10.3, MSVC to 2022, llvm clang to 14, apple clang to 11,
#   icc to 2021.6.0
#
#-----------------------------------------------------------------------------



#-----------------------------------------------------------------------------
# Minimum compiler version check: GCC >= 10.3
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.3)
      message(FATAL_ERROR "GCC 10.3 or later is required.")
    endif()
endif()

#-----------------------------------------------------------------------------
# Minimum compiler version check: LLVM Clang >= 14.0
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
  message(FATAL_ERROR "LLVM Clang 14.0 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: Apple Clang >= 11.0 (Xcode 11.3.1)
if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0)
  message(FATAL_ERROR "Apple Clang 11.0 or later is required.")
endif ()

#-----------------------------------------------------------------------------

# Minimum compiler version check: Microsoft C/C++ >= 19.44 (aka VS 2022)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.44)
  message(FATAL_ERROR "Microsoft Visual Studio 2022 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: Intel C++ (ICC) >= 2021.6.0
if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 2021.6.0)
  message(FATAL_ERROR "Intel C++ (ICC) 2021.6.0 or later is required.")
endif ()

#---------------------------------------------------------------------------
# Require C++17. Turning off extensions for maximum compatability.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_EXTENSIONS OFF)

set(BLT_CXX_STD "c++17" CACHE STRING "")

