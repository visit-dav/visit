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
 ******************************************************************************/

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
    sim->mmd.name = "csg";
    sim->mmd.meshType = VISIT_MESHTYPE_CSG;
    sim->mmd.topologicalDimension = 3;
    sim->mmd.spatialDimension = 3;
    sim->mmd.numBlocks = 1;
    sim->mmd.blockTitle = "Regions";
    sim->mmd.blockPieceName = "region";
    sim->mmd.blockNames = (char**)malloc(sizeof(char*));
    sim->mmd.blockNames[0] = "Clipped Hollow Sphere";
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

void simulate_one_timestep(simulation_data *sim);
void read_input_deck(void) { }

/*************************** CSG Mesh variables *****************************/
int csg_bound_typeflags[] = {
    VISIT_CSG_SPHERE_PR,
    VISIT_CSG_SPHERE_PR,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN
};

float csg_bound_coeffs[] = {
    0.,  0., 0., 8.,         /* sphere 1*/
    0.,  0., 0., 10.,        /* sphere 2*/
    0.,  2., 0., 0., 1., 0., /* plane 1 point, normal*/
    0., -2., 0., 0., 1., 0., /* plane 2 point, normal*/
    2.,  0., 0., 1., 0., 0., /* plane 3 point, normal*/
   -2.,  0., 0., 1., 0., 0.  /* plane 4 point, normal*/
};
int csg_lcoeffs = sizeof(csg_bound_coeffs) / sizeof(csg_bound_coeffs[0]);
int csg_nbounds = sizeof(csg_bound_typeflags) / sizeof(csg_bound_typeflags[0]);
double csg_extents[] = {-11., -11., -11., 11., 11., 11.};

#define PLANE1_COEFF 8
#define PLANE2_COEFF (PLANE1_COEFF+6)
#define PLANE3_COEFF (PLANE2_COEFF+6)
#define PLANE4_COEFF (PLANE3_COEFF+6)

int csg_reg_typeflags[] =
{
    VISIT_CSG_OUTER,          /* 0: outside of inner sphere */
    VISIT_CSG_INNER,          /* 1: inside of outer sphere  */
    VISIT_CSG_INTERSECT,      /* 2: intersection between them  */
    VISIT_CSG_INNER,          /* 3: plane 1 */
    VISIT_CSG_OUTER,          /* 4: plane 2 */
    VISIT_CSG_INTERSECT,      /* 5: intersection between plane 1,2 half spaces*/
    VISIT_CSG_INNER,          /* 6: plane 3 */
    VISIT_CSG_OUTER,          /* 7: plane 4 */
    VISIT_CSG_INTERSECT,      /* 8: intersection between plane 3,4 half spaces*/
    VISIT_CSG_UNION,          /* 9  add the 2 blocks together */
    VISIT_CSG_INTERSECT       /* 10: intersect the spherical shell with the blocks. */
};
/*                 0   1   2   3   4   5   6   7   8   9  10*/
int csg_leftids[] =  { 0,  1,  0,  2,  3,  3,  4,  5,  6,  5,  2};
int csg_rightids[] = {-1, -1,  1, -1, -1,  4, -1, -1,  7,  8,  9};
int csg_zonelist[] = {10};
int csg_nregs = sizeof(csg_reg_typeflags) / sizeof(csg_reg_typeflags[0]);
int csg_nzones = sizeof(csg_zonelist) / sizeof(csg_zonelist[0]);
/*************************** CSG Mesh variables *****************************/

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
    else if(strcmp(cmd, "saveimage") == 0)
    {
        char filename[100];
        sprintf(filename, "csg%04d.png", sim->cycle);
        if(VisItSaveWindow(filename, 600, 600, VISIT_IMAGEFORMAT_PNG) == VISIT_OKAY)
            printf("Saved %s\n", filename);
        else
            printf("The image could not be saved to %s\n", filename);
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
    else if(strcmp(cmd, "saveimage") == 0)
    {
        char filename[100];
        sprintf(filename, "csg%04d.png", sim->cycle);
        if(VisItSaveWindow(filename, 600, 600, VISIT_IMAGEFORMAT_PNG) == VISIT_OKAY)
            printf("Saved %s\n", filename);
        else
            printf("The image could not be saved to %s\n", filename);
    }
}

/* SIMULATE ONE TIME STEP */
#include <unistd.h>
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 30.);
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    /* Update the plane equations in the CSG mesh information to make
     * the planes rotate about the Y-Axis.
     */
    {
    double angle, cosA, sinA, cosB, sinB;
    angle = sim->time;
    cosA = cos(angle);
    sinA = sin(angle);
    cosB = cos(angle + M_PI);
    sinB = sin(angle + M_PI);
    csg_bound_coeffs[PLANE3_COEFF] = 2.* cosA;
    csg_bound_coeffs[PLANE3_COEFF+1] = 0.;
    csg_bound_coeffs[PLANE3_COEFF+2] = 2.* sinA;
    csg_bound_coeffs[PLANE3_COEFF+3] = cosA;
    csg_bound_coeffs[PLANE3_COEFF+4] = 0.;
    csg_bound_coeffs[PLANE3_COEFF+5] = sinA;

    csg_bound_coeffs[PLANE4_COEFF] = 2.* cosB;
    csg_bound_coeffs[PLANE4_COEFF+1] = 0.;
    csg_bound_coeffs[PLANE4_COEFF+2] = 2.* sinB;;
    csg_bound_coeffs[PLANE4_COEFF+3] = cosA;
    csg_bound_coeffs[PLANE4_COEFF+4] = 0.;
    csg_bound_coeffs[PLANE4_COEFF+5] = sinA;
    }

    sleep(1);
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
    VisItOpenTraceFile("csg_trace.txt");

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("csg",
        "Demonstrates creating a csg mesh",
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
    simulation_data *sim = (simulation_data *)cbdata;
    size_t sz;

    /* Set the simulation state. */
    md->currentMode = (sim->runMode == SIM_RUNNING) ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
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
    md->numGenericCommands = 5;
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

    md->genericCommands[4].name = strdup("saveimage");
    md->genericCommands[4].argType = VISIT_CMDARG_NONE;
    md->genericCommands[4].enabled = 1;

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

    if(strcmp(name, "csg") == 0)
    {
        if(VisIt_CSGMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle typeflags, leftids, rightids, zonelist;
            visit_handle boundaryTypes, boundaryCoeffs;

            /* Fill in the CSG mesh's data values. */
            VisIt_VariableData_alloc(&boundaryTypes);
            VisIt_VariableData_setDataI(boundaryTypes, VISIT_OWNER_SIM, 
                1, sizeof(csg_bound_typeflags)/sizeof(int), csg_bound_typeflags);
            VisIt_CSGMesh_setBoundaryTypes(h, boundaryTypes);

            VisIt_VariableData_alloc(&boundaryCoeffs);
            VisIt_VariableData_setDataF(boundaryCoeffs, VISIT_OWNER_SIM, 
                1, csg_lcoeffs, csg_bound_coeffs);
            VisIt_CSGMesh_setBoundaryCoeffs(h, boundaryCoeffs);

            /* Set the extents */
            VisIt_CSGMesh_setExtents(h, csg_extents, csg_extents+3);

            /* Set the regions */
            VisIt_VariableData_alloc(&typeflags);
            VisIt_VariableData_setDataI(typeflags, VISIT_OWNER_SIM, 
                1, csg_nregs, csg_reg_typeflags);

            VisIt_VariableData_alloc(&leftids);
            VisIt_VariableData_setDataI(leftids, VISIT_OWNER_SIM, 
                1, csg_nregs, csg_leftids);

            VisIt_VariableData_alloc(&rightids);
            VisIt_VariableData_setDataI(rightids, VISIT_OWNER_SIM, 
                1, csg_nregs, csg_rightids);

            VisIt_CSGMesh_setRegions(h, typeflags, leftids, rightids);

            /* Set the zonelist */
            VisIt_VariableData_alloc(&zonelist);
            VisIt_VariableData_setDataI(zonelist, VISIT_OWNER_SIM, 
                1, 1, csg_zonelist);
            VisIt_CSGMesh_setZonelist(h, zonelist);
        }
    }

    return h;
}
