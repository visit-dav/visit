// ************************************************************************* //
//                            avtZoneCenterQuery.C                           //
// ************************************************************************* //

#include <avtZoneCenterQuery.h>
#include <avtTerminatingSource.h>
#include <avtSILRestrictionTraverser.h>
#include <vector>
#include <snprintf.h>
#include <avtParallel.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

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
// ****************************************************************************

void
avtZoneCenterQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    bool singleDomain = false;
    intVector dlist;
    avtDataSpecification_p dspec = 
        GetInput()->GetTerminatingSource()->GetFullDataSpecification();
    dspec->GetSIL().GetDomainList(dlist);

    if (dlist.size() == 1 && dspec->UsesAllDomains())
    {
        singleDomain = true;
    }

    int blockOrigin = GetInput()->GetInfo().GetAttributes().GetBlockOrigin();
    int cellOrigin  = GetInput()->GetInfo().GetAttributes().GetCellOrigin();
    int dim         = GetInput()->GetInfo().GetAttributes().GetSpatialDimension();
    int domain      = qA->GetDomain()  - blockOrigin;
    int zone        = qA->GetElement() - cellOrigin;
    int ts          = qA->GetTimeStep();
    string var      = qA->GetVariables()[0];

    float coord[3] = {0., 0., 0.};

    domain = (domain < 0 ? 0 : domain);
    zone = (zone < 0 ? 0 : zone);

    avtSILRestrictionTraverser trav(querySILR);
    trav.GetDomainList(dlist);
    int success = 0;

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
                success = (int)src->QueryZoneCenter(var, domain, zone, ts, coord);
            }
        }
    }
    else if (PAR_Rank() == 0)
    {
        success = (int)src->QueryZoneCenter(var, domain, zone, ts, coord);
    }
    

#ifdef PARALLEL
    int myRank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if (myRank == 0)
    {
        MPI_Status stat, stat2;
        int good; 
        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&good, 1, MPI_INT, MPI_ANY_SOURCE,
                     MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (good)
            {
                MPI_Recv(coord, 3, MPI_FLOAT, stat.MPI_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &stat2);
                success = good;
            }
        }
    }
    else
    {
        MPI_Send(&success, 1, MPI_INT, 0, myRank, MPI_COMM_WORLD);
        if (success)
        {
            MPI_Send(coord, 3, MPI_FLOAT, 0, myRank, MPI_COMM_WORLD);
        }    
        return;    
    }
#endif
 
    char msg[120];

    if (success)
    {
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
            string domainName;
            src->GetDomainName(var, ts, domain, domainName);
            SNPRINTF(msg, 120, "The center of zone %d (%s) could not be determined.",
                     qA->GetElement(), domainName.c_str());
        }
    }

    qA->SetResultsMessage(msg);
    UpdateProgress(1, 0);
}

