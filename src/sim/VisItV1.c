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

#include "VisItV1.h"

/* ****************************************************************************
//  File:  VisItV1.C
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
//  Creation:    April  4, 2005
//
// ***************************************************************************/


// ----------------------------------------------------------------------------

#ifdef PARALLEL
#include <mpi.h>
static int ifparallel=1;
#else
static int ifparallel=0;
#endif

extern int par_rank;
extern int par_size;

static int BroadcastInt(int *value, int sender)
{
#ifdef PARALLEL
    return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
#else
    return 0;
#endif
}

static int BroadcastString(char *str, int len, int sender)
{
#ifdef PARALLEL
    return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
#else
    return 0;
#endif
}

// ----------------------------------------------------------------------------

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
void  (*v_set_slave_process_callback)(void(*)()) = NULL;
void  (*v_set_command_callback)(void*,void(*)(const char*,int,float,const char*))
                                                                        = NULL;

void *v_engine = NULL;
char  v_host[256] = "";
char  v_port[256] = "";
char  v_key[256] = "";

char simulationFileName[1024];

char securityKey[17];

char localhost[256];
int listenPort = -1;
int listenSock = -1;
struct sockaddr_in sockin;

int   v_argc = 0;
char *v_argv[100];

int engineinputdescriptor = -1;

void *dl_handle;

static void CreateRandomSecurityKey()
{
    int len = 8;
    int i;
    securityKey[0] = '\0';
#if defined(_WIN32)
    srand((unsigned)time(0));
#else
    srand48((long)(time(0)));
#endif
    for (i=0; i<len; i++)
    {
        char str[3];
#if defined(_WIN32)
        double d = (double)(rand()) / (double)(RAND_MAX);
        sprintf(str, "%02x", (int)(d * 255.));
#else
        sprintf(str, "%02x", (int)(lrand48() % 256));
#endif
        strcat(securityKey, str);
    }
}

int  VisItGetEngineSocket(void)
{
    return engineinputdescriptor;
}

int  VisItGetListenSocket(void)
{
    return listenSock;
}

void VisItDisconnect(void)
{
    printf("processor %d disconnecting\n", par_rank);
    v_disconnect();
    engineinputdescriptor = -1;
    v_engine = 0;
}

static void GetConnectionParameters(int desc)
{
    char buf[2000] = "";
    int i;

    if (par_rank == 0)
    {
        char *tbuf = buf;
        char *ptr = buf;
        int done = 0;
        int n;

        v_argc = 0;

        printf("Engine launch command:");
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
            printf("%s ", v_argv[v_argc]);
            v_argc++;
            tbuf = tmp+1;
        }
        printf("\n");

        if (ifparallel)
        {
            BroadcastInt(&v_argc, 0);
            for (i = 0 ; i < v_argc; i++)
            {
                int len = strlen(v_argv[i]);
                BroadcastInt(&len, 0);
                BroadcastString(v_argv[i], len+1, 0);
            }
        }
    }
    else
    {
        BroadcastInt(&v_argc, 0);
        for (i = 0 ; i < v_argc; i++)
        {
            int len;
            BroadcastInt(&len, 0);
            BroadcastString(buf, len+1, 0);
            v_argv[i] = strdup(buf);
            printf("process %d: argv[%d] = %s\n",par_rank, i, v_argv[i]);
        }
    }
}

static int CreateEngineAndConnectToViewer(void)
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
        VisItDisconnect();
        return FALSE;
    }

    if (!v_connectviewer(v_engine, v_argc, v_argv))
    {
        fprintf(stderr, "Could not initialize engine\n");
        VisItDisconnect();
        return FALSE;
    }

    return TRUE;
}

static void GetLocalhostName(void)
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

static int StartListening(void)
{
    int portFound = FALSE;
    int on = 1;
    int err;

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
        err = fcntl(listenSock, F_SETFL, O_NONBLOCK);
        err = bind(listenSock, (struct sockaddr *)&sockin, sizeof(sockin));
        if (err)
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

    err = listen(listenSock, 5);

    return TRUE;
}

static int AcceptConnection(void)
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

static const char *GetHomeDirectory(void)
{
    struct passwd *users_passwd_entry = NULL;
    users_passwd_entry = getpwuid(getuid());
    return users_passwd_entry->pw_dir;
}

static void EnsureSimulationDirectoryExists(void)
{
    char str[1024];
    snprintf(str, 1024, "%s/.visit", GetHomeDirectory());
    mkdir(str, 7*64 + 7*8 + 7);
    snprintf(str, 1024, "%s/.visit/simulations", GetHomeDirectory());
    mkdir(str, 7*64 + 7*8 + 7);
}

static void RemoveSimFile(void)
{
    unlink(simulationFileName);
}

void VisItInitializeSocketAndDumpSimFile(char *name,
                                         char *comment,
                                         char *path,
                                         char *inputfile)
{
    FILE *file;

    EnsureSimulationDirectoryExists();
    CreateRandomSecurityKey();
    
    snprintf(simulationFileName, 255, "%s/.visit/simulations/%012d.%s.sim1",
             GetHomeDirectory(), (int)time(NULL), name);

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
    fprintf(file, "key %s\n", securityKey);
    if (path)
        fprintf(file, "path %s\n", path);
    if (inputfile)
        fprintf(file, "inputfile %s\n", inputfile);
    if (comment)
        fprintf(file, "comment %s\n", comment);

    fclose(file);
}

static int LoadVisItLibrary(void)
{
#if 1
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

    if (ifparallel)
    {
        printf("processor %d attempting to open parallel\n",par_rank);
        dl_handle = dlopen("libvisitengine_par.so", RTLD_NOW | RTLD_GLOBAL);
        printf("processor %d opened parallel successfully\n",par_rank);
    }
    else
    {
        printf("processor %d attempting to open serial\n",par_rank);
        dl_handle = dlopen("libvisitengine_ser.so", RTLD_NOW | RTLD_GLOBAL);
        printf("processor %d opened serial successfully\n",par_rank);
    }

    if (!dl_handle)
    {
        fprintf(stderr, "couldn't dlopen libewrap.so: %s\n", dlerror());
        return FALSE;
    }

    /* get symbols */
    v_getengine = (void *(*)())dlsym(dl_handle, "get_engine");
    if (!v_getengine) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_getdescriptor = (int (*)(void *))dlsym(dl_handle, "get_descriptor");
    if (!v_getdescriptor) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_processinput = (int (*)(void *))dlsym(dl_handle, "process_input");
    if (!v_processinput) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_initialize = (int (*)(void *, int, char **))dlsym(dl_handle, "initialize");
    if (!v_initialize) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_connectviewer = (int (*)(void *, int, char **))dlsym(dl_handle, "connect_to_viewer");
    if (!v_connectviewer) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_time_step_changed = (void (*)(void *))dlsym(dl_handle, "time_step_changed");
    if (!v_time_step_changed) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_disconnect = (void (*)())dlsym(dl_handle, "disconnect");
    if (!v_disconnect) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_set_slave_process_callback = (void (*)(void (*)()))dlsym(dl_handle, "set_slave_process_callback");
    if (!v_set_slave_process_callback) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }

    v_set_command_callback = (void (*)(void*,void (*)(const char*,int,float,const char*)))dlsym(dl_handle, "set_command_callback");
    if (!v_set_command_callback) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); dl_handle = NULL; return FALSE; }
#endif

    return TRUE;
}

int VisItAttemptToCompleteConnection(void)
{
    int socket;

    /* wait for a connection -- only process 0 does this */
    if (par_rank == 0)
    {
        socket = AcceptConnection();

        if (socket < 0)
            return FALSE;
    }

    /* get the connection parameters */
    GetConnectionParameters(socket);

    /* load the library */
    if (LoadVisItLibrary() == 0)
        return FALSE;

    /* connect to the viewer */
    if (CreateEngineAndConnectToViewer() == 0)
        return FALSE;

    /* get the socket for listening from the viewer */
    if (par_rank == 0)
    {
        engineinputdescriptor = v_getdescriptor(v_engine);
    }

    return TRUE;
}

void VisItSetSlaveProcessCallback(void (*spic)())
{
    v_set_slave_process_callback(spic);
}


void VisItSetCommandCallback(void (*scc)(const char*,int,float,const char*))
{
    v_set_command_callback(v_engine,scc);
}

int VisItProcessEngineCommand(void)
{
    return v_processinput(v_engine);
}

void VisItTimeStepChanged(void)
{
    // Make sure the function exists before using it.
    if (v_time_step_changed)
    {
        v_time_step_changed(v_engine);
    }
}

void VisItAddLibraryPaths(int argc, char *argv[])
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

    /*
    buff = malloc(10000);
    sprintf(buff, "VISITPLUGINDIR=.:../plugins");
    putenv(buff);
    */

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

void VisItSetupEnvironment(void)
{
    char *envoutput = malloc(10000);
    char *envoutput_ptr;
    FILE *file;
    int n;
    int i;
    int done;

    // REMOVE ME!
    //return;

    printf("BEFORE: processor %d has VISITPLUGINDIR=%s\n",par_rank,getenv("VISITPLUGINDIR") ? getenv("VISITPLUGINDIR") : "unset" );

    /* VisIt can tell us what variables to set! */
    /* (redirect stderr so it won't complain if it can't find visit) */
    file = popen("visit -env 2>/dev/null", "r");
    envoutput_ptr = envoutput;
    while ((n = read(fileno(file), envoutput_ptr, PIPE_BUF)) > 0)
    {
        envoutput_ptr += n;
    }
    printf("proc %d: read from default visit env: %s\n", par_rank, envoutput);
    pclose(file);

    /* If there was no output, then visit wasn't in their path */
    if (envoutput_ptr == envoutput)
    {
        file = popen("/usr/gapps/visit/bin/visit -env 2>/dev/null", "r");
        envoutput_ptr = envoutput;
        while ((n = read(fileno(file), envoutput_ptr, PIPE_BUF)) > 0)
        {
            envoutput_ptr += n;
        }
        pclose(file);
    }
    printf("proc %d: read from default or gapps visit env: %s\n", par_rank, envoutput);

    /* Do a bunch of putenv calls; it should already be formatted correctly */
    envoutput_ptr = envoutput;
    while (envoutput_ptr[0]!='\0')
    {
        int i = 0;
        while (envoutput_ptr[i]!='\n')
            i++;
        envoutput_ptr[i] = '\0';
        printf("processor %d adding: %s\n",par_rank, envoutput_ptr);
        putenv(envoutput_ptr);
        envoutput_ptr += i+1;
    }
    // free(envoutput); <--- NO!  Do Not Free!  Bad Bad Bad Bad Bad!

    printf("AFTER: processor %d has VISITPLUGINDIR=%s\n",par_rank,getenv("VISITPLUGINDIR"));
}
