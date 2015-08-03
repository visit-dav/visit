/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "extract.h"
#include "SimulationExample.h"

#define SPHERICAL_COORDINATES
#define FREE(PTR) if((PTR)!=NULL){ free(PTR); PTR = NULL; }

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Communication Function prototypes */
int SimBroadcastInt(int *value, int sender, void *cbdata);
int SimBroadcastString(char *str, int len, int sender, void *cbdata);
void SimSlaveProcessCallback(void *);

/* Data Adaptor Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

/* ArrayOfStructures */ 
typedef struct
{
    double x,y,z;
    float  temperature;
    double vx,vy,vz;
    int    I,J,K;
} Node;

typedef struct
{
    Node *nodes;
    int   nnodes;
} ArrayOfStructures;

void ArrayOfStructures_ctor(ArrayOfStructures *obj, int nnodes)
{
    obj->nnodes = nnodes;
    obj->nodes = (Node *)malloc(nnodes * sizeof(Node));
}

void ArrayOfStructures_dtor(ArrayOfStructures *obj)
{
    FREE(obj->nodes);
    obj->nnodes = 0;
}

/* StructureOfArrays */ 
typedef struct
{
    double *x, *y, *z;
    double *xyz; /* for baseline case with contiguous coords. */
    float  *temperature;
    double *vx,*vy,*vz;
    int    *I,*J,*K;
    int    nnodes;
} StructureOfArrays;

void StructureOfArrays_ctor(StructureOfArrays *obj, int nnodes)
{
    obj->nnodes = nnodes;
    obj->x = (double *)malloc(nnodes * sizeof(double));
    obj->y = (double *)malloc(nnodes * sizeof(double));
    obj->z = (double *)malloc(nnodes * sizeof(double));
    obj->xyz = (double *)malloc(3 * nnodes * sizeof(double));
    obj->temperature = (float *)malloc(nnodes * sizeof(float));
    obj->vx = (double *)malloc(nnodes * sizeof(double));
    obj->vy = (double *)malloc(nnodes * sizeof(double));
    obj->vz = (double *)malloc(nnodes * sizeof(double));
    obj->I = (int *)malloc(nnodes * sizeof(int));
    obj->J = (int *)malloc(nnodes * sizeof(int));
    obj->K = (int *)malloc(nnodes * sizeof(int));
}

void StructureOfArrays_dtor(StructureOfArrays *obj)
{
    FREE(obj->x);
    FREE(obj->y);
    FREE(obj->z);
    FREE(obj->xyz);
    FREE(obj->temperature);
    FREE(obj->vx);
    FREE(obj->vy);
    FREE(obj->vz);
    FREE(obj->I);
    FREE(obj->J);
    FREE(obj->K);
    obj->nnodes = 0;
}

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
    int      dims[3];
    float    extents[6];
    int      echo;
    ArrayOfStructures aos;
    StructureOfArrays soa;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim, int nx, int ny, int nz)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->dims[0] = nx;
    sim->dims[1] = ny;
    sim->dims[2] = nz;
#ifdef SPHERICAL_COORDINATES
    sim->extents[0] = 0.0001f;
    sim->extents[1] = 1.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = M_PI;
    sim->extents[4] = 0.f;
    sim->extents[5] = M_PI;
#else
    sim->extents[0] = 0.f;
    sim->extents[1] = 4.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = 4.f;
    sim->extents[4] = 0.f;
    sim->extents[5] = 4.f;
#endif
    sim->echo = 0;
    ArrayOfStructures_ctor(&sim->aos, sim->dims[0]*sim->dims[1]*sim->dims[2]);
    StructureOfArrays_ctor(&sim->soa, sim->dims[0]*sim->dims[1]*sim->dims[2]);
}

void
simulation_data_dtor(simulation_data *sim)
{
    ArrayOfStructures_dtor(&sim->aos);
    StructureOfArrays_dtor(&sim->soa);
}

void 
simulation_data_update(simulation_data *sim)
{
    double x,y,z,sx,sy,sz,r,tx,ty,tz,offset,scale;
    int index, i,j,k;
#ifdef SPHERICAL_COORDINATES
    double theta,phi;
#endif
    if(sim->par_rank == 0)
    {
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
        fflush(stdout);
    }

    /* Update values. */
    offset = (sim->extents[1] - sim->extents[0]) * sim->par_rank;
    index = 0;
    for(k = 0; k < sim->dims[2]; ++k)
    {
        tz = ((double)k) / ((double)(sim->dims[2] - 1));
        z = (1.f-tz)*sim->extents[4] + tz*sim->extents[5];
        for(j = 0; j < sim->dims[1]; ++j)
        {
            ty = ((double)j) / ((double)(sim->dims[1] - 1));
            y = (1.f-ty)*sim->extents[2] + ty*sim->extents[3];
            for(i = 0; i < sim->dims[0]; ++i)
            {
                tx = ((double)i) / ((double)(sim->dims[0] - 1));
                x = (1.f-tx)*sim->extents[0] + tx*sim->extents[1];
#ifdef SPHERICAL_COORDINATES
                r = x + offset;
                theta = z + sim->time;
                phi = y;

                sx = r * cos(theta) * sin(phi);
                sy = r * sin(theta) * sin(phi);
                sz = r * cos(phi);
#else
                sx = x + offset;
                sy = y;
                sz = z;
#endif
                r = sqrt(sx*sx + sy*sy + sz*sz);
                scale = (r == 0.) ? 1. : (1. / r);

                /* Save AOS version */
                sim->aos.nodes[index].x = sx;
                sim->aos.nodes[index].y = sy;
                sim->aos.nodes[index].z = sz;
                sim->aos.nodes[index].temperature = r;
                sim->aos.nodes[index].vx = sx * scale;
                sim->aos.nodes[index].vy = sy * scale;
                sim->aos.nodes[index].vz = sz * scale;
                sim->aos.nodes[index].I = i;
                sim->aos.nodes[index].J = j;
                sim->aos.nodes[index].K = k;

                /* Save SOA version */
                sim->soa.x[index] = sx;
                sim->soa.y[index] = sy;
                sim->soa.z[index] = sz;
                sim->soa.xyz[index*3+0] = sx;
                sim->soa.xyz[index*3+1] = sy;
                sim->soa.xyz[index*3+2] = sz;
                sim->soa.temperature[index] = r;
                sim->soa.vx[index] = sx * scale;
                sim->soa.vy[index] = sy * scale;
                sim->soa.vz[index] = sz * scale;
                sim->soa.I[index] = i;
                sim->soa.J[index] = j;
                sim->soa.K[index] = k;

                ++index;
            }
        }
    }
}

void
simulation_advance(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 10.);
}

/* Callback function for control commands, which are the buttons in the 
 * GUI's Simulation window. This type of command is handled automatically
 * provided that you have registered a command callback such as this.
 */
void SimControlCommandCallback(const char *cmd, const char *args, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(cmd, "quit") == 0)
        sim->done = 1;
    else if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
    {
        simulation_data_update(sim);
        simulation_advance(sim);

        VisItTimeStepChanged();
        VisItUpdatePlots();
    }
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;

    if(sim->echo && sim->par_rank == 0)
    {
        fprintf(stderr, "Command '%s' completed.\n", cmd);
        fflush(stderr);
    }
}

void
SetupCallbacks(simulation_data *sim)
{
    VisItSetCommandCallback(SimControlCommandCallback, (void*)sim);
    VisItSetSlaveProcessCallback2(SimSlaveProcessCallback, (void*)sim);
    VisItSetGetMetaData(SimGetMetaData, (void*)sim);
    VisItSetGetMesh(SimGetMesh, (void*)sim);
    VisItSetGetVariable(SimGetVariable, (void*)sim);
    VisItSetGetDomainList(SimGetDomainList, (void*)sim);
}

/******************************************************************************
 *
 * Function: mainloop_batch
 *
 * Purpose: The batch version of the main loop. It iterates and makes extracts.
 *
 * Programmer: Brad Whitlock
 * Date:      Tue Feb  3 13:10:23 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop_batch(simulation_data *sim)
{
    char filebase[100];
    const char *extractvars[] = {"q", "xc", "radius", NULL};
    double origin[] = {5., 5., 5.}, normal[] = {0., 0.707, 0.707};
    double isos[] = {0.2, 0.5, 0.8};
    double v0[] = {1.,1.,1.}, v1[] = {5., 1.5, 7.}, v2[] = {8., 2., 5.};
    
    /* Explicitly load VisIt runtime functions and install callbacks. */
    VisItInitializeRuntime();
    SetupCallbacks(sim);

    while(!sim->done)
    {  
        /* Update the simulation data for this iteration.*/     
        simulation_data_update(sim);

        /* Tell VisIt that some metadata changed.*/
        VisItTimeStepChanged();

        /* Make some extracts. */
        sprintf(filebase, "slice3v_%04d", sim->cycle);
        extract_slice_3v(filebase, v0, v1, v2, extractvars);

        sprintf(filebase, "sliceON_%04d", sim->cycle);
        extract_slice_origin_normal(filebase, origin, normal, extractvars);

        sprintf(filebase, "sliceX_%04d", sim->cycle);
        extract_slice(filebase, 0, 0.5, extractvars);

        sprintf(filebase, "sliceY_%04d", sim->cycle);
        extract_slice(filebase, 1, 2.5, extractvars);

        sprintf(filebase, "sliceZ_%04d", sim->cycle);
        extract_slice(filebase, 2, 5., extractvars);

        sprintf(filebase, "iso_%04d", sim->cycle);
        extract_iso(filebase, "q", isos, 3, extractvars);

        simulation_advance(sim);
    }
}

/* Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 */
void
mainloop_interactive_console_command(simulation_data *sim)
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

    SimControlCommandCallback(cmd, NULL, sim);
}

/******************************************************************************
 *
 * Function: mainloop_interactive
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
int SimProcessVisItCommand(simulation_data *sim);

void mainloop_interactive(simulation_data *sim)
{
    int blocking, visitstate = 0, err = 0;

    /* If we're not running by default then simulate once there's something
     * once VisIt connects.
     */
    if(sim->runMode == SIM_STOPPED)
        simulation_data_update(sim);

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
            simulation_data_update(sim);
            simulation_advance(sim);

            VisItTimeStepChanged();
            VisItUpdatePlots();
            break;
        case 1:
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
            {
                fprintf(stderr, "VisIt connected\n");
                /* Install callbacks */
                SetupCallbacks(sim);
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
            if(!SimProcessVisItCommand(sim))
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
            mainloop_interactive_console_command(sim);
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
 * Date:       Tue Feb  3 14:53:45 PST 2015
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
    int i,nx = 50, ny = 50,nz = 50, batch = 0, echo = 0;
    char *env = NULL;
    simulation_data sim;

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
#endif

    /* Check for command line arguments. */
    for(i = 1; i < argc; ++i)
    {
        if((i+1) < argc)
        {
            if(strcmp(argv[i], "-nx") == 0)
            {
                sscanf(argv[i+1], "%d", &nx);
                i++;
            }
            else if(strcmp(argv[i], "-ny") == 0)
            {
                sscanf(argv[i+1], "%d", &ny);
                i++;
            }
            else if(strcmp(argv[i], "-nz") == 0)
            {
                sscanf(argv[i+1], "%d", &nz);
                i++;
            }
            else if(strcmp(argv[i], "-dir") == 0)
            {
                /* Specify the path to VisIt installation. */
                VisItSetDirectory(argv[i+1]);
                i++;
            }
        }
        else if(strcmp(argv[i], "-batch") == 0)
        {
            batch = 1;
        }
        else if(strcmp(argv[i], "-echo") == 0)
        {
            echo = 1;
        }
    }

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);

    simulation_data_ctor(&sim, nx, ny, nz);
    sim.echo = echo;

#ifdef PARALLEL
    /* Create a new communicator. */
    if (MPI_Comm_dup(MPI_COMM_WORLD, &sim.par_comm) != MPI_SUCCESS)
        sim.par_comm = MPI_COMM_WORLD;
    MPI_Comm_rank (sim.par_comm, &sim.par_rank);
    MPI_Comm_size (sim.par_comm, &sim.par_size);

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction2(SimBroadcastInt, (void*)&sim);
    VisItSetBroadcastStringFunction2(SimBroadcastString, (void*)&sim);

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

    /* Call the main loop. */
    if(batch)
        mainloop_batch(&sim);
    else
    {
        if(sim.par_rank == 0)
        {
            /* Write out .sim file that VisIt uses to connect. */
            VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "zerocopy_par",
#else
            "zerocopy",
#endif
            "Demonstrates VariableData setArrayData functions for zero copy",
            "/path/to/where/sim/was/started",
            NULL, NULL, SimulationFilename());
        }

        mainloop_interactive(&sim);
    }

    if(VisItIsConnected())
        VisItDisconnect();

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/*******************************************************************************
********************************************************************************
*****                           ADAPTOR FUNCTIONS
********************************************************************************
*******************************************************************************/

/* COMMUNICATION ADAPTOR FUNCTIONS. */

#ifdef PARALLEL
int SimBroadcastInt(int *value, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(value, 1, MPI_INT, sender, sim->par_comm);
}

int SimBroadcastString(char *str, int len, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(str, len, MPI_CHAR, sender, sim->par_comm);
}
#endif

void SimSlaveProcessCallback(void *cbdata)
{
#ifdef PARALLEL
    simulation_data *sim = (simulation_data *)cbdata;
    int command = VISIT_COMMAND_PROCESS;
    MPI_Bcast(&command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* Helper function for SimProcessVisItCommand */
static void BroadcastSlaveCommand(int *command, simulation_data *sim)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* Process commands from viewer on all processors. */
int SimProcessVisItCommand(simulation_data *sim)
{
    int command = VISIT_COMMAND_PROCESS;
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
/* DATA ADAPTOR FUNCTIONS */

/* soa_s means "structure of arrays with coordinates made of single variabledata object." 
   soa_m means "structure of arrays with coordinates made of multiple variabledata objects."
*/
const char *cmd_names[] = {"halt", "step", "run"};
const char *meshes[] = {"soa_s", "soa_m", "soa_i", "aos_s", "aos_m", "aos_c"};
const char *scalars[] = {"temperature", "vx", "vy", "vz", "I", "J", "K"};
const char *vectors[] = {"velocity"};

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 17 15:54:51 PDT 2015
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
        int i,j;
        visit_handle mmd = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        for(i = 0; i < sizeof(meshes)/sizeof(const char*); ++i)
        {
            /* Add mesh metadata. */
            if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
            {
                /* Set the mesh's properties.*/
                VisIt_MeshMetaData_setName(mmd, meshes[i]);
                VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR);
                VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
                VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
                VisIt_MeshMetaData_setNumDomains(mmd, sim->par_size);
                VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
                VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
                VisIt_MeshMetaData_setNumGroups(mmd, 0);
                VisIt_MeshMetaData_setXUnits(mmd, "cm");
                VisIt_MeshMetaData_setYUnits(mmd, "cm");
                VisIt_MeshMetaData_setZUnits(mmd, "cm");
                VisIt_MeshMetaData_setXLabel(mmd, "Width");
                VisIt_MeshMetaData_setYLabel(mmd, "Height");
                VisIt_MeshMetaData_setZLabel(mmd, "Depth");

                VisIt_SimulationMetaData_addMesh(md, mmd);
            }

            /* Add scalars. */
            for(j = 0; j < sizeof(scalars)/sizeof(const char*); ++j)
            {
                if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
                {
                    char varname[100];
                    sprintf(varname, "%s/%s", meshes[i], scalars[j]);

                    VisIt_VariableMetaData_setName(vmd, varname);
                    VisIt_VariableMetaData_setMeshName(vmd, meshes[i]);
                    VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
                    VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
                    VisIt_SimulationMetaData_addVariable(md, vmd);
                }
            }

            /* Add vectors. */
            for(j = 0; j < sizeof(vectors)/sizeof(const char*); ++j)
            {
                if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
                {
                    char varname[100];
                    sprintf(varname, "%s/%s", meshes[i], vectors[j]);

                    VisIt_VariableMetaData_setName(vmd, varname);
                    VisIt_VariableMetaData_setMeshName(vmd, meshes[i]);
                    VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_VECTOR);
                    VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
                    VisIt_SimulationMetaData_addVariable(md, vmd);
                }
            }
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
 * Date:       Fri Jul 17 15:54:51 PDT 2015
 *
 * Modifications:
 *
 *****************************************************************************/

int serror(int simerr)
{
    if(simerr == VISIT_ERROR)
        printf("error: %s\n", VisItGetLastError());
    return simerr;
}

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
    {
        visit_handle hx, hy, hz, hxyz;

        if(strcmp(name, "soa_s") == 0)
        {
            /* Set coordinates using a single variable data made from SOA data. */
            serror(VisIt_VariableData_alloc(&hxyz));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 0, VISIT_OWNER_SIM, sim->soa.nnodes, 0, sizeof(double), sim->soa.x));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 1, VISIT_OWNER_SIM, sim->soa.nnodes, 0, sizeof(double), sim->soa.y));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 2, VISIT_OWNER_SIM, sim->soa.nnodes, 0, sizeof(double), sim->soa.z));
            serror(VisIt_CurvilinearMesh_setCoords3(h, sim->dims, hxyz));
        }
        else if(strcmp(name, "soa_m") == 0)
        {
            /* Set coordinates using 3 variable datas made from SOA data. */
            serror(VisIt_VariableData_alloc(&hx));
            serror(VisIt_VariableData_alloc(&hy));
            serror(VisIt_VariableData_alloc(&hz));
            serror(VisIt_VariableData_setDataD(hx, VISIT_OWNER_SIM, 1, sim->soa.nnodes, sim->soa.x));
            serror(VisIt_VariableData_setDataD(hy, VISIT_OWNER_SIM, 1, sim->soa.nnodes, sim->soa.y));
            serror(VisIt_VariableData_setDataD(hz, VISIT_OWNER_SIM, 1, sim->soa.nnodes, sim->soa.z));
            serror(VisIt_CurvilinearMesh_setCoordsXYZ(h, sim->dims, hx, hy, hz));
        }
        else if(strcmp(name, "soa_i") == 0)
        {
            /* Set coordinates using a single variable data made from SOA interleaved data. */
            serror(VisIt_VariableData_alloc(&hxyz));
            serror(VisIt_VariableData_setDataD(hxyz, VISIT_OWNER_SIM, 3, sim->soa.nnodes, sim->soa.xyz));
            serror(VisIt_CurvilinearMesh_setCoords3(h, sim->dims, hxyz));
        }
        else if(strcmp(name, "aos_s") == 0)
        {
            /* Set coordinates using a single variable data made from AOS data. */
            serror(VisIt_VariableData_alloc(&hxyz));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 0, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].x)));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 1, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].y)));
            serror(VisIt_VariableData_setArrayDataD(hxyz, 2, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].z)));
            serror(VisIt_CurvilinearMesh_setCoords3(h, sim->dims, hxyz));
        }
        else if(strcmp(name, "aos_m") == 0)
        {
            /* Set coordinates using 3 variable datas made from AOS data. */
            serror(VisIt_VariableData_alloc(&hx));
            serror(VisIt_VariableData_alloc(&hy));
            serror(VisIt_VariableData_alloc(&hz));
            serror(VisIt_VariableData_setArrayDataD(hx, 0, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].x)));
            serror(VisIt_VariableData_setArrayDataD(hy, 0, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].y)));
            serror(VisIt_VariableData_setArrayDataD(hz, 0, VISIT_OWNER_SIM, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].z)));
            serror(VisIt_CurvilinearMesh_setCoordsXYZ(h, sim->dims, hx, hy, hz));
        }
        else if(strcmp(name, "aos_c") == 0)
        {
            /* Set coordinates using 3 variable datas made from AOS data. Let's COPY data */
            serror(VisIt_VariableData_alloc(&hx));
            serror(VisIt_VariableData_alloc(&hy));
            serror(VisIt_VariableData_alloc(&hz));
            serror(VisIt_VariableData_setArrayDataD(hx, 0, VISIT_OWNER_COPY, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].x)));
            serror(VisIt_VariableData_setArrayDataD(hy, 0, VISIT_OWNER_COPY, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].y)));
            serror(VisIt_VariableData_setArrayDataD(hz, 0, VISIT_OWNER_COPY, sim->aos.nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].z)));
            serror(VisIt_CurvilinearMesh_setCoordsXYZ(h, sim->dims, hx, hy, hz));
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 17 15:54:51 PDT 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strncmp(name, "aos", 3) == 0)
    {
        int nnodes, owner;
        owner = (strcmp(name, "aos_c") == 0) ? VISIT_OWNER_COPY : VISIT_OWNER_SIM;
        nnodes = sim->aos.nnodes;
        /* Use setDataArray functions to set up strided array access. */
        VisIt_VariableData_alloc(&h);
        if(strcmp(name+6, "temperature") == 0)
            serror(VisIt_VariableData_setArrayDataF(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].temperature)));
        else if(strcmp(name+6, "vx") == 0)
            serror(VisIt_VariableData_setArrayDataD(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vx)));
        else if(strcmp(name+6, "vy") == 0)
            serror(VisIt_VariableData_setArrayDataD(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vy)));
        else if(strcmp(name+6, "vz") == 0)
            serror(VisIt_VariableData_setArrayDataD(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vz)));
        else if(strcmp(name+6, "I") == 0)
            serror(VisIt_VariableData_setArrayDataI(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].I)));
        else if(strcmp(name+6, "J") == 0)
            serror(VisIt_VariableData_setArrayDataI(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].J)));
        else if(strcmp(name+6, "K") == 0)
            serror(VisIt_VariableData_setArrayDataI(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].K)));
        else if(strcmp(name+6, "velocity") == 0)
        {
            /* Combine separate strided array access into a vector. */
            serror(VisIt_VariableData_setArrayDataD(h, 0, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vx)));
            serror(VisIt_VariableData_setArrayDataD(h, 1, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vy)));
            serror(VisIt_VariableData_setArrayDataD(h, 2, owner, nnodes, 0, sizeof(Node), &(sim->aos.nodes[0].vz)));
        }
    }
    else if(strncmp(name, "soa", 3) == 0)
    {
        int nnodes = sim->soa.nnodes;
        /* Use setData functions to set up contiguous array access. */
        VisIt_VariableData_alloc(&h);
        if(strcmp(name+6, "temperature") == 0)
            serror(VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.temperature));
        else if(strcmp(name+6, "vx") == 0)
            serror(VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.vx));
        else if(strcmp(name+6, "vy") == 0)
            serror(VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.vy));
        else if(strcmp(name+6, "vz") == 0)
            serror(VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.vz));
        else if(strcmp(name+6, "I") == 0)
            serror(VisIt_VariableData_setDataI(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.I));
        else if(strcmp(name+6, "J") == 0)
            serror(VisIt_VariableData_setDataI(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.J));
        else if(strcmp(name+6, "K") == 0)
            serror(VisIt_VariableData_setDataI(h, VISIT_OWNER_SIM, 1, nnodes, sim->soa.K));
        else if(strcmp(name+6, "velocity") == 0)
        {
            /* Combine separate contiguous arrays into a vector. */
            serror(VisIt_VariableData_setArrayDataD(h, 0, VISIT_OWNER_SIM, nnodes, 0, sizeof(double), sim->soa.vx));
            serror(VisIt_VariableData_setArrayDataD(h, 1, VISIT_OWNER_SIM, nnodes, 0, sizeof(double), sim->soa.vy));
            serror(VisIt_VariableData_setArrayDataD(h, 2, VISIT_OWNER_SIM, nnodes, 0, sizeof(double), sim->soa.vz));
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list (the list of domains
 *          owned by this processor).
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jul 17 15:54:51 PDT 2015
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
        int *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int));
        *iptr = sim->par_rank;

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
        VisIt_DomainList_setDomains(h, sim->par_size, hdl);
    }
    return h;
}
