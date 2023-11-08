// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MPI_XFER_H
#define MPI_XFER_H
#include <engine_main_exports.h>
#ifdef PARALLEL
#include <mpi.h>
#endif
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
//    Added callback so the manager process could tell the workers they
//    are about to receive data.  This was needed for running inside a
//    parallel simulation because worker processes need some way to know
//    that the next command coming is visit-specific.
//
//    Mark C. Miller, Tue Feb 13 16:24:58 PST 2007
//    Added VisIt_MPI_Bcast method
//
//    Brad Whitlock, Wed Nov  4 12:04:42 PST 2009
//    I changed the API to ENGINE_MAIN_API.
//
// ****************************************************************************

class ENGINE_MAIN_API MPIXfer : public Xfer
{
public:
    MPIXfer();
    virtual ~MPIXfer();
    virtual void Process();
    virtual void Update(Subject*);
    virtual void SendInterruption(int mpiInterruptTag);

    void SetEnableReadHeader(bool val);
    static void SetWorkerProcessInstructionCallback(void (*)());

    static int VisIt_MPI_Bcast(void *buf, int count, MPI_Datatype datatype,
                                int root, MPI_Comm comm);
    static void SetUIBcastThresholds(int nssleep, int spinsecs)
        { nanoSecsOfSleeps = nssleep;
          if (spinsecs >= 0) secsOfSpinBeforeSleeps = spinsecs; };

protected:
    bool ReadHeader();
private:
    bool enableReadHeader;
    int  readsSinceReadHeaderDisabled;
    static void (*workerProcessInstruction)();
    static int nanoSecsOfSleeps;
    static int secsOfSpinBeforeSleeps;
};

#endif
