// ****************************************************************************
//  Programmer:  Jeremy Meredith
//  Creation:    July 10, 2003
//
//  Modifications:
//    Jeremy Meredith, Wed Jan 14 12:23:02 PST 2004
//    Made it work with the VisIt library.
//
//    Jeremy Meredith, Tue Mar 30 18:12:12 PDT 2004
//    Some major preliminary productization.
//
//    Jeremy Meredith, Wed Aug 25 13:36:11 PDT 2004
//    Getting closer.  Some hefty changes -- refactored some things
//    into a separate file that can be included by other simulations.
//
//    Jeremy Meredith, Mon Nov  1 17:27:54 PST 2004
//    Made it work in parallel, at least with two processors.
//
//    Jeremy Meredith, Thu Mar 17 12:59:15 PST 2005
//    Changed it to use float values.
//
//    Jeremy Meredith, Mon Apr  4 16:44:29 PDT 2005
//    Split command parsing out into a separate function.
//    Added more options to the sim file dumping.
//
// ****************************************************************************


#include "VisItV1.h"
#include "sim.h"

#include <stdlib.h>
#include <stdio.h>

#include <dlfcn.h>
#include <errno.h>
#include <signal.h> // for signal handling (may go away)
#include <string.h> // for strdup (may go away)
#include <unistd.h> // for execv (may go away)
#include <math.h>
#include <time.h>

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

static int consoleinputdescriptor = 0;
static int engineinputdescriptor = -1;
static int listenSock = -1;

int runflag = 0;
int quitflag = 0;

int cycle = 0;

int p_nx = 10;
int p_ny = 10;
int p_nz = 10;
float *p_xcoords;
float *p_ycoords;
float *p_zcoords;
float *p_zvalues;
float *p_nvalues;

int numdomains = 1;

void InitializeVariables()
{
    int i;
    p_xcoords = malloc(sizeof(float) * p_nx);
    p_ycoords = malloc(sizeof(float) * p_ny);
    p_zcoords = malloc(sizeof(float) * p_nz);
    p_zvalues = malloc(sizeof(float) * (p_nx-1)*(p_ny-1)*(p_nz-1));
    p_nvalues = malloc(sizeof(float) * p_nx*p_ny*p_nz);
    for (i=0; i<p_nx; i++)
    {
        int ii = (i + (p_nx-1)*par_rank);
        p_xcoords[i] = ii*2.0;
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

    numdomains = par_size;
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
        int ii = (i + (p_nx-1)*par_rank);
        p_xcoords[i] = ii * 2.0 + (ii*ii*0.1 * (double)(cycle));
    }
    for (i=0; i<p_nx; i++)
    {
        p_zvalues[i] = p_zvalues[i] + sqrt(i);
    }

    printf(" ... Finished cycle %d\n", cycle);
#ifdef PARALLEL
    MPI_Barrier(MPI_COMM_WORLD);
    printf(" and both processors synchronized.\n");
#endif
    cycle++;
    VisItTimeStepChanged();
}

void RunSimulation()
{
    runflag = 1;
}

void StopSimulation()
{
    runflag = 0;
    VisItTimeStepChanged();
}

void FakeConsoleCommand(char *str)
{
#ifdef PARALLEL
    char buff[10000];
    sprintf(buff, str);
    MPI_Bcast(buff, 10000, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif
}

void ProcessCommand(const char *cmd)
{
#ifdef PARALLEL
    char buff[10000];
    if (par_rank == 0)
        strcpy(buff, cmd);
    MPI_Bcast(buff, 10000, MPI_CHAR, 0, MPI_COMM_WORLD);
    cmd = buff;
#endif

    if (!strcmp(cmd, "quit"))
    {
        quitflag = 1;
    }
    else if (!strcmp(cmd, "step"))
    {
        RunSingleCycle();
    }
    else if (!strcmp(cmd, "run"))
    {
        RunSimulation();
    }
    else if (!strcmp(cmd, "halt"))
    {
        printf("execution paused....\n");
        StopSimulation();
    }
    else if (!strcmp(cmd, "visit_disconnect"))
    {
        VisItDisconnect();
    }
    else if (!strcmp(cmd, "visit_connect"))
    {
        VisItAttemptToCompleteConnection();
    }
    else if (!strcmp(cmd, "visit_process"))
    {
        VisItProcessEngineCommand();
    }
    else if (!strcmp(cmd, ""))
    {
        // Do nothing on blank input.
    }
    else
    {
        fprintf(stderr, "Error: unknown command '%s'\n", cmd);
    }
}

void ProcessConsoleCommand()
{
    // Read A Command
    char buff[10000];

    if (par_rank == 0)
    {
        int iseof = (fgets(buff, 10000, stdin) == NULL);
        if (iseof)
        {
            sprintf(buff, "quit");
            printf("quit\n");
        }

        if (strlen(buff)>0 && buff[strlen(buff)-1] == '\n')
            buff[strlen(buff)-1] = '\0';
    }

    ProcessCommand(buff);
}

void SlaveProcessCallback()
{
    FakeConsoleCommand("visit_process");
}

void ControlCommandCallback(const char *cmd,
                            int int_data,
                            float float_data,
                            const char *string_data)
{
    fprintf(stderr, "ControlCommandCallback(cmd=%s)\n",cmd);
    // Ignore int/float/string_data
    ProcessCommand(cmd);
}

void MainLoop()
{
    fprintf(stderr, "command> ");
    fflush(stderr);

    while (!quitflag)
    {
        if (par_rank == 0)
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
            {
                FD_SET(engineinputdescriptor, &readSet);
            }

            if (listenSock >= 0)
            {
                FD_SET(listenSock, &readSet);
            }

            ret = select(maxdescriptor+1, &readSet, (fd_set *)NULL, (fd_set *)NULL,
                         timeout);

            if (ret < 0)
            {
                int err = errno;
                if (err == EINTR)
                {
                    fprintf(stderr, "\nInterrupted...... quitting\n");
                    quitflag = 1;
                }
                else if (err == EBADF)
                {
                    fprintf(stderr,"\nBad file descriptor in select\n");
                    quitflag = 1;
                }
                else if (err == EINVAL)
                {
                    fprintf(stderr,"\nBad argument to select\n");
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
                    FakeConsoleCommand("step");
                    RunSingleCycle();
                }
            }
            else
            {
                if (FD_ISSET(listenSock, &readSet))
                {
                    fprintf(stderr, "GOT CONNECTION ATTEMPT ON LISTEN SOCKET!\n");
                    FakeConsoleCommand("visit_connect");
                    if (!VisItAttemptToCompleteConnection())
                    {
                        fprintf(stderr, "Failed to connect!\n");
                        continue;
                    }
                    engineinputdescriptor = VisItGetEngineSocket();
                    VisItSetSlaveProcessCallback(SlaveProcessCallback);
                    VisItSetCommandCallback(ControlCommandCallback);
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
                    if (!VisItProcessEngineCommand())
                    {
                        FakeConsoleCommand("visit_disconnect");
                        VisItDisconnect();
                        engineinputdescriptor = -1;
                    }
                }
                else
                {
                    fprintf(stderr, "Unknown FD set by select at line %d\n",__LINE__);
                    exit(1);
                }
            }
        }
        else
        {
            ProcessConsoleCommand();
        }
    }
}


int main(int argc, char *argv[])
{
    if (getenv("LD_LIBRARY_PATH")) printf("ld_library_path=%s\n", getenv("LD_LIBRARY_PATH")); else printf("ld_library_path=(null)\n");
    //VisItAddLibraryPaths(argc, argv);
    VisItSetupEnvironment();

#ifdef PARALLEL
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &par_size);
    printf("PARALLEL started: num procs = %d\n", par_size);
    if (par_size == 1)
    {
        printf("Probably not using mpirun; try again!!!!!!\n");
        exit(0);
    }
#endif

    if (par_rank == 0)
    {
        VisItInitializeSocketAndDumpSimFile("proto",
                                            "Prototype Simulation",
                                            "/no/useful/path",
                                            NULL);
        listenSock = VisItGetListenSocket();

        printf("\n          >>> STARTING SIMULATION PROTOTYPE <<<\n\n\n");

        printf("Known Commands:\n"
               "     quit  :        exit code\n"
               "     step  :        run for one cycle\n"
               "     run   :        run continuously\n"
               "     halt  :        stop running\n");
    }

    InitializeVariables();
    MainLoop();

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}
