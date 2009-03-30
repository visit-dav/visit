/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
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

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Data Access Function prototypes. */
int SimGetMetaData(VisIt_SimulationMetaData *, void *);
int SimGetMesh(int, const char *, VisIt_MeshData *, void *);
int SimGetVariable(int, const char *, VisIt_VariableData *, void *);

void simulate_one_timestep(void);
void read_input_deck(void) { }
int  simulation_done(void)   { return 0; }

/* Is the simulation in run mode (not waiting for VisIt input) */
static int    runFlag = 1;
static int    simcycle = 0;
static double simtime = 0.;

/******************************************************************************
 *
 * Purpose: Callback function for control commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:39:59 PST 2007
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
printf("ControlCommandCallback cmd=%s\n", cmd);

    if(strcmp(cmd, "halt") == 0)
        runFlag = 0;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep();
    else if(strcmp(cmd, "run") == 0)
        runFlag = 1;
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:35:53 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(void)
{
    int blocking, visitstate, err = 0;

    do
    {
        blocking = runFlag ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        visitstate = VisItDetectInput(blocking, -1);

        /* Do different things depending on the output from VisItDetectInput. */
        if(visitstate >= -5 && visitstate <= -1)
        {
            fprintf(stderr, "Can't recover from error!\n");
            err = 1;
        }
        else if(visitstate == 0)
        {
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep();
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection())
            {
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback, NULL);

                VisItSetGetMetaData(SimGetMetaData, NULL);
                VisItSetGetMesh(SimGetMesh, NULL);
                VisItSetGetVariable(SimGetVariable, NULL);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            runFlag = 0;
            if(!VisItProcessEngineCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                runFlag = 1;
            }
        }
    } while(!simulation_done() && err == 0);
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
    /* Initialize environment variables. */
    VisItSetupEnvironment();
    /* Write out .sim file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("var",
        "Demonstrates variable data access function",
        "/path/to/where/sim/was/started",
        NULL, NULL, NULL);

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    return 0;
}

/* SIMULATE ONE TIME STEP */
#include <unistd.h>
void simulate_one_timestep(void)
{
    ++simcycle;
    simtime += 0.0134;
    printf("Simulating time step: cycle=%d, time=%lg\n", simcycle, simtime);
    sleep(1);
}

/* DATA ACCESS FUNCTIONS */
#include <VisItDataInterface_V2.h>

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

int
SimGetMetaData(VisIt_SimulationMetaData *md, void *cbdata)
{
    size_t sz;

    /* Set the simulation state. */
    md->currentMode = runFlag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
    md->currentCycle = simcycle;
    md->currentTime = simtime;

#define NDOMAINS 1
    /* Allocate enough room for 2 meshes in the metadata. */
    md->numMeshes = 2;
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);

    /* Set the first mesh's properties.*/
    md->meshes[0].name = strdup("mesh2d");
    md->meshes[0].meshType = VISIT_MESHTYPE_RECTILINEAR;
    md->meshes[0].topologicalDimension = 2;
    md->meshes[0].spatialDimension = 2;
    md->meshes[0].numBlocks = NDOMAINS;
    md->meshes[0].blockTitle = strdup("Domains");
    md->meshes[0].blockPieceName = strdup("domain");
    md->meshes[0].numGroups = 0;
    md->meshes[0].units = strdup("cm");
    md->meshes[0].xLabel = strdup("Width");
    md->meshes[0].yLabel = strdup("Height");
    md->meshes[0].zLabel = strdup("Depth");

    /* Set the second mesh's properties.*/
    md->meshes[1].name = strdup("mesh3d");
    md->meshes[1].meshType = VISIT_MESHTYPE_CURVILINEAR;
    md->meshes[1].topologicalDimension = 3;
    md->meshes[1].spatialDimension = 3;
    md->meshes[1].numBlocks = NDOMAINS;
    md->meshes[1].blockTitle = strdup("Domains");
    md->meshes[1].blockPieceName = strdup("domain");
    md->meshes[1].numGroups = 0;
    md->meshes[1].units = strdup("Miles");
    md->meshes[1].xLabel = strdup("Width");
    md->meshes[1].yLabel = strdup("Height");
    md->meshes[1].zLabel = strdup("Depth");

    /* Add some variables. */
    md->numVariables = 5;
    sz = sizeof(VisIt_VariableMetaData) * md->numVariables;
    md->variables = (VisIt_VariableMetaData *)malloc(sz);
    memset(md->variables, 0, sz);

    /* Add a zonal scalar variable on mesh2d. */
    md->variables[0].name = strdup("zonal_scalar");
    md->variables[0].meshName = strdup("mesh2d");
    md->variables[0].type = VISIT_VARTYPE_SCALAR;
    md->variables[0].centering = VISIT_VARCENTERING_ZONE;

    /* Add a nodal scalar variable on mesh3d. */
    md->variables[1].name = strdup("nodal_scalar");
    md->variables[1].meshName = strdup("mesh3d");
    md->variables[1].type = VISIT_VARTYPE_SCALAR;
    md->variables[1].centering = VISIT_VARCENTERING_NODE;

    /* Add a zonal vector variable on mesh2d. */
    md->variables[2].name = strdup("zonal_vector");
    md->variables[2].meshName = strdup("mesh2d");
    md->variables[2].type = VISIT_VARTYPE_VECTOR;
    md->variables[2].centering = VISIT_VARCENTERING_ZONE;

    /* Add a nodal vector variable on mesh3d. */
    md->variables[3].name = strdup("nodal_vector");
    md->variables[3].meshName = strdup("mesh3d");
    md->variables[3].type = VISIT_VARTYPE_VECTOR;
    md->variables[3].centering = VISIT_VARCENTERING_NODE;

    /* Add a zonal label variable on mesh2d. */
    md->variables[4].name = strdup("zonal_label");
    md->variables[4].meshName = strdup("mesh2d");
    md->variables[4].type = VISIT_VARTYPE_LABEL;
    md->variables[4].centering = VISIT_VARCENTERING_ZONE;

    /* Add some custom commands. */
    md->numGenericCommands = 3;
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

    return VISIT_OKAY;
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

int
SimGetMesh(int domain, const char *name, VisIt_MeshData *mesh, void *cbdata)
{
    int ret = VISIT_ERROR;
    size_t sz;

    if(strcmp(name, "mesh2d") == 0)
    {
        /* Make VisIt_MeshData contain a VisIt_RectilinearMesh. */
        sz = sizeof(VisIt_RectilinearMesh);
        mesh->rmesh = (VisIt_RectilinearMesh *)malloc(sz);
        memset(mesh->rmesh, 0, sz);

        /* Tell VisIt which mesh object to use. */
        mesh->meshType = VISIT_MESHTYPE_RECTILINEAR;

        /* Set the mesh's number of dimensions. */
        mesh->rmesh->ndims = rmesh_ndims;

        /* Set the mesh dimensions. */
        mesh->rmesh->dims[0] = rmesh_dims[0];
        mesh->rmesh->dims[1] = rmesh_dims[1];
        mesh->rmesh->dims[2] = rmesh_dims[2];

        mesh->rmesh->baseIndex[0] = 0;
        mesh->rmesh->baseIndex[1] = 0;
        mesh->rmesh->baseIndex[2] = 0;

        mesh->rmesh->minRealIndex[0] = 0;
        mesh->rmesh->minRealIndex[1] = 0;
        mesh->rmesh->minRealIndex[2] = 0;
        mesh->rmesh->maxRealIndex[0] = rmesh_dims[0]-1;
        mesh->rmesh->maxRealIndex[1] = rmesh_dims[1]-1;
        mesh->rmesh->maxRealIndex[2] = rmesh_dims[2]-1;

        /* Let VisIt use the simulation's copy of the mesh coordinates. */
        mesh->rmesh->xcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, rmesh_x);
        mesh->rmesh->ycoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, rmesh_y);

        ret = VISIT_OKAY;
    }
    else if(strcmp(name, "mesh3d") == 0)
    {
        /* Make VisIt_MeshData contain a VisIt_CurvilinearMesh. */
        sz = sizeof(VisIt_CurvilinearMesh);
        mesh->cmesh = (VisIt_CurvilinearMesh *)malloc(sz);
        memset(mesh->cmesh, 0, sz);

        /* Tell VisIt which mesh object to use. */
        mesh->meshType = VISIT_MESHTYPE_CURVILINEAR;

        /* Set the mesh's number of dimensions. */
        mesh->cmesh->ndims = cmesh_ndims;

        /* Set the mesh dimensions. */
        mesh->cmesh->dims[0] = cmesh_dims[0];
        mesh->cmesh->dims[1] = cmesh_dims[1];
        mesh->cmesh->dims[2] = cmesh_dims[2];

        mesh->cmesh->baseIndex[0] = 0;
        mesh->cmesh->baseIndex[1] = 0;
        mesh->cmesh->baseIndex[2] = 0;

        mesh->cmesh->minRealIndex[0] = 0;
        mesh->cmesh->minRealIndex[1] = 0;
        mesh->cmesh->minRealIndex[2] = 0;
        mesh->cmesh->maxRealIndex[0] = cmesh_dims[0]-1;
        mesh->cmesh->maxRealIndex[1] = cmesh_dims[1]-1;
        mesh->cmesh->maxRealIndex[2] = cmesh_dims[2]-1;

        /* Let VisIt use the simulation's copy of the mesh coordinates. */
        mesh->cmesh->xcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, (float *)cmesh_x);
        mesh->cmesh->ycoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, (float *)cmesh_y);
        mesh->cmesh->zcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, (float *)cmesh_z);

        ret = VISIT_OKAY;
    }

    return ret;
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

int
SimGetVariable(int domain, const char *name, VisIt_VariableData *var, void *cbdata)
{
    int ret = VISIT_OKAY;

    if(strcmp(name, "zonal_scalar") == 0)
    {
        var->nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
        var->data = VisIt_CreateDataArrayFromFloat(
            VISIT_OWNER_SIM, zonal);
    }
    else if(strcmp(name, "nodal_scalar") == 0)
    {
        var->nTuples = cmesh_dims[0] * cmesh_dims[1] *
            cmesh_dims[2];
        var->data = VisIt_CreateDataArrayFromDouble(
            VISIT_OWNER_SIM, (double*)nodal);
    }
    else if(strcmp(name, "zonal_vector") == 0)
    {
        var->nComponents = 2;
        var->nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
        var->data = VisIt_CreateDataArrayFromFloat(
            VISIT_OWNER_SIM, (float *)zonal_vector);
    }
    else if(strcmp(name, "nodal_vector") == 0)
    {
        var->nComponents = 3;
        var->nTuples = cmesh_dims[0] * cmesh_dims[1] *
            cmesh_dims[2];
        var->data = VisIt_CreateDataArrayFromDouble(
            VISIT_OWNER_SIM, (double *)nodal_vector);
    }
    else if(strcmp(name, "zonal_label") == 0)
    {
        var->nComponents = 7;
        var->nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
        var->data = VisIt_CreateDataArrayFromChar(
            VISIT_OWNER_SIM, (char *)zonal_labels);
    }
    else 
    {
        ret = VISIT_ERROR;
    }

    return ret;
}
