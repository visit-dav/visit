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

void read_input_deck(void) { }

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED 0
#define SIM_RUNNING 1

#define NPTS        1000

typedef struct
{
    int     cycle;
    double  time;
    int     runMode;
    int     done;

    float    angle;
    float   *x;
    float   *y;
    float   *z;
    float   *var;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;

    sim->angle = 0.;
    sim->x = (float *)malloc(sizeof(float) * NPTS);
    sim->y = (float *)malloc(sizeof(float) * NPTS);
    sim->z = (float *)malloc(sizeof(float) * NPTS);
    sim->var = (float *)malloc(sizeof(float) * NPTS);
}

void
simulation_data_dtor(simulation_data *sim)
{
    free(sim->x);
    free(sim->y);
    free(sim->z);
    free(sim->var);
}

const char *cmd_names[] = {"halt", "step", "run", "update"};

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
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/
void simulate_one_timestep(simulation_data *sim)
{
    int i;
    for(i = 0; i < NPTS; ++i)
    {
        float t = ((float)i) / ((float)(NPTS-1));
        float a = 3.14159 * 10. * t;
        sim->x[i] = t * cos(a + (0.5 + 0.5 * t) * sim->angle);
        sim->y[i] = t * sin(a + (0.5 + 0.5 * t) * sim->angle);
        sim->z[i] = t;
        sim->var[i] = sqrt(sim->x[i]*sim->x[i] + 
                           sim->y[i]*sim->y[i] + 
                           sim->z[i]*sim->z[i]);
    }

    sim->angle = sim->angle + 0.05;

    ++sim->cycle;
    sim->time += (M_PI / 10.);

    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
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
 * Date:       Thu Nov 2 17:26:08 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    int blocking, visitstate, err = 0;

    /* Simulate one timestep upfront so we have coordinates. */
    simulate_one_timestep(sim);

    /* main loop */
    fprintf(stderr, "command> ");
    fflush(stderr);
    do
    {
        blocking = (sim->runMode == SIM_RUNNING) ? 0 : 1;
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
            simulate_one_timestep(sim);
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
            {
                sim->runMode = SIM_STOPPED;
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback, (void*)sim);

                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            if(VisItProcessEngineCommand() == VISIT_ERROR)
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                sim->runMode = SIM_RUNNING;
            }
        }
        else if(visitstate == 3)
        {
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand(sim);
            fprintf(stderr, "command> ");
            fflush(stderr);
        }
    } while(!sim->done && err == 0);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:36:17 PST 2007
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
    simulation_data sim;
    simulation_data_ctor(&sim);

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("point",
        "Demonstrates creating a point mesh",
        "/path/to/where/sim/was/started",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data.*/
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);

    return 0;
}

/* DATA ACCESS FUNCTIONS */

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
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

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "point3d");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_POINT);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 0);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
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

        /* Add variable metadata. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "pointvar");
            VisIt_VariableMetaData_setMeshName(vmd, "point3d");
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
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "point3d") == 0)
    {
        if(VisIt_PointMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hx, hy, hz;

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_alloc(&hz);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, NPTS, sim->x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, NPTS, sim->y);
            VisIt_VariableData_setDataF(hz, VISIT_OWNER_SIM, 1, NPTS, sim->z);
            VisIt_PointMesh_setCoordsXYZ(h, hx, hy, hz);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns variables.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "pointvar") == 0)
    {
        if(VisIt_VariableData_alloc(&h) != VISIT_ERROR)
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1, NPTS, sim->var);
    }

    return h;
}


