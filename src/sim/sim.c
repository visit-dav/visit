#include "simulation.h"

#include <stdlib.h>
#include <stdio.h>

#include <dlfcn.h>
#include <errno.h>
#include <signal.h> // for signal handling (may go away)
#include <string.h> // for strdup (may go away)
#include <unistd.h> // for execv (may go away)
#include <math.h>


#ifdef PARALLEL
#include <mpi.h>
#endif

int par_rank = 0;
int par_size = 1;

#define TRUE 1
#define FALSE 0
#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX3(a,b,c) MAX(a,MAX(b,c))

int consoleinputdescriptor = 0;
int engineinputdescriptor = -1;
int listenSock = -1;

int runflag = 0;
int quitflag = 0;

int cycle = 0;

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
    for (i=0; i<p_nx; i++)
    {
        p_zvalues[i] = p_zvalues[i] + sqrt(i);
    }

    printf(" ... Finished cycle %d\n", cycle);
    cycle++;
    TimeStepChanged();
}

void RunSimulation()
{
    runflag = 1;
}

void StopSimulation()
{
    runflag = 0;
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
    else if (!strcmp(buff, "stop"))
    {
        StopSimulation();
    }
    else if (!strcmp(buff, "disconnect"))
    {
        Disconnect();
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

void MainLoop()
{
    fprintf(stderr, "command> ");
    fflush(stderr);

    while (!quitflag)
    {
        fd_set readSet;

        int maxdescriptor = MAX3(consoleinputdescriptor,
                                 engineinputdescriptor,
                                 listenSock);

        int ret = 0;
        int blocking = runflag ? FALSE : TRUE;
        struct timeval ZeroTimeout = {0,0};
        struct timeval *timeout = (blocking ? NULL : &ZeroTimeout);

        FD_ZERO(&readSet);

        FD_SET(consoleinputdescriptor, &readSet);
        if (engineinputdescriptor >= 0)
            FD_SET(engineinputdescriptor, &readSet);
        if (listenSock >= 0)
            FD_SET(listenSock, &readSet);
        ret = select(maxdescriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                     timeout);

        if (ret < 0)
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
        else if (ret == 0)
        {
            if (runflag)
            {
                RunSingleCycle();
            }
        }
        else
        {
            if (FD_ISSET(listenSock, &readSet))
            {
                fprintf(stderr, "GOT CONNECTION ATTEMPT ON LISTEN SOCKET!\n");
                AttemptToCompleteConnection();
            }
            else if (FD_ISSET(consoleinputdescriptor, &readSet))
            {
                ProcessConsoleCommand();
                if (!quitflag && !runflag)
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


int main(int argc, char *argv[])
{
    if (getenv("LD_LIBRARY_PATH")) printf("ld_library_path=%s\n", getenv("LD_LIBRARY_PATH")); else printf("ld_library_path=(null)\n");
    AddVisItLibraryPaths(argc, argv);
    InitializeSocketAndDumpSimFile("proto");

#ifdef PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &par_size);
#endif

    if (par_rank == 0)
    {
        printf("\n          >>> STARTING SIMULATION PROTOTYPE <<<\n\n\n");

        printf("Known Commands:\n"
               "     quit  :        exit code\n"
               "     step  :        run for one cycle\n"
               "     run   :        run continuously\n");
    }

    InitializeVariables();
    MainLoop();

    return 0;
}
