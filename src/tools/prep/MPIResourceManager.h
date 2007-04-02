/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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


