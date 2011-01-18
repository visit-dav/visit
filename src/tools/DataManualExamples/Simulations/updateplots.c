/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "SimulationExample.h"

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

void read_input_deck(void) { }
/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetCurve(const char *name, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

#define SIM_STOPPED       0
#define SIM_RUNNING       1

typedef struct
{
    int     par_rank;
    int     par_size;
    int     cycle;
    double  time;
    int     runMode;
    int     done;
    int     savingFiles;
    int     saveCounter;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->savingFiles = 0;
    sim->saveCounter = 0;
}

void
simulation_data_dtor(simulation_data *sim)
{
}

const char *cmd_names[] = {"halt", "step", "run", "addplot"};

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 *
 * Purpose: This function simulates one time step
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jan 12 13:37:17 PST 2007
 *
 * Modifications:
 *   Brad Whitlock, Tue Jun 17 16:09:51 PDT 2008
 *   Call VisItTimeStepChanged on all processors to prevent a "merge"
 *   exception in the engine.
 *
 *****************************************************************************/
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    VisItTimeStepChanged();
    VisItUpdatePlots();

    if(sim->savingFiles)
    {
        char filename[100];
        sprintf(filename, "updateplots%04d.jpg", sim->saveCounter);
        if(VisItSaveWindow(filename, 800, 800, VISIT_IMAGEFORMAT_JPEG) == VISIT_OKAY)
        {
            sim->saveCounter++;
            if(sim->par_rank == 0)
                printf("Saved %s\n", filename);
        }
        else if(sim->par_rank == 0)
            printf("The image could not be saved to %s\n", filename);
    }
}

/* Callback function for control commands, which are the buttons in the 
 * GUI's Simulation window. This type of command is handled automatically
 * provided that you have registered a command callback such as this.
 */
void ControlCommandCallback(const char *cmd, const char *args, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;
    else if(strcmp(cmd, "addplot") == 0)
    {
        VisItExecuteCommand("AddPlot(\"Pseudocolor\", \"zonal\")\n");
        VisItExecuteCommand("DrawPlots()\n");
    }
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
void SlaveProcessCallback(void)
{
   int command = VISIT_COMMAND_PROCESS;
   BroadcastSlaveCommand(&command);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(simulation_data *sim)
{
    int command;
    if (sim->par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success == VISIT_OKAY)
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
ProcessConsoleCommand(simulation_data *sim)
{
    /* Read A Command */
    char cmd[1000];

    if (sim->par_rank == 0)
    {
        if(VisItReadConsole(1000, cmd) == VISIT_ERROR)
        {
            sprintf(cmd, "quit");
            printf("quit\n");
        }
    }

#ifdef PARALLEL
    /* Broadcast the command to all processors. */
    MPI_Bcast(cmd, 1000, MPI_CHAR, 0, MPI_COMM_WORLD);
#endif

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
    else if(strcmp(cmd, "addplot") == 0)
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

void mainloop(simulation_data *sim)
{
    int blocking, visitstate, err = 0;

    /* If we're not running by default then simulate once there's something
     * once VisIt connects.
     */
    if(sim->runMode == SIM_STOPPED)
        simulate_one_timestep(sim);

    if (sim->par_rank == 0)
    {
        fprintf(stderr, "command> ");
        fflush(stderr);
    }

    do
    {
        blocking = (sim->runMode == SIM_RUNNING) ? 0 : 1;
        /* Get input from VisIt or timeout so the simulation can run. */
        if(sim->par_rank == 0)
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
            simulate_one_timestep(sim);
            break;
        case 1:
            /* VisIt is trying to connect to sim. */
            if(VisItAttemptToCompleteConnection() == VISIT_OKAY)
            {
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback, (void*)sim);
                VisItSetSlaveProcessCallback(SlaveProcessCallback);

                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetCurve(SimGetCurve, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetDomainList(SimGetDomainList, (void*)sim);
            }
            else 
            {
                /* Print the error message */
                char *err = VisItGetLastError();
                fprintf(stderr, "VisIt did not connect: %s\n", err);
                free(err);
            }
            break;
        case 2:
            /* VisIt wants to tell the engine something. */
            if(!ProcessVisItCommand(sim))
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                /*sim->runMode = SIM_RUNNING;*/
            }
            break;
        case 3:
            /* VisItDetectInput detected console input - do something with it.
             * NOTE: you can't get here unless you pass a file descriptor to
             * VisItDetectInput instead of -1.
             */
            ProcessConsoleCommand(sim);
            if (sim->par_rank == 0)
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
    } while(!sim->done && err == 0);
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
    simulation_data sim;
    simulation_data_ctor(&sim);

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &sim.par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &sim.par_size);

    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell VSIL whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);
#endif

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(sim.par_rank == 0)
    {
        /* Write out .sim file that VisIt uses to connect. */
        VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "updateplots_par",
#else
            "updateplots",
#endif
            "Demonstrates VisItUpdatePlots function",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/* DATA ACCESS FUNCTIONS */

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
        visit_handle cmd = VISIT_INVALID_HANDLE;
        visit_handle emd = VISIT_INVALID_HANDLE;

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
            VisIt_MeshMetaData_setNumDomains(mmd, sim->par_size);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
            VisIt_MeshMetaData_setNumGroups(mmd, 0);
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setZUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");
            VisIt_MeshMetaData_setZLabel(mmd, "Depth");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add a curve variable. */
        if(VisIt_CurveMetaData_alloc(&cmd) == VISIT_OKAY)
        {
            VisIt_CurveMetaData_setName(cmd, "sine");
            VisIt_CurveMetaData_setXLabel(cmd, "Angle");
            VisIt_CurveMetaData_setXUnits(cmd, "radians");
            VisIt_CurveMetaData_setYLabel(cmd, "Amplitude");
            VisIt_CurveMetaData_setYUnits(cmd, "");

            VisIt_SimulationMetaData_addCurve(md, cmd);
        }

        /* Add an expression. */
        if(VisIt_ExpressionMetaData_alloc(&emd) == VISIT_OKAY)
        {
            VisIt_ExpressionMetaData_setName(emd, "zvec");
            VisIt_ExpressionMetaData_setDefinition(emd, "{zonal, zonal}");
            VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_VECTOR);

            VisIt_SimulationMetaData_addExpression(md, emd);
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

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "mesh2d") == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int i, minRealIndex[3]={0,0,0}, maxRealIndex[3]={0,0,0};
            double *rmesh_x, *rmesh_y;
            visit_handle hx, hy;

            maxRealIndex[0] = rmesh_dims[0]-1;
            maxRealIndex[1] = rmesh_dims[1]-1;
            maxRealIndex[2] = rmesh_dims[2]-1;

            rmesh_x = (double *)malloc(sizeof(double) * RNX);
            for(i = 0; i < RNX; ++i)
                rmesh_x[i] = ((double)i / (double)(RNX-1)) * 5. - 2.5 + 5. * domain;
            rmesh_y = (double *)malloc(sizeof(double) * RNY);
            for(i = 0; i < RNY; ++i)
                rmesh_y[i] = ((double)i / (double)(RNY-1)) * 5. - 2.5;

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_setDataD(hx, VISIT_OWNER_VISIT, 1, RNX, rmesh_x);
            VisIt_VariableData_setDataD(hy, VISIT_OWNER_VISIT, 1, RNY, rmesh_y);
            VisIt_RectilinearMesh_setCoordsXY(h, hx, hy);
            VisIt_RectilinearMesh_setRealIndices(h, minRealIndex, maxRealIndex);
        }
    }

    return h;
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

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "zonal") == 0)
    {
        double angle;
        double xpos, ypos, cellX, cellY, dX, dY, tx, ty, *zoneptr;
        double sx, ex, sy, ey, *rmesh_zonal;
        int i, j, nTuples;

        sx = -2.5  + domain * 5.;
        ex = sx + 5.;
        sy = -2.5;
        ey = sy + 5.;

        /* Calculate a zonal variable that moves around. */
        rmesh_zonal = (double*)malloc(sizeof(double) * (RNX-1) * (RNY-1));
        zoneptr = rmesh_zonal;
        angle = sim->time;
        xpos = 2.5 * cos(angle);
        ypos = 2.5 * sin(angle);
        for(j = 0; j < rmesh_dims[1]-1; ++j)
        {
            ty = (double)j / (double)(rmesh_dims[1]-1-1);
            cellY = (1.-ty)*sy + ey*ty;
            dY = cellY - ypos;
            for(i = 0; i < rmesh_dims[0]-1; ++i)
            {
                tx = (double)i / (double)(rmesh_dims[0]-1-1);
                cellX = (1.-tx)*sx + ex*tx;
                dX = cellX - xpos;
                *zoneptr++ = sqrt(dX * dX + dY * dY);
            }
        }

        nTuples = (rmesh_dims[0]-1) * (rmesh_dims[1]-1);
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
            nTuples, rmesh_zonal);
    }

    return h;
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

visit_handle
SimGetCurve(const char *name, void *cbdata)
{
    int h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "sine") == 0)
    {
        if(VisIt_CurveData_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxc, hyc;
            int i;
            double *x = NULL, *y = NULL;
            x = (double*)malloc(200 * sizeof(double));
            y = (double*)malloc(200 * sizeof(double));
        
            for(i = 0; i < 200; ++i)
            {
                double angle = sim->time + ((double)i / (double)(200-1)) * 4. * M_PI;
                x[i] = angle;
                y[i] = sin(x[i]);
            }

            /* Give the arrays to VisIt. VisIt will free them. */
            VisIt_VariableData_alloc(&hxc);
            VisIt_VariableData_alloc(&hyc);
            VisIt_VariableData_setDataD(hxc, VISIT_OWNER_VISIT, 1, 200, x);
            VisIt_VariableData_setDataD(hyc, VISIT_OWNER_VISIT, 1, 200, y);
            VisIt_CurveData_setCoordsXY(h, hxc, hyc);
        }
    }

    return h;
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

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
    {
        visit_handle hdl;
        int *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int));
        *iptr = sim->par_rank;

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
        VisIt_DomainList_setDomains(h, sim->par_size, hdl);
    }
    return h;
}
