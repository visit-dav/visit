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

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += 0.0134;
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
    sim_sleep(1);
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
    VisItInitializeSocketAndDumpSimFile("var",
        "Demonstrates creating different variable types",
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
        visit_handle m1 = VISIT_INVALID_HANDLE, m2 = VISIT_INVALID_HANDLE;
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
            VisIt_MeshMetaData_setXUnits(m1, "cm");
            VisIt_MeshMetaData_setYUnits(m1, "cm");
            VisIt_MeshMetaData_setXLabel(m1, "Width");
            VisIt_MeshMetaData_setYLabel(m1, "Height");

            VisIt_SimulationMetaData_addMesh(md, m1);
        }

        /* Set the second mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&m2) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(m2, "mesh3d");
            VisIt_MeshMetaData_setMeshType(m2, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(m2, 3);
            VisIt_MeshMetaData_setSpatialDimension(m2, 3);
            VisIt_MeshMetaData_setXUnits(m2, "cm");
            VisIt_MeshMetaData_setYUnits(m2, "cm");
            VisIt_MeshMetaData_setZUnits(m2, "cm");
            VisIt_MeshMetaData_setXLabel(m2, "Width");
            VisIt_MeshMetaData_setYLabel(m2, "Height");
            VisIt_MeshMetaData_setZLabel(m2, "Depth");

            VisIt_SimulationMetaData_addMesh(md, m2);
        }

        /* Add a zonal scalar variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a nodal scalar variable on mesh3d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_scalar");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh3d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a zonal vector variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_vector");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_VECTOR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a nodal vector variable on mesh3d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_vector");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh3d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_VECTOR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a zonal label variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_label");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_LABEL);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a zonal array variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_array");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_ARRAY);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_setNumComponents(vmd, 4);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a nodal array variable on mesh3d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_array");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh3d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_ARRAY);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_setNumComponents(vmd, 4);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a "hidden" variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "hidden");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh3d");
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_setHideFromGUI(vmd, 1);

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

/* Rectilinear mesh */
float rmesh_x[] = {0., 1., 2.5, 5.};
float rmesh_y[] = {0., 2., 2.25, 2.55,  5.};
int   rmesh_dims[] = {4, 5, 1};
int   rmesh_ndims = 2;
float zonal[] = {1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.,12.};
float zonal_vector[][2] = {
   { 1., 2.},{ 3., 4.},{ 5., 6.},{ 7., 8.},{ 9.,10.},{11.,12.},
   {13.,14.},{15.,16.},{17.,18.},{19.,20.},{21.,22.},{23.,24.}
};
float zonal_array[][4] = {
   { 1., 2.,0.,0.},{ 3., 4.,0.,0.},{ 5., 6.,0.,0.},{ 7., 8.,0.,0.},{ 9.,10.,0.,0.},{11.,12.,0.,0.},
   {13.,14.,0.,0.},{15.,16.,0.,0.},{17.,18.,0.,0.},{19.,20.,0.,0.},{21.,22.,0.,0.},{23.,24.,0.,0.}
};
const char *zonal_labels = "zone1\0\0zone2\0\0zone3\0\0zone4\0\0zone5\0\0zone6\0\0zone7\0\0zone8\0\0zone9\0\0zone10\0zone11\0zone12";

/* Curvilinear mesh */
float cmesh_x[2][3][4] = {
   {{0.,1.,2.,3.},{0.,1.,2.,3.}, {0.,1.,2.,3.}},
   {{0.,1.,2.,3.},{0.,1.,2.,3.}, {0.,1.,2.,3.}}
};
float cmesh_y[2][3][4] = {
   {{0.5,0.,0.,0.5},{1.,1.,1.,1.}, {1.5,2.,2.,1.5}},
   {{0.5,0.,0.,0.5},{1.,1.,1.,1.}, {1.5,2.,2.,1.5}}
};
float cmesh_z[2][3][4] = {
   {{0.,0.,0.,0.},{0.,0.,0.,0.},{0.,0.,0.,0.}},
   {{1.,1.,1.,1.},{1.,1.,1.,1.},{1.,1.,1.,1.}}
};
int cmesh_dims[] = {4, 3, 2};
int cmesh_ndims = 3;
double nodal[2][3][4] = {
   {{1.,2.,3.,4.},{5.,6.,7.,8.},{9.,10.,11.,12}},
   {{13.,14.,15.,16.},{17.,18.,19.,20.},{21.,22.,23.,24.}}
};
double nodal_vector[2][3][4][3] = {
   { {{ 0., 1., 2.},{ 3., 4., 5.},{ 6., 7., 8.},{ 9.,10.,11.}},
     {{12.,13.,14.},{15.,16.,17.},{18.,19.,20.},{21.,22.,23.}},
     {{24.,25.,26.},{27.,28.,29.},{30.,31.,32.},{33.,34.,35.}} },

   { {{36.,37.,38.},{39.,40.,41.},{42.,43.,44.},{45.,46.,47.}},
     {{48.,49.,50.},{51.,52.,53.},{54.,55.,56.},{57.,58.,59.}},
     {{60.,61.,62.},{63.,64.,65.},{66.,67.,68.},{69.,70.,71}} }
};
double nodal_array[2][3][4][4] = {
   { {{ 0., 1., 2.,0.},{ 3., 4., 5.,0.},{ 6., 7., 8.,0.},{ 9.,10.,11.,0.}},
     {{12.,13.,14.,0.},{15.,16.,17.,0.},{18.,19.,20.,0.},{21.,22.,23.,0.}},
     {{24.,25.,26.,0.},{27.,28.,29.,0.},{30.,31.,32.,0.},{33.,34.,35.,0.}} },

   { {{36.,37.,38.,0.},{39.,40.,41.,0.},{42.,43.,44.,0.},{45.,46.,47.,0.}},
     {{48.,49.,50.,0.},{51.,52.,53.,0.},{54.,55.,56.,0.},{57.,58.,59.,0.}},
     {{60.,61.,62.,0.},{63.,64.,65.,0.},{66.,67.,68.,0.},{69.,70.,71.,0.}} }
};

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

    if(strcmp(name, "mesh2d") == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxc, hyc;
            VisIt_VariableData_alloc(&hxc);
            VisIt_VariableData_alloc(&hyc);
            VisIt_VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, rmesh_dims[0], rmesh_x);
            VisIt_VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, rmesh_dims[1], rmesh_y);
            VisIt_RectilinearMesh_setCoordsXY(h, hxc, hyc);
        }
    }
    else if(strcmp(name, "mesh3d") == 0)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int nn;
            visit_handle hxc, hyc, hzc;
            nn = cmesh_dims[0] * cmesh_dims[1] * cmesh_dims[2];
            VisIt_VariableData_alloc(&hxc);
            VisIt_VariableData_alloc(&hyc);
            VisIt_VariableData_alloc(&hzc);
            VisIt_VariableData_setDataF(hxc, VISIT_OWNER_SIM, 1, nn, (float*)cmesh_x);
            VisIt_VariableData_setDataF(hyc, VISIT_OWNER_SIM, 1, nn, (float*)cmesh_y);
            VisIt_VariableData_setDataF(hzc, VISIT_OWNER_SIM, 1, nn, (float*)cmesh_z);
            VisIt_CurvilinearMesh_setCoordsXYZ(h, cmesh_dims, hxc, hyc, hzc);
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
 *   Brad Whitlock, Thu Dec 15 15:58:39 PST 2011
 *   I added array variables and a hidden scalar.
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    int nComponents = 1, nTuples = 0;

    if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        if(strcmp(name, "zonal_scalar") == 0)
        {
            nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, nComponents,
                nTuples, zonal);
        }
        else if(strcmp(name, "nodal_scalar") == 0 ||
                strcmp(name, "hidden") == 0)
        {
            nTuples = cmesh_dims[0] * cmesh_dims[1] *
                cmesh_dims[2];
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (double*)nodal);
        }
        else if(strcmp(name, "zonal_vector") == 0)
        {
            nComponents = 2;
            nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (float *)zonal_vector);
        }
        else if(strcmp(name, "nodal_vector") == 0)
        {
            nComponents = 3;
            nTuples = cmesh_dims[0] * cmesh_dims[1] *
                cmesh_dims[2];
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (double *)nodal_vector);
        }
        else if(strcmp(name, "zonal_label") == 0)
        {
            nComponents = 7;
            nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
            VisIt_VariableData_setDataC(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (char *)zonal_labels);
        }
        else if(strcmp(name, "zonal_array") == 0)
        {
            nComponents = 4;
            nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (float *)zonal_array);
        }
        else if(strcmp(name, "nodal_array") == 0)
        {
            nComponents = 4;
            nTuples = cmesh_dims[0] * cmesh_dims[1] *
                cmesh_dims[2];
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, nComponents,
                nTuples, (double *)nodal_array);
        }
        else
        {
            VisIt_VariableData_free(h);
            h = VISIT_INVALID_HANDLE;
        }
    }
    return h;
}
