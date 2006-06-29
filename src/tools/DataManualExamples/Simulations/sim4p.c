/* SIMPLE PARALLEL SIMULATION SKELETON */
#include <VisItControlInterface_V1.h>
#include <stdio.h>
#include <mpi.h>

#include <stubs.c>

/* CHANGE 1 */
#ifdef PARALLEL
static int visit_broadcast_int_callback(int *value, int sender)
{
    return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
}

static int visit_broadcast_string_callback(char *str, int len, int sender)
{
    return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
#endif

static int par_rank = 0;
static int par_size = 1;

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback()
{
   int command = VISIT_COMMAND_PROCESS;
   BroadcastSlaveCommand(&command);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(void)
{
    int command;
    if (par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success)
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

/* Is the simulation in run mode (not waiting for VisIt input) */
static int runFlag = 1;

/* New function to contain the program's main loop. */
void mainloop(void)
{
    int blocking, visitstate, err = 0;

    do
    {
        blocking = runFlag ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        visitstate = VisItDetectInput(blocking, -1);

        /* Do different things depending on the output from VisItDetectInput. */
        if(visitstate >= -5 && visitstate <= -1)
        {
            fprintf(stderr, "Can't recover from error!\n");
            err = 1;
        }
        else if(visitstate == 0)
        {
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep();
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection())
            {
                fprintf(stderr, "VisIt connected\n");
                VisItSetSlaveProcessCallback(SlaveProcessCallback);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            runFlag = 0;
            if(!ProcessVisItCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                runFlag = 1;
            }
        }
    } while(!simulation_done() && err == 0);
}

int main(int argc, char **argv)
{
    /* Initialize environment variables. */
    VisItSetupEnvironment();

/* CHANGE 2 */
#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &par_size);

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell VSIL whether the simulation is parallel. */
    VisItSetParallel(par_size > 1);
    VisItSetParallelRank(par_rank);
#endif

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(par_rank == 0)
    {
        VisItInitializeSocketAndDumpSimFile("sim4p",
        "Parallel C prototype simulation connects to VisIt",
        "/path/to/where/sim/was/started", NULL, NULL);
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck();

    /* Call the main loop. */
    mainloop();

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}


