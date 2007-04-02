#include <avtQueryFactory.h>
#include <avtDataObjectQuery.h>


// available queries
#include <avtActualDataMinMaxQuery.h>
#include <avtActualDataNumNodesQuery.h>
#include <avtActualDataNumZonesQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtCentroidQuery.h>
#include <avtCompactnessQuery.h>
#include <avtCycleQuery.h>
#include <avtEulerianQuery.h>
#include <avtIntegrateQuery.h>
#include <avtL2NormQuery.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtMomentOfInertiaQuery.h>
#include <avtNodeCoordsQuery.h>
#include <avtOriginalDataMinMaxQuery.h>
#include <avtOriginalDataNumNodesQuery.h>
#include <avtOriginalDataNumZonesQuery.h>
#include <avtTimeQuery.h>
#include <avtTotalRevolvedSurfaceAreaQuery.h>
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtVariableByNodeQuery.h>
#include <avtVariableByZoneQuery.h>
#include <avtVariableSummationQuery.h>
#include <avtWeightedVariableSummationQuery.h>
#include <avtZoneCenterQuery.h>


#include <QueryAttributes.h>
#include <string>
using std::string;

//
// Storage for static data elements.
//
avtQueryFactory *avtQueryFactory::instance=0;



// ****************************************************************************
//  Method: avtQueryFactory constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 23, 2004
//
//  Modifications:
//
// ****************************************************************************

avtQueryFactory::avtQueryFactory() 
{
}

// ****************************************************************************
//  Method: avtQueryFactory destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 23, 2004
//
//  Modifications:
//
// ****************************************************************************

avtQueryFactory::~avtQueryFactory()
{
}


// ****************************************************************************
//  Method: avtQueryFactory::Instance
//
//  Purpose:
//    Return a pointer to the sole instance of the avtQueryFactory
//    class.
//
//  Returns:    A pointer to the sole instance of the avtQueryFactory
//              class.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 30, 2004 
//
// ****************************************************************************

avtQueryFactory *
avtQueryFactory::Instance()
{
    //
    // If the sole instance hasn't been instantiated, then instantiate it.
    //
    if (instance == 0)
    {
        instance = new avtQueryFactory;
    }

    return instance;
}


// ****************************************************************************
//  Method: avtQueryFactory::CreateQuery
//
//  Purpose:
//    Return a pointer to a query instance, based on passed QueryAtts. 
//
//  Arguments:
//    qa        The QueryAttributes which specify the type of query to create.
//             
//  Returns:    A pointer to an avtDataObjectQuery. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 30, 2004 
//
//  Modifications:
//
//    Hank Childs, Tue Apr 13 12:50:51 PDT 2004
//    Allow for surface area to have multiple aliases.
//
//    Kathleen Bonnell, Tue May 25 16:16:25 PDT 2004 
//    Added Zone center query.
//
//    Kathleen Bonnell, Fri Jun 11 14:35:50 PDT 2004 
//    Added NodeCoords query.
//
//    Kathleen Bonnell, Thu Jul 29 17:07:08 PDT 2004 
//    Replaced avtVariableQuery with avtVariableByNodeQuery and
//    avtVariableByZoneQuery.
//
//    Hank Childs, Wed May 18 16:14:23 PDT 2005
//    Added Moment of Inertia, Centroid queries.
//
// ****************************************************************************


avtDataObjectQuery *
avtQueryFactory::CreateQuery(const QueryAttributes *qa)
{
    string qname = qa->GetName();
    bool actualData = qa->GetDataType() == QueryAttributes::ActualData;

    avtDataObjectQuery *query = NULL;

    if (qname == "Surface area" || qname == "2D area" ||
        qname == "3D surface area")
    {
        query = new avtTotalSurfaceAreaQuery();
    }
    else if (qname == "Volume")
    {
        query = new avtTotalVolumeQuery();
    }
    else if (qname == "Revolved volume")
    {
        query = new avtTotalRevolvedVolumeQuery();
    }
    else if (qname == "Revolved surface area")
    {
        query = new avtTotalRevolvedSurfaceAreaQuery();
    }
    else if (qname == "Eulerian")
    {
        query = new avtEulerianQuery();
    }
    else if (qname == "Compactness")
    {
        query = new avtCompactnessQuery();
    }
    else if (qname == "Cycle")
    {
        query = new avtCycleQuery();
    }
    else if (qname == "Integrate")
    {
        query = new avtIntegrateQuery();
    }
    else if (qname == "Time")
    {
        query = new avtTimeQuery();
    }
    else if (qname == "L2Norm")
    {
        query = new avtL2NormQuery();
    }
#if 0
    // problem with multiple input queries, so don't do them here
    else if (qname == "L2Norm Between Curves")
    {
        query = new avtL2NormBetweenCurvesQuery();
    }
    else if (qname == "Area Between Curves")
    {
        query = new avtAreaBetweenCurvesQuery();
    }
#endif
    else if (qname == "Variable Sum")
    {
        query = new avtVariableSummationQuery();
    }
    else if (qname == "Centroid")
    {
        query = new avtCentroidQuery();
    }
    else if (qname == "Moment of Inertia")
    {
        query = new avtMomentOfInertiaQuery();
    }
    else if (qname == "Weighted Variable Sum")
    {
        query = new avtWeightedVariableSummationQuery();
    }
    else if (qname == "Variable by Zone") 
    {
        query = new avtVariableByZoneQuery();
    }
    else if (qname == "Variable by Node")
    {
        query = new avtVariableByNodeQuery();
    }
    else if (qname == "MinMax")
    {
        if (actualData)
        {
            query = new avtActualDataMinMaxQuery();
        }
        else 
        {
            query = new avtOriginalDataMinMaxQuery();
        }
    }
    else if (qname == "Min")
    {
        if (actualData)
        {
            query = new avtActualDataMinMaxQuery(true, false);
        }
        else 
        {
            query = new avtOriginalDataMinMaxQuery(true, false);
        }
    }
    else if (qname == "Max")
    {
        if (actualData)
        {
            query = new avtActualDataMinMaxQuery(false, true);
        }
        else 
        {
            query = new avtOriginalDataMinMaxQuery(false, true);
        }
    }
    else if (qname == "NumZones") 
    {
        if (actualData)
        {
            query = new avtActualDataNumZonesQuery();
        }
        else 
        {
            query = new avtOriginalDataNumZonesQuery();
        }
    }
    else if (qname == "NumNodes")
    {
        if (actualData)
        {
            query = new avtActualDataNumNodesQuery();
        }
        else 
        {
            query = new avtOriginalDataNumNodesQuery();
        }
    }
    else if (qname == "Zone Center")
    {
        query = new avtZoneCenterQuery();
    }
    else if (qname == "Node Coords")
    {
        query = new avtNodeCoordsQuery();
    }
    return query;
}


