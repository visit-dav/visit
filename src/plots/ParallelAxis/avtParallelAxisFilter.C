// ************************************************************************* //
//                              avtParallelAxisFilter.C                      //
// ************************************************************************* //

#include <avtParallelAxisFilter.h>

#include <ParallelAxisAttributes.h>
#include <PlotInfoAttributes.h>

#include <limits.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <visitstream.h>

#include <unistd.h>
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
    drewAnnotations = false;
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
// ****************************************************************************
 
void
avtParallelAxisFilter::VerifyInput(void)
{
    if  (GetInput()->GetInfo().GetAttributes().GetSpatialDimension() != 3)
    {
        EXCEPTION2(InvalidDimensionsException, "ParallelAxis", " 3-D ");
    }
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
        return in_spec;

    const char *inPipelineVar = in_spec->GetDataSpecification()->GetVariable();
    std::string curPipelineVar(inPipelineVar);
    
    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    doubleVector curAxisMinima = parAxisAtts.GetAxisMinima();

    varTupleIndices.clear();

    for (int axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
    {
        if (curAxisVarNames[axisNum] == curPipelineVar)
        {
            varTupleIndices.push_back(0);
        }
        else
        {
            varTupleIndices.push_back(-1);
        }
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
// *****************************************************************************

void
avtParallelAxisFilter::PreExecute(void)
{
    avtDatasetToDatasetFilter::PreExecute();

    domainCount = GetInputDataTree()->GetNumberOfLeaves();

    ComputeCurrentDataExtentsOverAllDomains();

/*  Temporarily disable this until PlotInfoAttributes can be accessed by the
    Extents tool.  Use kludge scratch file in the meantime.

    StoreDataExtentsForOutsideQueries();
*/

    WriteAxisVariableNamesAndExtentsFile();  // FIX ME!  Kludge scratch file.

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
   
    CreateLabels();
    outAtts.SetLabels(layerLabels);
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

    outAtts.SetXUnits(""); outAtts.SetYUnits("");
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
//      Mark Blair, Fri Jun  9 14:29:00 PDT 2006
//      Rewrote to eliminate use of the ParallelCoordinatePlot class.  Code
//      adapted from that class was added directly to the filter code.
//
//      Mark Blair, Wed Aug 16 16:46:00 PDT 2006
//      Added check for attribute consistency.
//
// ****************************************************************************

avtDataTree_p 
avtParallelAxisFilter::ExecuteDataTree(vtkDataSet *in_ds, int domain, string label)
{
    if (!parAxisAtts.AttributesAreConsistent())
    {
        debug1 << "ParallelAxis plot attributes are inconsistent." << endl;
        return NULL;
    }

    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();

    axisCount = curAxisVarNames.size();

    // if (domain == 0)
    // {
    //     cout << "Entered ExecuteDataTree: domain = " << domain << endl;
    // }

    if (in_ds == NULL) return NULL;
    if (varTupleIndices.size() != axisCount) return NULL;

    int cellArrayCount  = in_ds->GetCellData()->GetNumberOfArrays();
    int pointArrayCount = in_ds->GetPointData()->GetNumberOfArrays();
    int cellCount       = in_ds->GetNumberOfCells();
    int pointCount      = in_ds->GetNumberOfPoints();

    bool plotCellData  = (cellArrayCount > 0);
    int plotTupleCount = (plotCellData) ? cellCount : pointCount;

    if (cellArrayCount + pointArrayCount == 0) return NULL;
    if (plotTupleCount == 0) return NULL;

    // If the input contains both cell data and point data, then by convention
    // the cell data takes precedence.  In this case, the value of a point
    // variable that is plotted for a given cell, by convention, is the average
    // value of that variable over all vertices of the cell.  (mb)

    const std::string pipeVariableName = pipelineVariable;
    
    int axisNum, tupleCount, tupleNum, varTupleIndex, componentCount;
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
            debug1 << "Input data array " << arrayName << " is bad or missing."
                   << endl;
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

    InitializeDataTupleInput();
    InitializeOutputDataSets();
    
    floatVector inputTuple = floatVector(axisCount);

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
        }
    }

    if (plotCellData && (pointArrayCount > 0))
    {
        pointIdList->Delete();
    }
    
    DrawDataCurves();

    if (parallelRank == 0)
    {
        if (!drewAnnotations)
        {
            DrawCoordinateAxes();
            DrawCoordinateAxisLabels();
            DrawCoordinateAxisTitles();

            drewAnnotations = true;
        }
    }

    vtkDataSet **outputDataSets = new vtkDataSet *[4];

    outputDataSets[0] = dataCurvePolyData;
    outputDataSets[1] = axisPolyData;
    outputDataSets[2] = titlePolyData;
    outputDataSets[3] = subrangePolyData;

    avtDataTree *outputDataTree =
        new avtDataTree(4, outputDataSets, domain, layerLabels);

    outputDataSets[0]->Delete();
    outputDataSets[1]->Delete();
    outputDataSets[2]->Delete();
    outputDataSets[3]->Delete();

    delete [] outputDataSets;

    // if (domain == domainCount - 1)
    // {
    //     cout << "Leaving ExecuteDataTree: (highest) domain = " << domain << endl;
    // }

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

    outAtts.SetSpatialDimension (2);
}


// ****************************************************************************
//  Method: avtParallelAxisFilter::CreateLabels
//
//  Purpose: (Probably unnecessary; vestigial holdover from a bygone era).
//
//  Programmer: Mark Blair
//  Creation:   Mon Mar 27 18:24:00 PST 2006
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelAxisFilter::CreateLabels()
{
    layerLabels.clear();

    layerLabels.push_back("Data Curves");
    layerLabels.push_back("Axes and Bounds");
    layerLabels.push_back("Axis Titles");
    layerLabels.push_back("Selector Arrows");
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
// ****************************************************************************

void
avtParallelAxisFilter::SetupParallelAxis (int plotAxisNum)
{
    const stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    const doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    const doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();
    const doubleVector curExtentMinima = parAxisAtts.GetExtentMinima();
    const doubleVector curExtentMaxima = parAxisAtts.GetExtentMaxima();

    double trueAxisMin   = curAxisMinima[plotAxisNum];
    double trueAxisMax   = curAxisMaxima[plotAxisNum];
    double trueAxisRange = trueAxisMax - trueAxisMin;
    double plotAxisRange = trueAxisRange * 1.1;
    double rangeMargin   = trueAxisRange * 0.05;
    double plotAxisMin   = trueAxisMin - rangeMargin;
    double plotAxisMax   = trueAxisMax + rangeMargin;

    plotAxisMinima.push_back(plotAxisMin);
    plotAxisMaxima.push_back(plotAxisMax);

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
    doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();

    if (varTupleIndices.size() != curAxisVarNames.size()) return;

    int axisNum;
    std::string axisVarName;
    double dataAverage;
    double *axisMinimum, *axisMaximum;
    double varDataExtent[2];

    for (axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++)
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
//  Method: avtParallelAxisPlot::StoreDataExtentsForOutsideQueries
//
//  Purpose: Stores name of each axis's scalar variable and that variable's
//           extent in a data structure that can be queried by viewer functions.
//
//  Programmer: Mark Blair
//  Creation:   Fri Sep  1 14:51:00 PDT 2006
//
//  Modifications:
//
// *****************************************************************************

void
avtParallelAxisFilter::StoreDataExtentsForOutsideQueries()
{
    stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();
    
    const char *axisVarName;
    int axisVarNameLen, axisNum, charNum;

    extentsArray.clear();

    for (axisNum = 0; axisNum < curAxisMinima.size(); axisNum++)
    {
        axisVarName = curAxisVarNames[axisNum].c_str();
        axisVarNameLen = strlen(axisVarName);
        
        for (charNum = 0; charNum < axisVarNameLen; charNum++)
        {
            extentsArray.push_back((double)axisVarName[charNum]);
        }
        
        extentsArray.push_back(0.0);

        extentsArray.push_back(curAxisMinima[axisNum]);
        extentsArray.push_back(curAxisMaxima[axisNum]);
    }

    extentsArray.push_back(0.0);
    
    PlotInfoAttributes plotInfoAtts;
    plotInfoAtts.SetOutputArray(extentsArray);
    GetOutput()->GetInfo().GetAttributes().SetPlotInfoAtts(&plotInfoAtts);
}


// *****************************************************************************
//  Method: avtParallelAxisFilter::WriteAxisVariableNamesAndExtentsFile
//
//  Purpose: Writes each axis's scalar variable name, along with the extent of
//           the variable, into a file for use by the Extents tool.
//
//  Note: This is a KLUDGE intended for use only until PlotInfoAttributes can be
//        used to communicate with the Extents tool.  Scratch file communication
//        will of course fail if the viewer and the engine are running on
//        different file systems.
//
//  Programmer: Mark Blair
//  Creation:   Fri Sep 02 14:37:00 PDT 2005
//
//  Modifications:
//
// *****************************************************************************

bool avtParallelAxisFilter::WriteAxisVariableNamesAndExtentsFile()
{
    FILE *avdFileStream;
    
    unlink (AXIS_VAR_DATA_FILE_NAME);

    if ((avdFileStream = fopen(AXIS_VAR_DATA_FILE_NAME, "w")) == NULL) return false;

    const stringVector curAxisVarNames = parAxisAtts.GetOrderedAxisNames();
    const doubleVector curAxisMinima   = parAxisAtts.GetAxisMinima();
    const doubleVector curAxisMaxima   = parAxisAtts.GetAxisMaxima();

    for (int axisNum = 0; axisNum < curAxisVarNames.size(); axisNum++ ) {
        fprintf(avdFileStream, "%s %f %f\n", curAxisVarNames[axisNum].c_str(),
        curAxisMinima[axisNum], curAxisMaxima[axisNum]);
    }

    fclose(avdFileStream);

    return true;
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
// *****************************************************************************

void
avtParallelAxisFilter::InitializePlotAtts()
{
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
// *****************************************************************************

void
avtParallelAxisFilter::InitializeDataTupleInput()
{
    useVerticalText = (axisCount > PCP_MAX_HORIZONTAL_TITLE_AXES);

    double leftAxisX = PCP_LEFT_AXIS_X_FRACTION;
    double axisSpacing = (PCP_RIGHT_AXIS_X_FRACTION - PCP_LEFT_AXIS_X_FRACTION) /
                         (double)(axisCount-1);
    double tickSpacing;

    if (useVerticalText)
    {
        bottomAxisY = PCP_V_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_V_TOP_AXIS_Y_FRACTION;

        tickSpacing = (PCP_V_TOP_AXIS_Y_FRACTION - PCP_V_BOTTOM_AXIS_Y_FRACTION) /
                      (double)PCP_DEFAULT_TICK_MARK_INTERVALS;
    }
    else
    {
        bottomAxisY = PCP_H_BOTTOM_AXIS_Y_FRACTION;
        topAxisY    = PCP_H_TOP_AXIS_Y_FRACTION;

        tickSpacing = (PCP_H_TOP_AXIS_Y_FRACTION - PCP_H_BOTTOM_AXIS_Y_FRACTION) /
                      (double)PCP_DEFAULT_TICK_MARK_INTERVALS;
    }

    double axisHeight = topAxisY - bottomAxisY;

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

        dataTransform->push_back((double)axisNum*axisSpacing + leftAxisX);
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
    double axisSpan, axisMinSpan, axisMaxSpan, minArrowOffset, maxArrowOffset;
    doubleVector dTrans;

    if (useVerticalText)
    {
        moveOffset = PCP_VERTICAL_MOVE_OFFSET_FRACTION;

        movedAxisTitleY = bottomAxisY - moveOffset;
        movedTopLabelY  = topAxisY    + moveOffset;
    }

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisSpan = plotAxisMaxima[axisNum] - plotAxisMinima[axisNum];

        axisMinSpan = subrangeMinima[axisNum] - plotAxisMinima[axisNum];
        axisMaxSpan = plotAxisMaxima[axisNum] - subrangeMaxima[axisNum];

        if ((axisMinSpan/axisSpan > 0.0001) || (axisMaxSpan/axisSpan > 0.0001))
        {
            applySubranges[axisNum] = true;

            dTrans = dataTransforms[axisNum];

            minArrowOffset = axisMinSpan * dTrans[1];
            maxArrowOffset = axisMaxSpan * dTrans[1];

            if (useVerticalText)
            {
                drawBottomLabels[axisNum] = false;
                drawBottomBounds[axisNum] = true;
                drawTopBounds[axisNum]    = true;

                if (minArrowOffset < arrowHeight) moveTitles[axisNum]    = true;
                if (maxArrowOffset < arrowHeight) moveTopLabels[axisNum] = true;
            }
            else
            {
                if (minArrowOffset >= arrowHeight) drawBottomBounds[axisNum] = true;
                if (maxArrowOffset >= arrowHeight) drawTopBounds[axisNum]    = true;
            }
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
// *****************************************************************************

void
avtParallelAxisFilter::InputDataTuple(const floatVector &inputTuple)
{
    int axisNum;
    double inputCoord;
    doubleVector dTrans;

    float outputCoords[3];
    outputCoords[2] = 0.0;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (applySubranges[axisNum])
        {
            if (inputTuple[axisNum] < subrangeMinima[axisNum]) break;
            if (inputTuple[axisNum] > subrangeMaxima[axisNum]) break;
        }
    }
    
    if (axisNum < axisCount) return;

    for (axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if ((inputCoord = (double)inputTuple[axisNum]) < plotAxisMinima[axisNum])
        {
            inputCoord = plotAxisMinima[axisNum];
        }
        else if (inputCoord > plotAxisMaxima[axisNum])
        {
            inputCoord = plotAxisMaxima[axisNum];
        }

        dTrans = dataTransforms[axisNum];

        outputCoords[0] = (float)dTrans[0];
        outputCoords[1] = (float)(dTrans[1]*inputCoord + dTrans[2]);
        
        dataCurvePoints->InsertNextPoint(outputCoords);
    }
    
    outputCurveCount++;
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
// *****************************************************************************

void
avtParallelAxisFilter::DrawDataCurves()
{
    vtkIdType vtkPointIDs[2];

    vtkIdType segmentCount = (vtkIdType)(axisCount - 1);
    vtkIdType firstVTKPointID, segmentNum;

    for (int curveNum = 0; curveNum < outputCurveCount; curveNum++)
    {
        firstVTKPointID = (vtkIdType)(curveNum * axisCount);

        for (segmentNum = 0; segmentNum < segmentCount; segmentNum++)
        {
            vtkPointIDs[0] = firstVTKPointID + segmentNum;
            vtkPointIDs[1] = vtkPointIDs[0] + 1;

            dataCurveLines->InsertNextCell(2, vtkPointIDs);
            dataCurveVerts->InsertNextCell(1, vtkPointIDs);
        }

        dataCurveVerts->InsertNextCell(1, &vtkPointIDs[1]);
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

    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisX = dataTransforms[axisNum][0];

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

    int segmentCount = axisCount * tickMarkIntervals;

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
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisLabels()
{
    if (textPlotter == NULL) textPlotter = new PortableFont;

    bool drawIt, centerIt;
    int labelLen;
    double minOrMax, axisX, xOffset, labelX, labelY;
    PortableFont::PF_ORIENTATION orientation;
    std::vector<floatVector> *strokeList = new std::vector<floatVector>;
    char axisLabel[81];

    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        minOrMax = plotAxisMinima[axisNum];
        labelY = (moveTitles[axisNum]) ? movedAxisTitleY : bottomLabelY;
        drawIt = drawBottomLabels[axisNum];

        for (int labelNum = 0; labelNum < 2; labelNum++)
        {
            if (drawIt)
            {
                textPlotter->DoubleNumericalString(axisLabel, minOrMax);
                labelLen = strlen(axisLabel);

                axisX = dataTransforms[axisNum][0];

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
                        xOffset = (drawBottomBounds[axisNum]) ?
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

                    if (axisNum == 0)
                    {
                        if (labelLen > 2)
                        {
                            labelX = axisX - labelCharWidth*1.5;
                            centerIt = false;
                        }
                    }
                    else if (axisNum == axisCount-1)
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

            minOrMax = plotAxisMaxima[axisNum];
            labelY = (moveTopLabels[axisNum]) ? movedTopLabelY : topLabelY;
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
    int axisVertexID = axisCount * tickMarkIntervals * 2;

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
// *****************************************************************************

void
avtParallelAxisFilter::DrawCoordinateAxisTitles()
{
    if (textPlotter == NULL) textPlotter = new PortableFont;

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

    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        axisX = dataTransforms[axisNum][0];

        if (useVerticalText)
        {
            titleY = (moveTitles[axisNum]) ? movedAxisTitleY : axisTitleY;
            titleX = axisX + axisTitleXOff;
        }
        else
        {
            if (axisNum == 0)
            {
                titleX = axisX - titleCharWidth;
                centerIt = false;
            }
            else if (axisNum == axisCount-1)
            {
                titleX = axisX -
                    (double)(plotAxisTitles[axisNum].length()-1)*titleCharWidth*1.5;
                centerIt = false;
            }
            else
            {
                titleX = axisX;
                centerIt = true;
            }
        }

        strcpy (axisTitle, plotAxisTitles[axisNum].c_str());

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
//        earlier.
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
    int axisNum, boundNum, boundLabelLen;
    double subrangeMinY, subrangeMaxY, halfSegmentLen, verticalOffset;
    double minOrMax, axisX, xOffset, boundX, boundY;
    PortableFont::PF_ORIENTATION orientation;
    doubleVector dTrans;
    std::vector<floatVector> *strokeList = new std::vector<floatVector>;
    char boundLabel[81];

    float outputCoords[3];
    outputCoords[2] = 0.0;


    for (int axisNum = 0; axisNum < axisCount; axisNum++)
    {
        if (!applySubranges[axisNum]) continue;

        dTrans = dataTransforms[axisNum];

        axisX = dTrans[0];
        subrangeMinY = dTrans[1]*subrangeMinima[axisNum] + dTrans[2];
        subrangeMaxY = dTrans[1]*subrangeMaxima[axisNum] + dTrans[2];

        minOrMax = subrangeMinima[axisNum];
        boundY = (moveTitles[axisNum]) ? movedAxisTitleY : bottomBoundY;
        drawIt = drawBottomBounds[axisNum];

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

                    if (axisNum == 0)
                    {
                        if (boundLabelLen > 2)
                        {
                            boundX = axisX - boundCharWidth*1.5;
                            centerIt = false;
                        }
                    }
                    else if (axisNum == axisCount-1)
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

            minOrMax = subrangeMaxima[axisNum];
            boundY = (moveTopLabels[axisNum]) ? movedTopLabelY : topBoundY;
            drawIt = drawTopBounds[axisNum];
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
