/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#include <RemoteProxyBase.h>
#include <RemoteProcess.h>
#include <ExistingRemoteProcess.h>
#include <HostProfile.h>
#include <snprintf.h>

// ****************************************************************************
// Method: RemoteProxyBase::RemoteProxyBase
//
// Purpose: 
//   Constructor for the RemoteProxyBase class.
//
// Arguments:
//   compName : The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:56:19 PST 2003
//
// Modifications:
//  Brad Whitlock, Thu Mar 11 12:47:34 PDT 2004
//  Added KeepAliveRPC.
//
// ****************************************************************************

RemoteProxyBase::RemoteProxyBase(const std::string &compName) :
    componentName(compName), xfer(), quitRPC(), keepAliveRPC(),
    remoteUserName(), argv()
{
    component = 0;
    nWrite = nRead = 1;
    progressCallback = 0;
    progressCallbackData = 0;
}

// ****************************************************************************
// Method: RemoteProxyBase::~RemoteProxyBase
//
// Purpose: 
//   Destructor for the RemoteProxyBase class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:56:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

RemoteProxyBase::~RemoteProxyBase()
{
    delete component;
}

// ****************************************************************************
// Method: RemoteProxyBase::Create
//
// Purpose: 
//   Creates the remote process and hooks up RPCs so that it can be used.
//
// Arguments:
//   hostName        : The host where the proxy should be created.
//   connectCallback : A callback that can be used to launch the component.
//   data            : Data to be used with the connect callback.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:57:06 PST 2003
//
// Modifications:
//    Jeremy Meredith, Thu Oct  9 14:04:45 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Brad Whitlock, Thu Mar 11 12:51:34 PDT 2004
//    I added keep alive RPC.
//
//    Jeremy Meredith, Thu May 24 10:20:57 EDT 2007
//    Added SSH tunneling argument; pass it along to RemoteProcess::Open.
//
// ****************************************************************************

void
RemoteProxyBase::Create(const std::string &hostName,
                        HostProfile::ClientHostDetermination chd,
                        const std::string &clientHostName,
                        bool manualSSHPort,
                        int sshPort,
                        bool useTunneling,
                        ConnectCallback *connectCallback, void *data,
                        bool createAsThoughLocal)
{
    // Create a remote process object for the remote component.
    if(connectCallback == NULL)
    {
        component = new RemoteProcess(GetVisItString());
    }
    else
    {
        ExistingRemoteProcess *p = 
            new ExistingRemoteProcess(GetVisItString());
        p->SetConnectCallback(connectCallback);
        p->SetConnectCallbackData(data);
        component = p;
    }

    // Add the component's name as an extra argument.
    component->AddArgument(componentName);

    // Set the engine's launch progress callback.
    component->SetProgressCallback(progressCallback, progressCallbackData);

    // Add extra arguments to the remote process object.
    AddExtraArguments();

    //
    // Open the remote component.
    //
    component->Open(hostName, chd, clientHostName,
                    manualSSHPort, sshPort, useTunneling,
                    nRead, nWrite, createAsThoughLocal);

    //
    // Hook up the sockets to the xfer object.
    //
    xfer.SetOutputConnection(component->GetReadConnection());
    xfer.SetInputConnection(component->GetWriteConnection());
    xfer.Add(&quitRPC);
    xfer.Add(&keepAliveRPC);

    //
    // Set up the RPC's for the remote component.
    //
    SetupComponentRPCs();

    //
    // List the objects that were hooked up. 
    //
    xfer.ListObjects();
}

// ****************************************************************************
// Method: RemoteProxyBase::Close
//
// Purpose: 
//   Tells the remote component to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::Close()
{
    quitRPC(true);
}

// ****************************************************************************
// Method: RemoteProxyBase::SendKeepAlive
//
// Purpose: 
//   Sends a keep alive RPC to the component.
//
// Note:       This method could throw an exception if the connection to the
//             component has been lost.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:11:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::SendKeepAlive()
{
    keepAliveRPC();
}

// ****************************************************************************
// Method: RemoteProxyBase::SetRemoteUserName
//
// Purpose: 
//   Sets the username to use on the remote machine.
//
// Arguments:
//   rName : The remote user name.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::SetRemoteUserName(const std::string &rName)
{
    remoteUserName = rName;
}

// ****************************************************************************
// Method: RemoteProxyBase::SetProgressCallback
//
// Purpose: 
//   This method sets a progress callback that is called while the remote
//   component is being launched.
//
// Arguments:
//   cb   : The progress callback.
//   data : Data to pass to the progress callback.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 14:59:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::SetProgressCallback(bool (*cb)(void *, int), void *data)
{
    progressCallback = cb;
    progressCallbackData = data;
}

// ****************************************************************************
// Method: RemoteProxyBase::AddArgument
//
// Purpose: 
//   Adds an argument to the list of arguments to be passed to the component.
//
// Arguments:
//   arg : The argument to be added to the list.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:00:55 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProxyBase::AddArgument(const std::string &arg)
{
    argv.push_back(arg);
}

// ****************************************************************************
//  Method:  RemoteProxyBase::AddProfileArguments
//
//  Purpose:
//    Adds the appropriate arguments to a remote proxy.
//
//  Arguments:
//    profile         : the host profile used to set the arguments.
//    addParallelArgs : true if this process is going to launch itself
//                      in parallel, and false if the vcl has already
//                      created a parallel job and we just need to
//                      choose the parallel engine when needed
//
//  Programmer:  Jeremy Meredith
//  Creation:    June 26, 2003
//
//  Modifications:
//    Brad Whitlock, Wed Aug 4 17:40:57 PST 2004
//    Moved this method from ViewerRemoteProcessChooser and changed the code
//    a little.
//
//    Brad Whitlock, Tue Aug 10 17:16:55 PST 2004
//    Undefined a macro on Windows because it interferes with HostProfile.
//
//    Jeremy Meredith, Thu Sep 15 16:34:15 PDT 2005
//    Added ability to launch visit script under mpirun in order to set up
//    the environment on beowulf clusters (and similar).
//
//    Hank Childs, Sat Dec  3 20:17:07 PST 2005
//    Added argument for hardware acceleration.
//
//    Eric Brugger, Thu Feb 15 12:04:45 PST 2007
//    Added an argument for passing additional sublauncher arguments.
//
// ****************************************************************************

void
RemoteProxyBase::AddProfileArguments(const HostProfile &profile,
    bool addParallelArgs)
{
    int  i;

    //
    // Set the user's login name.
    //
#if defined(_WIN32) && defined(GetUserName)
#undef GetUserName
#endif
    SetRemoteUserName(profile.GetUserName());

    //
    // Add the parallel arguments.
    //
    if (profile.GetParallel())
    {
        char temp[10];
        if (!addParallelArgs)
        {
            AddArgument("-par");
        }

        if (addParallelArgs)
        {
            SNPRINTF(temp, 10, "%d", profile.GetNumProcessors());
            AddArgument("-np");
            AddArgument(temp);
        }
        SetNumProcessors(profile.GetNumProcessors());

        if (profile.GetNumNodesSet() &&
            profile.GetNumNodes() > 0)
        {
            if (addParallelArgs)
            {
                SNPRINTF(temp, 10, "%d", profile.GetNumNodes());
                AddArgument("-nn");
                AddArgument(temp);
            }
            SetNumNodes(profile.GetNumNodes());
        }

        if (addParallelArgs)
        {
            if (profile.GetPartitionSet() &&
                profile.GetPartition().length() > 0)
            {
                AddArgument("-p");
                AddArgument(profile.GetPartition());
            }

            if (profile.GetBankSet() &&
                profile.GetBank().length() > 0)
            {
                AddArgument("-b");
                AddArgument(profile.GetBank());
            }

            if (profile.GetTimeLimitSet() &&
                profile.GetTimeLimit().length() > 0)
            {
                AddArgument("-t");
                AddArgument(profile.GetTimeLimit());
            }

            if (profile.GetLaunchMethodSet() &&
                profile.GetLaunchMethod().length() > 0)
            {
                AddArgument("-l");
                AddArgument(profile.GetLaunchMethod());
            }

            if (profile.GetLaunchArgsSet() &&
                profile.GetLaunchArgs().length() > 0)
            {
                AddArgument("-la");
                AddArgument(profile.GetLaunchArgs());
            }

            if (profile.GetSublaunchArgsSet() &&
                profile.GetSublaunchArgs().length() > 0)
            {
                AddArgument("-sla");
                AddArgument(profile.GetSublaunchArgs());
            }

            if (profile.GetMachinefileSet() &&
                profile.GetMachinefile().length() > 0)
            {
                AddArgument("-machinefile");
                AddArgument(profile.GetMachinefile());
            }

            if (profile.GetVisitSetsUpEnv())
            {
                AddArgument("-setupenv");
            }
        }
#if 0 // disabling dynamic load balancing for now
        if (profile.GetForceStatic())
        {
            if (addParallelArgs)
            {
                AddArgument("-forcestatic");
            }
            SetLoadBalancing(0);
        }

        if (profile.GetForceDynamic())
        {
            if (addParallelArgs)
            {
                AddArgument("-forcedynamic");
            }
            SetLoadBalancing(1);
        }
#else
        // force all static until speed issues are resolved
        if (addParallelArgs)
        {
            AddArgument("-forcestatic");
        }
        SetLoadBalancing(0);
#endif
    }
    if (profile.GetCanDoHWAccel())
    {
        AddArgument("-hw-accel");
        if (profile.GetHavePreCommand())
        {
            AddArgument("-hw-pre");
            AddArgument(profile.GetHwAccelPreCommand());
        }
        if (profile.GetHavePostCommand())
        {
            AddArgument("-hw-post");
            AddArgument(profile.GetHwAccelPostCommand());
        }
    }

    // Add the timeout argument
    char temp[10];
    SNPRINTF(temp, 10, "%d", profile.GetTimeout());
    AddArgument("-timeout");
    AddArgument(temp);

    //
    // Add any additional arguments specified in the profile
    //
    for (i = 0; i < profile.GetArguments().size(); ++i)
        AddArgument(profile.GetArguments()[i]);
}

// ****************************************************************************
// Method: RemoteProxyBase::Parallel
//
// Purpose: 
//   Returns whether the application on the other side is parallel.
//
// Returns:    false.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:15:46 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
RemoteProxyBase::Parallel() const
{
    return false;
}

// ****************************************************************************
// Method: RemoteProxyBase::GetVisItString
//
// Purpose: 
//   Returns the path to the visit command on the remote computer.
//
// Returns:    The path to the visit command.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:01:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
RemoteProxyBase::GetVisItString() const
{ 
    // The -dir flag means that the visit script is not in our path, so we
    // must prepend it to the name of the visit script.  Do that check now.
    std::string visitString = "visit";
    for (int i = 0 ; i < argv.size() ; ++i)
    {
        if (argv[i] == "-dir" && (i + 1) < argv.size())
        {
            const std::string &dirArg = argv[i + 1];
            visitString = dirArg;
            if (dirArg[dirArg.size() - 1] != '/')
                visitString += "/";
            visitString += "bin/visit";
            ++i;
        }
    }

    return visitString;
}

// ****************************************************************************
// Method: RemoteProxyBase::AddExtraArguments
//
// Purpose: 
//   Adds extra arguments to the component before it is launched.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:03:15 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 10 14:47:35 PST 2003
//   Let -dir pass through.
//
// ****************************************************************************

void
RemoteProxyBase::AddExtraArguments()
{
    //
    // Set the remote user name.
    //
    if(remoteUserName.size() > 0)
        component->SetRemoteUserName(remoteUserName);

    //
    // Add any extra arguments to the component before opening it.
    //
    for(int i = 0; i < argv.size(); ++i)
        component->AddArgument(argv[i]);
}

// ****************************************************************************
// Method: RemoteProxyBase::GetReadConnection
//
// Purpose: 
//   Get the component's i'th read connection.
//
// Arguments:
//   i : The index of the read connection. Default=0
//
// Returns:    The i'th connection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 11:44:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

Connection *
RemoteProxyBase::GetReadConnection(int i) const
{
    return component ? component->GetReadConnection(i) : 0;
}

// ****************************************************************************
// Method: RemoteProxyBase::GetWriteConnection
//
// Purpose: 
//   Get the component's i'th write connection.
//
// Arguments:
//   i : The index of the write connection. Default=0
//
// Returns:    The i'th connection.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 11:44:29 PST 2007
//
// Modifications:
//   
// ****************************************************************************

Connection *
RemoteProxyBase::GetWriteConnection(int i) const
{
    return component ? component->GetWriteConnection(i) : 0;
}
