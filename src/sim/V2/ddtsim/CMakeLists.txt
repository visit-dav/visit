#*****************************************************************************
#
# Copyright (c) 2011, Allinea
# All rights reserved.
#
# This file has been contributed to the VisIt project, which is
# Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
# https://visit.llnl.gov/.  The full copyright notice is contained in the 
# file COPYRIGHT located at the root of the VisIt distribution or at 
# http://www.llnl.gov/visit/copyright.html.
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
#****************************************************************************/

SET(DDTSIM_SOURCES
visitsim.cpp
visitdata.cpp
ddtsim.cpp
visitmesh.cpp
visitvariable.cpp
)

# Sources needed only when using dynamic linking
SET(DDTSIM_DYNAMIC_SOURCES
mpicompat/openmpi.c
mpicompat/mpich.c
dllibrary.cpp
libsimlibrary.cpp
)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_SOURCE_DIR}/mpicompat
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/sim/V2/lib
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

# Not much point in this library if it can't be read by a debugger!
ADD_DEFINITIONS(-g)

IF(NOT VISIT_STATIC)
	ADD_PARALLEL_LIBRARY(ddtsim SHARED ${DDTSIM_SOURCES} ${DDTSIM_DYNAMIC_SOURCES})
	INSTALL(TARGETS ddtsim
		ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ELSE(NOT VISIT_STATIC)
	ADD_DEFINITIONS(-DVISIT_STATIC)
	ADD_PARALLEL_LIBRARY(ddtsim_static STATIC ${DDTSIM_SOURCES})
	TARGET_LINK_LIBRARIES(ddtsim_static
		simV2_static_par
	)
	IF(NOT APPLE)
		SET_TARGET_PROPERTIES(ddtsim_static PROPERTIES INSTALL_RPATH "$ORIGIN")
	ENDIF(NOT APPLE)
	
	INSTALL(TARGETS ddtsim_static
		ARCHIVE DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		LIBRARY DESTINATION ${VISIT_INSTALLED_VERSION}/libsim/V2/lib
		PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
ENDIF(NOT VISIT_STATIC)

