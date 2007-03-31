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
// ****************************************************************************

class COMM_API ExistingRemoteProcess : public RemoteProcess
{
public:
    ExistingRemoteProcess(const std::string &rProgram);
    virtual ~ExistingRemoteProcess();
    virtual bool Open(const std::string &rHost,
                      HostProfile::ClientHostDetermination chd,
                      const std::string &clientHostName,
                      bool manualSSHPort,
                      int sshPort,
                      int numRead, int numWrite,
                      bool createAsThoughLocal = false);
    void SetConnectCallback(ConnectCallback *cb);
    void SetConnectCallbackData(void *data);
private:
    ConnectCallback *connectCallback;
    void            *connectCallbackData;
};

#endif
