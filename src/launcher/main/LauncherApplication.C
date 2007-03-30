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

#if defined(_WIN32)
#include <process.h>
#include <windows.h>
#else
#include <unistd.h> // alarm
#include <signal.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
//   
// ****************************************************************************

LauncherApplication::LauncherApplication() : parent(), xfer(), quitRPC(),
    launchRPC()
{
    quitExecutor = 0;
    launchExecutor = 0;
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
//   
// ****************************************************************************

LauncherApplication::~LauncherApplication()
{
    instance = 0;
    delete quitExecutor;
    delete launchExecutor;
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
        parent.Connect(argc, argv, true);
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
    xfer.Add(&launchRPC);

    // Hook up the RPC executors to the RPC's.
    quitExecutor   = new RPCExecutor<QuitRPC>(&quitRPC); 
    launchExecutor = new RPCExecutor<LaunchRPC>(&launchRPC);
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
        killer.Connect(&argc, &argv, true, 3);
    }
    CATCHALL(...)
    {
        // We know that we're going to get here, but no action is required.
    }
    ENDTRY
}
