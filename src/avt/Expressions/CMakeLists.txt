# Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
# Project developers.  See the top-level LICENSE file for dates and other
# details.  No copyright assignment is required to contribute to VisIt.

#*****************************************************************************
# Modifications:
#    Cyrus Harrison,Tue Feb  2 16:35:18 PST 2010
#    Added the python filter expression.
#
#    Cyrus Harrison, Wed Mar 10 14:28:33 PST 2010   
#    Use proper python include path ${PYTHON_INCLUDE_PATH}, instead of 
#    ${PYTHON_INCLUDE_DIRS}
#
#    Hank Childs, Thu May 20 21:26:47 PDT 2010
#    Add Base10LogWithMin.
#
#    Hank Childs, Mon Jun 28 06:43:20 PDT 2010
#    Add CoordinateExtrema.
#
#    Hank Childs, Sat Aug 21 14:05:14 PDT 2010
#    Rename ApplyDDFExpression to ApplyDataBinningExpression.
#
#    Hank Childs, Thu Sep 23 16:00:56 PDT 2010
#    Added avtDeferExpressionBaseFilter.
#
#    Hank Childs, Fri Feb  4 13:46:18 PST 2011
#    Rename avtExternalNodeExpression to avtFindExternalExpression.
#
#    Eric Brugger, Mon Aug 20 08:59:10 PDT 2012
#    Added avtCurveIntegrateExpression.
#
#    Eric Brugger, Mon Aug 27 11:32:43 PDT 2012
#    Added avtCurveSwapXYExpression.
#
#    Brad Whitlock, Wed Sep 12 17:16:29 PDT 2012
#    I added the bin expression.
#
#    Matthew Wheeler, Mon 20 May 12:00:00 GMT 2013
#    Added avtMinCornerArea and avtMinSinCorner.C
#
#    Gunther H. Weber, Tue May 13 10:40:56 PDT 2014
#    Added array_sum.
#
#    Kevin Griffin, Tue Jul 29 10:08:40 PDT 2014
#    Added q_criterion.
#
#    Timo Bremer, Fri Oct 28 09:09:27 PDT 2016
#    Added merge tree and local threshold.
#
#    Eddie Rusu, Tue Sep 24 09:07:44 PDT 2019
#    Added multiple input math expressions and smart divide.
# 
#    Justin Privitera, Wed Mar 30 12:45:19 PDT 2022
#    Added avtGhostZoneIdExpression.
#
#    Kathleen Biagas, Wed June 15, 2022
#    Added General/avtCrackWidthExpression.C
# 
#    Justin Privitera, Fri Sep 16 11:58:19 PDT 2022
#    Added Derivations/avtRelativeVolumeExpression
#    Added Derivations/avtStrainVolumetricExpression
#    Renamed Derivations/avtStrainTensorExpression to
#    Derivations/avtStrainExpression
#
#    Kathleen Biagas, Tue Dec 13, 2022
#    Moved setting of sources to subdirs.
#
#    Eric Brugger, Fri Feb 24 14:57:15 PST 2023
#    I removed the reference to the vtkm include directories since I
#    removed all the code that depended on them.
#
#****************************************************************************/

add_library(avtexpressions_ser)
if(VISIT_PARALLEL)
    ADD_PARALLEL_LIBRARY(avtexpressions_par)
endif()

add_subdirectory(Abstract)
add_subdirectory(Conditional)
add_subdirectory(CMFE)
add_subdirectory(Derivations)
add_subdirectory(General)
add_subdirectory(ImageProcessing)
add_subdirectory(Management)
add_subdirectory(Math)
add_subdirectory(MeshQuality)
add_subdirectory(TimeIterators)

include_directories(
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${VISIT_SOURCE_DIR}/avt/DBAtts/MetaData
    ${VISIT_SOURCE_DIR}/avt/DBAtts/SIL
    ${VISIT_SOURCE_DIR}/avt/Database/Database
    ${VISIT_SOURCE_DIR}/avt/DataBinning
    ${VISIT_SOURCE_DIR}/avt/Filters
    ${VISIT_SOURCE_DIR}/avt/MIR/Base
    ${VISIT_SOURCE_DIR}/avt/MIR/Tet
    ${VISIT_SOURCE_DIR}/avt/MIR/Zoo
    ${VISIT_SOURCE_DIR}/avt/Pipeline/AbstractFilters
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Data
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Pipeline
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sinks
    ${VISIT_SOURCE_DIR}/avt/Pipeline/Sources
    ${VISIT_SOURCE_DIR}/avt/Plotter
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${VISIT_SOURCE_DIR}/vtkm
    ${VISIT_SOURCE_DIR}/third_party_builtin/verdict
    ${VTK_INCLUDE_DIRS})

if(VISIT_PYTHON_FILTERS)
    include_directories(
        ${PYTHON_INCLUDE_PATH}
        ${VISIT_SOURCE_DIR}/avt/PythonFilters
        ${VISIT_SOURCE_DIR}/visitpy/common/)
endif()

#********************************* SERIAL ************************************
target_link_libraries(avtexpressions_ser visitcommon avtdbatts avtdatabase_ser
    avtdbin_ser avtplotter_ser avtfilters_ser avtpipeline_ser)
if(VISIT_PYTHON_FILTERS)
    target_link_libraries(avtexpressions_ser avtpythonfilters_ser)
endif()
VISIT_INSTALL_TARGETS(avtexpressions_ser)

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    target_link_libraries(avtexpressions_par visitcommon avtdbatts
        avtdatabase_par avtdbin_par avtplotter_par avtfilters_par
        avtpipeline_par)
    if(VISIT_PYTHON_FILTERS)
        target_link_libraries(avtexpressions_par avtpythonfilters_par)
    endif()
    VISIT_INSTALL_TARGETS(avtexpressions_par)
endif()

