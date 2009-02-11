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

/* Is the simulation in run mode (not waiting for VisIt input) */
static int    runFlag = 1;
static int    simcycle = 0;
static double simtime = 0.;
static int    simUpdatePlots = 0;
static int    simDone = 0;

void simulate_one_timestep(void);
void read_input_deck(void) { }
int  simulation_done(void)   { return simDone; }

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
 *   cmd         : The command string that we want the sim to execute.
 *   int_data    : Integer argument for the command.
 *   float_data  : Float argument for the command.
 *   string_data : String argument for the command.
 *
 * Modifications:
 *
 *****************************************************************************/

void ControlCommandCallback(const char *cmd,
    int int_data, float float_data,
    const char *string_data)
{
    if(strcmp(cmd, "halt") == 0)
        runFlag = 0;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep();
    else if(strcmp(cmd, "run") == 0)
        runFlag = 1;
}

/* Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 */
void
ProcessConsoleCommand()
{
    /* Read A Command */
    char buff[1000];

    int iseof = (fgets(buff, 1000, stdin) == NULL);
    if (iseof)
    {
        sprintf(buff, "quit");
        printf("quit\n");
    }

    if (strlen(buff)>0 && buff[strlen(buff)-1] == '\n')
        buff[strlen(buff)-1] = '\0';

    if(strcmp(buff, "run") == 0)
        runFlag = 1;
    else if(strcmp(buff, "halt") == 0)
        runFlag = 0;
    else if(strcmp(buff, "step") == 0)
        simulate_one_timestep();
    else if(strcmp(buff, "quit") == 0)
    {
        simDone = 1;
        simUpdatePlots = 1;
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

void mainloop(void)
{
    int blocking, visitstate, err = 0;
    
    fprintf(stderr, "command> ");
    fflush(stderr);

    do
    {
        blocking = runFlag ? 0 : 1;
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
            simulate_one_timestep();
        }
        else if(visitstate == 1)
        {
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection())
            {
                simUpdatePlots = 1;
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
        }
        else if(visitstate == 2)
        {
            /* VisIt wants to tell the engine something. */
            if(!VisItProcessEngineCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                runFlag = 1;
            }
        }
        else if(visitstate == 3)
        {
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand();
            fprintf(stderr, "command> ");
            fflush(stderr);
        }
    } while(!simulation_done() && err == 0);
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

/* SIMULATE ONE TIME STEP */
#include <unistd.h>
void simulate_one_timestep(void)
{
    ++simcycle;
    simtime += (M_PI / 30.);
    printf("Simulating time step: cycle=%d, time=%lg\n", simcycle, simtime);

    /* Update the plane equations in the CSG mesh information to make
     * the planes rotate about the Y-Axis.
     */
    {
    double angle, cosA, sinA, cosB, sinB;
    angle = simtime;
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

    if(simUpdatePlots == 1)
    {
        /* Tell VisIt that the timestep changed. */
        VisItTimeStepChanged();

        /* Tell VisIt that we should update the plots. */
        VisItUpdatePlots();
    }
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

VisIt_SimulationMetaData *VisItGetMetaData(void)
{
    /* Create a metadata object with no variables. */
    size_t sz = sizeof(VisIt_SimulationMetaData);
    VisIt_SimulationMetaData *md = 
        (VisIt_SimulationMetaData *)malloc(sz);
    memset(md, 0, sz);

    /* Set the simulation state. */
    md->currentMode = runFlag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
    md->currentCycle = simcycle;
    md->currentTime = simtime;

    /* Allocate enough room for 1 mesh in the metadata. */
    md->numMeshes = 1;
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);

    /* Set the first mesh's properties.*/
    md->meshes[0].name = strdup("csg");
    md->meshes[0].meshType = VISIT_MESHTYPE_CSG;
    md->meshes[0].topologicalDimension = 3;
    md->meshes[0].spatialDimension = 3;
    md->meshes[0].numBlocks = 1;
    md->meshes[0].blockTitle = strdup("Domains");
    md->meshes[0].blockPieceName = strdup("domain");
    md->meshes[0].numGroups = 0;
    md->meshes[0].units = strdup("cm");
    md->meshes[0].xLabel = strdup("Width");
    md->meshes[0].yLabel = strdup("Height");
    md->meshes[0].zLabel = strdup("Depth");

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

VisIt_MeshData *VisItGetMesh(int domain, const char *name)
{
    VisIt_MeshData *mesh = NULL;
    size_t sz = sizeof(VisIt_MeshData);

    if(strcmp(name, "csg") == 0)
    {
        /* Allocate VisIt_MeshData. */
        mesh = (VisIt_MeshData *)malloc(sz);
        memset(mesh, 0, sz);
        /* Make VisIt_MeshData contain a VisIt_CSGMesh. */
        sz = sizeof(VisIt_CSGMesh);
        mesh->csgmesh = (VisIt_CSGMesh *)malloc(sz);
        memset(mesh->csgmesh, 0, sz);

        /* Tell VisIt which mesh object to use. */
        mesh->meshType = VISIT_MESHTYPE_CSG;

        /* Fill in the CSG mesh's data values. */
        mesh->csgmesh->nbounds = csg_nbounds;
        mesh->csgmesh->typeflags = VisIt_CreateDataArrayFromInt(
            VISIT_OWNER_SIM, csg_bound_typeflags);

        mesh->csgmesh->coeffs = VisIt_CreateDataArrayFromFloat(
            VISIT_OWNER_SIM, csg_bound_coeffs);
        mesh->csgmesh->lcoeffs = csg_lcoeffs;

        mesh->csgmesh->min_extents[0] = csg_extents[0];
        mesh->csgmesh->min_extents[1] = csg_extents[1];
        mesh->csgmesh->min_extents[2] = csg_extents[2];
        mesh->csgmesh->max_extents[0] = csg_extents[3];
        mesh->csgmesh->max_extents[1] = csg_extents[4];
        mesh->csgmesh->max_extents[2] = csg_extents[5];

        mesh->csgmesh->zones.nregs = csg_nregs;
        mesh->csgmesh->zones.typeflags = VisIt_CreateDataArrayFromInt(
            VISIT_OWNER_SIM, csg_reg_typeflags);
        mesh->csgmesh->zones.leftids = VisIt_CreateDataArrayFromInt(
            VISIT_OWNER_SIM, csg_leftids);
        mesh->csgmesh->zones.rightids = VisIt_CreateDataArrayFromInt(
            VISIT_OWNER_SIM, csg_rightids);
        mesh->csgmesh->zones.nzones = 1;
        mesh->csgmesh->zones.zonelist = VisIt_CreateDataArrayFromInt(
            VISIT_OWNER_SIM, csg_zonelist);
    }

    return mesh;
}

VisIt_SimulationCallback visitCallbacks =
{
    /* reader functions */
    &VisItGetMetaData,
    &VisItGetMesh,
    NULL, /* GetMaterial */
    NULL, /* GetSpecies */
    NULL, /* GetScalar */
    NULL, /* GetCurve */
    NULL, /* GetMixedScalar */
    NULL, /* GetDomainList */

    /* writer functions */
    NULL, /* WriteBegin */
    NULL, /* WriteEnd */
    NULL, /* WriteCurvilinearMesh */
    NULL, /* WriteRectilinearMesh */
    NULL, /* WritePointMesh */
    NULL, /* WriteUnstructuredMesh */
    NULL  /* WriteDataArray */
};
