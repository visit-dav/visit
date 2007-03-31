#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#if defined(_WIN32)
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <PTY.h>
#include <signal.h>
#endif

#include <visit-config.h>

#include <RemoteProcess.h>
#include <BadHostException.h>
#include <SocketConnection.h>
#include <CommunicationHeader.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <CancelledConnectException.h>
#include <CouldNotConnectException.h>

#include <DebugStream.h>
#include <map>

#ifdef HAVE_THREADS
#if !defined(_WIN32)
#include <pthread.h>
// Variables required for pthreads.
static pthread_attr_t thread_atts;
static bool init_thread_atts = false;
#define MUTEX_CREATE(mutex)  pthread_mutex_init(&(mutex), NULL)
#define MUTEX_DESTROY(mutex) pthread_mutex_destroy(&(mutex))
#define MUTEX_LOCK(mutex)    pthread_mutex_lock(&(mutex))
#define MUTEX_UNLOCK(mutex)  pthread_mutex_unlock(&(mutex))
#define MUTEX_TYPE           pthread_mutex_t
#else
// Windows threads
#define MUTEX_CREATE(mutex)  InitializeCriticalSection(&(mutex))
#define MUTEX_DESTROY(mutex) DeleteCriticalSection(&(mutex))
#define MUTEX_LOCK(mutex)    EnterCriticalSection(&(mutex))
#define MUTEX_UNLOCK(mutex)  LeaveCriticalSection(&(mutex))
#define MUTEX_TYPE           CRITICAL_SECTION
#endif

// Data structure for the thread callback.
struct ThreadCallbackDataStruct
{
    int                 pid;
    int                 desc;
    DESCRIPTOR          listenSocketNum;
    struct sockaddr_in *sin;
    int                 Errno;
    bool                alive;
    MUTEX_TYPE          mutex;
};
#endif

// The port that we try to get for listening for remote connections.
#define INITIAL_PORT_NUMBER 5600

//
// Static data
//
void (*RemoteProcess::getAuthentication)(const char *, const char *, int) = NULL;
bool RemoteProcess::disablePTY = false;

using std::map;
static map<int, bool> childDied;

#if !defined(_WIN32)
// ****************************************************************************
//  Function:  catch_dead_child
//
//  Purpose:
//    Signal handler for a SIGCHLD even while waiting for remote connections.
//    Catch a child that died and mark it's success or failure in the
//    childDied array.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2002
//
// ****************************************************************************
static void
catch_dead_child(int sig)
{
    // assert (sig == SIGCHLD);
    int status;
    int pid;
    pid = wait(&status);

    childDied[pid] = (status == 0 ? false : true);

    signal(SIGCHLD, catch_dead_child);
}
#endif


// ****************************************************************************
// Method: RemoteProcess::RemoteProcess
//
// Purpose: 
//   Constructor for the RemoteProcess class.
//
// Arguments:
//    rProgram : The name of the remote program to execute.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:11:08 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//
//    Brad Whitlock, Fri Oct 20 12:46:49 PDT 2000
//    Added code to initialize the SocketConnection pointers.
//
//    Brad Whitlock, Mon Nov 20 17:03:34 PST 2000
//    Added initializer for localHost.
//
//    Brad Whitlock, Thu Feb 21 10:04:53 PDT 2002
//    Added initializer for localUserName.
//
//    Brad Whitlock, Thu Sep 26 16:50:04 PST 2002
//    Initialized progressCallback and progressCallbackData.
//
//    Brad Whitlock, Mon Dec 16 14:28:53 PST 2002
//    I added securityKey.
//
// ****************************************************************************

RemoteProcess::RemoteProcess(const std::string &rProgram) : localHost("notset"),
    localUserName(), securityKey(), remoteHost("localhost"),
    remoteProgram(rProgram), remoteUserName("notset"), argList()
{
    remoteProgramPid = -1;
    listenSocketNum = -1;

    // Zero out the SocketConnection pointers.
    readConnections = 0;
    writeConnections = 0;
    nReadConnections = 0;
    nWriteConnections = 0;

    // Set the callback information.
    progressCallback = 0;
    progressCallbackData = 0;
}

// ****************************************************************************
// Method: RemoteProcess::~RemoteProcess
//
// Purpose: 
//   Destructor for the RemoteProcess class. It closes any socket
//   file descriptors that were opened.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:11:51 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//
//    Brad Whitlock, Thu Oct 5 18:05:09 PST 2000
//    Added code to delete the SocketConnections.
//
//    Brad Whitlock, Tue Mar 19 15:55:08 PST 2002
//    Abstracted the deletion of the connections.
//
//    Brad Whitlock, Tue Oct 1 15:04:03 PST 2002
//    I closed the listen socket.
//
//    Brad Whitlock, Fri Jan 3 15:29:54 PST 2003
//    I moved the code to close the listen socket into another method.
//
// ****************************************************************************

RemoteProcess::~RemoteProcess()
{
    // Delete the read SocketConnections
    if(nReadConnections > 0)
    {
        for(int i = 0; i < nReadConnections; ++i)
        {
            // Delete the connection
            delete readConnections[i];
        }
        delete [] readConnections;
        readConnections = 0;
        nReadConnections = 0;
    }

    if(readConnections != 0)
    {
        delete [] readConnections;
        readConnections = 0;
        nReadConnections = 0;
    }

    // Delete the write SocketConnections
    if(nWriteConnections > 0)
    {
        for(int i = 0; i < nWriteConnections; ++i)
        {
            // Delete the connection
            delete writeConnections[i];
        }
    }

    if(writeConnections != 0)
    {
        delete [] writeConnections;
        writeConnections = 0;
        nWriteConnections = 0;
    }

    //
    // Close the listening socket so we don't waste file descriptors.
    //
    CloseListenSocket();
}

// ****************************************************************************
//  Method:  RemoteProcess::DisablePTY
//
//  Purpose:
//    Disables usage of PTYs.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July  3, 2003
//
// ****************************************************************************

void
RemoteProcess::DisablePTY()
{
    disablePTY = true;
}

// ****************************************************************************
// Method: RemoteProcess::AddArgument
//
// Purpose: 
//   Adds an argument to the RemoteProcess's argument list.
//
// Arguments:
//   arg : The argument to add.
//
// Returns:    
//
// Note:       
//   This method must be called prior to RemoteProcess::Open to have
//   any useful effect.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:12:35 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::AddArgument(const std::string &arg)
{
    argList.push_back(arg);
}

// ****************************************************************************
// Method: RemoteProcess::SetRemoteUserName
//
// Purpose: 
//   Sets the username to use when launching a process on a remote
//   machine.
//
// Arguments:
//   rUserName : A string containing the user's login name on the
//               remote machine.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 20 12:39:59 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::SetRemoteUserName(const std::string &rUserName)
{
    remoteUserName = rUserName;
}

// ****************************************************************************
// Method: RemoteProcess::GetReadConnection
//
// Purpose: 
//   Gets a pointer to the i'th read Connection.
//
// Arguments:
//   i : The index of the Connection we want.
//
// Returns:    A pointer to the i'th read Connection, or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:36:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
RemoteProcess::GetReadConnection(int i) const
{
    return (i < nReadConnections) ? readConnections[i] : 0;
}

// ****************************************************************************
// Method: RemoteProcess::GetWriteConnection
//
// Purpose: 
//   Gets a pointer to the i'th write Connection.
//
// Arguments:
//   i : The index of the Connection we want.
//
// Returns:    A pointer to the i'th write Connection, or 0.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:36:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

Connection *
RemoteProcess::GetWriteConnection(int i) const
{
    return (i < nWriteConnections) ? writeConnections[i] : 0;
}

// ****************************************************************************
// Method: RemoteProcess::GetLocalHostName
//
// Purpose: 
//   Returns the name of the localhost machine.
//
// Returns:    The name of the localhost machine.
//
// Notes:      This method only returns the correct name if the Open method
//             has been called.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 11:26:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const std::string &
RemoteProcess::GetLocalHostName() const
{
    return localHost;
}

// ****************************************************************************
// Method: RemoteProcess::GetLocalUserName
//
// Purpose: 
//   Returns the name of the local user.
//
// Returns:    The local user name
//
// Notes:      This method only returns the correct name if the Open method
//             has been called.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 10:04:05 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

const std::string &
RemoteProcess::GetLocalUserName() const
{
    return localUserName;
}

// ****************************************************************************
// Method: RemoteProcess::HostIsLocal
//
// Purpose: 
//   Returns whether or not a hostname is local.
//
// Arguments:
//   rHost : The name of the host to check.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 13:03:32 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
RemoteProcess::HostIsLocal(const std::string &rHost) const
{
    return (rHost == localHost || rHost == "localhost");
}

// ****************************************************************************
// Method: RemoteProcess::GetPid
//
// Purpose: 
//   Returns the process id of the remote process.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 13:29:36 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
RemoteProcess::GetProcessId() const
{
    return remoteProgramPid;
}

// ****************************************************************************
// Method: RemoteProcess::GetSocketAndPort
//
// Purpose: 
//   Creates a socket and gets a port to use.
//
// Returns:    
//    true if it worked, false if it did not.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:16:22 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//    It now also only listens on a single port.
//
//    Brad Whitlock, Tue Mar 19 15:56:46 PST 2002
//    Made it work on MS Windows. We don't want to re-use addresses on Windows
//    because it causes the ports to not get incremented.
//
//    Brad Whitlock, Mon Mar 17 08:51:59 PDT 2003
//    I made it use a different starting port.
//
// ****************************************************************************

bool
RemoteProcess::GetSocketAndPort()
{
    int  on = 1;
    bool portFound = false;

    // Open a socket.
    listenSocketNum = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocketNum < 0)
    {
        // Cannot open a socket.
        return false;
    }

    //
    // Look for a port that can be used.
    //
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    listenPortNum = INITIAL_PORT_NUMBER;
    while (!portFound && listenPortNum < 32767)
    {
        sin.sin_port = htons(listenPortNum);
#if !defined(_WIN32)
        setsockopt(listenSocketNum, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif
        if (bind(listenSocketNum, (struct sockaddr *)&sin, sizeof(sin)) < 0)
        {
            listenPortNum++;
        }
        else
        {
            portFound = true;
        }
    }
    if (!portFound)
    {
        // Cannot find unused port.
        return false;
    }

    return true;
}

// ****************************************************************************
// Method: RemoteProcess::CloseListenSocket
//
// Purpose: 
//   Closes the listen socket so the port that we've reserved gets released.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 3 15:28:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::CloseListenSocket()
{
    //
    // Close the listening socket so the port gets released.
    //
    if(listenSocketNum != -1)
    {
#if defined(_WIN32)
        closesocket(listenSocketNum);
#else
        close(listenSocketNum);
#endif
        listenSocketNum = -1;
    }
}

// ****************************************************************************
// Method: RemoteProcess::CallProgressCallback
//
// Purpose: 
//   Calls the launch progress callback function.
//
// Arguments:
//   stage : The reason we're calling the function.
//
// Returns:    The callback's return value.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 30 07:32:38 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

bool
RemoteProcess::CallProgressCallback(int stage)
{
    bool retval = true;
#ifdef HAVE_THREADS
    // Call the progress callback.
    if(progressCallback != 0)
        retval = (*progressCallback)(progressCallbackData, stage);
#endif
    return retval;
}

// ****************************************************************************
// Method: RemoteProcess::AcceptSocket
//
// Purpose: 
//   Accepts a socket connection from the remote process. Must be
//   called after GetSocketAndPort and the remote process has been
//   launched.
//
// Returns:
//   The socket file descriptor or -1.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:17:36 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//
//    Brad Whitlock, Thu Oct 5 18:02:19 PST 2000
//    I changed the code to use the class's global sockaddr_in struct
//    and to return the socket descriptor.
//
//    Brad Whitlock, Wed Nov 8 18:03:51 PST 2000
//    I added conditional compilation for gnu compilers since their
//    prototype for the accept call is a little different.
//
//    Jeremy Meredith, Wed Jun  6 21:43:16 PDT 2001
//    Changed the check for len to use socklen_t.  Added initialization of len.
//
//    Jeremy Meredith, Wed Jul 10 12:55:58 PDT 2002
//    Added a check to see if the child process died with an error status.
//    This signifies that we could not possibly accept successfully.
//
//    Brad Whitlock, Fri Sep 27 11:29:07 PDT 2002
//    I rewrote the routine so it makes use of subroutines and exception
//    handling so we can support cancelled connections.
//
// ****************************************************************************

int
RemoteProcess::AcceptSocket()
{
    int desc = -1;
    int opt = 1;

#ifdef HAVE_THREADS
    if(progressCallback == 0)
        desc = SingleThreadedAcceptSocket();
    else
        desc = MultiThreadedAcceptSocket();
#else
    desc = SingleThreadedAcceptSocket();
#endif

    // If the descriptor is -1, we could not connect to the remote process.
    if(desc == -1)
    {
        EXCEPTION0(CouldNotConnectException);
    }

    // If the descriptor is -2, we cancelled the connection to the
    // remote process.
    if(desc == -2)
    {
        EXCEPTION0(CancelledConnectException);
    }

    // Disable Nagle algorithm.
#if defined(_WIN32)
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&opt, sizeof(int));
#else
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    return desc;
}

// ****************************************************************************
// Method: RemoteProcess::SingleThreadedAcceptSocket
//
// Purpose: 
//   Accepts a socket connection from the remote process. Must be
//   called after GetSocketAndPort and the remote process has been
//   launched.
//
// Returns:
//   The socket file descriptor or -1.
//
// Note:       This is a single-threaded version that blocks the application
//             until the completion of the accept call.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 16:42:28 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
RemoteProcess::SingleThreadedAcceptSocket()
{
    int desc = -1;

    // Wait for the socket to become available on the other side.
    do
    {
#ifdef HAVE_SOCKLEN_T
        socklen_t len;
#else
        int len;
#endif
        len = sizeof(struct sockaddr);
        desc = accept(listenSocketNum, (struct sockaddr *)&sin, &len);
    }
    while (desc == -1 && errno == EINTR && childDied[GetProcessId()] == false);

    return desc;
}

#ifdef HAVE_THREADS
// ****************************************************************************
// Function: threaded_accept_callback
//
// Purpose:
//   This is a thread callback function that performs the accept network
//   function call. We do the accept on another thread so we can process other
//   events, etc while we do the blocking accept.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 16:54:30 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 6 12:13:27 PDT 2002
//   I put a loop around the accept function call so that if we catch a
//   signal during the accept, we have an opportunity to try it again.
//
// ****************************************************************************

#if defined(WIN32)
DWORD WINAPI threaded_accept_callback(LPVOID data)
#else
static void *threaded_accept_callback(void *data)
#endif
{
    ThreadCallbackDataStruct *cb = (ThreadCallbackDataStruct *)data;

#if !defined(_WIN32)
    // Set the thread cancellation policy so the main thread can terminate
    // this thread immediately if there is a problem.
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
#endif

    // Wait for the socket to connect back.
    int desc = -1;
    do
    {
#ifdef HAVE_SOCKLEN_T
    socklen_t len;
#else
    int len;
#endif
        len = sizeof(struct sockaddr);
        desc = accept(cb->listenSocketNum, (struct sockaddr *)cb->sin, &len);
    } while(desc == -1 && errno == EINTR && childDied[cb->pid] == false);

    // Set the results into the callback struct that the other thread is polling.
    MUTEX_LOCK(cb->mutex);
    cb->alive = false;
    cb->Errno = (desc == -1) ? errno : 0;
    cb->desc = desc;
    MUTEX_UNLOCK(cb->mutex);

    return 0;
}
#endif

// ****************************************************************************
// Method: RemoteProcess::MultiThreadedAcceptSocket
//
// Purpose: 
//   Accepts a socket connection from the remote process. Must be
//   called after GetSocketAndPort and the remote process has been
//   launched.
//
// Returns:    The socket descriptor or -1.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 16:56:14 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 6 12:20:32 PDT 2002
//   I fixed some bugs that prevented the routine from working with
//   engines that were submitted to the batch system.
//
// ****************************************************************************

int
RemoteProcess::MultiThreadedAcceptSocket()
{
#ifdef HAVE_THREADS
    int desc = -1;
 
    // Set up some callback data for the thread callback.
    ThreadCallbackDataStruct cb;
    cb.pid = GetProcessId();
    cb.desc = -1;
    cb.listenSocketNum = listenSocketNum;
    cb.sin = &sin;
    cb.Errno = 0;
    cb.alive = true;
    // Create the mutex. It has to be done before we create the thread because
    // the thread callback needs the mutex.
    MUTEX_CREATE(cb.mutex);

    bool validThread = true;
#if defined(WIN32)
    // Create the thread Windows style.
    DWORD  Id;
    HANDLE tid;
    tid = CreateThread(0, 0, threaded_accept_callback,(LPVOID)&cb, 0, &Id);
    validThread = (tid != INVALID_HANDLE_VALUE);
#else
    if(!init_thread_atts)
    {
        pthread_attr_init(&thread_atts);
        init_thread_atts = true;
    }
    // Create the thread pthread style.
    pthread_t tid;
    if(pthread_create(&tid, &thread_atts, threaded_accept_callback,
       (void *)&cb) == -1)
    {
        validThread = false;
    }
#endif

    if(validThread)
    {
        //
        // If the accept callback thread is still alive, then loop until it is
        // done or the user cancels the operation or there is a problem
        // launching the program.
        //
        bool alive;
        MUTEX_LOCK(cb.mutex);
        alive = cb.alive;
        MUTEX_UNLOCK(cb.mutex);
        if(alive)
        {
            //
            // Wait for the socket to become available on the other side.
            //
            bool noDescriptor, noProcessError, noCancel;
            do
            {
                // Call the progress callback.
                noCancel = CallProgressCallback(1);

                // Determine if we should keep looping. We have to access the
                // information through a mutex since there is more than one flag
                // that we're checking.
                MUTEX_LOCK(cb.mutex);
                noDescriptor = (cb.desc == -1);
                noProcessError = (cb.Errno == 0 && childDied[GetProcessId()] == false);
                MUTEX_UNLOCK(cb.mutex);
            }
            while(noDescriptor && noProcessError && noCancel);

            // If the thread is still alive, we encountered an error or we cancelled
            // the process launch. If the thread is still alive, cancel it.
            MUTEX_LOCK(cb.mutex);
            if(cb.alive)
            {
#if defined(WIN32)
                TerminateThread(tid, 0);
                CloseHandle(tid);
#else
                pthread_cancel(tid);
#endif
                // Only return -2 (CancelledConnection) if we did not cancel.
                // Otherwise, we probably got here because the desired executable
                // could not be run (CouldNotConnect).
                cb.desc = noCancel ? -1 : -2;
            }
            MUTEX_UNLOCK(cb.mutex);
        }

        // Set the return value.
        desc = cb.desc;
    }
    else
    {
        // We could not create the thread so do the single-threaded version.
        desc = SingleThreadedAcceptSocket();
    }

    // Destroy the mutex.
    MUTEX_DESTROY(cb.mutex);

    // Return the descriptor.
    return desc;
#else
    // No threads implementation.
    return -1;
#endif
}

// ****************************************************************************
// Method: RemoteProcess::Open
//
// Purpose: 
//   Opens sockets and launches a remote process using ssh.
//
// Arguments:
//   rHost    : The remote host to run on.
//   numRead  : The number of read sockets to create to the remote process.
//   numWrite : The number of write sockets to create to the remote process.
//   createAsThoughLocal : Creates the process as though it was a local process
//                         regardless of the hostname.
//
// Returns:    
//   true if it worked, false if it did not.
//
// Note:       
//   numRead and numWrite cannot both be 0, otherwise the method
//   will return without creating the remote process.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 14 09:19:03 PDT 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.
//    It now also only listens on a single port.
//
//    Brad Whitlock, Fri Aug 25 11:04:59 PDT 2000
//    Changed it to launch a local process if the rHost is
//    "localhost" or if the rHost is the same as the localHost name
//    that is looked up.
//
//    Brad Whitlock, Mon Nov 20 16:55:53 PST 2000
//    Moved the guts to smaller methods.
//
//    Jeremy Meredith, Wed Jul 10 12:57:14 PDT 2002
//    Added code to set up a handler to watch for the remote process to die.
//    It is allowed to close with a zero exit status, but it gets marked
//    as "dead" if it exits with a nonzero exit code while we are waiting
//    for it.
//
//    Jeremy Meredith, Tue Oct 22 15:02:50 PDT 2002
//    Moved the initialization of the signal handler into LaunchLocal and
//    LaunchRemote.  It must be the last thing called before fork(), since
//    if we are using PTYs, the call to grantpt may hang if there is a
//    signal handler for SIGCHLD that calls wait() -- grantpt gets the
//    status and wait() never returns.
//
//    Brad Whitlock, Mon May 5 13:00:59 PST 2003
//    I moved the code that creates the command line into CreateCommandLine
//    instead of having it inside of the methods to launch processes.
//
// ****************************************************************************

bool
RemoteProcess::Open(const std::string &rHost, int numRead, int numWrite,
    bool createAsThoughLocal)
{
    // Start making the connections and start listening.
    if(!StartMakingConnection(rHost, numRead, numWrite))
        return false;

    // Add all of the relevant command line arguments to a vector of strings.
    stringVector commandLine;
    CreateCommandLine(commandLine, rHost, numRead, numWrite,
                      createAsThoughLocal);

    //
    // Launch the remote process.
    //
    if (HostIsLocal(rHost) || createAsThoughLocal)
        LaunchLocal(commandLine);
    else
        LaunchRemote(commandLine);

    childDied[GetProcessId()] = false;

    // Finish the connections.
    FinishMakingConnection(numRead, numWrite);

#if !defined(_WIN32)
    // Stop watching for dead children
    signal(SIGCHLD, SIG_DFL);
#endif

    return true;
}

// ****************************************************************************
// Method: RemoteProcess::WaitForTermination
//
// Purpose: 
//   Waits for the remote process to quit.
//
// Programmer: Brad Whitlock
// Creation:   Wed Apr 3 12:03:11 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::WaitForTermination()
{
    if(remoteProgramPid != -1)
    {
        int result;
#if defined(_WIN32)
        _cwait(&result, remoteProgramPid, _WAIT_CHILD);
#else
        waitpid(remoteProgramPid, &result, 0);
#endif
    }
}

// ****************************************************************************
// Method: RemoteProcess::StartMakingConnection
//
// Purpose: 
//   Starts making the connection to the remote process so we're ready to
//   go on this end before the remote process is launched.
//
// Arguments:
//   rHost    : The remote host to run on.
//   numRead  : The number of read sockets to create to the remote process.
//   numWrite : The number of write sockets to create to the remote process.
//
// Returns:    
//   true if it worked, false if it did not.
//   
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 16:53:53 PST 2000
//
// Modifications:
//   Jeremy Meredith, Thu Apr 19 09:43:07 PDT 2001
//   Added code to get the full local host name.
//
//   Brad Whitlock, Thu Feb 21 10:14:35 PDT 2002
//   Added code to get the local user name.
//
//   Brad Whitlock, Thu Sep 12 11:34:58 PDT 2002
//   I added an extra lookup on Windows that gets the actual local hostname
//   so it does not use any bad names returned by Windows.
//
//   Brad Whitlock, Thu Dec 19 11:32:43 PDT 2002
//   I made the code create a security key.
//
// ****************************************************************************

bool
RemoteProcess::StartMakingConnection(const std::string &rHost, int numRead,
    int numWrite)
{
    //
    // If there are no sockets to be opened, get out since that seems
    // like a usedless thing to do.
    //
    if(numRead + numWrite == 0)
        return false;

    //
    // Set the remote host name and make sure that it is valid. If it
    // is not valid, throw a BadHostException. Do not bother checking
    // whether or not remoteHost is valid if it equals "localhost" since
    // in that case we'll be spawning a local process.
    //
    remoteHost = rHost;
    bool remote = (remoteHost != std::string("localhost"));
    if(remote && (gethostbyname(remoteHost.c_str()) == NULL))
    {
        EXCEPTION1(BadHostException, remoteHost);
    }

    //
    // Get the local host name since it will be a command line option
    // for the remote process.
    //
    char localHostStr[256];
    if (gethostname(localHostStr, 256) == -1)
    {
        // Couldn't get the hostname, it's probably invalid so
        // throw a BadHostException.
        EXCEPTION1(BadHostException, localHostStr);
    }
    struct hostent *localHostEnt = gethostbyname(localHostStr);
    if (localHostEnt == NULL)
    {
        // Couldn't get the full host entry; it's probably invalid so
        // throw a BadHostException.
        EXCEPTION1(BadHostException, localHostStr);
    }
    localHost = std::string(localHostEnt->h_name);
#if defined(_WIN32)
    // On some Windows systems, particularly those hooked up to dial up
    // connections where the actual network hostname may not match the
    // computer's hostname, we need to do another lookup to ensure that
    // we get the actual local hostname so the remote process can connect
    // back successfully.
    if(remote)
    {
        bool looping = true;
        for(int i = 0; (i < localHostEnt->h_length) && looping; ++i)
        {
            looping = (localHostEnt->h_addr_list[i] != 0);
            if(looping)
            {
                struct hostent *h;
                h = gethostbyaddr(localHostEnt->h_addr_list[i], 4, PF_INET);
                if(h)
                {
                    localHost = std::string(h->h_name);
                }
            }
        }
    }
#endif

    //
    // Get the local user name.
    //
#if defined(_WIN32)
    char username[100];
    DWORD maxLen = 100;
    GetUserName((LPTSTR)username, (LPDWORD)&maxLen);
    localUserName = std::string(username);
#else
    struct passwd *users_passwd_entry = NULL;
    if((users_passwd_entry = getpwuid(getuid())) != NULL)
        localUserName = std::string(users_passwd_entry->pw_name);
#endif

    //
    // Create a security key
    //
    securityKey = CommunicationHeader::CreateRandomKey();

    //
    // Open the socket for listening
    //
    if(!GetSocketAndPort())
    {
        // Could not open socket and port
        return false;
    }

    //
    // Start listening for connections.
    //
    listen(listenSocketNum, 5);

    return true;
}

// ****************************************************************************
// Method: RemoteProcess::FinishMakingConnection
//
// Purpose: 
//   Finishes making the connections needed to talk to the remote process.
//
// Arguments:
//   numRead  : The number of read sockets.
//   numWrite : The number of write sockets.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 20 16:50:57 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 25 15:13:41 PST 2002
//   Made it use Connection objects instead of SocketConnection.
//
//   Jeremy Meredith, Wed Jul 10 12:58:14 PDT 2002
//   Made it throw an exception if it gets a -1 descriptor for any socket.
//   This should be an error when trying to launch a remote process.
//
//   Brad Whitlock, Thu Sep 26 17:08:01 PST 2002
//   I made it call the progress callback.
//
//   Brad Whitlock, Fri Jan 3 15:27:54 PST 2003
//   I added code to close the listen socket.
//
// ****************************************************************************

void
RemoteProcess::FinishMakingConnection(int numRead, int numWrite)
{
    // Call the progress callback.
    CallProgressCallback(0);

    TRY
    {
        //
        // Accept the sockets that were created and create SocketConnection
        // objects for them.
        //
        nReadConnections = 0;
        if(numRead > 0)
        {
            readConnections = new Connection*[numRead];
            for(int i = 0; i < numRead; ++i)
            {
                int descriptor = AcceptSocket();
                readConnections[nReadConnections] = new SocketConnection(descriptor);
                ++nReadConnections;
            }
        }

        nWriteConnections = 0;
        if(numWrite > 0)
        {
            writeConnections = new Connection*[numWrite];
            for(int i = 0; i < numWrite; ++i)
            {
                int descriptor = AcceptSocket();
                writeConnections[nWriteConnections] = new SocketConnection(descriptor);
                ++nWriteConnections;
            }
        }

        //
        // Now that the sockets are open, exchange type representation info
        // and set that info in the socket connections.
        //
        ExchangeTypeRepresentations();
    }
    CATCHALL(...)
    {
        // Call the progress callback and tell it to end.
        CallProgressCallback(2);
        CloseListenSocket();

        RETHROW;
    }
    ENDTRY

    // Call the progress callback.
    CallProgressCallback(2);
    CloseListenSocket();
}

// ****************************************************************************
// Method: RemoteProcess::ExchangeTypeRepresentations
//
// Purpose: 
//   Exchanges the machine's type representation with that of the
//   machine on the other end of the sockets. It then puts that
//   type representation into the write sockets. Conversion is disabled
//   if the type representations are the same.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:25:19 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Apr 25 11:33:58 PDT 2001
//   Moved the specifics of the message header into the CommunicationHeader
//   class.
//
//   Jeremy Meredith, Fri Apr 27 15:30:29 PDT 2001
//   Added CouldNotConnectException.
//
//   Brad Whitlock, Mon Mar 25 14:13:34 PST 2002
//   Made it pass a connection to the communication header object.
//
// ****************************************************************************

void
RemoteProcess::ExchangeTypeRepresentations()
{
    // We can only exchange type representations if we opened
    // a read and a write socket.
    if(nReadConnections > 0 && nWriteConnections > 0)
    {
        TypeRepresentation  local;
        CommunicationHeader header;

        // Create a second key that will pass over the socket. The remote
        // process must pass this key back too.
        std::string socketKey(header.CreateRandomKey());

        // Send the local component and platform information.
        header.WriteHeader(writeConnections[0], VERSION,
                           "", socketKey);

        bool throwVersionException = false;
        bool throwConnectException = false;
        bool throwSecurityTokenException = false;
        TRY
        {
            // Set the security key that must be matched by the remote
            // process.
            header.SetSecurityKey(securityKey);

            // Read the remote component and platform information.
            header.ReadHeader(readConnections[0], VERSION);
        }
        CATCH(IncompatibleVersionException)
        {
            throwVersionException = true;
        }
        CATCH(CouldNotConnectException)
        {
            throwConnectException = true;
        }
        CATCH(IncompatibleSecurityTokenException)
        {
            throwSecurityTokenException = true;
        }
        ENDTRY

        // Now that we have the type representation for the remote machine,
        // if it is the same as the local type representation, turn off 
        // conversion in the write connections. Otherwise, set it into all
        // of the write sockets.
        if(local == header.GetTypeRepresentation())
        {
            for(int i = 0; i < nReadConnections; ++i)
                readConnections[i]->EnableConversion(false);
        }
        else
        {
            for(int i = 0; i < nReadConnections; ++i)
            {
                readConnections[i]->SetDestinationFormat(
                    header.GetTypeRepresentation());
            }
        }

        // If an IncompatibleVersionException was thrown, we caught it so
        // we could send this side's communication header. Now that we've done
        // that, we can re-throw the exception.
        if(throwVersionException)
        {
            EXCEPTION0(IncompatibleVersionException);
        }
        if(throwConnectException)
        {
            EXCEPTION0(CouldNotConnectException);
        }
        if(throwSecurityTokenException)
        {
            EXCEPTION0(IncompatibleSecurityTokenException);
        }
    }
}

// ****************************************************************************
// Method: RemoteProcess::SecureShell
//
// Purpose: 
//   Returns the name of the SSH program being used to launch remote components.
//
// Returns:    A string containing the name of the SSH program used to
//             launch remote components.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 12:49:52 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

const char *
RemoteProcess::SecureShell() const
{
    const char *ssh =  (const char *)getenv("VISITSSH");
    if(ssh == 0)
#if defined(_WIN32)
        ssh = "qtssh.exe";
#else
        ssh = "ssh";
#endif
    return ssh;
}

// ****************************************************************************
// Method: RemoteProcess::SecureShellArgs
//
// Purpose: 
//   Returns a string that contains any arguments that should be passed to
//   the SSH program.
//
// Returns:    NULL or a string containing arguments to pass to SSH.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 12:49:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

const char *
RemoteProcess::SecureShellArgs() const
{
    return (const char *)getenv("VISITSSHARGS");
}

// ****************************************************************************
// Method: RemoteProcess::CreateCommandLine
//
// Purpose: 
//   Creates the command line for the process that we want to launch.
//
// Arguments:
//   args     : The return string vector.
//   rHost    : The name of the host that we want to launch the process on.
//   numRead  : The number of read sockets to create.
//   numWrite : The number of write sockets to create.
//   local    : Prevents ssh arguments from being added.
//
// Returns:   The command line in the args vector. 
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 13:05:24 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::CreateCommandLine(stringVector &args, const std::string &rHost,
    int numRead, int numWrite, bool local) const
{
    //
    // If the host is not local, then add some ssh arguments to the
    // front of the command line argument list.
    //
    if(!HostIsLocal(rHost) && !local)
    {
#if defined(_WIN32)
        // If the path to secure shell contains spaces, enclose the program
        // name in quotes. This is mostly so the ssh program runs correctly
        //  on Windows when VisIt is installed into a path containing spaces.
        std::string ssh(SecureShell());
        if(ssh.find(" ") != std::string::npos)
        {
            std::string q("\"");
            args.push_back(q + ssh + q);
        }
        else
            args.push_back(SecureShell());
#else
        args.push_back(SecureShell());
#endif

        // Set any optional ssh arguments into the command line.
        const char *sshArgs = SecureShellArgs();
        if(sshArgs)
            args.push_back(sshArgs);

        // Set the username.
        if(remoteUserName != std::string("notset"))
        {
             args.push_back("-l");
             args.push_back(remoteUserName);
        }
    
        // Set the name of the host to run on.
        args.push_back(remoteHost.c_str());
    }

    //
    // Add the name of the remote program to run 
    //
    args.push_back(remoteProgram);

    //
    // Add the version so that we try to launch a version that is compatible
    // with this version.
    //
    args.push_back("-v");
    args.push_back(VERSION);

    //
    // Add the program's additional arguments.
    //
    for(int i = 0; i < argList.size(); ++i)
        args.push_back(argList[i]);

    //
    // Add the local hostname and the ports we'll be talking on.
    //
    args.push_back("-host");
    args.push_back(localHost.c_str());

    args.push_back("-port");
    char tmp[20];
    sprintf(tmp, "%d", listenPortNum);
    args.push_back(tmp);

    args.push_back("-nread");
    sprintf(tmp, "%d", numRead);
    args.push_back(tmp);

    args.push_back("-nwrite");
    sprintf(tmp, "%d", numWrite);
    args.push_back(tmp);

    //
    // Add the security key
    //
    args.push_back("-key");
    args.push_back(securityKey);
}

// ****************************************************************************
// Method: RemoteProcess::LaunchRemote
//
// Purpose: 
//   Launches a process on a remote machine using ssh.
//
// Arguments:
//   args : The command line arguments of the process to launch.
//    
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 13:24:29 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//    Changed it to allow more than one read/write socket.   
//
//    Hank Childs, Mon Oct 16 10:59:22 PDT 2000
//    Fixed memory leak.
//
//    Jeremy Meredith, Fri Apr 27 15:31:06 PDT 2001
//    Switched to using PTY's when available.  Added callback for
//    authentication.  Added CouldNotConnectException for failure condition.
//    Removed the -f and -n flags from ssh as well as the close(0).  These
//    were preventing us reading status back from the remote process.
//
//    Brad Whitlock, Tue Mar 19 15:59:33 PST 2002
//    Made it work on MS Windows.
//
//    Brad Whitlock, Mon Aug 26 12:32:03 PDT 2002
//    It didn't really work on MS Windows - now it does.
//
//    Jeremy Meredith, Tue Oct 22 15:02:50 PDT 2002
//    Moved the initialization of the signal handler into pty_fork or
//    just above fork. It must be the last thing called before fork(),
//    or at least after grantpt, due to some grantpt vileness.
//
//    Brad Whitlock, Mon Dec 16 11:54:29 PDT 2002
//    I added code to send a random number token as an argument to the
//    new process. I also added code to close the listen socket in the case
//    that a CouldNotConnectException is thrown.
//
//    Brad Whitlock, Thu Apr 24 07:22:24 PDT 2003
//    I made it pass the version as a command line argument.
//
//    Brad Whitlock, Mon May 5 13:10:52 PST 2003
//    I moved large portions of the command line creation code elsewhere.
//
//    Jeremy Meredith, Thu Jul  3 15:01:25 PDT 2003
//    Allowed disabling of PTYs even when they are available.
//
// ****************************************************************************

void
RemoteProcess::LaunchRemote(const stringVector &args)
{
    // 
    // Create the parameters for the exec
    //
    int  argc = 0;
    char **argv = CreateSplitCommandLine(args, argc);

    //
    // Start the program on the remote host.
    // 
#if defined(_WIN32)
    // Start the program using the WIN32 _spawnvp function.
    remoteProgramPid = _spawnvp(_P_NOWAIT, SecureShell(), argv);
#else
    // Start the program in UNIX
#ifdef USE_PTY
    int ptyFileDescriptor;
    if (!disablePTY)
    {
        // we will tell pty_fork to set up the signal handler for us, because
        // this call must come after the grantpt call inside pty_fork()
        remoteProgramPid = pty_fork(ptyFileDescriptor, catch_dead_child);
    }
    else
    {
        signal(SIGCHLD, catch_dead_child);
        remoteProgramPid = fork();
    }
#else
    signal(SIGCHLD, catch_dead_child);
    remoteProgramPid = fork();
#endif
    switch (remoteProgramPid)
    {
    case -1:
        // Could not fork.
        exit(-1);
        break;
    case 0:
        // Do not close stdout, stderr, because ssh will fail if
        // not set up exactly right and we want to see the error
        // messages.
        for (int k = 3 ; k < 32 ; ++k)
        {
            close(k);
        }
        execvp(SecureShell(), argv);
        close(0); close(1); close(2);
        exit(-1);
        break;   // OCD
    default:
        break;
    }

#ifdef USE_PTY
    if (!disablePTY && getAuthentication)
    {
        TRY
        {
            (*getAuthentication)(remoteUserName.c_str(), remoteHost.c_str(),
                                 ptyFileDescriptor);
        }
        CATCH(CouldNotConnectException)
        {
            // Clean up memory
            DestroySplitCommandLine(argv, argc);

            // Close the listen socket.
            CloseListenSocket();

            RETHROW;
        }
        ENDTRY
    }
#endif
#endif

    // Clean up memory
    DestroySplitCommandLine(argv, argc);
}

// ****************************************************************************
// Method: RemoteProcess::LaunchLocal
//
// Purpose: 
//   Launches a process on the machine we're running on.
//
// Arguments:
//   args : The command line arguments for the process we're launching.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 13:24:29 PST 2000
//
// Modifications:
//   Hank Childs, Mon Oct 16 10:54:36 PDT 2000
//   Fixed memory leak.
//
//   Brad Whitlock, Tue Mar 19 16:00:26 PST 2002
//   Made it work on MS Windows.
//
//   Jeremy Meredith, Tue Oct 22 15:02:50 PDT 2002
//   Moved the initialization of the signal handler into here so it
//   is the last thing executed before fork.
//
//   Brad Whitlock, Mon Dec 16 14:29:32 PST 2002
//   I added a security key.
//
//   Jeremy Meredith, Mon Mar 24 12:47:50 PST 2003
//   Closed STDIN before launching local processes.  Sometimes when
//   launching parallel engines, the CLI would start to drop lines
//   or characters, apparently because of competition for stdin.
//   I did not *yet* close STDIN for remote processes because (1) it is
//   currently unknown if there is even a problem after using ssh, and
//   (2) it may break ssh launching itself.  More investigation required.
//
//   Brad Whitlock, Thu Apr 24 07:24:50 PDT 2003
//   I added code to pass the desired version on the command line.
//
//   Brad Whitlock, Mon May 5 13:28:17 PST 2003
//   I made the command line be passed in instead of assembling it in
//   this method.
//
// ****************************************************************************

void
RemoteProcess::LaunchLocal(const stringVector &args)
{
    // 
    // Create the parameters for the exec
    //
    int  argc = 0;
    char **argv = CreateSplitCommandLine(args, argc);

    //
    // Start the program on the remote host.
    //
#if defined(_WIN32)
    // Do it the WIN32 way where we use the _spawnvp system call.
    remoteProgramPid = _spawnvp(_P_NOWAIT, remoteProgram.c_str(), argv);
#else
    // Watch for a remote process who died
    signal(SIGCHLD, catch_dead_child);

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
        execvp(remoteProgram.c_str(), argv);
        exit(-1);
        break;   // OCD
    default:
        break;
    }
#endif

    // Clean up memory
    DestroySplitCommandLine(argv, argc);
}

// ****************************************************************************
// Method: RemoteProcess::CreateSplitCommandLine
//
// Purpose: 
//   Splits a string vector into a char ** array that can be passed to
//   exec type functions.
//
// Arguments:
//   args : The input string vector.
//   argc : The number of strings in the return array.
//
// Returns:    An array of char strings.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 13:20:07 PST 2003
//
// Modifications:
//   
// ****************************************************************************

char **
RemoteProcess::CreateSplitCommandLine(const stringVector &args, int &argc) const
{
    argc = args.size();
    char **retval = new char*[argc + 1];
    for(int i = 0; i < argc; ++i)
        retval[i] = StrDup(args[i]);
    retval[argc] = NULL;

    return retval;
}

// ****************************************************************************
// Method: RemoteProcess::DestroySplitCommandLine
//
// Purpose: 
//   Destroys an array of char strings.
//
// Arguments:
//   args : The array to destroy.
//   argc : The number of strings in the array.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 5 13:23:26 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::DestroySplitCommandLine(char **args, int argc) const
{
    for (int i = 0 ; i < argc ; ++i)
        delete [] args[i];
    delete [] args;
}

// ****************************************************************************
// Method: RemoteProcess::StrDup
//
// Purpose: 
//   Duplicates a string. This function was written to avoid memory
//   mismanagement problems created when using strdup with delete[].
//
// Arguments:
//   str : The string to duplicate.
// Returns:    
//   A pointer to a new copy of the string.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 25 16:27:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

char *
RemoteProcess::StrDup(const std::string &str) const
{
    if(str.size() == 0)
        return NULL;

    int len = str.size();
    char *newStr = new char[len + 1];
    for(int i = 0; i < len; ++i)
        newStr[i] = str[i];
    newStr[len] = '\0';

    return newStr;
}

// ****************************************************************************
//  Method:  RemoteProcess::SetAuthenticationCallback
//
//  Purpose:
//    Set a callback for automating authentication for remote processses
//
//  Arguments:
//    callback:  the callback function
//
//  Programmer:  Jeremy Meredith
//  Creation:    April 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Aug 29 10:50:53 PDT 2002
//    I made it use const char* instead of string.
//
// ****************************************************************************

void
RemoteProcess::SetAuthenticationCallback(void (*callback)(const char *, const char *, int))
{
    getAuthentication = callback;
}

// ****************************************************************************
// Method: RemoteProcess::SetProgressCallback
//
// Purpose: 
//   Sets the progress callback that will be called as a process is launched.
//
// Arguments:
//   callback : The callback function.
//   data     : Data for the callback function.
//
// Note:       The progress callback is only called when we have threads.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 26 17:11:57 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
RemoteProcess::SetProgressCallback(bool (*callback)(void *, int), void *data)
{
    progressCallback = callback;
    progressCallbackData = data;
}
