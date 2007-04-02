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

#include <stdio.h>
#include <LauncherApplication.h>
#include <ConnectionGroup.h>
#include <SocketConnection.h>
#include <DebugStream.h>
#include <IncompatibleVersionException.h>
#include <CouldNotConnectException.h>
#include <LostConnectionException.h>
#include <RPCExecutor.h>
#include <map>
#include <snprintf.h>

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

//
// Static member variables.
//
LauncherApplication *LauncherApplication::instance = 0;
std::map<int, bool> LauncherApplication::childDied;

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
// ****************************************************************************

LauncherApplication::LauncherApplication() : parent(), xfer(), quitRPC(),
    keepAliveRPC(), launchRPC()
{
    quitExecutor = 0;
    keepAliveExecutor = 0;
    launchExecutor = 0;
    connectSimExecutor = 0;
    timeout = 60;
    keepGoing = true;

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
// ****************************************************************************

LauncherApplication::~LauncherApplication()
{
    instance = 0;
    delete quitExecutor;
    delete keepAliveExecutor;
    delete launchExecutor;
    delete connectSimExecutor;
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
// ****************************************************************************

void
LauncherApplication::Connect(int *argc, char **argv[])
{
    //
    // Try and connect back to the client.
    //
    TRY
    {
        parent.Connect(1, 1, argc, argv, true);
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
//   This is the main loop for the laucher application.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 17:32:23 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
LauncherApplication::MainLoop()
{
    // Create a connection group that we will use to check if any
    // connections have input to be read.
    ConnectionGroup connGroup;
    connGroup.AddConnection(parent.GetWriteConnection());

    // The application's main loop
    while(keepGoing)
    {
        TurnOnAlarm();

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
//   
// ****************************************************************************

void
LauncherApplication::LaunchProcess(const stringVector &launchArgs)
{
    if(launchArgs.size() < 1)
        return;

    std::string remoteProgram(launchArgs[0]);
    debug2 << "LaunchRPC command = " << remoteProgram.c_str() << ", args=(";

    // Make a command line array for the exec functions.
    char **args = new char *[launchArgs.size() + 1];
    memset(args, 0, (launchArgs.size() + 1) * sizeof(char *));
    int i;
    for(i = 0; i < launchArgs.size(); ++i)
    {
        args[i] = new char[launchArgs[i].size() + 1];
        strcpy(args[i], launchArgs[i].c_str());
        if(i > 0)
            debug2 << launchArgs[i].c_str() << " ";
    }
    debug2 << ")" << endl;

    // We have command line arguments for a command to launch.

    int remoteProgramPid;
#if defined(_WIN32)
    // Do it the WIN32 way where we use the _spawnvp system call.
    remoteProgramPid = _spawnvp(_P_NOWAIT, remoteProgram.c_str(), args);
#else
    // Watch for a process who died
    childDied[remoteProgramPid] = false;
    signal(SIGCHLD, DeadChildHandler);

    switch (remoteProgramPid = fork())
    {
    case -1:
        // Could not fork.
        exit(-1);
        break;
    case 0:
        // Close stdin and any other file descriptors.
        fclose(stdin);
        for (int k = 3 ; k < 32 ; ++k)
        {
            close(k);
        }
        // Execute the process on the local machine.
        execvp(remoteProgram.c_str(), args);
        exit(-1);
        break;   // OCD
    default:
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
        for(i = 0; i < launchArgs.size(); ++i)
            args2[i] = args[i];

        // Tell the client that we could not connect.
        TerminateConnectionRequest(launchArgs.size(), args2);

        delete [] args2;
    }   
#endif

    // Free the command line storage.
    for(i = 0; i < launchArgs.size(); ++i)
        delete [] args[i];
    delete [] args;
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
// ****************************************************************************
void
LauncherApplication::ConnectSimulation(const stringVector &launchArgs,
                                       const std::string &simHost, int simPort,
                                       const std::string &simSecurityKey)
{
    int                s;
    struct hostent     *hp;
    struct sockaddr_in server;

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
    for (int i=0; i<launchArgs.size(); i++)
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
    CATCHALL(...)
    {
        // We know that we're going to get here, but no action is required.
    }
    ENDTRY
}
