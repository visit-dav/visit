#ifndef REMOTE_PROXY_BASE_H    
#define REMOTE_PROXY_BASE_H
#include <proxybase_exports.h>
#include <Xfer.h>
#include <KeepAliveRPC.h>
#include <QuitRPC.h>
#include <ConnectCallback.h>
#include <HostProfile.h>
#include <string>

// Forward declaration.
class RemoteProcess;

// ****************************************************************************
// Class: RemoteProxyBase
//
// Purpose:
//   This class serves as a base class for our remote process proxies.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:12:55 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Jun 17 11:55:51 PDT 2003
//    Added parallel functions that were only needed for the engine but
//    that are helpful to have in the base class.
//
//    Jeremy Meredith, Thu Oct  9 14:04:37 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Brad Whitlock, Thu Mar 11 12:44:23 PDT 2004
//    I added KeepAliveRPC so we don't lose connections to remote components
//    whose connections have been idle for a long time.
//
// ****************************************************************************

class PROXYBASE_API RemoteProxyBase
{
public:
    RemoteProxyBase(const std::string &componentName);
    virtual ~RemoteProxyBase();

    void SetRemoteUserName(const std::string &rName);
    void SetProgressCallback(bool (*cb)(void *, int), void *data);
    void AddArgument(const std::string &arg);

    void Create(const std::string &hostName,
                HostProfile::ClientHostDetermination chd,
                const std::string &clientHostName,
                bool manualSSHPort,
                int sshPort,
                ConnectCallback *connectCallback = 0,
                void *data = 0, bool createAsThoughLocal = false);
    void Close();
    virtual void SendKeepAlive();

    virtual bool Parallel() const;
    virtual std::string GetComponentName() const = 0;

    virtual void SetNumProcessors(int)  { }
    virtual void SetNumNodes(int)       { }
    virtual void SetLoadBalancing(int)  { }

protected:
    virtual void         SetupComponentRPCs() = 0;
    std::string          GetVisItString() const;
    virtual void         AddExtraArguments();

    std::string          componentName;
    RemoteProcess       *component;
    Xfer                 xfer;
    QuitRPC              quitRPC;
    KeepAliveRPC         keepAliveRPC;

    std::string          remoteUserName;
    int                  nWrite;
    int                  nRead;

    // Extra command line arguments to pass to the remote process.
    stringVector         argv;

    bool               (*progressCallback)(void *, int);
    void                *progressCallbackData;
};

#endif
