// ************************************************************************* //
//                           MPIResourceManager.C                            //
// ************************************************************************* //

#include <fcntl.h>
#include <iostream.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <MPIResourceManager.h>


// 
// Cannot initialize static constants in class definition, so do that here.
// Hope my birth year provides an unique MPI tag.
//

int const   MPIResourceManager::MPI_TAG         = 1976;  


// ****************************************************************************
//  Method: MPIResourceManager constructor
//
//  Purpose:
//      Creates all of the communicators and posts a receive to be prepared 
//      the resource is passed to this processor.
//
//  Arguments:
//      numPartitions   The number of partitions each resource should be
//                      split into.
//      nr              The number of resources.
//
//  Note:       MPI_Comm_create is a collective operation, so MPI_COMM_WORLD
//              must be used as the parenting communicator.
//
//  Programmer: Hank Childs
//  Creation:   December 16, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 13:20:45 PDT 2000
//      Changed class so that there should be one instance of a manager instead
//      of many in a program and that one manager can handle all of the
//      resources.
//
// ****************************************************************************

MPIResourceManager::MPIResourceManager(int numPartitions, int nr)
{
    numResources = nr;
    haveResource = new bool[numResources];

    //
    // Determine how many communicators there will be.
    //
    int    size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int    myRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

    numCommunicators = numPartitions;
    if (numCommunicators <= 0 || numCommunicators >= size)
    {
        //
        // There are more resources than processors.  Since the resources 
        // will not be shared among the processors in this case, just return.
        // 
        noContentionForResources = true;
        return;
    }
    else
    {
        noContentionForResources = false;
    }
  
    //
    // Make the partitions and the communicators.
    //
    int   *procIDs    = new int[(size/numCommunicators) + 1];
    int    lastProc   = 0;
    int    extraProcs = size % numCommunicators;

    //
    // Get the underlying group to MPI_COMM_WORLD.
    //
    MPI_Group  worldGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &worldGroup);
 
    for (int i = 0 ; i < numCommunicators ; i++)
    {
        bool   myGroup = false;

        //
        // Determine how many processors should be with this communicator.
        //
        int  numProcsForThisGroup = size/numCommunicators;
        if (extraProcs > 0)
        {
            numProcsForThisGroup++;
            extraProcs--;
        }
       
        //
        // Create the list of processors.
        //
        for (int j = 0 ; j < numProcsForThisGroup ; j++)
        {
            if (lastProc == myRank)
            {
                myGroup   = true;
                groupNum  = i;
                groupSize = numProcsForThisGroup;
            }
            procIDs[j] = lastProc;
            lastProc++;
        }

        //
        // Create a new group.
        //
        MPI_Group   newGroup;
        MPI_Group_incl(worldGroup, numProcsForThisGroup, procIDs, &newGroup);

        //
        // Create the new communicator.  This is collective.  Also, put
        // that communicator into our data member if this is our processor's
        // group.
        //
        if (myGroup == true)
        {
            MPI_Comm_create(MPI_COMM_WORLD, newGroup, &groupComm);
        }
        else
        {
            MPI_Comm   newComm;
            MPI_Comm_create(MPI_COMM_WORLD, newGroup, &newComm);
        }
    }

    //
    // Determine rank in selected partition.
    //
    MPI_Comm_rank(groupComm, &groupRank);

    //
    // The first processor in the group starts with the resources.
    //
    for (int k = 0 ; k < numResources ; k++)
    {
        haveResource[k] = (groupRank == 0 ? true : false);
    }
}


// ****************************************************************************
//  Method: MPIResourceManager destructor
//
//  Programmer: Hank Childs
//  Creation:   December 16, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 13:20:45 PDT 2000
//      Freed haveResource array.
//
// ****************************************************************************

MPIResourceManager::~MPIResourceManager()
{
    //
    // MPI_Finalize takes care of deleting all of the groups and communicators.
    // Doing that again just causes an error.
    //

    if (haveResource != NULL)
    {
        delete [] haveResource;
    }
}


// ****************************************************************************
//  Method: MPIResourceManager::ObtainResource
//
//  Purpose:
//      Tries to receive a message indicating that it can have the resource.
//
//  Arguments:
//      resource   The number of the resource to obtain.
//
//  Note:       This routine may block for a long time in trying to obtain
//              the resource.
//
//  Programmer: Hank Childs
//  Creation:   December 16, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 13:20:45 PDT 2000
//      Added argument so object could work on multiple resources.
//
// ****************************************************************************

void
MPIResourceManager::ObtainResource(int resource)
{
    if (noContentionForResources)
    {
        return;
    }

    //
    // Check to make sure a valid resource was specified.
    //
    if (resource >= numResources || resource < 0)
    {
        return;
    }

    if (haveResource[resource])
    {
        //
        // The resource has already been obtained.
        //
        return;
    }

    while (haveResource[resource] == false)
    {
        MPI_Status   status;
        int  mpiBuffer[2];     // The manager passes messages of size 2.
        MPI_Recv(mpiBuffer, 2, MPI_INT, Previous(), MPI_TAG, groupComm, 
                 &status);
        haveResource[mpiBuffer[0]] = true;
    }
}


// ****************************************************************************
//  Method: MPIResourceManager::RelinquishResource
//
//  Purpose:
//      Send a message to the next processor indicating that it can have the
//      resource.
//
//  Arguments:
//      resource   The number of the resource to relinquish.
//
//  Programmer: Hank Childs
//  Creation:   December 16, 1999
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 13:20:45 PDT 2000
//      Added argument so object could work on multiple resources.
//
// ****************************************************************************

void
MPIResourceManager::RelinquishResource(int resource)
{
    if (noContentionForResources)
    {
        return;
    }

    if (!haveResource[resource])
    {
        //
        // Don't have the resource to relinquish.
        //
        return;
    }

    int  mpiBuffer[2];
    mpiBuffer[0] = resource;
    mpiBuffer[1] = 0;
    MPI_Send(mpiBuffer, 2, MPI_INT, Next(), MPI_TAG, groupComm);
    haveResource[resource] = false;
}


// ****************************************************************************
//  Method: MPIResourceManager::ForceControlToRoot
//
//  Purpose:
//      Give control back to the root processor.  This must be done by every
//      processor.
//
//  Programmer: Hank Childs
//  Creation:   February 18, 2000
//
//  Modifications:
//
//      Hank Childs, Tue Jun 13 17:33:04 PDT 2000
//      Re-wrote function to prevent deadlock.
//
// ****************************************************************************

void
MPIResourceManager::ForceControlToRoot(void)
{
    bool  someProcessorsAreStillWorking = true;
    MPI_Status   status;
    int  mpiBuffer[2];

    //
    // If the resources were never passed around, then this method could lead
    // to everyone waiting for a message that will never come.  Prevent that
    // by passing a message if this processor has the first resource.
    //
    if (haveResource[0])
    {
        mpiBuffer[0] = 0;
        mpiBuffer[1] = 1;
        MPI_Send(mpiBuffer, 2, MPI_INT, Next(), MPI_TAG, groupComm);
    }
        
    while (someProcessorsAreStillWorking)
    {
        MPI_Recv(mpiBuffer, 2, MPI_INT, Previous(), MPI_TAG, groupComm, 
                 &status);

        if (mpiBuffer[0] == -1)
        {
            //
            // One of the processors have declared that all the processors are
            // done.  Send the message on in the ring.
            //
            someProcessorsAreStillWorking = false;
            MPI_Send(mpiBuffer, 2, MPI_INT, Next(), MPI_TAG, groupComm);
        }
        else if (mpiBuffer[1] >= groupSize && mpiBuffer[0] == 0)
        {
            //
            // This is the first processor to realize that all the processors
            // are done.
            //
            someProcessorsAreStillWorking = false;
            mpiBuffer[0] = -1;
            MPI_Send(mpiBuffer, 2, MPI_INT, Next(), MPI_TAG, groupComm);

            //
            // Clean up the message from the last processor to realize that
            // all processors are done.
            //
            MPI_Recv(mpiBuffer, 2, MPI_INT, Previous(), MPI_TAG, groupComm, 
                     &status);
        }
        else
        {
            //
            // No one has declared that all of the processors are done yet.
            // Indicate that we are done by incrementing the second element.
            //
            mpiBuffer[1]++;
            MPI_Send(mpiBuffer, 2, MPI_INT, Next(), MPI_TAG, groupComm);
        }
    }

    //
    // All processors are now going to agree that the root node has control
    // of the resource.
    //
    for (int i = 0 ; i < numResources ; i++)
    {
        haveResource[i] = (groupRank == 0 ? true : false);
    }
}


// ****************************************************************************
//  Method: MPIResourceManager::Next
//
//  Purpose:
//      Determines the next processor in the ring for its communication group.
//
//  Returns:    The next processor in the ring.
//
//  Programmer: Hank Childs
//  Creation:   June 14, 2000
//
// ****************************************************************************

int
MPIResourceManager::Next(void)
{
    return (groupRank + 1) % groupSize;
}


// ****************************************************************************
//  Method: MPIResourceManager::Previous
//
//  Purpose:
//      Determines the previous processor in the ring for its communication 
//      group.
//
//  Returns:    The previous processor in the ring.
//
//  Programmer: Hank Childs
//  Creation:   June 14, 2000
//
// ****************************************************************************

int
MPIResourceManager::Previous(void)
{
    int          prev = (groupRank - 1) % groupSize;

    //
    // Quoth K&R, pg. 41, "the sign of the result for % are machine-dependent
    // for negative operands."
    //
    prev += groupSize;
    prev %= groupSize;

    return prev;
}


