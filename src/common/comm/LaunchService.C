/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <LaunchService.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#if defined(_WIN32)
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h> // alarm
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>
#endif

#include <DebugStream.h>
#include <FileFunctions.h>
#include <ParentProcess.h>
#include <VisItException.h>
#include <SocketBridge.h>
#include <SocketConnection.h>

// Capture child output on UNIX & Mac. 
#if !defined(_WIN32)
#define CAPTURE_CHILD_OUTPUT
#endif

//
// Static member variables.
//
std::map<int, bool> LaunchService::childDied;

// ****************************************************************************
//  Function:  CreateSocketBridge
//
//  Purpose:
//    Initiate the socket bridge.
//
//  Arguments:
//    ports      (really an int[2]):
//                 ports[0]=new local port
//                 ports[1]=old local port
//
//  Programmer:  Jeremy Meredith
//  Creation:    June  5, 2007
//
//  Modifications:
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
// ****************************************************************************
static void CreateSocketBridge(void *ports)
{
    int newlocalport = ((int*)ports)[0];
    int oldlocalport = ((int*)ports)[1];

    SocketBridge bridge(newlocalport,oldlocalport);
    bridge.Bridge();
}

// ****************************************************************************
// Method: LaunchService::LaunchService
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 28 17:11:19 PST 2011
//
// Modifications:
//   
// ****************************************************************************

LaunchService::LaunchService()
{
}

// ****************************************************************************
// Method: LaunchService::~LaunchService
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 28 17:11:19 PST 2011
//
// Modifications:
//   
// ****************************************************************************

LaunchService::~LaunchService()
{
}

// ****************************************************************************
// Method: LaunchService::DeadChildHandler
//
// Purpose: 
//    Signal handler for a SIGCHLD even while waiting for remote connections.
//    Catch a child that died and mark it's success or failure in the
//    childDied array.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 11:18:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
LaunchService::DeadChildHandler(int)
{
#if !defined(_WIN32)
    int status;
    int pid;
    pid = wait(&status);

    childDied[pid] = (status == 0 ? false : true);

    signal(SIGCHLD, DeadChildHandler);
#endif
}

// ****************************************************************************
//  Method:  LaunchService::SetupGatewaySocketBridgeIfNeeded
//
//  Purpose:
//    If SSH tunneling is enabled and we're about to launch a parallel
//    engine, we need to set up a local port from any incoming host
//    that gets forwarded through the appropriate SSH tunnel.  We cannot
//    access SSH tunnels at the login node for a cluster from the
//    compute nodes, because by default SSH only listens for connections
//    from localhost.
//
//    The launch arguments containing the login node forward ("localhost":port)
//    are also converted to the new bridge (loginnode:newport);
//
//  Arguments:
//    launchArgs    the launch arguments (these will be modified in-place!)
//
//  Programmer:  Jeremy Meredith
//  Creation:    May 24, 2007
//
//  Modifications:
//    Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//    Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//    Brad Whitlock, Mon Apr 27 16:31:23 PST 2009
//    I changed the routine so the check for setting up the bridge is passed
//    in rather than calculated in here from launch arguments.
//
// ****************************************************************************

bool
LaunchService::SetupGatewaySocketBridgeIfNeeded(stringVector &launchArgs)
{
    const char *mName="LaunchService::SetupGatewaySocketBridgeIfNeeded: ";

    // Get the port and host.
    int  oldlocalport       = -1;
    int  portargument       = -1;
    int  hostargument       = -1;
    for (size_t i=0; i<launchArgs.size(); i++)
    {
        if (i<launchArgs.size()-1 && launchArgs[i] == "-port")
        {
            oldlocalport = atoi(launchArgs[i+1].c_str());
            portargument = i+1;
        }
        else if (i<launchArgs.size()-1 && launchArgs[i] == "-host")
        {
            hostargument = i+1;
        }
    }

    bool setupBridge = (portargument != -1 && hostargument != -1);
    if(setupBridge)
    {
        debug5 << mName << "Setting up gateway port bridge.\n";
        // find a new local port
        int lowerRemotePort = 10000;
        int upperRemotePort = 40000;
        int remotePortRange = 1+upperRemotePort-lowerRemotePort;

#if defined(_WIN32)
        srand((unsigned)time(0));
        int newlocalport = lowerRemotePort+(rand()%remotePortRange);
#else
        srand48(long(time(0)));
        int newlocalport = lowerRemotePort+(lrand48()%remotePortRange);
#endif
        debug5 << mName << "Bridging new port INADDR_ANY/" << newlocalport
               << " to tunneled port localhost/" << oldlocalport << endl;

        // replace the host with my host name
        char hostname[1024];
        gethostname(hostname,1024);
        launchArgs[hostargument] = hostname;

        // replace the launch argument port number
        char newportstr[10];
        sprintf(newportstr,"%d",newlocalport);
        launchArgs[portargument] = newportstr;

        // fork and start the socket bridge
        int *ports = new int[2];
        ports[0] = newlocalport;
        ports[1] = oldlocalport;
#ifdef _WIN32
        _beginthread(CreateSocketBridge, 0, (void*)ports);
#else
        switch (fork())
        {
          case -1:
            // Could not fork.
            exit(-1); // HOOKS_IGNORE
            break;
          case 0:
              {
                  // The child process will start the bridge
                  // Close stdin and any other file descriptors.
                  fclose(stdin);
                  for (int k = 3 ; k < 32 ; ++k)
                  {
                      close(k);
                  }
                  CreateSocketBridge((void*)ports);
                  exit(0); // HOOKS_IGNORE
                  break;
              }
          default:
            // Parent process continues on as normal
            // Caution: there is a slight race condition here, though
            // it would require the engine to launch and try to connect
            // back before the child process got the bridge set up.
            // The odds of this happening are low, but it should be fixed.
            break;
        }
#endif
    }
    else
    {
        debug5 << mName << "Required -host or -port argument not found" << endl;
    }

    return setupBridge;
}

// ****************************************************************************
// Method: LaunchService::LaunchProcess
//
// Purpose: 
//   Launch a process, optionally reading its output.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 28 17:07:30 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
LaunchService::Launch(const stringVector &origLaunchArgs, bool doBridge, 
    SocketConnection **conn)
{
    const char *mName = "LaunchService::LaunchProcess: ";
    debug1 << mName << "start" << endl;

    stringVector launchArgs(origLaunchArgs);
    bool readOutput = conn != NULL;
    if(readOutput)
        *conn = NULL;

    if(launchArgs.empty())
        return;

    // Set up a socket bridge if we need one.
    if(doBridge)
        SetupGatewaySocketBridgeIfNeeded(launchArgs);

    std::string remoteProgram(launchArgs[0]);
    debug1 << mName << "LaunchRPC command = " << remoteProgram.c_str() << ", args=(";

    // Make a command line array for the exec functions.
    char **args = new char *[launchArgs.size() + 1];
    memset(args, 0, (launchArgs.size() + 1) * sizeof(char *));
    for(size_t i = 0; i < launchArgs.size(); ++i)
    {
        args[i] = new char[launchArgs[i].size() + 1];
        strcpy(args[i], launchArgs[i].c_str());
        if(i > 0)
            debug1 << launchArgs[i].c_str() << " ";
    }
    debug1 << ")" << endl;

    // We have command line arguments for a command to launch.

    int remoteProgramPid = 0;
#if defined(_WIN32)
    // Do it the WIN32 way where we use the _spawnvp system call.
    remoteProgramPid = _spawnvp(_P_NOWAIT, remoteProgram.c_str(), args);
#else
    // Watch for a process who died
    childDied[remoteProgramPid] = false;
    signal(SIGCHLD, DeadChildHandler);

#ifdef CAPTURE_CHILD_OUTPUT
    // Create a pipe.
    int f_des[2];
    if(pipe(f_des) == -1)
        readOutput = false;
#endif

    switch (remoteProgramPid = fork())
    {
    case -1:
        // Could not fork.
        exit(-1); // HOOKS_IGNORE
        break;
    case 0:
        // Close stdin and any other file descriptors.
        fclose(stdin);
#ifdef CAPTURE_CHILD_OUTPUT
        // Send the process' stdout/stderr to our pipe.
        if(readOutput)
        {
            dup2(f_des[1], fileno(stdout));
            dup2(f_des[1], fileno(stderr));
            close(f_des[0]);
            close(f_des[1]);
        }
#endif
        for (int k = 3 ; k < 32 ; ++k)
        {
            close(k);
        }
        // Execute the process on the local machine.
        if (remoteProgram.size() > 0 && remoteProgram[0] == '~')
            remoteProgram = ExpandUserPath(remoteProgram);
        execvp(remoteProgram.c_str(), args);
        exit(-1); // HOOKS_IGNORE
        break;   // OCD
    default:
#ifdef CAPTURE_CHILD_OUTPUT
        if(readOutput)
            close(f_des[1]);
#endif
        break;
    }

    // Stop watching for dead children
    signal(SIGCHLD, SIG_DFL);

    // If we had a dead child, try and connect back to the client that
    // wanted to connect to the dead child.
    if(childDied[remoteProgramPid])
    {
        // Create a temp array of pointers to the strings that we
        // created and pass the temp array to the TerminateConnectionRequest
        // method because it creates a ParentProcess object that will
        // rearrange the pointers in the array.
        char **args2 = new char *[launchArgs.size() + 1];
        for(size_t i = 0; i < launchArgs.size(); ++i)
            args2[i] = args[i];

        // Tell the client that we could not connect.
        TerminateConnectionRequest(launchArgs.size(), args2);

        delete [] args2;
    }
#ifdef CAPTURE_CHILD_OUTPUT
    else if(readOutput)
    {
        // Add the child's output pipe to the list of descriptors that
        // we will check. We add the pipe file descriptor as a 
        // SocketConnection object.
        *conn = new SocketConnection(f_des[0]);
    }
#endif
#endif

    // Free the command line storage.
    for(size_t i = 0; i < launchArgs.size(); ++i)
        delete [] args[i];
    delete [] args;
    debug1 << mName << "end" << endl;
}

// ****************************************************************************
// Method: LaunchService::TerminateConnectionRequest
//
// Purpose: 
//   Tells the client that we could not launch the desired process. This
//   lets the client fail gracefully instead of hang.
//
// Arguments:
//   argc : The number of arguments in argv.
//   argv : The argument array used to connect back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 11:46:35 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 29 11:39:03 PDT 2003
//   Changed interface to ParentProcess::Connect.
//
//   Mark C. Miller, Wed Jun 17 14:27:08 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL.
// ****************************************************************************

void
LaunchService::TerminateConnectionRequest(int argc, char *argv[])
{
    // Try and connect back to the process that initiated the request and
    // send it a non-zero fail code so it will terminate the connection.
    TRY
    {
        debug1 << "Terminating connection request to the client." << endl;

        ParentProcess killer;

        // Connect back to the process and say that we could not connect.
        killer.Connect(1, 1, &argc, &argv, true, 3);
    }
    CATCHALL
    {
        // We know that we're going to get here, but no action is required.
    }
    ENDTRY
}
