#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "simulation.h"

// ****************************************************************************
//  File:  simulation.h
//
//  Purpose:
//    Abstraction of VisIt Engine wrapper library.  Handles the
//    grunt work of actually connecting to visit that must be done
//    outside of the VisItEngine DLL.
//       1) setting up a non-blocking listen socket
//       2) writing a .sim file
//       3) opening the VisItEngine .so and retrieving the functions from it
//       4) accepting an incoming socket connection
//       5) removing the .sim file when the program exits
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************


#define INITIAL_PORT_NUMBER 5600
#define TRUE 1
#define FALSE 0

void *(*v_getengine)() = NULL;
int   (*v_getdescriptor)(void*) = NULL;
int   (*v_processinput)(void*) = NULL;
int   (*v_initialize)(void*,int,char**) = NULL;
int   (*v_connectviewer)(void*,int,char**) = NULL;
void  (*v_time_step_changed)(void*) = NULL;
void  (*v_disconnect)() = NULL;
void *v_engine = NULL;
char  v_host[256] = "";
char  v_port[256] = "";
char  v_key[256] = "";

char simulationFileName[1024];

char localhost[256];
int listenPort;
extern int listenSock;
struct sockaddr_in sockin;

int   v_argc = 0;
char *v_argv[100];

extern int engineinputdescriptor;

void *dl_handle;


void Disconnect()
{
    v_disconnect();
    engineinputdescriptor = -1;
    v_engine = 0;
}

void GetConnectionParameters(int desc)
{
    char buf[200] = "";
    char *tbuf = buf;
    char *ptr = buf;
    int done = 0;
    int n;

    v_argc = 0;

    fprintf(stderr, "Engine launch command:");
    while (!done)
    {
        char *tmp = strstr(tbuf, "\n");
        while (!tmp)
        {
            n = recv(desc, ptr, 2000, 0);
            ptr += n;
            *ptr = 0;
            tmp = strstr(tbuf, "\n");
                
        }

        if (tbuf == tmp)
        {
            break;
        }

        *tmp = 0;
        v_argv[v_argc] = strdup(tbuf);
        fprintf(stderr, "%s ", v_argv[v_argc]);
        v_argc++;
        tbuf = tmp+1;
    }
    fprintf(stderr, "\n");
}

int CreateEngineAndConnectToViewer()
{
    /* get the engine */
    v_engine = v_getengine();
    if (!v_engine)
    {
        fprintf(stderr, "Could not initialize engine\n");
        return FALSE;
    }

    if (!v_initialize(v_engine, v_argc, v_argv))
    {
        fprintf(stderr, "Could not initialize engine\n");
        Disconnect();
        return FALSE;
    }

    if (!v_connectviewer(v_engine, v_argc, v_argv))
    {
        fprintf(stderr, "Could not initialize engine\n");
        Disconnect();
        return FALSE;
    }

    return TRUE;
}

void GetLocalhostName()
{
    char localhostStr[256];
    struct hostent *localhostEnt = NULL;
    if (gethostname(localhostStr, 256) == -1)
    {
        // Couldn't get the hostname, it's probably invalid
        fprintf(stderr,"couldn't get host name\n");
        return;
    }

    localhostEnt = gethostbyname(localhostStr);
    if (localhostEnt == NULL)
    {
        // Couldn't get the full host entry; it's probably invalid
        fprintf(stderr,"couldn't get host name entry\n");
        return;
    }
    sprintf(localhost, localhostEnt->h_name);
}

int StartListening()
{
    int portFound = FALSE;
    int on = 1;

    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock < 0)
    {
        // Cannot open a socket.
        return FALSE;
    }

    //
    // Look for a port that can be used.
    //
    sockin.sin_family = AF_INET;
    sockin.sin_addr.s_addr = htonl(INADDR_ANY);
    listenPort = INITIAL_PORT_NUMBER;
    while (!portFound && listenPort < 32767)
    {
        sockin.sin_port = htons(listenPort);
#if !defined(_WIN32)
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
#endif

        // Set the listen socket to non-blocking
        fcntl(listenSock, F_SETFL, O_NONBLOCK);
        if (bind(listenSock, (struct sockaddr *)&sockin, sizeof(sockin)) < 0)
        {
            listenPort++;
        }
        else
        {
            portFound = TRUE;
        }
    }
    if (!portFound)
    {
        // Cannot find unused port.
        return FALSE;
    }

    listen(listenSock, 5);

    return TRUE;
}

int AcceptConnection()
{
    int desc = -1;
    int opt = 1;

    // Wait for the socket to become available on the other side.
    do
    {
#ifdef HAVE_SOCKLEN_T
        socklen_t len;
#else
        int len;
#endif
        len = sizeof(struct sockaddr);
        desc = accept(listenSock, (struct sockaddr *)&sockin, &len);
    }
    while (desc == -1 && errno != EWOULDBLOCK);

    // Disable Nagle algorithm.
#if defined(_WIN32)
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&opt, sizeof(int));
#else
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    return desc;
}

const char *GetHomeDirectory()
{
    struct passwd *users_passwd_entry = NULL;
    users_passwd_entry = getpwuid(getuid());
    return users_passwd_entry->pw_dir;
}

void EnsureSimulationDirectoryExists()
{
    char str[1024];
    snprintf(str, 1024, "%s/.visit", GetHomeDirectory());
    mkdir(str, 7*64 + 7*8 + 7);
    snprintf(str, 1024, "%s/.visit/simulations", GetHomeDirectory());
    mkdir(str, 7*64 + 7*8 + 7);
}

void RemoveSimFile()
{
    unlink(simulationFileName);
}

void InitializeSocketAndDumpSimFile(const char *name)
{
    FILE *file;

    EnsureSimulationDirectoryExists();
    
    snprintf(simulationFileName, 255, "%s/.visit/simulations/%012d.%s.sim",
             GetHomeDirectory(), time(NULL), name);

    file = fopen(simulationFileName, "wt");
    if (!file)
    {
        fprintf(stderr,"COULDN'T CREATE FILE '%s'\n",simulationFileName);
    }

    atexit(RemoveSimFile);

    GetLocalhostName();
    StartListening();

    fprintf(file, "host %s\n", localhost);
    fprintf(file, "port %d\n", listenPort);

    fclose(file);
}

int LoadVisItLibrary()
{
#ifdef HACKHACK
    /* It is necessary right now to re-open the .so.  Without doing so,
       something is hanging, and I have not yet diagnosed what.  Things
       work fine if you just reopen the .so, so that is the current
       workaround.  However, this section of code should be re-enabled
       ASAP. */
    if (dl_handle)
        return;
#endif

    /* load library */

#ifdef PARALLEL
    dl_handle = dlopen("libvisitengine_par.so", RTLD_LAZY | RTLD_GLOBAL);
#else
    dl_handle = dlopen("libvisitengine_ser.so", RTLD_LAZY | RTLD_GLOBAL);
#endif

    if (!dl_handle)
    {
        fprintf(stderr, "couldn't dlopen libewrap.so: %s\n", dlerror());
        return FALSE;
    }

    /* get symbols */
    v_getengine = dlsym(dl_handle, "get_engine");
    if (!v_getengine) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_getdescriptor = dlsym(dl_handle, "get_descriptor");
    if (!v_getdescriptor) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_processinput = dlsym(dl_handle, "process_input");
    if (!v_processinput) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_initialize = dlsym(dl_handle, "initialize");
    if (!v_initialize) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_connectviewer = dlsym(dl_handle, "connect_to_viewer");
    if (!v_connectviewer) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_time_step_changed = dlsym(dl_handle, "time_step_changed");
    if (!v_time_step_changed) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_disconnect = dlsym(dl_handle, "disconnect");
    if (!v_disconnect) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    return TRUE;
}

int AttemptToCompleteConnection()
{
    int socket;

    /* wait for a connection */
    socket = AcceptConnection();

    if (socket < 0)
        return FALSE;

    /* get the connection parameters */
    GetConnectionParameters(socket);

    /* load the library */
    if (LoadVisItLibrary() == 0)
        return FALSE;

    /* connect to the viewer */
    if (CreateEngineAndConnectToViewer() == 0)
        return FALSE;

    /* get the socket for listening from the viewer */
    engineinputdescriptor = v_getdescriptor(v_engine);

    return TRUE;
}

void ProcessEngineCommand()
{
    if (!v_processinput(v_engine))
    {
        Disconnect();
    }
}

void TimeStepChanged()
{
    v_time_step_changed(v_engine);
}

void AddVisItLibraryPaths(int argc, char *argv[])
{
    int i;
    char *buff;
    char **argv2;

    if (getenv("VISIT_PATHS_SET"))
        return;

    /*if (getenv("LD_LIBRARY_PATH")) printf("ld_library_path=%s\n", getenv("LD_LIBRARY_PATH")); else printf("ld_library_path=(null)\n");*/
    buff = malloc(10000);
    if (getenv("LD_LIBRARY_PATH"))
        sprintf(buff, "LD_LIBRARY_PATH=.:../lib:%s", getenv("LD_LIBRARY_PATH"));
    else
        sprintf(buff, "LD_LIBRARY_PATH=.:../lib");
    putenv(buff);

    buff = malloc(10000);
    sprintf(buff, "VISITPLUGINDIR=.:../plugins");
    putenv(buff);

    buff = strdup("VISIT_PATHS_SET=1");
    putenv(buff);

    argv2 = malloc(argc * sizeof(char*));
    for (i=0; i<argc; i++)
    {
        argv2[i] = strdup(argv[i]);
    }
    execv(argv[0],argv2);
    fprintf(stderr, "exec failed\n");
    exit(-1);
}
