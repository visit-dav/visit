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
    int     echo;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;
    sim->echo = 0;
}

void
simulation_data_dtor(simulation_data *sim)
{
}

const char *cmd_names[] = {"halt", "step", "run", "update", "saveimage"};

void simulate_one_timestep(simulation_data *sim);
void read_input_deck(void) { }

/*************************** CSG Mesh variables *****************************/
#define PLANE1_COEFF 8
#define PLANE2_COEFF (PLANE1_COEFF+6)
#define PLANE3_COEFF (PLANE2_COEFF+6)
#define PLANE4_COEFF (PLANE3_COEFF+6)
double csg_extents[] = {-11., -11., -11., 11., 11., 11.};

/* CSG Boundaries */
int csg_bound_types[] = {
    VISIT_CSG_SPHERE_PR,
    VISIT_CSG_SPHERE_PR,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN,
    VISIT_CSG_PLANE_PN
};

double csg_bound_coeffs[] = {
    0.,  0., 0.,  8.,          /* sphere 1*/
    0.,  0., 0., 10.,          /* sphere 2*/
    0.,  2., 0.,  0., -1., 0., /* plane 1 point, normal*/
    0., -2., 0.,  0.,  1., 0., /* plane 2 point, normal*/
    2.,  0., 0., -1.,  0., 0., /* plane 3 point, normal*/
   -2.,  0., 0.,  1.,  0., 0.  /* plane 4 point, normal*/
};
int csg_num_bound_coeffs = sizeof(csg_bound_coeffs) / sizeof(float);
int csg_num_bound_types = sizeof(csg_bound_types) / sizeof(int);

/* CSG Regions */
int csg_region_operations[] =
{
    VISIT_CSG_OUTER,          /* 0: outside of inner sphere */
    VISIT_CSG_INNER,          /* 1: inside of outer sphere  */
    VISIT_CSG_OUTER,          /* 2: plane 1 */
    VISIT_CSG_OUTER,          /* 3: plane 2 */
    VISIT_CSG_OUTER,          /* 4: plane 3 */
    VISIT_CSG_OUTER,          /* 5: plane 4 */
    VISIT_CSG_INTERSECT,      /* 6: intersection between sphere 0,1  */
    VISIT_CSG_INTERSECT,      /* 7: intersection between plane 1,2 half spaces*/
    VISIT_CSG_INTERSECT,      /* 8: intersection between plane 3,4 half spaces*/
    VISIT_CSG_UNION,          /* 9  add the 2 blocks together */
    VISIT_CSG_INTERSECT       /* 10: intersect the spherical shell with the blocks. */
};

/* index               0   1   2   3   4   5   6   7   8   9  10*/
int csg_leftids[] =  { 0,  1,  2,  3,  4,  5,  0,  2,  4,  7,  6};
int csg_rightids[] = {-1, -1, -1, -1, -1, -1,  1,  3,  5,  8,  9};
int csg_num_region_operations = sizeof(csg_region_operations) / sizeof(int);

/* CSG Zones */
int csg_zonelist[] = {10};
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

    if (sim->echo)
    {
        fprintf(stderr, "Command '%s' completed.\n", cmd);
        fflush(stderr);
    }
}

/* SIMULATE ONE TIME STEP */
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
    csg_bound_coeffs[PLANE3_COEFF+3] = -cosA;
    csg_bound_coeffs[PLANE3_COEFF+4] = 0.;
    csg_bound_coeffs[PLANE3_COEFF+5] = -sinA;

    csg_bound_coeffs[PLANE4_COEFF] = 2.* cosB;
    csg_bound_coeffs[PLANE4_COEFF+1] = 0.;
    csg_bound_coeffs[PLANE4_COEFF+2] = 2.* sinB;;
    csg_bound_coeffs[PLANE4_COEFF+3] = cosA;
    csg_bound_coeffs[PLANE4_COEFF+4] = 0.;
    csg_bound_coeffs[PLANE4_COEFF+5] = sinA;
    }

    sim_sleep(1);
}

/******************************************************************************
 *
 * Purpose: This is the main event loop function.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Feb  6 14:29:36 PST 2009
 *
 * Modifications:
 *    Kathleen Biagas, Wed Jul  2 15:17:23 MST 2014
 *    Move construction of simulation to main, send simulation_data as arg.
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
 *   Kathleen Biagas, Wed Jul  2 15:18:18 PDT 2014
 *   Construct/destruct simulation_data here.
 *
 *****************************************************************************/

int main(int argc, char **argv)
{
    int i;
    /* Set up some simulation data. */
    simulation_data sim;
    simulation_data_ctor(&sim);

    VisItOpenTraceFile("csg_trace.txt");

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);
    VisItSetupEnvironment();

    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-echo") == 0)
            sim.echo = 1;
    }

    /* Write out .sim2 file that VisIt uses to connect. */
    VisItInitializeSocketAndDumpSimFile("csg",
        "Demonstrates creating a csg mesh",
        "/path/to/where/sim/was/started",
        NULL, NULL, SimulationFilename());

    /* Read input problem setup, geometry, data. */
    read_input_deck();

    /* Call the main loop. */
    mainloop(&sim);

    /* Clean up */
    simulation_data_dtor(&sim);

    VisItCloseTraceFile();

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

        /* Fill in the AMR metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "csg");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CSG);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Regions");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "region");
            VisIt_MeshMetaData_addDomainName(mmd, "Clipped Hollow Sphere");
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setZUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");
            VisIt_MeshMetaData_setZLabel(mmd, "Depth");

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
                1, csg_num_bound_types, csg_bound_types);
            VisIt_CSGMesh_setBoundaryTypes(h, boundaryTypes);

            VisIt_VariableData_alloc(&boundaryCoeffs);
            VisIt_VariableData_setDataD(boundaryCoeffs, VISIT_OWNER_SIM, 
                1, csg_num_bound_coeffs, csg_bound_coeffs);
            VisIt_CSGMesh_setBoundaryCoeffs(h, boundaryCoeffs);

            /* Set the extents */
            VisIt_CSGMesh_setExtents(h, csg_extents, csg_extents+3);

            /* Set the regions */
            VisIt_VariableData_alloc(&typeflags);
            VisIt_VariableData_setDataI(typeflags, VISIT_OWNER_SIM, 
                1, csg_num_region_operations, csg_region_operations);

            VisIt_VariableData_alloc(&leftids);
            VisIt_VariableData_setDataI(leftids, VISIT_OWNER_SIM, 
                1, csg_num_region_operations, csg_leftids);

            VisIt_VariableData_alloc(&rightids);
            VisIt_VariableData_setDataI(rightids, VISIT_OWNER_SIM, 
                1, csg_num_region_operations, csg_rightids);

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
