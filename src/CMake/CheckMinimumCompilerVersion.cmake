#*****************************************************************************
#
# Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-442911
# All rights reserved.
#
# This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
# full copyright notice is contained in the file COPYRIGHT located at the root
# of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
#
# Redistribution  and  use  in  source  and  binary  forms,  with  or  without
# modification, are permitted provided that the following conditions are met:
#
#  - Redistributions of  source code must  retain the above  copyright notice,
#    this list of conditions and the disclaimer below.
#  - Redistributions in binary form must reproduce the above copyright notice,
#    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
#    documentation and/or other materials provided with the distribution.
#  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
#    be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
# ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
# LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
# DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
# SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
# CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
# LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
# OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#
#*****************************************************************************

#-----------------------------------------------------------------------------
# Checks for minimum compiler version that is supported by VisIt
# Borrowed from VTK.
#-----------------------------------------------------------------------------



#-----------------------------------------------------------------------------
# Minimum compiler version check: GCC >= 4.8
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
  message(FATAL_ERROR "GCC 4.8 or later is required.")
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
# Minimum compiler version check: Microsoft C/C++ >= 18.0 (aka VS 2013 aka VS 12.0)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 18.0)
  message(FATAL_ERROR "Microsoft Visual Studio 2013 or later is required.")
endif ()

#-----------------------------------------------------------------------------
# Minimum compiler version check: Intel C++ (ICC) >= 14
if (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.0)
  message(FATAL_ERROR "Intel C++ (ICC) 14.0 or later is required.")
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)



