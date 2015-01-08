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
#   Kathleen Biagas, Wed Oct 10 10:39:15 PDT 2012
#   Add ddtsim only if VISIT_DDT is chosen.
#
#   Brad Whitlock, Fri May 17 13:43:48 PDT 2013
#   Define LIBSIM_SOURCES.
#
#****************************************************************************/

# We define this value here so we can use it in multiple CMakeLists.txt under
# this directory.
SET(LIBSIM_SOURCES
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisItControlInterface_V2.c 
${CMAKE_CURRENT_SOURCE_DIR}/lib/SimUI.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/SimV2Tracing.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisItFortran.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CommandMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CSGMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurveData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurveMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_CurvilinearMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainBoundaries.c 
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainList.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_DomainNesting.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_ExpressionMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MaterialData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MaterialMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_MeshMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_NameList.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_PointMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_RectilinearMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SimulationMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SpeciesData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_SpeciesMetaData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_UnstructuredMesh.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_VariableData.c
${CMAKE_CURRENT_SOURCE_DIR}/lib/VisIt_VariableMetaData.c
)

ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(runtime)
IF(VISIT_DDT)
    ADD_SUBDIRECTORY(ddtsim)
ENDIF(VISIT_DDT)
# Disable swig bindings on Windows for now. Generated code has some errors.
IF(UNIX)
    ADD_SUBDIRECTORY(swig)
ENDIF(UNIX)
