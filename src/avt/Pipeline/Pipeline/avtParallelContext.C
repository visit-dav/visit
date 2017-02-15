/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
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
#include <avtParallelContext.h>
#include <avtParallel.h>

#ifdef PARALLEL
  #include <BufferConnection.h>
#endif

#include <DebugStream.h>
#include <ImproperUseException.h>
#include <AttributeGroup.h>

#include <ref_ptr.h>

#include <cstring>
#include <float.h>

// Minimum value for use in GetUniqueMessageTag
// So that certain other tags can be hard-coded with values < MIN_TAG_VALUE
// if they should be needed prior to MPI_Init
#define MIN_TAG_VALUE 100

#ifdef PARALLEL
// ****************************************************************************
// Class: Communicator
//
// Purpose:
//   Wrapper for an MPI communicator so we have an object we can use with ref_ptr.
//
// Notes:    Instances of this class are used with ref_ptr so we can pass around
//           copies of a communicator and also have communicators self-delete
//           when they are no longer needed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 18 00:41:32 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class Communicator
{
public:
    Communicator()
    {
        if(VISIT_MPI_COMM_PTR == NULL)
            mpicomm = MPI_COMM_WORLD;
        else
            mpicomm = VISIT_MPI_COMM;
        ownsCommunicator = false;
    }

    Communicator(const Communicator &obj)
    {
        mpicomm = obj.mpicomm;
        ownsCommunicator = obj.ownsCommunicator;
    }

    ~Communicator()
    {
        if(ownsCommunicator)
        {
debug5 << "avtParallelContext::PrivateData::~PrivateData: deleting mpicomm" << endl;
            MPI_Comm_free(&mpicomm);
            ownsCommunicator = false;
        }
    }

    void operator = (const Communicator &obj)
    {
        mpicomm = obj.mpicomm;
        ownsCommunicator = obj.ownsCommunicator;
    }

    MPI_Comm mpicomm;
    bool     ownsCommunicator;
};
#endif

// ****************************************************************************
// Class: avtParallelContext::PrivateData
//
// Purpose:
//   Private data for the parallel context.
//
// Notes:    We store the communicator here. We could store other things like
//           per-thread data, etc long term.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 16:51:00 PDT 2015
//
// Modifications:
//
// ****************************************************************************

class avtParallelContext::PrivateData
{
public:
    PrivateData()
    {
        group_rank = 0;
        group_size = 1;
#ifdef PARALLEL
        communicator = new Communicator;
#endif
    }
    PrivateData(const PrivateData &obj)
    {
        group_rank = obj.group_rank;
        group_size = obj.group_size;
#ifdef PARALLEL
        communicator = obj.communicator;
#endif
    }

    ~PrivateData()
    {
    }

    int group_rank;
    int group_size;
#ifdef PARALLEL
    // We use a ref_ptr so we can have many objects use reference counting to
    // access the same communicator and have it go away when we no longer need it.
    ref_ptr<Communicator> communicator;

    static MPI_Op AVT_MPI_MINMAX;
    static int    mpiTagUpperBound;
    static int    mpiTag;
#endif
    // We could add other things for thread context, etc. down the road.
};

#ifdef PARALLEL
MPI_Op avtParallelContext::PrivateData::AVT_MPI_MINMAX   = MPI_OP_NULL;
int    avtParallelContext::PrivateData::mpiTagUpperBound = 32767;
int    avtParallelContext::PrivateData::mpiTag           = MIN_TAG_VALUE;
#endif

// ****************************************************************************
// Method: avtParallelContext::avtParallelContext
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 16:51:46 PDT 2015
//
// Modifications:
//
// ****************************************************************************

avtParallelContext::avtParallelContext()
{
    d = new PrivateData();
}

avtParallelContext::avtParallelContext(const avtParallelContext &obj)
{
    d = new PrivateData(*obj.d);
}

avtParallelContext::~avtParallelContext()
{
    delete d;
}

void
avtParallelContext::operator = (const avtParallelContext &obj)
{
    delete d;
    d = new PrivateData(*obj.d);
}

void
avtParallelContext::Init()
{
#ifdef PARALLEL
    int success = 0;
    // MPI_Attr_get requires void *
    // Also, MPI_TAG_UB is perm attr of the lib and accessible only
    // from MPI_COMM_WORLD
    void *value;
    MPI_Attr_get(MPI_COMM_WORLD, MPI_TAG_UB, &value, &success);
    if (success)
    {
        avtParallelContext::PrivateData::mpiTagUpperBound = *(int*)value;
    }
    else
    {
        // Cannot use debug logs here, because they haven't been initialized.
        cerr << "Unable to get value for MPI_TAG_UB, assuming 32767." << endl;
        cerr << "success = " << success << endl;
        avtParallelContext::PrivateData::mpiTagUpperBound = 32767;
    }
#endif
}

#ifdef PARALLEL
MPI_Comm
avtParallelContext::GetCommunicator()
{
    return d->communicator->mpicomm;
}

void
avtParallelContext::SetCommunicator(MPI_Comm comm, bool owns)
{
    d->communicator->mpicomm = comm;
    d->communicator->ownsCommunicator = owns;
}
#endif

int
avtParallelContext::Rank()
{
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(this->GetCommunicator(), &rank);
#endif
    return rank;
}

int
avtParallelContext::Size()
{
    int size = 1;
#ifdef PARALLEL
    MPI_Comm_size(this->GetCommunicator(), &size);
#endif
    return size;
}

int
avtParallelContext::GroupRank()
{
    return d->group_rank;
}

int
avtParallelContext::GroupSize()
{
    return d->group_size;
}

int
avtParallelContext::GlobalRank()
{
    int rank = 0;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
#endif
    return rank;
}

int
avtParallelContext::GlobalSize()
{
    int size = 1;
#ifdef PARALLEL
    MPI_Comm_size(VISIT_MPI_COMM, &size);
#endif
    return size;
}

// ****************************************************************************
// Method: avtParallelContext::CreateGroup
//
// Purpose:
//   Create a new group of processors that can communicate and return a new
//   parallel context for them.
//
// Arguments:
//   ranks : The ranks to include from the current communicator.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 16:27:07 PDT 2015
//
// Modifications:
//
// ****************************************************************************

avtParallelContext
avtParallelContext::CreateGroup(const std::vector<int> &ranks)
{
#ifdef PARALLEL
    avtParallelContext ctx(*this);

    if(!ranks.empty())
    {
        MPI_Group orig_group, new_group;
        // Get the original group for the communicator.
        MPI_Comm_group(this->GetCommunicator(), &orig_group);
        // Create a smaller group of ranks.
        int *nonconst = new int[ranks.size()];
        memcpy(nonconst, &ranks[0], ranks.size() * sizeof(int));
        MPI_Group_incl(orig_group, ranks.size(), nonconst, &new_group);
        delete [] nonconst;
        // Create a new communicator using the smaller group of ranks.
        ctx.d->communicator = new Communicator;
        MPI_Comm_create(this->GetCommunicator(), new_group, &ctx.d->communicator->mpicomm);
        ctx.d->communicator->ownsCommunicator = true;
    }

    return ctx;
#else
    return *this;
#endif
}

// ****************************************************************************
// Method: avtParallelContext::CreateGroupsOfN
//
// Purpose:
//   Create a new group of N processors that can communicate and return a new
//   parallel context for them. Each group of N processors will be a member of
//   the same parallel context group.
//
// Arguments:
//   N : The number of ranks to include in each group.
//
// Returns:    
//
// Note:       The grouping will put leftover ranks (group smaller than N) into
//             the last group so the last group might have fewer than N members.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 16:27:07 PDT 2015
//
// Modifications:
//
// ****************************************************************************

avtParallelContext
avtParallelContext::CreateGroupsOfN(int N)
{
#ifdef PARALLEL
    int size = Size();
    int rank = Rank();

    // Adjust the number of groups to make an extra group for the leftovers.
    int numberOfGroups = size / N;
    if(numberOfGroups*N < size)
        ++numberOfGroups;

    int myGroup = rank / N;
    int offsetToGroup = N * myGroup;
    int groupSize = N;
    if(myGroup == numberOfGroups-1)
        groupSize = size - offsetToGroup;

    avtParallelContext ctx(*this);
    MPI_Group orig_group, new_group;
    // Get the original group for the communicator.
    MPI_Comm_group(this->GetCommunicator(), &orig_group);
    // Create a smaller group using a range of ranks.
    int ranges[1][3];
    ranges[0][0] = offsetToGroup;
    ranges[0][1] = offsetToGroup + groupSize-1;
    ranges[0][2] = 1;
    MPI_Group_range_incl(orig_group, 1, ranges, &new_group);
    // Create a new communicator using the smaller group of ranks.
    ctx.d->communicator = new Communicator;
    MPI_Comm_create(this->GetCommunicator(), new_group, &ctx.d->communicator->mpicomm);
    ctx.d->communicator->ownsCommunicator = true;

    // Record where the new group is in the groups we made.
    ctx.d->group_rank = myGroup;
    ctx.d->group_size = numberOfGroups;

    return ctx;
#else
    return *this;
#endif
}

// ****************************************************************************
// Method: avtParallelContext::Split
//
// Purpose:
//   Create a new group of processors that belong to a user-specified group.
//
// Arguments:
//   groupId : The id of the group.
//   nGroups : The number of possible groups we're creating.
//
// Returns:    
//
// Note:       All ranks that are part of the context's communicator must 
//             call this.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug  4 16:27:07 PDT 2015
//
// Modifications:
//
// ****************************************************************************

avtParallelContext
avtParallelContext::Split(int groupId, int nGroups)
{
#ifdef PARALLEL
    avtParallelContext ctx(*this);
    ctx.d->communicator = new Communicator;
    MPI_Comm_split(this->GetCommunicator(), groupId, 0, &ctx.d->communicator->mpicomm);
    ctx.d->communicator->ownsCommunicator = true;

    ctx.d->group_rank = groupId;
    ctx.d->group_size = nGroups;
debug5 << "avtParallelContext::Split: created new communicator for group " << groupId << endl;
debug5 << "avtParallelContext::Split: rank=" << ctx.Rank() << ", size=" << ctx.Size() << endl;
debug5 << "avtParallelContext::Split: grouprank=" << ctx.GroupRank() << ", groupsize=" << ctx.GroupSize() << endl;
debug5 << "avtParallelContext::Split: globalrank=" << ctx.GlobalRank() << ", globalsize=" << ctx.GlobalSize() << endl;
    return ctx;
#else
    return *this;
#endif
}

//
// Functions adapted from avtParallel.
//

// ****************************************************************************
//  Function: MinMaxOp
//
//  Purpose:
//      User defined MPI reduction operator. We can assume double values
//
//  Programmer: Mark C. Miller
//  Creation:   January 29, 2004
//
// ****************************************************************************

#ifdef PARALLEL
static void
MinMaxOp(void *ibuf, void *iobuf, int *len, MPI_Datatype *)
{
    int i;
    double *iovals = (double *) iobuf;
    double  *ivals = (double *) ibuf;


    // there is a chance, albeit slim for small values of *len, that if MPI
    // decides to chop up the buffers, it could decide to chop them on an
    // odd boundary. That would be catastrophic!
    if (*len % 2 != 0)
    {
        EXCEPTION0(ImproperUseException);
    }

    // handle the minimums by copying any values in ibuff that are less than
    // respective value in iobuff into iobuff
    for (i = 0; i < *len; i += 2)
    {
        if (ivals[i] < iovals[i])
            iovals[i] = ivals[i];
    }

    // handle the maximums by copying any values in ibuff that are greater than
    // respective value in iobuff into iobuff
    for (i = 1; i < *len; i += 2)
    {
        if (ivals[i] > iovals[i])
            iovals[i] = ivals[i];
    }
}
#endif

// ****************************************************************************
//  Function: avtParallelContext::UnifyMinMax
//
//  Purpose:
//      Makes a collective call across all processors to unify an array that
//      has alternating minimum and maximum values.
//
//  Arguments:
//      buff     An array that contains alternating min's and max's.
//      size     The size of exts (this clearly must be even).
//
//  Programmer:  Hank Childs
//  Creation:    June 18, 2001
//
//  Modifications
//
//    Mark C. Miller, Thu Jan 29 21:26:10 PST 2004
//    Modified to use a single all reduce with the user-defined reduction
//    operator. Also modified to accept an alternate size to use if there is
//    a chance that all processors don't agree on size upon entering. ALL
//    PROCESSORS MUST AGREE ON altsize THOUGH. An altsize of zero means to
//    use size (e.g. operate as normally). An altsize > 0 means to use
//    the alternate size for any communication, but truncate the result to
//    size upon exit. An altsize of -1 means to do an initial communication
//    to get all processors to agree on a size before proceeding.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to d->communicator
//
//    Burlen Loring, Sun Sep 27 16:05:36 PDT 2015
//    Fix a memory leak.
//
// ****************************************************************************

/* ARGSUSED */
void
avtParallelContext::UnifyMinMax(double *buff, int size, int altsize)
{
#ifndef PARALLEL
    (void)buff;
    (void)size;
    (void)altsize;
#else
    double *rbuff = NULL;

    // if it hasn't been created yet, create the min/max MPI reduction operator
    if (PrivateData::AVT_MPI_MINMAX == MPI_OP_NULL)
        MPI_Op_create((MPI_User_function *)MinMaxOp, true, &PrivateData::AVT_MPI_MINMAX);

    // we do this 'extra' communication if we can't be sure all processors
    // have an agreed upon size to work with. This will have effect of
    // overwriting altsize with a maximum agreed upon size
    if (altsize == -1)
        MPI_Allreduce(&size, &altsize, 1, MPI_INT, MPI_MAX, this->GetCommunicator());

    if (altsize == 0)
    {
        if (size % 2 != 0)
        {
            debug1 << "Min/max layout must be divisible by 2." << endl;
            EXCEPTION0(ImproperUseException);
        }

        rbuff = new double[size];

        MPI_Allreduce(buff, rbuff, size, MPI_DOUBLE, PrivateData::AVT_MPI_MINMAX, this->GetCommunicator());
    }
    else if (altsize > 0)
    {
        if ((altsize % 2 != 0) || (altsize < size))
        {
            EXCEPTION0(ImproperUseException);
        }

        rbuff = new double[altsize];

        // we're going to be reducing a buffer that is larger than size
        // so populate it with appropriate default values
        double *tbuff = new double[altsize];
        for (int i = 0; i < size; ++i)
            tbuff[i] = buff[i];
        for (int i = size; i < altsize; i += 2)
        {
            tbuff[i  ] = +DBL_MAX;
            tbuff[i+1] = -DBL_MAX;
        }

        MPI_Allreduce(tbuff, rbuff, altsize, MPI_DOUBLE, PrivateData::AVT_MPI_MINMAX, this->GetCommunicator());

        delete [] tbuff;
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    // put the reduced results back into buff
    for (int i = 0; i < size ; ++i)
        buff[i] = rbuff[i];

    delete [] rbuff;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

int
avtParallelContext::UnifyMinimumValue(int mymin)
{
#ifdef PARALLEL
    int allmin;
    MPI_Allreduce(&mymin, &allmin, 1, MPI_INT, MPI_MIN, this->GetCommunicator());
    return allmin;
#else
    return mymin;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

float
avtParallelContext::UnifyMinimumValue(float mymin)
{
#ifdef PARALLEL
    float allmin;
    MPI_Allreduce(&mymin, &allmin, 1, MPI_FLOAT, MPI_MIN, this->GetCommunicator());
    return allmin;
#else
    return mymin;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMinimumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the minimum
//      value over all processors.
//
//  Arguments:
//      mymin    The minimum on this processor.
//
//  Returns:     The minimum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

double
avtParallelContext::UnifyMinimumValue(double mymin)
{
#ifdef PARALLEL
    double allmin;
    MPI_Allreduce(&mymin, &allmin, 1, MPI_DOUBLE, MPI_MIN, this->GetCommunicator());
    return allmin;
#else
    return mymin;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Hank Childs
//  Creation:    February 6, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

/* ARGSUSED */
int
avtParallelContext::UnifyMaximumValue(int mymax)
{
#ifdef PARALLEL
    int allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_INT, MPI_MAX, this->GetCommunicator());
    return allmax;
#else
    return mymax;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

float
avtParallelContext::UnifyMaximumValue(float mymax)
{
#ifdef PARALLEL
    float allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_FLOAT, MPI_MAX, this->GetCommunicator());
    return allmax;
#else
    return mymax;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      value over all processors.
//
//  Arguments:
//      mymax    The maximum on this processor.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Dave Pugmire
//  Creation:    May 19, 2008
//
// ****************************************************************************

double
avtParallelContext::UnifyMaximumValue(double mymax)
{
#ifdef PARALLEL
    double allmax;
    MPI_Allreduce(&mymax, &allmax, 1, MPI_DOUBLE, MPI_MAX, this->GetCommunicator());
    return allmax;
#else
    return mymax;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::Collect
//
//  Purpose:
//      Takes the buffer from a specific processor and take the maximum entry
//      in each buffer.
//
//  Arguments:
//      buff    The buffer.
//      size    The length of the buffer.
//
//  Returns:    Whether or not the buffer is up-to-date for this processor.  It
//              only reduces the data onto processor 0 (but this makes it
//              convenient that derived types don't have to worry about MPI).
//
//  Programmer: Hank Childs
//  Creation:   June 18, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Dec 18 07:55:21 PST 2001
//    Fix memory leak.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

/* ARGSUSED */
bool
avtParallelContext::Collect(float *buff, int size)
{
#ifndef PARALLEL
    (void)buff;
    (void)size;
    return true;
#else
    float *newbuff = new float[size];
    MPI_Reduce(buff, newbuff, size, MPI_FLOAT, MPI_MAX, 0, this->GetCommunicator());
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    if (rank == 0)
    {
        for (int i = 0 ; i < size ; i++)
        {
            buff[i] = newbuff[i];
        }
    }

    delete [] newbuff;

    return (rank == 0 ? true : false);
#endif
}

bool
avtParallelContext::Collect(double *buff, int size)
{
#ifndef PARALLEL
    (void)buff;
    (void)size;
    return true;
#else
    double *newbuff = new double[size];
    MPI_Reduce(buff, newbuff, size, MPI_DOUBLE, MPI_MAX, 0, this->GetCommunicator());
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    if (rank == 0)
    {
        for (int i = 0 ; i < size ; i++)
        {
            buff[i] = newbuff[i];
        }
    }

    delete [] newbuff;

    return (rank == 0 ? true : false);
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::Collect
//
//  Purpose:
//      Takes the buffer from a specific processor and take the maximum entry
//      in each buffer.  This variant is for ints.
//
//  Arguments:
//      buff    The buffer.
//      size    The length of the buffer.
//
//  Returns:    Whether or not the buffer is up-to-date for this processor.  It
//              only reduces the data onto processor 0 (but this makes it
//              convenient that derived types don't have to worry about MPI).
//
//  Programmer: Hank Childs
//  Creation:   June 30, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

/* ARGSUSED */
bool
avtParallelContext::Collect(int *buff, int size)
{
#ifndef PARALLEL
    (void)buff;
    (void)size;
    return true;
#else

    int *newbuff = new int[size];
    MPI_Reduce(buff, newbuff, size, MPI_INT, MPI_MAX, 0, this->GetCommunicator());
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    if (rank == 0)
    {
        for (int i = 0 ; i < size ; i++)
        {
            buff[i] = newbuff[i];
        }
    }

    delete [] newbuff;

    return (rank == 0 ? true : false);
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::Barrier
//
//  Purpose:
//      Acts as a barrier.  This is sometimes useful in determining
//      bottlenecks.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::Barrier(void)
{
#ifdef PARALLEL
    MPI_Barrier(this->GetCommunicator());
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumIntArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of integers across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      January 1, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumIntArrayAcrossAllProcessors(int *inArray, int *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_INT, MPI_SUM, this->GetCommunicator());
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumLongLongArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of long longs across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Jeremy Meredith
//  Creation:      February 22, 2008
//
//  Modifications:
//    Mark C. Miller, Tue Jan 27 18:34:41 PST 2009
//    MPI_Type_get_extent is only in MPI-2. Likewise for
//    MPI_UNSIGNED_LONG_LONG. So, I made the first check conditional
//    on MPI_UNSIGNED_LONG_LONG being defined.
//
//    Gunther H. Weber, Mon Apr  6 20:50:50 PDT 2009
//    Check whether MPI_INTEGER8 defined even if MPI_UNSIGNED_LONG_LONG
//    is defined.
//
//    Brad Whitlock, Mon Apr 20 12:06:25 PDT 2009
//    Check MPI_VERSION and MPI_SUBVERSION before using MPI_Type_get_extent.
//
// ****************************************************************************

void
avtParallelContext::SumLongLongArrayAcrossAllProcessors(VISIT_LONG_LONG *inArray,
                                    VISIT_LONG_LONG *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Datatype datatype = MPI_LONG_LONG;
    // On at least one mpi implementation (mpich2-1.0.5, Linux-x86-64),
    // MPI_LONG_LONG blatantly fails.  But for some reason INTEGER8 works.
    // Luckily we can tell this by checking the datatype size of the type.
    // We'll try a few different ones, and if none work, just do it slowly
    // using a single-precision int.
#if (MPI_VERSION >= 2) || ((MPI_VERSION == 1) && (MPI_SUBVERSION > 2))
    MPI_Aint lb,e;
    MPI_Type_get_extent(datatype, &lb, &e);
#if defined(MPI_UNSIGNED_LONG_LONG)
    if (e != sizeof(VISIT_LONG_LONG))
    {
        datatype = MPI_UNSIGNED_LONG_LONG;
        MPI_Type_get_extent(datatype, &lb, &e);
    }
#endif
#if defined(MPI_INTEGER8)  // ... may only be MPI-2.
    if (e != sizeof(VISIT_LONG_LONG))
    {
        datatype = MPI_INTEGER8;
        MPI_Type_get_extent(datatype, &lb, &e);
    }
#endif
#else
    MPI_Aint e;
    MPI_Type_extent(datatype, &e);
#if defined(MPI_UNSIGNED_LONG_LONG)
    if (e != sizeof(VISIT_LONG_LONG))
    {
        datatype = MPI_UNSIGNED_LONG_LONG;
        MPI_Type_extent(datatype, &e);
    }
#endif
#endif

    if (e == sizeof(VISIT_LONG_LONG))
    {
        MPI_Allreduce(inArray, outArray, nArray, datatype, MPI_SUM,
                      this->GetCommunicator());
    }
    else
    {
        // This is pathetic, but I don't have a better idea.
        int *tmpInArray = new int[nArray];
        int *tmpOutArray = new int[nArray];
        for (int i=0; i<nArray; i++)
            tmpInArray[i] = inArray[i];
        MPI_Allreduce(tmpInArray, tmpOutArray, nArray, MPI_INT, MPI_SUM,
                      this->GetCommunicator());
        for (int i=0; i<nArray; i++)
            outArray[i] = tmpOutArray[i];
        delete [] tmpInArray;
        delete [] tmpOutArray;
    }
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of doubles across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      August 30, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_SUM,
                  this->GetCommunicator());
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
}


// ****************************************************************************
// Function:  avtParallelContext::SumDoubleArray
//
// Purpose:
//   Sum an array across all procs, and leave result on process 0.
//
// Programmer:  Dave Pugmire
// Creation:    November 23, 2011
//
// ****************************************************************************


void
avtParallelContext::SumDoubleArray(double *inArray, double *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Reduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_SUM, 0,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(double));
#endif
}

// ****************************************************************************
// Function:  avtParallelContext::SumDoubleArrayInPlace
//
// Purpose:
//   Allow the root proc (0) to reuse its input buffer to store the sum
//   accross all procs. The input data is taken from inOutArray and then replaced
//   by the output data. This method should ONLY be called by the root proc (0), all
//   other procs should call SumDoubleArray.
//
// Arguments:
//   inOutArray The root proc input data and also where the overall sum (output data)
//               will be placed.
//   nArray     The number of entries in inOutArray
//
// Programmer:  Kevin Griffin
// Creation:    July 7, 2015
//
// ****************************************************************************


void
avtParallelContext::SumDoubleArrayInPlace(double *inOutArray, int nArray)
{
#ifndef PARALLEL
    (void)inOutArray;
    (void)nArray;
#else
    MPI_Reduce(MPI_IN_PLACE, inOutArray, nArray, MPI_DOUBLE, MPI_SUM, 0, this->GetCommunicator());
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Sums an array of floats across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The sums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      June 26, 2002
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumFloatArrayAcrossAllProcessors(float *inArray, float *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_SUM,
                  this->GetCommunicator());
#else
    for (int i = 0 ; i < nArray ; i++)
    {
        outArray[i] = inArray[i];
    }
#endif
}

// ****************************************************************************
// Function:  avtParallelContext::SumFloatArray
//
// Purpose:
//   Sum an array across all procs, and leave result on process 0.
//
// Programmer:  Dave Pugmire
// Creation:    November 23, 2011
//
// ****************************************************************************


void
avtParallelContext::SumFloatArray(float *inArray, float *outArray, int nArray)
{
#ifdef PARALLEL
    MPI_Reduce(inArray, outArray, nArray, MPI_FLOAT, MPI_SUM, 0,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(float));
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumFloatAcrossAllProcessors
//
//  Purpose:
//      Sums a single float across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumFloatAcrossAllProcessors(float &value)
{
#ifndef PARALLEL
    (void)value;
#else
    float newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_FLOAT, MPI_SUM,
                  this->GetCommunicator());
    value = newvalue;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::UnifyMinimumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the minimum for each element of an array of doubles
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The minimums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Cyrus Harrison
//  Creation:      March 27, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelContext::UnifyMinimumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,
                                           int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_MIN,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(double));
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMinimumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the minimum for each element of an array of floats
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The minimums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      February 20, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::UnifyMinimumFloatArrayAcrossAllProcessors(float *inArray, float *outArray,
                                          int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_MIN,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(float));
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumDoubleArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the maximum for each element of an array of doubles
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The maximums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Cyrus Harrison
//  Creation:      March 27, 2009
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelContext::UnifyMaximumDoubleArrayAcrossAllProcessors(double *inArray, double *outArray,
                                            int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_DOUBLE, MPI_MAX,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(double));
#endif
}



// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumFloatArrayAcrossAllProcessors
//
//  Purpose:
//      Finds the maximum for each element of an array of floats
//      across all processors.
//
//  Arguments:
//      inArray    The input.
//      outArray   The maximums of the inArrays across all processors.
//      nArray     The number of entries in inArray/outArray.
//
//  Programmer:    Hank Childs
//  Creation:      February 20, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::UnifyMaximumFloatArrayAcrossAllProcessors(float *inArray, float *outArray,
                                          int nArray)
{
#ifdef PARALLEL
    MPI_Allreduce(inArray, outArray, nArray, MPI_FLOAT, MPI_MAX,
                  this->GetCommunicator());
#else
    memcpy(outArray, inArray, nArray*sizeof(float));
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::SumIntAcrossAllProcessors
//
//  Purpose:
//      Sums a single int across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 17, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumIntAcrossAllProcessors(int &value)
{
#ifndef PARALLEL
    (void)value;
#else
    int newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_INT, MPI_SUM,
                  this->GetCommunicator());
    value = newvalue;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::SumLongAcrossAllProcessors
//
//  Purpose:
//      Sums a single int across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 17, 2003
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelContext::SumLongAcrossAllProcessors(long &value)
{
#ifndef PARALLEL
    (void)value;
#else
    long newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_LONG, MPI_SUM,
                  this->GetCommunicator());
    value = newvalue;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::SumDoubleAcrossAllProcessors
//
//  Purpose:
//      Sums a single double across all processors.
//
//  Arguments:
//      value      The input and output.
//
//  Programmer:    Jeremy Meredith
//  Creation:      April 12, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::SumDoubleAcrossAllProcessors(double &value)
{
#ifndef PARALLEL
    (void)value;
#else
    double newvalue;
    MPI_Allreduce(&value, &newvalue, 1, MPI_DOUBLE, MPI_SUM,
                  this->GetCommunicator());
    value = newvalue;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::ThisProcessorHasMinimumValue
//
//  Purpose:
//      Determines if this processor has the minimum value over all the
//      processors.
//
//  Arguments:
//      min     The minimum for this processor.
//
//  Returns:    true if it does have the minimum, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2002
//
//  Modifications:
//    Kathleen Bonnell, Mon Jan  6 11:01:09 PST 2003
//    The rcount argument of MPI_Allgather should be the number of elements
//    received from any processor, not the size of the receiving buffer.
//
//    Kathleen Bonnell, Mon Apr  7 16:20:20 PDT 2003
//    Changed theBestMin from bool to double so this method will
//    work properly.
//
//    Mark C. Miller, Thu Aug 19 15:58:19 PDT 2004
//    Fixed memory leak for allValues by re-implementing using more-efficient
//    MPI_Allreduce and the MINLOC operator. Where before the processor of
//    maximum rank with minimum value would return true, now the processor of
//    minimum rank with minimum value will return true. Otherwise, behavior
//    is identical. And, now there is no need to allocate an array of size
//    numProcs or do an expensive Allgather.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

bool
avtParallelContext::ThisProcessorHasMinimumValue(double min)
{
#ifndef PARALLEL
    (void)min;
    return true;
#else
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    struct { double val; int rank; } tmp, rtmp;
    tmp.val = min;
    tmp.rank = rank;
    MPI_Allreduce(&tmp, &rtmp, 1, MPI_DOUBLE_INT, MPI_MINLOC, this->GetCommunicator());
    return (rtmp.rank == rank ? true : false);
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::ThisProcessorHasMaximumValue
//
//  Purpose:
//      Determines if this processor has the maximum value over all the
//      processors.
//
//  Arguments:
//      max     The maximum for this processor.
//
//  Returns:    true if it does have the maximum, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 27, 2003
//
//  Modifications:
//
//    Mark C. Miller, Thu Aug 19 15:58:19 PDT 2004
//    Fixed memory leak for allValues by re-implementing using more-efficient
//    MPI_Allreduce and the MAXLOC operator. Where before the processor of
//    maximum rank with minimum value would return true, now the processor of
//    minimum rank with minimum value will return true. Otherwise, behavior
//    is identical. And, now there is no need to allocate an array of size
//    numProcs or do an expensive Allgather.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

bool
avtParallelContext::ThisProcessorHasMaximumValue(double max)
{
#ifndef PARALLEL
    (void)max;
    return true;
#else
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    struct { double val; int rank; } tmp, rtmp;
    tmp.val = max;
    tmp.rank = rank;
    MPI_Allreduce(&tmp, &rtmp, 1, MPI_DOUBLE_INT, MPI_MAXLOC, this->GetCommunicator());
    return (rtmp.rank == rank ? true : false);
#endif
}


// ****************************************************************************
//  Function:  avtParallelContext::BroadcastInt
//
//  Purpose:
//    Broadcast an integer from processor 0 to all other processors
//
//  Arguments:
//    i          the int
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************
void avtParallelContext::BroadcastInt(int &i)
{
#ifndef PARALLEL
    (void)i;
#else
    MPI_Bcast(&i, 1, MPI_INT, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastLongLong
//
//  Purpose:
//    Broadcast a long long from processor 0 to all other processors
//
//  Arguments:
//    l          the long long
//
//  Programmer:  Hank Childs
//  Creation:    December 10, 2012
//
// ****************************************************************************
void avtParallelContext::BroadcastLongLong(VISIT_LONG_LONG &l)
{
#ifndef PARALLEL
    (void)l;
#else
    MPI_Bcast(&l, 1, MPI_LONG_LONG, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
// Function: avtParallelContext::BroadcastIntArray
//
// Purpose:
//   Broadcast an array of int from processor 0 to all other processors.
//
// Arguments:
//   array  : The array to send (or receive on non-rank-0).
//   nArray : The number of values to send/receive.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 15:00:02 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void avtParallelContext::BroadcastIntArray(int *array, int nArray)
{
#ifndef PARALLEL
    (void)array;
    (void)nArray;
#else
    MPI_Bcast(array, nArray, MPI_INT, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastIntVector
//
//  Purpose:
//    Broadcast a vector<int> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Tom Fogal, Mon May 25 16:05:23 MDT 2009
//    Added a check for empty vectors.
//
// ****************************************************************************
void avtParallelContext::BroadcastIntVector(std::vector<int> &vi, int myrank)
{
#ifndef PARALLEL
    (void)vi;
    (void)myrank;
#else
    int len;
    if (myrank==0)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());
    if (myrank!=0)
        vi.resize(len);

    if(len == 0)
    {
        debug1 << "Don't know how to broadcast empty vector!  "
               << "Bailing out early." << std::endl;
        return;
    }

    MPI_Bcast(&vi[0], len, MPI_INT, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  BroadcastIntVectorFromAny
//
//  Purpose:
//    Broadcast a vector<int> from processor root to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//  Programmer:  Kevin Griffin
//  Creation:    Mon Aug 3 15:03:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void avtParallelContext::BroadcastIntVectorFromAny(std::vector<int> &vi, int myrank, int root)
{
#ifndef PARALLEL
    (void)vi;
    (void)myrank;
    (void)root;
#else
    int len;
    if (myrank==root)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, root, this->GetCommunicator());
    if (myrank!=root)
        vi.resize(len);
    
    if(len == 0)
    {
        debug1 << "Don't know how to broadcast empty vector!  "
        << "Bailing out early." << std::endl;
        return;
    }
    
    MPI_Bcast(&vi[0], len, MPI_INT, root, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastBool
//
//  Purpose: Broadcast a bool from processor 0 to all other processors
//
//  Arguments:
//    b          reference to the bool
//
//  Programmer:  Mark C. Miller
//  Creation:    June 7, 2005
//
// ****************************************************************************
void avtParallelContext::BroadcastBool(bool &b)
{
    int tmp = b ? 1 : 0;
    BroadcastInt(tmp);
    b = tmp == 1 ? true : false;
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastBoolVector
//
//  Purpose:
//    Broadcast a vector<bool> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<bool>
//    myrank     the rank of this process
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Jun 18 12:02:26 PDT 2009
//
//  Modifications:
//
// ****************************************************************************
void avtParallelContext::BroadcastBoolVector(std::vector<bool> &vb, int myrank)
{
#ifndef PARALLEL
    (void)vb;
    (void)myrank;
#else
    int len;
    if (myrank==0)
        len = vb.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());
    if (myrank!=0)
        vb.resize(len);

    if(len == 0)
    {
        debug1 << "Don't know how to broadcast empty vector!  "
               << "Bailing out early." << std::endl;
        return;
    }

    std::vector<unsigned char> v;
    v.resize(len);
    if (myrank==0)
    {
        for (size_t i = 0; i < vb.size(); ++i)
            v[i] = vb[i] ? 1 : 0;
    }
    MPI_Bcast(&v[0], len, MPI_UNSIGNED_CHAR, 0, this->GetCommunicator());
    if (myrank!=0)
    {
        for (size_t i = 0; i < vb.size(); ++i)
            vb[i] = v[i]==1;
    }
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastDouble
//
//  Purpose:
//    Broadcast an double from processor 0 to all other processors
//
//  Arguments:
//    i          the double
//
//  Programmer:  Hank Childs
//  Creation:    June 6, 2005
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************
void avtParallelContext::BroadcastDouble(double &i)
{
#ifndef PARALLEL
    (void)i;
#else
    MPI_Bcast(&i, 1, MPI_DOUBLE, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
// Function: avtParallelContext::BroadcastDoubleArray
//
// Purpose:
//   Broadcast an array of double from processor 0 to all other processors.
//
// Arguments:
//   array  : The array to send (or receive on non-rank-0).
//   nArray : The number of values to send/receive.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Fri May 20 15:00:02 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void avtParallelContext::BroadcastDoubleArray(double *array, int nArray)
{
#ifndef PARALLEL
    (void)array;
    (void)nArray;
#else
    MPI_Bcast(array, nArray, MPI_DOUBLE, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  BroadcastDoubleArrayFromAny
//
//  Purpose:
//    Broadcast a vector<double> from processor root to all other processors
//
//  Arguments:
//    array      The array to send (or receive on non-root).
//    nArray     The number of values to send/receive.
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//
//  Programmer:  Kevin Griffin
//  Creation:    Mon Aug 3 15:03:38 PDT 2015
//
//  Modifications:
//
// ****************************************************************************
void avtParallelContext::BroadcastDoubleArrayFromAny(double *array, int nArray, int root)
{
#ifndef PARALLEL
    (void)array;
    (void)nArray;
    (void)root;
#else
    MPI_Bcast(array, nArray, MPI_DOUBLE, root, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastDoubleVector
//
//  Purpose:
//    Broadcast a vector<double> from processor 0 to all other processors
//
//  Arguments:
//    vi         the vector<double>
//    myrank     the rank of this process
//
//  Programmer:  Hank Childs
//  Creation:    June 6, 2005
//
//  Modifications:
//
//    Hank Childs, Thu Jun 23 14:02:03 PDT 2005
//    Change type of "myrank" to be int.  Too much cut-n-paste previously.
//
//    Tom Fogal, Mon May 25 16:06:09 MDT 2009
//    Added check to make sure we don't try to broadcast an empty vector.
//
// ****************************************************************************
void avtParallelContext::BroadcastDoubleVector(std::vector<double> &vi, int myrank)
{
#ifndef PARALLEL
    (void)vi;
    (void)myrank;
#else
    int len;
    if (myrank==0)
        len = vi.size();

    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());
    if (myrank!=0)
        vi.resize(len);

    if(len == 0)
    {
        debug5 << "Don't know how to broadcast empty vector!  "
               << "Bailing out early." << std::endl;
        return;
    }

    MPI_Bcast(&vi[0], len, MPI_DOUBLE, 0, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  BroadcastDoubleVectorFromAny
//
//  Purpose:
//    Broadcast a vector<double> from processor root to all other processors
//
//  Arguments:
//    vi         the vector<int>
//    myrank     the rank of this process
//    root       the rank of the broadcast root
//
//  Programmer:  Kevin Griffin
//  Creation:
//
//  Modifications:
//
// ****************************************************************************
void avtParallelContext::BroadcastDoubleVectorFromAny(std::vector<double> &vi, int myrank, int root)
{
#ifndef PARALLEL
    (void)vi;
    (void)myrank;
    (void)root;
#else
    int len;
    if (myrank==root)
        len = vi.size();
    MPI_Bcast(&len, 1, MPI_INT, root, this->GetCommunicator());
    if (myrank!=root)
        vi.resize(len);
    
    if(len == 0)
    {
        debug1 << "Don't know how to broadcast empty vector!  "
        << "Bailing out early." << std::endl;
        return;
    }
    
    MPI_Bcast(&vi[0], len, MPI_DOUBLE, root, this->GetCommunicator());
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastString
//
//  Purpose:
//    Broadcast an STL string from processor 0 to all other processors
//
//  Arguments:
//    s          the string
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Brad Whitlock, Wed Aug 12 13:25:07 PDT 2015
//    Avoid use of global buffer by receiving directly into the string's buffer.
//
/// ****************************************************************************
void avtParallelContext::BroadcastString(std::string &s, int myrank)
{
#ifndef PARALLEL
    (void)s;
    (void)myrank;
#else
    int len = 0;
    if (myrank==0)
        len = s.length();
    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());

    if (myrank==0)
    {
        MPI_Bcast((void*)(s.c_str()), len, MPI_CHAR, 0, this->GetCommunicator());
    }
    else
    {
        s.resize(len);
        MPI_Bcast(&s[0], len, MPI_CHAR, 0, this->GetCommunicator());
    }
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastStringVector
//
//  Purpose:
//    Broadcast an vector<string> from processor 0 to all other processors
//
//  Arguments:
//    vs         the vector<string>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Jun  6 17:13:08 PDT 2005
//    Re-implemented to improve efficiency for vectors with lots of strings.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Tom Fogal, Mon May 25 15:53:31 MDT 2009
//    Added a check for empty string vectors.
//
//    Brad Whitlock, Wed Aug 12 13:25:07 PDT 2015
//    Removed an extra string copy of each string in the vector.
//
// ****************************************************************************
void avtParallelContext::BroadcastStringVector(std::vector<std::string> &vs, int myrank)
{
#ifndef PARALLEL
    (void)vs;
    (void)myrank;
#else
    int i;

    int len;
    if (myrank==0)
        len = static_cast<int>(vs.size());

    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());

    if(len == 0)
    {
        //debug5 << "Don't know how to broadcast empty vector!  "
        //       << "Bailing out early." << std::endl;
        vs.clear();
        return;
    }

    std::vector<int> lens(len);
    if (myrank == 0)
        for (i = 0 ; i < len ; i++)
            lens[i] = vs[i].length();
    MPI_Bcast(&(lens[0]), len, MPI_INT, 0, this->GetCommunicator());

    int total_len = 0;
    for (i = 0 ; i < len ; i++)
        total_len += lens[i];

    char *buff = new char[total_len];
    if (myrank == 0)
    {
        char *buff_ptr = buff;
        for (i = 0 ; i < len ; i++)
        {
            strncpy(buff_ptr, vs[i].c_str(), lens[i]);
            buff_ptr += lens[i];
        }
    }

    MPI_Bcast((void*)buff, total_len, MPI_CHAR, 0, this->GetCommunicator());

    if (myrank != 0)
    {
        vs.resize(len);

        // Assign strings into the string vector.
        const char *buff_ptr = buff;
        for (i=0; i<len; i++)
        {
            vs[i].assign(buff_ptr, lens[i]);
            buff_ptr += lens[i];
        }
    }
    delete [] buff;
#endif
}

// ****************************************************************************
//  Function:  avtParallelContext::BroadcastStringVectorVector
//
//  Purpose:
//    Broadcast a vector<vector<string>> from processor 0 to all
//    other processors
//
//  Arguments:
//    vvs        the vector<vector<string>>
//    myrank     the rank of this process
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Tom Fogal, Mon May 25 16:07:08 MDT 2009
//    Added check to make sure we don't try to broadcast an empty vector.
//
// ****************************************************************************
void avtParallelContext::BroadcastStringVectorVector(std::vector< std::vector<std::string> > &vvs, int myrank)
{
#ifndef PARALLEL
    (void)vvs;
    (void)myrank;
#else
    int len;
    if (myrank==0)
        len = vvs.size();
    MPI_Bcast(&len, 1, MPI_INT, 0, this->GetCommunicator());
    if (myrank!=0)
        vvs.resize(len);

    if(len == 0)
    {
        debug1 << "Don't know how to broadcast empty vector!  "
               << "Bailing out early." << std::endl;
        return;
    }

    for (int i=0; i<len; i++)
    {
        BroadcastStringVector(vvs[i], myrank);
    }
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::GetListToRootProc
//
//  Purpose:
//      Gets a variable list to processor 0.  At the beginning of this call,
//      it is not clear which processor has a good list.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
//  Modifications:
//
//    Mark C. Miller, Wed Jun  9 21:50:12 PDT 2004
//    Eliminated use of MPI_ANY_TAG and modified to use GetUniqueMessageTags
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

bool avtParallelContext::GetListToRootProc(std::vector<std::string> &vars, int total)
{
#ifndef PARALLEL
    (void)vars;
    (void)total;
    return true;
#else
    int rank;
    MPI_Comm_rank(this->GetCommunicator(), &rank);
    int red_val = 10000000;
    if (vars.size() == (size_t)total)
        red_val = rank;

    int tags[2];
    GetUniqueMessageTags(tags, 2);
    int mpiSizeTag = tags[0];
    int mpiDataTag = tags[1];

    int lowest_with_list = 0;
    MPI_Allreduce(&red_val, &lowest_with_list, 1, MPI_INT, MPI_MIN,
                  this->GetCommunicator());

    if (lowest_with_list == 0)
        return (rank == 0);

    if (lowest_with_list == rank)
    {
        for (int i = 0 ; i < total ; i++)
        {
            int size = strlen(vars[i].c_str());
            MPI_Send(&size, 1, MPI_INT, mpiSizeTag, rank, this->GetCommunicator());
            void *ptr = (void *) vars[i].c_str();
            MPI_Send(ptr, size, MPI_CHAR, mpiDataTag, rank, this->GetCommunicator());
        }
    }
    else if (rank == 0)
    {
        vars.clear();
        for (int i = 0 ; i < total ; i++)
        {
            int len;
            MPI_Status stat;
            MPI_Recv(&len, 1, MPI_INT, MPI_ANY_SOURCE, mpiSizeTag,
                     this->GetCommunicator(), &stat);
            char *varname = new char[len+1];
            void *buff = (void *) varname;
            MPI_Recv(buff, len, MPI_CHAR, stat.MPI_SOURCE, mpiDataTag,
                     this->GetCommunicator(), &stat);
            varname[len] = '\0';
            vars.push_back(varname);
            delete [] varname;
        }
    }

    return (rank == 0);
#endif
}


// ****************************************************************************
//  Function: CollectIntArraysOnRootProc
//
//  Purpose:
//      Collects a collection of arrays from all the processors on the root
//      process.  The arrays can be of different sizes.  The receiveBuf and
//      receiveCounts are allocated in this routine and must be deleted by
//      the caller.
//
//  Programmer: Eric Brugger
//  Creation:   June 22, 2009
//
//  Modifications:
//    Brad Whitlock, Fri May 20 13:54:46 PDT 2011
//    I moved the body into a static helper that I templated so I could add
//    another version that uses doubles.
//
// ****************************************************************************

template <class T>
static void
CollectArraysOnRank(int rank, int nProc, T *&receiveBuf, int *&receiveCounts,
    T *sendBuf, int sendCount,
#ifdef PARALLEL
    MPI_Datatype dataType,
    MPI_Comm communicator,
#endif
    int root)
{
#ifdef PARALLEL
    // Determine the receive counts.
    receiveCounts = NULL;
    if (rank == root)
    {
        receiveCounts = new int[nProc];
    }
    MPI_Gather(&sendCount, 1, MPI_INT, receiveCounts, 1, MPI_INT,
               root, communicator);

    // Determine the processor offsets.
    int *procOffset = NULL;
    if (rank == root)
    {
        procOffset = new int[nProc];
        procOffset[0] = 0;
        for (int i = 1; i < nProc; i++)
            procOffset[i] = procOffset[i-1] + receiveCounts[i-1];
    }

    // Allocate the receive buffer.
    receiveBuf = NULL;
    if (rank == root)
    {
        // Determine the size of the receive buffer.
        int nReceiveBuf = 0;
        for (int i  = 0 ; i < nProc; i++)
            nReceiveBuf += receiveCounts[i];

        // Allocate it.
        receiveBuf = new T[nReceiveBuf];
    }

    MPI_Gatherv(sendBuf, sendCount, dataType, receiveBuf,
                receiveCounts, procOffset, dataType, root, communicator);

    if (rank == root)
    {
        delete [] procOffset;
    }
#else
    receiveCounts = new int[1];
    receiveCounts[0] = sendCount;

    receiveBuf = new T[sendCount];
    for (int i = 0; i < sendCount; i++)
        receiveBuf[i] = sendBuf[i];
#endif
}

void
avtParallelContext::CollectIntArraysOnRank(int *&receiveBuf, int *&receiveCounts,
    int *sendBuf, int sendCount, int root)
{
    CollectArraysOnRank<int>(Rank(), Size(), receiveBuf, receiveCounts, sendBuf, sendCount,
#ifdef PARALLEL
                             MPI_INT, this->GetCommunicator(),
#endif
                             root);
}

void
avtParallelContext::CollectIntArraysOnRootProc(int *&receiveBuf, int *&receiveCounts,
    int *sendBuf, int sendCount)
{
    CollectArraysOnRank<int>(Rank(), Size(), receiveBuf, receiveCounts, sendBuf, sendCount,
#ifdef PARALLEL
                             MPI_INT, this->GetCommunicator(),
#endif
                             0);
}

void
avtParallelContext::CollectDoubleArraysOnRootProc(double *&receiveBuf, int *&receiveCounts,
    double *sendBuf, int sendCount)
{
    CollectArraysOnRank<double>(Rank(), Size(), receiveBuf, receiveCounts, sendBuf, sendCount,
#ifdef PARALLEL
                                MPI_DOUBLE, this->GetCommunicator(),
#endif
                                0);
}

// ****************************************************************************
//  Function: avtParallelContext::GetUniqueMessageTag
//
//  Purpose: Returns a suitable, unique message tag to be used in MPI_Send/Recv
//           Calls. NOTE, IF ANY PROCESSOR CALLS THIS FUNCTION FROM SOME POINT
//           IN VISIT'S EXECUTION, THEN ALL ON THE CONTEXT'S COMMUNICATOR MUST
//           CALL IT FROM THAT POINT.
//
//  Programmer: Mark C. Miller
//  Creation:   June 9, 2004
//
//  Modifications:
//    Kathleen Bonnell, Wed Sep  8 15:08:00 PDT 2004
//    Use mpiTagUpperBound instead of MPI_TAG_UB (which was being used
//    incorrectly).  Also use MIN_TAG_VALUE.
//
//    Brad Whitlock, Fri Aug 21 17:21:26 PDT 2015
//    Move code into GetUniqueMessageTags.
//
// ****************************************************************************

int avtParallelContext::GetUniqueMessageTag()
{
    int tag = 0;
    GetUniqueMessageTags(&tag, 1);
    return tag;
}

// ****************************************************************************
//  Function: avtParallelContext::GetUniqueMessageTags
//
//  Purpose: Returns suitable, unique message tags to be used in MPI_Send/Recv
//           Calls. ALL processors in the local communicator need to call this.
//
//  Note: Since we allreduce to sync the tags, calling this method is better for
//        getting multiple tags than calling GetUniqueMessageTag repeatedly.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Aug 21 17:42:31 PDT 2015
//
//  Modifications:
//
// ****************************************************************************

void
avtParallelContext::GetUniqueMessageTags(int *tags, int ntags)
{
#ifdef PARALLEL
    // With different parallel contexts in play, it is possible for processors
    // to have gotten out of sync their tags. The reason is that they could have
    // taken different code paths with some not calling GetUniqueMessageTags.
    // Get them back in sync by picking the max value among the ranks in the
    // communicator.
    int tag = avtParallelContext::PrivateData::mpiTag;
    int maxtag = tag;
    MPI_Allreduce((void*)&tag, (void*)&maxtag, 1, MPI_INT, MPI_MAX, this->GetCommunicator());
    if(maxtag != avtParallelContext::PrivateData::mpiTag)
    {
        avtParallelContext::PrivateData::mpiTag = maxtag;
        debug5 << "Updating local mpiTag to " << maxtag << endl;
    }

    // Now we're up to date, get us ntags tags.
    for(int i = 0; i < ntags; ++i)
    {
        // Do the standard wrap-around behavior.
        if (avtParallelContext::PrivateData::mpiTag == avtParallelContext::PrivateData::mpiTagUpperBound)
        {
            avtParallelContext::PrivateData::mpiTag = MIN_TAG_VALUE;
            debug5 << "Unique message tags have wrapped back to "
                   << MIN_TAG_VALUE << " from " << avtParallelContext::PrivateData::mpiTagUpperBound << endl;
        }
        else
            avtParallelContext::PrivateData::mpiTag++;

        tags[i] = avtParallelContext::PrivateData::mpiTag;
    }
#else
    for(int i = 0; i < ntags; ++i)
        tags[i] = MIN_TAG_VALUE;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::GetUniqueStaticMessageTag
//
//  Purpose: Returns a suitable, unique message tag to be used in MPI_Send/Recv
//           Calls. This is to be used for tags that must be initialized
//           before PAR_Init is called -- e.g. for static message tags.
//
//  Programmer: Kathleen Bonnell
//  Creation:   September 9, 2004
//
//  Modifications:
//
// ****************************************************************************

int avtParallelContext::GetUniqueStaticMessageTag()
{
    static int rv = 0;
#ifdef PARALLEL
    //
    //  Cannot go beyond the starting value for normal UniqueMessageTags
    //
    if (rv == MIN_TAG_VALUE -1)
    {
        rv = 0;
        debug1 << "Static Unique message tags have wrapped back to zero "
               << "from " << MIN_TAG_VALUE -1 << endl;
    }
    else
        rv++;
#endif
    return rv;
}


// ****************************************************************************
//  Function: avtParallelContext::GetAttToRootProc
//
//  Purpose:
//    Gets an attribute to processor 0.
//
//  Arguments:
//    att       The att to get to proc 0.
//    hasAtt    Inidicates whether his processor has the attribute or not.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 6, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Burlen Loring, Mon Aug  3 09:48:30 PDT 2015
//    Use MPI collectives
//
// ****************************************************************************

void
avtParallelContext::GetAttToRootProc(AttributeGroup &att, int hasAtt)
{
#ifndef PARALLEL
    (void)att;
    (void)hasAtt;
#else
    int rank = 0;
    int nprocs = 1;

    MPI_Comm_rank(this->GetCommunicator(), &rank);
    MPI_Comm_size(this->GetCommunicator(), &nprocs);

    // serialize the attribute for sending
    unsigned char *sbuf = 0;
    int size = 0;
    if (hasAtt)
    {
        BufferConnection deq;
        att.SelectAll();
        att.Write(deq);
        size = att.CalculateMessageSize(deq);
        sbuf = new unsigned char[size];
        deq.DirectRead(sbuf, size);
    }
    std::vector<int> sizes;
    if (rank == 0)
        sizes.resize(nprocs, 0);
    // gather the sizes of the sent data
    MPI_Gather(&size, 1, MPI_INT, &sizes[0], 1, MPI_INT, 0, this->GetCommunicator());

    unsigned char *rbuf = 0;
    std::vector<int> offs;
    if (rank == 0)
    {
        // compute offset of results and allocate recv bufffer
        offs.resize(nprocs, 0);
        int total = 0;
        for (int i = 0; i < nprocs; ++i)
        {
            offs[i] = total;
            total += sizes[i];
        }
        rbuf = new unsigned char [total];
    }

    // gather the attributes
    MPI_Gatherv(sbuf, size, MPI_UNSIGNED_CHAR,
        rbuf, &sizes[0], &offs[0], MPI_UNSIGNED_CHAR,
        0, this->GetCommunicator());

    if (rank == 0)
    {
        // desreialize the attributes
        BufferConnection deq;
        for (int i = 0; i < nprocs; ++i)
        {
            if (sizes[i])
            {
                deq.Append(rbuf+offs[i], sizes[i]);
                att.Read(deq);
            }
        }
    }

    delete [] sbuf;
    delete [] rbuf;
#endif
}

// ****************************************************************************
// Method: avtParallelContext::BroadcastAttributes
//
// Purpose:
//   Broadcast an attribute group from rank 0 of the group to other ranks.
//
// Arguments:
//   atts : The AttributeGroup that we want to broadcast.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 17 15:27:20 PDT 2015
//
// Modifications:
//
// ****************************************************************************

void
avtParallelContext::BroadcastAttributes(AttributeGroup &atts)
{
#ifndef PARALLEL
    (void)atts;
#else
    // Serialize the atts
    BufferConnection deq;
    unsigned char *sbuf = 0;
    int size = 0;
    if(Rank() == 0)
    {
        // Serialize the attributes to a buffer connection.
        atts.SelectAll();
        atts.Write(deq);
        size = atts.CalculateMessageSize(deq);
        sbuf = new unsigned char[size];
        deq.DirectRead(sbuf, size);

        // Send the serialized contents to other ranks.
        MPI_Bcast(&size, 1, MPI_INT, 0, this->GetCommunicator());
        MPI_Bcast(sbuf, size, MPI_UNSIGNED_CHAR, 0, this->GetCommunicator());
    }
    else
    {
        MPI_Bcast(&size, 1, MPI_INT, 0, this->GetCommunicator());
        sbuf = new unsigned char[size];
        MPI_Bcast(sbuf, size, MPI_UNSIGNED_CHAR, 0, this->GetCommunicator());

        deq.Append(sbuf, size);
        atts.Read(deq);
    }

    delete [] sbuf;
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::GetFloatArrayToRootProc
//
//  Purpose:
//    Gets a float array to processor 0.
//
//  Arguments:
//    fa        The  float array.
//    nf        The number of items in the array.
//    success   Inidicates whether or not this processor has the float array.
//
//  Programmer: Kathleen Bonnell
//  Creation:   July 6, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

void
avtParallelContext::GetFloatArrayToRootProc(float *fa, int nf, bool &success)
{
#ifndef PARALLEL
    (void)fa;
    (void)nf;
    success = true;
#else
    int myRank, numProcs;
    MPI_Comm_rank(this->GetCommunicator(), &myRank);
    MPI_Comm_size(this->GetCommunicator(), &numProcs);
    int tags[2];
    GetUniqueMessageTags(tags, 2);
    int mpiGoodTag       = tags[0];
    int mpiFloatArrayTag = tags[1];

    if (myRank == 0)
    {
        MPI_Status stat, stat2;
        int good;
        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&good, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiGoodTag, this->GetCommunicator(), &stat);
            if (good)
            {
                MPI_Recv(fa, nf, MPI_FLOAT, stat.MPI_SOURCE, mpiFloatArrayTag,
                         this->GetCommunicator(), &stat2);
                success = good;
            }
        }
    }
    else
    {
        int good = success ? 1 : 0;
        MPI_Send(&good, 1, MPI_INT, 0, mpiGoodTag, this->GetCommunicator());
        if (success)
        {
            MPI_Send(fa, nf, MPI_FLOAT, 0, mpiFloatArrayTag, this->GetCommunicator());
        }
    }
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::UnifyMaximumValue
//
//  Purpose:
//      Makes a collective call across all processors to unify the maximum
//      values in the intVector over all processors.
//
//  Arguments:
//      mymax    The maximum values on this processor.
//      results  The maximum values over all processors.
//
//  Returns:     The maximum over all processors.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    November 9, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
// ****************************************************************************

/* ARGSUSED */
void
avtParallelContext::UnifyMaximumValue(std::vector<int> &mymax, std::vector<int> &results)
{
#ifdef PARALLEL
    results.resize(mymax.size());
    MPI_Allreduce(&mymax[0], &results[0], mymax.size(), MPI_INT, MPI_MAX,
                  this->GetCommunicator());
#else
    results = mymax;
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::GetDoubleArrayToRootProc
//
//  Purpose:
//    Gets a double array to processor 0.
//
//  Arguments:
//    da        The double array.
//    nd        The number of items in the array.
//    success   Inidicates whether or not this processor has the double array.
//
//  Programmer: Kathleen Bonnell
//  Creation:   March 13, 2006
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to this->GetCommunicator()
//
//    Hank Childs, Tue Sep  4 17:00:21 PDT 2007
//    Cast a Boolean to an int, to make sure that a cast to a pointer was
//    correctly word aligned.
//
// ****************************************************************************

void
avtParallelContext::GetDoubleArrayToRootProc(double *da, int nd, bool &success)
{
#ifndef PARALLEL
    (void)da;
    (void)nd;
    success = true;
#else
    int myRank, numProcs;
    MPI_Comm_rank(this->GetCommunicator(), &myRank);
    MPI_Comm_size(this->GetCommunicator(), &numProcs);
    int tags[2];
    GetUniqueMessageTags(tags, 2);
    int mpiGoodTag        = tags[0];
    int mpiDoubleArrayTag = tags[1];

    if (myRank == 0)
    {
        MPI_Status stat, stat2;
        int good;
        for (int i = 1; i < numProcs; i++)
        {
            MPI_Recv(&good, 1, MPI_INT, MPI_ANY_SOURCE,
                     mpiGoodTag, this->GetCommunicator(), &stat);
            if (good)
            {
                MPI_Recv(da, nd, MPI_DOUBLE, stat.MPI_SOURCE, mpiDoubleArrayTag,
                         this->GetCommunicator(), &stat2);
                success = good;
            }
        }
    }
    else
    {
        int val = (int) success;
        MPI_Send(&val, 1, MPI_INT, 0, mpiGoodTag, this->GetCommunicator());
        if (success)
        {
            MPI_Send(da, nd, MPI_DOUBLE, 0, mpiDoubleArrayTag, this->GetCommunicator());
        }
    }
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::WaitAll
//
//  Purpose:
//    Wait on all requests in a vector.
//
//  Arguments:
//    req       Vector of requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
// ****************************************************************************

void
avtParallelContext::WaitAll(std::vector<int> &reqs, std::vector<int> &status )
{
#ifndef PARALLEL
    (void)reqs;
    (void)status;
#else
    status.resize( reqs.size() );
    MPI_Waitall( reqs.size(), (MPI_Request *)&reqs[0], (MPI_Status *)&status[0] );
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::WaitSome
//
//  Purpose:
//    Wait on a some requests in a vector.
//
//  Arguments:
//    req       Vector of requests.
//    done      Vector of completed requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
// ****************************************************************************

void
avtParallelContext::WaitSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status )
{
#ifndef PARALLEL
    (void)reqs;
    (void)done;
    (void)status;
#else
    status.resize( reqs.size() );
    done.resize( reqs.size() );
    int nDone;
    MPI_Waitsome( reqs.size(), (MPI_Request *)&reqs[0], &nDone, (int *)&done[0], (MPI_Status *)&status[0] );
    done.resize( nDone );
#endif
}


// ****************************************************************************
//  Function: avtParallelContext::TestSome
//
//  Purpose:
//    Test a vector of requests.
//
//  Arguments:
//    req       Vector of requests.
//    done      Vector of completed requests.
//    status    Vector of returned status.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
//
// ****************************************************************************

void
avtParallelContext::TestSome(std::vector<int> &reqs, std::vector<int> &done, std::vector<int> &status )
{
#ifndef PARALLEL
    (void)reqs;
    (void)done;
    (void)status;
#else
    status.resize( reqs.size() );
    done.resize( reqs.size() );
    int nDone;
    MPI_Testsome( reqs.size(), (MPI_Request *)&reqs[0], &nDone, (int *)&done[0], (MPI_Status *)&status[0] );
#endif
}

// ****************************************************************************
//  Function: avtParallelContext::CancelRequest
//
//  Purpose:
//    Cancel a request.
//
//  Arguments:
//    req       The request.
//
//  Programmer: Dave Pugmire
//  Creation:   June 18, 2008
//
//  Modifications:
//
//    Tom Fogal, Wed Aug  6 11:36:21 EDT 2008
//    Changed the argument type and added a cast to avoid a type mismatch.
//
// ****************************************************************************

void
avtParallelContext::CancelRequest(void *req)
{
#ifndef PARALLEL
    (void)req;
#else
    MPI_Cancel(static_cast<MPI_Request*>(req));
#endif
}
