/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
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
//                              avtParallelCoordinatesFilter.C                      //
// ************************************************************************* //

#include <avtParallelCoordinatesFilter.h>

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
//  Method: avtParallelCoordinatesFilter
//
//  Purpose: Constructor for the avtParallelCoordinatesFilter class.
//
//  Arguments:
//      atts    The ParallelCoordinates plot attributes.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

avtParallelCoordinatesFilter::avtParallelCoordinatesFilter(ParallelCoordinatesAttributes &atts)
{
    parCoordsAtts = atts;
    parallelRank = PAR_Rank();
}


// ****************************************************************************
//  Method: ~avtParallelCoordinatesFilter
//
//  Purpose: Destructor for the avtParallelCoordinatesFilter class.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

avtParallelCoordinatesFilter::~avtParallelCoordinatesFilter()
{
    return;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::VerifyInput
//
//  Purpose: Verifies that the input is 3D data, throws an exception if not.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now accepts input data of any dimension.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************
 
void
avtParallelCoordinatesFilter::VerifyInput(void)
{
/*
    if  (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "ParallelCoordinates", " 3-D ");
    }
 */
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::PerformRestriction
//
//  Purpose: Restrict input domains if an interval tree is available.  Also set
//           up axis position information needed by the plot.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//    Added check for attribute consistency.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

avtPipelineSpecification_p
avtParallelCoordinatesFilter::PerformRestriction(avtPipelineSpecification_p in_spec)
{
    if (!parCoordsAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PR1: ParallelCoordinates plot attributes are inconsistent."
               << endl;

        return in_spec;
    }
        
    const char *inPipelineVar = in_spec->GetDataSpecification()->GetVariable();
    std::string curPipelineVar(inPipelineVar);
    
    stringVector curAxisVarNames = parCoordsAtts.GetOrderedAxisNames();
    doubleVector curAxisMinima = parCoordsAtts.GetAxisMinima();

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

    doubleVector curExtentMinima = parCoordsAtts.GetExtentMinima();
    doubleVector curExtentMaxima = parCoordsAtts.GetExtentMaxima();
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
//  Method: avtParallelCoordinatesFilter::PreExecute
//
//  Purpose: Computes the data extents of all relevant scalar variables.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Thu Oct 26 18:40:28 PDT 2006
//    No longer writes scratch file, which is no longer necessary.  Also added
//    support for non-uniform axis spacing.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//    Create colormap labels from both the datacurve/annotation labels and
//    the context labels.  Also initialize the context accumulation bins.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    domainCount = GetInputDataTree()->GetNumberOfLeaves();
    
    if (!parCoordsAtts.AttributesAreConsistent())
    {
        debug3 << "PCP/aPAF/PE1: ParallelCoordinates plot attributes are inconsistent."
               << endl;
        
        sendNullOutput = true;
        return;
    }

    stringVector curAxisVarNames = parCoordsAtts.GetOrderedAxisNames();
        
    axisCount = curAxisVarNames.size();
    sendNullOutput = false;

    ComputeCurrentDataExtentsOverAllDomains();
    
    if (sendNullOutput) return;
    
    int axisNum;
        
    if (sendNullOutput)
    {
        debug3 << "PCP/aPAF/PE2: ParallelCoordinates plot shown/selected axis marks "
               << "missing or out of order." << endl;
        return;
    }

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
    CreateLabels();

    // We need to create the colormap labels from the line, annotation,
    // and context labels.  This ordering must match what's created
    // in the avtParallelCoordinatesPlot, and the intent is that the context
    // is drawn underneath the annotations and individual lines.
    stringVector colorMapLabels;
    colorMapLabels.insert(colorMapLabels.end(),
                          contextLabels.begin(),contextLabels.end());
    colorMapLabels.insert(colorMapLabels.end(),
                          curveAndAxisLabels.begin(),curveAndAxisLabels.end());
    outAtts.SetLabels(colorMapLabels);
    
    // initialize things needed for drawing the context
    if (parCoordsAtts.GetDrawContext())
    {
        InitializePairwiseBins();
    }
}


// *****************************************************************************
//  Method: avtParallelCoordinatesFilter::PostExecute
//
// Purpose: Changes the vis window's spatial extents to match the viewport of
//          the plot's curves.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//     Mark Blair, Fri Feb 23 12:19:33 PST 2007
//     Axis attribute for outside queries now stored after filter is executed.
//
//     Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//     Draw the context, and clean up.
//
//     Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//     Adapted from Parallel Axis plot and repurposed into this new
//     Parallel Coordinates plot.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::PostExecute(void)
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

        spatialExtents[0] = 0.0; spatialExtents[1] = axisCount-1;
        spatialExtents[2] = 0.0; spatialExtents[3] = 1.0;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }
    else if (inAtts.GetCumulativeTrueSpatialExtents()->HasExtents())
    {
        inAtts.GetCumulativeTrueSpatialExtents()->CopyTo(spatialExtents);

        spatialExtents[0] = 0.0; spatialExtents[1] = axisCount-1;
        spatialExtents[2] = 0.0; spatialExtents[3] = 1.0;

        outAtts.GetCumulativeTrueSpatialExtents()->Set(spatialExtents);
    }

    outAtts.SetXLabel("");
    outAtts.SetYLabel("");

    outAtts.SetXUnits("");
    outAtts.SetYUnits("");

    if (parCoordsAtts.GetDrawContext())
    {
        DrawContext();
        CleanUpPairwiseBins();
    }
}

// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::ExecuteDataTree
//
//  Purpose: Processes one domain chunk of the input mesh into corresponding
//           curves of a ParallelCoordinates plot.
//
//  Arguments:
//      in_ds  :  The input dataset.
//      domain :  The domain number of the input dataset.
//
//  Returns: The output dataset (curves of the ParallelCoordinates plot).
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Mark Blair, Fri Jun  9 14:29:00 PDT 2006
//    Rewrote to eliminate use of the ParallelCoordinatePlot class.  Code
//    adapted from that class was added directly to the filter code.
//
//    Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//    Added check for attribute consistency.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Fri Mar 16 13:50:26 EDT 2007
//    Count each tuple in the appropriate pairwise axis bins.
//    Avoid drawing data curves (lines) if we didn't ask for them.
//    Renamed data set labels specific to data curves and axis annotations.
//
//    Jeremy Meredith, Wed Mar 21 18:22:08 EDT 2007
//    Added a new setting which will hide the lines until the extents
//    tool has actually been enabled and thresholded the range of one or
//    more axes.  This allows the "lines" to be used strictly as a "focus".
//
//    Jeremy Meredith, Fri May 18 09:41:30 EDT 2007
//    Only add the individual tuples to the data sets if the lines are
//    actually going to be drawn.  This lets situtions where only the context
//    is drawn scale easily to huge datasets without going into SR mode.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

avtDataTree_p 
avtParallelCoordinatesFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (sendNullOutput) return NULL;

    if (in_ds == NULL)
    {
        debug3 << "PCP/aPAF/EDT/1: ParallelCoordinates plot input is NULL." << endl;
        return NULL;
    }

    if (varTupleIndices.size() != axisCount)
    {
        debug3 << "PCP/aPAF/EDT/2: ParallelCoordinates plot internal data is "
               << "inconsistent." << endl;
        return NULL;
    }

    stringVector curAxisVarNames = parCoordsAtts.GetOrderedAxisNames();
    
    int cellArrayCount  = in_ds->GetCellData()->GetNumberOfArrays();
    int pointArrayCount = in_ds->GetPointData()->GetNumberOfArrays();
    int cellCount       = in_ds->GetNumberOfCells();
    int pointCount      = in_ds->GetNumberOfPoints();

    bool plotCellData  = (cellArrayCount > 0);
    int plotTupleCount = (plotCellData) ? cellCount : pointCount;

    if (cellArrayCount + pointArrayCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/3: No ParallelCoordinates plot input data arrays found."
               << endl;
        return NULL;
    }

    if (plotTupleCount == 0)
    {
        debug3 << "PCP/aPAF/EDT/4: ParallelCoordinates plot input data array "
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
            debug3 << "PCP/aPAF/EDT/5: ParallelCoordinates plot input data array "
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
        SetupParallelCoordinates(axisNum);
    }

    floatVector inputTuple = floatVector(axisCount);
    
    InitializeDataTupleInput();
    InitializeOutputDataSets();
    
    bool extentsApplied = false;
    for (int axisID = 0; axisID < axisCount; axisID++)
    {
        if (applySubranges[axisID])
            extentsApplied = true;
    }

    bool drawLines = false;
    if (parCoordsAtts.GetDrawLines() &&
        (!parCoordsAtts.GetDrawLinesOnlyIfExtentsOn() || extentsApplied))
    {
        drawLines = true;
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

    vtkDataSet **outputDataSets = new vtkDataSet *[1];

    outputDataSets[0] = dataCurvePolyData;

    avtDataTree *outputDataTree =
        new avtDataTree(1, outputDataSets, domain, curveAndAxisLabels);

    outputDataSets[0]->Delete();
    delete [] outputDataSets;

    return outputDataTree;
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::RefashionDataObjectInfo
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
avtParallelCoordinatesFilter::RefashionDataObjectInfo(void)
{
    avtDataAttributes &inAtts  = GetInput()->GetInfo().GetAttributes();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    
    outAtts.SetTopologicalDimension(inAtts.GetTopologicalDimension()-1);

    GetOutput()->GetInfo().GetValidity().InvalidateZones();

    outAtts.SetSpatialDimension(2);
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::CreateLabels
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
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::CreateLabels()
{
    curveAndAxisLabels.clear();

    curveAndAxisLabels.push_back("Data Curves");

    char str[100];
    contextLabels.clear();
    for (int i=0; i<PCP_CTX_BRIGHTNESS_LEVELS; i++)
    {
        sprintf(str, "Data Context %03d", i);
        contextLabels.push_back(str);
    }
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::ReleaseData
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
avtParallelCoordinatesFilter::ReleaseData(void)
{
    avtDataTreeStreamer::ReleaseData();
}


// ****************************************************************************
//  Method: avtParallelCoordinatesFilter::SetupParallelCoordinates
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
//    Mark Blair, Wed Dec 20 17:52:01 PST 2006
//    Added support for non-uniform axis spacing.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Removed the aesthetic 5% margin between data extrema and axis extrema.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
//    Jeremy Meredith, Fri Feb  1 17:58:21 EST 2008
//    Use actual values for extents, not normlized to 0..1.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::SetupParallelCoordinates (int plotAxisNum)
{
    const stringVector curAxisVarNames   = parCoordsAtts.GetOrderedAxisNames();
    const doubleVector curAxisMinima     = parCoordsAtts.GetAxisMinima();
    const doubleVector curAxisMaxima     = parCoordsAtts.GetAxisMaxima();
    const doubleVector curExtentMinima   = parCoordsAtts.GetExtentMinima();
    const doubleVector curExtentMaxima   = parCoordsAtts.GetExtentMaxima();

    double plotAxisMin   = curAxisMinima[plotAxisNum];
    double plotAxisRange = curAxisMaxima[plotAxisNum] - plotAxisMin;

    plotAxisMinima.push_back(plotAxisMin);
    plotAxisMaxima.push_back(curAxisMaxima[plotAxisNum]);


    // the old way expected the extents to be limited in the range (0,1)
    // but that's not very intuitive if you're setting them e.g. via
    // the cli, so instead, use the actual extents.
    subrangeMinima.push_back(curExtentMinima[plotAxisNum]);
    subrangeMaxima.push_back(curExtentMaxima[plotAxisNum]);
    //subrangeMinima.push_back(
    //    plotAxisMin + plotAxisRange*curExtentMinima[plotAxisNum]);
    //subrangeMaxima.push_back(
    //    plotAxisMin + plotAxisRange*curExtentMaxima[plotAxisNum]);
}


// *****************************************************************************
//  Method: avtParallelCoordinatesPlot::ComputeCurrentDataExtentsOverAllDomains
//
//  Purpose: Computes extent of each axis's scalar variable.
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::ComputeCurrentDataExtentsOverAllDomains()
{
    stringVector curAxisVarNames = parCoordsAtts.GetOrderedAxisNames();
    
    if (varTupleIndices.size() != curAxisVarNames.size())
    {
        debug3 << "PCP/aPAF/CCDEOAD/1: ParallelCoordinates plot internal data is "
               << "inconsistent." << endl;
        sendNullOutput = true;
        return;
    }

    doubleVector curAxisMinima   = parCoordsAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parCoordsAtts.GetAxisMaxima();

    int axisNum;
    std::string axisVarName;
    double dataAverage;
    double *axisMinimum, *axisMaximum;
    double varDataExtent[2];

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisVarName = curAxisVarNames[axisNum];

        avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
        GetDataExtents(varDataExtent, axisVarName.c_str());

        outAtts.GetCumulativeTrueDataExtents(axisVarName.c_str())->Set(varDataExtent);
        outAtts.SetUseForAxis(axisNum, axisVarName.c_str());

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
    
    parCoordsAtts.SetAxisMinima(curAxisMinima);
    parCoordsAtts.SetAxisMaxima(curAxisMaxima);
}



// *****************************************************************************
// Method: avtParallelCoordinatesFilter::InitializePlotAtts
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
//    Mark Blair, Wed Dec 20 17:52:01 PST 2006
//    Added support for non-uniform axis spacing.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::InitializePlotAtts()
{
    plotAxisMinima.clear(); plotAxisMaxima.clear();
    subrangeMinima.clear(); subrangeMaxima.clear();
}


// *****************************************************************************
// Method: avtParallelCoordinatesPlot:InitializeDataTupleInput
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
//    Mark Blair, Wed Nov  8 16:01:27 PST 2006
//    Applies extents to input data only when Extents tool is active.
//
//    Mark Blair, Wed Dec 20 17:52:01 PST 2006
//    Added support for non-uniform axis spacing.
//
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::InitializeDataTupleInput()
{
    double plotWidth = axisCount-1;

    double axisHeight = 1;

    dataTransforms.clear();

    int axisNum;
    double axisScale;
    doubleVector *dataTransform;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        dataTransform = new doubleVector;

        axisScale = axisHeight / (plotAxisMaxima[axisNum] - plotAxisMinima[axisNum]);

        dataTransform->push_back(plotWidth*double(axisNum)/double(axisCount-1));
        dataTransform->push_back(axisScale);
        dataTransform->push_back(0 - axisScale*plotAxisMinima[axisNum]);

        dataTransforms.push_back(*dataTransform);
    }

    applySubranges.clear();

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        applySubranges.push_back(false);
    }

    double axisSpan, axisMinSpan, axisMaxSpan;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisSpan = plotAxisMaxima[axisNum] - plotAxisMinima[axisNum];

        axisMinSpan = subrangeMinima[axisNum] - plotAxisMinima[axisNum];
        axisMaxSpan = plotAxisMaxima[axisNum] - subrangeMaxima[axisNum];

        if ((axisMinSpan/axisSpan > 0.0001) || (axisMaxSpan/axisSpan > 0.0001))
        {
            applySubranges[axisNum] = true;
        }
    }
    
    outputCurveCount = 0;
}


// *****************************************************************************
// Method: avtParallelCoordinatesPlot:InitializeOutputDataSets
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
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::InitializeOutputDataSets()
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
}


// *****************************************************************************
// Method: avtParallelCoordinatesFilter::InputDataTuple
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
//    Mark Blair, Fri Feb 23 12:19:33 PST 2007
//    Now supports all variable axis spacing and axis group conventions.
//
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::InputDataTuple(const floatVector &inputTuple)
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

    for (axisID = 0; axisID < axisCount; axisID++)
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
// Method: avtParallelCoordinatesFilter::CountDataTuple
//
// Purpose: Takes the n-dimensional data tuple and increments the appropriate
//          bin for each pair of consecutive axes.
//
// Programmer: Jeremy Meredith
// Creation:   March 14, 2007
//
// Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::CountDataTuple(const floatVector &inputTuple)
{
    if (!parCoordsAtts.GetDrawContext())
        return;

    int nparts = parCoordsAtts.GetContextNumPartitions();
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
// Method: avtParallelCoordinatesFilter::DrawDataCurves
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
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.  Fixed a bug in number of lines drawn.
//
// *****************************************************************************

void
avtParallelCoordinatesFilter::DrawDataCurves()
{
    vtkIdType vtkPointIDs[2];

    vtkIdType segmentCount = axisCount-1;
    vtkIdType firstVTKPointID, segmentNum;

    for (int curveNum = 0; curveNum < outputCurveCount; curveNum++)
    {
        firstVTKPointID = (vtkIdType)(curveNum * axisCount);

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


// ****************************************************************************
//  Method:  avtParallelCoordinatesFilter::DrawContext
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
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
// ****************************************************************************

void
avtParallelCoordinatesFilter::DrawContext()
{
    int nparts = parCoordsAtts.GetContextNumPartitions();

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

    float gamma = parCoordsAtts.GetContextGamma();
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

#if defined(__GNUC__) && ((__GNUC__ < 3) || (__GNUC__ == 3 && __GNUC_MINOR__ < 2) || (__GNUC__ == 3 && __GNUC_MINOR__ == 2 && __GNUC_PATCHLEVEL__ == 0))
                alpha = pow(alpha,1./gamma);
#else
                alpha = powf(alpha,1.f/gamma);
#endif

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
//  Method:  avtParallelCoordinatesFilter::InitializePairwiseBins
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
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::InitializePairwiseBins()
{
    int nparts = parCoordsAtts.GetContextNumPartitions();
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
//  Method:  avtParallelCoordinatesFilter::CleanUpPairwiseBins
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
//  Modifications:
//    Jeremy Meredith, Thu Jan 31 13:56:50 EST 2008
//    Adapted from Parallel Axis plot and repurposed into this new
//    Parallel Coordinates plot.
//
// ****************************************************************************

void
avtParallelCoordinatesFilter::CleanUpPairwiseBins()
{
    for (int i=0; i<axisCount; i++)
    {
        delete[] binnedAxisCounts[i];
    }
    delete[] binnedAxisCounts;
    binnedAxisCounts = NULL;
}


