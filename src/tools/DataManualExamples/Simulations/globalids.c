/*****************************************************************************
*
* Copyright (c) 2000 - 2014, Lawrence Livermore National Security, LLC
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

#define FREE(ptr) if(ptr != NULL){free(ptr); ptr = NULL;}

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetCurve(const char *name, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Mesh functions
 ******************************************************************************/

/* Unstructured mesh */
typedef struct
{
    int   nnodes;
    int   ncells;
    float extents[6];
    float *xyz;
    int   connectivityLen;
    int   *connectivity;
    int   *globalNodeIds;
    int   *globalCellIds;
    float *data;
} ucdmesh;

void
ucdmesh_ctor(ucdmesh *m)
{
    memset(m, 0, sizeof(ucdmesh));
}

void
ucdmesh_dtor(ucdmesh *m)
{
    FREE(m->xyz);
    FREE(m->connectivity);
    FREE(m->globalNodeIds);
    FREE(m->globalCellIds);
    FREE(m->data);
}

void
ucdmesh_init(float ext[6], int nx, int ny, int nz)
{
}

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
#ifdef PARALLEL
    MPI_Comm        par_comm;
#endif
    int             par_rank;
    int             par_size;
    int             cycle;
    double          time;
    int             runMode;
    int             done;
    int             echo;

    int             nx,ny,nz;
    float           extents[6];
    int             ijk_split[3];
    int             owns[8];
    ucdmesh         domains[8];
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    int i;

    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->echo = 0;

    sim->nx = 101;
    sim->ny = 101;
    sim->nz = 101;
    sim->extents[0] = 0.f;
    sim->extents[1] = 0.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = 1.f;
    sim->extents[4] = 1.f;
    sim->extents[5] = 1.f;
    sim->ijk_split[0] = sim->nx / 2;
    sim->ijk_split[1] = sim->ny / 2;
    sim->ijk_split[2] = sim->nz / 2;

    for(i = 0; i < 8; ++i)
        ucdmesh_ctor(&sim->domains[i]);
}

void
simulation_data_dtor(simulation_data *sim)
{
    int i;

    for(i = 0; i < 8; ++i)
        ucdmesh_dtor(&sim->domains[i]);
}

const int dom2i[8] = {0,1,0,1,0,1,0,1};
const int dom2j[8] = {0,0,1,1,0,0,1,1};
const int dom2k[8] = {0,0,0,0,1,1,1,1};

void
simulation_data_update(simulation_data *sim)
{
    float angle,rad,x,y,z,cx,cy,cz,dx,dy,dz,sx,sy,sz;
    int d,i,j,k, idx, ijk_start[3], ijk_end[3],nx,ny,nz;

    /* Get rid of the old meshes. */
    for(i = 0; i < 8; ++i)
        ucdmesh_dtor(&sim->domains[i]);

    /* Determine which meshes we'll work on based on the rank. */
    for(i = 0; i < 8; i++)
    {
        int owner_of_domain = i % sim->par_size;
        if(sim->par_rank == owner_of_domain)
            sim->owns[i] = 1;
        else
            sim->owns[i] = 0;
    }

    angle = sim->time;

    /* We're going to move the ijk_split node around to determine where the
     * domain split occurs in the mesh.
     */
    dx = sim->extents[3] - sim->extents[0];
    dy = sim->extents[4] - sim->extents[1];
    dz = sim->extents[5] - sim->extents[2];
    rad = sqrt(dx*dx + dz*dz) / 10.;
    sx = ((sim->extents[0] + sim->extents[3]) / 2) + rad * cos(angle);
    sy = ((sim->extents[1] + sim->extents[4]) / 2);
    sz = ((sim->extents[2] + sim->extents[5]) / 2) + rad * sin(angle);

    /* We have a new x,y,z. Figure out where that is in ijk. */
    i = (int)(sim->nx * ((sx - sim->extents[0]) / dx));
    j = (int)(sim->ny * ((sy - sim->extents[1]) / dy));
    k = (int)(sim->nz * ((sz - sim->extents[2]) / dz));
    sim->ijk_split[0] = i;
    sim->ijk_split[1] = j;
    sim->ijk_split[2] = k;

    /* Now, make new domains according to the ijk split. */
    cx = dx / ((float)(sim->nx-1));
    cy = dy / ((float)(sim->ny-1));
    cz = dz / ((float)(sim->nz-1));
    for(d = 0; d < 8; ++d)
    {
        ucdmesh *dom = &sim->domains[d];

        /* See if this processor owns the domain. */
        if(sim->owns[d] == 0)
            continue;

        /* Figure node boundaries for this domain within "whole".*/
        ijk_start[0] = (dom2i[d] == 0) ? 0 : sim->ijk_split[0];
        ijk_start[1] = (dom2j[d] == 0) ? 0 : sim->ijk_split[1];
        ijk_start[2] = (dom2k[d] == 0) ? 0 : sim->ijk_split[2];
        ijk_end[0]   = (dom2i[d] == 0) ? sim->ijk_split[0] : (sim->nx-1);
        ijk_end[1]   = (dom2j[d] == 0) ? sim->ijk_split[1] : (sim->ny-1);
        ijk_end[2]   = (dom2k[d] == 0) ? sim->ijk_split[2] : (sim->nz-1);

        /* Extents for this domain. */
        dom->extents[0] = ((float)ijk_start[0]) * cx;
        dom->extents[1] = ((float)ijk_start[1]) * cy;
        dom->extents[2] = ((float)ijk_start[2]) * cz;
        dom->extents[3] = ((float)ijk_end[0]) * cx;
        dom->extents[4] = ((float)ijk_end[1]) * cy;
        dom->extents[5] = ((float)ijk_end[2]) * cz;

        /* nnodes for this domain*/
        nx = ijk_end[0] - ijk_start[0] + 1;
        ny = ijk_end[1] - ijk_start[1] + 1;
        nz = ijk_end[2] - ijk_start[2] + 1;

        /* Populate mesh and data for this domain. */
        dom->nnodes = nx * ny * nz;
        dom->ncells = (nx-1)*(ny-1)*(nz-1);
        dom->xyz = (float *)malloc(3 * dom->nnodes * sizeof(float));
        dom->connectivityLen = 9 * dom->ncells;
        dom->connectivity = (int *)malloc(dom->connectivityLen * sizeof(int));
        dom->globalNodeIds = (int *)malloc(dom->nnodes * sizeof(int));
        dom->globalCellIds = (int *)malloc(dom->ncells * sizeof(int));
        dom->data = (float *)malloc(dom->nnodes * sizeof(float));
        idx = 0;
        for(k = ijk_start[2]; k <= ijk_end[2]; ++k)
        {
            z = sim->extents[2] + ((float)k) * cz;
            for(j = ijk_start[1]; j <= ijk_end[1]; ++j)
            {
                y = sim->extents[1] + ((float)j) * cy;
                for(i = ijk_start[0]; i <= ijk_end[0]; ++i)
                {
                    x = sim->extents[0] + ((float)i) * cx;

                    dom->globalNodeIds[idx] = k*sim->nx*sim->ny + j*sim->nx + i;

                    dom->xyz[3*idx+0] = x;
                    dom->xyz[3*idx+1] = y;
                    dom->xyz[3*idx+2] = z;

                    dom->data[idx] = sqrt((sx-x)*(sx-x) + (sy-y)*(sy-y) + (sz-z)*(sz-z));

                    ++idx;
                }
            }
        }

#define LOCAL_NODE(I,J,K) ((K)*nx*ny + (J)*nx + (I))
        /* Connectivity, etc */
        idx = 0;
        for(k = 0; k < nz-1; ++k)
        {
            for(j = 0; j < ny-1; ++j)
            {
                for(i = 0; i < nx-1; ++i)
                {
                    /*local cell connectivity*/
                    int ii,jj,kk, idx9 = idx*9;
                    dom->connectivity[idx9+0] = VISIT_CELL_HEX;
                    dom->connectivity[idx9+1] = LOCAL_NODE(i  ,j  ,k+1);
                    dom->connectivity[idx9+2] = LOCAL_NODE(i+1,j  ,k+1);
                    dom->connectivity[idx9+3] = LOCAL_NODE(i+1,j  ,k);
                    dom->connectivity[idx9+4] = LOCAL_NODE(i  ,j  ,k);
                    dom->connectivity[idx9+5] = LOCAL_NODE(i  ,j+1,k+1);
                    dom->connectivity[idx9+6] = LOCAL_NODE(i+1,j+1,k+1);
                    dom->connectivity[idx9+7] = LOCAL_NODE(i+1,j+1,k);
                    dom->connectivity[idx9+8] = LOCAL_NODE(i  ,j+1,k);

                    /* global cell id */
                    ii = i + ijk_start[0];
                    jj = j + ijk_start[1];
                    kk = k + ijk_start[2];
                    dom->globalCellIds[idx] = kk*(sim->nx-1)*(sim->ny-1) + jj*(sim->nx-1) + ii;

                    idx++;
                }
            }
        }
    }
}

const char *cmd_names[] = {"halt", "step", "run"};

/******************************************************************************
 * Functions to really populate data
 ******************************************************************************/


/******************************************************************************
 *
 * Purpose: Create the data that the simulation will use.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
read_input_deck(simulation_data *sim)
{
}

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    simulation_data_update(sim);

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

    if(sim->echo && sim->par_rank == 0)
    {
        fprintf(stderr, "Command %s completed.\n", cmd);
        fflush(stderr);
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
 * Date:       Thu Nov 2 17:26:08 PST 2006
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
 * Date:       Fri Aug 12 15:05:36 PDT 2011
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
    int i;
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

    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-echo") == 0)
            sim.echo = 1;
    }

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
            "globalids_par",
#else
            "globalids",
#endif
            "Demonstrate global node and cell ids on an unstructured mesh",
            "/path/to/where/sim/was/started",
            NULL, NULL, SimulationFilename());
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck(&sim);

    /* Call the main loop. */
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
 * Date:       Fri Jun 13 13:47:15 PDT 2014
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
        double extents[6];

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
            VisIt_MeshMetaData_setName(mmd, "mesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 8);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "radial");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);

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
 * Date:       Fri Jun 13 13:49:52 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "mesh") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxyz, hc, gln, glc;

            VisIt_VariableData_alloc(&hxyz);
            VisIt_VariableData_setDataF(hxyz, VISIT_OWNER_SIM, 3, sim->domains[domain].nnodes, sim->domains[domain].xyz);
            VisIt_UnstructuredMesh_setCoords(h, hxyz);

            VisIt_VariableData_alloc(&hc);
            VisIt_VariableData_setDataI(hc, VISIT_OWNER_SIM, 1, sim->domains[domain].connectivityLen,
                sim->domains[domain].connectivity);
            VisIt_UnstructuredMesh_setConnectivity(h, sim->domains[domain].ncells, hc);

            /* Global Node Ids */
            VisIt_VariableData_alloc(&gln);
            VisIt_VariableData_setDataI(gln, VISIT_OWNER_SIM, 1, 
                sim->domains[domain].nnodes, sim->domains[domain].globalNodeIds);
            VisIt_UnstructuredMesh_setGlobalNodeIds(h, gln);

            /* Global Cell Ids */
            VisIt_VariableData_alloc(&glc);
            VisIt_VariableData_setDataI(glc, VISIT_OWNER_SIM, 1, 
                sim->domains[domain].ncells, sim->domains[domain].globalCellIds);
            VisIt_UnstructuredMesh_setGlobalCellIds(h, glc);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 13 13:49:52 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "radial") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            sim->domains[domain].nnodes, sim->domains[domain].data);
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 13 13:49:52 PDT 2014
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
        int i, *iptr = NULL, dcount = 0;
        int ndoms = 8;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(ndoms * sizeof(int));
        memset(iptr, 0, ndoms * sizeof(int));

        for(i = 0; i < ndoms; i++)
        {
            if(sim->owns[i] == 1)
                iptr[dcount++] = i;
        }

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, dcount, iptr);
        VisIt_DomainList_setDomains(h, ndoms, hdl);
    }
    return h;
}
