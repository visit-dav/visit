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

# Use the UINTAH_DIR hint from the config-site .cmake file 

INCLUDE(${VISIT_SOURCE_DIR}/CMake/SetUpThirdParty.cmake)

# Note UINTAH_UDA2VIS_LIB is called in the reader using dlopen
# so the true library name is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the base name is set and passed to the third party set up
SET(UINTAH_UDA2VIS_LIB
  StandAlone_tools_uda2vis
)

IF (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR ../src)
ELSE (${VISIT_USE_SYSTEM_UINTAH})
  SET(INCLUDE_TMP_DIR include)
ENDIF (${VISIT_USE_SYSTEM_UINTAH})

SET_UP_THIRD_PARTY(UINTAH lib ${INCLUDE_TMP_DIR}
  ${UINTAH_UDA2VIS_LIB}
#  CCA_Components_Arches
#  CCA_Components_Arches_fortran
#  CCA_Components_DataArchiver
#  CCA_Components_Examples
#  CCA_Components_ICE
#  CCA_Components_LoadBalancers
#  CCA_Components_MPM
#  CCA_Components_MPMArches
#  CCA_Components_MPMICE
#  CCA_Components_Models
#  CCA_Components_OnTheFlyAnalysis
#  CCA_Components_Parent
#  CCA_Components_PatchCombiner
  CCA_Components_ProblemSpecification
#  CCA_Components_Regridder
#  CCA_Components_Schedulers
#  CCA_Components_SimulationController
#  CCA_Components_Solvers
#  CCA_Components_SwitchingCriteria
  CCA_Ports
#  Core_Basis
  Core_Containers
  Core_DataArchive
  Core_Datatypes
  Core_Disclosure
  Core_Exceptions
  Core_Geometry
#  Core_GeometryPiece
  Core_Grid
#  Core_IO
#  Core_Labels
  Core_Malloc
  Core_Math
#  Core_OS
  Core_Parallel
  Core_Persistent
  Core_ProblemSpec
  Core_Thread
#  Core_Tracker
  Core_Util
#  testprograms_TestBoxGrouper
#  testprograms_TestConsecutiveRangeSet
#  testprograms_TestMatrix3
#  testprograms_TestRangeTree
#  testprograms_TestSuite
  )

# Note UINTAH_UDA2VIS_LIB is called in the reader using dlopen
# so the true library is need for the compiled operating system.
# This variable MUST BE the FIRST variable in the library list.
# Here the true name is retrieved from the third party set up
LIST(GET UINTAH_LIB 0 UINTAH_UDA2VIS_LIB)

#MESSAGE(STATUS "UINTAH_UDA2VIS_LIB = ${UINTAH_UDA2VIS_LIB}")

UNSET(INCLUDE_TMP_DIR)
