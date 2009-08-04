/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
//    Hank Childs, Tue Jan 13 14:27:02 PST 2009
//    Add support for jittering.
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
    void                SetJittering(bool j) { jittering = j; };

  protected:
    int                 numProcs;
    int                 myRank;
    avtImagePartition  *imagePartition;
    bool                jittering;

    void                Execute(void);
    char               *CommunicateMessages(char **, int *, char **, int*);

    void                EstablishImagePartitionBoundaries(void);
    void                DetermineImagePartitionAssignments(const int *,
                                                           const int *);
    char               *MutateMessagesByAssignment(char **, int *, char *&,
                                                   int &);
};


#endif


