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
visit_handle SimGetMesh(int, const char *, void *);

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

    VisIt_MeshMetaData mmd;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;

    /* Let's initialize some mesh metadata here to show that we can pass
     * user-defined data pointers to the various data access functions. In this
     * case, we'll pass the address of this "mmd" structure and use it in the
     * data access function to initialize metadata that we pass back to VisIt.
     */
    memset(&sim->mmd, 0, sizeof(VisIt_MeshMetaData));
    sim->mmd.name = "polyhedral";
    sim->mmd.meshType = VISIT_MESHTYPE_UNSTRUCTURED;
    sim->mmd.topologicalDimension = 3;
    sim->mmd.spatialDimension = 3;
    sim->mmd.numBlocks = 1;
    sim->mmd.blockTitle = "Regions";
    sim->mmd.blockPieceName = "region";
    sim->mmd.blockNames = (char**)malloc(sizeof(char*));
    sim->mmd.blockNames[0] = "zones";
    sim->mmd.numGroups = 0;
    sim->mmd.units = "cm";
    sim->mmd.xLabel = "Width";
    sim->mmd.yLabel = "Height";
    sim->mmd.zLabel = "Depth";
}

void
simulation_data_dtor(simulation_data *sim)
{
    free(sim->mmd.blockNames);
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

    /* Cleanup */
    simulation_data_dtor(&sim);

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
    simulation_data *sim = (simulation_data *)cbdata;

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
    copy_VisIt_MeshMetaData(&md->meshes[0], &sim->mmd);

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
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "polyhedral") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) == VISIT_OKAY)
        {
            visit_handle x,y,z,conn;
            int nnodes = sizeof(xc) / sizeof(float);

            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_alloc(&y);
            VisIt_VariableData_alloc(&z);
            VisIt_VariableData_setDataF(x, VISIT_OWNER_SIM, 1, nnodes, xc);
            VisIt_VariableData_setDataF(y, VISIT_OWNER_SIM, 1, nnodes, yc);
            VisIt_VariableData_setDataF(z, VISIT_OWNER_SIM, 1, nnodes, zc);
            VisIt_UnstructuredMesh_setCoordsXYZ(h, x, y, z);

            VisIt_VariableData_alloc(&conn);
            VisIt_VariableData_setDataI(conn, VISIT_OWNER_SIM, 1, 
                sizeof(connectivity)/sizeof(int), connectivity);
            VisIt_UnstructuredMesh_setConnectivity(h, npolyzones, conn);

            VisIt_UnstructuredMesh_setRealIndices(h, 0, npolyzones-1);
        }
    }

    return h;
}

