// ************************************************************************* //
//                           MPIResourceManager.h                            //
// ************************************************************************* //

#ifndef MPI_RESOURCE_MANAGER_H
#define MPI_RESOURCE_MANAGER_H

#include <mpi.h>


// ****************************************************************************
//  Class: MPIResourceManager
//
//  Purpose:
//      A class that will manage control of a group of resources, provided that
//      the processors that are concerned with those resources are split
//      into a partition.  The resource manager actually creates the partition.
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

class MPIResourceManager
{
  public:
                MPIResourceManager(int, int);
               ~MPIResourceManager();

    void        ForceControlToRoot(void);
    void        ObtainResource(int);
    void        RelinquishResource(int);

    MPI_Comm   &GetCommunicator(void) { return groupComm; };
    int         GetGroupNum(void)     { return groupNum;  };
    int         GetGroupRank(void)    { return groupRank; };

  private:
    MPI_Comm    groupComm;
    int         groupRank;
    int         groupSize;

    bool       *haveResource;
    int         numResources;

    bool        noContentionForResources;

    int         groupNum;
    int         numCommunicators;

    int         Next(void);
    int         Previous(void);

    // Constants
    static int const   MPI_TAG;
};


#endif


