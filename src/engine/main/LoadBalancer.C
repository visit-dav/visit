// ************************************************************************* //
//                                LoadBalancer.C                             //
// ************************************************************************* //

#include <LoadBalancer.h>

#include <stdlib.h>

#include <vector>
#include <deque>
#include <set>

#include <avtOriginatingSink.h>
#include <avtTerminatingSource.h>
#include <avtIOInformation.h>
#include <avtSILRestrictionTraverser.h>
#include <VisItException.h>
#include <DebugStream.h>
#include <AbortException.h>
#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using     std::string;
using     std::vector;
using     std::deque;
using     std::set;


//
// Function Prototypes.
//

static avtDataSpecification_p    ReduceCallback(void *,
                                                   avtPipelineSpecification_p);
static bool                      DynamicCheckerCallback(void *,
                                                   avtPipelineSpecification_p);
static bool                      ContinueCallback(void *, int);

//
// Static class data
//
ParAbortCallback        LoadBalancer::abortCallback = NULL;
void                   *LoadBalancer::abortCallbackArgs = NULL;
ProgressCallback        LoadBalancer::progressCallback = NULL;
void                   *LoadBalancer::progressCallbackArgs = NULL;

bool                    LoadBalancer::forceStatic  = false;
bool                    LoadBalancer::forceDynamic = false;
LoadBalanceScheme       LoadBalancer::scheme       =
                                       LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER;

// ****************************************************************************
//  Method:  LoadBalancer::ForceStatic
//
//  Purpose:
//    Force static load balancing when possible.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
// ****************************************************************************

void
LoadBalancer::ForceStatic()
{
    LoadBalancer::forceStatic = true;
}

// ****************************************************************************
//  Method:  LoadBalancer::ForceDynamic
//
//  Purpose:
//    Force dynamic load balancing when possible.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
// ****************************************************************************

void
LoadBalancer::ForceDynamic()
{
    LoadBalancer::forceDynamic = true;
}

// ****************************************************************************
//  Method:  LoadBalancer::SetScheme
//
//  Purpose:
//      Sets the load balancing scheme used when static load balancing.
//
//  Programmer:  Hank Childs
//  Creation:    May 12, 2003
//
// ****************************************************************************

void
LoadBalancer::SetScheme(LoadBalanceScheme s)
{
    LoadBalancer::scheme = s;
}

// ****************************************************************************
//  Method: LoadBalancer::RegisterAbortCallback
//
//  Purpose:
//      Registers the AbortCallback.
//
//  Arguments:
//      pc      The abort callback.
//      args    The arguments to the abort callback.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 20, 2001
//
// ****************************************************************************

void
LoadBalancer::RegisterAbortCallback(ParAbortCallback pc, void *args)
{
    abortCallback     = pc;
    abortCallbackArgs = args;
}

// ****************************************************************************
//  Method: LoadBalancer::RegisterProgressCallback
//
//  Purpose:
//      Registers the ProgressCallback.
//
//  Arguments:
//      pc      The progress callback.
//      args    The arguments to the progress callback.
//
//  Programmer: Jeremy Meredith
//  Creation:   September 19, 2001
//
// ****************************************************************************

void
LoadBalancer::RegisterProgressCallback(ProgressCallback pc, void *args)
{
    progressCallback     = pc;
    progressCallbackArgs = args;
}

// ****************************************************************************
//  Method:  LoadBalancer::CheckAbort
//
//  Purpose:
//    Check for an abort message
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 20, 2001
//
// ****************************************************************************

bool
LoadBalancer::CheckAbort(bool informSlaves)
{
    return abortCallback(abortCallbackArgs, informSlaves);
}

// ****************************************************************************
//  Method:  LoadBalancer::UpdateProgress
//
//  Purpose:
//    Send a progress update using the registered callback.
//
//  Arguments:
//    current    number of parts processed
//    total      total number of parts
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 19, 2001
//
// ****************************************************************************

void
LoadBalancer::UpdateProgress(int current, int total)
{
    if (progressCallback != NULL)
        progressCallback(progressCallbackArgs, "Calculating", "Calculating",
                         current, total);
}

// ****************************************************************************
//  Method: LoadBalancer constructor
//
//  Arguments:
//      np      The number of processors.
//      r       The index of this processor (0-origin).
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 12:29:29 PDT 2001
//    Use a data structure for pipeline info.
//
//    Jeremy Meredith, Thu Sep 20 00:49:26 PDT 2001
//    Added the registration of the DynamicChecker callback.
//
// ****************************************************************************

LoadBalancer::LoadBalancer(int np, int r)
{
    //
    // Pipeline index 0 is reserved for meta-data pipelines.
    //
    pipelineInfo.push_back(LBInfo("dummy_pipeline"));

    rank   = r;
    nProcs = np;

    //
    // Register callbacks with the avt pipeline.
    //
    avtTerminatingSource::SetLoadBalancer(ReduceCallback, this);
    avtTerminatingSource::SetDynamicChecker(DynamicCheckerCallback, this);
    avtOriginatingSink::SetGuideFunction(ContinueCallback, this);
}


// ****************************************************************************
//  Method: LoadBalancer::CheckDynamicLoadBalancing
//
//  Purpose:
//      Takes in the pipeline specification for the entire pipeline and
//      determines whether or not it will be dynamically load balanced
//
//  Arguments:
//      spec    A pipeline specification.
//
//  Returns:    true if dynamic; false if static or none
//
//  Programmer: Jeremy Meredith
//  Creation:   September 19, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 21 14:39:58 PDT 2001
//    Added checks for forcing static/dynamic load balancing.
//
// ****************************************************************************

bool
LoadBalancer::CheckDynamicLoadBalancing(avtPipelineSpecification_p input)
{
#ifdef PARALLEL
    avtDataSpecification_p data = input->GetDataSpecification();

    // Some hard and fast rules

    // Pipeline index 0 is reserved for meta-data.
    // No load balancing for single processor engines.
    // Also, cannot dynamic load balance some pipelines.
    if (input->GetPipelineIndex() == 0 ||
        nProcs <= 1                    ||
        input->ShouldUseDynamicLoadBalancing() == false)
    {
        return false;
    }

    // Check for forcing of static
    if (forceStatic)
        return false;

    // Check for forcing of static
    if (forceDynamic)
        return true;

    // Here's the heuristics for dynamic load balancing
    // Also, make them use a minimum of 4 procs for dynamic load balancing
    if (nProcs > 3)
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return false;
#endif
}

// ****************************************************************************
//  Method: LoadBalancer::Reduce
//
//  Purpose:
//      Takes in the pipeline specification for the entire pipeline and
//      determines which portion this processor should do on this pass.
//
//  Arguments:
//      spec    A pipeline specification.
//
//  Returns:    A data specification that is a subset of the data specification
//              in the pipeline specification.
//
//  Programmer: Hank Childs
//  Creation:   June 17, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 12:21:57 PDT 2001
//    Added dynamic load balancing.
//
//    Hank Childs, Wed Aug  8 16:48:45 PDT 2001
//    Added intersection with original restriction when in parallel (to
//    preserve restrictions that don't involve domains).
//
//    Jeremy Meredith, Wed Aug 29 12:07:23 PDT 2001
//    Fixed a bug in static load balancing -- there was overlap in 
//    assignment of domains, preventing all domains from being assigned
//    to processors.
//
//    Jeremy Meredith, Mon Sep 17 23:07:43 PDT 2001
//    Made the master be aware of the domains already cached on each 
//    processor, and to choose those domains for a processor when possible.
//
//    Jeremy Meredith, Thu Sep 20 00:52:37 PDT 2001
//    Made it use the CheckDynamicLoadBalancing function.
//    Made it send progress updates in dynamic mode.
//    Added logic to make full use of IO hints.
//
//    Jeremy Meredith, Fri Sep 21 14:41:08 PDT 2001
//    Added support for aborting dynamic loadbalanced execution.
//    Changed dynamic loadbalancing to delay sending the "complete"
//    signal to slave processes until it is sure no abort has happened.
//
//    Hank Childs, Mon Dec  2 14:46:03 PST 2002
//    Use a SIL restriction traverser to find the domain list.
//
//    Hank Childs, Wed Dec  4 17:23:12 PST 2002
//    Made use of a more efficient call to intersect SIL restrictions.
//
//    Hank Childs, Mon May 12 19:34:54 PDT 2003
//    Account for different load balancing schemes.
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
// ****************************************************************************

avtDataSpecification_p
LoadBalancer::Reduce(avtPipelineSpecification_p input)
{
    avtDataSpecification_p data = input->GetDataSpecification();

    //
    // Pipeline index 0 is reserved for meta-data.  It should already be
    // load balanced.
    //
    if (input->GetPipelineIndex() == 0)
    {
        return data;
    }

    //
    // No load balancing needed for single-processor engines
    //
    if (nProcs <= 1)
    {
        pipelineInfo[input->GetPipelineIndex()].complete = true;
        return data;
    }

#ifdef PARALLEL

    avtSILRestriction_p orig_silr   = data->GetRestriction();
    avtSILRestriction_p silr        = new avtSILRestriction(orig_silr);
    avtDataSpecification_p new_data = new avtDataSpecification(data, silr);
    avtSILRestrictionTraverser trav(silr);

    // set up MPI message tags
    int lastDomDoneMsg = GetUniqueMessageTag();
    int newDomToDoMsg = GetUniqueMessageTag();

    // Can we do dynamic load balancing?
    if (! CheckDynamicLoadBalancing(input))
    {
        //
        // We probably want to do something more sophisticated in the future 
        // (like walking through a SIL).  For now, just use the "chunks"
        // mechanism set up with convenience methods.
        //
        vector<int> list;
        vector<int> mylist;
        trav.GetDomainList(list);

        if (scheme == LOAD_BALANCE_CONTIGUOUS_BLOCKS_TOGETHER)
        {
            int amountPer = list.size() / nProcs;
            int oneExtraUntil = list.size() % nProcs;
            int lastDomain = 0;
            for (int i = 0 ; i < nProcs ; i++)
            {
                if (i == rank)
                {
                    int amount = amountPer + (i < oneExtraUntil ? 1 : 0);
                    for (int j = 0 ; j < amount ; j++)
                    {
                        mylist.push_back(list[j+lastDomain]);
                    }
                }
                lastDomain += amountPer + (i < oneExtraUntil ? 1 : 0);
            }
        }
        else if (scheme == LOAD_BALANCE_STRIDE_ACROSS_BLOCKS)
        {
            for (int j = 0 ; j < list.size() ; j++)
            {
                if (j % nProcs == rank)
                    mylist.push_back(list[j]);
            }
        }
        else if (scheme == LOAD_BALANCE_RANDOM_ASSIGNMENT)
        {
            // all procs randomly jumble the list of domain ids
            // all procs compute same jumbled list due to same seed
            // [ which won't be true on a heterogeneous platform ]
            int j;
            vector<int> jumbledList = list;
            srand(0xDeadBeef);
            for (j = 0 ; j < list.size() * 5; j++)
            {
               int i1 = rand() % list.size();
               int i2 = rand() % list.size();
               int tmp = jumbledList[i1];
               jumbledList[i1] = jumbledList[i2];
               jumbledList[i2] = tmp;
            }
            // now, do round-robin assignment from the jumbled list
            for (j = 0 ; j < list.size() ; j++)
            {
                if (j % nProcs == rank)
                    mylist.push_back(jumbledList[j]);
            }
        }

        silr->RestrictDomainsForLoadBalance(mylist);
        pipelineInfo[input->GetPipelineIndex()].complete = true;
    }
    else
    {
        // disable progress updates from the filters this time around
        avtDataObjectSource::RegisterProgressCallback(NULL,NULL);

        LBInfo &lbInfo(pipelineInfo[input->GetPipelineIndex()]);
        IOInfo &ioInfo(ioMap[lbInfo.db]);
        if (rank == 0)
        {
            // -------------------------------------
            //     MASTER LOADBALANCER PROCESSES
            // -------------------------------------

            // Allocate enough space to hold the completed domains
            ioInfo.domains.resize(nProcs);
            ioInfo.files.resize(nProcs);

            // Get the list of domains to process
            vector<int> domainList;
            trav.GetDomainList(domainList);

            // Make a work list and a completed list
            int         totaldomains = domainList.size();
            deque<int>  incomplete(domainList.begin(), domainList.end());
            vector<int> complete;

            debug5 << "LoadBalancer Master -- starting with " 
                   << incomplete.size() << " domains\n";

            // pull from the incomplete list and push onto the complete list
            // until all domains are complete
            bool abort = false;
            int domain;
            UpdateProgress(0,0);
            while (complete.size() < totaldomains)
            {
                // check for an abort
                if (!abort &&
                    CheckAbort(false))
                {
                    abort = true;
                    totaldomains -= incomplete.size();
                    incomplete.clear();
                }

                // update the progress
                UpdateProgress(complete.size() + (domainList.size() - incomplete.size()),
                               domainList.size()*2);


                // get the completed domain number
                MPI_Status stat;
                MPI_Recv(&domain, 1, MPI_INT, MPI_ANY_SOURCE,
                         lastDomDoneMsg, MPI_COMM_WORLD, &stat);
                int processor = stat.MPI_SOURCE;

                // -1 means the first pass by the slave; nothing completed yet
                if (domain != -1)
                {
                    // add it to the complete list
                    complete.push_back(domain);
                }

                // figure out what to tell this processor to do
                if (incomplete.empty())
                    continue;

                // find a cached domain for next processor
                deque<int>::iterator i;
                for (i = incomplete.begin(); i != incomplete.end(); i++)
                {
                    if (ioInfo.domains[processor].find(*i) != 
                        ioInfo.domains[processor].end())
                        break;
                }
                // if no match, try to find one that is in a file
                // already opened by this processor
                if (i == incomplete.end())
                {
                    for (i = incomplete.begin(); i != incomplete.end(); i++)
                    {
                        int fileno = ioInfo.fileMap[*i];
                        if (ioInfo.files[processor].count(fileno) > 0)
                            break;
                    }
                }
                // if still no match, find one that is in a file
                // opened by the fewest number of processors
                if (i == incomplete.end())
                {
                    int mindomain = -1;
                    int minopen   = 999999999;
                    for (i = incomplete.begin(); i != incomplete.end(); i++)
                    {
                        int fileno = ioInfo.fileMap[*i];
                        // count the number of processors which have
                        // this file opened
                        int nopen = 0;
                        for (int j=0; j<ioInfo.files.size(); j++)
                            if (ioInfo.files[j].count(fileno) > 0)
                                nopen++;
                        if (nopen < minopen)
                        {
                            mindomain = *i;
                            minopen   = nopen;
                        }
                    }
                    for (i = incomplete.begin(); i != incomplete.end(); i++)
                    {
                        if (*i == mindomain)
                            break;
                    }
                }                    

                // if no match, just take the next one in line
                if (i == incomplete.end())
                    i=incomplete.begin();

                domain = *i;
                incomplete.erase(i);

                ioInfo.domains[processor].insert(domain);
                ioInfo.files[processor].insert(ioInfo.fileMap[domain]);

                // send the new domain number to that processor
                debug5 << "LoadBalancer Master: sending domain " 
                       << domain << " to processor "<<processor<<"\n";
                MPI_Send(&domain, 1, MPI_INT, processor, newDomToDoMsg, MPI_COMM_WORLD);
            }

            // we're all done -- -2 means to abort, -1 means to send results
            int status = abort ? -2 : -1;
            for (int i=1; i<nProcs; i++)
                MPI_Send(&status, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

            if (abort)
                EXCEPTION0(AbortException);

            // all work is done
            UpdateProgress(1,0);
            lbInfo.complete = true;
            new_data->GetRestriction()->TurnOffAll();
            MPI_Barrier(MPI_COMM_WORLD);
        }
        else
        {
            // -------------------------------------
            //            SLAVE PROCESSES
            // -------------------------------------

            // send our last completed domain to the master
            int domain = lbInfo.current;
            MPI_Send(&domain, 1, MPI_INT, 0, lastDomDoneMsg, MPI_COMM_WORLD);

            // get our new work unit
            MPI_Status stat;
            MPI_Recv(&domain, 1, MPI_INT, 0, newDomToDoMsg, MPI_COMM_WORLD, &stat);
            lbInfo.current = domain;

            if (domain == -2)
            {
                EXCEPTION0(AbortException);
            }
            else if (domain == -1)
            {
                //  -1 is a tag for "no work" -- we are all done
                lbInfo.complete = true;
                new_data->GetRestriction()->TurnOffAll();
                MPI_Barrier(MPI_COMM_WORLD);
            }
            else
            {
                vector<int> domainList(1, domain);
                new_data->GetRestriction()->RestrictDomainsForLoadBalance(domainList);
            }
        }
    }

    // By intersecting with the original restricting, we will ensure that
    // we are catching restrictions beyond domains, like materials, etc.
    // See comments in SIL restriction code regarding 'FastIntersect'.
    new_data->GetRestriction()->FastIntersect(orig_silr);

    return new_data;
#else
    EXCEPTION1(VisItException, "nprocs was > 1 in a non-parallel code");
#endif

}


// ****************************************************************************
//  Method: LoadBalancer::AddDatabase
//
//  Purpose:
//      Add a database to the load balancer.  Also adds the I/O information 
//      that should be used when balancing a load.
//
//  Arguments:
//      name     The name of the database.
//      ioinfo   Information about which domains should be grouped together on
//               the same processor.
//
//  Notes:  This will need to expand to support IO restrictions for
//          clustered (non-global) file systems.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
//  Modifications:
//    Jeremy Meredith, Thu Sep 20 00:54:58 PDT 2001
//    Added setting of fileMap from the iohints.
//
// ****************************************************************************

void
LoadBalancer::AddDatabase(const string &db, const avtIOInformation &io)
{
    ioMap[db].ioInfo = io;
    ioMap[db].fileMap.resize(io.GetNDomains());

    debug4 << "LoadBalancer::AddDatabase - db=" << db.c_str() << endl;
    debug4 << "    iohints=[";
    const HintList &hints = io.GetHints();
    for (int i=0; i<hints.size(); i++)
    {
        debug4 << " {";
        for (int j=0; j<hints[i].size(); j++)
        {
            ioMap[db].fileMap[hints[i][j]] = i;
            debug4 << hints[i][j];
            if (j<hints[i].size()-1) debug4 << ",";
        }
        debug4 << "}";
        if (i<hints.size()-1)
            debug4 << "\n             ";
    }
    debug4 << "]  " << endl;
}

 
// ****************************************************************************
//  Method: LoadBalancer::AddPipeline
//
//  Purpose:
//      Creates a unique pipeline index for an avt pipeline.  Also tells the
//      load balancer what database is associated with that pipeline.
//
//  Arguments:
//      name     The name of the database.
//
//  Returns:     A unique index for a pipeline.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
// ****************************************************************************

int
LoadBalancer::AddPipeline(const string &db)
{
    int index = pipelineInfo.size();
    pipelineInfo.push_back(LBInfo(db));
    return index;
}

 
// ****************************************************************************
//  Method: LoadBalancer::ResetPipeline
//
//  Purpose:
//      Resets the status of a pipeline so it can re-execute.
//
//  Arguments:
//      index   The pipeline index.
//
//  Programmer: Hank Childs
//  Creation:   November 21, 2001
//
// ****************************************************************************

void
LoadBalancer::ResetPipeline(int index)
{
    if (index < 0 || index >= pipelineInfo.size())
    {
        debug1 << "Given an invalid pipeline index to reset (" << index << ")."
               << endl;
        return;
    }

    pipelineInfo[index].complete = false;
    pipelineInfo[index].current  = -1;
}

   
// ****************************************************************************
//  Method: LoadBalancer::ContinueExecute
//
//  Purpose:
//      Determines if all of the data has been read in for a pipeline and if
//      it can stop executing.  Data object sinks call this method.
//
//  Arguments:
//      index    A pipeline index.
//
//  Returns:     true if that pipeline needs to continue executing.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 26, 2001
//
// ****************************************************************************

bool
LoadBalancer::ContinueExecute(int index)
{
    return (! pipelineInfo[index].complete);
}


// ****************************************************************************
//  Function: ReduceCallback
//
//  Purpose:
//      A C function that can be registered with avt pipelines and can serve
//      as a callback when a pipeline needs to be balanced.  This should be
//      registered by a LoadBalancer when that object is contructed.
//
//  Arguments:
//      ptr      A pointer to the load balancer.
//      spec     The pipeline specification to balance.
//
//  Returns:     The data specification.
//
//  Programmer:  Hank Childs
//  Creation:    June 17, 2001
//
// ****************************************************************************

static avtDataSpecification_p
ReduceCallback(void *ptr, avtPipelineSpecification_p spec)
{
    LoadBalancer *lb = (LoadBalancer *) ptr;
    return lb->Reduce(spec);
}


// ****************************************************************************
//  Function: DynamicCheckerCallback
//
//  Purpose:
//      A C function that can be registered with avt pipelines and can serve
//      as a callback when one needs to determine if the loadbalancer will
//      perform dynamic load balancing.  This should be registered by a 
//      LoadBalancer when that object is contructed.
//
//  Arguments:
//      ptr      A pointer to the load balancer.
//      spec     The pipeline specification to balance.
//
//  Returns:     The data specification.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 19, 2001
//
// ****************************************************************************

static bool
DynamicCheckerCallback(void *ptr, avtPipelineSpecification_p spec)
{
    LoadBalancer *lb = (LoadBalancer *) ptr;
    return lb->CheckDynamicLoadBalancing(spec);
}


// ****************************************************************************
//  Function: ContinueCallback
//
//  Purpose:
//      A C function that can be registered with avt pipelines and can serve as
//      a callback when a pipeline is not sure if it should continue executing.
//      This should be registered by a LoadBalancer when that object is
//      constructed.
//
//  Arguments:
//      ptr       A pointer to the load balancer.
//      index     A pipeline index.
//
//  Returns:      true if the object should continue executing.
//
//  Programmer:   Hank Childs
//  Creation:     June 17, 2001
//
// ****************************************************************************

static bool
ContinueCallback(void *ptr, int index)
{
    LoadBalancer *lb = (LoadBalancer *) ptr;
    return lb->ContinueExecute(index);
}



