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
#   Kathleen Biagas, Wed Oct 22 14:20:27 MST 2014
#   Send 'diagnostics/osmesatest' type argument to VISIT_TOOLS_ADD_FOLDER.
#
#****************************************************************************/

SET(TARGETS osmesatest_ser osmesavtktest_ser)

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_BINARY_DIR}/include
${VISIT_SOURCE_DIR}/include
${MESA_INCLUDE_DIR}
${VISIT_SOURCE_DIR}/visit_vtk/osmesa
${VTK_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
${LIBRARY_OUTPUT_DIRECTORY}
)

SET(REQUIRED_VTK_LIBS
vtkCommonCore 
vtkCommonDataModel
vtkCommonExecutionModel
vtkFiltersGeometry 
vtkFiltersHybrid
vtkFiltersSources
vtkIOImage
vtkRenderingCore
vtkRenderingOpenGL
vtkRenderingFreeType 
vtkRenderingFreeTypeOpenGL
vtkInteractionStyle 
)

IF(NOT WIN32)
    SET(LIBDL dl)
    SET(PTHREAD pthread)
ENDIF(NOT WIN32)

ADD_EXECUTABLE(osmesatest_ser osmesatest.cpp)
TARGET_LINK_LIBRARIES(osmesatest_ser ${MESA_LIBRARY_DIR}/${MESA_LIB} ${LIBDL} ${PTHREAD})

ADD_EXECUTABLE(osmesavtktest_ser osmesavtktest.cpp)
TARGET_LINK_LIBRARIES(osmesavtktest_ser visit_vtk_osmesa visitcommon ${REQUIRED_VTK_LIBS} ${LIBDL} ${PTHREAD})

IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(osmesatest_par osmesatest.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(osmesatest_par ${MESA_LIBRARY_DIR}/${MESA_LIB} ${LIBDL} ${PTHREAD})

    ADD_PARALLEL_EXECUTABLE(osmesavtktest_par osmesavtktest.cpp)
    PARALLEL_EXECUTABLE_LINK_LIBRARIES(osmesavtktest_par visit_vtk_osmesa visitcommon ${REQUIRED_VTK_LIBS} ${LIBDL} ${PTHREAD})

    SET(TARGETS ${TARGETS} osmesatest_par osmesavtktest_par)
ENDIF(VISIT_PARALLEL)

VISIT_INSTALL_TARGETS(${TARGETS})
VISIT_TOOLS_ADD_FOLDER(diagnostics/osmesatest ${TARGETS})
