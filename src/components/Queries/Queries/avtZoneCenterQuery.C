// ************************************************************************* //
//                            avtZoneCenterQuery.C                           //
// ************************************************************************* //

#include <avtZoneCenterQuery.h>

#include <avtParallel.h>
#include <avtTerminatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <vector>
#include <snprintf.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtZoneCenterQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 25, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtZoneCenterQuery::avtZoneCenterQuery() : avtDatasetQuery() 
{
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
// ****************************************************************************

void
avtZoneCenterQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    bool singleDomain = false;
    if (!qA->GetUseGlobalId())
    {
        intVector dlist;
        avtDataSpecification_p dspec = 
            GetInput()->GetTerminatingSource()->GetFullDataSpecification();
        dspec->GetSIL().GetDomainList(dlist);

        if (dlist.size() == 1 && dspec->UsesAllDomains())
        {
            singleDomain = true;
        }
    }
    else
    {
        singleDomain = true;
    }

    float coord[3] = {0., 0., 0.};
    bool success = false;

    if (!qA->GetUseGlobalId())
        success = FindLocalCenter(coord);    
    else 
        success = FindGlobalCenter(coord);    

    GetFloatArrayToRootProc(coord, 3, success);   

    if (PAR_Rank() != 0)
        return;

    char msg[120];

    if (success)
    {
        int dim = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
        if (singleDomain)
        {
            if (dim == 2)
            {
                SNPRINTF(msg, 120, "The center of zone %d is (%g, %g).", 
                         qA->GetElement(), coord[0], coord[1]);
            }
            else 
            {
                SNPRINTF(msg, 120, "The center of zone %d is (%g, %g, %g).", 
                         qA->GetElement(), coord[0], coord[1], coord[2]);
            }
        }
        else
        {
            string domainName;
            avtTerminatingSource *src = GetInput()->GetTerminatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int domain      = queryAtts.GetDomain()  - blockOrigin;
            int ts          = queryAtts.GetTimeStep();
            string var      = queryAtts.GetVariables()[0];
            src->GetDomainName(var, ts, domain, domainName);
            if (dim == 2)
            {
                SNPRINTF(msg, 120, "The center of zone %d (%s) is (%g, %g).", 
                         qA->GetElement(), domainName.c_str(),
                         coord[0], coord[1]);
            }
            else 
            {
                SNPRINTF(msg, 120, "The center of zone %d (%s) is (%g, %g, %g).", 
                         qA->GetElement(), domainName.c_str(),
                         coord[0], coord[1], coord[2]);
            }
        }
  
        doubleVector c;
        c.push_back((double)coord[0]);
        c.push_back((double)coord[1]);
        c.push_back((double)coord[2]);
        qA->SetResultsValue(c);
    }
    else
    {
        if (singleDomain)
        {
            SNPRINTF(msg, 120, "The center of zone %d could not be determined.",
                     qA->GetElement());
        }
        else
        {
            avtTerminatingSource *src = GetInput()->GetTerminatingSource();
            int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
            int domain      = queryAtts.GetDomain()  - blockOrigin;
            int ts          = queryAtts.GetTimeStep();
            string var      = queryAtts.GetVariables()[0];
            string domainName;
            src->GetDomainName(var, ts, domain, domainName);
            SNPRINTF(msg, 120, "The center of zone %d (%s) could not be determined.",
                     qA->GetElement(), domainName.c_str());
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
//
// ****************************************************************************

bool
avtZoneCenterQuery::FindGlobalCenter(float coord[3])
{
    int zone        = queryAtts.GetElement();
    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0]  = 0.;
    coord[1]  = 0.;
    coord[2]  = 0.;

    intVector dlist;
    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    bool success = false;

    avtTerminatingSource *src = GetInput()->GetTerminatingSource();
    for (int i = 0; i < dlist.size() && !success; ++i) 
    {
        success = src->QueryCoords(var, dlist[i], zone, ts, coord, true, true);
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
//
// ****************************************************************************

bool
avtZoneCenterQuery::FindLocalCenter(float coord[3])
{

    intVector dlist;
    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();
    dspec->GetSIL().GetDomainList(dlist);

    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    int cellOrigin  = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
    int domain      = queryAtts.GetDomain()  - blockOrigin;
    int zone        = queryAtts.GetElement() - cellOrigin;

    int ts          = queryAtts.GetTimeStep();
    string var      = queryAtts.GetVariables()[0];

    coord[0] = 0.;
    coord[1] = 0.;
    coord[2] = 0.;

    domain = (domain < 0 ? 0 : domain);
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
    for (int j = 0; j < dAllProc.size() && !domainUsed; j++)
    {
        if (dAllProc[j] == domain)
            domainUsed = true;
    }

    avtTerminatingSource *src = GetInput()->GetTerminatingSource();
    if (domainUsed)
    {
        for (int i = 0; i < dlist.size() && !success; ++i) 
        {
            if (dlist[i] == domain)
            {
                success = src->QueryCoords(var, domain, zone, ts, coord, true, false);
            }
        }
    }
    else if (PAR_Rank() == 0)
    {
        success = src->QueryCoords(var, domain, zone, ts, coord, true, false);
    }

    return success;
}
