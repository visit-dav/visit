// ************************************************************************* //
//                         avtSamplePointCommunicator.h                      //
// ************************************************************************* //

#ifndef AVT_SAMPLE_POINT_COMMUNICATOR_H
#define AVT_SAMPLE_POINT_COMMUNICATOR_H

#include <filters_exports.h>

#include <avtSamplePointsToSamplePointsFilter.h>


class     avtImagePartition;
class     avtRay;
class     avtVolume;


// ****************************************************************************
//  Class: avtSamplePointCommunicator
//
//  Purpose:
//      Communicates samples across processors.  This is only intended to be
//      used when run in parallel.
//   
//  Programmer: Hank Childs
//  Creation:   January 24, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Feb  4 10:16:13 PST 2001
//    Pushed functionality into rays, cells.
//
//    Hank Childs, Sun Mar  4 19:55:26 PST 2001
//    Added code to dynamically assign the image partitions to minimize
//    communication.
//
//    Hank Childs, Tue Jan  1 13:04:10 PST 2002
//    Account for non-uniform image partitions.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class AVTFILTERS_API avtSamplePointCommunicator 
    : public avtSamplePointsToSamplePointsFilter
{
  public:
                        avtSamplePointCommunicator();
    virtual            ~avtSamplePointCommunicator();

    const char         *GetType(void) { return "avtSamplePointCommunicator"; };
    const char         *GetDescription(void) {return "Communicating samples";};

    void                SetImagePartition(avtImagePartition *);

  protected:
    int                 numProcs;
    int                 myRank;
    avtImagePartition  *imagePartition;

    void                Execute(void);
    char               *CommunicateMessages(char **, int *, char **, int*);

    void                EstablishImagePartitionBoundaries(void);
    void                DetermineImagePartitionAssignments(const int *,
                                                           const int *);
    char               *MutateMessagesByAssignment(char **, int *, char *&,
                                                   int &);
};


#endif


