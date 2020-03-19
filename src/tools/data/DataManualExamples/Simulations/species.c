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
visit_handle SimGetSpecies(int domain, const char *name, void *cbdata);
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
                VisItSetGetSpecies(SimGetSpecies, (void*)sim);
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
    VisItInitializeSocketAndDumpSimFile("species",
        "Demonstrates species data access function",
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
    {{1.f,0.f,0.f},{0.75f,0.25f,0.f},  {0.8125f,0.1875f, 0.f},{1.f,0.f,0.f}},
    {{1.f,0.f,0.f},{0.625f,0.375f,0.f},{0.5625f,0.4375f,0.f}, {1.f,0.f,0.f}},
    {{1.f,0.f,0.f},{0.3f,0.7f,0.f},    {0.2f,0.4f,0.4f},      {0.55f,0.45f,0.f}}
};

/* Species data. */
int nmaterialSpecies[3] = {3, 1, 4};
const char *materialSpecies[3][4] = {
    {"H2O", "Ca", "NaCl", NULL},
    {"Latex", NULL, NULL, NULL},
    {"N2", "O2", "CO2", "Ar"}
};
const float matspeciesMF[3][4] = {
    {0.99f, 0.0005f, 0.0095f, 0.f },
    {1.f,   0.f,     0.f,     0.f},
    {0.7f,  0.2f,    0.06f,   0.04f}
};

visit_handle
SpeciesNames(int i)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(VisIt_NameList_alloc(&h) == VISIT_OKAY)
    {
        int j;
        for(j = 0; j < nmaterialSpecies[i]; ++j)
            VisIt_NameList_addName(h, materialSpecies[i][j]);
    }
    return h;
}

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
        visit_handle smd = VISIT_INVALID_HANDLE;

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

        /* Add species */
        if(VisIt_SpeciesMetaData_alloc(&smd) == VISIT_OKAY)
        {
            VisIt_SpeciesMetaData_setName(smd, "Species");
            VisIt_SpeciesMetaData_setMeshName(smd, "mesh2d");
            VisIt_SpeciesMetaData_setMaterialName(smd, "Material");
            VisIt_SpeciesMetaData_addSpeciesName(smd, SpeciesNames(0));
            VisIt_SpeciesMetaData_addSpeciesName(smd, SpeciesNames(1));
            VisIt_SpeciesMetaData_addSpeciesName(smd, SpeciesNames(2));

            VisIt_SimulationMetaData_addSpecies(md, smd);
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
    if(VisIt_MaterialData_alloc(&h) == VISIT_OKAY)
    {
        int i, j, m, cell = 0;
        int nmats, cellmat[10], matnos[3]={1,2,3};
        float cellmatvf[10];

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

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns species data.
 *
 * Programmer: Brad Whitlock
 * Date:       Mon Feb  9 15:27:09 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetSpecies(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(VisIt_SpeciesData_alloc(&h) == VISIT_OKAY)
    {
        visit_handle hspecies = VISIT_INVALID_HANDLE;
        visit_handle hspeciesMF = VISIT_INVALID_HANDLE;
        visit_handle hmixedSpecies = VISIT_INVALID_HANDLE;

        int *species = NULL, *mixedSpecies = NULL;
        int c, mixc, mfc, i, j;
        float *speciesMF = NULL;

        /* Allocate the species arrays */
        species = (int *)malloc(100 * sizeof(int));
        memset(species, 0, 100 * sizeof(int));

        mixedSpecies = (int *)malloc(100 * sizeof(int));
        memset(mixedSpecies, 0, 100 * sizeof(int));
    
        speciesMF = (float *)malloc(100 * sizeof(float));
        memset(speciesMF, 0, 100 * sizeof(float));

        /* Populate the species arrays based on our simpler data */
        c = 0;
        mixc = 0;
        mfc = 0;
        for (j = 0; j < NY-1; j++)
        {
            for (i = 0; i < NX-1; i++, c++)
            {
                int m, mi, s, nmats = 0;
                /* Count the number of materials in the cell. */
                for(mi = 0; mi < 3; ++mi)
                {
                    if(matlist[j][i][mi] > 0)
                        nmats++;
                }   

                if (nmats == 1)
                {
                    m = matlist[j][i][0]-1;
    
                    if (nmaterialSpecies[m] == 1)
                    {
                        /* This 1 material has 1 species. */
                        species[c] = 0;
                    }
                    else
                    {
                        /* This 1 material has many species */
                        species[c] = mfc + 1; /* 1-origin */
                        for (s = 0; s < nmaterialSpecies[m]; s++)
                        {
                            speciesMF[mfc] = matspeciesMF[m][s];
                            mfc++;
                        }
                    }
                }
                else
                {
                    /* There are mixed materials */
                    species[c] = -mixc - 1;

                    for (mi = 0; mi < nmats; ++mi)
                    {
                        m = matlist[j][i][mi]-1;

                        if (nmaterialSpecies[m] == 1)
                        {
                            /* The current material has 1 species. */
                            mixedSpecies[mixc] = 0;
                        }
                        else
                        {
                            /* The current material has many species. */
                            int     s;
                            mixedSpecies[mixc] = mfc + 1; /* 1-origin */
                            for (s = 0; s < nmaterialSpecies[m]; s++)
                            {
                                speciesMF[mfc] = matspeciesMF[m][s];
                                mfc++;
                            }
                        }
                        mixc++;
                    }
                }
            }
        }

        VisIt_SpeciesData_addSpeciesName(h, SpeciesNames(0));
        VisIt_SpeciesData_addSpeciesName(h, SpeciesNames(1));
        VisIt_SpeciesData_addSpeciesName(h, SpeciesNames(2));

        VisIt_VariableData_alloc(&hspecies);
        VisIt_VariableData_setDataI(hspecies, VISIT_OWNER_VISIT, 1,
            (NX-1)*(NY-1), species);
        VisIt_SpeciesData_setSpecies(h, hspecies);

        VisIt_VariableData_alloc(&hspeciesMF);
        VisIt_VariableData_setDataF(hspeciesMF, VISIT_OWNER_VISIT, 1,
            mfc, speciesMF);
        VisIt_SpeciesData_setSpeciesMF(h, hspeciesMF);

        VisIt_VariableData_alloc(&hmixedSpecies);
        VisIt_VariableData_setDataI(hmixedSpecies, VISIT_OWNER_VISIT, 1,
            mixc, mixedSpecies);
        VisIt_SpeciesData_setMixedSpecies(h, hmixedSpecies);
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

