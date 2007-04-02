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
// ****************************************************************************

avtParallelAxisFilter::avtParallelAxisFilter(ParallelAxisAttributes &atts)
{
    parAxisAtts = atts;
    parallelRank = PAR_Rank();
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
//  Method: avtParallelAxisFilter::PerformRestriction
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

avtPipelineSpecification_p
avtParallelAxisFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    if (!parAxisAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PR1: ParallelAxis plot attributes are inconsistent."
               << endl;

        return in_spec;
    }
        
    const char *inPipelineVar = in_spec->GetDataSpecification()->GetVariable();
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

    avtPipelineSpecification_p outSpec = new avtPipelineSpecification(in_spec);
    
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

        outSpec->GetDataSpecification()->GetRestriction()->RestrictDomains(outDomains);
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
// *****************************************************************************

void
avtParallelAxisFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    domainCount = GetInputDataTree()->GetNumberOfLeaves();
    
    if (!parAxisAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PE1: ParallelAxis plot attributes are inconsistent."
               << endl;
        
        sendNullOutput = true;
        return;
    }

    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    intVector    curAxisFlagSets = parAxisAtts.GetAxisInfoFlagSets();
        
    axisCount = curAxisVarNames.size();
    sendNullOutput = false;

    ComputeCurrentDataExtentsOverAllDomains();
    
    if (sendNullOutput) return;
    
    int axisNum, axisInfoFlagSet;
    
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

    parAxisAtts.RecalculateAxisXPositions(leftPlotAxisID, rightPlotAxisID);
    DetermineAxisBoundsAndGroupNames();

    int plotToolModeFlags = parAxisAtts.GetPlotToolModeFlags();
    
    if ((plotToolModeFlags & EA_TOOL_DRAWS_AXIS_INFO_FLAG) == 0)
        parAxisAtts.IdentifyReasonableAxesToLabel();
    else if ((plotToolModeFlags & EA_AXIS_INFO_AUTO_LAYOUT_FLAG) != 0)
    {
        parAxisAtts.IdentifyReasonableAxesToLabel();   // Redundant but intuitive
        
        plotToolModeFlags ^= EA_AXIS_INFO_AUTO_LAYOUT_FLAG;
        parAxisAtts.SetPlotToolModeFlags(plotToolModeFlags);
    }
    
    useVerticalText = ((plotToolModeFlags & EA_VERTICAL_TEXT_AXIS_INFO_FLAG) != 0);
    
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
            
            InputDataTuple(inputTuple);
            CountDataTuple(inputTuple);
        }
    }

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList->Delete();
    }

    if (parAxisAtts.GetDrawLines())
    {
        DrawDataCurves();
    }

    if (parallelRank == 0)
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
    outputDataSets[2] = titlePolyData;
    outputDataSets[3] = subrangePolyData;

    avtDataTree *outputDataTree =
        new avtDataTree(4, outputDataSets, domain, curveAndAxisLabels);

    outputDataSets[0]->Delete();
    outputDataSets[1]->Delete();
    outputDataSets[2]->Delete();
    outputDataSets[3]->Delete();

    delete [] outputDataSets;

    return outputDataTree;
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::RefashionDataObjectInfo
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
avtParallelAxisFilter::RefashionDataObjectInfo(void)
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
    curveAndAxisLabels.push_back("Axes and Bounds");
    curveAndAxisLabels.push_back("Axis Titles");
    curveAndAxisLabels.push_back("Selector Arrows");

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

    doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();

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
//  Method: avtParallelAxisPlot::DetermineAxisBoundsAndGroupNames
//
//  Purpose: This method determines the min and max bounds of each axis in the
//           plot, based on group associations of the axes, individual extents
//           of the input data for each axis, and any forced axis bounds that
//           may have been supplied by the user.  Also sets the text name of
//           the axis group to which each axis belongs.
//
//  Programmer: Mark Blair
//  Creation:   Wed Feb  7 17:54:18 PST 2007
//
//  Modifications:
//
// *****************************************************************************

void
avtParallelAxisFilter::DetermineAxisBoundsAndGroupNames()
{
    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();

    stringVector curAxisAttVars = parAxisAtts.GetAxisAttributeVariables();
    doubleVector curAxisAttData = parAxisAtts.GetAxisAttributeData();
    
    intVector    axisGroupIDNums;
    stringVector axisGroupNames;
    
    intVector    groupIDList;
    doubleVector groupAxisMinima;
    doubleVector groupAxisMaxima;
    
    int attVarCount = curAxisAttVars.size();
    int attributesPerAxis = parAxisAtts.GetAttributesPerAxis();
    int dummyGroupID = PCP_FIRST_DUMMY_AXIS_GROUP_ID;
    int axisID, axisGroupID, attVarID, attValueMap, groupIDNum;
    double axisMinimum, axisMaximum;
    double *axisAttData;
    std::string axisName;
    
    char groupName[81];

    for (axisID = 0; axisID < axisCount; axisID++)
    {
        axisName = curAxisVarNames[axisID];
        
        for (attVarID = 0; attVarID < attVarCount; attVarID++)
        {
            if (curAxisAttVars[attVarID] == axisName) break;
        }
        
        if (attVarID < attVarCount)
        {
            axisAttData = &curAxisAttData[attVarID*(attributesPerAxis+1)];
            attValueMap = (int)axisAttData[attributesPerAxis];
            
            if ((attValueMap & PCP_GROUP_ID_ATTRIBUTE_FLAG) != 0)
            {
                axisGroupID = (int)axisAttData[PCP_GROUP_ID_ATTRIBUTE_OFFSET];
                sprintf(groupName, "group_%d", axisGroupID);
            }
            else
            {
                axisGroupID = dummyGroupID++;
                strcpy(groupName, "(not_in_a_group)");
            }

            for (groupIDNum = 0; groupIDNum < groupIDList.size(); groupIDNum++)
            {
                if (groupIDList[groupIDNum] == axisGroupID) break;
            }
            
            if (groupIDNum >= groupIDList.size())
            {
                groupIDNum = groupIDList.size();
                
                groupIDList.push_back(axisGroupID);
                groupAxisMinima.push_back(+1e+37);
                groupAxisMaxima.push_back(-1e+37);
            }
            
            if ((attValueMap & PCP_LOWER_BOUND_ATTRIBUTE_FLAG) != 0)
                axisMinimum = axisAttData[PCP_LOWER_BOUND_ATTRIBUTE_OFFSET];
            else
                axisMinimum = curAxisMinima[axisID];
            
            if ((attValueMap & PCP_UPPER_BOUND_ATTRIBUTE_FLAG) != 0)
                axisMaximum = axisAttData[PCP_UPPER_BOUND_ATTRIBUTE_OFFSET];
            else
                axisMaximum = curAxisMaxima[axisID];
                
            if (axisMinimum < groupAxisMinima[groupIDNum])
                groupAxisMinima[groupIDNum] = axisMinimum;
            if (axisMaximum > groupAxisMaxima[groupIDNum])
                groupAxisMaxima[groupIDNum] = axisMaximum;
        }
        else
        {
            groupIDNum = groupIDList.size();

            groupIDList.push_back(dummyGroupID); dummyGroupID++;
            groupAxisMinima.push_back(curAxisMinima[axisID]);
            groupAxisMaxima.push_back(curAxisMaxima[axisID]);

            strcpy(groupName, "(not_in_a_group)");
        }
            
        axisGroupIDNums.push_back(groupIDNum);
        axisGroupNames.push_back(std::string(groupName));
    }

    for (axisID = 0; axisID < axisCount; axisID++)
    {
        groupIDNum = axisGroupIDNums[axisID];
        
        curAxisMinima[axisID] = groupAxisMinima[groupIDNum];
        curAxisMaxima[axisID] = groupAxisMaxima[groupIDNum];
    }
    
    parAxisAtts.SetAxisMinima(curAxisMinima);
    parAxisAtts.SetAxisMaxima(curAxisMaxima);
    
    parAxisAtts.SetAxisGroupNames(axisGroupNames);
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
    
    textPlotter = NULL;
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
    drawBottomLabels.clear(); drawBottomBounds.clear(); drawTopBounds.clear();
    moveTitles.clear(); moveTopLabels.clear();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        applySubranges.push_back(false);

        drawBottomLabels.push_back(true);
        drawBottomBounds.push_back(false);
        drawTopBounds.push_back(false);

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
        if (selectedVarsOnly)
        {
            if (axisNum <  leftSelectedAxisID) continue;
            if (axisNum > rightSelectedAxisID) continue;
        }

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

//
//  Initialize polyline dataset for the subrange labels.
//
    subrangePolyData = vtkPolyData::New();

    subrangePoints = vtkPoints::New();
    subrangePolyData->SetPoints(subrangePoints);
    subrangePoints->Delete();

    subrangeLines = vtkCellArray::New();
    subrangePolyData->SetLines(subrangeLines);
    subrangeLines->Delete();

    subrangeVerts = vtkCellArray::New();
    subrangePolyData->SetVerts(subrangeVerts);
    subrangeVerts->Delete();
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
//    Jeremy Meredith, Thu Mar  1 13:34:58 EST 2007
//    Disabled verts.  They're not visible under the lines and they
//    will slow things down if we add them.
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawDataCurves()
{
    vtkIdType vtkPointIDs[2];

    vtkIdType segmentCount = (vtkIdType)(rightPlotAxisID - leftPlotAxisID);
    int plotAxisCount = (int)segmentCount + 1;
    vtkIdType firstVTKPointID, segmentNum;

    for (int curveNum = 0; curveNum < outputCurveCount; curveNum++)
    {
        firstVTKPointID = (vtkIdType)(curveNum * plotAxisCount);

        for (segmentNum = 0; segmentNum < segmentCount; segmentNum++)
        {
            vtkPointIDs[0] = firstVTKPointID + segmentNum;
            vtkPointIDs[1] = vtkPointIDs[0] + 1;

            dataCurveLines->InsertNextCell(2, vtkPointIDs);
            //dataCurveVerts->InsertNextCell(1, vtkPointIDs);
        }

        //dataCurveVerts->InsertNextCell(1, &vtkPointIDs[1]);
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
        axisVerts->InsertNextCell(1, vtkPointIDs);
        axisVerts->InsertNextCell(1, &vtkPointIDs[1]);
    }
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
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisLabels()
{
    if (textPlotter == NULL) textPlotter = new PortableFont;
    
    intVector axisInfoFlagSets = parAxisAtts.GetAxisInfoFlagSets();

    bool drawIt, centerIt;
    int labelLen;
    double minOrMax, axisX, xOffset, labelX, labelY;
    PortableFont::PF_ORIENTATION orientation;
    std::vector<floatVector> *strokeList = new std::vector<floatVector>;
    char axisLabel[81];

    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        if ((axisInfoFlagSets[axisID] & EA_AXIS_INFO_SHOWN_FLAG) == 0) continue;

        minOrMax = plotAxisMinima[axisID];
        labelY = (moveTitles[axisID]) ? movedAxisTitleY : bottomLabelY;
        drawIt = drawBottomLabels[axisID];

        for (int labelNum = 0; labelNum < 2; labelNum++)
        {
            if (drawIt)
            {
                textPlotter->DoubleNumericalString(axisLabel, minOrMax);
                labelLen = strlen(axisLabel);

                axisX = dataTransforms[axisID][0];

                if (useVerticalText)
                {
                    if (labelNum == 0)
                    {
                        orientation = PortableFont::Downward;
                        xOffset = bottomLabelXOff;
                    }
                    else
                    {
                        orientation = PortableFont::Upward;
                        xOffset = (drawBottomBounds[axisID]) ?
                        topLabelXOff2 : topLabelXOff1;
                    }

                    labelX = axisX + xOffset;
                    centerIt = false;
                }
                else
                {
                    orientation = PortableFont::Rightward;

                    labelX = axisX;
                    centerIt = true;

                    if (axisID == leftPlotAxisID)
                    {
                        if (labelLen > 2)
                        {
                            labelX = axisX - labelCharWidth*1.5;
                            centerIt = false;
                        }
                    }
                    else if (axisID == rightPlotAxisID)
                    {
                        if (labelLen > 2)
                        {
                            labelX = axisX -
                                (double)(labelLen-2)*labelCharWidth*1.428571;
                            centerIt = false;
                        }
                    }
                }

                textPlotter->StrokeText(strokeList, orientation, centerIt,
                labelX, labelY, labelCharWidth, labelCharHeight, 4, axisLabel);
            }

            minOrMax = plotAxisMaxima[axisID];
            labelY = (moveTopLabels[axisID]) ? movedTopLabelY : topLabelY;
            drawIt = true;
        }
    }

    floatVector strokeCoords;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    int strokeNum, xCoordID;

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        strokeCoords = (*strokeList)[strokeNum];

        for (xCoordID = 0; xCoordID < 4; xCoordID += 2)
        {
            outputCoords[0] = strokeCoords[xCoordID];
            outputCoords[1] = strokeCoords[xCoordID+1];
            axisPoints->InsertNextPoint(outputCoords);
        }
    }

    vtkIdType vtkPointIDs[2];
    int axisVertexID =
        (rightPlotAxisID - leftPlotAxisID + 1) * tickMarkIntervals * 2;

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        vtkPointIDs[0] = (vtkIdType)axisVertexID;
        vtkPointIDs[1] = vtkPointIDs[0] + 1;

        axisLines->InsertNextCell(2, vtkPointIDs);
        axisVerts->InsertNextCell(1, vtkPointIDs);
        axisVerts->InsertNextCell(1, &vtkPointIDs[1]);

        axisVertexID += 2;
    }

    delete strokeList;
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
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisTitles()
{
    if (textPlotter == NULL) textPlotter = new PortableFont;
    
    intVector axisInfoFlagSets = parAxisAtts.GetAxisInfoFlagSets();

    bool centerIt;
    int thickness;
    double axisX, titleX, titleY;
    PortableFont::PF_ORIENTATION orientation;
    std::vector<floatVector> *strokeList = new std::vector<floatVector>;
    char axisTitle[41];

    if (useVerticalText)
    {
        orientation = PortableFont::Downward;
        thickness = 4;
        centerIt = false;
    }
    else
    {
        orientation = PortableFont::Rightward;
        thickness = 6;
        titleY = axisTitleY;
    }

    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        if ((axisInfoFlagSets[axisID] & EA_AXIS_INFO_SHOWN_FLAG) == 0) continue;

        axisX = dataTransforms[axisID][0];

        if (useVerticalText)
        {
            titleY = (moveTitles[axisID]) ? movedAxisTitleY : axisTitleY;
            titleX = axisX + axisTitleXOff;
        }
        else
        {
            if (axisID == leftPlotAxisID)
            {
                titleX = axisX - titleCharWidth;
                centerIt = false;
            }
            else if (axisID == rightPlotAxisID)
            {
                titleX = axisX -
                    (double)(plotAxisTitles[axisID].length()-1)*titleCharWidth*1.5;
                centerIt = false;
            }
            else
            {
                titleX = axisX;
                centerIt = true;
            }
        }

        strcpy (axisTitle, plotAxisTitles[axisID].c_str());

        textPlotter->StrokeText(strokeList, orientation, centerIt,
        titleX, titleY, titleCharWidth, titleCharHeight, thickness, axisTitle);
    }

    floatVector strokeCoords;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    int strokeNum, xCoordID;

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        strokeCoords = (*strokeList)[strokeNum];

        for (xCoordID = 0; xCoordID < 4; xCoordID += 2)
        {
            outputCoords[0] = strokeCoords[xCoordID];
            outputCoords[1] = strokeCoords[xCoordID+1];
            titlePoints->InsertNextPoint(outputCoords);
        }
    }

    vtkIdType vtkPointIDs[2];

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        vtkPointIDs[0] = (vtkIdType)(strokeNum * 2);
        vtkPointIDs[1] = vtkPointIDs[0] + 1;

        titleLines->InsertNextCell(2, vtkPointIDs);
        titleVerts->InsertNextCell(1, vtkPointIDs);
        titleVerts->InsertNextCell(1, &vtkPointIDs[1]);
    }

    delete strokeList;
}


// *****************************************************************************
// Method: avtParallelAxisFilter::DrawDataSubrangeBounds
//
// Purpose: Draws the numerical value corresponding to the minimum and maximum
//          of any data coordinate subrange that is not the entire range of its
//          coordinate axis.  These are placed at the bottom and top of the axis.
//
// Notes: Adapted from more general "parallel coordinate plot" package developed
//        earlier.  Subrange bounds are currently (8 June 2006) not drawn by the
//        plot.  Future plans call for the Extents tool, which is used to select
//        the subrange bounds, to draw those bounds on the plot axes when the
//        tool is enabled.
//
// Programmer: Mark Blair
// Creation:   Thu Jun  8 17:18:00 PDT 2006
//
// Modifications:
//
// *****************************************************************************

void
avtParallelAxisFilter::DrawDataSubrangeBounds()
{
    if (textPlotter == NULL) textPlotter = new PortableFont;

    bool drawIt, centerIt;
    int boundNum, boundLabelLen;
    double subrangeMinY, subrangeMaxY;
    double minOrMax, axisX, xOffset, boundX, boundY;
    PortableFont::PF_ORIENTATION orientation;
    doubleVector dTrans;
    std::vector<floatVector> *strokeList = new std::vector<floatVector>;
    char boundLabel[81];

    float outputCoords[3];
    outputCoords[2] = 0.0;


    for (int axisID = leftPlotAxisID; axisID <= rightPlotAxisID; axisID++)
    {
        if (!applySubranges[axisID]) continue;

        dTrans = dataTransforms[axisID];

        axisX = dTrans[0];
        subrangeMinY = dTrans[1]*subrangeMinima[axisID] + dTrans[2];
        subrangeMaxY = dTrans[1]*subrangeMaxima[axisID] + dTrans[2];

        minOrMax = subrangeMinima[axisID];
        boundY = (moveTitles[axisID]) ? movedAxisTitleY : bottomBoundY;
        drawIt = drawBottomBounds[axisID];

        for (boundNum = 0; boundNum < 2; boundNum++)
        {
            if (drawIt)
            {
                textPlotter->DoubleNumericalString(boundLabel, minOrMax);
                boundLabelLen = strlen(boundLabel);

                if (useVerticalText)
                {
                    if (boundNum == 0)
                    {
                        orientation = PortableFont::Downward;
                        xOffset = bottomBoundXOff;
                    }
                    else
                    {
                        orientation = PortableFont::Upward;
                        xOffset = topBoundXOff;
                    }

                    boundX = axisX + xOffset;
                    centerIt = false;
                }
                else
                {
                    orientation = PortableFont::Rightward;

                    boundX = axisX;
                    centerIt = true;

                    if (axisID == leftPlotAxisID)
                    {
                        if (boundLabelLen > 2)
                        {
                            boundX = axisX - boundCharWidth*1.5;
                            centerIt = false;
                        }
                    }
                    else if (axisID == rightPlotAxisID)
                    {
                        if (boundLabelLen > 2)
                        {
                            boundX = axisX -
                                (double)(boundLabelLen-2)*boundCharWidth*1.428571;
                            centerIt = false;
                        }
                    }
                }

                textPlotter->StrokeText(strokeList, orientation, centerIt,
                boundX, boundY, boundCharWidth, boundCharHeight, 4, boundLabel);
            }

            minOrMax = subrangeMaxima[axisID];
            boundY = (moveTopLabels[axisID]) ? movedTopLabelY : topBoundY;
            drawIt = drawTopBounds[axisID];
        }
    }

    int strokeNum, xCoordID;
    floatVector strokeCoords;

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        strokeCoords = (*strokeList)[strokeNum];

        for (xCoordID = 0; xCoordID < 4; xCoordID += 2)
        {
            outputCoords[0] = strokeCoords[xCoordID];
            outputCoords[1] = strokeCoords[xCoordID+1];
            subrangePoints->InsertNextPoint(outputCoords);
        }
    }

    vtkIdType vtkPointIDs[2];

    for (strokeNum = 0; strokeNum < strokeList->size(); strokeNum++)
    {
        vtkPointIDs[0] = (vtkIdType)(strokeNum * 2);
        vtkPointIDs[1] = vtkPointIDs[0] + 1;

        subrangeLines->InsertNextCell(2, vtkPointIDs);
        subrangeVerts->InsertNextCell(1, vtkPointIDs);
        subrangeVerts->InsertNextCell(1, &vtkPointIDs[1]);
    }

    delete strokeList;
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

    vtkPolyData        *dataContextPolyData[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkPoints          *dataContextPoints[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *dataContextLines[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *dataContextVerts[PCP_CTX_BRIGHTNESS_LEVELS];
    vtkCellArray       *dataContextPolys[PCP_CTX_BRIGHTNESS_LEVELS];

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

    for (int part = 0 ; part <= nparts ; part++)
    {
        for (int axisNum = 0; axisNum < axisCount; axisNum++)
        {
            float varmin = plotAxisMinima[axisNum];
            float varmax = plotAxisMaxima[axisNum];
            float val = varmin+part*((varmax-varmin)/float(nparts));

            float pt[3];
            pt[0] = dataTransforms[axisNum][0];
            pt[1] = dataTransforms[axisNum][1]*val+dataTransforms[axisNum][2];
            pt[2] = 0.0;
            for (int i = 0 ; i < PCP_CTX_BRIGHTNESS_LEVELS ; i++)
            {
                dataContextPoints[i]->InsertNextPoint(pt);
            }
        }
    }

    float gamma = parAxisAtts.GetContextGamma();
    if (gamma<.1)
        gamma=.1;
    if (gamma>10)
        gamma=10;

    for (int axis = 0; axis < axisCount-1; axis++)
    {
        // Find the maximum count in the bins
        int maxcount = 0;
        for (int bin=0; bin<nparts*nparts; bin++)
        {
            if (binnedAxisCounts[axis][bin] > maxcount)
                maxcount = binnedAxisCounts[axis][bin];
        }
        // Draw each bin as a polygon in the appropriately
        // colored (and layered) context polydata
        for (int a=0; a<nparts; a++)
        {
            for (int b=0; b<nparts; b++)
            {
                float alpha = float(binnedAxisCounts[axis][a*nparts+b]) /
                              float(maxcount);

                alpha = pow(alpha,1./gamma);

                int c = int(float(PCP_CTX_BRIGHTNESS_LEVELS-1) * alpha);

                if (c != 0)
                {
                    vtkIdType poly[4];
                    poly[0] = axis + axisCount * a;
                    poly[1] = (axis+1) + axisCount * b;
                    poly[2] = poly[1] + axisCount;
                    poly[3] = poly[0] + axisCount;
                    dataContextPolys[c]->InsertNextCell(4, poly);
                }
            }
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


