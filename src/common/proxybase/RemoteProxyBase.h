// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef REMOTE_PROXY_BASE_H    
#define REMOTE_PROXY_BASE_H
#include <proxybase_exports.h>
#include <Xfer.h>
#include <KeepAliveRPC.h>
#include <QuitRPC.h>
#include <ConnectCallback.h>
#include <LaunchProfile.h>
#include <MachineProfile.h>
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
//    Brad Whitlock, Thu Aug 5 09:59:11 PDT 2004
//    Added AddProfileArguments from another class.
//
//    Jeremy Meredith, Thu May 24 10:20:32 EDT 2007
//    Added SSH tunneling argument to Create.
//
//    Brad Whitlock, Wed Nov 21 11:41:52 PST 2007
//    Added methods to access the RemoteProcess's connections.
//
//    Brad Whitlock, Fri Dec  7 16:58:33 PST 2007
//    Added SetupAllRPCs and GetXfer.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.
//
//    Eric Brugger, Mon May  2 16:41:53 PDT 2011
//    I added the ability to use a gateway machine when connecting to a
//    remote host.
//
//    Brad Whitlock, Tue Jun  5 17:30:06 PDT 2012
//    Change Create method so it takes a MachineProfile instead of a bunch
//    of separate options.
//
// ****************************************************************************

class PROXYBASE_API RemoteProxyBase
{
public:
    RemoteProxyBase(const std::string &componentName);
    virtual ~RemoteProxyBase();

    void SetProgressCallback(bool (*cb)(void *, int), void *data);
    void AddArgument(const std::string &arg);
    void AddProfileArguments(const MachineProfile &machine,
                             bool addParallelArgs);

    virtual void Create(const MachineProfile &profile, 
                        ConnectCallback *connectCallback = 0, 
                        void *connectCallbackData = 0,
                        bool createAsThoughLocal = false);
    void Close();
    virtual void SendKeepAlive();

    virtual bool Parallel() const;
    virtual std::string GetComponentName() const = 0;

    virtual void SetNumProcessors(int)  { }
    virtual void SetNumNodes(int)       { }
    virtual void SetLoadBalancing(int)  { }

    Connection *GetReadConnection(int i=0) const;
    Connection *GetWriteConnection(int i=0) const;

    // This method should rarely need to be used.
    const Xfer &GetXfer();
protected:
    void                 SetupAllRPCs();
    virtual void         SetupComponentRPCs() = 0;
    std::string          GetVisItString(const MachineProfile &profile) const;
    virtual void         AddExtraArguments();

    std::string          componentName;
    RemoteProcess       *component;
    Xfer                 xfer;
    bool                 rpcSetup;
    QuitRPC              quitRPC;
    KeepAliveRPC         keepAliveRPC;

    int                  nWrite;
    int                  nRead;

    // Extra command line arguments to pass to the remote process.
    stringVector         argv;

    bool               (*progressCallback)(void *, int);
    void                *progressCallbackData;
};

#endif
