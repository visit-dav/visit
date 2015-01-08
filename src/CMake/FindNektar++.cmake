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
#****************************************************************************/

# Use the NEKTAR++_DIR hint from the config-site .cmake file 

#MESSAGE(STATUS "  NEKTAR++_VERSION=${NEKTAR++_VERSION}")
#MESSAGE(STATUS "  VISIT_NEKTAR++_DIR=${VISIT_NEKTAR++_DIR}")

If( EXISTS ${VISIT_NEKTAR++_DIR} )

    IF(EXISTS ${VISIT_NEKTAR++_DIR}/lib/nektar++-${NEKTAR++_VERSION}/cmake/NEKTAR++Config.cmake)
        SET(NEKTAR++_DIR ${VISIT_NEKTAR++_DIR})
    ENDIF()

    MESSAGE(STATUS "Checking for Nektar++ in ${NEKTAR++_DIR}/lib/nektar++-${NEKTAR++_VERSION}/cmake")

    #  Find Nektar++
    set(CMAKE_PREFIX_PATH ${VISIT_NEKTAR++_DIR}/lib/nektar++-${NEKTAR++_VERSION}/cmake ${CMAKE_PREFIX_PATH})
    set(CMAKE_LIBRARY_PATH ${VISIT_NEKTAR++_DIR}/lib/nektar++-${NEKTAR++_VERSION} ${CMAKE_LIBRARY_PATH})

    FIND_PACKAGE(Nektar++)
    #FIND_PACKAGE(NEKTAR++ 4.0.0 REQUIRED PATHS ${NEKTAR++_DIR})

    #MESSAGE(STATUS "  NEKTAR++_INCLUDE_DIRS=${NEKTAR++_INCLUDE_DIRS}")
    #MESSAGE(STATUS "  NEKTAR++_TP_INCLUDE_DIRS=${NEKTAR++_TP_INCLUDE_DIRS}")

    #MESSAGE(STATUS "  NEKTAR++_LIBRARY_DIRS=${NEKTAR++_LIBRARY_DIRS}")
    #MESSAGE(STATUS "  NEKTAR++_TP_LIBRARY_DIRS=${NEKTAR++_TP_LIBRARY_DIRS}")

    #MESSAGE(STATUS "  NEKTAR++_DEFINITIONS=${NEKTAR++_DEFINITIONS}")
ENDIF()

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

SET_UP_THIRD_PARTY(NEKTAR++
    lib/nektar++-${NEKTAR++_VERSION}
    include/nektar++-${NEKTAR++_VERSION}
    MultiRegions
    LocalRegions
    SpatialDomains
    StdRegions
    LibUtilities)

