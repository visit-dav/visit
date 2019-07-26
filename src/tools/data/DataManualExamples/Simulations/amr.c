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
visit_handle SimGetDomainNesting(const char *, void *);

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
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
}

void
simulation_data_dtor(simulation_data *sim)
{
}

const char *cmd_names[] = {"halt", "step", "run", "update"};

void simulate_one_timestep(simulation_data *sim);
void read_input_deck(void) { }

/******************************************************************************
 *
 * Purpose: Callback function for control commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Jun  8 13:24:40 PDT 2010
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
    else if(strcmp(cmd, "update") == 0)
        VisItUpdatePlots();
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
        VisItUpdatePlots();
}

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += 0.0134;
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
    sim_sleep(1);

    VisItTimeStepChanged();
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Jun  8 13:24:40 PDT 2010
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
                sim.runMode = SIM_STOPPED;
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback, (void*)&sim);

                VisItSetGetMetaData(SimGetMetaData, (void*)&sim);
                VisItSetGetMesh(SimGetMesh, (void*)&sim);
                VisItSetGetDomainNesting(SimGetDomainNesting, (void*)&sim);
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
    simulation_data_dtor(&sim);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Jun  8 13:24:40 PDT 2010
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
    VisItInitializeSocketAndDumpSimFile("amr",
        "Demonstrates domain nesting data access function",
        "/no/useful/path",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    return 0;
}

/* DATA ACCESS FUNCTIONS */

/* AMR mesh 
 *
 * NOTE: Patch 4 spans patches 2,3 in level 2 and only exists on even sim cycles.
 */
#define ITER 2
#define NPATCHES 5
float rmx[NPATCHES][2]    = {{0., 10.}, {3., 10.}, {5., 7.5}, {7.5, 10.}, {6, 8.75}};
float rmy[NPATCHES][2]    = {{0., 10.}, {1., 9.}, {2., 7.}, {2., 7.}, {3., 4.75}};
int   rmxext[NPATCHES][2] = {{0,9}, {6,19}, {20,29}, {30,39}, {48,69}};
int   rmyext[NPATCHES][2] = {{0,9}, {2,17}, {8,27},  {8,27}, {24,37}};
int   level[NPATCHES]     = {0, 1, 2, 2, 3};
int   ncpatch[ITER][NPATCHES]   = {{1,2,1,1,0},
                                   {1,2,0,0,0}};
int   cpatch[ITER][NPATCHES][2] = {{{1,-1},{2,3},{4,-1},{4,-1},{-1,-1}},
                                   {{1,-1},{2,3},{-1,-1},{-1,-1},{-1,-1}}};
int   npatches[ITER] = {NPATCHES,NPATCHES-1};
int   nlevels[ITER] = {4, 3};

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Jun  8 13:24:40 PDT 2010
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

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Set the first mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Vary the number of patches based on the cycle so we can test
             * AMR SILs that change over time.
             */
            int iter = (sim->cycle % 2);

            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "amr");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_AMR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, npatches[iter]);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Patches");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "patch");
            VisIt_MeshMetaData_setNumGroups(mmd, nlevels[iter]);
            VisIt_MeshMetaData_setGroupTitle(mmd, "Levels");
            VisIt_MeshMetaData_setGroupPieceName(mmd, "level");
            for(i = 0; i < npatches[iter]; ++i)
                VisIt_MeshMetaData_addGroupId(mmd, level[i]);
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
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
 * Date:       Tue Jun  8 13:24:40 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "amr") == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle x, y;
            int   i, nx, ny;
            float cx[100], cy[100], t;

            /* Create the X coordinates */
            nx = rmxext[domain][1] - rmxext[domain][0]+1+1;
            for(i = 0; i < nx; ++i)
            {
                t = (float)i / (float)(nx-1);
                cx[i] = (1.-t)*rmx[domain][0] + t*rmx[domain][1];
            }

            /* Create the Y coordinates */
            ny = rmyext[domain][1] - rmyext[domain][0]+1+1;
            for(i = 0; i < ny; ++i)
            {
                t = (float)i / (float)(ny-1);
                cy[i] = (1.-t)*rmy[domain][0] + t*rmy[domain][1];
            }

            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_alloc(&y);
            VisIt_VariableData_setDataF(x, VISIT_OWNER_COPY, 1, nx, cx);
            VisIt_VariableData_setDataF(y, VISIT_OWNER_COPY, 1, ny, cy);
            VisIt_RectilinearMesh_setCoordsXY(h, x, y);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns the domain nesting for the mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Jun  8 13:26:52 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetDomainNesting(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(VisIt_DomainNesting_alloc(&h) != VISIT_ERROR)
    {
#define XMIN 0
#define YMIN 1
#define ZMIN 2
#define XMAX 3
#define YMAX 4
#define ZMAX 5
        int i, dom;
        int ratios[3] = {2,2,1}, ext[6]={0,0,0,0,0,0}, patch[2]={0,0};

        /* Vary the number of patches based on the cycle so we can test
         * AMR SILs that change over time.
         */
        int iter = (sim->cycle % 2);
        VisIt_DomainNesting_set_dimensions(h, npatches[iter], nlevels[iter], 2);

        for(i = 0; i < nlevels[iter]; ++i)
            VisIt_DomainNesting_set_levelRefinement(h, i, ratios);

        for(dom = 0; dom < npatches[iter]; ++dom)
        {
            ext[XMIN] = rmxext[dom][0];
            ext[YMIN] = rmyext[dom][0];
            ext[ZMIN] = 0;
            ext[XMAX] = rmxext[dom][1];
            ext[YMAX] = rmyext[dom][1];
            ext[ZMAX] = 0;
            for(i = 0; i < ncpatch[iter][dom]; ++i)
                patch[i] = cpatch[iter][dom][i];
            VisIt_DomainNesting_set_nestingForPatch(h, dom, level[dom],
                patch, ncpatch[iter][dom], ext);
        }
    }

    return h;
}
