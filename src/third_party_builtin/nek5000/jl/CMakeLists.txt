#*****************************************************************************
#
# Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
# Modifications:
#
#   Kathleen Biagas, Wed Nov  6 18:01:08 PST 2013
#   Changed location of generated 'nek_config.h' so that it won't overwrite
#   repo object on Windows.
#
#****************************************************************************/
set(CMAKE_VERBOSE_MAKEFILE ON)

set(FUN_PREFIX "nek_" CACHE STRING "prefix for exported symbols")

option(LOCAL_LONG "use long for local index type (instead of int)" ON)
option(LOCAL_LONG_LONG "use long long for local index type (instead of int)" OFF)

ADD_DEFINITIONS(-DPREFIX=${FUN_PREFIX} -DUSE_NAIVE_BLAS -DNO_NEK_EXITT -DUNDERSCORE)

if(LOCAL_LONG)
  add_definitions(-DUSE_LONG)
  set(NEK_UINT "unsigned long")
elseif(LOCAL_LONG_LONG)
  add_definitions(-DUSE_LONG_LONG)
  set(NEK_UINT "unsigned long long")
else()
  set(NEK_UINT "unsigned")
endif()

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/ext/nek_config.h.in"
  "${VISIT_BINARY_DIR}/include/nek_config.h"
  )


SET(NEK5000_INTERP_SOURCES
sort.c
sarray_sort.c
obbox.c
tensor.c
poly.c
lob_bnd.c
findpts_el_3.c
findpts_el_2.c
fail.c
comm.c
gs_local.c
findpts_local.c
mem.c
ext/findpts_local_ext.c
)

INCLUDE_DIRECTORIES(
${VISIT_SOURCE_DIR}/third_party_builtin/nek5000
${VISIT_SOURCE_DIR}/visit_vtk/full
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
)

#Add link directories needed.
LINK_DIRECTORIES()

ADD_LIBRARY(nek5000_interp ${NEK5000_INTERP_SOURCES})
TARGET_LINK_LIBRARIES(nek5000_interp)

VISIT_INSTALL_TARGETS(nek5000_interp)
SET_TARGET_PROPERTIES(nek5000_interp PROPERTIES FOLDER thirdparty_builtin/nek5000_interp)
