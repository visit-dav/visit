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

const char *cmd_names[] = {"halt", "step", "run"};

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
    printf("Simulating time step: cycle=%d, time=%g\n", sim->cycle, sim->time);
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
                VisItSetGetVariable(SimGetVariable, (void*)&sim);
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
    VisItInitializeSocketAndDumpSimFile("aresamr",
        "Demonstrates domain nesting data plus ghost zones",
        "/no/useful/path",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    return 0;
}

/* DATA ACCESS FUNCTIONS */

/* AMR mesh */
#define NLEVELS  2
#define NPATCHES 2
/* The level in which each patch resides. */
int   level[NPATCHES]     = {0, 1};

/* The number of child patches for each patch. */
int   ncpatch[NPATCHES]   = {1,0};

/* The patch indices of the child patches for each patch. A -1 indicates
 * no child patch.
 */
int   cpatch[NPATCHES][2] = {{1,-1},{-1,-1}};

/* The x,y,z refinement ratios.*/
int   ratios[3]           = {2,2,1};

/* Spatial extents of patches. */
float rmx[NPATCHES][2]    = {{-2., 12.}, {3., 12.}};
float rmy[NPATCHES][2]    = {{-2., 12.}, {-2., 12.}};

/* Tell where cells exist in the global mesh view and the indices are
   given in the patch's level (i.e. level 1 in a 2x refinement would have
   2x as many valid indices).
 
   patch0 is a 10x10 mesh with 2 layers of ghost zones, giving a 14x14 mesh.
      * Since patch0 covers the whole 14x14 mesh, we pass 0-13 as extents in x,y.

   patch1 is supposed to occupy the right half of patch0's real zones and 
   refine 2x.
      * The min x index for patch 1 is:
             (nghostlayers + patch0.width/2 - nghostlayers) * refinement
           = (2 + 10 / 2 - 2) * 2
           = 10
      * Since patch1's max extents cover the right half of the 14x14 mesh
        of patch0, we use 14*refinement-1 = 27 as the max index.
 */
int   rmxext[NPATCHES][2] = {{0,13}, {10,27}};
int   rmyext[NPATCHES][2] = {{0,13}, {0,27}};

#define USE_GHOST_ZONES
#ifdef USE_GHOST_ZONES
/* The presence of ghost zones affects the domain nesting. What you have to
   imagine is that the ghost zones get removed prior to the domain nesting
   object getting used. This means that different indices must be used.

   In our example, patch0 is supposed to be a 10x10 mesh but we added 2 ghost
   zone layers in X and Y so we get a 14x14 mesh. This would normally mean that
   we provide 0,13 for the X extents of patch0. However, since ghost zones are
   removed first, that removes the 2 extra layers of ghost zones on each side so
   must therefore pass 0,9 for the nesting for patch0.

   Patch1 is supposed to cover  the right half of patch1 so it would normally
   occupy indices 10-27. Again, since ghost zones get removed, we have to specify
   the indices relative to patch0 without ghost zones so we pass 10-19.
*/
int   rmxext_sdn[NPATCHES][2] = {{0,9}, {10,19}};
int   rmyext_sdn[NPATCHES][2] = {{0,9}, {0,19}};

/* We have ghost zones and these indices are relative to the local size of
   patch0 and patch1. These indices say which zones are real.

   patch0's real zones begin at zone 2 and end at zone 12 since we have 2 ghost
   zone layers.

   patch1 ends up being a 18x28 mesh. The 2 layers of ghosting we had at patch0
   become 4 layers here (at least in this example) so we adjust the dimensions
   of the mesh by 4.
 */
int   min_real_indices[NPATCHES][3] = {{2,2,0},   {4,4,0}};
int   max_real_indices[NPATCHES][3] = {{12,12,0}, {14,24}};

#else
/* Just consider all zones valid. This gives domain nesting indices equivalent
   to rmxext, rmyext.
 */
int   rmxext_sdn[NPATCHES][2] = {{0,13}, {10,27}};
int   rmyext_sdn[NPATCHES][2] = {{0,13}, {0,27}};
#endif

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
        visit_handle vmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Set the first mesh's properties.*/
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
#ifdef VARYING_NUMBER_OF_PATCHES
            /* Vary the number of patches based on the cycle so we can test
             * AMR SILs that change over time.
             */
            int np = NPATCHES-1;
            if(sim->cycle % 2 == 1)
                np = NPATCHES;
#else
            int np = NPATCHES; /*FOR NOW*/
#endif
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "amr");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_AMR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, np);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Patches");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "patch");
            VisIt_MeshMetaData_setNumGroups(mmd, NLEVELS);
            VisIt_MeshMetaData_setGroupTitle(mmd, "Levels");
            VisIt_MeshMetaData_setGroupPieceName(mmd, "level");
            for(i = 0; i < np; ++i)
                VisIt_MeshMetaData_addGroupId(mmd, level[i]);
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a zonal scalar variable on mesh2d. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar");
            VisIt_VariableMetaData_setMeshName(vmd, "amr");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add some custom commands. */
        for(i = 0; i < (int) (sizeof(cmd_names)/sizeof(const char *)); ++i)
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

#ifdef USE_GHOST_ZONES
            /* Indicate which zones are real, also telling us which are ghost. */
            VisIt_RectilinearMesh_setRealIndices(h, 
                min_real_indices[domain], max_real_indices[domain]);
#endif
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
    int nComponents = 1;

    if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        if(strcmp(name, "zonal_scalar") == 0)
        {
            int   i, j, nx, ny, nTuples, idx = 0;
            float t, t0, t1, x, y, *rad = NULL;

            /* The number of nodes. */
            nx = rmxext[domain][1] - rmxext[domain][0]+1+1;
            ny = rmyext[domain][1] - rmyext[domain][0]+1+1;
            nTuples = (nx-1)*(ny-1);

            /* Compute distance of cell center to CX,CY */
#define CX 3.
#define CY 5.
            rad = (float *)malloc(sizeof(float)*nTuples);
            for(j = 0; j < ny-1; ++j)
            {
                t0 = (float)j / (float)(ny);
                t1 = (float)(j+1) / (float)(ny);
                t = (t0 + t1) / 2.;
                y = (1.-t)*rmy[domain][0] + t*rmy[domain][1];

                for(i = 0; i < nx-1; ++i)
                {
                    t0 = (float)i / (float)(nx);
                    t1 = (float)(i+1) / (float)(nx);
                    t = (t0 + t1) / 2.;
                    x = (1.-t)*rmx[domain][0] + t*rmx[domain][1];
                    
                    rad[idx++] = sqrt((x-CX)*(x-CX) + (y-CY)*(y-CY));
                }
            }

            VisIt_VariableData_setDataF(h, VISIT_OWNER_VISIT, nComponents,
                nTuples, rad);
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

    if(VisIt_DomainNesting_alloc(&h) != VISIT_ERROR)
    {
#define XMIN 0
#define YMIN 1
#define ZMIN 2
#define XMAX 3
#define YMAX 4
#define ZMAX 5
        int i, dom, nlevels = NLEVELS;
        int ext[6]={0,0,0,0,0,0}, patch[2]={0,0};

        /* Vary the number of patches based on the cycle so we can test
         * AMR SILs that change over time.
         */
#ifdef VARYING_NUMBER_OF_PATCHES
        int np = NPATCHES-1;
        simulation_data *sim = (simulation_data *)cbdata;
        if(sim->cycle % 2 == 1)
        {
            np = NPATCHES;
            ncpatch[1] = 2;
        }
        else
        {
            ncpatch[1] = 1;
        }
#else
        int np = NPATCHES;
#endif
        VisIt_DomainNesting_set_dimensions(h, np, nlevels, 2);

        for(i = 0; i < nlevels; ++i)
            VisIt_DomainNesting_set_levelRefinement(h, i, ratios);

        for(dom = 0; dom < np; ++dom)
        {
            ext[XMIN] = rmxext_sdn[dom][0];
            ext[YMIN] = rmyext_sdn[dom][0];
            ext[ZMIN] = 0;
            ext[XMAX] = rmxext_sdn[dom][1];
            ext[YMAX] = rmyext_sdn[dom][1];
            ext[ZMAX] = 0;
            for(i = 0; i < ncpatch[dom]; ++i)
                patch[i] = cpatch[dom][i];
            VisIt_DomainNesting_set_nestingForPatch(h, dom, level[dom],
                patch, ncpatch[dom], ext);
        }
    }

    return h;
}
