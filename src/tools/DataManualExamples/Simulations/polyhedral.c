/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
#include <VisItDataInterface_V2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "SimulationExample.h"

/* Data Access Function prototypes */
int SimGetMetaData(VisIt_SimulationMetaData *, void *);
int SimGetMesh(int, const char *, VisIt_MeshData *, void *);

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

void read_input_deck(void) { }

/* SIMULATE ONE TIME STEP */
#include <unistd.h>
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 30.);
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    sleep(1);
}

/*************************** Polyhedral Mesh variables ***********************/

float xc[] = {
2.5, 2.5, 2.5, 2.5,
1.5, 1.5, 1.5, 1.5,
/*NOTE: the 0.45's are to displace a node so we don't have 3 colinear nodes
        since it will cause the tessellator to ignore a node.
 */
0.5, 0.45 ,0.5, 0.45, 0.5, 0.45, 0.5, 0.45, 0.5,
0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.,  0.
};

float yc[] = {
0., 0., 1., 1.,
0., 0., 1., 1.,
0., 0., 0., 0.5, 0.5, 0.5, 1., 1., 1.,
0., 0., 0., 0.5, 0.5, 0.5, 1., 1., 1.
};

float zc[] = {
1., 0., 0., 1.,
1., 0., 0., 1.,
1., 0.5, 0.,1., 0.5, 0.,1., 0.5, 0.,
1., 0.5, 0.,1., 0.5, 0.,1., 0.5, 0.
};

int connectivity[] = {
    VISIT_CELL_HEX,
        0,1,5,4,3,2,6,7,
    VISIT_CELL_POLYHEDRON,
        9, /* # faces*/
        4, /*ids*/ 4,5,6,7,
        5, /*ids*/ 8,4,7,14,11,
        5, /*ids*/ 14,7,6,16,15,
        5, /*ids*/ 16,6,5,10,13,
        5, /*ids*/ 10,5,4,8,9,
        4, /*ids*/ 9,8,11,12,
        4, /*ids*/ 10,9,12,13,
        4, /*ids*/ 12,11,14,15,
        4, /*ids*/ 13,12,15,16,
    VISIT_CELL_HEX,
        8,9,18,17,11,12,21,20,
    VISIT_CELL_HEX,
        9,10,19,18,12,13,22,21,
    VISIT_CELL_HEX,
        11,12,21,20,14,15,24,23,
    VISIT_CELL_HEX,
        12,13,22,21,15,16,25,24,
};

int npolyzones = 6;

/*************************** Polyhedral Mesh variables ***********************/

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

#define IS_COMMAND(C) (strstr(cmd, C) != NULL) 
    if(IS_COMMAND("halt"))
        sim->runMode = SIM_STOPPED;
    else if(IS_COMMAND("step"))
        simulate_one_timestep(sim);
    else if(IS_COMMAND("run"))
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
    void *cbdata[2] = {NULL, NULL};

    /* Let's initialize some mesh metadata here to show that we can pass
     * user-defined data pointers to the various data access functions. In this
     * case, we'll pass the address of this "mmd" structure and use it in the
     * data access function to initialize metadata that we pass back to VisIt.
     */
    VisIt_MeshMetaData mmd;
    memset(&mmd, 0, sizeof(VisIt_MeshMetaData));
    mmd.name = "polyhedral";
    mmd.meshType = VISIT_MESHTYPE_UNSTRUCTURED;
    mmd.topologicalDimension = 3;
    mmd.spatialDimension = 3;
    mmd.numBlocks = 1;
    mmd.blockTitle = "Regions";
    mmd.blockPieceName = "region";
    mmd.blockNames = (char**)malloc(sizeof(char*));
    mmd.blockNames[0] = "zones";
    mmd.numGroups = 0;
    mmd.units = "cm";
    mmd.xLabel = "Width";
    mmd.yLabel = "Height";
    mmd.zLabel = "Depth";

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

                cbdata[0] = (void*)&mmd;
                cbdata[1] = (void*)sim;
                VisItSetGetMetaData(SimGetMetaData, (void*)cbdata);
                VisItSetGetMesh(SimGetMesh, NULL);
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
    VisItInitializeSocketAndDumpSimFile("polyhedral",
        "Demonstrates creating a polyhedral mesh",
        "/path/to/where/sim/was/started",
        NULL, NULL, NULL);

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    return 0;
}

/* DATA ACCESS FUNCTIONS */
static void
copy_VisIt_MeshMetaData(VisIt_MeshMetaData *dest, VisIt_MeshMetaData *src)
{
#define STRDUP(s) ((s == NULL) ? NULL : strdup(s))
    int i;

    memcpy(dest, src, sizeof(VisIt_MeshMetaData));
    dest->name           = STRDUP(src->name);
    dest->blockTitle     = STRDUP(src->blockTitle);
    dest->blockPieceName = STRDUP(src->blockPieceName);
    if(src->blockNames != NULL)
    {
        dest->blockNames = (char**)malloc(src->numBlocks*sizeof(char*));
        for(i = 0; i < src->numBlocks; ++i)
            dest->blockNames[i] = STRDUP(src->blockNames[i]);
    }
    dest->units          = STRDUP(src->units);
    dest->xLabel         = STRDUP(src->xLabel);
    dest->yLabel         = STRDUP(src->yLabel);
    dest->zLabel         = STRDUP(src->zLabel);
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

int
SimGetMetaData(VisIt_SimulationMetaData *md, void *cbdata)
{
    size_t sz;
    VisIt_MeshMetaData *mmd = NULL;
    simulation_data *sim = NULL;
    mmd = (VisIt_MeshMetaData *)(((void**)cbdata)[0]);
    sim = (simulation_data *)(((void**)cbdata)[1]);

    /* Set the simulation state. */
    md->currentMode = (sim->runMode == SIM_RUNNING) ?
        VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
    md->currentCycle = sim->cycle;
    md->currentTime = sim->time;

    /* Allocate enough room for 1 mesh in the metadata. */
    md->numMeshes = 1;
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);

    /* Set the first mesh's properties based on data that we passed in
     * as callback data.
     */
    copy_VisIt_MeshMetaData(&md->meshes[0], mmd);

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

int
SimGetMesh(int domain, const char *name, VisIt_MeshData *mesh, void *cbdata)
{
    int ret = VISIT_ERROR;

    if(strcmp(name, "polyhedral") == 0)
    {
        /* Make VisIt_MeshData contain a VisIt_CSGMesh. */
        size_t sz = sizeof(VisIt_UnstructuredMesh);
        mesh->umesh = (VisIt_UnstructuredMesh *)malloc(sz);
        memset(mesh->umesh, 0, sz);

        /* Tell VisIt which mesh object to use. */
        mesh->meshType = VISIT_MESHTYPE_UNSTRUCTURED;

        /* Fill in the polyhedral mesh's data values. */
        mesh->umesh->ndims = 3;
        /* Set the number of nodes and zones in the mesh. */
        mesh->umesh->nnodes = sizeof(xc) / sizeof(float);
        mesh->umesh->nzones = npolyzones;

        /* Set the indices for the first and last real zones. */
        mesh->umesh->firstRealZone = 0;
        mesh->umesh->lastRealZone = npolyzones-1;

        /* Let VisIt use the simulation's copy of the mesh coordinates. */
        mesh->umesh->xcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, xc);
        mesh->umesh->ycoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, yc);
        mesh->umesh->zcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_SIM, zc);

        /* Let VisIt use the simulation's copy of the connectivity. */
        mesh->umesh->connectivity = VisIt_CreateDataArrayFromInt(
           VISIT_OWNER_SIM, connectivity);
        mesh->umesh->connectivityLen = sizeof(connectivity)/sizeof(int);

        ret = VISIT_OKAY;
    }

    return ret;
}

