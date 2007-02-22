/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtParallelAxisFilter.h                       //
// ************************************************************************* //

#ifndef AVT_PARALLEL_AXIS_FILTER_H
#define AVT_PARALLEL_AXIS_FILTER_H


#include <ParallelAxisAttributes.h>
#include <avtDataTreeStreamer.h>

#include <PortableFont.h>

#include <vector>
#include <string>

#define PCP_MIN_COORDINATE_DIMENSIONS     2
#define PCP_MAX_COORDINATE_DIMENSIONS     120
#define PCP_MIN_TICK_MARK_INTERVALS       2
#define PCP_MAX_TICK_MARK_INTERVALS       100

#define PCP_DEFAULT_COORDINATE_DIMENSIONS 3
#define PCP_DEFAULT_TICK_MARK_INTERVALS   10

#define PCP_DEFAULT_DATA_CURVE_COLOR      0x808080ff
#define PCP_DEFAULT_AXIS_COLOR            0x00c0c0ff
#define PCP_DEFAULT_AXIS_TITLE_COLOR      0x0000ffff
#define PCP_DEFAULT_RANGE_BOUND_COLOR     0xff80c0ff

#define PCP_ALTERNATE_DATA_CURVE_COLOR    0xc0c0c0ff

#define PCP_LEFT_AXIS_X_FRACTION          0.04
#define PCP_RIGHT_AXIS_X_FRACTION         0.96
#define PCP_H_BOTTOM_AXIS_Y_FRACTION      0.09
#define PCP_V_BOTTOM_AXIS_Y_FRACTION      0.12
#define PCP_H_TOP_AXIS_Y_FRACTION         0.93
#define PCP_V_TOP_AXIS_Y_FRACTION         0.88

#define PCP_H_TITLE_Y_FRACTION            0.01
#define PCP_V_TITLE_Y_FRACTION            0.11
#define PCP_V_TITLE_X_OFFSET_FRACTION     0.0025
#define PCP_BOTTOM_H_LABEL_Y_FRACTION     0.05
#define PCP_BOTTOM_V_LABEL_X_OFF_FRAC    -0.0175
#define PCP_TOP_H_LABEL_Y_FRACTION        0.96
#define PCP_TOP_V_LABEL_Y_FRACTION        0.89
#define PCP_TOP_V_LABEL_X_OFF_FRAC_1      0.0075
#define PCP_TOP_V_LABEL_X_OFF_FRAC_2      0.0175
#define PCP_BOTTOM_H_BOUND_Y_FRACTION     0.07
#define PCP_TOP_H_BOUND_Y_FRACTION        0.94
#define PCP_TOP_V_BOUND_X_OFF_FRAC       -0.0025
#define PCP_VERTICAL_MOVE_OFFSET_FRACTION 0.02

#define PCP_H_TITLE_CHAR_WIDTH_FRACTION   0.0112
#define PCP_H_TITLE_CHAR_HEIGHT_FRACTION  0.016
#define PCP_V_TITLE_CHAR_WIDTH_FRACTION   0.0084
#define PCP_V_TITLE_CHAR_HEIGHT_FRACTION  0.012
#define PCP_LABEL_CHAR_WIDTH_FRACTION     0.0084
#define PCP_LABEL_CHAR_HEIGHT_FRACTION    0.012
#define PCP_BOUND_CHAR_WIDTH_FRACTION     0.0084
#define PCP_BOUND_CHAR_HEIGHT_FRACTION    0.012

#define PCP_TICK_MARK_LENGTH_FRACTION     0.01
#define PCP_SELECTOR_ARROW_WIDTH_FRACTION 0.03

#define PCP_MAX_HORIZONTAL_TITLE_AXES     7


class vtkDataSet;
class vtkPolyData;
class vtkCellArray;
class vtkPoints;


// ****************************************************************************
//  Class: avtParallelAxisFilter
//
//  Purpose: A filter that outputs plot data for the ParallelAxis plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//      Mark Blair, Thu Jun  8 17:18:00 PDT 2006
//      Added code from a more general "parallel coordinate plot" package
//      developed earlier, to remove dependency on that package.
//
//      Kathleen Bonnell, Wed Oct 18 10:59:46 PDT 2006 
//      Removed unnecessary FILTER_EXPORTS specification so that filter will
//      build on Windows.
//      
//      Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//      Removed scratch file method, which is no longer necessary.  Also added
//      support for non-uniform axis spacing.
//
//      Mark Blair, Wed Nov  8 16:01:27 PST 2006
//      Changed InitializeDataTupleInput API.
//
// ****************************************************************************

class avtParallelAxisFilter : public avtDataTreeStreamer
{
public:
                                avtParallelAxisFilter(
                                    ParallelAxisAttributes &atts);
    virtual                    ~avtParallelAxisFilter();

    void                        ShouldCreateLabels(bool b) { return; }
    virtual const char         *GetType(void)
                                    { return "avtParallelAxisFilter"; };
    virtual const char         *GetDescription(void)
                                    { return "Parallel axis plot"; };
    virtual void                ReleaseData(void);

protected:
    virtual avtPipelineSpecification_p
                                PerformRestriction(avtPipelineSpecification_p);
    virtual avtDataTree_p       ExecuteDataTree(vtkDataSet *, int, string);

    virtual void                RefashionDataObjectInfo(void);
    virtual void                PreExecute(void);
    virtual void                PostExecute(void);
    virtual void                VerifyInput(void);

    void                        CreateLabels(void);

private:
    void                        SetupParallelAxis (int plotAxisNum);
    void                        ComputeCurrentDataExtentsOverAllDomains();
    void                        StoreAxisAttributesForOutsideQueries();

    void                        InitializePlotAtts();
    void                        InitializeDataTupleInput(bool drawAxisLabels);
    void                        InitializeOutputDataSets();
    void                        InputDataTuple(const floatVector &inputTuple);

    void                        DrawDataCurves();
    void                        DrawCoordinateAxes();
    void                        DrawCoordinateAxisLabels();
    void                        DrawCoordinateAxisTitles();
    void                        DrawDataSubrangeBounds();

    ParallelAxisAttributes      parAxisAtts;

    int                         parallelRank;
    bool                        drewAnnotations;

    stringVector                layerLabels;

    int                         domainCount;

    intVector                   varTupleIndices;
    
    doubleVector                axisAttsArray;

    int                         axisCount;
    int                         tickMarkIntervals;

    double                      bottomAxisY;
    double                      topAxisY;

    bool                        useVerticalText;

    doubleVector                plotAxisMinima;
    doubleVector                plotAxisMaxima;
    doubleVector                subrangeMinima;
    doubleVector                subrangeMaxima;

    stringVector                plotAxisTitles;
    
    boolVector                  applySubranges;

    double                      axisTitleY;
    double                      movedAxisTitleY;
    double                      axisTitleXOff;
    double                      bottomLabelY;
    double                      bottomLabelXOff;
    double                      topLabelY;
    double                      movedTopLabelY;
    double                      topLabelXOff1;
    double                      topLabelXOff2;
    double                      bottomBoundY;
    double                      bottomBoundXOff;
    double                      topBoundY;
    double                      topBoundXOff;

    double                      titleCharWidth;
    double                      titleCharHeight;
    double                      labelCharWidth;
    double                      labelCharHeight;
    double                      boundCharWidth;
    double                      boundCharHeight;

    double                      halfTickLength;
    double                      halfArrowWidth;
    double                      arrowHeight;

    std::vector<doubleVector>   dataTransforms;
    doubleVector                tickMarkOrdinates;

    boolVector                  drawBottomLabels;
    boolVector                  drawBottomBounds;
    boolVector                  drawTopBounds;
    boolVector                  moveTitles;
    boolVector                  moveTopLabels;

    int                         outputCurveCount;

    vtkPolyData                *dataCurvePolyData;
    vtkPoints                  *dataCurvePoints;
    vtkCellArray               *dataCurveLines;
    vtkCellArray               *dataCurveVerts;

    vtkPolyData                *axisPolyData;
    vtkPoints                  *axisPoints;
    vtkCellArray               *axisLines;
    vtkCellArray               *axisVerts;

    vtkPolyData                *titlePolyData;
    vtkPoints                  *titlePoints;
    vtkCellArray               *titleLines;
    vtkCellArray               *titleVerts;

    vtkPolyData                *subrangePolyData;
    vtkPoints                  *subrangePoints;
    vtkCellArray               *subrangeLines;
    vtkCellArray               *subrangeVerts;

    PortableFont               *textPlotter;
};


#endif
