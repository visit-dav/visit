#*****************************************************************************
#
# Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
# Produced at the Lawrence Livermore National Laboratory
# LLNL-CODE-400142
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
#    Kathleen Bonnell, Wed Dec  9 15:09:03 MT 2009
#    Make PDB_LIB a cached STRING, and PDB_FOUND a cached BOOL.
#
#****************************************************************************/

# Use the SILO_DIR hint from the config-site .cmake file 
#

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

IF (WIN32)
  SET_UP_THIRD_PARTY(SILO lib/${VISIT_MSVC_VERSION} include silohdf5)
ELSE (WIN32)
  SET_UP_THIRD_PARTY(SILO lib include siloh5)
ENDIF (WIN32)

# We use Silo for PDB most of the time so set up additional PDB variables.
IF(SILO_FOUND)
    SET(PDB_FOUND 1 CACHE BOOL "PDB library found")
    SET(PDB_INCLUDE_DIR ${SILO_INCLUDE_DIR} CACHE PATH "PDB include directory")
    SET(PDB_LIBRARY_DIR ${SILO_LIBRARY_DIR} CACHE PATH "PDB library directory")
    SET(PDB_LIB ${SILO_LIB} CACHE STRING "PDB library")
    MARK_AS_ADVANCED(PDB_INCLUDE_DIR PDB_LIBRARY_DIR PDB_LIB)
ENDIF(SILO_FOUND)
