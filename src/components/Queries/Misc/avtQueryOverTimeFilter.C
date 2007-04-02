// ************************************************************************* //
//                         avtQueryOverTimeFilter.C                          //
// ************************************************************************* //

#include <float.h>
#include <snprintf.h>
#include <string.h>

#include <avtDataObjectQuery.h>
#include <avtQueryFactory.h>
#include <avtQueryOverTimeFilter.h>

#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>

#include <VisItException.h>
#include <DebugStream.h>

using std::string;



// ****************************************************************************
//  Method: avtQueryOverTimeFilter constructor
//
//  Arguments:
//    atts      The attributes the filter should use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 15, 2004
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 11:16:40 PST 2005
//    Add call to SetTimeLoop.  Initialize finalOutputCreated.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Initialize useTimeforXAxis and nResultsToStore. 
//
// ****************************************************************************

avtQueryOverTimeFilter::avtQueryOverTimeFilter(const AttributeGroup *a)
{
    atts = *(QueryOverTimeAttributes*)a;
    SetTimeLoop(atts.GetStartTime(), atts.GetEndTime(), atts.GetStride());
    finalOutputCreated = false;
    useTimeForXAxis = true;
    nResultsToStore = 1;
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004 
//
// ****************************************************************************

avtQueryOverTimeFilter::~avtQueryOverTimeFilter()
{
}


// ****************************************************************************
//  Method:  avtQueryOverTimeFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    March 15, 2004
//
// ****************************************************************************

avtFilter *
avtQueryOverTimeFilter::Create(const AttributeGroup *atts)
{
    return new avtQueryOverTimeFilter(atts);
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::Execute
//
//  Purpose:
//    Peforms a query over time, and collects the results into a 
//    vtkDataSet consisting of 2d points (time, results).
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 15, 2004
//
//  Modifications:
//    Kathleen Bonnell, Fri Apr  2 13:18:08 PST 2004
//    Pass along to the query: startTime,  endTimes and stride instead of an 
//    intVector representing timesteps.  Send a doubleVector for the query
//    to fill with values requested for x-axis (cycle, time, or timestep).
//
//    Kathleen Bonnell, Tue May  4 14:21:37 PDT 2004 
//    Replaced query->SetSILUseSet with query->SetSILRestriction. 
//
//    Kathleen Bonnell, Thu Jun 24 07:54:44 PDT 2004 
//    Pass storage for skippedTimesteps and error message to the query.
//    Check for skippedTimeSteps and issue a Warning message.
//    Changed CATCH to CATCHALL.  Reset avtDataValidity before processing.
//
//    Kathleen Bonnell, Mon Jan  3 15:21:44 PST 2005
//    Reworked, since time-looping is now handled by parent class 
//    'avtTimeLoopFilter' instead of avtDatasetQuery::PerformQueryInTime.
//    Test for error condition upstream, capture and store query results
//    and times.   Moved IssueWarning callbacks to CreateFinalOutput.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Added call to query.GetTimeCurveSpecs.  Only retrive time value
//    if it will be used for the X-axis.  Store the correct number of results.
//
// ****************************************************************************

void
avtQueryOverTimeFilter::Execute(void)
{
    //
    // The real output will be created after all time steps have completed, 
    // so create a dummy output to pass along for now.
    //
    avtDataTree_p dummy = new avtDataTree();

    //
    // Set up error conditions and return early if any processor had an 
    // error upstream.
    //

    int hadError = 0;
    if (GetInput()->GetInfo().GetValidity().HasErrorOccurred())
    {
        errorMessage = GetInput()->GetInfo().GetValidity().GetErrorMessage();
        hadError = 1;
    }
    hadError = UnifyMaximumValue(hadError);
    if (hadError)
    {
        SetOutputDataTree(dummy);
        success = false;    
        return;
    }

    //
    // Set up the query. 
    //
    QueryAttributes qatts = atts.GetQueryAtts();
    qatts.SetTimeStep(currentTime);
    avtDataObjectQuery *query = avtQueryFactory::Instance()->
        CreateQuery(&qatts);

    query->GetTimeCurveSpecs(useTimeForXAxis, nResultsToStore);
    query->SetTimeVarying(true);
    query->SetInput(GetInput());
    query->SetSILRestriction(currentSILR);

    //
    // HokeyHack ... we want only 1 curve, so limit the
    // query to 1 variable to avoid unnecessary processing.
    //
    if (nResultsToStore==1)
    {
        stringVector useThisVar;
        useThisVar.push_back(qatts.GetVariables()[0]);
        qatts.SetVariables(useThisVar);
    }
    //
    // End HokeyHack. 
    //

    TRY
    {
        query->PerformQuery(&qatts);
    }
    CATCHALL( ... )
    {
        SetOutputDataTree(dummy);
        success = false;
        delete query;
        RETHROW;
    }
    ENDTRY

    SetOutputDataTree(dummy);
    delete query;

    doubleVector results = qatts.GetResultsValue();
    if (results.size() == 0)
    {
        success = false;
        return;
    }
    else
    {
        success = true;
    }

    //
    // Store the necessary time value 
    //
    if (useTimeForXAxis)
    {
        double tval;  
        switch(atts.GetTimeType()) 
        {
        case QueryOverTimeAttributes::Cycle:
            tval = (double) GetInput()->GetInfo().GetAttributes().GetCycle();
            break;
        case QueryOverTimeAttributes::DTime: 
            tval = GetInput()->GetInfo().GetAttributes().GetTime();
            break;
        case QueryOverTimeAttributes::Timestep: 
        default: // timestep
            tval = (double)currentTime;
            break;
        }
        times.push_back(tval);
    }
    for (int i = 0; i < nResultsToStore; i++)
        qRes.push_back(results[i]);
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::RefashionDataObjectInfo
//
//  Purpose:
//    Indicates the zones no longer correspond to the original problem.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 15, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Thu Jan  6 11:21:44 PST 2005
//    Moved setting of  dataAttributes from PostExecute to here.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Use different labels/units if Time not used for X-Axis. 
//
// ****************************************************************************

void
avtQueryOverTimeFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);

    if (finalOutputCreated)
    {
        outAtts.GetTrueSpatialExtents()->Clear();
        outAtts.GetEffectiveSpatialExtents()->Clear();
        if (useTimeForXAxis)
        {
            outAtts.SetXLabel("Time");
            outAtts.SetYLabel(atts.GetQueryAtts().GetName());
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
        }
        else 
        {
            string xl = atts.GetQueryAtts().GetVariables()[0] + "(t)";
            string yl = atts.GetQueryAtts().GetVariables()[1] + "(t)";
            outAtts.SetXLabel(xl);
            outAtts.SetYLabel(yl);
            outAtts.SetXUnits(atts.GetQueryAtts().GetXUnits());
            outAtts.SetYUnits(atts.GetQueryAtts().GetYUnits());
        }

        double bounds[6];
        avtDataset_p ds = GetTypedOutput();
        avtDatasetExaminer::GetSpatialExtents(ds, bounds);
        outAtts.GetCumulativeTrueSpatialExtents()->Set(bounds);
    }
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::SetSILAtts
//
//  Purpose:
//    Sets the SILRestriction atts necessary to create a SILRestriction. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 4, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
avtQueryOverTimeFilter::SetSILAtts(const SILRestrictionAttributes *silAtts)
{
    querySILAtts = *silAtts;
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::CreateFinalOutput
//
//  Purpose:
//    Combines the results of all timesteps into one vtkDataSet output.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   January 3, 2005 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Time not always used for X-Axis. 
//
// ****************************************************************************

void
avtQueryOverTimeFilter::CreateFinalOutput()
{
    if (qRes.size() == 0)
    {
        debug4 << "Query failed at all timesteps" << endl;
        avtCallback::IssueWarning("Query failed at all timesteps");
        avtDataTree_p dummy = new avtDataTree();
        SetOutputDataTree(dummy);
        return;
    }
    if (useTimeForXAxis && qRes.size() != times.size())
    {
        debug4 << "QueryOverTime ERROR, number of results (" 
               << qRes.size() << ") does not equal number "
               << "of timesteps (" << times.size() << ")." << endl;
        avtCallback::IssueWarning(
            "\nQueryOverTime error, number of results does not equal "
            "number of timestates.  Curve being created may be missing "
            "some values.  Please contact a VisIt developer."); 
    }
    else if (qRes.size() % 2 != 0)
    {
        debug4 << "QueryOverTime ERROR, number of results (" 
               << qRes.size() << ") is not a multiple of 2 and "
               << "therefore cannot generate x,y pairs." << endl;
        avtCallback::IssueWarning(
            "\nQueryOverTime error, number of results is not multiple "
            "of 2. Curve being created may be missing some values. "
            " Please contact a VisIt developer."); 
    }
    if (skippedTimes.size() != 0)
    {
        ostrstream osm;
        osm << "\nQueryOverTime (" << atts.GetQueryAtts().GetName().c_str()
            << ") experienced\n"
            << "problems with the following timesteps and \n"
            << "skipped them while generating the curve:\n   ";

        for (int j = 0; j < skippedTimes.size(); j++)
            osm << skippedTimes[j] << " ";
        osm << "\nLast message received: " << errorMessage.c_str() << ends;
        debug4 << osm.str() << endl;
        avtCallback::IssueWarning(osm.str());
    }

    vtkPolyData *outpolys = CreatePolys(times, qRes);
    avtDataTree_p tree = new avtDataTree(outpolys, 0);
    outpolys->Delete();
    SetOutputDataTree(tree);
    finalOutputCreated = true;
}


// ****************************************************************************
//  Method: CreatePolys
//
//  Purpose:
//    Creates a polydata dataset that consists of points and vertices.
//
//  Arguments:
//    x           The values to use for x-coordinates. 
//    y           The values to use for y-coordinates. 
//
//  Programmer:   Kathleen Bonnell
//  Creation:     March 15, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Made this a member method. Time not always used for x-axis.
//
// ****************************************************************************

vtkPolyData *
avtQueryOverTimeFilter::CreatePolys(const doubleVector &times, 
                                    const doubleVector &res)
{
    vtkPolyData *pd = vtkPolyData::New();

    vtkPoints *points = vtkPoints::New();
    pd->SetPoints(points);
    points->Delete();

    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    verts->Delete();

    int nPts = 0;
    if (useTimeForXAxis && nResultsToStore == 1)
    {
       // Single curve with time for x axis.  NORMAL case.
       // Most queries currently use this option. 
       nPts = (times.size() <= res.size() ? times.size() : res.size());
    }
    else if (!useTimeForXAxis && nResultsToStore == 2)
    {
       // Single curve, res[odd] = x, res[even] = y.
       nPts = res.size() / 2;
    }
    else if (useTimeForXAxis && nResultsToStore > 1)
    {
       // Create multiple curves with time as x-axis
       // NOT IMPLEMENTED YET, need to create multiple
       // vtkPolyData objects, but pipeline for the Curve
       // plot not yet set up correctly.
    }
    else if (!useTimeForXAxis && nResultsToStore > 2)
    {
       // multiple curves, res[odd] = x, res[even] = y.
    }

    points->SetNumberOfPoints(nPts);
    verts->InsertNextCell(nPts);
    for (int i = 0; i < nPts; i++)
    {
        if (useTimeForXAxis)
            points->SetPoint(i, times[i], res[i], 0);
        else 
            points->SetPoint(i, res[i*2], res[i*2+1], 0);
        verts->InsertCellPoint(i);
    }
    return pd;
}
