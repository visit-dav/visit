#include <avtQueryFactory.h>
#include <avtDataObjectQuery.h>


// available queries
#include <avtActualDataMinMaxQuery.h>
#include <avtActualDataNumNodesQuery.h>
#include <avtActualDataNumZonesQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtCompactnessQuery.h>
#include <avtCycleQuery.h>
#include <avtEulerianQuery.h>
#include <avtIntegrateQuery.h>
#include <avtL2NormQuery.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtOriginalDataMinMaxQuery.h>
#include <avtOriginalDataNumNodesQuery.h>
#include <avtOriginalDataNumZonesQuery.h>
#include <avtTimeQuery.h>
#include <avtTotalRevolvedSurfaceAreaQuery.h>
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtVariableQuery.h>
#include <avtVariableSummationQuery.h>
#include <avtWeightedVariableSummationQuery.h>


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
    else if (qname == "Weighted Variable Sum")
    {
        query = new avtWeightedVariableSummationQuery();
    }
    else if (qname == "Variable by Zone") 
    {
        query = new avtVariableQuery();
    }
    else if (qname == "Variable by Node")
    {
        query = new avtVariableQuery();
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
    return query;
}


// ****************************************************************************
//  Method: avtQueryFactory::CreateTimeQuery
//
//  Purpose:
//    Return a pointer to a query instance, based on passed QueryAtts. 
//
//  Arguments:
//    qa        The QueryAttributes which specify the type of query to create.
//             
//  Returns:    A pointer to an avtDataObjectQuery which can be performed
//              over time.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 30, 2004 
//
//  Modifications:
//
//    Hank Childs, Wed Apr 14 07:48:12 PDT 2004
//    Allow for surface area to have multiple aliases.
//
// ****************************************************************************

avtDataObjectQuery *
avtQueryFactory::CreateTimeQuery(const QueryAttributes *qa)
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
    else if (qname == "Variable Sum")
    {
        query = new avtVariableSummationQuery();
    }
    else if (qname == "Weighted Variable Sum")
    {
        query = new avtWeightedVariableSummationQuery();
    }
    else if (qname ==  "Min")
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
    else if (qname == "Variable by Zone") 
    {
        query = new avtVariableQuery();
    }
    else if (qname == "Variable by Node") 
    {
        query = new avtVariableQuery();
    }

    //
    // There are a lot of queries that would not get instantiaed
    // here -- (non-time-queries) -- so don't check
    // for NULL, allow caller to do that.
    //
    return query;
}

