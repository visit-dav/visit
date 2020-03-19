// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef EXISTING_REMOTE_PROCESS_H
#define EXISTING_REMOTE_PROCESS_H
#include <comm_exports.h>
#include <RemoteProcess.h>
#include <ConnectCallback.h>

// ****************************************************************************
// Class: ExistingRemoteProcess
//
// Purpose:
//   Connects to a remote process that is already running.
//
// Notes:      The user-supplied ConnectCallback function is used to
//             initiate the connection with the remote process.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 13:09:38 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon May 5 17:24:01 PST 2003
//   I added the createAsThoughLocal argument to the Open method to force
//   the argument list used to create the process to be the local form
//   even though the process may be launched on a remote machine.
//
//   Jeremy Meredith, Thu Oct  9 14:04:08 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Jeremy Meredith, Thu May 24 11:10:15 EDT 2007
//   Added SSH tunneling argument to Open.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
//
//   Eric Brugger, Mon May  2 16:44:20 PDT 2011
//   I added the ability to use a gateway machine when connecting to a
//   remote host.
//
//   Brad Whitlock, Tue Jun  5 17:24:03 PDT 2012
//   Pass in MachineProfile to Open.
//
// ****************************************************************************

class COMM_API ExistingRemoteProcess : public RemoteProcess
{
public:
    ExistingRemoteProcess(const std::string &rProgram);
    virtual ~ExistingRemoteProcess();
    virtual bool Open(const MachineProfile &profile,
                      int numRead, int numWrite,
                      bool createAsThoughLocal = false);
    void SetConnectCallback(ConnectCallback *cb);
    void SetConnectCallbackData(void *data);
private:
    ConnectCallback *connectCallback;
    void            *connectCallbackData;
};

#endif
