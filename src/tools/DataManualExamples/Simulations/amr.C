/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "patch.h"
#include "SimulationExample.h"

/* Data Access Function prototypes */
int SimGetMetaData(VisIt_SimulationMetaData *, void *);
int SimGetMesh(int, const char *, VisIt_MeshData *, void *);
int SimGetVariable(int, const char *, VisIt_VariableData *, void *);
int SimGetDomainNesting(const char *, visit_handle, void *);

/******************************************************************************
 * Code for calculating data values
 ******************************************************************************/

class complex
{
public:
    complex() : a(0.), b(0.) { }
    complex(float A, float B) : a(A), b(B) { }
    complex(const complex &obj) : a(obj.a), b(obj.b) { }
    complex operator = (const complex &obj) { a = obj.a; b = obj.b; return *this;}
    complex operator + (const complex &obj) const
    {
        return complex(a + obj.a,  b + obj.b);
    }
    complex operator * (const complex &obj) const
    {
        return complex(a * obj.a - b * obj.b, a * obj.b + b * obj.a);
    }
    float mag2() const
    {
        return a*a + b*b;
    }
    float mag() const
    {
        return sqrt(a*a + b*b);
    }
private:
    float a,b;
};

#define MAXIT 30

int
mandelbrot(const complex &C)
{
    complex Z;
    for(int zit = 0; zit < MAXIT; ++zit)
    {
        Z = (Z * Z) + C;
        if(Z.mag2() > 4.f)
            return zit+1;
    }
    return 0;
}

void
patch_calculate_data(patch_t *patch)
{
    int i,j;
    float ty,y,tx,x;
    float cellWidth, x0, x1;
    float cellHeight, y0, y1;
    unsigned char *data = patch->data;
    
    /* Compute x0, x1 and y0,y1 which help us locate cell centers. */
    cellWidth = (patch->window[1] - patch->window[0]) / ((float)patch->nx);
    x0 = patch->window[0] + cellWidth / 2.f;
    x1 = patch->window[1] - cellWidth / 2.f;
    cellHeight = (patch->window[3] - patch->window[2]) / ((float)patch->ny);
    y0 = patch->window[2] + cellHeight / 2.f;
    y1 = patch->window[3] - cellHeight / 2.f;
    for(j = 0; j < patch->ny; ++j)
    {
        ty = (float)j / (float)(patch->ny - 1);
        y = (1.f-ty)*y0 + ty*y1;
        for(i = 0; i < patch->nx; ++i)
        {
            tx = (float)i / (float)(patch->nx - 1);
            x = (1.f-tx)*x0 + tx*x1;

            *data++ = (unsigned char)mandelbrot(complex(x, y));
        }
    }
}

/******************************************************************************
 * Code for helping calculate AMR refinement
 ******************************************************************************/

int
neighbors(patch_t *patch, int i, int j)
{
    const float kernel[3][3] = {
    {0.08f, 0.17f, 0.08f},
    {0.17f, 0.f,   0.17f},
    {0.08f, 0.17f, 0.08f}
    };

    float sum = 0;
    for(int jj = 0; jj < 3; ++jj)
    {
        int J = j + jj - 1;
        for(int ii = 0; ii < 3; ++ii)
        {
            int I = i + ii - 1;
            float value = (float)patch->data[J*patch->nx+I];
            sum += value * kernel[jj][ii];
        }
    }
    return (int)sum;
}

void
detect_refinement(patch_t *patch, image_t *mask)
{
    int i,j;
    /* Let's look for large differences within a kernel. This lets us
     * figure out areas that we need to refine because they contain
     * features. We set a 1 into the mask for cells that need refinement
     */
    for(j = 1; j < patch->ny-1; ++j)
        for(i = 1; i < patch->nx-1; ++i)
        {
            int index = j*patch->nx+i;
            int dval = (int)patch->data[index] - neighbors(patch, i, j);
            if(dval < 0) dval = -dval;
            if(dval > 2) 
                mask->data[index] = 1;
            else
                mask->data[index] = 0;
        }
}

/******************************************************************************
 * Calculate the Mandelbrot set using AMR
 ******************************************************************************/

void
calculate_amr_helper(patch_t *patch, int level, int max_levels, int ratio, int *patchid)
{
    int i;

    patch->id = *patchid;
    *patchid = *patchid + 1;
    patch->level = level;

    /* Calculate the data on this patch */
    patch_calculate_data(patch);

    if(level+1 > max_levels)
        return;

    /* Examine this patch's data and see if we need to refine. If we
     * do refine then we'll create subpatches that we can operate on.
     */
    patch_refine(patch, ratio, detect_refinement);

    for(i = 0; i < patch->nsubpatches; ++i)
        calculate_amr_helper(&patch->subpatches[i], level+1, max_levels, ratio, patchid);
}

void
calculate_amr(patch_t *patch, int level, int max_levels, int ratio)
{
    int patchid = 0;
    calculate_amr_helper(patch, level, max_levels, ratio, &patchid);
}

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

struct simulation_data
{
    int     cycle;
    double  time;
    int     runMode;
    int     done;
    int     max_levels;
    int     refinement_ratio;
    int     savingFiles;
    int     saveCounter;
    patch_t patch;
};

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->max_levels = 2;
    sim->refinement_ratio = 4;
    sim->savingFiles = 0;
    sim->saveCounter = 0;
    patch_ctor(&sim->patch);
}

void
simulation_data_dtor(simulation_data *sim)
{
    patch_dtor(&sim->patch);
}

/******************************************************************************
 *
 * Purpose: Called to simulate one timestep of data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 12:14:23 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

void
simulate_one_timestep(simulation_data *sim)
{
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    const float window0[] = {-1.6, 0.6, -1.1, 1.1};
#define ORIGINX -1.5
#define ORIGINY -0.5
#define WSIZE 0.5
    const float window1[] = {ORIGINX, ORIGINX + WSIZE, ORIGINY, ORIGINY + WSIZE};
#define NX 256
#define NY 256

    /* oscillate between 2 windows */
    float window[4];
    float t = 0.5 * sin(sim->time) + 0.5;
    window[0] = (1. - t)*window0[0] + t*window1[0];
    window[1] = (1. - t)*window0[1] + t*window1[1];
    window[2] = (1. - t)*window0[2] + t*window1[2];
    window[3] = (1. - t)*window0[3] + t*window1[3];

    /* Blow away the previous patch data and calculate. */
    patch_dtor(&sim->patch);
    patch_ctor(&sim->patch);
    sim->patch.window[0] = window[0];
    sim->patch.window[1] = window[1];
    sim->patch.window[2] = window[2];
    sim->patch.window[3] = window[3];
    sim->patch.logical_extents[0] = 0;
    sim->patch.logical_extents[1] = NX-1;
    sim->patch.logical_extents[2] = 0;
    sim->patch.logical_extents[3] = NY-1;
    patch_alloc_data(&sim->patch, NX, NY);
    calculate_amr(&sim->patch, 0, sim->max_levels, sim->refinement_ratio);

    /* If we're saving files then save one now. */
    if(VisItIsConnected())
    {
        VisItTimeStepChanged();
        if(sim->savingFiles)
        {
            VisItUpdatePlots();

            char filename[100];
            sprintf(filename, "amr%04d.jpg", sim->saveCounter++);
            if(VisItSaveWindow(filename, 480, 480, VISIT_IMAGEFORMAT_JPEG) == VISIT_OKAY)
                printf("Saved %s\n", filename);
            else
                printf("The image could not be saved to %s\n", filename);
        }
    }

    ++sim->cycle;
    sim->time += (M_PI / 30.);
}

/******************************************************************************
 *
 * Purpose: Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 12:14:23 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

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
    else if(strcmp(cmd, "saveon") == 0)
        sim->savingFiles = 1;
    else if(strcmp(cmd, "saveoff") == 0)
        sim->savingFiles = 0;
    else if(strcmp(cmd, "reset") == 0)
    {
        patch_dtor(&sim->patch);
        simulation_data_ctor(sim);
    }
    else if(strncmp(cmd, "levels", 6) == 0)
    {
        int level;
        sscanf(cmd+6+1, "%d", &level);
        if(level > 1 && level < 10)
            sim->max_levels = level;
    }
    else if(strncmp(cmd, "ratio", 5) == 0)
    {
        int ratio;
        sscanf(cmd+5+1, "%d", &ratio);
        if(ratio > 1 && ratio < 10)
            sim->refinement_ratio = ratio;
    }
}

/******************************************************************************
 *
 * Purpose: Callback function for control commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Input Arguments:
 *   cmd         : The command string that we want the sim to execute.
 *   string_data : String argument for the command.
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

void read_input_deck(void) { }

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 12:14:23 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(void)
{
    int blocking, visitstate, err = 0;

    // Set up some simulation data.
    simulation_data sim;
    simulation_data_ctor(&sim);

    /* If we're not running by default then simulate once there's something
     * once VisIt connects.
     */
    if(sim.runMode == SIM_STOPPED)
        simulate_one_timestep(&sim);

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

    // Clean up
    simulation_data_dtor(&sim);
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 11:54:04 PDT 2009
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
    VisItOpenTraceFile("amr_trace.txt");

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("amr",
        "Demonstrates creating an AMR mesh",
        "/path/to/where/sim/was/started",
        NULL, NULL, NULL);

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    VisItCloseTraceFile();

    return 0;
}

/* DATA ACCESS FUNCTIONS */

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 11:54:04 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

int
SimGetMetaData(VisIt_SimulationMetaData *md, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    int i, nlevels, *pcount;
    size_t sz;

    /* Set the simulation state. */
    md->currentMode = (sim->runMode == SIM_STOPPED) ? VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING;
    md->currentCycle = sim->cycle;
    md->currentTime = sim->time;

    /* Allocate enough room for 1 mesh in the metadata. */
    md->numMeshes = 1;
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);

    /* Fill in the AMR metadata. */
    nlevels = patch_num_levels(&sim->patch);
    md->meshes[0].name = strdup("AMR_mesh");
    md->meshes[0].meshType = VISIT_MESHTYPE_AMR;
    md->meshes[0].topologicalDimension = 2;
    md->meshes[0].spatialDimension = 2;
    md->meshes[0].numBlocks = patch_num_patches(&sim->patch);
    md->meshes[0].blockTitle = strdup("patches");
    md->meshes[0].blockPieceName = strdup("patch");
    md->meshes[0].numGroups = nlevels;
    md->meshes[0].groupTitle = strdup("levels");
    md->meshes[0].groupPieceName = strdup("level");
    md->meshes[0].groupIds = (int *)malloc(md->meshes[0].numBlocks * sizeof(int));
    patch_t **patches = patch_flat_array(&sim->patch);
    md->meshes[0].blockNames = (char**)malloc(md->meshes[0].numBlocks * sizeof(char*));
    pcount = (int *)malloc(nlevels * sizeof(int));
    memset(pcount, 0, nlevels * sizeof(int));
    for(i = 0; i < md->meshes[0].numBlocks; ++i)
    {
        char tmpName[100];
        sprintf(tmpName, "level%d,patch%04d", patches[i]->level, pcount[patches[i]->level]++);
        md->meshes[0].blockNames[i] = strdup(tmpName);
        md->meshes[0].groupIds[i] = patches[i]->level;
    }
    FREE(pcount);
    FREE(patches);
    /* nothing to set the blockpiecenames yet. */
    md->meshes[0].units = strdup("cm");
    md->meshes[0].xLabel = strdup("Real");
    md->meshes[0].yLabel = strdup("Imaginary");

    /* Add a variable. */
    md->numVariables = 1;
    sz = sizeof(VisIt_VariableMetaData) * md->numVariables;
    md->variables = (VisIt_VariableMetaData *)malloc(sz);
    memset(md->variables, 0, sz);

    /* Add a zonal variable on mesh2d. */
    md->variables[0].name = strdup("mandelbrot");
    md->variables[0].meshName = strdup("AMR_mesh");
    md->variables[0].type = VISIT_VARTYPE_SCALAR;
    md->variables[0].centering = VISIT_VARCENTERING_ZONE;

    /* Add some custom commands. */
    md->numGenericCommands = 4;
    sz = sizeof(VisIt_SimulationControlCommand) * md->numGenericCommands;
    md->genericCommands = (VisIt_SimulationControlCommand *)malloc(sz);
    memset(md->genericCommands, 0, sz);

    md->genericCommands[0].name = strdup("halt");
    md->genericCommands[0].argType = VISIT_CMDARG_NONE;
    md->genericCommands[0].enabled = 1;

    md->genericCommands[1].name = strdup("step");
    md->genericCommands[1].argType = VISIT_CMDARG_NONE;
    md->genericCommands[1].enabled = 1;

    md->genericCommands[2].name = strdup("run");
    md->genericCommands[2].argType = VISIT_CMDARG_NONE;
    md->genericCommands[2].enabled = 1;

    md->genericCommands[3].name = strdup("update");
    md->genericCommands[3].argType = VISIT_CMDARG_NONE;
    md->genericCommands[3].enabled = 1;

    return VISIT_OKAY;
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 11:54:04 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

int
SimGetMesh(int domain, const char *name, VisIt_MeshData *mesh, void *cbdata)
{
    size_t sz;
    int ret = VISIT_ERROR;
    /* Get the patch with the appropriate domain id. */
    simulation_data *sim = (simulation_data *)cbdata;
    patch_t *patch = patch_get_patch(&sim->patch, domain);

    if(strcmp(name, "AMR_mesh") == 0 && patch != NULL)
    {
        int i;

        /* Make VisIt_MeshData contain a VisIt_RectilinearMesh. */
        sz = sizeof(VisIt_RectilinearMesh);
        mesh->rmesh = (VisIt_RectilinearMesh *)malloc(sz);
        memset(mesh->rmesh, 0, sz);

        /* Tell VisIt which mesh object to use. */
        mesh->meshType = VISIT_MESHTYPE_RECTILINEAR;

        /* Set the mesh's number of dimensions. */
        mesh->rmesh->ndims = 2;

        /* Set the mesh dimensions. */
        mesh->rmesh->dims[0] = patch->nx+1;
        mesh->rmesh->dims[1] = patch->ny+1;
        mesh->rmesh->dims[2] = 1;

        mesh->rmesh->baseIndex[0] = 0;
        mesh->rmesh->baseIndex[1] = 0;
        mesh->rmesh->baseIndex[2] = 0;

        mesh->rmesh->minRealIndex[0] = 0;
        mesh->rmesh->minRealIndex[1] = 0;
        mesh->rmesh->minRealIndex[2] = 0;
        mesh->rmesh->maxRealIndex[0] = patch->nx;
        mesh->rmesh->maxRealIndex[1] = patch->ny;
        mesh->rmesh->maxRealIndex[2] = 1;

        /* Initialize X coords. */
        float *coordX = (float *)malloc(sizeof(float) * (patch->nx+1));
        float width  = sim->patch.window[1] - sim->patch.window[0];
        float x0 = (patch->window[0] - sim->patch.window[0]) / width;
        float x1 = (patch->window[1] - sim->patch.window[0]) / width;
        for(i = 0; i < (patch->nx+1); ++i)
        {
            float t = float(i) / float(patch->nx);
            coordX[i] = (1.-t)*x0 + t*x1;
        }
        /* Initialize Y coords. */
        float *coordY = (float *)malloc(sizeof(float) * (patch->ny+1));
        float height = sim->patch.window[3] - sim->patch.window[2];
        float y0 = (patch->window[2] - sim->patch.window[2]) / height;
        float y1 = (patch->window[3] - sim->patch.window[2]) / height;
        for(i = 0; i < (patch->ny+1); ++i)
        {
            float t = float(i) / float(patch->ny);
            coordY[i] = (1.-t)*y0 + t*y1;
        }

        /* Let VisIt use the simulation's copy of the mesh coordinates. */
        mesh->rmesh->xcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_VISIT, coordX);
        mesh->rmesh->ycoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_VISIT, coordY);

        ret = VISIT_OKAY;
    }

    return ret;
}

/******************************************************************************
 *
 * Purpose: This callback function returns the domain nesting for the mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 14:31:51 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

int
SimGetDomainNesting(const char *name, visit_handle nesting, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;

    int npatches = patch_num_patches(&sim->patch);
    int nlevels = patch_num_levels(&sim->patch);
    VisIt_DomainNesting_set_dimensions(nesting, npatches, nlevels, 2);

    int ratios[3];
    for(int level = 0; level < nlevels; ++level)
    {
        if(level == 0)
        {
            ratios[0] = ratios[1] = ratios[2] = 1;
            VisIt_DomainNesting_set_levelRefinement(nesting, level, ratios);
        }
        else
        {
            ratios[0] = sim->refinement_ratio;
            ratios[1] = sim->refinement_ratio;
            ratios[2] = 1;
        }
        VisIt_DomainNesting_set_levelRefinement(nesting, level, ratios);
    }

    patch_t **patches = patch_flat_array(&sim->patch);
    int logicalExtents[6];
    for(int i = 0; i < npatches; ++i)
    {
        int *child_patches = NULL;
        int nchild_patches = patches[i]->nsubpatches;
        if(nchild_patches > 0)
        {
            child_patches = ALLOC(nchild_patches, int);
            for(int j = 0; j < nchild_patches; ++j)
                child_patches[j] = patches[i]->subpatches[j].id;
        }

        // logical extents are stored lowI,lowJ,lowL,hiI,hiJ,hiK
        logicalExtents[0] = patches[i]->logical_extents[0];
        logicalExtents[1] = patches[i]->logical_extents[2];
        logicalExtents[2] = 0;
        logicalExtents[3] = patches[i]->logical_extents[1];
        logicalExtents[4] = patches[i]->logical_extents[3];
        logicalExtents[5] = 0;

        VisIt_DomainNesting_set_nestingForPatch(nesting, 
            patches[i]->id, patches[i]->level, child_patches, nchild_patches,
            logicalExtents);

        FREE(child_patches);
    }
    FREE(patches);

    return VISIT_OKAY;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Mar 19 14:31:51 PDT 2009
 *
 * Modifications:
 *
 *****************************************************************************/

int
SimGetVariable(int domain, const char *name, VisIt_VariableData *var, void *cbdata)
{
    int ret = VISIT_ERROR;

    /* Get the patch with the appropriate domain id. */
    simulation_data *sim = (simulation_data *)cbdata;
    patch_t *patch = patch_get_patch(&sim->patch, domain);

    if(strcmp(name, "mandelbrot") == 0 && patch != NULL)
    { 
        var->nTuples = patch->nx * patch->ny;
        var->data = VisIt_CreateDataArrayFromChar(
            VISIT_OWNER_SIM, (char *)patch->data);
        ret = VISIT_OKAY;
    }

    return ret;
}
