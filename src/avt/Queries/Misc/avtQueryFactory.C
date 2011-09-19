/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#include <avtQueryFactory.h>
#include <avtDataObjectQuery.h>

// available queries
#include <avtActualDataMinMaxQuery.h>
#include <avtActualDataNumNodesQuery.h>
#include <avtActualDataNumZonesQuery.h>
#include <avtAggregateChordLengthDistributionQuery.h>
#include <avtAggregateRayLengthDistributionQuery.h>
#include <avtAreaBetweenCurvesQuery.h>
#include <avtAverageMeanCurvatureQuery.h>
#include <avtAverageValueQuery.h>
#include <avtBestFitLineQuery.h>
#include <avtCentroidQuery.h>
#include <avtCompactnessQuery.h>
#include <avtConnComponentsQuery.h>
#include <avtConnComponentsAreaQuery.h>
#include <avtConnComponentsCentroidQuery.h>
#include <avtConnComponentsLengthQuery.h>
#include <avtConnComponentsSummaryQuery.h>
#include <avtConnComponentsVolumeQuery.h>
#include <avtConnComponentsVariableQuery.h>
#include <avtConnComponentsWeightedVariableQuery.h>
#include <avtCycleQuery.h>
#include <avtDistanceFromBoundaryQuery.h>
#include <avtEllipticalCompactnessFactorQuery.h>
#include <avtEulerianQuery.h>
#include <avtExpectedValueQuery.h>
#include <avtHohlraumFluxQuery.h>
#include <avtIndividualChordLengthDistributionQuery.h>
#include <avtIndividualRayLengthDistributionQuery.h>
#include <avtIntegrateQuery.h>
#include <avtL2NormQuery.h>
#include <avtL2NormBetweenCurvesQuery.h>
#include <avtLineScanTransformQuery.h>
#include <avtLocalizedCompactnessFactorQuery.h>
#include <avtLocateAndPickNodeQuery.h>
#include <avtLocateAndPickZoneQuery.h>
#include <avtKurtosisQuery.h>
#include <avtMassDistributionQuery.h>
#include <avtMemoryUsageQuery.h>
#include <avtMomentOfInertiaQuery.h>
#include <avtNodeCoordsQuery.h>
#include <avtOriginalDataMinMaxQuery.h>
#include <avtOriginalDataNumNodesQuery.h>
#include <avtOriginalDataNumZonesQuery.h>
#include <avtOriginalDataSpatialExtentsQuery.h>
#include <avtSampleStatisticsQuery.h>
#include <avtShapeletDecompositionQuery.h>
#include <avtSkewnessQuery.h>
#include <avtSphericalCompactnessFactorQuery.h>
#include <avtTimeQuery.h>
#include <avtTotalRevolvedSurfaceAreaQuery.h>
#include <avtTotalRevolvedVolumeQuery.h>
#include <avtTotalSurfaceAreaQuery.h>
#include <avtTotalLengthQuery.h>
#include <avtTotalVolumeQuery.h>
#include <avtTrajectoryByNode.h>
#include <avtTrajectoryByZone.h>
#include <avtVariableByNodeQuery.h>
#include <avtVariableByZoneQuery.h>
#include <avtVariableSummationQuery.h>
#include <avtWatertightQuery.h>
#include <avtWeightedVariableSummationQuery.h>
#include <avtXRayImageQuery.h>
#include <avtZoneCenterQuery.h>
#include <avtStreamlineInfoQuery.h>

#include <visit-python-config.h>
#ifdef VISIT_PYTHON_FILTERS
#include <avtPythonQuery.h>
#endif

#include <QueryAttributes.h>
#include <string>
#include <StringHelpers.h>

using std::string;
using namespace StringHelpers;

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
//    Hank Childs, Thu Jul 14 14:12:12 PDT 2005
//    Add Spherical Compactness Factor.
//
//    Hank Childs, Fri Aug  5 09:49:12 PDT 2005
//    Added kurtosis, skewness.
//
//    Hank Childs, Fri Sep 23 16:11:07 PDT 2005
//    Added watertight query.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005 
//    Added TrajectoryByNode/Zone queries.
//
//    Brad Whitlock, Thu Nov 17 10:18:41 PDT 2005
//    Added Best Fit Line.
//
//    Kathleen Bonnell, Tue Jan 31 15:52:18 PST 2006
//    Added OriginalData SpatialExtents query.
//
//    Hank Childs, Sat Apr 29 14:40:47 PDT 2006
//    Added localized and elliptical compactness factor queries.
//
//    Hank Childs, Thu May 11 13:21:18 PDT 2006
//    Added average mean curvature.
//
//    Hank Childs, Sat Jul  8 11:25:45 PDT 2006
//    Added chord length distribution query.
//
//    Hank Childs, Thu Jul 20 11:23:07 PDT 2006
//    Added mass distribution query.
//
//    Hank Childs, Mon Aug  7 18:05:38 PDT 2006
//    Added distance from boundary query.
//
//    Hank Childs, Fri Aug 25 15:40:35 PDT 2006
//    Added expected value query.
//
//    Hank Childs, Mon Aug 28 16:52:47 PDT 2006
//    Added aggregate and individual variants of chord and ray length
//    distributions.
//
//    Dave Bremer, Thu Sep  7 16:16:41 PDT 2006
//    Added line scan transform query.
//
//    Kathleen Bonnell, Fri Sep 15 09:23:50 PDT 2006
//    Added Volume2, which calculates hex-volumes differently than
//    Volume (which uses verdict).
//
//    Hank Childs, Fri Nov  3 15:49:40 PST 2006
//    Added total length query.
//
//    Dave Bremer, Fri Dec  8 17:52:22 PST 2006
//    Added hohlraum flux query.
//
//    Cyrus Harrison, Tue Feb 20 15:20:14 PST 2007
//    Added connected components queries.
//
//    Cyrus Harrison, Thu Mar  1 16:20:27 PST 2007
//    Added connected components summary query.
//
//    Kathleen Bonnell, Tue Nov 20 10:33:49 PST 2007 
//    Added Locate and Pick Zone/Node queries. 
//
//    Cyrus Harrison, Tue Dec 18 14:15:58 PST 2007
//    Added Shapelet Decomposition Query.
//
//    Cyrus Harrison, Wed Mar  5 08:56:01 PST 2008
//    Added Memory Usage Query. 
//
//    Jeremy Meredith, Wed Mar 11 17:49:06 EDT 2009
//    Added sample and population statistics.
//
//    Eric Brugger, Mon May 11 12:19:32 PDT 2009
//    Enhanced the hohlraum flux query so that it took an additional argument
//    that caused it to optionally use the emissivity divided by the
//    absorbtivity in place of the emissivity.
//
//    Cyrus Harrison, Tue Feb  2 16:03:19 PST 2010
//    Added the python filter query.
//
//    Eric Brugger, Thu Mar 25 09:30:02 PDT 2010
//    Renamed the individual/aggregate "Chord Length Distribution" and
//    individual/aggregate "Ray Length Distribution" queries so that they do
//    not use special characters.
//
//    Eric Brugger, Wed Jun 30 14:03:06 PDT 2010
//    Added the xray image query.
//
//    Cyrus Harrison, Tue Sep 21 11:12:17 PDT 2010
//    Added explicit passing of args to the python filter query.
//
//    Dave Pugmire, Tue Nov  9 14:57:20 EST 2010
//    Add Streamline Info query.
//
//    Kathleen Bonnell, Thu Feb 17 09:51:44 PST 2011
//    Set number of vars for LocateAndPickNode/Zone queries, and 
//    VariableyByNode/Zone queries.
//
//    Hank Childs, Thu May 12 15:37:21 PDT 2011
//    Add average value query.
//
//    Cyrus Harrison, Wed Jun 15 13:14:49 PDT 2011
//    Added Connected Components Length.
//
//    Kathleen Biagas, Fri Jun 17 16:26:06 PDT 2011
//    Call SetInputParams after query is created, precludes need to set
//    individual query options here.
//
//    Kathleen Biagas, Mon Sep 12 18:27:26 PDT 2011
//    Clean up query memory if SetInputParams fails.
//
//    Cyrus Harrison, Mon Sep 19 10:49:37 PDT 2011
//    Use case insensitive match for query names.
//
// ****************************************************************************

avtDataObjectQuery *
avtQueryFactory::CreateQuery(const QueryAttributes *qa)
{
    string qname = qa->GetName();
    int actualData = 0;
    if (qa->GetQueryInputParams().HasEntry("use_actual_data"))
        actualData = qa->GetQueryInputParams().GetEntry("use_actual_data")->AsInt();

    avtDataObjectQuery *query = NULL;
    bool foundAQuery = false;

    if (CaseInsenstiveEqual(qname,"Surface area") || 
        CaseInsenstiveEqual(qname,"2D area")||
        CaseInsenstiveEqual(qname,"3D surface area"))
    {
        query = new avtTotalSurfaceAreaQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Volume"))
    {
        query = new avtTotalVolumeQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Volume2"))
    {
        query = new avtTotalVolumeQuery(false);
    }
    else if (CaseInsenstiveEqual(qname,"Total Length"))
    {
        query = new avtTotalLengthQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Revolved volume"))
    {
        query = new avtTotalRevolvedVolumeQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Revolved surface area"))
    {
        query = new avtTotalRevolvedSurfaceAreaQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Eulerian"))
    {
        query = new avtEulerianQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Compactness"))
    {
        query = new avtCompactnessQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Cycle"))
    {
        query = new avtCycleQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Integrate"))
    {
        query = new avtIntegrateQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Expected Value"))
    {
        query = new avtExpectedValueQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Time"))
    {
        query = new avtTimeQuery();
    }
    else if (CaseInsenstiveEqual(qname,"L2Norm"))
    {
        query = new avtL2NormQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Line Scan Transform"))
    {
        query = new avtLineScanTransformQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Chord Length Distribution - aggregate"))
    {
        query = new avtAggregateChordLengthDistributionQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Chord Length Distribution - individual"))
    {
        query = new avtIndividualChordLengthDistributionQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Ray Length Distribution - aggregate"))
    {
        query = new avtAggregateRayLengthDistributionQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Ray Length Distribution - individual"))
    {
        query = new avtIndividualRayLengthDistributionQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Mass Distribution"))
    {
        query = new avtMassDistributionQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Distance From Boundary"))
    {
        query = new avtDistanceFromBoundaryQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Kurtosis"))
    {
        query = new avtKurtosisQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Skewness"))
    {
        query = new avtSkewnessQuery();
    }
    // problem with multiple input queries, so don't do them here
    else if (CaseInsenstiveEqual(qname,"L2Norm Between Curves"))
    {
#if 0
        query = new avtL2NormBetweenCurvesQuery();
#endif
        foundAQuery = true;
    }
    else if (CaseInsenstiveEqual(qname,"Area Between Curves"))
    {
#if 0
        query = new avtAreaBetweenCurvesQuery();
#endif
        foundAQuery = true;
    }
    else if (CaseInsenstiveEqual(qname,"Variable Sum"))
    {
        query = new avtVariableSummationQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Centroid"))
    {
        query = new avtCentroidQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Moment of Inertia"))
    {
        query = new avtMomentOfInertiaQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Spherical Compactness Factor"))
    {
        query = new avtSphericalCompactnessFactorQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Localized Compactness Factor"))
    {
        query = new avtLocalizedCompactnessFactorQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Elliptical Compactness Factor"))
    {
        query = new avtEllipticalCompactnessFactorQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Watertight"))
    {
        query = new avtWatertightQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Weighted Variable Sum"))
    {
        query = new avtWeightedVariableSummationQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Variable by Zone"))
    {
        query = new avtVariableByZoneQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Variable by Node"))
    {
        query = new avtVariableByNodeQuery();
    }
    else if (CaseInsenstiveEqual(qname,"MinMax"))
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
    else if (CaseInsenstiveEqual(qname,"Min"))
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
    else if (CaseInsenstiveEqual(qname,"Max"))
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
    else if (CaseInsenstiveEqual(qname,"NumZones"))
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
    else if (CaseInsenstiveEqual(qname,"NumNodes"))
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
    else if (CaseInsenstiveEqual(qname,"Zone Center"))
    {
        query = new avtZoneCenterQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Node Coords"))
    {
        query = new avtNodeCoordsQuery();
    }
    else if (CaseInsenstiveEqual(qname,"TrajectoryByZone"))
    {
        query = new avtTrajectoryByZone();
    }
    else if (CaseInsenstiveEqual(qname,"TrajectoryByNode"))
    {
        query = new avtTrajectoryByNode();
    }
    else if (CaseInsenstiveEqual(qname,"Best Fit Line"))
    {
        query = new avtBestFitLineQuery();
    }
    else if (CaseInsenstiveEqual(qname,"SpatialExtents"))
    {
        query = new avtOriginalDataSpatialExtentsQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Average Mean Curvature"))
    {
        query = new avtAverageMeanCurvatureQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Average Value"))
    {
        query = new avtAverageValueQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Hohlraum Flux"))
    {
        query  = new avtHohlraumFluxQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Number of Connected Components"))
    {
        query = new avtConnComponentsQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Centroids"))
    {
        query = new avtConnComponentsCentroidQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Area"))
    {
        query = new avtConnComponentsAreaQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Length"))
    {
        query = new avtConnComponentsLengthQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Volume"))
    {
        query = new avtConnComponentsVolumeQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Variable Sum"))
    {
        query = new avtConnComponentsVariableQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Component Weighted Variable Sum"))
    {
        query = new avtConnComponentsWeightedVariableQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Components Summary"))
    {
        query = new avtConnComponentsSummaryQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Locate and Pick Zone"))
    {
        query = new avtLocateAndPickZoneQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Locate and Pick Node"))
    {
        query = new avtLocateAndPickNodeQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Shapelet Decomposition"))
    {
        query = new avtShapeletDecompositionQuery();
    }
    else if( CaseInsenstiveEqual(qname,"Memory Usage"))
    {
        query = new avtMemoryUsageQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Sample Statistics"))
    {
        query = new avtSampleStatisticsQuery(false);
    }
    else if (CaseInsenstiveEqual(qname,"Population Statistics"))
    {
        query = new avtSampleStatisticsQuery(true);
    }
    else if (CaseInsenstiveEqual(qname,"XRay Image"))
    {
        query = new avtXRayImageQuery();
    }
    else if (CaseInsenstiveEqual(qname,"Python"))
    {
#ifdef VISIT_PYTHON_FILTERS
        avtPythonQuery *py_query = new avtPythonQuery();
        const stringVector &args = qa->GetVariables();
        int nargs = args.size();
        stringVector vars;

        for(int i= 0; i < nargs -2;i++)
            vars.push_back(args[i]);

        // set variable names
        py_query->SetVariableNames(vars);
        // python script is passed in as the last variable
        py_query->SetPythonArgs(args[nargs-2]);
        py_query->SetPythonScript(args[nargs-1]);
        query = py_query;
#else
           EXCEPTION1(VisItException,
                      "Cannot execute Python Filter Query because "
                      "VisIt was build without Python Filter support.");
#endif
    }
    else if (CaseInsenstiveEqual(qname,"Streamline Info"))
    {
        query = new avtStreamlineInfoQuery();
    }

    if (query == NULL && !foundAQuery)
    {
        EXCEPTION1(VisItException, "No query to execute was found. "
             " Developers: if you are developing this query, make sure that "
             "the name specified in the ViewerQueryManager matches that of "
             "the avtQueryFactory.");
    }

    if (query != NULL)
    {
        TRY
        {
            query->SetInputParams(qa->GetQueryInputParams());
        }
        CATCH2(VisItException, e)
        {
            string msg = qname + " did not find all of its input parameters. (";
            msg += e.Message();
            msg += ")  Developers:  if you are developing this query, make ";
            msg += "sure the map node containing the input parameter has been ";
            msg += "set up correctly in the gui and in the cli.";
            delete query;
            query = NULL;
            EXCEPTION1(VisItException, msg.c_str());
        }
        ENDTRY
    }
    return query;
}


// ****************************************************************************
//  Method: avtQueryFactory::GetDefaultInputParams
//
//  Purpose:
//    Retrieve default input values for named query.
//
//  Arguments:
//    qname     The name of the query.
//             
//  Returns:    A string representation of the MapNode parameters.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
//  Modifications:
//    Cyrus Harrison, Mon Sep 19 10:49:37 PDT 2011
//    Use case insensitive match for query names.
//
// ****************************************************************************

string 
avtQueryFactory::GetDefaultInputParams(const string &qname)
{
    MapNode params;
    string retval;
    if (CaseInsenstiveEqual(qname,"XRay Image"))
    {
        avtXRayImageQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Hohlraum Flux"))
    {
        avtHohlraumFluxQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if( CaseInsenstiveEqual(qname,"Connected Components Summary"))
    {
        avtConnComponentsSummaryQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Chord Length Distribution - aggregate")  ||
             CaseInsenstiveEqual(qname,"Chord Length Distribution - individual") || 
             CaseInsenstiveEqual(qname,"Ray Length Distribution - aggregate")    ||
             CaseInsenstiveEqual(qname,"Ray Length Distribution - individual")   ||
             CaseInsenstiveEqual(qname,"Mass Distribution")                      || 
             CaseInsenstiveEqual(qname,"Line Scan Transform")                    ||
             CaseInsenstiveEqual(qname,"Distance From Boundary"))
    {
        avtLineScanQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Node Coords"))
    {
        avtNodeCoordsQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Zone Center"))
    {
        avtZoneCenterQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if( CaseInsenstiveEqual(qname,"Shapelet Decomposition"))
    {
        avtShapeletDecompositionQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Streamline Info"))
    {
        avtStreamlineInfoQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"Min") ||
             CaseInsenstiveEqual(qname,"Max") ||
             CaseInsenstiveEqual(qname,"MinMax"))
    {
        avtMinMaxQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"NumNodes"))
    {
        avtNumNodesQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"NumZones"))
    {
        avtNumZonesQuery::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"TrajectoryByZone"))
    {
        avtTrajectoryByZone::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    else if (CaseInsenstiveEqual(qname,"TrajectoryByNode"))
    {
        avtTrajectoryByNode::GetDefaultInputParams(params);
        retval = params.ToXML();
    }
    return retval;
}
