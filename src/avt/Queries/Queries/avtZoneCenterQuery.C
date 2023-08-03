// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtZoneCenterQuery.C                           //
// ************************************************************************* //

#include <avtZoneCenterQuery.h>

#include <avtParallel.h>
#include <avtOriginatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <QueryArgumentException.h>
#include <vector>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtZoneCenterQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:13:00 PDT 2011
//    Added domain,element, useGlobalId.
//
// ****************************************************************************

avtZoneCenterQuery::avtZoneCenterQuery() : avtDatasetQuery() 
{
    domain = 0;
    element = 0;
    useGlobalId = false;
}


// ****************************************************************************
//  Method: avtZoneCenterQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtZoneCenterQuery::~avtZoneCenterQuery() 
{
}


// ****************************************************************************
//  Method: avtZoneCenterQuery::SetInputParams
//
//  Purpose:    Allows this query to read input parameters set by user.
//
//  Arguments:
//    params:   MapNode containing input parameters.
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 20, 2011
//
//  Modifications:
//    Kathleen Biagas, Thu Jan 10 08:12:47 PST 2013
//    Use newer MapNode methods that check for numeric entries and retrieves 
//    to specific type.
//
// ****************************************************************************

void
avtZoneCenterQuery::SetInputParams(const MapNode &params) 
{
    if(params.HasNumericEntry("use_global_id"))
        useGlobalId = params.GetEntry("use_global_id")->ToBool();

    if(params.HasNumericEntry("domain"))
        domain = params.GetEntry("domain")->ToInt();

    if(params.HasNumericEntry("element"))
        element = params.GetEntry("element")->ToInt();
    else
        EXCEPTION1(QueryArgumentException, "element");
}


// ****************************************************************************
//  Method: avtZoneCenterQuery::GetDefaultInputParams
//
//  Purpose:    Retrieve default input values.
//
//  Arguments:
//    params:   MapNode to store the default input values.
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011
//
// ****************************************************************************

void
avtZoneCenterQuery::GetDefaultInputParams(MapNode &params) 
{
    params["use_global_id"] = 0;
    params["domain"] = 0;
    params["element"] = 0;
}


// ****************************************************************************
//  Method: avtZoneCenterQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Kathleen Bonnell, Fri Jun 11 14:35:50 PDT 2004 
//    Renamed QueryZoneCenter to QueryCoords, added bool arg. 
//
//    Kathleen Bonnell, Tue Jul  6 15:25:02 PDT 2004 
//    Removed MPI calls, use GetFloatArrayToRootProc. 
//
//    Kathleen Bonnell, Thu Dec 16 17:16:33 PST 2004 
//    Moved code that actually finds zone center to FindLocalCenter and
//    FindGlobalCenter. 
//
//    Kathleen Bonnell, Tue Dec 28 14:52:22 PST 2004 
//    Add 'global' to output string as necessary. 
//
//    Cyrus Harrison, Tue Sep 18 13:45:35 PDT 2007
//    Added support for user settable floating point format string
//
//    Kathleen Biagas, Tue Jun 21 10:14:51 PDT 2011
//    Domain, Element, useGlobalId retrieved in SetInputParams.
//
//    Kathleen Biagas, Thu Feb 13 15:04:58 PST 2014
//    Add Xml results.
//
//    Eric Brugger, Thu Aug  3 14:25:20 PDT 2023
//    Correct the setting of the results value and results message in the
//    case where the query fails.
//
// ****************************************************************************

void
avtZoneCenterQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    string floatFormat = queryAtts.GetFloatFormat();
    string format = "";
    
    UpdateProgress(0, 0);

    bool singleDomain = false;
    if (!useGlobalId)
    {
        intVector dlist;
        avtDataRequest_p dataRequest = 
            GetInput()->GetOriginatingSource()->GetFullDataRequest();
        dataRequest->GetSIL().GetDomainList(dlist);

        if (dlist.size() == 1 && dataRequest->UsesAllDomains())
        {
            singleDomain = true;
        }
    }
    else
    {
        singleDomain = true;
    }

    double coord[3] = {0., 0., 0.};
    bool success = false;

    if (!useGlobalId)
        success = FindLocalCenter(coord);    
    else 
        success = FindGlobalCenter(coord);    

    GetDoubleArrayToRootProc(coord, 3, success);   

    if (PAR_Rank() != 0)
        return;

    char msg[120];

    if (success)
    {
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();

        doubleVector c;
        c.push_back(coord[0]);
        c.push_back(coord[1]);
        if (dim == 3)
            c.push_back(coord[2]);
        qA->SetResultsValue(c);

        MapNode result_node;
        result_node["center"] = c;
 
        if (singleDomain)
        {
            string global;
            if (useGlobalId)
            {
                global = "global";
                result_node["global_zone"] = element;
            }
            else
            {
                result_node["zone"] = element;
            }
            if (dim == 2)
            {
                format = "The center of %s zone %d is (" + floatFormat + ", " 
                                                         + floatFormat + ").";
                snprintf(msg, 120, format.c_str(), 
                         global.c_str(), element, c[0], c[1]);
            }
            else 
            {
                format = "The center of %s zone %d is (" + floatFormat + ", " 
                                                         + floatFormat + ", " 
                                                         + floatFormat + ").";
                snprintf(msg, 120, format.c_str(), 
                         global.c_str(), element,
                         c[0], c[1], c[2]);
            }
        }
        else
        {
            string domainName;
            avtOriginatingSource *src = GetInput()->GetOriginatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int dom         = domain  - blockOrigin;
            int ts          = queryAtts.GetTimeStep();
            string var      = queryAtts.GetVariables()[0];
            src->GetDomainName(var, ts, dom, domainName);
            result_node["zone"] = element;
            result_node["domain"] = domainName;
            if (dim == 2)
            {
                format = "The center of zone %d (%s) is (" + floatFormat + ", " 
                                                         + floatFormat + ").";
                snprintf(msg, 120, format.c_str(),
                         element, domainName.c_str(),
                         c[0], c[1]);
            }
            else 
            {
                format = "The center of zone %d (%s) is (" + floatFormat + ", " 
                                                           + floatFormat + ", " 
                                                           + floatFormat + ").";
                snprintf(msg, 120, format.c_str(), 
                         element, domainName.c_str(),
                         c[0], c[1], c[2]);
            }
        }
        qA->SetXmlResult(result_node.ToXML());
    }
    else
    {
        doubleVector c;
        qA->SetResultsValue(c);

        if (singleDomain)
        {
            snprintf(msg, 120, "The center of zone %d could not be determined.",
                     element);
        }
        else
        {
            string domainName;
            avtOriginatingSource *src = GetInput()->GetOriginatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int dom         = domain  - blockOrigin;
            int ts          = queryAtts.GetTimeStep();
            string var      = queryAtts.GetVariables()[0];
            src->GetDomainName(var, ts, dom, domainName);
            snprintf(msg, 120, "The center of zone %d (%s) could not be determined.",
                     element, domainName.c_str());
        }
    }

    qA->SetResultsMessage(msg);
    UpdateProgress(1, 0);
}


// ****************************************************************************
//  Method: avtZoneCenterQuery::FindGlobalCenter
//
//  Purpose:
//    Find the center of globa zone specified in queryAtts.  Must search all
//    domains.
//
//  Returns:
//    true upon succesful location of zone and determination of its center,
//    false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the zone center.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:14:51 PDT 2011
//    Zone retrieved as Element SetInputParams.
//
// ****************************************************************************

bool
avtZoneCenterQuery::FindGlobalCenter(double coord[3])
{
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0]  = 0.;
    coord[1]  = 0.;
    coord[2]  = 0.;

    intVector dlist;
    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    avtOriginatingSource *src = GetInput()->GetOriginatingSource();
    for (size_t i = 0; i < dlist.size() && !success; ++i)
    {
        success = src->QueryCoords(var, dlist[i], element, ts, coord, true, true);
    }
    return success;
}


// ****************************************************************************
//  Method: avtZoneCenterQuery::FindLocalCenter
//
//  Purpose:
//    Find the center of zone specified in queryAtts in domain specified
//    by queryAtts. 
//
//  Returns:
//    true upon succesful location of zone and determination of its center,
//    false otherwise.
//
//  Arguments:
//    coord     A place to store the coordinates of the zone center.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 16, 2004 (moved from method PerformQuery)
//
//  Modifications:
//    Kathleen Biagas, Tue Jun 21 10:14:51 PDT 2011
//    Domain, zone, retrieved in SetInputParams.
//
// ****************************************************************************

bool
avtZoneCenterQuery::FindLocalCenter(double coord[3])
{
    intVector dlist;
    avtDataRequest_p dataRequest = 
        GetInput()->GetOriginatingSource()->GetFullDataRequest();
    dataRequest->GetSIL().GetDomainList(dlist);

    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    int cellOrigin  = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
    int dom         = domain  - blockOrigin;
    int zone        = element - cellOrigin;


    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    dom  = (dom < 0 ? 0 : dom);
    zone = (zone < 0 ? 0 : zone);

    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    //
    //  See if any processor is working with this domain.
    //
    intVector dAllProc;
    trav.GetDomainListAllProcs(dAllProc);
    bool domainUsed = false;
    for (size_t j = 0; j < dAllProc.size() && !domainUsed; j++)
    {
        if (dAllProc[j] == dom)
            domainUsed = true;
    }

    avtOriginatingSource *src = GetInput()->GetOriginatingSource();
    if (domainUsed)
    {
        for (size_t i = 0; i < dlist.size() && !success; ++i)
        {
            if (dlist[i] == dom)
            {
                success = src->QueryCoords(var, dom, zone, ts, coord, true, false);
            }
        }
    }
    else if (PAR_Rank() == 0)
    {
        success = src->QueryCoords(var, dom, zone, ts, coord, true, false);
    }

    return success;
}
