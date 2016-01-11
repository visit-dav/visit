/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
visit_handle SimGetMetaData(void *);
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

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += 0.0134;
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
    sim_sleep(1);
    VisItTimeStepChanged();
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
    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("higherorder",
        "Demonstrates creating an unstructured mesh of higher order cells",
        "/path/to/where/sim/was/started",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop();

    return 0;
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

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Fill in the metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "edges");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 1);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "surfaces");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "volumes");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add some custom commands. */
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

/* A mesh of quadratic edge line segments. */
double edge_coords[][2] = {{1., 0.}, {1.,1.}, {0.,1.}, {-1, 1.}, 
                           {-1,0}, {-1,-1}, {0,-1}, {1, -1}};
int edge_conn[] = {
    VISIT_CELL_QUADRATIC_EDGE, 0,2,1,
    VISIT_CELL_QUADRATIC_EDGE, 2,4,3,
    VISIT_CELL_QUADRATIC_EDGE, 4,6,5,
    VISIT_CELL_QUADRATIC_EDGE, 6,0,7
};
int edge_nnodes = 8;
int edge_nzones = 4;
int ledge_conn = sizeof(edge_conn) / sizeof(int);

/* A mesh of quadratic surface cell types */
double surface_coords[][2] = {
    {0., 0.}, {1., 0.}, {1.,1.}, {0.25,-0.25}, {1.25,0.5}, {0.6,0.4},
    {2,0}, {4,0}, {4,4}, {2,4}, {3,0.3}, {4.3,2}, {3,3.7}, {2.3,2}
};
int surface_conn[] = {
    VISIT_CELL_QUADRATIC_TRI, 0,1,2,3,4,5,
    VISIT_CELL_QUADRATIC_QUAD, 6,7,8,9,10,11,12,13
};
int surface_nnodes = 14;
int surface_nzones = 2;
int lsurface_conn = sizeof(surface_conn) / sizeof(int);

/* A mesh of volume cell types */
double volume_coords[][3] = {
/* cell 0 coordinates */
/* 0*/{1.5, 0, 3},
/* 1*/{4.5, 0, 2.7},
/* 2*/{1, 0.5, 0.}, 
/* 3*/{3, 3, 1.5},
/* 4*/{2.8, -0.1, 3.1},
/* 5*/{2.8, 0.2, 1.4},
/* 6*/{0.8, 0.1, 2.2},
/* 7*/{2.2, 1.5, 2.8},
/* 8*/{3.5, 1.5, 2.5},
/* 9*/{2, 2, 0.8},
/*cell 1 coordinates*/
/*10*/{6,0,4},
/*11*/{10,0,4},
/*12*/{10,0,0},
/*13*/{6,0,0},
/*14*/{6,4,4},
/*15*/{11,4,4},
/*16*/{11,4,0},
/*17*/{6,4,0},
/*18*/{8,0,4},
/*19*/{10,0,2},
/*20*/{8,0,0},
/*21*/{6,0,2},
/*22*/{8,4,4},
/*23*/{11.5,4,2},
/*24*/{8,4,0},
/*25*/{6,4,2},
/*26*/{6.5,2,3.5},
/*27*/{2+7.5,2,3.5},
/*28*/{2+7.5,2,0.5},
/*29*/{6.5,2,0.5},
/*cell 2 coordinates */
/*30*/{13,0,2},
/*31*/{15,0,2},
/*32*/{14,0,0},
/*33*/{13,4,2},
/*34*/{15,4,2},
/*35*/{14,4,0},
/*36*/{14,0,2},
/*37*/{14.5,0,1},
/*38*/{13.5,0,1},
/*39*/{14,4,2},
/*40*/{14.5,4,1},
/*41*/{13.5,4,1},
/*42*/{13.25,2,2},
/*43*/{15,2,2},
/*44*/{13.75,2,0},
/*cell 3 coordinates */
/*45*/{17,0,4},
/*46*/{19,0,4},
/*47*/{19,0,0},
/*48*/{17,0,0},
/*49*/{18,4,2},
/*50*/{18,0,4},
/*51*/{19,0,2},
/*52*/{18,0,0},
/*53*/{16.5,0,2},
/*54*/{17.25,3,2},
/*55*/{18.5,2,2.5},
/*56*/{18.5,2,1.5},
/*57*/{17.25,3,1.25}
};
int volume_conn[] = {
    VISIT_CELL_QUADRATIC_TET, 0,1,2,3,4,5,6,7,8,9,
    VISIT_CELL_QUADRATIC_HEX, 10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,
    VISIT_CELL_QUADRATIC_WEDGE, 30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,
    VISIT_CELL_QUADRATIC_PYR, 45,46,47,48,49,50,51,52,53,54,55,56,57
};
int volume_nnodes = sizeof(volume_coords) / (sizeof(double)*3);
int volume_nzones = 4;
int lvolume_conn = sizeof(volume_conn) / sizeof(int);

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

    if(strcmp(name, "edges") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle coords, conn;

            VisIt_VariableData_alloc(&coords);
            VisIt_VariableData_setDataD(coords, VISIT_OWNER_SIM, 2, 
                edge_nnodes, (double*)edge_coords);

            VisIt_VariableData_alloc(&conn);
            VisIt_VariableData_setDataI(conn, VISIT_OWNER_SIM, 1, 
                ledge_conn, edge_conn);

            VisIt_UnstructuredMesh_setCoords(h, coords);
            VisIt_UnstructuredMesh_setConnectivity(h, edge_nzones, conn);
        }
    }
    else if(strcmp(name, "surfaces") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle coords, conn;

            VisIt_VariableData_alloc(&coords);
            VisIt_VariableData_setDataD(coords, VISIT_OWNER_SIM, 2, 
                surface_nnodes, (double*)surface_coords);

            VisIt_VariableData_alloc(&conn);
            VisIt_VariableData_setDataI(conn, VISIT_OWNER_SIM, 1, 
                lsurface_conn, surface_conn);

            VisIt_UnstructuredMesh_setCoords(h, coords);
            VisIt_UnstructuredMesh_setConnectivity(h, surface_nzones, conn);
        }
    }
    else if(strcmp(name, "volumes") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle coords, conn;

            VisIt_VariableData_alloc(&coords);
            VisIt_VariableData_setDataD(coords, VISIT_OWNER_SIM, 3, 
                volume_nnodes, (double*)volume_coords);

            VisIt_VariableData_alloc(&conn);
            VisIt_VariableData_setDataI(conn, VISIT_OWNER_SIM, 1, 
                lvolume_conn, volume_conn);

            VisIt_UnstructuredMesh_setCoords(h, coords);
            VisIt_UnstructuredMesh_setConnectivity(h, volume_nzones, conn);
        }
    }

    return h;
}
