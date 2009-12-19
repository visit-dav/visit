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
#
#****************************************************************************/

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
${VISIT_SOURCE_DIR}/avt/DDF
${VISIT_SOURCE_DIR}/avt/Database/Database
${VISIT_SOURCE_DIR}/avt/Database/Ghost
${VISIT_SOURCE_DIR}/avt/Expressions/Abstract
${VISIT_SOURCE_DIR}/avt/Expressions/CMFE
${VISIT_SOURCE_DIR}/avt/Expressions/Conditional
${VISIT_SOURCE_DIR}/avt/Expressions/Derivations
${VISIT_SOURCE_DIR}/avt/Expressions/General
${VISIT_SOURCE_DIR}/avt/Expressions/ImageProcessing
${VISIT_SOURCE_DIR}/avt/Expressions/Management
${VISIT_SOURCE_DIR}/avt/Expressions/Math
${VISIT_SOURCE_DIR}/avt/Expressions/MeshQuality
${VISIT_SOURCE_DIR}/avt/FileWriter
${VISIT_SOURCE_DIR}/avt/Filters
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_SOURCE_DIR}/avt/Pipeline/Data
${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
${VISIT_SOURCE_DIR}/avt/Plotter
${VISIT_SOURCE_DIR}/avt/Queries/Abstract
${VISIT_SOURCE_DIR}/avt/Queries/Misc
${VISIT_SOURCE_DIR}/avt/Queries/Pick
${VISIT_SOURCE_DIR}/avt/Queries/Queries
${VISIT_SOURCE_DIR}/avt/View
${VISIT_SOURCE_DIR}/avt/VisWindow/Colleagues
${VISIT_SOURCE_DIR}/avt/VisWindow/Proxies
${VISIT_SOURCE_DIR}/avt/VisWindow/Tools
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${VTK_INCLUDE_DIRS}
)

# Add link directories
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH} ${MESA_LIBRARY_DIR} ${GLEW_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

SET(VTK_EXTRA_LIBS
vtkjpeg 
vtkpng 
vtkexpat
vtkzlib 
vtkDICOMParser 
vtkMPEG2Encode 
vtkfreetype 
vtkftgl
)

#********************************* SERIAL ************************************
ADD_EXECUTABLE(convert_ser convert.C)
VISIT_INSTALL_TARGETS(convert_ser)

IF(VISIT_DBIO_ONLY)
    SET_TARGET_PROPERTIES(convert_ser PROPERTIES OUTPUT_NAME convert_ser_lite)
    TARGET_LINK_LIBRARIES(convert_ser 
        avtdatabase_ser
        avtdbatts
        avtpipeline_ser
        lightweight_visit_vtk 
        visitcommon
        visit_vtk 
        ${VTK_EXTRA_LIBS}
        ${MESA_LIBS}
        ${GL_LIBS}
        ${CMAKE_THREAD_LIBS} 
        ${ZLIB_LIB}
    )
ELSE(VISIT_DBIO_ONLY)
    TARGET_LINK_LIBRARIES(convert_ser 
        avtdatabase_ser
        avtdbatts
        avtddf_ser 
        avtexpressions_ser
        avtfilters_ser 
        avtivp
        avtmath
        avtmir_ser
        avtpipeline_ser
        avtplotter_ser 
        avtview 
        lightweight_visit_vtk 
        visitcommon
        visit_verdict  
        visit_vtk 
        ${VTK_EXTRA_LIBS}
        ${MESA_LIBS}
        ${GL_LIBS}
        ${CMAKE_THREAD_LIBS} 
        ${ZLIB_LIB}
    )
ENDIF(VISIT_DBIO_ONLY)

#********************************* PARALLEL **********************************
IF(VISIT_PARALLEL)
    ADD_PARALLEL_EXECUTABLE(convert_par convert.C)
    VISIT_INSTALL_TARGETS(convert_par)

    IF(VISIT_DBIO_ONLY)
        SET_TARGET_PROPERTIES(convert_par PROPERTIES OUTPUT_NAME convert_par_lite)
        TARGET_LINK_LIBRARIES(convert_par 
            avtdatabase_par
            avtdbatts
            avtpipeline_par
            lightweight_visit_vtk 
            visitcommon
            visit_verdict  
            visit_vtk 
            ${VTK_EXTRA_LIBS}
            ${MESA_LIBS}
            ${GL_LIBS}
            ${CMAKE_THREAD_LIBS} 
            ${ZLIB_LIB} 
        )
    ELSE(VISIT_DBIO_ONLY)
        TARGET_LINK_LIBRARIES(convert_par 
            avtdatabase_par
            avtdbatts
            avtddf_par 
            avtexpressions_par
            avtfilters_par 
            avtivp
            avtmath
            avtmir_par
            avtpipeline_par
            avtplotter_par 
            avtview 
            lightweight_visit_vtk 
            visitcommon
            visit_verdict  
            visit_vtk 
            ${VTK_EXTRA_LIBS}
            ${MESA_LIBS}
            ${GL_LIBS}
            ${CMAKE_THREAD_LIBS} 
            ${ZLIB_LIB} 
        )
    ENDIF(VISIT_DBIO_ONLY)
ENDIF(VISIT_PARALLEL)
