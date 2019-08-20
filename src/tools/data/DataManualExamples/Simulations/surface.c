// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "SimulationExample.h"

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
    int     cycle;
    double  time;
    int     runMode;
    int     done;
    int     dims[2];
    float   extents[4];
    float   *xc, *yc, *zc;
    float   *interleaved;
    int      npts, ncells;
    int     *conn;
    int      connlen;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->dims[0] = 100;
    sim->dims[1] = 100;
    sim->extents[0] = -4.*M_PI;
    sim->extents[1] = 4.*M_PI;
    sim->extents[2] = -4.*M_PI;
    sim->extents[3] = 4.*M_PI;
    sim->npts = sim->dims[0]*sim->dims[1];
    sim->ncells = (sim->dims[0]-1)*(sim->dims[1]-1);
    sim->xc = (float *)malloc(sizeof(float)*sim->npts);
    sim->yc = (float *)malloc(sizeof(float)*sim->npts);
    sim->zc = (float *)malloc(sizeof(float)*sim->npts);
    sim->interleaved = (float *)malloc(sizeof(float)*3*sim->npts);
    sim->conn = NULL;
    sim->connlen = 0;
}

void
simulation_data_dtor(simulation_data *sim)
{
    if(sim->xc != NULL)
        free(sim->xc);
    if(sim->yc != NULL)
        free(sim->yc);
    if(sim->zc != NULL)
        free(sim->zc);
    if(sim->conn != NULL)
        free(sim->conn);
}

void
simulation_data_update(simulation_data *sim)
{
    float ti, tj, x, y, z, r;
    int i,j, idx = 0;

    if(sim->conn == NULL)
    {
        int *conn;
        sim->connlen = 5*sim->ncells;
        sim->conn = conn = (int *)malloc(sizeof(int)*sim->connlen);
        for(j = 0; j < sim->dims[1]-1; ++j)
        for(i = 0; i < sim->dims[0]-1; ++i)
        {
            conn[0] = VISIT_CELL_QUAD;
            conn[1] = j*sim->dims[0] + i;
            conn[2] = j*sim->dims[0] + i+1;
            conn[3] = (j+1)*sim->dims[0] + i+1;
            conn[4] = (j+1)*sim->dims[0] + i;
            conn += 5;
        }
    }

    for(j = 0; j < sim->dims[1]; ++j)
    {
        tj = ((float)j) / ((float)(sim->dims[1]-1));
        y = sim->extents[2] + tj * (sim->extents[3] - sim->extents[2]);
        for(i = 0; i < sim->dims[0]; ++i)
        {
            ti = ((float)i) / ((float)(sim->dims[0]-1));
            x = sim->extents[0] + ti * (sim->extents[1] - sim->extents[0]);

            r = sqrt(x*x + y*y);
            z = sin(r - sim->time);

            sim->xc[idx] = x;
            sim->yc[idx] = y;
            sim->zc[idx] = z;

            sim->interleaved[3*idx] = x;
            sim->interleaved[3*idx+1] = y;
            sim->interleaved[3*idx+2] = z;

            idx++;
        }
    }

}

const char *cmd_names[] = {"halt", "step", "run"};

void simulate_one_timestep(simulation_data *sim);
void read_input_deck(void) { }

/******************************************************************************
 *
 * Purpose: Callback function for control commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Input Arguments:
 *   cmd    : The command string that we want the sim to execute.
 *   args   : String argument for the command.
 *   cbdata : User-provided callback data.
 *
 * Modifications:
 *
 *****************************************************************************/

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

/* Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 */
void
ProcessConsoleCommand(simulation_data *sim)
{
    /* Read A Command */
    char cmd[1000];

    int iseof = (fgets(cmd, 1000, stdin) == NULL);
    if (iseof)
    {
        sprintf(cmd, "quit");
        printf("quit\n");
    }

    if (strlen(cmd)>0 && cmd[strlen(cmd)-1] == '\n')
        cmd[strlen(cmd)-1] = '\0';

    if(strcmp(cmd, "quit") == 0)
        sim->done = 1;
    else if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;
}

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += M_PI / 20;
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
    simulation_data_update(sim);
    VisItTimeStepChanged();
    VisItUpdatePlots();
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(void)
{
    int blocking, visitstate, err = 0;

    /* Set up some simulation data. */
    simulation_data sim;
    simulation_data_ctor(&sim);
    simulation_data_update(&sim);

    /* main loop */
    fprintf(stderr, "command> ");
    fflush(stderr);
    do
    {
        blocking = (sim.runMode == SIM_RUNNING) ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        visitstate = VisItDetectInput(blocking, fileno(stdin));

        /* Do different things depending on the output from VisItDetectInput. */
        if(visitstate >= -5 && visitstate <= -1)
        {
            fprintf(stderr, "Can't recover from error!\n");
            err = 1;
        }
        else if(visitstate == 0)
        {
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep(&sim);
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
            {
                fprintf(stderr, "VisIt connected\n");
                sim.runMode = SIM_STOPPED;
                VisItSetCommandCallback(ControlCommandCallback, (void*)&sim);
                VisItSetGetMetaData(SimGetMetaData, (void*)&sim);
                VisItSetGetMesh(SimGetMesh, (void*)&sim);
                VisItSetGetVariable(SimGetVariable, (void*)&sim);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            if(VisItProcessEngineCommand() == VISIT_ERROR)
            {
                fprintf(stderr, "VisIt error, disconnecting\n");
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                sim.runMode = SIM_RUNNING;
            }
        }
        else if(visitstate == 3)
        {
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand(&sim);
            fprintf(stderr, "command> ");
            fflush(stderr);
        }
    } while(!sim.done && err == 0);

    /* Clean up */
    VisItDisconnect();
    simulation_data_dtor(&sim);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
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
    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("surface",
        "Demonstrates creating a 2D unstructured mesh in 3D space",
        "/path/to/where/sim/was/started",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

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
        visit_handle mmd = VISIT_INVALID_HANDLE, vmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Fill in the metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "surface");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
/*#define FORCE_ALLENS_ISSUE*/
#ifdef FORCE_ALLENS_ISSUE
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
#else
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
#endif
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setZUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");
            VisIt_MeshMetaData_setZLabel(mmd, "Depth");
            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a nodal scalar variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "z");
            VisIt_VariableMetaData_setMeshName(vmd, "surface");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }       

        /* Add some custom commands. */
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
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "surface") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle x,y,z,conn;

            /* coordinates */
/*#define INTERLEAVE_COORDINATES*/
#ifdef INTERLEAVE_COORDINATES
            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_setDataF(x, VISIT_OWNER_SIM, 3, sim->npts, sim->interleaved);
            VisIt_UnstructuredMesh_setCoords(h, x);         
#else
            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_alloc(&y);
            VisIt_VariableData_alloc(&z);

            VisIt_VariableData_setDataF(x, VISIT_OWNER_SIM, 1, sim->npts, sim->xc);
            VisIt_VariableData_setDataF(y, VISIT_OWNER_SIM, 1, sim->npts, sim->yc);
            VisIt_VariableData_setDataF(z, VISIT_OWNER_SIM, 1, sim->npts, sim->zc);

            VisIt_UnstructuredMesh_setCoordsXYZ(h, x, y, z);
#endif
            /* cells */
            VisIt_VariableData_alloc(&conn);
            VisIt_VariableData_setDataI(conn, VISIT_OWNER_SIM, 1, sim->connlen, sim->conn);
            VisIt_UnstructuredMesh_setConnectivity(h, sim->ncells, conn);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;
    if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                sim->npts, sim->zc);
    }
    return h;
}
