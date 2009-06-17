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

#include <visit-config.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <map>

#if defined(_WIN32)
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h> // alarm
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

#include <LauncherApplication.h>
#include <ConnectionGroup.h>
#include <SocketConnection.h>
#include <Utility.h>
#include <vectortypes.h>

#if defined(PANTHERHACK)
// Broken on Panther
#else
#include <SocketBridge.h>
#endif
#include <DebugStream.h>
#include <IncompatibleVersionException.h>
#include <CouldNotConnectException.h>
#include <LostConnectionException.h>
#include <RPCExecutor.h>
#include <snprintf.h>

// Capture child output on UNIX & Mac. VCL is never run on Windows so this
// should not matter.
#if !defined(_WIN32) && !defined(PANTHERHACK)
#define CAPTURE_CHILD_OUTPUT
#endif

//
// Static member variables.
//
LauncherApplication *LauncherApplication::instance = 0;
std::map<int, bool> LauncherApplication::childDied;

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
#if defined(PANTHERHACK)
// Broken on Panther
#else
    SocketBridge bridge(newlocalport,oldlocalport);
    bridge.Bridge();
#endif
}



// ****************************************************************************
// Method: RPCExecutor<QuitRPC>::Execute
//
// Purpose:
//   Execute a QuitRPC.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:06:04 PST 2003
//
// Modifications:
//
// ****************************************************************************
template<>
void
RPCExecutor<QuitRPC>::Execute(QuitRPC *quit)
{
    debug2 << "Executing QuitRPC" << endl;
    if (!quit->GetQuit())
        quit->SendError();
    else
        quit->SendReply();
}

// ****************************************************************************
// Method: RPCExecutor<KeepAliveRPC>::Execute
//
// Purpose: 
//   Executes a KeepAliveRPC.
//
// Arguments:
//   keepAlive : A pointer to the KeepAliveRPC object.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 10:36:46 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

template<>
void
RPCExecutor<KeepAliveRPC>::Execute(KeepAliveRPC *keepAlive)
{
    debug3 << "Executing KeepAliveRPC" << endl;
    keepAlive->SendReply();
}

// ****************************************************************************
// Method: RPCExecutor<LaunchRPC>::Execute
//
// Purpose: 
//   Uses the command line arguments passed from the client to fork a child
//   process that connects back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 09:46:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

template<>
void
RPCExecutor<LaunchRPC>::Execute(LaunchRPC *launch)
{
    debug2 << "Executing LaunchRPC" << endl;
    LauncherApplication::Instance()->LaunchProcess(launch->GetLaunchArgs());
    launch->SendReply();
}

// ****************************************************************************
// Method: RPCExecutor<ConnectSimRPC>::Execute
//
// Purpose: 
//   Uses the arguments passed from the client to tell a simulation to connect
//   back to the client.
//
// Programmer: Jeremy Meredith
// Creation:   March 23, 2004
//
// Modifications:
//   
// ****************************************************************************

template<>
void
RPCExecutor<ConnectSimRPC>::Execute(ConnectSimRPC *connect)
{
    debug2 << "Executing ConnectSimRPC"
           << "  Host=" << connect->GetSimHost().c_str()
           << "  Port=" << connect->GetSimPort() << endl;
    LauncherApplication::Instance()->ConnectSimulation(
                                                 connect->GetLaunchArgs(),
                                                 connect->GetSimHost(),
                                                 connect->GetSimPort(),
                                                 connect->GetSimSecurityKey());
    connect->SendReply();
}

// ****************************************************************************
// Method: LauncherApplication::Instance
//
// Purpose: 
//   Creates the one instance of the LauncherApplication object.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:27:15 PST 2003
//
// Modifications:
//   
// ****************************************************************************

LauncherApplication *
LauncherApplication::Instance()
{
     if(!LauncherApplication::instance)
         LauncherApplication::instance = new LauncherApplication;

     return LauncherApplication::instance;
}

// ****************************************************************************
// Method: LauncherApplication::LauncherApplication
//
// Purpose: 
//   Constructor for the LauncherApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:27:46 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 12 10:37:49 PDT 2004
//   Added new members to handle the KeepAliveRPC.
//
//   Jeremy Meredith, Tue Mar 30 17:27:59 PST 2004
//   Added connectSimExecutor.
//
//   Jeremy Meredith, Thu May 24 11:25:16 EDT 2007
//   Initialize useSSHTunneling.
//
//   Brad Whitlock, Wed Nov 21 11:13:20 PST 2007
//   Added support for forwarding child output.
//
// ****************************************************************************

LauncherApplication::LauncherApplication() : parent(), xfer(), quitRPC(),
    keepAliveRPC(), launchRPC(), childOutput()
{
    quitExecutor = 0;
    keepAliveExecutor = 0;
    launchExecutor = 0;
    connectSimExecutor = 0;
    timeout = 60;
    keepGoing = true;
    useSSHTunneling = false;

#if !defined(_WIN32)
    // Set up an alarm signal handler to exit gracefully.
    signal(SIGALRM, LauncherApplication::AlarmHandler);
#endif
}

// ****************************************************************************
// Method: LauncherApplication::~LauncherApplication
//
// Purpose: 
//   Destructor for the LauncherApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:28:17 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 12 10:38:13 PDT 2004
//   Added keepAliveExecutor.
//
//   Jeremy Meredith, Tue Mar 30 17:27:59 PST 2004
//   Added connectSimExecutor.
//
//   Brad Whitlock, Wed Nov 21 11:13:20 PST 2007
//   Added support for forwarding child output.
//
// ****************************************************************************

LauncherApplication::~LauncherApplication()
{
    instance = 0;
    delete quitExecutor;
    delete keepAliveExecutor;
    delete launchExecutor;
    delete connectSimExecutor;

#ifdef CAPTURE_CHILD_OUTPUT
    for(int i = 0; i < childOutput.size(); ++i)
        delete childOutput[i];
#endif
}

// ****************************************************************************
// Method: LauncherApplication::Execute
//
// Purpose: 
//   Executes the launcher application.
//
// Arguments:
//   argc : The number of command line arguments.
//   argv : The command line arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:28:42 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherApplication::Execute(int *argc, char **argv[])
{
    // Process the command line arguments.
    ProcessArguments(argc, argv);

    // Connect back to the client application.
    Connect(argc, argv);

    // Take care of the application main loop.
    MainLoop();
}

// ****************************************************************************
// Method: LauncherApplication::ProcessArguments
//
// Purpose: 
//   Processes the command line arguments.
//
// Arguments:
//   argc : The number of command line arguments.
//   argv : The command line arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:29:41 PST 2003
//
// Modifications:
//   Jeremy Meredith, Thu May 24 11:25:31 EDT 2007
//   Added detection of the SSH tunneling argument.  This is how
//   the client tells us the SSH port forwarding is in effect.
//   
// ****************************************************************************

void
LauncherApplication::ProcessArguments(int *argcp, char **argvp[])
{
    int argc = *argcp;
    char **argv = *argvp;

    // Process any relevant command line arguments.
    for(int i = 0; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if(arg == "-timeout" && (i+1) < argc)
        {
            timeout = atol(argv[i+1]);
            ++i;
        }
        else if (arg == "-sshtunneling")
        {
            useSSHTunneling = true;
        }
    }
}

// ****************************************************************************
// Method: LauncherApplication::Connect
//
// Purpose: 
//   Connects back to the client process.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:30:33 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 29 11:19:26 PDT 2003
//   I changed the interface to ParentProcess::Connect.
//
//   Brad Whitlock, Fri Mar 12 10:39:26 PDT 2004
//   I added the KeepAlive RPC.
//
//   Jeremy Meredith, Tue Mar 30 17:27:59 PST 2004
//   Added connectSimExecutor.
//
//   Brad Whitlock, Wed Nov 21 11:15:42 PST 2007
//   Added another socket to forward output to the client.
//
// ****************************************************************************

void
LauncherApplication::Connect(int *argc, char **argv[])
{
    //
    // Try and connect back to the client.
    //
    TRY
    {
        parent.Connect(1, 2, argc, argv, true);
    }
    CATCH(IncompatibleVersionException)
    {
        debug1 << "The launcher connected to a client that has a different "
               << "version number than the launcher itself."
               << endl;
        RETHROW;
    }
    CATCH(CouldNotConnectException)
    {
        debug1 << "The launcher could not connect to the client." << endl;
        RETHROW;
    }
    ENDTRY

    // Connect the xfer object to the parent process's connections.
    xfer.SetInputConnection(parent.GetWriteConnection());
    xfer.SetOutputConnection(parent.GetReadConnection());

    // Hook up the RPC's to the xfer object.
    xfer.Add(&quitRPC);
    xfer.Add(&keepAliveRPC);
    xfer.Add(&launchRPC);
    xfer.Add(&connectSimRPC);

    // Hook up the RPC executors to the RPC's.
    quitExecutor      = new RPCExecutor<QuitRPC>(&quitRPC); 
    keepAliveExecutor = new RPCExecutor<KeepAliveRPC>(&keepAliveRPC); 
    launchExecutor    = new RPCExecutor<LaunchRPC>(&launchRPC);
    connectSimExecutor= new RPCExecutor<ConnectSimRPC>(&connectSimRPC);
}

// ****************************************************************************
// Method: LauncherApplication::TurnOnAlarm
//
// Purpose: 
//   Turns on an alarm signal to make the application timeout and die.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 09:05:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherApplication::TurnOnAlarm()
{
#if !defined(_WIN32)
    // Reset the alarm
    alarm(timeout*60);
#endif
}

// ****************************************************************************
// Method: LauncherApplication::TurnOffAlarm
//
// Purpose: 
//   Turns off the alarm signal that will make the application terminate.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 09:06:05 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherApplication::TurnOffAlarm()
{
#if !defined(_WIN32)
    // We've got some input.  Turn off the alarm so we can't
    // time out.
    alarm(0);
#endif
}

// ****************************************************************************
// Method: LauncherApplication::AlarmHandler
//
// Purpose: 
//   Handles the alarm signal to terminate the application.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 09:06:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherApplication::AlarmHandler(int)
{
    debug1 << "Launcher exited due to an inactivity timeout of "
           << LauncherApplication::Instance()->timeout << " minutes." << endl;
    exit(0);
}

// ****************************************************************************
// Method: LauncherApplication::DeadChildHandler
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
LauncherApplication::DeadChildHandler(int)
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
// Method: LauncherApplication::MainLoop
//
// Purpose: 
//   This is the main loop for the launcher application.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:32:23 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Nov 21 10:36:01 PST 2007
//   Added support for forwarding child process output to the client.
//
// ****************************************************************************

void
LauncherApplication::MainLoop()
{
    // The application's main loop
    while(keepGoing)
    {
        TurnOnAlarm();

        // Create a connection group that we will use to check if any
        // connections have input to be read.
        ConnectionGroup connGroup;
        connGroup.AddConnection(parent.GetWriteConnection());
#ifdef CAPTURE_CHILD_OUTPUT
        for(int i = 0; i < childOutput.size(); ++i)
            connGroup.AddConnection(childOutput[i]);
#endif
        // Check the connections for input that needs to be processed.
        if(connGroup.CheckInput())
        {
            if(connGroup.NeedsRead(0))
            {
                TurnOffAlarm();

                TRY
                {
                    // Process input.
                    ProcessInput();

                    // See if we've been told to quit yet.
                    keepGoing &= !quitRPC.GetQuit();
                }
                CATCH(LostConnectionException)
                {
                    // Terminate the application.
                    keepGoing = false;
                }
                ENDTRY
            }
#ifdef CAPTURE_CHILD_OUTPUT
            else if(childOutput.size() > 0)
            {
                char         buf[1000 + 1];
                ssize_t      nbuf = 0;
                std::string *outputs = new std::string[childOutput.size()];
                bool        *valid = new bool[childOutput.size()];

                // Gather all of the output from each child in sequence.
                for(int i = 0; i < childOutput.size(); ++i)
                {
                    valid[i] = true;
                    if(connGroup.NeedsRead(i + 1))
                    {
                        debug1 << "Child " << i << " needs to be read (desc="
                               << childOutput[i]->GetDescriptor() << ")" << endl;
                        // Read from the child process's pipe. Note that we use
                        // the read() function because we can't use 
                        // SocketConnection::DirectRead because it calls recv
                        // and that requires a real socket descriptor, whereas
                        // in this case, we have a pipe file descriptor.
                        int nZeroesRead = 0;
                        do
                        {
                            nbuf = read(childOutput[i]->GetDescriptor(), (void*)buf, 1000);

                            if(nbuf > 0)
                            {
                                nZeroesRead = 0;
                                buf[nbuf] = 0;
                                outputs[i] = outputs[i] + std::string(buf);
                            }
                            else
                                ++nZeroesRead;
                        } while(nZeroesRead < 100 && childOutput[i]->NeedsRead(false));

                        // If we read enough zeroes in a row, consider the connection dead.
                        if(nZeroesRead >= 100)
                        {
                            valid[i] = false;
                            debug1 << "Lost connection to child " << i << endl;
                        }
                        else
                            debug1 << "Done reading for child " << i << endl;
                    }
                }

                // Now that we have output from all of the child processes, send it
                // back to the launcher proxy so the client can do something with
                // it.
                std::string completeOutput;
                std::vector<Connection *> validConnections;
                for(int i = 0; i < childOutput.size(); ++i)
                {
                    // Append the output to a complete output that we'll send 
                    // to the client.
                    if(outputs[i].size() > 0)
                    {
                        debug5 << "CHILD OUTPUT[" << i << "]: " << outputs[i].c_str() << endl;
                        completeOutput += outputs[i];
                        if(outputs[i][outputs[i].size()-1] != '\n' && i < childOutput.size()-1)
                            completeOutput += std::string("\n");
                    }

                    // Delete any child connections that we lost.
                    if(!valid[i])
                        delete childOutput[i];
                    else
                        validConnections.push_back(childOutput[i]);
                }
                childOutput = validConnections;
                delete [] outputs;
                delete [] valid;

                // Forward the output to the client.
                if(completeOutput.size() > 0)
                {
                    debug1 << "Sending " << completeOutput.size() << " bytes" << endl;
                    parent.GetReadConnection(1)->DirectWrite(
                        (const unsigned char *)completeOutput.c_str(), 
                        completeOutput.size());
                }
            }
#endif
        }
    }
}

// ****************************************************************************
// Method: LauncherApplication::ProcessInput
//
// Purpose: 
//   Processes input from the client application.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 11:00:54 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
LauncherApplication::ProcessInput()
{
    // Try reading input from the parent process's write socket.
    int amountRead = xfer.GetInputConnection()->Fill();

    // Process the new information.
    if(amountRead > 0)
        xfer.Process();

    return (amountRead > 0);
}

// ****************************************************************************
//  Method:  LauncherApplication::SetupGatewaySocketBridgeIfNeeded
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
#if defined(PANTHERHACK)
// Broken on Panther
#else
void
LauncherApplication::SetupGatewaySocketBridgeIfNeeded(stringVector &launchArgs, 
    bool doBridge)
{
    const char *mName="LauncherApplication::SetupGatewaySocketBridgeIfNeeded: ";

    if (!useSSHTunneling)
        return;

    debug5 << mName << "SSH Tunneling was enabled\n";

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

    if (doBridge && portargument != -1 && hostargument != -1)
    {
        debug5 << mName << "Parallel Engine launch detected; "
               << "Setting up gateway port bridge.\n";
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
            exit(-1);
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
                  exit(0);
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
        debug5 << mName << "Not launching parallel engine; "
               << "skipping gateway port bridge." << endl;
    }
}
#endif

// ****************************************************************************
// Method: LauncherApplication::LaunchProcess
//
// Purpose: 
//   Forks and executes a process with the specified arguments.
//
// Arguments:
//   launchArgs : The arguments used to launch the process.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 11:23:56 PDT 2003
//
// Modifications:
//   Thomas R. Treadway, Mon Oct  8 13:27:42 PDT 2007
//   Backing out SSH tunneling on Panther (MacOS X 10.3)
//
//   Brad Whitlock, Wed Nov 21 10:30:50 PST 2007
//   I added support for forwarding child process stdout to VCL through a pipe
//   so we can forward that output to the client.
//
//   Brad Whitlock, Mon Apr 27 16:31:23 PST 2009
//   I moved the testing for parallel & engine up to here so I can call
//   SetupGatewaySocketBridgeIfNeeded when I need to connect to simulations.
//
// ****************************************************************************

void
LauncherApplication::LaunchProcess(const stringVector &origLaunchArgs)
{
    const char *mName = "LauncherApplication::LaunchProcess: ";

    stringVector launchArgs(origLaunchArgs);

    if(launchArgs.size() < 1)
        return;

    // Set up an extra indirection if we're tunneling and 
    // launching a parallel engine.  SSH port forwarding
    // is typically restricted to forwarding from localhost
    // which doesn't work if we're on a compute node.
#if defined(PANTHERHACK)
// Broken on Panther
#else
    bool launching_parallel = false;
    bool launching_engine = false;
    for (size_t i=0; i<launchArgs.size(); i++)
    {
        if (launchArgs[i] == "-np" || launchArgs[i] == "-par")
        {
            launching_parallel = true;
        }
        else if (launchArgs[i] == "-engine")
        {
            launching_engine = true;
        }
    }
    bool doBridge = launching_parallel && launching_engine;
    SetupGatewaySocketBridgeIfNeeded(launchArgs, doBridge);
#endif

    std::string remoteProgram(launchArgs[0]);
    debug2 << "LaunchRPC command = " << remoteProgram.c_str() << ", args=(";

    // Make a command line array for the exec functions.
    char **args = new char *[launchArgs.size() + 1];
    memset(args, 0, (launchArgs.size() + 1) * sizeof(char *));
    for(size_t i = 0; i < launchArgs.size(); ++i)
    {
        args[i] = new char[launchArgs[i].size() + 1];
        strcpy(args[i], launchArgs[i].c_str());
        if(i > 0)
            debug2 << launchArgs[i].c_str() << " ";
    }
    debug2 << ")" << endl;

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
        exit(-1);
#endif

    switch (remoteProgramPid = fork())
    {
    case -1:
        // Could not fork.
        exit(-1);
        break;
    case 0:
        // Close stdin and any other file descriptors.
        fclose(stdin);
#ifdef CAPTURE_CHILD_OUTPUT
        // Send the process' stdout to our pipe.
        dup2(f_des[1], fileno(stdout));
        dup2(f_des[1], fileno(stderr));
        close(f_des[0]);
        close(f_des[1]);
#endif
        for (int k = 3 ; k < 32 ; ++k)
        {
            close(k);
        }
        // Execute the process on the local machine.
        execvp(remoteProgram.c_str(), args);
        exit(-1);
        break;   // OCD
    default:
#ifdef CAPTURE_CHILD_OUTPUT
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
    else
    {
        // Add the child's output pipe to the list of descriptors that
        // we will check. We add the pipe file descriptor as a 
        // SocketConnection object.
        childOutput.push_back(new SocketConnection(f_des[0]));
    }
#endif
#endif

    // Free the command line storage.
    for(size_t i = 0; i < launchArgs.size(); ++i)
        delete [] args[i];
    delete [] args;
}

// ****************************************************************************
// Method: GetSSHClient
//
// Purpose: 
//   Gets the SSH_CLIENT variable if it exists.
//
// Arguments:
//   sshClient : The return variable.
//
// Returns:    True on success; false on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 24 15:21:18 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

bool
GetSSHClient(std::string &sshClient)
{
    bool retval = false;
    const char *s = NULL;
    if((s = getenv("SSH_CLIENT")) != NULL)
    {
        stringVector sv = SplitValues(s, ' ');
        if(sv.size() > 0)
        {
            retval = true;
            sshClient = sv[0];
        }
    }
    else if((s = getenv("SSH2_CLIENT")) != NULL)
    {
        stringVector sv = SplitValues(s, ' ');
        if(sv.size() > 0)
        {
            retval = true;
            sshClient = sv[0];
        }
    }
    else if((s = getenv("SSH_CONNECTION")) != NULL)
    {
        stringVector sv = SplitValues(s, ' ');
        if(sv.size() > 0)
        {
            retval = true;
            sshClient = sv[0];
        }
    }
    else
    {
        debug1 << "None of the SSH environment variables were set!" << endl;
    }
    return retval;
}

// ****************************************************************************
//  Method:  LauncherApplication::ConnectSimulation
//
//  Purpose:
//    Connect to a running simulation
//
//  Arguments:
//    launchArgs   the arguments to be passed to the engine
//    simHost      the hostname of the machine where the simulation is running
//    simPort      the port number where the simulation is listening
//
//  Note:  Much of this code was taken from ParentProcess.  Both should
//         have their socket stuff abstracted into a common location.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
//    Hank Childs, Fri Jun  9 15:53:20 PDT 2006
//    Replace sprintf(tmp, "") with strcpy(tmp, "") to remove compiler warning.
//
//    Brad Whitlock, Fri Apr 24 15:21:58 PDT 2009
//    I fixed "parse from ssh_client" for simulations.
//
//    Brad Whitlock, Mon Apr 27 16:31:23 PST 2009
//    I added support for SSH tunnelling.
//
// ****************************************************************************

void
LauncherApplication::ConnectSimulation(const stringVector &origLaunchArgs,
                                       const std::string &simHost, int simPort,
                                       const std::string &simSecurityKey)
{
    const char *mName = "LauncherApplication::ConnectSimulation: ";
    int                s;
    struct hostent     *hp;
    struct sockaddr_in server;
    stringVector       launchArgs;
    size_t             i;

    debug1 << mName << "origLaunchArgs={";
    for(int i = 0; i < origLaunchArgs.size(); ++i)
        debug1 << origLaunchArgs[i] << " ";
    debug1 << "}" << endl;
    debug1 << "simHost=" << simHost << endl;
    debug1 << "simPort=" << simPort << endl;
    debug1 << "simSecurityKey=" << simSecurityKey << endl;

    //
    // Convert -guesshost to -host XXXX
    //
    for (size_t i=0; i<origLaunchArgs.size(); i++)
    {
        if(origLaunchArgs[i] == "-guesshost")
        {
            // replace -guesshost with -host XXXX
            std::string sshClient;
            if(GetSSHClient(sshClient))
            {
                launchArgs.push_back("-host");
                launchArgs.push_back(sshClient);
            }
        }
        else        
            launchArgs.push_back(origLaunchArgs[i]);
    }

    debug1 << mName << "AFTER -guesshost conversion: launchArgs={";
    for(int i = 0; i < launchArgs.size(); ++i)
        debug1 << launchArgs[i] << " ";
    debug1 << "}" << endl;

    // Set up an extra indirection if we're tunneling and 
    // launching a parallel engine.  SSH port forwarding
    // is typically restricted to forwarding from localhost
    // which doesn't work if we're on a compute node.
    SetupGatewaySocketBridgeIfNeeded(launchArgs, true);

    debug1 << mName << "AFTER Socket Bridge: launchArgs={";
    for(int i = 0; i < launchArgs.size(); ++i)
        debug1 << launchArgs[i] << " ";
    debug1 << "}" << endl;

    //
    // Get the simulation host information
    //
    void *hostInfo = (void *)gethostbyname(simHost.c_str());

    //
    // Set up the structures for opening the sockets.
    //
    hp = (struct hostent *)hostInfo;
    if (hp == NULL)
    {
        EXCEPTION0(CouldNotConnectException);
    }

    memset(&server, 0, sizeof(server));
    memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
    server.sin_family = hp->h_addrtype;
    server.sin_port = htons(simPort);
    
    // 
    // Create a socket.
    // 
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        EXCEPTION0(CouldNotConnectException);
    }

    // Disable the Nagle algorithm 
    int opt = 1;
#if defined(_WIN32)
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char FAR *)&opt, sizeof(int));
#else
    setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    int success = connect(s, (struct sockaddr *)&server, sizeof(server));
    if (success < 0)
    {
#if defined(_WIN32)
        closesocket(s);
#else
        close(s);
#endif
        EXCEPTION0(CouldNotConnectException);
    }

    //
    // Send the security key and launch information to the simulation
    //
    char tmp[2000];
    size_t          nleft, nwritten;
    const char      *ptr;

    sprintf(tmp, "%s\n", simSecurityKey.c_str());

    ptr = (const char*)tmp;
    nleft = strlen(tmp);
    while (nleft > 0)
    {
        if((nwritten = send(s, (const char *)ptr, nleft, 0)) <= 0)
        {
            EXCEPTION0(CouldNotConnectException);
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }

    //
    // Receive a reply
    //
    strcpy(tmp,"");
    char *tbuf = tmp;
    char *tptr = tmp;
    int n;
    char *newline = strstr(tbuf, "\n");
    while (!newline)
    {
        n = recv(s, tptr, 2000, 0);
        tptr += n;
        *tptr = 0;
        newline = strstr(tbuf, "\n");
    }
    *newline = 0;
    if (strcmp(tmp, "success") != 0)
    {
        EXCEPTION0(CouldNotConnectException);
    }

    // Create the Launch args
    strcpy(tmp, "");
    for (size_t i=0; i<launchArgs.size(); i++)
    {
        strcat(tmp, launchArgs[i].c_str());
        strcat(tmp, "\n");
    }
    strcat(tmp, "\n");

    // Send it!
    ptr = (const char*)tmp;
    nleft = strlen(tmp);
    while (nleft > 0)
    {
        if((nwritten = send(s, (const char *)ptr, nleft, 0)) <= 0)
        {
            EXCEPTION0(CouldNotConnectException);
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
}

// ****************************************************************************
// Method: LauncherApplication::TerminateConnectionRequest
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
LauncherApplication::TerminateConnectionRequest(int argc, char *argv[])
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
