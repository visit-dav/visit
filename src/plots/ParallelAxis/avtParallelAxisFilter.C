/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                              avtParallelAxisFilter.C                      //
// ************************************************************************* //

#include <avtParallelAxisFilter.h>

#include <PlotInfoAttributes.h>

#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <visitstream.h>

#include <sys/types.h>

#include <vector>
#include <string>

#include <vtkDataSet.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkVisItScalarTree.h>

#include <avtDataAttributes.h>
#include <avtExtents.h>
#include <avtDataTree.h>
#include <avtIntervalTree.h>
#include <avtMetaData.h>
#include <avtParallel.h>

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidLimitsException.h>
#include <NoDefaultVariableException.h>
#include <InvalidDimensionsException.h>
#include <TimingsManager.h>


// ****************************************************************************
//  Method: avtParallelAxisFilter
//
//  Purpose: Constructor for the avtParallelAxisFilter class.
//
//  Arguments:
//      atts    The ParallelAxis plot attributes.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Added initialization of parallel processor partition size.
//
// ****************************************************************************

avtParallelAxisFilter::avtParallelAxisFilter(ParallelAxisAttributes &atts)
{
    parAxisAtts = atts;

    processorRank = PAR_Rank();
    partitionSize = PAR_Size();
}


// ****************************************************************************
//  Method: ~avtParallelAxisFilter
//
//  Purpose: Destructor for the avtParallelAxisFilter class.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

avtParallelAxisFilter::~avtParallelAxisFilter()
{
    return;
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::VerifyInput
//
//  Purpose: Verifies that the input is 3D data, throws an exception if not.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now accepts input data of any dimension.
//
// ****************************************************************************
 
void
avtParallelAxisFilter::VerifyInput(void)
{
/*
    if  (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "ParallelAxis", " 3-D ");
    }
 */
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::ModifyContract
//
//  Purpose: Restrict input domains if an interval tree is available.  Also set
//           up axis position information needed by the plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//      Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//      Added check for attribute consistency.
//
// ****************************************************************************

avtContract_p
avtParallelAxisFilter::ModifyContract(avtContract_p in_spec)
{
    if (!parAxisAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PR1: ParallelAxis plot attributes are inconsistent."
               << endl;

        return in_spec;
    }
        
    const char *inPipelineVar = in_spec->GetDataRequest()->GetVariable();
    std::string curPipelineVar(inPipelineVar);
    
    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    doubleVector curAxisMinima = parAxisAtts.GetAxisMinima();

    varTupleIndices.clear();

    for (int axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if (curAxisVarNames[axisNum] == curPipelineVar)
            varTupleIndices.push_back(0);
        else
            varTupleIndices.push_back(-1);
    }

    avtContract_p outSpec = new avtContract(in_spec);
    
    outSpec->NoDynamicLoadBalancing();
    
/* Add this interval tree code eventually (mb)
    bool atLeastOneTree = false;
    avtIntervalTree *it;
    int varDomNum, curDomNum, curDomain;
    intVector varDomains;
    intVector curDomains;
    intVector outDomains;

    doubleVector curExtentMinima = parAxisAtts.GetExtentMinima();
    doubleVector curExtentMaxima = parAxisAtts.GetExtentMaxima();
    double extentMin, extentMax;

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        axisVarName = curAxisVarNames[axisNum];
        
        if ((it = GetMetaData()->GetDataExtents(axisVarName.c_str())) != NULL)
        {
            extentMin = curExtentMinima[axisNum];
            extentMax = curExtentMaxima[axisNum];

            if (atLeastOneTree)
            {
                it->GetElementsListFromRange(&extentMin, &extentMax, varDomains);

                for (curDomNum = 0; curDomNum < curDomains.size(); curDomNum++)
                {
                    if ((curDomain = curDomains[curDomNum]) != -1)
                    {
                        for (varDomNum=0; varDomNum<varDomains.size(); varDomNum++)
                        {
                            if (varDomains[varDomNum] == curDomain) break;
                        }

                        if (varDomNum >= varDomains.size())
                        {
                            curDomains[curDomNum] = -1;
                        }
                    }
                }
            }
            else
            {
                it->GetElementsListFromRange(&extentMin, &extentMax, curDomains);
                atLeastOneTree = true;
            }
        }
    }

    if (atLeastOneTree)
    {
        for (curDomNum = 0; curDomNum < curDomains.size(); curDomNum++)
        {
            if (curDomains[curDomNum] != -1)
            {
                outDomains.push_back(curDomains[curDomNum]);
            }
        }

        outSpec->GetDataRequest()->GetRestriction()->RestrictDomains(outDomains);
    }
*/

    return outSpec;
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::PreExecute
//
//  Purpose: Computes the data extents of all relevant scalar variables.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//     Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//     No longer writes scratch file, which is no longer necessary.  Also added
//     support for non-uniform axis spacing.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
//     Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//     Create colormap labels from both the datacurve/annotation labels and
//     the context labels.  Also initialize the context accumulation bins.
//
//     Mark Blair, Thu Jul  5 19:06:33 PDT 2007
//     Axis layout and labeling are now always calculated in the viewer.  Code
//     that did this here was removed.
//
// *****************************************************************************

void
avtParallelAxisFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    if (!parAxisAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PE1: ParallelAxis plot attributes are inconsistent."
               << endl;
        
        sendNullOutput = true;
        return;
    }

    intVector curAxisFlagSets = parAxisAtts.GetAxisInfoFlagSets();
        
    axisCount = curAxisFlagSets.size();
    sendNullOutput = false;

    ComputeCurrentDataExtentsOverAllDomains();
    
    parAxisAtts.DetermineAxisBoundsAndGroupNames();
    
    if (sendNullOutput) return;
    
    int axisNum, axisInfoFlagSet;

    //
    // Left and right plot axes and selected axes needed later.  Perform sanity
    // check while computing them.
    //
    if ((parAxisAtts.GetPlotToolModeFlags() & EA_TOOL_DRAWS_AXIS_INFO_FLAG) == 0)
    {
        leftPlotAxisID     = 0; rightPlotAxisID     = axisCount - 1;
        leftSelectedAxisID = 0; rightSelectedAxisID = axisCount - 1;
    }
    else
    {
        leftPlotAxisID     = -1; rightPlotAxisID     = -1;
        leftSelectedAxisID = -1; rightSelectedAxisID = -1;
    
        for (axisNum = 0; axisNum < axisCount; axisNum++)
        {
            axisInfoFlagSet = curAxisFlagSets[axisNum];
        
            if ((axisInfoFlagSet & EA_LEFT_SHOWN_AXIS_FLAG) != 0)
                leftPlotAxisID = axisNum;
            if ((axisInfoFlagSet & EA_RIGHT_SHOWN_AXIS_FLAG) != 0)
                rightPlotAxisID = axisNum;
        
            if ((axisInfoFlagSet & EA_LEFT_SELECTED_AXIS_FLAG) != 0)
                leftSelectedAxisID = axisNum;
            if ((axisInfoFlagSet & EA_RIGHT_SELECTED_AXIS_FLAG) != 0)
                rightSelectedAxisID = axisNum;
        }
    
        if (leftPlotAxisID  == -1) sendNullOutput = true;
        if (rightPlotAxisID == -1) sendNullOutput = true;

        if (leftPlotAxisID >= rightPlotAxisID) sendNullOutput = true;
    
        if (leftSelectedAxisID  == -1) sendNullOutput = true;
        if (rightSelectedAxisID == -1) sendNullOutput = true;

        if (leftSelectedAxisID >= rightSelectedAxisID) sendNullOutput = true;
        
        if (leftSelectedAxisID  <  leftPlotAxisID) sendNullOutput = true;
        if (rightSelectedAxisID > rightPlotAxisID) sendNullOutput = true;
    }

    if (sendNullOutput)
    {
        debug3 << "PCP/aPAF/PE2: ParallelAxis plot shown/selected axis marks "
               << "missing or out of order." << endl;
        return;
    }

    useVerticalText =
        ((parAxisAtts.GetPlotToolModeFlags() & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) != 0);
    
    drewAnnotations = false;

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
    CreateLabels();

    // We need to create the colormap labels from the line, annotation,
    // and context labels.  This ordering must match what's created
    // in the avtParallelAxisPlot, and the intent is that the context
    // is drawn underneath the annotations and individual lines.
    stringVector colorMapLabels;
    colorMapLabels.insert(colorMapLabels.end(),
                          contextLabels.begin(),contextLabels.end());
    colorMapLabels.insert(colorMapLabels.end(),
                          curveAndAxisLabels.begin(),curveAndAxisLabels.end());
    outAtts.SetLabels(colorMapLabels);
    
    // initialize things needed for drawing the context
    if (parAxisAtts.GetDrawContext())
    {
        InitializePairwiseBins();
    }
}


// *****************************************************************************
//  Method: avtParallelAxisFilter::PostExecute
//
// Purpose: Changes the vis window's spatial extents to match the viewport of
//          the plot's curves.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Axis attribute for outside queries now stored after filter is executed.
//
//     Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//     Draw the context, and clean up.
//
// ****************************************************************************

void
avtParallelAxisFilter::PostExecute(void)
{
    avtDatasetToDatasetFilter::PostExecute();

    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
    outAtts.GetTrueSpatialExtents()->Clear();
    outAtts.GetCumulativeTrueSpatialExtents()->Clear();

    double spatialExtents[6];

    if (inAtts.GetTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetTrueSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[0] = 0.0; spatialExtents[1] = 1.0;
        spatialExtents[2] = 0.0; spatialExtents[3] = 1.0;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }
    else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[0] = 0.0; spatialExtents[1] = 1.0;
        spatialExtents[2] = 0.0; spatialExtents[3] = 1.0;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }

    outAtts.SetXLabel("");
    outAtts.SetYLabel("");

    outAtts.SetXUnits("");
    outAtts.SetYUnits("");

    StoreAxisAttributesForOutsideQueries();

    if (parAxisAtts.GetDrawContext())
    {
        DrawContext();
        CleanUpPairwiseBins();
    }
}

// ****************************************************************************
//  Method: avtParallelAxisFilter::ExecuteDataTree
//
//  Purpose: Processes one domain chunk of the input mesh into corresponding
//           curves of a ParallelAxis plot.
//
//  Arguments:
//      in_ds  :  The input dataset.
//      domain :  The domain number of the input dataset.
//
//  Returns: The output dataset (curves of the ParallelAxis plot).
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
//     Mark Blair, Fri Jun  9 14:29:00 PDT 2006
//     Rewrote to eliminate use of the ParallelCoordinatePlot class.  Code
//     adapted from that class was added directly to the filter code.
//
//     Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//     Added check for attribute consistency.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
//     Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//     Count each tuple in the appropriate pairwise axis bins.
//     Avoid drawing data curves (lines) if we didn't ask for them.
//     Renamed data set labels specific to data curves and axis annotations.
//
//     Jeremy Meredith, Wed Mar 21 18:22:08 EDT 2007
//     Added a new setting which will hide the lines until the extents
//     tool has actually been enabled and thresholded the range of one or
//     more axes.  This allows the "lines" to be used strictly as a "focus".
//
//     Jeremy Meredith, Fri May 18 09:41:30 EDT 2007
//     Only add the individual tuples to the data sets if the lines are
//     actually going to be drawn.  This lets situtions where only the context
//     is drawn scale easily to huge datasets without going into SR mode.
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Now examines extents themselves to determine whether extents are applied.
//     Custom renderer changes made "applySubranges" flags invalid for this.
//
// ****************************************************************************

avtDataTree_p 
avtParallelAxisFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (sendNullOutput) return NULL;

    if (in_ds == NULL)
    {
        debug3 << "PCP/aPAF/EDT/1: ParallelAxis plot input is NULL." << endl;
        return NULL;
    }

    if (varTupleIndices.size() != axisCount)
    {
        debug3 << "PCP/aPAF/EDT/2: ParallelAxis plot internal data is "
               << "inconsistent." << endl;
        return NULL;
    }

    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    
    int plotToolModeFlags = parAxisAtts.GetPlotToolModeFlags();
    bool drawLabelsAndTitles =
        ((plotToolModeFlags & EA_TOOL_DRAWS_AXIS_INFO_FLAG) == 0);
    bool selectedVarsOnly =
        ((plotToolModeFlags & EA_THRESHOLD_SELECTED_ONLY_FLAG) != 0);

    int cellArrayCount  = in_ds->GetCellData()->GetNumberOfArrays();
    int pointArrayCount = in_ds->GetPointData()->GetNumberOfArrays();
    int cellCount       = in_ds->GetNumberOfCells();
    int pointCount      = in_ds->GetNumberOfPoints();

    bool plotCellData  = (cellArrayCount > 0);
    int plotTupleCount = (plotCellData) ? cellCount : pointCount;

    if (cellArrayCount + pointArrayCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/3: No ParallelAxis plot input data arrays found."
               << endl;
        return NULL;
    }

    if (plotTupleCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/4: ParallelAxis plot input data array "
               << "is of zero length." << endl;
        return NULL;
    }
    
    // If the input contains both cell data and point data, then by convention
    // the cell data takes precedence.  In this case, the value of a point
    // variable that is plotted for a given cell, by convention, is the average
    // value of that variable over all vertices of the cell.  (mb)

    const std::string pipeVariableName = pipelineVariable;

    int axisNum;
    int tupleCount, tupleNum, varTupleIndex, componentCount;
    int cellVertexCount, vertexNum, valueNum;
    bool arrayIsCellData, dataBadOrMissing;
    std::string arrayName;
    vtkDataArray *dataArray;
    vtkIdList *pointIdList;
    float *arrayValues;
    float valueSum;
    
    std::vector<float *> varArrayValues;
    boolVector           varIsCellData;
    intVector            varTupleCompIDs;
    intVector            varComponentCounts;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if ((varTupleIndex = varTupleIndices[axisNum]) >= 0)
            arrayName = pipeVariableName;
        else
        {
            arrayName = curAxisVarNames[axisNum];
            varTupleIndex = 0;
        }

        dataArray = in_ds->GetCellData()->GetArray(arrayName.c_str());
        arrayIsCellData = true; tupleCount = cellCount;

        if (dataArray == NULL)
        {
            dataArray = in_ds->GetPointData()->GetArray(arrayName.c_str());
            arrayIsCellData = false; tupleCount = pointCount;
        }

        if (dataArray == NULL)
            dataBadOrMissing = true;
        else if (dataArray->GetDataType() != VTK_FLOAT)
            dataBadOrMissing = true;
        else if (dataArray->GetNumberOfTuples() != tupleCount)
            dataBadOrMissing = true;
        else
            dataBadOrMissing = false;
            
        if (dataBadOrMissing)
        {
            debug3 << "PCP/aPAF/EDT/5: ParallelAxis plot input data array "
                   << arrayName << " is bad or missing." << endl;
            return NULL;
        }
    
        varArrayValues.push_back((float *)dataArray->GetVoidPointer(0));
        varIsCellData.push_back(arrayIsCellData);
        varTupleCompIDs.push_back(varTupleIndex);
        varComponentCounts.push_back(dataArray->GetNumberOfComponents());
    }

    InitializePlotAtts();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        SetupParallelAxis(axisNum);
    }

    floatVector inputTuple = floatVector(axisCount);
    
    InitializeDataTupleInput(drawLabelsAndTitles, selectedVarsOnly);
    InitializeOutputDataSets();
    
    const doubleVector curExtentMinima = parAxisAtts.GetExtentMinima();
    const doubleVector curExtentMaxima = parAxisAtts.GetExtentMaxima();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (curExtentMinima[axisNum] != 0.0) break;
        if (curExtentMaxima[axisNum] != 1.0) break;
    }

    bool drawLines = parAxisAtts.GetDrawLines();
    bool onlyIfExtentsOn = parAxisAtts.GetDrawLinesOnlyIfExtentsOn();

    if (drawLabelsAndTitles)
    {
        if (onlyIfExtentsOn) drawLines = false;
    }
    else
    {
        if (onlyIfExtentsOn && (axisNum >= axisCount)) drawLines = false;
    }

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList = vtkIdList::New();
    }

    if (plotCellData)
    {
        for (tupleNum = 0; tupleNum < cellCount; tupleNum++)
        {
            for (axisNum = 0; axisNum < axisCount; axisNum++)
            {
                arrayValues     = varArrayValues[axisNum];
                varTupleIndex   = varTupleCompIDs[axisNum];
                componentCount  = varComponentCounts[axisNum];

                if (varIsCellData[axisNum])
                {
                    inputTuple[axisNum] =
                    arrayValues[tupleNum*componentCount + varTupleIndex];
                }
                else
                {
                    in_ds->GetCellPoints(tupleNum, pointIdList);
                    cellVertexCount = pointIdList->GetNumberOfIds();

                    valueSum = 0.0;

                    for (vertexNum = 0; vertexNum < cellVertexCount; vertexNum++)
                    {
                        valueNum = pointIdList->GetId(vertexNum)*componentCount +
                                   varTupleIndex;
                        valueSum += arrayValues[valueNum];
                    }

                    if (cellVertexCount == 0) cellVertexCount = 1;

                    inputTuple[axisNum] = valueSum / (float)cellVertexCount;
                }
            }
            
            if (drawLines)
                InputDataTuple(inputTuple);
            CountDataTuple(inputTuple);
        }
    }
    else
    {
        for (tupleNum = 0; tupleNum < pointCount; tupleNum++)
        {
            for (axisNum = 0; axisNum < axisCount; axisNum++)
            {
                arrayValues     = varArrayValues[axisNum];
                varTupleIndex   = varTupleCompIDs[axisNum];
                componentCount  = varComponentCounts[axisNum];

                inputTuple[axisNum] =
                arrayValues[tupleNum*componentCount + varTupleIndex];
            }
            
            if (drawLines)
                InputDataTuple(inputTuple);
            CountDataTuple(inputTuple);
        }
    }

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList->Delete();
    }

    if (drawLines)
    {
        DrawDataCurves();
    }

    if (processorRank == 0)
    {
        if (!drewAnnotations)
        {
            DrawCoordinateAxes();
            if (drawLabelsAndTitles) DrawCoordinateAxisLabels();
            if (drawLabelsAndTitles) DrawCoordinateAxisTitles();

            drewAnnotations = true;
        }
    }

    vtkDataSet **outputDataSets = new vtkDataSet *[4];

    outputDataSets[0] = dataCurvePolyData;
    outputDataSets[1] = axisPolyData;
    outputDataSets[2] = labelPolyData;
    outputDataSets[3] = titlePolyData;

    avtDataTree *outputDataTree =
        new avtDataTree(4, outputDataSets, domain, curveAndAxisLabels);
        
    for (int dataSetNum = 0; dataSetNum < 4; dataSetNum++)
        outputDataSets[dataSetNum]->Delete();

    delete [] outputDataSets;

    return outputDataTree;
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::UpdateDataObjectInfo
//
//  Purpose: Indicates that the topological dimension of the output is not the
//           same as the input.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelAxisFilter::UpdateDataObjectInfo(void)
{
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    GetOutput()->GetInfo().GetValidity().InvalidateZones();

    outAtts.SetSpatialDimension(2);
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::CreateLabels
//
//  Purpose: Create the labels to be used for the data set arrays.
//           These are also concatenated into colormap labels elsewhere
//           in this filter.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Mar 16 13:54:09 EDT 2007
//    Also create labels for the context data sets.  Renamed the old labels
//    so it's clear they are specific to the data curves and axes annotations.
//
// ****************************************************************************

void
avtParallelAxisFilter::CreateLabels()
{
    curveAndAxisLabels.clear();

    curveAndAxisLabels.push_back("Data Curves");
    curveAndAxisLabels.push_back("Axes");
    curveAndAxisLabels.push_back("Axis Bounds");
    curveAndAxisLabels.push_back("Axis Titles");

    char str[100];
    contextLabels.clear();
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
    {
        sprintf(str, "Data Context %03d", i);
        contextLabels.push_back(str);
    }
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::ReleaseData
//
//  Purpose: Release all problem-sized data associated with this filter.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelAxisFilter::ReleaseData(void)
{
    avtDataTreeStreamer::ReleaseData();
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::SetupParallelAxis
//
//  Purpose: Set up parallel axis plot data associated with one of plot's axes.
//
//  Notes: Adapted from more general "parallel coordinate plot" package developed
//         earlier.
//
//  Programmer: Mark Blair
//  Creation:   Thu Jun  8 17:18:00 PDT 2006
//
//  Modifications:
//
//     Mark Blair, Wed Dec 20 17:52:01 PST 2006
//     Added support for non-uniform axis spacing.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Removed the aesthetic 5% margin between data extrema and axis extrema.
//
// ****************************************************************************

void
avtParallelAxisFilter::SetupParallelAxis (int plotAxisNum)
{
    const stringVector curAxisVarNames   = parAxisAtts.GetOrderedAxisNames();
    const doubleVector curAxisXPositions = parAxisAtts.GetAxisXPositions();
    const doubleVector curAxisMinima     = parAxisAtts.GetAxisMinima();
    const doubleVector curAxisMaxima     = parAxisAtts.GetAxisMaxima();
    const doubleVector curExtentMinima   = parAxisAtts.GetExtentMinima();
    const doubleVector curExtentMaxima   = parAxisAtts.GetExtentMaxima();

    double plotAxisMin   = curAxisMinima[plotAxisNum];
    double plotAxisRange = curAxisMaxima[plotAxisNum] - plotAxisMin;

    plotAxisXPositions.push_back(curAxisXPositions[plotAxisNum]);

    plotAxisMinima.push_back(plotAxisMin);
    plotAxisMaxima.push_back(curAxisMaxima[plotAxisNum]);

    subrangeMinima.push_back(
        plotAxisMin + plotAxisRange*curExtentMinima[plotAxisNum]);
    subrangeMaxima.push_back(
        plotAxisMin + plotAxisRange*curExtentMaxima[plotAxisNum]);
    
    plotAxisTitles.push_back(curAxisVarNames[plotAxisNum]);
}


// *****************************************************************************
//  Method: avtParallelAxisPlot::ComputeCurrentDataExtentsOverAllDomains
//
//  Purpose: Computes extent of each axis's scalar variable.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// *****************************************************************************

void
avtParallelAxisFilter::ComputeCurrentDataExtentsOverAllDomains()
{
    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    
    if (varTupleIndices.size() != curAxisVarNames.size())
    {
        debug3 << "PCP/aPAF/CCDEOAD/1: ParallelAxis plot internal data is "
               << "inconsistent." << endl;
        sendNullOutput = true;
        return;
    }

    doubleVector curAxisMinima = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima = parAxisAtts.GetAxisMaxima();

    int axisNum;
    std::string axisVarName;
    double dataAverage;
    double *axisMinimum, *axisMaximum;
    double varDataExtent[2];

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisVarName = curAxisVarNames[axisNum];

        if (!TryDataExtents(varDataExtent, axisVarName.c_str()))
        {
            GetDataExtents(varDataExtent, axisVarName.c_str());
        }
                
        curAxisMinima[axisNum] = varDataExtent[0];
        curAxisMaxima[axisNum] = varDataExtent[1];
    }

    for (axisNum = 0; axisNum < curAxisMinima.size(); axisNum++)
    {
        axisMinimum = &curAxisMinima[axisNum];
        axisMaximum = &curAxisMaxima[axisNum];

        if (fabs(*axisMinimum) < 1e-20) *axisMinimum = 0.0;
        if (fabs(*axisMaximum) < 1e-20) *axisMaximum = 0.0;

        if (fabs(*axisMaximum-*axisMinimum) < 1e-20)
        {
            dataAverage = (*axisMinimum + *axisMaximum) * 0.5;
            *axisMinimum = dataAverage - 1e-20;
            *axisMaximum = dataAverage + 1e-20;
        }
    }
    
    parAxisAtts.SetAxisMinima(curAxisMinima);
    parAxisAtts.SetAxisMaxima(curAxisMaxima);
}


// *****************************************************************************
//  Method: avtParallelAxisPlot::StoreAxisAttributesForOutsideQueries
//
//  Purpose: Stores name of each axis's scalar variable and that variable's
//           extent in a data structure that can be queried by other parts of
//           VisIt.
//
//  Programmer: Mark Blair
//  Creation:   Fri Sep  1 14:51:00 PDT 2006
//
//  Modifications:
//
//     Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//     Added support for non-uniform axis spacing.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Added axis flag attributes to exported axis attribute data.
//   
// *****************************************************************************

void
avtParallelAxisFilter::StoreAxisAttributesForOutsideQueries()
{
    stringVector curAxisNames    = parAxisAtts.GetOrderedAxisNames();
    stringVector curGroupNames   = parAxisAtts.GetAxisGroupNames();
    doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();
    doubleVector curSliderMinima = parAxisAtts.GetExtentMinima();
    doubleVector curSliderMaxima = parAxisAtts.GetExtentMaxima();
    intVector    curMinTimeOrds  = parAxisAtts.GetExtMinTimeOrds();
    intVector    curMaxTimeOrds  = parAxisAtts.GetExtMaxTimeOrds();
    intVector    curInfoFlagSets = parAxisAtts.GetAxisInfoFlagSets();
    doubleVector curXPositions   = parAxisAtts.GetAxisXPositions();
    
    axisCount = curAxisNames.size();
    
    const char *axisName, *groupName;
    int axisNameLen, groupNameLen, axisNum, charNum;

    axisAttsArray.clear();

    axisAttsArray.push_back(PCP_LEFT_AXIS_X_FRACTION);
    axisAttsArray.push_back(PCP_RIGHT_AXIS_X_FRACTION);

    if (useVerticalText)
    {
        axisAttsArray.push_back(PCP_V_BOTTOM_AXIS_Y_FRACTION);
        axisAttsArray.push_back(PCP_V_TOP_AXIS_Y_FRACTION);
    }
    else
    {
        axisAttsArray.push_back(PCP_H_BOTTOM_AXIS_Y_FRACTION);
        axisAttsArray.push_back(PCP_H_TOP_AXIS_Y_FRACTION);
    }
    
    axisAttsArray.push_back((double)(parAxisAtts.GetPlotToolModeFlags()));
    
    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisName = curAxisNames[axisNum].c_str();
        axisNameLen = strlen(axisName);
        
        for (charNum = 0; charNum < axisNameLen; charNum++)
        {
            axisAttsArray.push_back((double)axisName[charNum]);
        }
        
        axisAttsArray.push_back(0.0);

        groupName = curGroupNames[axisNum].c_str();
        groupNameLen = strlen(groupName);
        
        for (charNum = 0; charNum < groupNameLen; charNum++)
        {
            axisAttsArray.push_back((double)groupName[charNum]);
        }
        
        axisAttsArray.push_back(0.0);

        axisAttsArray.push_back(curAxisMinima[axisNum]);
        axisAttsArray.push_back(curAxisMaxima[axisNum]);
        axisAttsArray.push_back(curSliderMinima[axisNum]);
        axisAttsArray.push_back(curSliderMaxima[axisNum]);
        axisAttsArray.push_back((double)curMinTimeOrds[axisNum]);
        axisAttsArray.push_back((double)curMaxTimeOrds[axisNum]);
        axisAttsArray.push_back((double)curInfoFlagSets[axisNum]);
        axisAttsArray.push_back(curXPositions[axisNum]);
    }

    axisAttsArray.push_back(0.0);
    
    PlotInfoAttributes plotInfoAtts;
    plotInfoAtts.SetOutputArray(axisAttsArray);
    GetOutput()->GetInfo().GetAttributes().SetPlotInfoAtts(&plotInfoAtts);
}


// *****************************************************************************
// Method: avtParallelAxisFilter::InitializePlotAtts
//
// Purpose: Initializes general attributes of the parallel axis plot.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Wed Dec 20 17:52:01 PST 2006
//     Added support for non-uniform axis spacing.
//
// *****************************************************************************

void
avtParallelAxisFilter::InitializePlotAtts()
{
    plotAxisXPositions.clear();
    plotAxisMinima.clear(); plotAxisMaxima.clear();
    subrangeMinima.clear(); subrangeMaxima.clear();

    plotAxisTitles.clear();
}


// *****************************************************************************
// Method: avtParallelAxisPlot:InitializeDataTupleInput
//
// Purpose: Initializes parallel axis plot for the input of data tuples in
//          n-dimensional space.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Wed Nov  8 16:01:27 PST 2006
//     Applies extents to input data only when Extents tool is active.
//
//     Mark Blair, Wed Dec 20 17:52:01 PST 2006
//     Added support for non-uniform axis spacing.
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Now applies extents only to those axes the Extents tool will not see.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Removed some obsolete code.
//
// *****************************************************************************

void
avtParallelAxisFilter::InitializeDataTupleInput(
    bool drawLabelsAndTitles, bool selectedVarsOnly)
{
    double leftAxisX = PCP_LEFT_AXIS_X_FRACTION;
    double plotWidth = PCP_RIGHT_AXIS_X_FRACTION - PCP_LEFT_AXIS_X_FRACTION;
    double tickSpacing;

    if (useVerticalText)
    {
        bottomAxisY = PCP_V_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_V_TOP_AXIS_Y_FRACTION;
    }
    else
    {
        bottomAxisY = PCP_H_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_H_TOP_AXIS_Y_FRACTION;
    }
    
    if (!drawLabelsAndTitles)
    {
        bottomAxisY += EA_TOOL_BUTTON_MARGIN_FRACTION;
        topAxisY    -= EA_TOOL_MARK_MARGIN_FRACTION;
    }
    
    double axisHeight = topAxisY - bottomAxisY;

    tickSpacing = axisHeight / (double)PCP_DEFAULT_TICK_MARK_INTERVALS;

    if (useVerticalText)
    {
        axisTitleY      = PCP_V_TITLE_Y_FRACTION;
        axisTitleXOff   = PCP_V_TITLE_X_OFFSET_FRACTION;
        bottomLabelY    = axisTitleY;
        bottomLabelXOff = PCP_BOTTOM_V_LABEL_X_OFF_FRAC;
        topLabelY       = PCP_TOP_V_LABEL_Y_FRACTION;
        topLabelXOff1   = PCP_TOP_V_LABEL_X_OFF_FRAC_1;
        topLabelXOff2   = PCP_TOP_V_LABEL_X_OFF_FRAC_2;
        bottomBoundY    = bottomLabelY;
        bottomBoundXOff = bottomLabelXOff;
        topBoundY       = topLabelY;
        topBoundXOff    = PCP_TOP_V_BOUND_X_OFF_FRAC;

        titleCharWidth  = PCP_V_TITLE_CHAR_WIDTH_FRACTION;
        titleCharHeight = PCP_V_TITLE_CHAR_HEIGHT_FRACTION;
    }
    else
    {
        axisTitleY   = PCP_H_TITLE_Y_FRACTION;
        bottomLabelY = PCP_BOTTOM_H_LABEL_Y_FRACTION;
        topLabelY    = PCP_TOP_H_LABEL_Y_FRACTION;
        bottomBoundY = PCP_BOTTOM_H_BOUND_Y_FRACTION;
        topBoundY    = PCP_TOP_H_BOUND_Y_FRACTION;

        titleCharWidth  = PCP_H_TITLE_CHAR_WIDTH_FRACTION;
        titleCharHeight = PCP_H_TITLE_CHAR_HEIGHT_FRACTION;
    }

    if (!drawLabelsAndTitles)
    {
        axisTitleY   += EA_TOOL_BUTTON_MARGIN_FRACTION;
        bottomLabelY += EA_TOOL_BUTTON_MARGIN_FRACTION;
        bottomBoundY += EA_TOOL_BUTTON_MARGIN_FRACTION;

        topLabelY -= EA_TOOL_MARK_MARGIN_FRACTION;
        topBoundY -= EA_TOOL_MARK_MARGIN_FRACTION;
    }
    
    labelCharWidth  = PCP_LABEL_CHAR_WIDTH_FRACTION;
    labelCharHeight = PCP_LABEL_CHAR_HEIGHT_FRACTION;
    boundCharWidth  = PCP_BOUND_CHAR_WIDTH_FRACTION;
    boundCharHeight = PCP_BOUND_CHAR_HEIGHT_FRACTION;

    tickMarkIntervals = PCP_DEFAULT_TICK_MARK_INTERVALS;
    halfTickLength    = PCP_TICK_MARK_LENGTH_FRACTION * 0.5;

    halfArrowWidth = PCP_SELECTOR_ARROW_WIDTH_FRACTION * 0.5;
    arrowHeight    = PCP_SELECTOR_ARROW_WIDTH_FRACTION * 0.5;

    dataTransforms.clear();

    int axisNum, tickNum;
    double axisScale;
    doubleVector *dataTransform;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        dataTransform = new doubleVector;

        axisScale = axisHeight / (plotAxisMaxima[axisNum] - plotAxisMinima[axisNum]);

        dataTransform->push_back(plotAxisXPositions[axisNum]*plotWidth + leftAxisX);
        dataTransform->push_back(axisScale);
        dataTransform->push_back(bottomAxisY - axisScale*plotAxisMinima[axisNum]);

        dataTransforms.push_back(*dataTransform);
    }

    tickMarkOrdinates.clear();

    for (tickNum = 1; tickNum < tickMarkIntervals; tickNum++)
    {
        tickMarkOrdinates.push_back((double)tickNum*tickSpacing + bottomAxisY);
    }

    applySubranges.clear();
    moveTitles.clear(); moveTopLabels.clear();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        applySubranges.push_back(false);

        moveTitles.push_back(false);
        moveTopLabels.push_back(false);
    }

    double moveOffset;
    double axisSpan, axisMinSpan, axisMaxSpan;

    if (useVerticalText)
    {
        moveOffset = PCP_VERTICAL_MOVE_OFFSET_FRACTION;

        movedAxisTitleY = bottomAxisY - moveOffset;
        movedTopLabelY  = topAxisY    + moveOffset;

        for (axisNum = 0; axisNum < axisCount; axisNum++)
        {
            moveTitles[axisNum]    = true;
            moveTopLabels[axisNum] = true;
        }
    }

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (selectedVarsOnly) continue;
        if ((axisNum >= leftPlotAxisID) && (axisNum <= rightPlotAxisID))
            continue;

        axisSpan = plotAxisMaxima[axisNum] - plotAxisMinima[axisNum];

        axisMinSpan = subrangeMinima[axisNum] - plotAxisMinima[axisNum];
        axisMaxSpan = plotAxisMaxima[axisNum] - subrangeMaxima[axisNum];

        if ((axisMinSpan/axisSpan > 0.0001) || (axisMaxSpan/axisSpan > 0.0001))
        {
            applySubranges[axisNum] = !drawLabelsAndTitles;
        }
    }
    
    outputCurveCount = 0;
}


// *****************************************************************************
// Method: avtParallelAxisPlot:InitializeOutputDataSets
//
// Purpose: Initializes VTK output datasets returned by GetPlotOutput as output
//          for a parallel axis plot.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Tue Aug 14 16:20:25 PDT 2007
//     Removed initialization of subrange bounds; now drawn by Extents tool.
//
// *****************************************************************************

void
avtParallelAxisFilter::InitializeOutputDataSets()
{
//
//  Initialize polyline dataset for the output data curves.
//
    dataCurvePolyData = vtkPolyData::New();

    dataCurvePoints = vtkPoints::New();
    dataCurvePolyData->SetPoints(dataCurvePoints);
    dataCurvePoints->Delete();

    dataCurveLines = vtkCellArray::New();
    dataCurvePolyData->SetLines(dataCurveLines);
    dataCurveLines->Delete();

    dataCurveVerts = vtkCellArray::New();
    dataCurvePolyData->SetVerts(dataCurveVerts);
    dataCurveVerts->Delete();

//
//  Initialize polyline dataset for the coordinate axes and corresponding labels.
//
    axisPolyData = vtkPolyData::New();

    axisPoints = vtkPoints::New();
    axisPolyData->SetPoints(axisPoints);
    axisPoints->Delete();

    axisLines = vtkCellArray::New();
    axisPolyData->SetLines(axisLines);
    axisLines->Delete();

    axisVerts = vtkCellArray::New();
    axisPolyData->SetVerts(axisVerts);
    axisVerts->Delete();

//
//  Initialize polyline dataset for the coordinate axis labels.
//
    labelPolyData = vtkPolyData::New();

    labelPoints = vtkPoints::New();
    labelPolyData->SetPoints(labelPoints);
    labelPoints->Delete();

    labelLines = vtkCellArray::New();
    labelPolyData->SetLines(labelLines);
    labelLines->Delete();

    labelVerts = vtkCellArray::New();
    labelPolyData->SetVerts(labelVerts);
    labelVerts->Delete();

//
//  Initialize polyline dataset for the coordinate axis titles.
//
    titlePolyData = vtkPolyData::New();

    titlePoints = vtkPoints::New();
    titlePolyData->SetPoints(titlePoints);
    titlePoints->Delete();

    titleLines = vtkCellArray::New();
    titlePolyData->SetLines(titleLines);
    titleLines->Delete();

    titleVerts = vtkCellArray::New();
    titlePolyData->SetVerts(titleVerts);
    titleVerts->Delete();
}


// *****************************************************************************
// Method: avtParallelAxisFilter::InputDataTuple
//
// Purpose: Called to input an n-dimensional data tuple in a parallel axis
//          plot.  All attributes of the plot are assumed to have been set.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now supports all variable axis spacing and axis group conventions.
//
// *****************************************************************************

void
avtParallelAxisFilter::InputDataTuple(const floatVector &inputTuple)
{
    int axisID;
    double plotAxisMin, plotAxisMax, inputCoord;
    doubleVector dTrans;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    for (axisID = 0; axisID < axisCount; axisID++)
    {
        if (applySubranges[axisID])
        {
            if (inputTuple[axisID] < subrangeMinima[axisID]) break;
            if (inputTuple[axisID] > subrangeMaxima[axisID]) break;
        }
    }
    
    if (axisID < axisCount) return;

    for (axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        plotAxisMin = plotAxisMinima[axisID];
        plotAxisMax = plotAxisMaxima[axisID];

        if ((inputCoord = (double)inputTuple[axisID]) < plotAxisMin)
            inputCoord = plotAxisMin;
        else if (inputCoord > plotAxisMax)
            inputCoord = plotAxisMax;

        dTrans = dataTransforms[axisID];

        outputCoords[0] = (float)dTrans[0];
        outputCoords[1] = (float)(dTrans[1]*inputCoord + dTrans[2]);
        
        dataCurvePoints->InsertNextPoint(outputCoords);
    }
    
    outputCurveCount++;
}


// *****************************************************************************
// Method: avtParallelAxisFilter::CountDataTuple
//
// Purpose: Takes the n-dimensional data tuple and increments the appropriate
//          bin for each pair of consecutive axes.
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2007
//
// Modifications:
// *****************************************************************************

void
avtParallelAxisFilter::CountDataTuple(const floatVector &inputTuple)
{
    if (!parAxisAtts.GetDrawContext())
        return;

    int nparts = parAxisAtts.GetContextNumPartitions();
    int axisID;
    for (axisID = 0; axisID < axisCount-1; axisID++)
    {
        int a0 = axisID;
        int a1 = axisID+1;
        // Normalize the raw values to [0,1]
        float v0 = ((inputTuple[a0] - plotAxisMinima[a0]) / 
                    (plotAxisMaxima[a0] - plotAxisMinima[a0]));
        float v1 = ((inputTuple[a1] - plotAxisMinima[a1]) / 
                    (plotAxisMaxima[a1] - plotAxisMinima[a1]));
        // Convert to [0,nparts]
        int i0 = int(nparts*v0);
        int i1 = int(nparts*v1);
        // Clamp to [0,nparts)
        if (i0<0)
            i0=0;
        if (i0>=nparts)
            i0=nparts-1;
        if (i1<0)
            i1=0;
        if (i1>=nparts)
            i1=nparts-1;
        // Increment the bin
        binnedAxisCounts[axisID][i0*nparts+i1]++;
    }
}


// *****************************************************************************
// Method: avtParallelAxisFilter::DrawDataCurves
//
// Purpose: Draws the n-vertex polylines that represent the n-dimensional data
//          points in a parallel axis plot.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Jeremy Meredith, Thu Mar  1 13:34:58 EST 2007
//     Disabled verts.  They're not visible under the lines and they
//     will slow things down if we add them.
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Added tag to output dataset identifying it as data curves.
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawDataCurves()
{
    vtkIdType vtkPointIDs[2];

    vtkIdType segmentCount = (vtkIdType)(rightPlotAxisID - leftPlotAxisID);
    int plotAxisCount = (int)segmentCount + 1;
    vtkIdType firstVTKPointID, segmentNum;
    double pointXYZ[3];

    for (int curveNum = 0; curveNum < outputCurveCount; curveNum++)
    {
        firstVTKPointID = (vtkIdType)(curveNum * plotAxisCount);

        for (segmentNum = 0; segmentNum < segmentCount; segmentNum++)
        {
            vtkPointIDs[0] = firstVTKPointID + segmentNum;
            vtkPointIDs[1] = vtkPointIDs[0] + 1;

            dataCurveLines->InsertNextCell(2, vtkPointIDs);
        }
    }

    if (outputCurveCount > 0)
    {
        // Tag second point of output as data curves for custom renderer.
        dataCurvePolyData->GetPoint(0, pointXYZ);
        pointXYZ[2] = double((PCP_RENDERER_DATA_CURVE_INPUT << 10) |
                             (processorRank >> 7));
        dataCurvePolyData->GetPoints()->SetPoint(0, pointXYZ);

        dataCurvePolyData->GetPoint(1, pointXYZ);
        pointXYZ[2] = double(((processorRank & 0x7f) << 16) | partitionSize);
        dataCurvePolyData->GetPoints()->SetPoint(1, pointXYZ);
    }
}


// *****************************************************************************
// Method: avtParallelAxisFilter::DrawCoordinateAxes
//
// Purpose: Draws the n parallel coordinate axes in an n-dimensional parallel
//          axis plot.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Added tag to output dataset identifying it as axes and tick marks and/or 
//     axis labels.
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Now can always tag output as axis lines only.
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxes()
{
    double axisX;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        axisX = dataTransforms[axisID][0];

        outputCoords[0] = (float)axisX;
        outputCoords[1] = (float)bottomAxisY;
        axisPoints->InsertNextPoint(outputCoords);

        outputCoords[1] = (float)topAxisY;
        axisPoints->InsertNextPoint(outputCoords);

        for (int tickOrdID = 0; tickOrdID < tickMarkOrdinates.size(); tickOrdID++)
        {
            outputCoords[0] = (float)(axisX - halfTickLength);
            outputCoords[1] = (float)tickMarkOrdinates[tickOrdID];
            axisPoints->InsertNextPoint(outputCoords);

            outputCoords[0] = (float)(axisX + halfTickLength);
            axisPoints->InsertNextPoint(outputCoords);
        }
    }

    vtkIdType vtkPointIDs[2];

    int segmentCount = (rightPlotAxisID - leftPlotAxisID + 1) * tickMarkIntervals;

    for (int segmentNum = 0; segmentNum < segmentCount; segmentNum++)
    {
        vtkPointIDs[0] = (vtkIdType)(segmentNum * 2);
        vtkPointIDs[1] = vtkPointIDs[0] + 1;

        axisLines->InsertNextCell(2, vtkPointIDs);
    }

    // Tag first point of output as axes and tick marks for custom renderer.
    double firstPoint[3];
    axisPolyData->GetPoint(0, firstPoint);
    firstPoint[2] = double(PCP_RENDERER_AXIS_LINE_INPUT << 10);
    axisPolyData->GetPoints()->SetPoint(0, firstPoint);
}


// *****************************************************************************
// Method: avtParallelAxisFilter::DrawCoordinateAxisLabels
//
// Purpose: Draws the numerical labels at the bottom and top of each axis in a
//          parallel axis plot.  The bottom label corresponds to the axis
//          minimum and the top label corresponds to its maximum.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now draws bound labels for an axis only if its labels are flagged visible.
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Added tag to output dataset identifying it as axes and tick marks and/or
//     axis labels.
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Now calculates label positions only; custom renderer handles everything
//     else.
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisLabels()
{
    double axisX;
    float startCoords[3];
    startCoords[2] = 0.0;

    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        startCoords[1] = (moveTitles[axisID]) ? movedAxisTitleY : bottomLabelY;

        for (int labelNum = 0; labelNum < 2; labelNum++)
        {
            axisX = dataTransforms[axisID][0];

            if (useVerticalText)
                startCoords[0] =
                axisX + ((labelNum == 0) ? bottomLabelXOff : topLabelXOff1);
            else
                startCoords[0] = axisX;

            labelPoints->InsertNextPoint(startCoords);

            startCoords[1] = (moveTopLabels[axisID]) ? movedTopLabelY : topLabelY;
        }
        
        startCoords[0] = (float)plotAxisMinima[axisID];
        startCoords[1] = (float)plotAxisMaxima[axisID];

        labelPoints->InsertNextPoint(startCoords);
    }

    // No purpose for cell definitions other than to prevent points from being
    // deleted because they are not used in any polygons.  Custom renderer only
    // needs the points.
    int lineCount = rightPlotAxisID - leftPlotAxisID + 1;
    vtkIdType vtkPointIDs[3];

    for (int lineNum = 0; lineNum < lineCount; lineNum++)
    {
        vtkPointIDs[0] = (vtkIdType)lineNum * 3;
        vtkPointIDs[1] = vtkPointIDs[0] + 1;
        vtkPointIDs[2] = vtkPointIDs[0] + 2;

        labelLines->InsertNextCell(3, vtkPointIDs);
    }

    // Tag first point of output as axis label start points for custom renderer.
    double firstPoint[3];
    labelPolyData->GetPoint(0, firstPoint);
    firstPoint[2] = double(PCP_RENDERER_AXIS_BOUND_INPUT << 10);
    labelPolyData->GetPoints()->SetPoint(0, firstPoint);
}


// *****************************************************************************
// Method: avtParallelAxisFilter::DrawCoordinateAxisTitles
//
// Purpose: Draws the titles at the bottom of each axis in a parallel axis plot.
//          An axis title is simply the name of the variable whose values are
//          displayed on that axis.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Now draws title of an axis only if its title is flagged visible.
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Added tag to output dataset identifying it as axis titles.
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Mark Blair, Tue Oct 16 19:33:48 PDT 2007
//     Now calculates title positions only; custom renderer handles everything
//     else.
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisTitles()
{
    double axisX;
    float startCoords[3];
    startCoords[2] = 0.0;

    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        axisX = dataTransforms[axisID][0];

        if (useVerticalText)
        {
            startCoords[0] = axisX + axisTitleXOff;
            startCoords[1] = (moveTitles[axisID]) ? movedAxisTitleY : axisTitleY;
        }
        else
        {
            startCoords[0] = axisX;
            startCoords[1] = axisTitleY;
        }

        titlePoints->InsertNextPoint(startCoords);
    }

    // No purpose for cell definitions other than to prevent points from being
    // deleted because they are not used in any polygons.  Custom renderer only
    // needs the points.
    int lineCount = rightPlotAxisID - leftPlotAxisID;
    vtkIdType vtkPointIDs[2];

    for (int lineNum = 0; lineNum < lineCount; lineNum++)
    {
        vtkPointIDs[0] = (vtkIdType)lineNum;
        vtkPointIDs[1] = vtkPointIDs[0] + 1;

        titleLines->InsertNextCell(2, vtkPointIDs);
    }

    // Tag first point of output as axis title start points for custom renderer.
    double firstPoint[3];
    titlePolyData->GetPoint(0, firstPoint);
    firstPoint[2] = double(PCP_RENDERER_AXIS_TITLE_INPUT << 10);
    titlePolyData->GetPoints()->SetPoint(0, firstPoint);
}


// ****************************************************************************
//  Method:  avtParallelAxisFilter::DrawContext
//
//  Purpose:
//    Draw the parallel axis context bins.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 14, 2007
//
// Modifications:
//
//     Mark Blair, Fri Aug  3 17:10:19 PDT 2007
//     Added tag to each output dataset identifying it as "Context" polygons.
//
//     Mark Blair, Thu Aug  9 14:11:55 PDT 2007
//     Modified logic to generate points only for those polygons actually drawn.
//     And now only draws polygons between axes currently shown on the plot.
//
//     Mark Blair, Wed Aug 22 15:56:42 PDT 2007
//     Modified to correct an oversight in parallel rendering.
//
//     Hank Childs, Tue Oct 16 16:16:34 PDT 2007
//     Remove reference to powf.  Always use pow.
//
//    Hank Childs, Wed Oct 17 16:04:19 PDT 2007
//    Make sure both arguments to pow are doubles.  Otherwise, this causes
//    an ambiguity that xlc can't handle.
//
// ****************************************************************************

void
avtParallelAxisFilter::DrawContext()
{
    int nparts = parAxisAtts.GetContextNumPartitions();

#ifdef PARALLEL
    int binsize = nparts*nparts;
    int *newbin = new int[binsize];
    for (int axis=0; axis<axisCount; axis++)
    {
        int *oldbin = binnedAxisCounts[axis];
        SumIntArrayAcrossAllProcessors(oldbin, newbin, binsize);
        binnedAxisCounts[axis] = newbin;
        newbin = oldbin;
    }
    delete[] newbin;

    if (PAR_Rank() != 0)
    {
        // only process 0 has to draw the context
        return;
    }
#endif

    vtkPolyData  *dataContextPolyData[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkPoints    *dataContextPoints[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray *dataContextLines[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray *dataContextVerts[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray *dataContextPolys[PCP_CTX_BRIGHTNESS_LEVELS];

    //
    //  Initialize polygon datasets for the context.
    //
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
    {
        dataContextPolyData[i] = vtkPolyData::New();

        dataContextPoints[i] = vtkPoints::New();
        dataContextPolyData[i]->SetPoints(dataContextPoints[i]);
        dataContextPoints[i]->Delete();

        dataContextLines[i] = vtkCellArray::New();
        dataContextPolyData[i]->SetLines(dataContextLines[i]);
        dataContextLines[i]->Delete();

        dataContextVerts[i] = vtkCellArray::New();
        dataContextPolyData[i]->SetVerts(dataContextVerts[i]);
        dataContextVerts[i]->Delete();

        dataContextPolys[i] = vtkCellArray::New();
        dataContextPolyData[i]->SetPolys(dataContextPolys[i]);
        dataContextPolys[i]->Delete();
    }

    float gamma = parAxisAtts.GetContextGamma();
    if (gamma<.1)
        gamma=.1;
    if (gamma>10)
        gamma=10;

    double partHeight = (topAxisY - bottomAxisY) / double(nparts);
    double dA, dB;
    float alpha;
    int axis, bin, maxcount, a, b, c;

    float vertexXYZ[3];
    vertexXYZ[2] = 0.0;

    for (axis = leftPlotAxisID; axis < rightPlotAxisID; axis++)
    {
        // Find the maximum count in the bins
        maxcount = 0;
        for (bin=0; bin<nparts*nparts; bin++)
        {
            if (binnedAxisCounts[axis][bin] > maxcount)
                maxcount = binnedAxisCounts[axis][bin];
        }
        // Draw each bin as a polygon in the appropriately
        // colored (and layered) context polydata
        for (a=0; a<nparts; a++)
        {
            for (b=0; b<nparts; b++)
            {
                alpha = float(binnedAxisCounts[axis][a*nparts+b]) /
                        float(maxcount);

                alpha = pow((double)alpha,1./gamma);

                int c = int(float(PCP_CTX_BRIGHTNESS_LEVELS-1) * alpha);

                if (c != 0)
                {
                    dA = (double)a; dB = (double)b;

                    vertexXYZ[0] = float(dataTransforms[axis][0]);
                    vertexXYZ[1] = float(dA*partHeight + bottomAxisY);
                    dataContextPoints[c]->InsertNextPoint(vertexXYZ);

                    vertexXYZ[0] = float(dataTransforms[axis+1][0]);
                    vertexXYZ[1] = float(dB*partHeight + bottomAxisY);
                    dataContextPoints[c]->InsertNextPoint(vertexXYZ);

                    vertexXYZ[1] = float((dB+1.0)*partHeight + bottomAxisY);
                    dataContextPoints[c]->InsertNextPoint(vertexXYZ);

                    vertexXYZ[0] = float(dataTransforms[axis][0]);
                    vertexXYZ[1] = float((dA+1.0)*partHeight + bottomAxisY);
                    dataContextPoints[c]->InsertNextPoint(vertexXYZ);
                }
            }
        }
    }

    int brightLev, pointCount, pointID, n;
    vtkIdType polyPointIDs[4];
    double firstPoint[3];

    for (brightLev = 0; brightLev < PCP_CTX_BRIGHTNESS_LEVELS; brightLev++)
    {
        if ((pointCount = dataContextPolyData[brightLev]->GetNumberOfPoints()) > 0)
        {
            for (pointID = 0; pointID < pointCount; pointID += 4)
            {
                for (n = 0; n < 4; n++)
                    polyPointIDs[n] = pointID + n;

                dataContextPolys[brightLev]->InsertNextCell(4, polyPointIDs);
            }

            // Tag first point of output as plot "context" for custom renderer.
            dataContextPolyData[brightLev]->GetPoint(0, firstPoint);
            firstPoint[2] = double(brightLev << 10);
            dataContextPolyData[brightLev]->GetPoints()->SetPoint(0, firstPoint);
        }
    }

    // We have to explicitly convert these into base types
    vtkDataSet **outputDataSets = new vtkDataSet*[PCP_CTX_BRIGHTNESS_LEVELS];
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
        outputDataSets[i] = dataContextPolyData[i];

    // Add them to a data tree
    avtDataTree_p contextTree = new avtDataTree(PCP_CTX_BRIGHTNESS_LEVELS,
                                                outputDataSets,
                                                -1, contextLabels);
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
        outputDataSets[i]->Delete();
    delete [] outputDataSets;

    // Make a new data tree with the context first and lines second
    // so that the lines are drawn on top of the context
    avtDataTree_p linesTree = GetDataTree();
    avtDataTree_p trees[2] = { contextTree, linesTree };
    avtDataTree_p newOutput = new avtDataTree(2, trees);
    SetOutputDataTree(newOutput);
}


// ****************************************************************************
//  Method:  avtParallelAxisFilter::InitializePairwiseBins
//
//  Purpose:
//    Allocate arrays to hold counts of binned data.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
avtParallelAxisFilter::InitializePairwiseBins()
{
    int nparts = parAxisAtts.GetContextNumPartitions();
    binnedAxisCounts = new int*[axisCount];
    for (int i=0; i<axisCount; i++)
    {
        // initialize counts to zero
        int size = nparts*nparts;
        binnedAxisCounts[i] = new int[size];
        for (int j=0; j<size; j++)
            binnedAxisCounts[i][j] = 0;
    }
}

// ****************************************************************************
//  Method:  avtParallelAxisFilter::CleanUpPairwiseBins
//
//  Purpose:
//    Free data allocated to hold the bins for the context.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 16, 2007
//
// ****************************************************************************

void
avtParallelAxisFilter::CleanUpPairwiseBins()
{
    for (int i=0; i<axisCount; i++)
    {
        delete[] binnedAxisCounts[i];
    }
    delete[] binnedAxisCounts;
    binnedAxisCounts = NULL;
}


