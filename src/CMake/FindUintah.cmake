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
# Modifications:
#   Kevin Griffin, Wed Sep 4 08:31:26 PDT 2019
#   Added Allen's fix for stripping off the filename path.
#
#****************************************************************************/

# Use the UINTAH_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

# Note UINTAH_INTERFACES_LIB is called in the reader using dlopen
# so the true library name is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the base name is set and passed to the third party set up
SET(UINTAH_INTERFACES_LIB
  VisIt_interfaces
)

IF (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR ../src)
ELSE (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR include)
ENDIF (${VISIT_USE_SYSTEM_UINTAH})


SET(REQ_UINTAH_MODS 
  CCA_Components_DataArchiver
  CCA_Components_LoadBalancers
  CCA_Components_ProblemSpecification
  CCA_Components_Schedulers
  CCA_Ports
  Core_Containers
  Core_DataArchive
  Core_Datatypes
  Core_Disclosure
  Core_Exceptions
  Core_Geometry
  Core_GeometryPiece
  Core_Grid
  Core_IO
  Core_Malloc
  Core_Math
  Core_OS
  Core_Parallel
  Core_ProblemSpec
  Core_Util
  )

SET_UP_THIRD_PARTY(UINTAH lib ${INCLUDE_TMP_DIR}
  ${UINTAH_INTERFACES_LIB}
  ${REQ_UINTAH_MODS}
)

# Note UINTAH_INTERFACES_LIB is called in the reader using dlopen
# so the true library is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the true name is retrieved from the third party set up
LIST(GET UINTAH_LIB 0 UINTAH_INTERFACES_LIB)

GET_FILENAME_COMPONENT(UINTAH_INTERFACES_LIB ${UINTAH_INTERFACES_LIB} NAME)

MESSAGE(STATUS "UINTAH_INTERFACES_LIB = ${UINTAH_INTERFACES_LIB}")

UNSET(INCLUDE_TMP_DIR)
