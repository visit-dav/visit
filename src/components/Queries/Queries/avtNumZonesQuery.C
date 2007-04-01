// ************************************************************************* //
//                            avtNumZonesQuery.C                             //
// ************************************************************************* //

#include <avtNumZonesQuery.h>

#include <avtDatasetExaminer.h>
#include <snprintf.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

// ****************************************************************************
//  Method: avtNumZonesQuery constructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************

avtNumZonesQuery::avtNumZonesQuery() : avtDatasetQuery() 
{
}


// ****************************************************************************
//  Method: avtNumZonesQuery destructor
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//
// ****************************************************************************
avtNumZonesQuery::~avtNumZonesQuery() 
{
}


// ****************************************************************************
//  Method: avtNumZonesQuery::PerformQuery
//
//  Purpose:
//    Perform the requested query. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 18, 2004 
//
//  Modifications:
//    Brad Whitlock, Mon Feb 23 12:11:02 PDT 2004
//    I made it use SNPRINTF to get it to build on Linux.
//    
//    Kathleen Bonnell, Fri Mar  5 15:36:23 PST 2004 
//    Rewrote code to work correctly in parallel when some processors have
//    not done any work and thus have a different value for ghost type. 
//    
//    Kathleen Bonnell, Tue Apr 20 12:49:46 PDT 2004 
//    Modified msg creation so that it works properly on all platforms. 
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
// ****************************************************************************

void
avtNumZonesQuery::PerformQuery(QueryAttributes *qA)
{
    queryAtts = *qA;
    Init(); 

    UpdateProgress(0, 0);

    avtDataObject_p dob = ApplyFilters(GetInput());
    SetTypedInput(dob);

    int usedDomains = 
        GetInput()->GetInfo().GetValidity().GetHasEverOwnedAnyDomain() ? 1 : 0;

    avtGhostType gt = GetInput()->GetInfo().GetAttributes().GetContainsGhostZones();
    int totalZones [2] = {0, 0};
    char msg[200];
    if (usedDomains)
    {
        avtDataset_p input = GetTypedInput();

        if (gt != AVT_HAS_GHOSTS)
        {
            totalZones[0] = avtDatasetExaminer::GetNumberOfZones(input);
        }
        else 
        {
            avtDatasetExaminer::GetNumberOfZones(input, totalZones[0], totalZones[1]);
        }
    }

#ifdef PARALLEL
    int myRank, numProcs;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
    int mpiUsedDomsTag = GetUniqueMessageTag();
    int mpiTzTag = GetUniqueMessageTag();

    if (myRank == 0)
    {
        MPI_Status stat, stat2;

        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&usedDomains, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiUsedDomsTag, MPI_COMM_WORLD, &stat);
            if (usedDomains)
            {
                int tz[2];
                MPI_Recv(tz, 2, MPI_INT, stat.MPI_SOURCE, mpiTzTag,
                         MPI_COMM_WORLD, &stat2);
                totalZones[0] += tz[0];
                totalZones[1] += tz[1];
            }
        }
    }
    else
    {
        MPI_Send(&usedDomains, 1, MPI_INT, 0, mpiUsedDomsTag, MPI_COMM_WORLD);
        if (usedDomains)
        {
            MPI_Send(totalZones, 2, MPI_INT, 0, mpiTzTag, MPI_COMM_WORLD);
        }    
        return;    
    }
#endif

    if (OriginalData())
        SNPRINTF(msg, 200, "The original number of zones is %d.", totalZones[0]);
    else 
        SNPRINTF(msg, 200, "The actual number of zones is %d.", totalZones[0]);
    
    if (gt != AVT_HAS_GHOSTS)
    {
        qA->SetResultsValue((double)totalZones[0]);
        qA->SetResultsMessage(msg);
    }
    else
    {
        char msg2[200];
        SNPRINTF(msg2, 200, "%s\nThe number of ghost zones is %d.", msg, totalZones[1]);
        double results[2] = {(double) totalZones[0], (double) totalZones[1]};
        qA->SetResultsValues(results, 2);
        qA->SetResultsMessage(msg2);
    }
    UpdateProgress(1, 0);
}

