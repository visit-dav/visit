/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

/* The purpose of this simulation is to demonstrate how to use a domain
 * boundaries object. Domain boundaries objects describe how domains abut. For
 * the simple cases supported by libsim, you can associate the domain with
 * some global zone numbers using the domain boundaries object and this tells
 * VisIt how domains touch so ghost data can be created on the fly and exchanged
 * in parallel. When you use this strategy, you DO NOT have to create ghost
 * zones yourself in how you create your mesh or variables; VisIt will do the
 * proper exchanges among domains to reconstruct the values at domain boundaries.
 *
 * Bottom line: This approach gives the benefits of ghost zones without you
 *              having to create them in your data.
 *
 *              See SimGetDomainBoundaries() for more information.
 */

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

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetCurve(const char *name, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);
visit_handle SimGetDomainBoundaries(const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define XMIN -30.f
#define XMAX  30.f
#define YMIN -40.f
#define YMAX  40.f
#define ZMIN -50.f
#define ZMAX  50.f

#define NX 11
#define NY 11
#define NZ 11

#define NDOMAINS_X 3
#define NDOMAINS_Y 4
#define NDOMAINS_Z 5

typedef struct
{
    int    globalIndex;

    int    dims[3];
    float  extents[6];
    float *x;
    float *y;
    float *z;
    float *variable;
} Domain;

void
Domain_ctor(Domain *dom, int globalIndex, int dx, int dy, int dz)
{
    int i;

    /* Save information about where this domain lives in the global model. */
    dom->globalIndex = globalIndex;

    /* #nodes in this domain */
    dom->dims[0] = NX;
    dom->dims[1] = NY;
    dom->dims[2] = NZ;

    /* spatial extents for this domain */
    dom->extents[0] = XMIN + ((float)dx) * ((XMAX - XMIN) / ((float)NDOMAINS_X));
    dom->extents[1] = XMIN + ((float)dx+1) * ((XMAX - XMIN) / ((float)NDOMAINS_X));
    dom->extents[2] = YMIN + ((float)dy) * ((YMAX - YMIN) / ((float)NDOMAINS_Y));
    dom->extents[3] = YMIN + ((float)dy+1) * ((YMAX - YMIN) / ((float)NDOMAINS_Y));
    dom->extents[4] = ZMIN + ((float)dz) * ((ZMAX - ZMIN) / ((float)NDOMAINS_Z));
    dom->extents[5] = ZMIN + ((float)dz+1) * ((ZMAX - ZMIN) / ((float)NDOMAINS_Z));

    /* coordinates for this domain */
    dom->x = (float *)malloc(sizeof(float) * dom->dims[0]);
    dom->y = (float *)malloc(sizeof(float) * dom->dims[1]);
    dom->z = (float *)malloc(sizeof(float) * dom->dims[2]);
    for(i = 0; i < dom->dims[0]; ++i)
    {
        float t = (float)i / (float)(dom->dims[0]-1);
        dom->x[i] = (1.f - t) * dom->extents[0] + t * dom->extents[1];
    }
    for(i = 0; i < dom->dims[1]; ++i)
    {
        float t = (float)i / (float)(dom->dims[1]-1);
        dom->y[i] = (1.f - t) * dom->extents[2] + t * dom->extents[3];
    }
    for(i = 0; i < dom->dims[2]; ++i)
    {
        float t = (float)i / (float)(dom->dims[2]-1);
        dom->z[i] = (1.f - t) * dom->extents[4] + t * dom->extents[5];
    }

    /* variable for this domain. */
    dom->variable = (float*)malloc(sizeof(float) * 
        (dom->dims[0]-1) * (dom->dims[1]-1) * (dom->dims[2]-1));
}

void
Domain_dtor(Domain *dom)
{
    free(dom->x);
    free(dom->y);
    free(dom->z);
    free(dom->variable);
}

void
Domain_update_variable(Domain *dom, float t)
{
    int i,j,k, idx;
    float X,Y,Z;

    /* Make a moving point f(t)*/
    X = 0.f;
    Y = 10.f * sin(t);
    Z = 10.f * cos(t);

    /* Make the variable be the distance to the moving point. */
    idx = 0;
    for(k = 0; k < dom->dims[2]-1; ++k)
    {
        float cz = (dom->z[k] + dom->z[k+1]) / 2.f;
        for(j = 0; j < dom->dims[1]-1; ++j)
        {
            float cy = (dom->y[j] + dom->y[j+1]) / 2.f;
            for(i = 0; i < dom->dims[0]-1; ++i, ++idx)
            {
                float cx, dX, dY, dZ;
                cx = (dom->x[i] + dom->x[i+1]) / 2.f;

                dX = cx - X;
                dY = cy - Y;
                dZ = cz - Z;

                dom->variable[idx] = sqrt(dX*dX + dY*dY + dZ*dZ);
            }
        }
    }
}

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
    int     par_rank;
    int     par_size;
    int     cycle;
    double  time;
    int     runMode;
    int     done;

    int     doDomainBoundaries;
    int     nTotalDomains;
    int     nDomains;
    Domain *domains;
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

    sim->doDomainBoundaries = 1;
    sim->nDomains = 0;
    sim->nTotalDomains = 0;
    sim->domains = NULL;
}

void
simulation_data_create_domains(simulation_data *sim)
{
    int dx, dy, dz, index = 0;

    /* Create the domains for this processor */
    sim->nDomains = 0;
    sim->nTotalDomains = (NDOMAINS_X * NDOMAINS_Y * NDOMAINS_Z);
    sim->domains = (Domain *)malloc(sizeof(Domain) * sim->nTotalDomains);
    memset(sim->domains, 0, sizeof(Domain) * sim->nTotalDomains);
    for(dz = 0; dz < NDOMAINS_Z; ++dz)
    {
        for(dy = 0; dy < NDOMAINS_Y; ++dy)
        {
            for(dx = 0; dx < NDOMAINS_X; ++dx, ++index)
            {
                if(index % sim->par_size == sim->par_rank)
                {
                    Domain_ctor(&sim->domains[sim->nDomains], index, dx, dy, dz);
                    sim->nDomains++;
                }
            }
        }
    }
}

void
simulation_data_dtor(simulation_data *sim)
{
    int i;
    for(i = 0; i < sim->nDomains; ++i)
        Domain_dtor(&sim->domains[i]);
    free(sim->domains);
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
 * Date:       Wed Feb  1 10:10:18 PST 2012
 *
 * Modifications:
 *
 *****************************************************************************/
void simulate_one_timestep(simulation_data *sim)
{
    int i;

    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    /* Update the variables */
    for(i = 0; i < sim->nDomains; ++i)
        Domain_update_variable(&sim->domains[i], sim->time);    

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
static int visit_broadcast_int_callback(int *value, int sender)
{
    return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
}

static int visit_broadcast_string_callback(char *str, int len, int sender)
{
    return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
#endif


/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, MPI_COMM_WORLD);
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
    int command;
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
        if(VisItReadConsole(1000, cmd) == VISIT_ERROR)
        {
            sprintf(cmd, "quit");
            printf("quit\n");
        }
    }

#ifdef PARALLEL
    /* Broadcast the command to all processors. */
    MPI_Bcast(cmd, 1000, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

    if(strcmp(cmd, "quit") == 0)
        sim->done = 1;
    else if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;
}

/******************************************************************************
 *
 * Function: mainloop
 *
 * Purpose: Handles the program's main event loop and dispatches events to 
 *          other functions for processing.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Feb  1 10:10:18 PST 2012
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
        MPI_Bcast(&visitstate, 1, MPI_INT, 0, MPI_COMM_WORLD);
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

                if(sim->doDomainBoundaries)
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
 * Date:       Wed Feb  1 10:10:18 PST 2012
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
    MPI_Comm_rank (MPI_COMM_WORLD, &sim.par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &sim.par_size);
#endif

    /* Process command line arguments. */
    SimulationArguments(argc, argv);
    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-nobounds") == 0)
            sim.doDomainBoundaries = 0;
    }

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell libsim whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);
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
            "domainbounds_par",
#else
            "domainbounds",
#endif
            "Demonstrates domain boundaries",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

    simulation_data_create_domains(&sim);

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
 * Date:       Wed Feb  1 11:39:55 PST 2012
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
            VisIt_MeshMetaData_setName(mmd, "mesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, sim->nTotalDomains);
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

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
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
 * Date:       Wed Feb  1 11:39:55 PST 2012
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
        /* Find the right domain. */
        int i;
        Domain *d = NULL;
        for(i = 0; i < sim->nDomains; ++i)
        {
            if(sim->domains[i].globalIndex == domain)
            {
                d = &sim->domains[i];
                break;
            }
        }

        if(d != NULL && VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hx, hy, hz;

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_alloc(&hz);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, d->dims[0], d->x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, d->dims[1], d->y);
            VisIt_VariableData_setDataF(hz, VISIT_OWNER_SIM, 1, d->dims[2], d->z);
            VisIt_RectilinearMesh_setCoordsXYZ(h, hx, hy, hz);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Feb  1 11:39:55 PST 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "zonal") == 0)
    {
        /* Find the right domain. */
        int i;
        Domain *d = NULL;
        for(i = 0; i < sim->nDomains; ++i)
        {
            if(sim->domains[i].globalIndex == domain)
            {
                d = &sim->domains[i];
                break;
            }
        }

        if(d != NULL && VisIt_VariableData_alloc(&h) != VISIT_ERROR)
        {
            int nTuples = (d->dims[0]-1) * (d->dims[1]-1) * (d->dims[2]-1);
            VisIt_VariableData_alloc(&h);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                nTuples, d->variable);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Feb  1 11:39:55 PST 2012
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
        int i, *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int) * sim->nDomains);
        for(i = 0; i < sim->nDomains; ++i)
            iptr[i] = sim->domains[i].globalIndex;

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, sim->nDomains, iptr);
        VisIt_DomainList_setDomains(h, sim->nTotalDomains, hdl);
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain boundaries object.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Feb  1 11:39:55 PST 2012
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
        int ext[6], domainSize[3], dx, dy, dz, index = 0;

        /* Set the type. 0=rectilinear, 1=curvilinear. */
        VisIt_DomainBoundaries_set_type(h, 0);

        /* Set the total number of domains in the problem. */
        VisIt_DomainBoundaries_set_numDomains(h, sim->nTotalDomains);

        /* We can't use our sim->domains to populate the global structure
         * because it's a local-only structure. Since our domains are
         * simply arranged and all the same size (in this case), we can easily 
         * figure out the zone ranges that they occupy.
         */
        domainSize[0] = NX-1;
        domainSize[1] = NY-1;
        domainSize[2] = NZ-1;
        for(dz = 0; dz < NDOMAINS_Z; ++dz)
        {
            for(dy = 0; dy < NDOMAINS_Y; ++dy)
            {
                for(dx = 0; dx < NDOMAINS_X; ++dx, ++index)
                {
                    int origin[3];
                    origin[0] = dx * domainSize[0];
                    origin[1] = dy * domainSize[1];
                    origin[2] = dz * domainSize[2];

                    ext[0] = origin[0];
                    ext[1] = origin[0] + domainSize[0];
                    ext[2] = origin[1];
                    ext[3] = origin[1] + domainSize[1];
                    ext[4] = origin[2];
                    ext[5] = origin[2] + domainSize[2];
                    VisIt_DomainBoundaries_set_rectIndices(h, index, ext);
                }
            }
        }
    }

    return h;
}
