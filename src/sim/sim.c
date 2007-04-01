#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>

int consoleinputdescriptor = 0;
int engineinputdescriptor = -1;

int runflag = 0;
int quitflag = 0;

int cycle = 0;

void *(*v_loadengine)() = NULL;
int   (*v_getdescriptor)(void*) = NULL;
void  (*v_processinput)(void*) = NULL;
void  (*v_initialize)(void*,int,char**) = NULL;
void  (*v_connectviewer)(void*,int,char**) = NULL;
void *v_engine = NULL;

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
     time_t starttime = time(NULL);
     while (time(NULL) == starttime)
     {
         // Wait
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

void ConnectToViewer()
{
    char v_host[256];
    char v_port[256];
    char v_key[256];
    char *argv[10];
    int nargs = 9;

    fprintf(stderr, "host: ");
    gets(v_host);

    fprintf(stderr, "port: ");
    gets(v_port);

    fprintf(stderr, "key : ");
    gets(v_key);


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
}

void OpenVisit()
{
    void *handle = dlopen("libewrap.so", RTLD_LAZY | RTLD_GLOBAL);

    if (!handle)
    {
        fprintf(stderr, "couldn't dlopen libewrap.so: %s\n", dlerror());
        return;
    }

    v_loadengine = dlsym(handle, "load_engine");
    if (!v_loadengine) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_getdescriptor = dlsym(handle, "get_descriptor");
    if (!v_getdescriptor) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_processinput = dlsym(handle, "process_input");
    if (!v_processinput) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_initialize = dlsym(handle, "initialize");
    if (!v_initialize) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_connectviewer = dlsym(handle, "connect_to_viewer");
    if (!v_connectviewer) { fprintf(stderr, "couldn't find symbol: %s\n", dlerror()); return; }

    v_engine = v_loadengine();

    ConnectToViewer();

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

    MainLoop();

    return 0;
}
