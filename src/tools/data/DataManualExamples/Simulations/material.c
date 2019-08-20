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
visit_handle SimGetMaterial(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetMixedVariable(int, const char *, void *);

/******************************************************************************
 * Simulation data and functions
 *****************************************************************************/

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

void read_input_deck(void) { }

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

#define IS_COMMAND(C) (strcmp(cmd, C) == 0) 
    if(IS_COMMAND("halt"))
        sim->runMode = SIM_STOPPED;
    else if(IS_COMMAND("step"))
        simulate_one_timestep(sim);
    else if(IS_COMMAND("run"))
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
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(simulation_data *sim)
{
    int blocking, visitstate, err = 0;

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
                VisItSetGetMaterial(SimGetMaterial, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetMixedVariable(SimGetMixedVariable, (void*)sim);
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
    simulation_data sim;
    simulation_data_ctor(&sim);

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("material",
        "Demonstrates material data access function",
        "/path/to/where/sim/was/started",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    /* Cleanup */
    simulation_data_dtor(&sim);

    return 0;
}

/* DATA ACCESS FUNCTIONS */

/* Values to match the Fortran example. */
#define NX 5
#define NY 4
#define XMIN 0.f
#define XMAX 4.f
#define YMIN 0.f
#define YMAX 3.f
const char *matNames[] = {"Water", "Membrane", "Air"};

#define DX (XMAX - XMIN)

/* Rectilinear mesh */
static float rmesh_x[NX];
static float rmesh_y[NY];
static const int   rmesh_dims[] = {NX, NY, 1};
static const int   rmesh_ndims = 2;

/* Unstructured mesh */
float coords2d[NY][NX][2] = {
    {{0,0}, {1,0}, {2,0}, {3,0}, {4,0}},
    {{0,1}, {1,1}, {2,1}, {3,1}, {4,1}},
    {{0,2}, {1,2}, {2,2}, {3,2}, {4,2}},
    {{0,3}, {1,3}, {2,3}, {3,3}, {4,3}}
};
int connectivity[] = {
    VISIT_CELL_QUAD,0,1,6,5,
    VISIT_CELL_QUAD,1,2,7,6,
    VISIT_CELL_QUAD,2,3,8,7,
    VISIT_CELL_QUAD,3,4,9,8,
    VISIT_CELL_QUAD,5,6,11,10,
    VISIT_CELL_QUAD,6,7,12,11,
    VISIT_CELL_QUAD,7,8,13,12,
    VISIT_CELL_QUAD,8,9,14,13,
    VISIT_CELL_QUAD,10,11,16,15,
    VISIT_CELL_QUAD,11,12,17,16,
    VISIT_CELL_QUAD,12,13,18,17,
    VISIT_CELL_QUAD,13,14,19,18
};
/* Mixed material encoding (materials 11,22,33) */
int matlist[] = {
    33, -1, -3, 11,
    33, -5, -7, 11,
    33, -9, -11, -14
};
int mix_zone[] = {
    1,1,
    2,2,
    5,5,
    6,6,
    9,9,
    10,10,10,
    11,11
};
int mix_mat[] =  {
    22,33,
    22,11,
    22,33,
    22,11,
    22,33,
    11,22,33,
    22,11
};
double mix_vf[] = {
    0.75,0.25,
    0.1875, 0.8125,
    0.625, 0.375,
    0.4375, 0.5625,
    0.3, 0.7,
    0.2, 0.4, 0.4,
    0.45, 0.55
};
int mix_next[] = {
    2, 0,
    4, 0,
    6, 0,
    8, 0,
    10, 0,
    12, 13, 0,
    15, 0
};

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
        visit_handle mmd = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;
        visit_handle mat = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "mesh2d");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
       
        /* Add a material */
        if(VisIt_MaterialMetaData_alloc(&mat) == VISIT_OKAY)
        {
            VisIt_MaterialMetaData_setName(mat, "Material");
            VisIt_MaterialMetaData_setMeshName(mat, "mesh2d");
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[0]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[1]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[2]);

            VisIt_SimulationMetaData_addMaterial(md, mat);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "scalar");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add mesh metadata for a second mesh. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "ucdmesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
       
        /* Add a material on the second mesh. */
        if(VisIt_MaterialMetaData_alloc(&mat) == VISIT_OKAY)
        {
            VisIt_MaterialMetaData_setName(mat, "MaterialFromArrays");
            VisIt_MaterialMetaData_setMeshName(mat, "ucdmesh");
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[0]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[1]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[2]);

            VisIt_SimulationMetaData_addMaterial(md, mat);
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
        if(VisIt_RectilinearMesh_alloc(&h) == VISIT_OKAY)
        {
            int i;
            visit_handle x,y;

            /* Initialize X coords. */
            for(i = 0; i < NX; ++i)
            {
                float t = (float)i / (float)(NX-1);
                rmesh_x[i] = (1.f-t)*XMIN + t*XMAX;
            }
            /* Initialize Y coords. */
            for(i = 0; i < NY; ++i) 
            {
                float t = (float)i / (float)(NY-1);
                rmesh_y[i] = (1.f-t)*YMIN + t*YMAX;
            }

            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_alloc(&y);
            VisIt_VariableData_setDataF(x, VISIT_OWNER_SIM, 1, NX, rmesh_x);
            VisIt_VariableData_setDataF(y, VISIT_OWNER_SIM, 1, NY, rmesh_y);
            VisIt_RectilinearMesh_setCoordsXY(h, x, y);
        }
    }
    else if(strcmp(name, "ucdmesh") == 0)
    {
        visit_handle c, hc;
        if(VisIt_UnstructuredMesh_alloc(&h) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&c) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&hc) == VISIT_OKAY)
        {
            int nnodes, nzones;
            nnodes = (NX * NY);
            nzones = (NX-1)*(NY-1);
            VisIt_VariableData_setDataF(c, VISIT_OWNER_SIM, 2, nnodes, (float*)coords2d);
            VisIt_VariableData_setDataI(hc, VISIT_OWNER_SIM, 1, 5*nzones,
                connectivity);
            VisIt_UnstructuredMesh_setCoords(h, c);
            VisIt_UnstructuredMesh_setConnectivity(h, nzones, hc);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns material data.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *   Brad Whitlock, Thu Feb 26 10:21:57 PST 2009
 *   Use SimV2 API.
 *
 *****************************************************************************/

visit_handle
SimGetMaterial(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    /* Allocate a VisIt_MaterialData */
    VisIt_MaterialData_alloc(&h);
    if(strcmp(name, "Material") == 0)
    {
        int i, j, m, cell = 0;
        int nmats, cellmat[10], matnos[3]={1,2,3};
        float cellmatvf[10];

        /* The matlist table indicates the material numbers that are found in
         * each cell. Every 3 numbers indicates the material numbers in a cell.
         * A material number of 0 means that the material entry is not used.
         */
        int matlist[NY-1][NX-1][3] = {
            {{3,0,0},{2,3,0},{1,2,0},{1,0,0}},
            {{3,0,0},{2,3,0},{1,2,0},{1,0,0}},
            {{3,0,0},{2,3,0},{1,2,3},{1,2,0}}
        };

        /* The mat_vf table indicates the material volume fractions that are
         * found in a cell.
         */
        float mat_vf[NY-1][NX-1][3] = {
            {{1.,0.,0.},{0.75,0.25,0.},  {0.8125,0.1875, 0.},{1.,0.,0.}},
            {{1.,0.,0.},{0.625,0.375,0.},{0.5625,0.4375,0.}, {1.,0.,0.}},
            {{1.,0.,0.},{0.3,0.7,0.},    {0.2,0.4,0.4},      {0.55,0.45,0.}}
        };

        /* Tell the object we'll be adding cells to it using add*Cell functions */
        VisIt_MaterialData_appendCells(h, (NX-1)*(NY-1));

        /* Fill in the VisIt_MaterialData */
        VisIt_MaterialData_addMaterial(h, matNames[0], &matnos[0]);
        VisIt_MaterialData_addMaterial(h, matNames[1], &matnos[1]);
        VisIt_MaterialData_addMaterial(h, matNames[2], &matnos[2]);

        for(j = 0; j < NY-1; ++j)
        {
            for(i = 0; i < NX-1; ++i, ++cell)
            {
                nmats = 0;
                for(m = 0; m < 3; ++m)
                {
                    if(matlist[j][i][m] > 0)
                    {
                        cellmat[nmats] = matnos[matlist[j][i][m] - 1];
                        cellmatvf[nmats] = mat_vf[j][i][m];
                        nmats++;
                    }
                }        
                if(nmats > 1)
                    VisIt_MaterialData_addMixedCell(h, cell, cellmat, cellmatvf, nmats);
                else
                    VisIt_MaterialData_addCleanCell(h, cell, cellmat[0]);
            }
        }
    }
    else if(strcmp(name, "MaterialFromArrays") == 0)
    {
        int nTuples, matnos[] = {11,22,33};
        visit_handle hmatlist, hmix_zone, hmix_mat, hmix_vf, hmix_next;
      
        /* Fill in the VisIt_MaterialData using arrays encode the material. */
        VisIt_MaterialData_addMaterial(h, matNames[0], &matnos[0]);
        VisIt_MaterialData_addMaterial(h, matNames[1], &matnos[1]);
        VisIt_MaterialData_addMaterial(h, matNames[2], &matnos[2]);

        VisIt_VariableData_alloc(&hmatlist);
        VisIt_VariableData_setDataI(hmatlist, VISIT_OWNER_SIM, 1, (NX-1)*(NY-1), matlist);
        VisIt_MaterialData_setMaterials(h, hmatlist);

        nTuples = sizeof(mix_zone) / sizeof(int);
        VisIt_VariableData_alloc(&hmix_zone);
        VisIt_VariableData_setDataI(hmix_zone, VISIT_OWNER_SIM, 1, nTuples, mix_zone);

        VisIt_VariableData_alloc(&hmix_mat);
        VisIt_VariableData_setDataI(hmix_mat, VISIT_OWNER_SIM, 1, nTuples, mix_mat);

        VisIt_VariableData_alloc(&hmix_vf);
        VisIt_VariableData_setDataD(hmix_vf, VISIT_OWNER_SIM, 1, nTuples, mix_vf);

        VisIt_VariableData_alloc(&hmix_next);
        VisIt_VariableData_setDataI(hmix_next, VISIT_OWNER_SIM, 1, nTuples, mix_next);

        VisIt_MaterialData_setMixedMaterials(h, hmix_mat, hmix_zone, hmix_next, hmix_vf);
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Apr 10 11:47:56 PDT 2008
 *
 * Modifications:
 *   Brad Whitlock, Thu Feb 26 10:21:57 PST 2009
 *   Use SimV2 API.
 *
 *****************************************************************************/

/* Make a constant field of 1. on the mesh. We'll break up the 1.0 into
 * pieces in the mixvar.
 */
#define C1 1.f
#define C2 2.f
#define C3 3.f
#define C4 4.f
float zonal_scalar[NY-1][NX-1] = {
{C1*1.f, C2*1.f, C3*1.f, C4*1.f},
{C1*1.f, C2*1.f, C3*1.f, C4*1.f},
{C1*1.f, C2*1.f, C3*1.f, C4*1.f}
};

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "scalar") == 0)
    {
        if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
        {
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                (NX-1) * (NY-1), &zonal_scalar[0][0]); 
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns mixed scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *   Brad Whitlock, Thu Feb 26 10:21:57 PST 2009
 *   Use SimV2 API.
 *
 *****************************************************************************/

/* The mesh has some mixed cells. Mixed scalars are only defined for
 * those mixed cells. That means that clean cells have no entries
 * in the mixed scalar array. For this example, we're reproducing the
 * volume fractions for the mixed cells as the mixvar data. All cells in
 * mesh have a comment indicating their location, even if they add no data.
 * Cells that have data provide their data after the comment. See the
 * mat_vf array in the GetMaterial function to draw comparisons.
 */
float mixvar[] = {
/*cell 0,0*/ /*cell 0,1*/ C2*0.75,C2*0.25,  /*cell 0,2*/ C3*0.8125,C3*0.1875, /*cell 0,3*/
/*cell 1,0*/ /*cell 1,1*/ C2*0.625,C2*0.375,/*cell 1,2*/ C3*0.5625,C3*0.4375, /*cell 1,3*/
/*cell 2,0*/ /*cell 2,1*/ C2*0.3,C2*0.7,    /*cell 2,2*/ C3*0.2,C3*0.4,C3*0.4,   /*cell 2,3*/C4*0.55,C4*0.45
};

visit_handle
SimGetMixedVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "scalar") == 0)
    {
        if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
        {
            int nTuples = sizeof(mixvar) / sizeof(float);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                nTuples, mixvar);
        }
    }

    return h;
}

