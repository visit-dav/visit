#include <dlfcn.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define INITIAL_PORT_NUMBER 5600
#define TRUE 1
#define FALSE 0

int consoleinputdescriptor = 0;
int engineinputdescriptor = -1;

int runflag = 0;
int quitflag = 0;

int cycle = 0;

void *(*v_getengine)() = NULL;
int   (*v_getdescriptor)(void*) = NULL;
void  (*v_processinput)(void*) = NULL;
void  (*v_initialize)(void*,int,char**) = NULL;
void  (*v_connectviewer)(void*,int,char**) = NULL;
void *v_engine = NULL;
char  v_host[256] = "";
char  v_port[256] = "";
char  v_key[256] = "";

char localhost[256];
int listenPort;
int listenSock;
struct sockaddr_in sockin;

int p_nx = 10;
int p_ny = 10;
int p_nz = 10;
double *p_xcoords;
double *p_ycoords;
double *p_zcoords;
double *p_zvalues;
double *p_nvalues;

void InitializeVariables()
{
    int i;
    p_xcoords = malloc(sizeof(double) * p_nx);
    p_ycoords = malloc(sizeof(double) * p_ny);
    p_zcoords = malloc(sizeof(double) * p_nz);
    p_zvalues = malloc(sizeof(double) * (p_nx-1)*(p_ny-1)*(p_nz-1));
    p_nvalues = malloc(sizeof(double) * p_nx*p_ny*p_nz);
    for (i=0; i<p_nx; i++)
    {
        p_xcoords[i] = i*2.0;
    }
    for (i=0; i<p_ny; i++)
    {
        p_ycoords[i] = i*1.5;
    }
    for (i=0; i<p_nz; i++)
    {
        p_zcoords[i] = i*1.0;
    }
    for (i=0; i<(p_nx-1)*(p_ny-1)*(p_nz-1); i++)
    {
        p_zvalues[i] = i*1.0;
    }
    for (i=0; i<p_nx*p_ny*p_nz; i++)
    {
        p_nvalues[i] = i*1.0;
    }
}

void ControlCHandler(int sig)
{
    if (runflag)
    {
        fprintf(stderr, "\n   pausing simulation.\n");
    }
    runflag = 0;
    signal(SIGINT, ControlCHandler);
}

void RunSingleCycle()
{
    int i;

    time_t starttime = time(NULL);
    while (time(NULL) == starttime)
    {
        // Wait
    }

    for (i=0; i<p_nx; i++)
    {
        p_xcoords[i] = i * 2.0 + (i*i*0.1 * (double)(cycle));
    }

    printf(" ... Finished cycle %d\n", cycle);
    cycle++;
}

void RunSimulation()
{
    runflag = 1;
    while (runflag)
    {
        RunSingleCycle();
    }
}

int   v_argc = 0;
char *v_argv[100];

void GetConnectionParameters(int desc)
{
#if 1
    char buf[200] = "";
    char *tbuf = buf;
    char *ptr = buf;
    int done = 0;
    int n;

    v_argc = 0;

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
            fprintf(stderr, "Got an empty string; stopping!\n");
            break;
        }

        *tmp = 0;
        v_argv[v_argc] = strdup(tbuf);
        fprintf(stderr, "Got string: %s\n", v_argv[v_argc]);
        v_argc++;
        tbuf = tmp+1;
    }
    
#else
    fprintf(stderr, "host: ");
    gets(v_host);

    fprintf(stderr, "port: ");
    gets(v_port);

    fprintf(stderr, "key : ");
    gets(v_key);
#endif
}

void ConnectToViewer()
{
#if 1
    v_initialize(v_engine, v_argc, v_argv);
    v_connectviewer(v_engine, v_argc, v_argv);
#else
    char *argv[10];
    int nargs = 9;

    argv[0] = strdup("engine");
    argv[1] = strdup("-host");
    argv[2] = strdup(v_host);
    argv[3] = strdup("-port");
    argv[4] = strdup(v_port);
    argv[5] = strdup("-key");
    argv[6] = strdup(v_key);
    argv[7] = strdup("-debug");
    argv[8] = strdup("5");
    argv[9] = NULL;

    v_initialize(v_engine, nargs, argv);
    v_connectviewer(v_engine, nargs, argv);
#endif
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

    fprintf(stderr,"AcceptConnection\n");
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
    while (desc == -1 && errno == EINTR);

    fprintf(stderr,"Finished Accept\n");

    // Disable Nagle algorithm.
#if defined(_WIN32)
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, (const char FAR*)&opt, sizeof(int));
#else
    setsockopt(desc, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(int));
#endif

    return desc;
}

void DumpSimFile()
{
    FILE *file = fopen("test.sim", "wt");

    GetLocalhostName();
    StartListening();
    fprintf(file, "host %s\n", localhost);
    fprintf(file, "port %d\n", listenPort);

    fclose(file);
}


void OpenVisit()
{
    int socket;

    /* load library */
    void *handle = dlopen("libewrap.so", RTLD_LAZY | RTLD_GLOBAL);

    if (!handle)
    {
        fprintf(stderr, "couldn't dlopen libewrap.so: %s\n", dlerror());
        return;
    }

    /* dump sim file */
    DumpSimFile();

    /* wait for a connection */
    socket = AcceptConnection();

    /* get symbols */
    v_getengine = dlsym(handle, "get_engine");
    if (!v_getengine) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_getdescriptor = dlsym(handle, "get_descriptor");
    if (!v_getdescriptor) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_processinput = dlsym(handle, "process_input");
    if (!v_processinput) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_initialize = dlsym(handle, "initialize");
    if (!v_initialize) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_connectviewer = dlsym(handle, "connect_to_viewer");
    if (!v_connectviewer) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    /* get the engine */
    v_engine = v_getengine();

    /* get the connection parameters */
    GetConnectionParameters(socket);

    /* connect to the viewer */
    ConnectToViewer();

    /* get the socket for listening from the viewer */
    engineinputdescriptor = v_getdescriptor(v_engine);
}

void ProcessConsoleCommand()
{
    // Read A Command
    char buff[10000];
    int iseof = (fgets(buff, 10000, stdin) == NULL);

    if (iseof)
    {
        sprintf(buff, "quit");
        printf("quit\n");
    }

    if (strlen(buff)>0 && buff[strlen(buff)-1] == '\n')
        buff[strlen(buff)-1] = '\0';

    if (!strcmp(buff, "quit"))
    {
        quitflag = 1;
    }
    else if (!strcmp(buff, "step"))
    {
        RunSingleCycle();
    }
    else if (!strcmp(buff, "run"))
    {
        RunSimulation();
    }
    else if (!strcmp(buff, "visit"))
    {
        OpenVisit();
    }
    else if (!strcmp(buff, ""))
    {
        // Do nothing on blank input.
    }
    else
    {
        fprintf(stderr, "Error: unknown command '%s'\n", buff);
    }
}

void ProcessEngineCommand()
{
    printf("Received engine command\n");
    v_processinput(v_engine);
}

void MainLoop()
{
    signal(SIGINT, ControlCHandler);

    fprintf(stderr, "command> ");
    fflush(stderr);

    while (!quitflag)
    {
        fd_set readSet;

        int maxdescriptor = (consoleinputdescriptor > engineinputdescriptor)
                           ? consoleinputdescriptor : engineinputdescriptor;

        int ret = 0;

        FD_ZERO(&readSet);

        FD_SET(consoleinputdescriptor, &readSet);
        if (engineinputdescriptor > 0)
            FD_SET(engineinputdescriptor, &readSet);
        ret = select(maxdescriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                     NULL);

        if (ret <= 0)
        {
            if (errno == EINTR)
            {
                fprintf(stderr, "\nInterrupted...... quitting\n");
                quitflag = 1;
            }
            else
            {
                fprintf(stderr, "Error in select at line %d\n",__LINE__);
                exit(1);
            }
        }
        else
        {
            if (FD_ISSET(consoleinputdescriptor, &readSet))
            {
                ProcessConsoleCommand();
                if (!quitflag)
                {
                    fprintf(stderr, "command> ");
                    fflush(stderr);
                }

            }
            else if (FD_ISSET(engineinputdescriptor, &readSet))
            {
                ProcessEngineCommand();
            }
            else
            {
                fprintf(stderr, "Unknown FD set by select at line %d\n",__LINE__);
                exit(1);
            }
        }
    }
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

int main(int argc, char *argv[])
{
    if (getenv("LD_LIBRARY_PATH")) printf("ld_library_path=%s\n", getenv("LD_LIBRARY_PATH")); else printf("ld_library_path=(null)\n");
    AddVisItLibraryPaths(argc, argv);

    printf("\n          >>> STARTING SIMULATION PROTOTYPE <<<\n\n\n");

    printf("Known Commands:\n"
           "     quit  :        exit code\n"
           "     step  :        run for one cycle\n"
           "     run   :        run continuously\n"
           "     visit :        start visit engine\n\n");

    InitializeVariables();
    MainLoop();

    return 0;
}
