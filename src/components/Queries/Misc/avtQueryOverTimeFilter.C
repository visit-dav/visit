// ************************************************************************* //
//                         avtQueryOverTimeFilter.C                          //
// ************************************************************************* //

#include <float.h>
#include <snprintf.h>

#include <avtQueryOverTimeFilter.h>
#include <avtQueryFactory.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

// available queries
#include <avtActualDataMinMaxQuery.h>
#include <avtActualDataNumNodesQuery.h>
#include <avtActualDataNumZonesQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtDataObjectQuery.h>
#include <avtCompactnessQuery.h>
#include <avtCycleQuery.h>
#include <avtEulerianQuery.h>
#include <avtIntegrateQuery.h>
#include <avtL2NormQuery.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtOriginalDataMinMaxQuery.h>
#include <avtOriginalDataNumNodesQuery.h>
#include <avtOriginalDataNumZonesQuery.h>
#include <avtTotalRevolvedSurfaceAreaQuery.h>
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtVariableQuery.h>
#include <avtVariableSummationQuery.h>
#include <avtWeightedVariableSummationQuery.h>

#include <avtCallback.h>
#include <avtExtents.h>
#include <avtDatasetExaminer.h>
#include <VisItException.h>
#include <DebugStream.h>


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
// ****************************************************************************

avtQueryOverTimeFilter::avtQueryOverTimeFilter(const AttributeGroup *a)
{
    atts = *(QueryOverTimeAttributes*)a;
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
// ****************************************************************************

void
avtQueryOverTimeFilter::Execute(void)
{
    QueryAttributes qatts = atts.GetQueryAtts();
    avtDataObjectQuery *query = avtQueryFactory::Instance()->
        CreateQuery(&qatts);

    query->SetTimeVarying(true);
    query->SetInput(GetInput());
    query->SetSILRestriction(&querySILAtts);

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

    doubleVector times;
    int startTime = atts.GetStartTime();
    int endTime = atts.GetEndTime();
    int stride = atts.GetStride();
    int timeType = (int) atts.GetTimeType();  // 0 = cycle, 1 = time, 2 = timestep

    TRY
    {
        query->PerformQueryInTime(&qatts, startTime, endTime, stride, 
                                  timeType, times);
    }
    CATCH( ... )
    {
        avtDataTree_p dummy = new avtDataTree();
        SetOutputDataTree(dummy);
        delete query;
        RETHROW;
    }
    ENDTRY

    doubleVector results = qatts.GetResultsValue();

    vtkPolyData *output = CreatePolys(times, results);
    avtDataTree_p tree = new avtDataTree(output, 0);
    output->Delete();

    if (results.size() != times.size())
    {
        debug5 << "QueryOverTime ERROR, number of results (" 
               << results.size() << ") does not equal number "
               << "of timesteps (" << times.size() << ")." << endl;
        avtCallback::IssueWarning(
            "QueryOverTime error, number of results does not equal "
            "number of timestates.  Curve being created may be missing "
            "some values.  Please contact a VisIt developer."); 
    }

    SetOutputDataTree(tree);
    delete query;
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
// ****************************************************************************

void
avtQueryOverTimeFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
    GetOutput()->GetInfo().GetValidity().InvalidateSpatialMetaData();
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
    outAtts.SetTopologicalDimension(1);
}


// ****************************************************************************
//  Method: avtQueryOverTimeFilter::CreatePolys
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



// ****************************************************************************
//  Method: avtQueryOverTimeFilter::PostExecute
//
//  Purpose:
//    Cleans up after the execution.  This manages extents, labels and units.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 15, 2004
//
//  Modifications:
//
// ****************************************************************************

void
avtQueryOverTimeFilter::PostExecute(void)
{
    avtDataAttributes &outAtts = GetOutput()->GetInfo().GetAttributes();
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
