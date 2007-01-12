/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

/* SIMPLE SIMULATION SKELETON */
#include <VisItControlInterface_V1.h>
#include <stdio.h>
#include <string.h>

void simulate_one_timestep(void);
void read_input_deck(void) { }
int  simulation_done(void)   { return 0; }

/* Is the simulation in run mode (not waiting for VisIt input) */
static int    runFlag = 1;
static int    simcycle = 0;
static double simtime = 0.;

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
                fprintf(stderr, "VisIt connected\n");
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
    VisItInitializeSocketAndDumpSimFile("sim8",
        "Demonstrates creating scalar metadata",
        "/path/to/where/sim/was/started",
        NULL, NULL);

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
#include <VisItDataInterface_V1.h>

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

    /* Add some scalar variables. */
    md->numScalars = 2;
        sz = sizeof(VisIt_ScalarMetaData) * md->numScalars;
    md->scalars = (VisIt_ScalarMetaData *)malloc(sz);
    memset(md->scalars, 0, sz);

    /* Add a zonal variable on mesh2d. */
    md->scalars[0].name = strdup("zonal");
    md->scalars[0].meshName = strdup("mesh2d");
    md->scalars[0].centering = VISIT_VARCENTERING_ZONE;

    /* Add a nodal variable on mesh3d. */
    md->scalars[1].name = strdup("nodal");
    md->scalars[1].meshName = strdup("mesh3d");
    md->scalars[1].centering = VISIT_VARCENTERING_NODE;

    return md;
}

VisIt_SimulationCallback visitCallbacks =
{
    &VisItGetMetaData,
    NULL, /* GetMesh */
    NULL, /* GetMaterial */
    NULL, /* GetSpecies */
    NULL, /* GetScalar */
    NULL, /* GetCurve */
    NULL, /* GetMixedScalar */
    NULL /* GetDomainList */
};
