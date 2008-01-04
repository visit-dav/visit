/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
#include <VisItControlInterface_V1.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

#define VISIT_OKAY 0

/* Is the simulation in run mode (not waiting for VisIt input) */
static int    runFlag = 0;
static int    simcycle = 0;
static double simtime = 0.;
static int    simUpdatePlots = 0;
static int    simDone = 0;

static int par_rank = 0;
static int par_size = 1;

void simulate_one_timestep(void);
void read_input_deck(void) { }
int  simulation_done(void)   { return simDone; }

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/* Callback function for control commands, which are the buttons in the 
 * GUI's Simulation window. This type of command is handled automatically
 * provided that you have registered a command callback such as this.
 */
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

/* CHANGE 1 */
#ifdef PARALLEL
static int visit_broadcast_int_callback(int *value, int sender)
{
    return MPI_Bcast(value, 1, MPI_INT, sender, MPI_COMM_WORLD);
}

static int visit_broadcast_string_callback(char *str, int len, int sender)
{
    return MPI_Bcast(str, len, MPI_CHAR, sender, MPI_COMM_WORLD);
}
#endif


/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback()
{
   int command = VISIT_COMMAND_PROCESS;
   BroadcastSlaveCommand(&command);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(void)
{
    int command;
    if (par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success)
        {
            command = VISIT_COMMAND_SUCCESS;
            BroadcastSlaveCommand(&command);
            return 1;
        }
        else
        {
            command = VISIT_COMMAND_FAILURE;
            BroadcastSlaveCommand(&command);
            return 0;
        }
    }
    else
    {
        /* Note: only through the SlaveProcessCallback callback
         * above can the rank 0 process send a VISIT_COMMAND_PROCESS
         * instruction to the non-rank 0 processes. */
        while (1)
        {
            BroadcastSlaveCommand(&command);
            switch (command)
            {
            case VISIT_COMMAND_PROCESS:
                VisItProcessEngineCommand();
                break;
            case VISIT_COMMAND_SUCCESS:
                return 1;
            case VISIT_COMMAND_FAILURE:
                return 0;
            }
        }
    }
}

/* Called to handle case 3 from VisItDetectInput where we have console
 * input that needs to be processed in order to accomplish an action.
 */
void
ProcessConsoleCommand()
{
    /* Read A Command */
    char buff[1000];

    if (par_rank == 0)
    {
        int iseof = (fgets(buff, 1000, stdin) == NULL);
        if (iseof)
        {
            sprintf(buff, "quit");
            printf("quit\n");
        }

        if (strlen(buff)>0 && buff[strlen(buff)-1] == '\n')
            buff[strlen(buff)-1] = '\0';
    }

#ifdef PARALLEL
    /* Broadcast the command to all processors. */
    MPI_Bcast(buff, 1000, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

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
    else if(strcmp(buff, "addplot") == 0)
    {
        VisItExecuteCommand("AddPlot(\"Pseudocolor\", \"zonal\")\n");
        VisItExecuteCommand("DrawPlots()\n");
    }
}

/******************************************************************************
 *
 * Function: mainloop
 *
 * Purpose: Handles the program's main event loop and dispatches events to 
 *          other functions for processing.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:26:08 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop(void)
{
    int blocking, visitstate, err = 0;

    if (par_rank == 0)
    {
        fprintf(stderr, "command> ");
        fflush(stderr);
    }

    do
    {
        blocking = runFlag ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        if(par_rank == 0)
        {
            visitstate = VisItDetectInput(blocking, fileno(stdin));
        }
#ifdef PARALLEL
        /* Broadcast the return value of VisItDetectInput to all procs. */
        MPI_Bcast(&visitstate, 1, MPI_INT, 0, MPI_COMM_WORLD);
#endif
        /* Do different things depending on the output from VisItDetectInput. */
        switch(visitstate)
        {
        case 0:
            /* There was no input from VisIt, return control to sim. */
            simulate_one_timestep();
            break;
        case 1:
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection())
            {
                simUpdatePlots = 1;
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback);
                VisItSetSlaveProcessCallback(SlaveProcessCallback);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
            break;
        case 2:
            /* VisIt wants to tell the engine something. */
            if(!ProcessVisItCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                runFlag = 1;
                simUpdatePlots = 0;
            }
            break;
        case 3:
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand();
            if (par_rank == 0)
            {
                fprintf(stderr, "command> ");
                fflush(stderr);
            }
            break;
        default:
            fprintf(stderr, "Can't recover from error %d!\n", visitstate);
            err = 1;
            break;
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
 *    Shelly Prevost,Thu Jul 26 16:34:40 PDT 2007
 *    Added a absolute filename argument to VisItInitializeSocketAndDumpSimFile.
 *
 *****************************************************************************/

int main(int argc, char **argv)
{
    /* Initialize environment variables. */
    VisItSetupEnvironment();

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &par_size);

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell VSIL whether the simulation is parallel. */
    VisItSetParallel(par_size > 1);
    VisItSetParallelRank(par_rank);
#endif

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(par_rank == 0)
    {
        /* Write out .sim file that VisIt uses to connect. */
        VisItInitializeSocketAndDumpSimFile("updateplots",
            "Demonstrates VisItUpdatePlots function",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck();

    /* Call the main loop. */
    mainloop();

#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(void)
{
    ++simcycle;
    simtime += (M_PI / 10.);

    if(par_rank == 0)
    {
        printf("Simulating time step: cycle=%d, time=%lg\n", simcycle, simtime);

        if(simUpdatePlots == 1)
        {
            /* Tell VisIt that the timestep changed. */
            VisItTimeStepChanged();

            /* Tell VisIt that we should update the plots. */
            VisItUpdatePlots();
        }
    }
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

    /* Allocate enough room for 1 mesh in the metadata. */
    md->numMeshes = 1;
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);

    /* Set the first mesh's properties.*/
    md->meshes[0].name = strdup("mesh2d");
    md->meshes[0].meshType = VISIT_MESHTYPE_RECTILINEAR;
    md->meshes[0].topologicalDimension = 2;
    md->meshes[0].spatialDimension = 2;
    md->meshes[0].numBlocks = par_size;
    md->meshes[0].blockTitle = strdup("Domains");
    md->meshes[0].blockPieceName = strdup("domain");
    md->meshes[0].numGroups = 0;
    md->meshes[0].units = strdup("cm");
    md->meshes[0].xLabel = strdup("Width");
    md->meshes[0].yLabel = strdup("Height");
    md->meshes[0].zLabel = strdup("Depth");

    /* Add a scalar variable. */
    md->numScalars = 1;
    sz = sizeof(VisIt_ScalarMetaData) * md->numScalars;
    md->scalars = (VisIt_ScalarMetaData *)malloc(sz);
    memset(md->scalars, 0, sz);

    /* Add a zonal variable on mesh2d. */
    md->scalars[0].name = strdup("zonal");
    md->scalars[0].meshName = strdup("mesh2d");
    md->scalars[0].centering = VISIT_VARCENTERING_ZONE;

    /* Add a curve variable. */
    md->numCurves = 1;
    sz = sizeof(VisIt_CurveMetaData) * md->numCurves;
    md->curves = (VisIt_CurveMetaData *)malloc(sz);
    memset(md->curves, 0, sz);

    md->curves[0].name = strdup("sine");
    md->curves[0].xUnits = strdup("radians");
    md->curves[0].xLabel = strdup("angle");
    md->curves[0].yLabel = strdup("amplitude");

    /* Add an expression. */
    md->numExpressions = 1;
    sz = sizeof(VisIt_ExpressionMetaData) * md->numExpressions;
    md->expressions = (VisIt_ExpressionMetaData *)malloc(sz);
    memset(md->expressions, 0, sz);

    md->expressions[0].name = strdup("zvec");
    md->expressions[0].definition = strdup("{zonal, zonal}");
    md->expressions[0].vartype = VISIT_VARTYPE_VECTOR;

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

    md->genericCommands[3].name = strdup("addplot");
    md->genericCommands[3].argType = VISIT_CMDARG_NONE;
    md->genericCommands[3].enabled = 1;

    return md;
}

/* Rectilinear mesh */
#define RNX 50
#define RNY 50
int   rmesh_dims[] = {RNX, RNY, 1};
int   rmesh_ndims = 2;


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

VisIt_MeshData *VisItGetMesh(int domain, const char *name)
{
    int i;
    VisIt_MeshData *mesh = NULL;
    size_t sz = sizeof(VisIt_MeshData);

    if(strcmp(name, "mesh2d") == 0)
    {
        float *rmesh_x, *rmesh_y;

        /* Allocate VisIt_MeshData. */
        mesh = (VisIt_MeshData *)malloc(sz);
        memset(mesh, 0, sz);
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

        rmesh_x = (float *)malloc(sizeof(float) * RNX * RNY);
        for(i = 0; i < RNX; ++i)
            rmesh_x[i] = ((float)i / (float)(RNX-1)) * 5. - 2.5 + 5 * domain;
        rmesh_y = (float *)malloc(sizeof(float) * RNX * RNY);
        for(i = 0; i < RNY; ++i)
            rmesh_y[i] = ((float)i / (float)(RNY-1)) * 5. - 2.5;

        /* Let VisIt use the simulation's copy of the mesh coordinates. */
        mesh->rmesh->xcoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_VISIT, rmesh_x);
        mesh->rmesh->ycoords = VisIt_CreateDataArrayFromFloat(
           VISIT_OWNER_VISIT, rmesh_y);
    }

    return mesh;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_ScalarData *VisItGetScalar(int domain, const char *name)
{
    size_t sz = sizeof(VisIt_ScalarData);
    VisIt_ScalarData *scalar = (VisIt_ScalarData*)malloc(sz);
    memset(scalar, 0, sz);

    if(strcmp(name, "zonal") == 0)
    {
        float angle, xpos, ypos, cellX, cellY, dX, dY, tx, ty, *zoneptr;
        float sx, ex, sy, ey, *rmesh_zonal;
        int i, j;

        sx = -2.5  + domain * 5.;
        ex = sx + 5.;
        sy = -2.5;
        ey = sy + 5.;

        /* Calculate a zonal variable that moves around. */
        rmesh_zonal = (float*)malloc(sizeof(float) * (RNX-1) * (RNY-1));
        zoneptr = rmesh_zonal;
        angle = simtime;
        xpos = 2.5 * cos(angle);
        ypos = 2.5 * sin(angle);
        for(j = 0; j < rmesh_dims[1]-1; ++j)
        {
            ty = (float)j / (float)(rmesh_dims[1]-1-1);
            cellY = (1.-ty)*sy + ey*ty;
            dY = cellY - ypos;
            for(i = 0; i < rmesh_dims[0]-1; ++i)
            {
                tx = (float)i / (float)(rmesh_dims[0]-1-1);
                cellX = (1.-tx)*sx + ex*tx;
                dX = cellX - xpos;
                *zoneptr++ = sqrt(dX * dX + dY * dY);
            }
        }

        scalar->len = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
        scalar->data = VisIt_CreateDataArrayFromFloat(
            VISIT_OWNER_VISIT, rmesh_zonal);
    }
    else 
    {
        free(scalar);
        scalar = NULL;
    }

    return scalar;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a curve.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *   Brad Whitlock, Thu Jan 25 15:46:19 PST 2007
 *   Added simtime into the angle so the plot will animate.
 *
 *****************************************************************************/

VisIt_CurveData *VisItGetCurve(const char *name)
{
    size_t sz = sizeof(VisIt_CurveData);
    VisIt_CurveData *curve = (VisIt_CurveData*)malloc(sz);
    memset(curve, 0, sz);

    if(strcmp(name, "sine") == 0)
    {
        int i;
        float *x = NULL, *y = NULL;
        x = (float*)malloc(200 * sizeof(float));
        y = (float*)malloc(200 * sizeof(float));
        
        for(i = 0; i < 200; ++i)
        {
            float angle = simtime + ((float)i / (float)(200-1)) * 4. * M_PI;
            x[i] = angle;
            y[i] = sin(x[i]);
        }

        /* Give the arrays to VisIt. VisIt will free them. */
        curve->len = 200;
        curve->x = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_VISIT, x);
        curve->y = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_VISIT, y);
    }
    else 
    {
        free(curve);
        curve = NULL;
    }

    return curve;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_DomainList *VisItGetDomainList(void)
{
    int i, *iptr = NULL;

    iptr = (int *)malloc(sizeof(int));
    *iptr = par_rank;

    VisIt_DomainList *dl = (VisIt_DomainList *)malloc(sizeof(VisIt_DomainList));
    memset(dl, 0, sizeof(VisIt_DomainList));
    dl->nTotalDomains = par_size;
    dl->nMyDomains = 1;
    dl->myDomains = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT, iptr);

    return dl;
}

VisIt_SimulationCallback visitCallbacks =
{
    &VisItGetMetaData,
    &VisItGetMesh,
    NULL, /* GetMaterial */
    NULL, /* GetSpecies */
    &VisItGetScalar, /* GetScalar */
    &VisItGetCurve, /* GetCurve */
    NULL, /* GetMixedScalar */
    NULL /* GetDomainList */
};
