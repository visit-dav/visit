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

#ifndef MPI_XFER_H
#define MPI_XFER_H
#ifdef PARALLEL
#include <mpi.h>
#endif
#include <engine_parstate_exports.h>
#include <Xfer.h>

// ****************************************************************************
// Class: MPIXfer
//
// Purpose:
//   Observes multiple AttributeSubject objects. This class is
//   responsible for writing AttributeSubject objects onto a Connection.
//   The difference between this and Xfer is that before an object is
//   told to read its connection in the Process method, the message is
//   broadcast to other processes on the MPI world communicator.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 13 10:15:20 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Thu Sep 21 22:19:48 PDT 2000
//    Added override for Xfer::Update.
//
//    Brad Whitlock, Fri Mar 16 10:52:06 PDT 2001
//    Added override of Xfer::ReadHeader.
//
//    Jeremy Meredith, Fri Sep 21 14:43:21 PDT 2001
//    Added override of SendInterruption.
//
//    Mark C. Miller, Thu Jun 10 09:08:18 PDT 2004
//    Added arg for mpiInterruptTag to SendInterruption
//
//    Jeremy Meredith, Thu Oct  7 14:09:10 PDT 2004
//    Added callback so the master process could tell the slaves they
//    are about to receive data.  This was needed for running inside a
//    parallel simulation because slave processes need some way to know
//    that the next command coming is visit-specific.
//
//    Mark C. Miller, Tue Feb 13 16:24:58 PST 2007
//    Added VisIt_MPI_Bcast method
// ****************************************************************************

class ENGINE_PARSTATE_API MPIXfer : public Xfer
{
public:
    MPIXfer();
    virtual ~MPIXfer();
    virtual void Process();
    virtual void Update(Subject*);
    virtual void SendInterruption(int mpiInterruptTag);

    void SetEnableReadHeader(bool val);
    static void SetSlaveProcessInstructionCallback(void (*)());

    static int VisIt_MPI_Bcast(void *buf, int count, MPI_Datatype datatype,
                                int root, MPI_Comm comm);
protected:
    bool ReadHeader();
private:
    bool enableReadHeader;
    int  readsSinceReadHeaderDisabled;
    static void (*slaveProcessInstruction)();
};

#endif
