/******************************************************************************
 * Game of Life demo example for in-situ
 * Written by Jean M Favre, Swiss National Supercomputer Center
 * Wed May 26 11:42:47 CEST 2010
 * inspired by the examples from VisIt, written by Brad Whitlock
 * runs in parallel
 * Some initial conditions were taken from the Game of life Wiki page on Wikipedia
 ******************************************************************************/

#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif
#include "SimulationExample.h"

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

#ifdef PARALLEL
MPI_Comm comm;
#endif

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Life data and functions
 ******************************************************************************/

/* The number of cells (or zones) displayed on screen is NN * NN*/
/*#define NN 48*96*2 */
#define NN 48*2 

typedef enum {RANDOM = 0, GLIDER=1, ACORN=2, DIEHARD=3} BCtype;

typedef struct
{
    int    Nrows;     /* size of true_life array */
    int    Ncolumns;  /* size of true_life array */
    float *rmesh_x;
    float *rmesh_y;
    int    rmesh_dims[3];
    int    rmesh_ndims;
    int   *true_life;     /* true_life is the NNxNN array exposed to VisIt*/
    int   *working_life;  /* working_life is the (NN+2)x(NN+2) array used to compute it.*/
} life_data;

void
life_data_ctor(int par_size, life_data *life)
{
    life->Nrows = NN/par_size;
    life->Ncolumns = NN;
    life->rmesh_x = NULL;
    life->rmesh_y = NULL;
    life->rmesh_dims[0] = NN+1; /* shall be redefined later after Parallel init*/
    life->rmesh_dims[1] = NN+1;
    life->rmesh_dims[2] = 1;
    life->rmesh_ndims = 2;
    life->true_life = NULL;
    life->working_life = NULL;
}

void
life_data_dtor(life_data *life)
{
    if(life->rmesh_x != NULL)
    {
        free(life->rmesh_x);
        life->rmesh_x = NULL;
    }
    if(life->rmesh_y != NULL)
    {
        free(life->rmesh_y);
        life->rmesh_y = NULL;
    }
    if(life->true_life != NULL)
    {
        free(life->true_life);
        life->true_life = NULL;
    }
    if(life->working_life != NULL)
    {
        free(life->working_life);
        life->working_life = NULL;
    }
}

/* A 2D Rectilinear mesh of size NNxNN is built and will be visualized
   A second array used during the computation, and including ghost-cells is also required.
*/
void
life_data_allocate(life_data *life, int par_rank, int par_size)
{
    int i;
    float offset;

    life->rmesh_x = (float*)malloc(sizeof(float) * (life->Ncolumns + 1));
    life->rmesh_y = (float*)malloc(sizeof(float) * (life->Nrows + 1));

    life->rmesh_dims[0] = life->Ncolumns + 1;
    life->rmesh_dims[1] = life->Nrows + 1;
    life->rmesh_dims[2] = 1;

    for(i=0; i<= life->Ncolumns; i++)
    {
        life->rmesh_x[i] = (float)(i)/life->Ncolumns;
    }

    offset = (float)(par_rank) / par_size;
    for(i=0; i<=life->Nrows; i++)
    {
        life->rmesh_y[i] = offset + (float)(i)/life->Ncolumns;
    }

    /* 2D array of data items exposed as solution */
    life->true_life = (int*)malloc(sizeof(int) * life->Nrows * life->Ncolumns);

    /* 2D array of data items with 2 extra zones for ghost-cells */
    life->working_life = (int*)malloc(sizeof(int) * (life->Nrows+2) * (life->Ncolumns+2));
}

void
life_data_simulate(life_data *life, int par_rank, int par_size)
{
    int nsum, i, j, JPNN, JNN, JMNN;
    int *true_life = NULL, *working_life = NULL;
#ifdef PARALLEL
    int source, dest;
    MPI_Status status;
    MPI_Request request;
#endif

    /* Alias for readability */
    working_life = life->working_life;
    true_life = life->true_life;

    /* copy true_life to working_life to prepare for new iteration. Valid for 
       all MPI processes.
     */
    for(j=1; j <= life->Nrows; j++)
    {
        for(i=1; i <= life->Ncolumns; i++)
        {
            working_life[j*(life->Ncolumns + 2) + i] = true_life[(j-1) * life->Ncolumns +(i-1)];
        }
    }

    /* The Parallel allocation splits the whole mesh into horizontal stripes.
       All procs manage the full set of columns while treating only a subet 
       of rows initialize ghost cells from the working_life array. Valid for 
       all MPI processes because each rank has a full set of columns
     */
    for(j=1; j<= life->Nrows; j++)
    {
        /* left-most column at index i=0 */
        working_life[j*(life->Ncolumns+2) + 0]    = true_life[(j-1)*life->Ncolumns + life->Ncolumns-1];
        /* right-most column at index i=life->Ncolumns+1  */
        working_life[j*(life->Ncolumns+2) + life->Ncolumns+1] = true_life[(j-1)*life->Ncolumns + 0];
    }

    /* This next part depends on the MPI rank
       bottom row at index j=0 at rank J gets initialized from the top row from rank J-1
       top row at index j=(life->Nrows+1) at rank J gets initialized from bottom row from rank J+1
       Using the integer modulo op allows us to wrap around the number of procs
     */
#ifdef PARALLEL
    source = (par_size + par_rank - 1) % par_size;
    dest = (par_rank+1) % par_size;
    MPI_Irecv(&working_life[1], life->Ncolumns, MPI_INT, dest, 1000, comm, &request);
    MPI_Send(&true_life[(life->Nrows-1)*life->Ncolumns], life->Ncolumns, MPI_INT, source,  1000, comm);
    MPI_Wait( &request, &status);

    MPI_Irecv(&working_life[(life->Nrows+1)*(life->Ncolumns+2) + 1], life->Ncolumns, MPI_INT, dest, 1001, comm, &request);
    MPI_Send(&true_life[0], life->Ncolumns, MPI_INT, source,  1001, comm);
    MPI_Wait( &request, &status);
#else
    memcpy(&working_life[1], &true_life[(life->Nrows-1)*life->Ncolumns], life->Ncolumns * sizeof(int));
    memcpy(&working_life[(life->Nrows+1)*(life->Ncolumns+2) + 1], &true_life[0], life->Ncolumns * sizeof(int));
#endif

    /* corners 
       rank 0, lower left corner    <= rank Max upper right corner
       rank 0, lower right corner   <= rank Max, upper left corner
       rank max, upper left corner  <= rank 0, lower right corner
       rank max, upper right corner <= rank 0, lower left corner
    */
#ifdef PARALLEL
    if(par_rank == 0)
    {
        source = par_size -1;
        MPI_Irecv(&working_life[(life->Nrows+1)*(life->Ncolumns+2) + 0], 1, MPI_INT, source, 1002, comm, &request);
        MPI_Send(&true_life[life->Ncolumns-1], 1, MPI_INT, source,  1004, comm);
        MPI_Wait( &request, &status);
  
        MPI_Irecv(&working_life[life->Ncolumns+1], 1, MPI_INT, source, 1003, comm, &request);
        MPI_Send(&true_life[0], 1, MPI_INT, source,  1005, comm);
        MPI_Wait( &request, &status);
    }
    if(par_rank == (par_size -1))
    {
        source = 0;
        MPI_Irecv(&working_life[0], 1, MPI_INT, source, 1004, comm, &request);
        MPI_Send(&true_life[life->Ncolumns*(life->Nrows-1) + life->Ncolumns-1], 1, MPI_INT, source,  1002, comm);
        MPI_Wait( &request, &status);
  
        MPI_Irecv(&working_life[(life->Nrows+1)*(life->Ncolumns+2) + life->Ncolumns+1], 1, MPI_INT, source, 1005, comm, &request);
        MPI_Send(&true_life[life->Ncolumns*(life->Nrows-1) + 0], 1, MPI_INT, source,  1003, comm);
        MPI_Wait( &request, &status);
    }
#else
    working_life[0              ] = true_life[life->Ncolumns*(life->Nrows-1) + life->Ncolumns-1];     /* copy the upper right corner */
    working_life[life->Ncolumns+1] = true_life[life->Ncolumns*(life->Nrows-1) + 0];                   /* copy the uppper left corner */
    working_life[(life->Nrows+1)*(life->Ncolumns+2) + 0              ] = true_life[life->Ncolumns-1]; /* copy the bottom right corner */
    working_life[(life->Nrows+1)*(life->Ncolumns+2) + life->Ncolumns+1] = true_life[0];               /* copy the bottom left corner */
#endif

    /* calculate true_life(i-1, j-1) using working_life */
    for(j=1; j<= life->Nrows; j++)
    {
        JPNN = (j+1)*(life->Ncolumns+2);
        JNN  =  j   *(life->Ncolumns+2);
        JMNN = (j-1)*(life->Ncolumns+2);
        for(i=1; i <= life->Ncolumns; i++)
        {
            nsum =  working_life[JPNN + i - 1] + working_life[JPNN + i] + working_life[JPNN + i + 1]
                  + working_life[ JNN + i - 1]                          + working_life[ JNN + i + 1]
                  + working_life[JMNN + i - 1] + working_life[JMNN + i] + working_life[JMNN + i + 1];
    
            switch(nsum)
            {
            case 3:
                true_life[(j-1)*life->Ncolumns + (i-1)] = 1;
                break;
            case 2:
                /* keep its current life status */
                true_life[(j-1)*life->Ncolumns + (i-1)] = working_life[JNN + i];
                break;
            default:
                /* dies by over-crowding or under-population */
                true_life[(j-1)*life->Ncolumns + (i-1)] = 0;
            }
        }
    }
}

void
life_data_ResetInitialConditions(life_data *life, BCtype bc)
{
    /* in all cases, we center the figure in the middle of the grid */
    int i, j;

    switch(bc)
    {
    case RANDOM:
        for(j=0; j < life->Nrows; j++)
        {
            for(i=0; i < life->Ncolumns; i++)
            {
                float x = rand()/((float)RAND_MAX + 1);
                if(x<0.5)
                    life->true_life[j*life->Ncolumns + i] = 0;
                else
                    life->true_life[j*life->Ncolumns + i] = 1;
            }
        }
        break;

    case GLIDER:
        for(j=0; j < life->Nrows; j++)
        {
            for(i=0; i< life->Ncolumns; i++)
            {
               if(
                 ( (j == life->Nrows/2)     && (i >= (life->Ncolumns-1)/2 && i <= (life->Ncolumns-1)/2 + 2)) || /* row NN/2 */
                 ( (j == (life->Nrows/2)+1) && (i == (life->Ncolumns-1)/2 + 2)) ||                        /* next row up  */
                 ( (j == (life->Nrows/2)+2) && (i == (life->Ncolumns-1)/2 + 1))                          /* next row up */
                 )
                    life->true_life[j*life->Ncolumns + i] = 1;
               else
                    life->true_life[j*life->Ncolumns + i] = 0;
            }
        }
        break;

    case ACORN:
        for(j=0; j < life->Nrows; j++)
        {
            for(i=0; i< life->Ncolumns; i++)
            {
                if(
                  ( (j == life->Nrows/2)     && (i >= (life->Ncolumns-1)/2 + 0 && i <= (life->Ncolumns-1)/2 + 1)) ||
                  ( (j == life->Nrows/2)     && (i >= (life->Ncolumns-1)/2 + 4 && i <= (life->Ncolumns-1)/2 + 6)) ||
                  ( (j == (life->Nrows/2)+1) && (i == (life->Ncolumns-1)/2 + 3)) ||
                  ( (j == (life->Nrows/2)+2) &&  i == (life->Ncolumns-1)/2 + 1)
                  )
                    life->true_life[j*life->Ncolumns + i] = 1;
                else
                    life->true_life[j*life->Ncolumns + i] = 0;
            }
        }
        break;

    case DIEHARD:
        for(j=0; j < life->Nrows; j++)
        {
            for(i=0; i< life->Ncolumns; i++)
            {
                if(
                  ( (j == life->Nrows/2)     && (i == (life->Ncolumns-1)/2 + 1)) ||
                  ( (j == life->Nrows/2)     && (i >= (life->Ncolumns-1)/2 + 5 && i <= (life->Ncolumns-1)/2 + 7)) ||
                  ( (j == (life->Nrows/2)+1)  && (i >= (life->Ncolumns-1)/2 + 0 && i <= (life->Ncolumns-1)/2 + 1)) ||
                  ( (j == (life->Nrows/2)+2) &&  i == (life->Ncolumns-1)/2 + 6)
                  )
                    life->true_life[j*life->Ncolumns + i] = 1;
                else
                    life->true_life[j*life->Ncolumns + i] = 0;
            }
        }
        break;
    } /* end of switch */
}

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
    int       cycle;
    double    time;
    int       runMode;
    int       done;
    int       par_rank;
    int       par_size;

    life_data life;
} simulation_data;


void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->par_rank = 0;
    sim->par_size = 1;

    life_data_ctor(sim->par_size, &sim->life);
}

void
simulation_data_dtor(simulation_data *sim)
{
    life_data_dtor(&sim->life);
}

const char *cmd_names[] = {"halt", "step", "run", "update", "reset"};

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

void
simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += 1;

    /* Simulate the current round of life. */
    life_data_simulate(&sim->life, sim->par_rank, sim->par_size);

    VisItTimeStepChanged();
    VisItUpdatePlots();
}

/* Callback function for control commands, which are the buttons in the 
 * GUI's Simulation window. This type of command is handled automatically
 * provided that you have registered a command callback such as this.
 */
void ControlCommandCallback(const char *cmd, const char *args, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;
    else if(strcmp(cmd, "update") == 0)
    {
        VisItTimeStepChanged();
        VisItUpdatePlots();
    }
    else if(strcmp(cmd, "reset") == 0)
    {
        life_data_ResetInitialConditions(&sim->life, RANDOM);
        VisItTimeStepChanged();
        VisItUpdatePlots();
    }
}

#ifdef PARALLEL
static int visit_broadcast_int_callback(int *value, int sender)
{
    return MPI_Bcast(value, 1, MPI_INT, sender, comm);
}

static int visit_broadcast_string_callback(char *str, int len, int sender)
{
    return MPI_Bcast(str, len, MPI_CHAR, sender, comm);
}
#endif


/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, comm);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback(void)
{
   int command = VISIT_COMMAND_PROCESS;
   BroadcastSlaveCommand(&command);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(simulation_data *sim)
{
    int command = VISIT_COMMAND_FAILURE;
    if (sim->par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success == VISIT_OKAY)
        {
            command = VISIT_COMMAND_SUCCESS;
            BroadcastSlaveCommand(&command);
            return 1;
        }
        else
        {
            command = VISIT_COMMAND_FAILURE;
            BroadcastSlaveCommand(&command);
            return 0;
        }
    }
    else
    {
        /* Note: only through the SlaveProcessCallback callback
         * above can the rank 0 process send a VISIT_COMMAND_PROCESS
         * instruction to the non-rank 0 processes. */
        while (1)
        {
            BroadcastSlaveCommand(&command);
            switch (command)
            {
            case VISIT_COMMAND_PROCESS:
                VisItProcessEngineCommand();
                break;
            case VISIT_COMMAND_SUCCESS:
                return 1;
            case VISIT_COMMAND_FAILURE:
                return 0;
            }
        }
    }
}

/* Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 */
void
ProcessConsoleCommand(simulation_data *sim)
{
    /* Read A Command */
    char cmd[1000];
    if (sim->par_rank == 0)
    {
        int iseof = (fgets(cmd, 1000, stdin) == NULL);
        if (iseof)
        {
            sprintf(cmd, "quit");
            printf("quit\n");
        }

        if (strlen(cmd)>0 && cmd[strlen(cmd)-1] == '\n')
            cmd[strlen(cmd)-1] = '\0';
    }

#ifdef PARALLEL
    /* Broadcast the command to all processors. */
    MPI_Bcast(cmd, 1000, MPI_CHAR, 0, comm);
#endif

    if(strcmp(cmd, "quit") == 0)
        sim->done = 1;
    else if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;
    else if(strcmp(cmd, "update") == 0)
    {
        VisItTimeStepChanged();
        VisItUpdatePlots();
    }
    else if(strcmp(cmd, "reset") == 0)
    {
        VisItTimeStepChanged();
        life_data_ResetInitialConditions(&sim->life, RANDOM);
        VisItUpdatePlots();
    }
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Jean Favre
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    int blocking, visitstate = 0, err = 0;

    if (sim->par_rank == 0)
    {
        fprintf(stderr, "command> ");
        fflush(stderr);
    }
    do
    {
        blocking = (sim->runMode == VISIT_SIMMODE_RUNNING) ? 0 : 1;

        /* Get input from VisIt or timeout so the simulation can run. */
        if(sim->par_rank == 0)
            visitstate = VisItDetectInput(blocking, fileno(stdin));
#ifdef PARALLEL
        MPI_Bcast(&visitstate, 1, MPI_INT, 0, comm);
#endif
        /* Do different things depending on the output from VisItDetectInput. */
        switch(visitstate)
        {
        case 0:
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep(sim);
            break;
        case 1:
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
            {
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback, (void*)sim);
                VisItSetSlaveProcessCallback(SlaveProcessCallback);

                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetDomainList(SimGetDomainList, (void*)sim);
            }
            else 
            {
                /* Print the error message */
                char *err = VisItGetLastError();
                fprintf(stderr, "VisIt did not connect: %s\n", err);
                free(err);
            }
            break;
        case 2:
            /* VisIt wants to tell the engine something. */
            if(!ProcessVisItCommand(sim))
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                /*sim->runMode = SIM_RUNNING;*/
            }
            break;
        case 3:
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand(sim);
            if (sim->par_rank == 0)
            {
                fprintf(stderr, "command> ");
                fflush(stderr);
            }
            break;
        default:
            fprintf(stderr, "Can't recover from error %d!\n", visitstate);
            err = 1;
            break;
        }
    } while(!sim->done && err == 0);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Jean Favre
 * Date:       Wed May 26 11:42:47 CEST 2010
 *
 * Input Arguments:
 *   argc : The number of command line arguments.
 *   argv : The command line arguments.
 *
 * Modifications:
 *
 *****************************************************************************/

int main(int argc, char **argv)
{
#ifdef PARALLEL
    int i, customcomm = 0;
#endif
    simulation_data sim;
    simulation_data_ctor(&sim);

    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

#ifdef PARALLEL
    /* Scan command line for -customcomm. */
    for(i = 0; i < argc; ++i)
        if(strcmp(argv[i], "-customcomm") == 0)
            customcomm = 1;

    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Determine communicator. */
    if(customcomm)
    {
        int gsize = 1, grank = 0;
        MPI_Comm_rank (MPI_COMM_WORLD, &grank);
        MPI_Comm_size (MPI_COMM_WORLD, &gsize);
  
        /* Create custom communicator with even ranks from world comm. */
        MPI_Comm_split(MPI_COMM_WORLD, grank%2, grank, &comm);

        /* Odd processes exit. */
        if(grank%2 == 1)
        {
            MPI_Finalize();
            return 0;
        }
    }
    else
    {
        comm = MPI_COMM_WORLD;
    }

    /* comm = MPI_COMM_WORLD; */
    MPI_Comm_rank (comm, &sim.par_rank);
    MPI_Comm_size (comm, &sim.par_size);
 
    /* Adjust the life partitioning */
    sim.life.Nrows = NN/sim.par_size; /* assume they divide evenly */
    if((float)(NN)/sim.par_size - NN/sim.par_size > 0.0)
    {
        fprintf(stderr,"The total number of rows does not divide evenly by the number of MPI tasks. Resubmit\n");
        exit(1);
    }

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);
#endif

    if(sim.par_rank == 0)
    {
        VisItInitializeSocketAndDumpSimFile("life",
            "Game of life by John Conway",
            "/path/to/where/sim/was/started",
            NULL, NULL, SimulationFilename());
    }
#ifdef PARALLEL
    /* Install custom communicator. */
    if(customcomm)
    {
        printf("Setting custom communicator: %p\n", (void*)&comm);
        VisItSetMPICommunicator(&comm);
    }
#endif

    life_data_allocate(&sim.life, sim.par_rank, sim.par_size);
    life_data_ResetInitialConditions(&sim.life, RANDOM);

    mainloop(&sim);

    simulation_data_dtor(&sim);

#ifdef PARALLEL
    MPI_Finalize();
#endif
    return 0;
}

/* DATA ACCESS FUNCTIONS */

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMetaData(void *cbdata)
{
    visit_handle md = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create metadata. */
    if(VisIt_SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
        int i;
        visit_handle m1 = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
                VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Set the first mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&m1) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(m1, "mesh2d");
            VisIt_MeshMetaData_setMeshType(m1, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(m1, 2);
            VisIt_MeshMetaData_setSpatialDimension(m1, 2);
            VisIt_MeshMetaData_setNumDomains(m1, sim->par_size);
            VisIt_MeshMetaData_setXUnits(m1, "cm");
            VisIt_MeshMetaData_setYUnits(m1, "cm");
            VisIt_MeshMetaData_setXLabel(m1, "Width");
            VisIt_MeshMetaData_setYLabel(m1, "Height");
    
            VisIt_SimulationMetaData_addMesh(md, m1);
        }

        /* Add a zonal scalar variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "life");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
   
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add some custom commands. */
        for(i = 0; i < (int) (sizeof(cmd_names)/sizeof(const char *)); ++i)
        {
            visit_handle cmd = VISIT_INVALID_HANDLE;
            if(VisIt_CommandMetaData_alloc(&cmd) == VISIT_OKAY)
            {
                VisIt_CommandMetaData_setName(cmd, cmd_names[i]);
                VisIt_SimulationMetaData_addGenericCommand(md, cmd);
            }
        }
    }

    return md;
}

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "mesh2d") == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxc, hyc;
            int minRealIndex[3], maxRealIndex[3];
            minRealIndex[0] = minRealIndex[1] = minRealIndex[2] = 0;
      
            maxRealIndex[0] = sim->life.rmesh_dims[0]-1;
            maxRealIndex[1] = sim->life.rmesh_dims[1]-1;
            maxRealIndex[2] = 0;

            VisIt_VariableData_alloc(&hxc);
            VisIt_VariableData_alloc(&hyc);
            VisIt_VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, sim->life.rmesh_dims[0], sim->life.rmesh_x);
            VisIt_VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, sim->life.rmesh_dims[1], sim->life.rmesh_y);
            VisIt_RectilinearMesh_setCoordsXY(h, hxc, hyc);

            VisIt_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex);
        }
    }
    return h;
}

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    int nComponents = 1, nTuples = 0;
    simulation_data *sim = (simulation_data *)cbdata;

    if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        if(strcmp(name, "life") == 0)
        {
            nTuples = (sim->life.rmesh_dims[0]-1) * (sim->life.rmesh_dims[1]-1);
            VisIt_VariableData_setDataI(h, VISIT_OWNER_SIM, nComponents, nTuples, sim->life.true_life);
        }
        else
        {
            VisIt_VariableData_free(h);
            h = VISIT_INVALID_HANDLE;
        }
    }
    return h;
}

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
    {
        visit_handle hdl;
        int *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int));
        *iptr = sim->par_rank;

        if(VisIt_VariableData_alloc(&hdl) == VISIT_OKAY)
        {
            VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
            VisIt_DomainList_setDomains(h, sim->par_size, hdl);
        }
    }
    return h;
}

