// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtDirectDatabaseQOTFilter.C                        //
// ************************************************************************* //

#include <string>

#include <avtDirectDatabaseQOTFilter.h>
#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>

#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkRectilinearGrid.h>
#include <vtkVisItUtility.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>
#include <avtCommonDataFunctions.h>

#include <VisItException.h>
#include <DebugStream.h>
#include <MapNode.h>


// ****************************************************************************
//  Method: avtDirectDatabaseQOTFilter constructor
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Alister Maguire 
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

avtDirectDatabaseQOTFilter::avtDirectDatabaseQOTFilter(const AttributeGroup *a)
 : avtQueryOverTimeFilter(a)
{
    success            = true;
    finalOutputCreated = false;
    useTimeForXAxis    = true;
    useVarForYAxis     = false;
    YLabel             = "";

    //
    // Let's first try to retreive some information about the query. 
    //
    TRY
    {
        QueryAttributes qatts     = atts.GetQueryAtts();
        avtDataObjectQuery *query = avtQueryFactory::Instance()->
            CreateQuery(&qatts);

        if (query->GetShortDescription() != NULL)
        {
            YLabel = query->GetShortDescription();
        }
        else
        {
            YLabel = qatts.GetName();
        }

        const MapNode &tqs = query->GetTimeCurveSpecs();
        useTimeForXAxis    = tqs.GetEntry("useTimeForXAxis")->AsBool();
        useVarForYAxis     = tqs.GetEntry("useVarForYAxis")->AsBool();
        delete query;
    }
    CATCHALL
    {
        debug1 << "There was a problem trying to instantiate a query for "
               << "a query over time.  Stifling the error handling, because "
               << "this problem will be caught later when we are better "
               << "prepared to do error handling." << endl;
    }
    ENDTRY
}


// ****************************************************************************
//  Method: avtDirectDatabaseQOTFilter destructor
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
// ****************************************************************************

avtDirectDatabaseQOTFilter::~avtDirectDatabaseQOTFilter()
{
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
// ****************************************************************************

avtFilter *
avtDirectDatabaseQOTFilter::Create(const AttributeGroup *atts)
{
    return new avtDirectDatabaseQOTFilter(atts);
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::Execute
//
//  Purpose:
//      Construct our time query data tree. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDatabaseQOTFilter::Execute(void)
{
    //
    // Assume success until proven otherwise. 
    //
    success = true;

    if (PAR_Rank() == 0)
    {
        avtDataTree_p dataTree = GetInputDataTree();

        bool multiCurve = false;
        if (atts.GetQueryAtts().GetQueryInputParams().
            HasNumericEntry("curve_plot_type"))
        {
            multiCurve = (atts.GetQueryAtts().GetQueryInputParams().
                GetEntry("curve_plot_type")->ToInt() == 1);
        }

        int numLeaves = 0;
        vtkDataSet **leaves = dataTree->GetAllLeaves(numLeaves);

        if (numLeaves > 0)
        {
            vtkPolyData *QOTData = (vtkPolyData *) leaves[0];

            if (QOTData == NULL)
            {
                debug1 << "Direct Database QOT leaf is NULL!" << endl;

                SetOutputDataTree(new avtDataTree());
            }

            vtkPolyData *refined = VerifyAndRefineTimesteps(QOTData);
            avtDataTree_p tree   = ConstructCurveTree(refined, multiCurve);
            SetOutputDataTree(tree);

            if (refined != NULL)
            {
                refined->Delete();
            }

            delete [] leaves; 
        }
        else
        {
            debug1 << "Direct Database QOT recieved no leaves!" << endl;

            SetOutputDataTree(new avtDataTree());
        }
    }
    else
    {
        SetOutputDataTree(new avtDataTree());
    }

    finalOutputCreated = true;
    UpdateDataObjectInfo();
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::VerifyAndRefineTimesteps
//
//  Purpose:
//      Verify that all timesteps have been retrieved. If we are
//      missing any, then let's reduce our dataset to only include
//      the valid data, and let the user know which timesteps
//      were skipped. 
//
//      Note: timesteps that encountered errors will have added
//            NaN values to the associated data positions. 
//
//  Arguments:
//      polyData    The polydata containing the curves. 
//
//  Returns:
//      A vtkPolyData object only containing valid curves. 
//
//  Programmer: Alister Maguire
//  Creation:   Mon Sep 30 14:17:20 MST 2019
//
//  Modifications:
//
//    Alister Maguire, Mon Feb 24 13:23:22 MST 2020
//    Modified to handle vectors, arrays, and tensors in the same manner
//    that the TimeLoopQOTFilter works.
//
// ****************************************************************************

vtkPolyData *
avtDirectDatabaseQOTFilter::VerifyAndRefineTimesteps(vtkPolyData *inPolyData)
{
    vtkPolyData *outPolyData = vtkPolyData::New();
    outPolyData->ShallowCopy(inPolyData);

    vtkPointData *inPtData = inPolyData->GetPointData();
    vtkPoints *inPts       = inPolyData->GetPoints();

    if (inPtData ==  NULL || inPts == NULL)
    {
        return outPolyData;
    }

    int numCurves = inPtData->GetNumberOfArrays();

    if (numCurves == 0)
    { 
        return outPolyData;
    }

    const int numPts  = inPtData->GetNumberOfTuples();
    const int stride  = atts.GetStride();
    const int startT  = atts.GetStartTime();
    const int stopT   = atts.GetEndTime();

    //
    // We need to check if any of the arrays have multiple
    // components. If so, these require special treatment.
    //
    int maxComp = 0; 
    for (int c = 0; c < numCurves; ++c)
    {
        int ncTemp = inPtData->GetArray(c)->GetNumberOfComponents();
        maxComp    = ncTemp > maxComp ? ncTemp : maxComp;
    }

    //
    // First pass: look for invalid data and mark their locations
    // and record their time states.
    //
    bool missingData = false;
    intVector invalidStateList;
    invalidStateList.reserve(numPts);

    boolVector isValid;
    isValid.resize(numPts, true);

    double coord[] = {0.0, 0.0, 0.0};
    double *tupleTemp = new double[maxComp];

    for (int i = 0; i < maxComp; ++i)
    {
        tupleTemp[i] = 0.0;
    }

    //
    // In cases with multiple variables, only time states that 
    // are valid across ALL variables will be kept.
    //
    for (int c = 0; c < numCurves; ++c)
    {
        vtkFloatArray *inCurve = 
            (vtkFloatArray *) inPtData->GetArray(c);

        int ts = startT;
        for (int i = 0; i < numPts; ++i, ts += stride)
        {
            inCurve->GetTuple(i, tupleTemp);

            //
            // Invalid states will contain NaN values. 
            //
            if (visitIsNan(tupleTemp[0]))
            {
                missingData = true;
 
                if (isValid[i])
                {
                    isValid[i]  = false;
                    invalidStateList.push_back(ts);
                }
            }
        }
    }

    //
    // There are two cases that require more work:
    //     1. We have missing data that needs to be handled and reported.
    //     2. We have vectors, tensors, or arrays that need to be reduced
    //        to a single scalar.
    //
    if (missingData || maxComp > 1)
    {
        int numInvalid = invalidStateList.size();
        int numValid   = numPts - numInvalid;

        if (missingData)
        {
            //
            // Report the missing timesteps. 
            //
            std::ostringstream osm;
            osm << "\nQueryOverTime (" << atts.GetQueryAtts().GetName().c_str()
                << ") experienced\n"
                << "problems with the following timesteps and \n"
                << "skipped them while generating the curve:\n   ";
            
            for (int j = 0; j < numInvalid; j++)
            {
                osm << invalidStateList[j] << " ";
            }
            debug4 << osm.str() << endl;
            avtCallback::IssueWarning(osm.str().c_str());
        }

        if (numValid == 0)
        {
            //
            // We have no valid output. Let's not do any work here. 
            //
            outPolyData->Delete();
            vtkPolyData *empty = vtkPolyData::New();
            return empty;
        }

        vtkPoints *outPts = vtkPoints::New();

        //
        // If we're missing data, this will be an over-estimate, but
        // it should never be an under-estimate. 
        //
        outPts->Allocate(numPts);

        //
        // Second pass: re-write the arrays so that they only contain
        // valid time states. 
        //
        vtkPointData *outPtData = outPolyData->GetPointData();

        for (int c = 0; c < numCurves; ++c) 
        {
            vtkFloatArray *inCurve = 
                (vtkFloatArray *) inPtData->GetArray(c);

            const char *name = inCurve->GetName();
        
            vtkFloatArray *outCurve = vtkFloatArray::New();
            outCurve->SetNumberOfTuples(numValid);
            outCurve->SetNumberOfComponents(1);
            outCurve->SetName(name);
        
            int vIdx = 0;
            for (int i = 0; i < numPts; ++i)
            {
                if (isValid[i])
                {
                    inCurve->GetTuple(i, tupleTemp);
                    int numComp = inCurve->GetNumberOfComponents();

                    //
                    // We handle scalars, vectors, arrays, and tensors
                    // differently.
                    //     Scalars: use the raw value.
                    //     Vectors: use the magnitude.
                    //     Tensors/Arrays: use the major eigenvalue.
                    //
                    if (numComp == 1)
                    {
                        outCurve->SetTuple1(vIdx++, tupleTemp[0]);
                    }
                    else if (numComp < 9)
                    {
                        float mag = 0.0;
                        for (int ti = 0; ti < numComp; ++ti)
                        {
                            mag += tupleTemp[ti] * tupleTemp[ti];
                        }
                        outCurve->SetTuple1(vIdx++, sqrt(mag));
                    }
                    else if (numComp >= 9)
                    {
                        outCurve->SetTuple1(vIdx++, MajorEigenvalue(tupleTemp));
                    }
                    
                    if (c == 0) 
                    {
                        inPts->GetPoint(i, coord);        
                        outPts->InsertNextPoint(coord[0], coord[1], coord[2]);
                    }
                }
            }

            outPtData->RemoveArray(name);
            if (c == 0)
            {
                outPtData->SetScalars(outCurve);
            }
            else
            {
                outPtData->AddArray(outCurve);
            }

            outCurve->Delete();
        }

        outPolyData->SetPoints(outPts);
        outPts->Delete();
    }

    delete [] tupleTemp;

    return outPolyData;
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::ConstructCurveTree
//
//  Purpose:
//      Construct a tree from the time query curves. 
//
//  Arguments:
//      polyData             The polydata containing the curves. 
//      doMultiCurvePlot     Whether or not to do a multi curve plot. 
//
//  Returns:
//      A data tree containing the curves. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

avtDataTree_p
avtDirectDatabaseQOTFilter::ConstructCurveTree(vtkPolyData *polyData,
                                               const bool doMultiCurvePlot)
{
    vtkPointData *inPtData = polyData->GetPointData();
    vtkPoints *inPts       = polyData->GetPoints();

    if (inPtData ==  NULL || inPts == NULL ||
        inPtData->GetScalars() == NULL)
    {
        debug1 << "avtDirectDatabaseQOTFilter: missing curves and/or points."
            << endl;
        return new avtDataTree();
    }

    int numCurves = inPtData->GetNumberOfArrays();
    int numPts    = inPtData->GetScalars()->GetNumberOfTuples();

    if (numPts == 0 || numCurves == 0)
    {
        success = false;
        debug2 << "avtDirectDatabaseQOTFilter: missing curves and/or points" 
            << endl;
        return new avtDataTree();
    }

    if (numCurves == 1)
    {
        vtkFloatArray *curve = 
            (vtkFloatArray *) inPtData->GetScalars();

        if (curve == NULL)
        {
            success = false;
            char msg[512];
            snprintf(msg, 512, "VisIt was unable to retreive data for the "
                "following variable: %s\n", curve->GetName());
            avtCallback::IssueWarning(msg);
            return new avtDataTree();
        }

        vtkRectilinearGrid *rgrid = vtkVisItUtility::Create1DRGrid(numPts);
        rgrid->SetDimensions(numPts, 1 , 1);

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts);
        scalars->SetName(curve->GetName());

        double coord[] = {0.0, 0.0, 0.0};

        for (int i = 0; i < numPts; ++i)
        {
            inPts->GetPoint(i, coord);        
            xCoords->SetTuple1(i, coord[0]);
        }

        scalars->ShallowCopy(curve);
        rgrid->GetPointData()->SetScalars(scalars);

        avtDataTree_p tree = new avtDataTree(rgrid, 0);

        if (rgrid != NULL)
        { 
            rgrid->Delete();
        }

        scalars->Delete(); 
        return tree;
    }
    else if (doMultiCurvePlot)
    {
        vtkRectilinearGrid *rgrid =
            vtkVisItUtility::CreateEmptyRGrid(numPts, numCurves, 1, VTK_FLOAT);

        vtkDataArray *xCoords   = rgrid->GetXCoordinates();
        vtkDataArray *yCoords   = rgrid->GetYCoordinates();
        vtkDoubleArray *scalars = vtkDoubleArray::New();

        scalars->SetNumberOfComponents(1);
        scalars->SetNumberOfTuples(numPts * numCurves);

        rgrid->GetPointData()->SetScalars(scalars);

        double coord[] = {0.0, 0.0, 0.0};

        for (int i = 0; i < numPts; ++i)
        {
            inPts->GetPoint(i, coord);        
            xCoords->SetTuple1(i, coord[0]);
        }

        for (int i = 0; i < numCurves; i++)
        {
            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inPtData->GetArray(i);

            if (curve == NULL)
            {
                char msg[512];
                snprintf(msg, 512, "VisIt was unable to retreive data for the "
                    "following variable: %s\n", curve->GetName()); 
                continue;
            }

            yCoords->SetTuple1(i, i);

            int baseIdx = i*numPts;

            for (int j = 0; j < numPts; j++)
            {
                scalars->SetTuple1(baseIdx + j, curve->GetTuple1(j));
            }
        }

        avtDataTree_p tree = new avtDataTree(rgrid, 0);

        if (rgrid != NULL)
        {
            rgrid->Delete();
        }

        scalars->Delete();

        return tree;
    }
    else
    {
        stringVector vars;
        vars.reserve(numCurves);

        vtkDataSet **grids = new vtkDataSet *[numCurves];

        for (int i = 0; i< numCurves; ++i)
        {
            vtkDoubleArray *curve = 
                (vtkDoubleArray *) inPtData->GetArray(i);

            if (curve == NULL)
            {
                char msg[512];
                snprintf(msg, 512, "VisIt was unable to retreive data for the "
                    "following variable: %s\n", curve->GetName());
                continue;
            }

            vars.push_back(curve->GetName());

            grids[i] = vtkVisItUtility::Create1DRGrid(numPts, VTK_FLOAT);

            vtkDataArray *xCoords   = ((vtkRectilinearGrid*)grids[i])->GetXCoordinates();
            vtkDoubleArray *scalars = vtkDoubleArray::New();

            scalars->SetNumberOfComponents(1);
            scalars->SetNumberOfTuples(numPts);
            scalars->SetName(vars[i].c_str());
          
            scalars->ShallowCopy(curve);
            grids[i]->GetPointData()->SetScalars(scalars);

            double coord[] = {0.0, 0.0, 0.0};

            for (int i = 0; i < numPts; ++i)
            {
                inPts->GetPoint(i, coord);        
                xCoords->SetTuple1(i, coord[0]);
            }
        }

        avtDataTree_p tree = new avtDataTree(numCurves, grids, -1, vars);

        for (int i = 0; i< numCurves; ++i)
        {
            if (grids[i] != NULL)
            {
                grids[i]->Delete();
            }
        }

        delete [] grids;

        return tree;
    }
}


// ****************************************************************************
//  Method:  avtDirectDatabaseQOTFilter::UpdateDataObjectInfo
//
//  Purpose:
//      Update the attributes and validity. 
//
//  Programmer: Alister Maguire
//  Creation:   Tue Sep 24 11:15:10 MST 2019 
//
//  Modifications:
//
// ****************************************************************************

void
avtDirectDatabaseQOTFilter::UpdateDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();

    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
    outAtts.GetOriginalSpatialExtents()->Clear();
    outAtts.GetDesiredSpatialExtents()->Clear();

    if (finalOutputCreated)
    {
        stringVector qotVars = atts.GetQueryAtts().GetVariables();

        if (qotVars.size() > 0)
        {
            //
            // The first requested variable becomes the active one. 
            //
            outAtts.SetActiveVariable(qotVars[0].c_str());
        }

        if (useTimeForXAxis)
        {
            outAtts.SetXLabel("Time");
            outAtts.SetYLabel(YLabel);

            if (atts.GetTimeType() == QueryOverTimeAttributes::Cycle)
            {
                outAtts.SetXUnits("cycle");
            }
            else if (atts.GetTimeType() == QueryOverTimeAttributes::DTime)
            {
                outAtts.SetXUnits("time");
            }
            else if (atts.GetTimeType() == QueryOverTimeAttributes::Timestep)
            {
                outAtts.SetXUnits("timestep");
            }
            else
            {
                outAtts.SetXUnits("");
            }

            if (useVarForYAxis)
            {
                std::string yl = outAtts.GetVariableName();
                outAtts.SetYLabel(yl);
                outAtts.SetYUnits(outAtts.GetVariableUnits(yl.c_str()));
            }
        }
        else if (qotVars.size() >= 2)
        {
            std::string xl = qotVars[0] + "(t)";
            std::string yl = qotVars[1] + "(t)";
            outAtts.SetXLabel(xl);
            outAtts.SetYLabel(yl);
            outAtts.SetXUnits(atts.GetQueryAtts().GetXUnits());
            outAtts.SetYUnits(atts.GetQueryAtts().GetYUnits());
        }

        outAtts.SetLabels(qotVars);

        if (qotVars.size() > 1)
        {
            outAtts.SetConstructMultipleCurves(true);
        }
        else
        {
            outAtts.SetConstructMultipleCurves(false);
        }

        double bounds[6];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetSpatialExtents(ds, bounds);
        outAtts.GetThisProcsOriginalSpatialExtents()->Set(bounds);
    }
}
