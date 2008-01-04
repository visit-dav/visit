/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
class HostProfile;
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
// ****************************************************************************

class PROXYBASE_API RemoteProxyBase
{
public:
    RemoteProxyBase(const std::string &componentName);
    virtual ~RemoteProxyBase();

    void SetRemoteUserName(const std::string &rName);
    void SetProgressCallback(bool (*cb)(void *, int), void *data);
    void AddArgument(const std::string &arg);
    void AddProfileArguments(const HostProfile &profile, bool addParallelArgs);

    void Create(const std::string &hostName,
                HostProfile::ClientHostDetermination chd,
                const std::string &clientHostName,
                bool manualSSHPort,
                int sshPort,
                bool useTunneling,
                ConnectCallback *connectCallback = 0,
                void *data = 0, bool createAsThoughLocal = false);
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
    std::string          GetVisItString() const;
    virtual void         AddExtraArguments();

    std::string          componentName;
    RemoteProcess       *component;
    Xfer                 xfer;
    bool                 rpcSetup;
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
