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
#*****************************************************************************

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

visit_append_list(
    NAME avtexpressions_ser_DEPENDS
    ITEMS visitcommon avtdbatts)


if(VISIT_PYTHON_FILTERS)
    visit_append_list(
        NAME avtexpressions_ser_INCLUDES
        ITEMS $<BUILD_INTERFACE:${PYTHON_INCLUDE_PATH}>)
endif()

set(avtexpressions_par_SOURCES  ${avtexpressions_ser_SOURCES})
set(avtexpressions_par_HEADERS  ${avtexpressions_ser_HEADERS})
set(avtexpressions_par_INCLUDES ${avtexpressions_ser_INCLUDES})
set(avtexpressions_par_DEPENDS  ${avtexpressions_ser_DEPENDS})

list(APPEND avtexpressions_ser_DEPENDS
    avtdatabase_ser
    avtdbin_ser
    avtplotter_ser
    avtfilters_ser
    avtpipeline_ser)

list(APPEND avtexpressions_par_DEPENDS
    avtdatabase_par
    avtdbin_par
    avtplotter_par
    avtfilters_par
    avtpipeline_par)

if(VISIT_PYTHON_FILTERS)
    list(APPEND avtexpressions_ser_DEPENDS avtpythonfilters_ser)
    list(APPEND avtexpressions_par_DEPENDS avtpythonfilters_par)
endif()

#********************************* SERIAL ************************************
visit_add_library(
    NAME       avtexpressions_ser
    SOURCES    ${avtexpressions_ser_SOURCES}
    HEADERS    ${avtexpressions_ser_HEADERS}
    INCLUDES   ${avtexpressions_ser_INCLUDES}
    DEPENDS_ON ${avtexpressions_ser_DEPENDS})

#********************************* PARALLEL **********************************
if(VISIT_PARALLEL)
    visit_add_parallel_library(
        NAME       avtexpressions_par
        SOURCES    ${avtexpressions_par_SOURCES}
        HEADERS    ${avtexpressions_par_HEADERS}
        INCLUDES   ${avtexpressions_par_INCLUDES}
        DEPENDS_ON ${avtexpressions_par_DEPENDS})
endif()

