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
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <avtCallback.h>
#include <avtDatasetExaminer.h>
#include <avtExtents.h>
#include <avtParallel.h>

#include <VisItException.h>
#include <DebugStream.h>

using std::string;
//
// Function Prototypes
//

vtkPolyData     *CreatePolys(const doubleVector &, const doubleVector &);



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
// ****************************************************************************

avtQueryOverTimeFilter::avtQueryOverTimeFilter(const AttributeGroup *a)
{
    atts = *(QueryOverTimeAttributes*)a;
    SetTimeLoop(atts.GetStartTime(), atts.GetEndTime(), atts.GetStride());
    finalOutputCreated = false;
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

    query->SetTimeVarying(true);
    query->SetInput(GetInput());
    query->SetSILRestriction(currentSILR);

    //
    // HokeyHack ... we want only 1 curve, so limit the
    // query to 1 variable to avoid unnecessary processing.
    //
    stringVector useThisVar;
    useThisVar.push_back(qatts.GetVariables()[0]);
    qatts.SetVariables(useThisVar);
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

    //
    // Store the query results ... currently only one result.
    //
    qRes.push_back(results[0]);
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
        outAtts.SetXLabel("Time");
        outAtts.SetYLabel(atts.GetQueryAtts().GetName());
        // later, can use cycles or dtime
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
        outAtts.SetYUnits("");

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
    if (qRes.size() != times.size())
    {
        debug4 << "QueryOverTime ERROR, number of results (" 
               << qRes.size() << ") does not equal number "
               << "of timesteps (" << times.size() << ")." << endl;
        avtCallback::IssueWarning(
            "\nQueryOverTime error, number of results does not equal "
            "number of timestates.  Curve being created may be missing "
            "some values.  Please contact a VisIt developer."); 
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
//
// ****************************************************************************

vtkPolyData *
CreatePolys(const doubleVector &x, const doubleVector &y)
{
    vtkPolyData *pd = vtkPolyData::New();

    vtkPoints *points = vtkPoints::New();
    pd->SetPoints(points);
    points->Delete();

    vtkCellArray *verts = vtkCellArray::New();
    pd->SetVerts(verts);
    verts->Delete();

    int nPts = (x.size() <= y.size() ? x.size() : y.size());
    points->SetNumberOfPoints(nPts);
    verts->InsertNextCell(nPts);
    for (int i = 0; i < nPts; i++)
    {
        points->SetPoint(i, x[i], y[i], 0);
        verts->InsertCellPoint(i);
    }

    return pd;
}
