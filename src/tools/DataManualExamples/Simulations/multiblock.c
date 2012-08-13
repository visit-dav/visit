/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "SimulationExample.h"

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

void read_input_deck(void) { }
/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);
visit_handle SimGetDomainBoundaries(const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
#ifdef PARALLEL
    MPI_Comm par_comm;
#endif
    int      par_rank;
    int      par_size;
    int      cycle;
    double   time;
    int      runMode;
    int      done;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
}

void
simulation_data_dtor(simulation_data *sim)
{
}

const char *cmd_names[] = {"halt", "step", "run"};

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 *
 * Purpose: This function simulates one time step
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

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
}

/* CHANGE 1 */
#ifdef PARALLEL
static int visit_broadcast_int_callback(int *value, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(value, 1, MPI_INT, sender, sim->par_comm);
}

static int visit_broadcast_string_callback(char *str, int len, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(str, len, MPI_CHAR, sender, sim->par_comm);
}
#endif


/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command, simulation_data *sim)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback(void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    int command = VISIT_COMMAND_PROCESS;
    BroadcastSlaveCommand(&command, sim);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(simulation_data *sim)
{
    int command;
    if (sim->par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success == VISIT_OKAY)
        {
            command = VISIT_COMMAND_SUCCESS;
            BroadcastSlaveCommand(&command, sim);
            return 1;
        }
        else
        {
            command = VISIT_COMMAND_FAILURE;
            BroadcastSlaveCommand(&command, sim);
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
            BroadcastSlaveCommand(&command, sim);
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
        if(VisItReadConsole(1000, cmd) == VISIT_ERROR)
        {
            sprintf(cmd, "quit");
            printf("quit\n");
        }
    }

#ifdef PARALLEL
    /* Broadcast the command to all processors. */
    MPI_Bcast(cmd, 1000, MPI_CHAR, 0, sim->par_comm);
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
}

/******************************************************************************
 *
 * Function: mainloop
 *
 * Purpose: Handles the program's main event loop and dispatches events to 
 *          other functions for processing.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    int blocking, visitstate, err = 0;

    /* If we're not running by default then simulate once there's something
     * once VisIt connects.
     */
    if(sim->runMode == SIM_STOPPED)
        simulate_one_timestep(sim);

    if (sim->par_rank == 0)
    {
        fprintf(stderr, "command> ");
        fflush(stderr);
    }

    do
    {
        blocking = (sim->runMode == SIM_RUNNING) ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        if(sim->par_rank == 0)
        {
            visitstate = VisItDetectInput(blocking, fileno(stdin));
        }
#ifdef PARALLEL
        /* Broadcast the return value of VisItDetectInput to all procs. */
        MPI_Bcast(&visitstate, 1, MPI_INT, 0, sim->par_comm);
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
                VisItSetSlaveProcessCallback2(SlaveProcessCallback, (void*)sim);

                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetDomainList(SimGetDomainList, (void*)sim);
                VisItSetGetDomainBoundaries(SimGetDomainBoundaries, (void*)sim);
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
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
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
    char *env = NULL;
    simulation_data sim;
    simulation_data_ctor(&sim);

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Create a new communicator. */
    if (MPI_Comm_dup(MPI_COMM_WORLD, &sim.par_comm) != MPI_SUCCESS)
        sim.par_comm = MPI_COMM_WORLD;

    MPI_Comm_rank (sim.par_comm, &sim.par_rank);
    MPI_Comm_size (sim.par_comm, &sim.par_size);
#endif

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction2(visit_broadcast_int_callback, (void*)&sim);
    VisItSetBroadcastStringFunction2(visit_broadcast_string_callback, (void*)&sim);

    /* Tell libsim whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);

    /* Tell libsim which communicator to use. You must pass the address of
     * an MPI_Comm object.
     */
    VisItSetMPICommunicator((void *)&sim.par_comm);
#endif

    /* Only read the environment on rank 0. This could happen before MPI_Init if
     * we are using an MPI that does not like to let us spawn processes but we
     * would not know our processor rank.
     */
    if(sim.par_rank == 0)
        env = VisItGetEnvironment();

    /* Pass the environment to all other processors collectively. */
    VisItSetupEnvironment2(env);
    if(env != NULL)
        free(env);

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(sim.par_rank == 0)
    {
        /* Write out .sim file that VisIt uses to connect. */
        VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "multiblock_par",
#else
            "multiblock",
#endif
            "Demonstrates multiple blocks (collections of domains) and "
            "reduced/enhanced connectivity via domain boundaries.",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/* DATA ACCESS FUNCTIONS */

#define NBLOCKS 3
#define NDOMAINS 6

/* Number of zones for each domain. */
#define HiX_0 3
#define HiY_0 4

#define HiX_1 3
#define HiY_1 4

#define HiX_2 3
#define HiY_2 3

#define HiX_3 3
#define HiY_3 4

#define HiX_4 3
#define HiY_4 4

#define HiX_5 3
#define HiY_5 4

/* The dimensions (#nodes) for each domain. */
int mesh_dims[NDOMAINS][3] = {
    {HiX_0+1, HiY_0+1, 1},
    {HiX_1+1, HiY_1+1, 1},
    {HiX_2+1, HiY_2+1, 1},
    {HiX_3+1, HiY_3+1, 1},
    {HiX_4+1, HiY_4+1, 1},
    {HiX_5+1, HiY_5+1, 1}
};

/* Coordinate arrays for each domain. */
float domain0_coords[][2] = {
    {0.0, 0.0}, {1.2, 0.0}, {2.8, 0.0}, {4.0, 0.0}, 
    {0.0, 0.8}, {1.2, 0.8}, {2.8, 0.8}, {4.0, 0.8}, 
    {0.0, 1.6}, {1.2, 1.6}, {2.8, 1.6}, {4.0, 1.6}, 
    {0.0, 2.2}, {1.2, 2.2}, {2.8, 2.2}, {4.0, 2.2}, 
    {0.0, 3.0}, {1.2, 3.0}, {2.8, 3.0}, {4.0, 3.0}
};

float domain1_coords[][2] = {
    {4.0, 0.0}, {4.7, 0.0},  {5.5, 0.0},  {6.1, 0.0},
    {4.0, 0.8}, {4.7, 0.87}, {5.5, 0.93}, {6.1, 1.0},
    {4.0, 1.6}, {4.7, 1.73}, {5.5, 1.87}, {6.1, 2.0},
    {4.0, 2.2}, {4.7, 2.7},  {5.5, 3.3},  {6.1, 3.8},
    {4.0, 3.0}, {4.7, 4.0},  {5.5, 5.1},  {6.1, 5.8}
};

float domain2_coords[][2] = {
    {0.0,3.0}, {1.2, 3.0}, {2.8, 3.0}, {4.0, 3.0},
    {0.0,4.0}, {1.2+(2.2-1.2)*1./3., 4.0}, {2.8+(4.7-2.8)*1./3., 4.0}, {4.7, 4.0},
    {0.0,5.1}, {1.2+(2.2-1.2)*2./3., 5.1}, {2.8+(4.7-2.8)*2./3., 5.1}, {5.5, 5.1},
    {0.0,5.8}, {2.2, 5.8}, {4.7, 5.8}, {6.1, 5.8}
};

float domain3_coords[][2] = {
    {0.0, 5.8}, {2.2, 5.8}, {4.7, 5.8}, {6.1, 5.8},
    {0.0, 6.7}, {2.2, 6.7}, {4.7, 6.7}, {6.1, 6.7},
    {0.0, 7.8}, {2.2, 7.8}, {4.7, 7.8}, {6.1, 7.8},
    {0.0, 8.2}, {2.2, 8.2}, {4.7, 8.2}, {6.1, 8.2},
    {0.0, 8.8}, {2.2, 8.8}, {4.7, 8.8}, {6.1, 8.8}
};

float domain4_coords[][2] = {
    {6.1, 0.0}, {7.2, 0.0}, {8.8, 0.0}, {10., 0.0},
    {6.1, 1.0}, {7.2, 1.0}, {8.8, 1.0}, {10., 1.0},
    {6.1, 2.0}, {7.2, 2.0}, {8.8, 2.0}, {10., 2.0},
    {6.1, 3.8}, {7.2, 3.933}, {8.8, 4.066}, {10., 4.2},
    {6.1, 5.8}, {7.2, 5.8}, {8.8, 5.8}, {10., 5.8},
};

float domain5_coords[][2] = {
    {6.1, 5.8}, {7.2, 5.8}, {8.8, 5.8}, {10., 5.8},
    {6.1, 6.7}, {7.2, 6.7}, {8.8, 6.7}, {10., 6.7},
    {6.1, 7.8}, {7.2, 7.8}, {8.8, 7.8}, {10., 7.8},
    {6.1, 8.2}, {7.2, 8.2}, {8.8, 8.2}, {10., 8.2},
    {6.1, 8.8}, {7.2, 8.8}, {8.8, 8.8}, {10., 8.8}
};

float *mesh_coords[NDOMAINS] = {
    (float *)domain0_coords,
    (float *)domain1_coords,
    (float *)domain2_coords,
    (float *)domain3_coords,
    (float *)domain4_coords,
    (float *)domain5_coords
};

/* Which block/group each domain lives in. */
int block_id[NDOMAINS] = {0, 0, 1, 1, 2, 2};

/* Domain boundary information is often built up from 11-tuples of numbers that
   describe each boundary.

   The 11-tuple that describes the boundary has the following order:
   
   boundary {
      int neighbordomain;
      int mi;
      int orientation[3];
      int extents[6];
   }
   
   The neighbordomain value is the index of the neighboring domain, in other
   words: its 0-based domain number.

   The mi value is an index to where the boundary for the current domain falls
   in the other domain's boundary list. You can think of each domain as having
   a list of boundaries and the mi value is a "pointer". We'll write the boundary
   table like this:

   Domain 0
     1, mi, ...
   ...
   Domain 1
     0, ...

   You'd compute "mi" for the 1st boundary in domain 0's list by saying: "Domain 0 in 
   domain 1's list appears at which index?". Of course, the value is 0 in this case 
   because the boundary for domain 0 is the first one in domain 1's boundary list.

   The orientation values can specify which axes are first. For X,Y,Z, use 1,2,3.

   The extents consist of 6 values (loX, hiX, loY, hiY, loZ, hiZ) that indicate 
   where the cells in the neighbor domain touches this domain. So, all of the extents
   values for a boundary will be relative to the domain that owns the boundary in
   its boundary list. Each boundary must have an equally sized boundary in the 
   neighbor domain's boundary list. This means that each time you add a boundary,
   you'll need to add it for 2 domains.

   In this example, we have 6, 2D domains that abut on edges. Since there is a
   enhanced connectivity point, some domains that share a node but not cells also
   are said to share a boundary ( a single cell). The additional cell sharing 
   improves interpolation.

   We use 0-based indexing so low values are 0. Hi values are the number of zones
   in a particular dimension. For example if we wanted to select all of the X zones
   along a boundary for domain 0, we could say: lo, HiX_0.

   3D domains would share faces, edges, and in some cases single cells.
*/
#define lo 0
int neighbors[] = {
/* Domain 0 */
    1, 0,  1,2,3, HiX_0, HiX_0, lo, HiY_0,    lo, lo,
    2, 0,  1,2,3, lo, HiX_0,    HiY_0, HiY_0, lo, lo,
/* Domain 1 */
    0, 0,  1,2,3, lo, lo,       lo, HiY_1,    lo, lo,
    2, 1,  1,2,3, lo, HiX_1,    HiY_1, HiY_1, lo, lo,
    4, 0,  1,2,3, HiX_1, HiX_1, lo, HiY_1,    lo, lo,
    3, 2,  1,2,3, HiX_1, HiX_1, HiY_1, HiY_1, lo, lo,
    5, 2,  1,2,3, HiX_1, HiX_1, HiY_1, HiY_1, lo, lo,
/* Domain 2 */
    0, 1,  1,2,3, lo, HiX_2,    lo, lo,       lo, lo,
    1, 1,  1,2,3, HiX_2, HiX_2, lo, HiY_2,    lo, lo,
    3, 0,  1,2,3, lo, HiX_2,    HiY_2, HiY_2, lo, lo,
    4, 2,  1,2,3, HiX_2, HiX_2, HiY_2, HiY_2, lo, lo,
    5, 3,  1,2,3, HiX_2, HiX_2, HiY_2, HiY_2, lo, lo,
/* Domain 3 */
    2, 2,  1,2,3, lo, HiX_3,    lo, lo,       lo, lo,
    5, 0,  1,2,3, HiX_3, HiX_3, lo, HiY_3,    lo, lo,
    1, 3,  1,2,3, HiX_3, HiX_3, lo, lo,       lo, lo,
    4, 3,  1,2,3, HiX_3, HiX_3, lo, lo,       lo, lo,
/* Domain 4 */
    1, 2,  1,2,3, lo, lo,       lo, HiY_4,    lo, lo,
    5, 1,  1,2,3, lo, HiX_4,    HiY_4, HiY_4, lo, lo,
    2, 3,  1,2,3, lo, lo,       HiY_4, HiY_4, lo, lo,
    3, 3,  1,2,3, lo, lo,       HiY_4, HiY_4, lo, lo,
/* Domain 5 */
    3, 1,  1,2,3, lo, lo,       lo, HiY_5,    lo, lo,
    4, 1,  1,2,3, lo, HiX_5,    lo, lo,       lo, lo,
    1, 4,  1,2,3, lo, lo,       lo, lo,       lo, lo,
    2, 4,  1,2,3, lo, lo,       lo, lo,       lo, lo
};

/* The number of boundaries in each domain's boundary list. */
int nneighbors[NDOMAINS] = {2, 5, 5, 4, 4, 4};

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
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
        visit_handle mmd = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;
        visit_handle cmd = VISIT_INVALID_HANDLE;
        visit_handle emd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "mesh2d");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, NDOMAINS);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");

            VisIt_MeshMetaData_setNumGroups(mmd, NBLOCKS);
            VisIt_MeshMetaData_setGroupTitle(mmd, "Blocks");
            VisIt_MeshMetaData_setGroupPieceName(mmd, "block");
            for(i = 0; i < NDOMAINS; ++i)
                VisIt_MeshMetaData_addGroupId(mmd, block_id[i]);

            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "domainID");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
            
        /* Add some commands. */
        for(i = 0; i < sizeof(cmd_names)/sizeof(const char *); ++i)
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

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "mesh2d") == 0)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int nTuples;
            visit_handle hxy;
            nTuples = mesh_dims[domain][0] * mesh_dims[domain][1];

            VisIt_VariableData_alloc(&hxy);
            VisIt_VariableData_setDataF(hxy, VISIT_OWNER_SIM, 2, nTuples, 
                                        mesh_coords[domain]);
            VisIt_CurvilinearMesh_setCoords2(h, mesh_dims[domain], hxy);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Aug  9 15:44:34 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "domainID") == 0)
    {
        double *domainID = NULL;
        int i, nTuples;

        /* Fill in the domain id. */
        nTuples = (mesh_dims[domain][0]-1)*(mesh_dims[domain][1]-1);
        domainID = (double*)malloc(sizeof(double) * nTuples);
        for(i = 0; i < nTuples; ++i)
            domainID[i] = (double)domain;

        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
            nTuples, domainID);
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Aug  8 13:57:40 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
    {
        visit_handle hdl;
        int *iptr = NULL, i, nTuples = 0;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int) * NDOMAINS);
        for(i = 0; i < NDOMAINS; ++i)
        {
            if(i % sim->par_size == sim->par_rank)
                iptr[nTuples++] = i;
        }

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, nTuples, iptr);
        VisIt_DomainList_setDomains(h, NDOMAINS, hdl);
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain boundaries object.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Aug 13 11:42:47 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetDomainBoundaries(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Note: VisIt currently relies on domain boundaries objects that hold the
     *       information for all domains in the problem, even ones not held
     *       on this processor. We're working to correct that. In the meantime,
     *       we construct the global domain boundaries info for each domain.
     */
    if(VisIt_DomainBoundaries_alloc(&h) != VISIT_ERROR)
    {
        int dom, k, offset = 0;

        /* Set the type. 0=rectilinear, 1=curvilinear. */
        VisIt_DomainBoundaries_set_type(h, 1);

        /* Set the total number of domains in the problem. */
        VisIt_DomainBoundaries_set_numDomains(h, NDOMAINS);

        for(dom = 0; dom < NDOMAINS; ++dom)
        {
            /* Number of zones in this domain. */
            int extents[6];
            extents[0] = 0;
            extents[1] = mesh_dims[dom][0]-1;
            extents[2] = 0;
            extents[3] = mesh_dims[dom][1]-1;
            extents[4] = 0;
            extents[5] = mesh_dims[dom][2]-1;

            /* Set the domain's extents. */
            VisIt_DomainBoundaries_set_extents(h, dom, extents);

            /* Add the domain's neighbors. */
            for(k = 0; k < nneighbors[dom]; ++k)
            {
                const int *n = neighbors + offset;
                VisIt_DomainBoundaries_add_neighbor(h, dom, n[0], n[1],
                                                    &n[2], &n[5]);
                offset += 11;
            }

            /* We're done adding neighbors for this domain. */
            VisIt_DomainBoundaries_finish(h, dom);
        }
    }

    return h;
}
