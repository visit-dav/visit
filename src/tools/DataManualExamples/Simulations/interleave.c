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

#include "SimulationExample.h"

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
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

const char *cmd_names[] = {"halt", "step", "run", "update"};

void read_input_deck(void) { }

/* SIMULATE ONE TIME STEP */
void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += 0.0134;
    printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
    sim_sleep(1);
}

/******************************************************************************
 *
 * Purpose: Callback function for control commands.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar  3 14:37:57 PST 2010
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
 * Date:       Wed Mar  3 14:37:57 PST 2010
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
 * Date:       Wed Mar  3 14:37:57 PST 2010
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
    VisItInitializeSocketAndDumpSimFile("interleave",
        "Demonstrates interleaved coordinates",
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

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar  3 14:37:57 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMetaData(void *cbdata)
{
    visit_handle md = VISIT_INVALID_HANDLE;
    const char *meshNames[] = {
        "curv2d_static_float", "curv2d_static_double", 
        "curv2d_dynamic_float", "curv2d_dynamic_double",
        "curv3d_static_float", "curv3d_static_double", 
        "curv3d_dynamic_float", "curv3d_dynamic_double",

        "point2d_static_float", "point2d_static_double", 
        "point2d_dynamic_float", "point2d_dynamic_double",
        "point3d_static_float", "point3d_static_double", 
        "point3d_dynamic_float", "point3d_dynamic_double",

        "ucd2d_static_float", "ucd2d_static_double", 
        "ucd2d_dynamic_float", "ucd2d_dynamic_double",
        "ucd3d_static_float", "ucd3d_static_double", 
        "ucd3d_dynamic_float", "ucd3d_dynamic_double"
    };
    const int meshTypes[] = {
        VISIT_MESHTYPE_CURVILINEAR,  VISIT_MESHTYPE_CURVILINEAR,
        VISIT_MESHTYPE_CURVILINEAR,  VISIT_MESHTYPE_CURVILINEAR,
        VISIT_MESHTYPE_CURVILINEAR,  VISIT_MESHTYPE_CURVILINEAR,
        VISIT_MESHTYPE_CURVILINEAR,  VISIT_MESHTYPE_CURVILINEAR,

        VISIT_MESHTYPE_POINT,        VISIT_MESHTYPE_POINT,
        VISIT_MESHTYPE_POINT,        VISIT_MESHTYPE_POINT,
        VISIT_MESHTYPE_POINT,        VISIT_MESHTYPE_POINT,
        VISIT_MESHTYPE_POINT,        VISIT_MESHTYPE_POINT,

        VISIT_MESHTYPE_UNSTRUCTURED, VISIT_MESHTYPE_UNSTRUCTURED,
        VISIT_MESHTYPE_UNSTRUCTURED, VISIT_MESHTYPE_UNSTRUCTURED,
        VISIT_MESHTYPE_UNSTRUCTURED, VISIT_MESHTYPE_UNSTRUCTURED,
        VISIT_MESHTYPE_UNSTRUCTURED, VISIT_MESHTYPE_UNSTRUCTURED
    };
    const int dims[] = {
        2,2,2,2,3,3,3,3,
        2,2,2,2,3,3,3,3,
        2,2,2,2,3,3,3,3
    };
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create metadata. */
    if(VisIt_SimulationMetaData_alloc(&md) == VISIT_OKAY)
    {
        int i;
        visit_handle cmd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        for(i = 0; i < 24; ++i)
        {
            visit_handle mmd = VISIT_INVALID_HANDLE;
            if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
            {
                /* Set the mesh's properties.*/
                VisIt_MeshMetaData_setName(mmd, meshNames[i]);
                VisIt_MeshMetaData_setMeshType(mmd, meshTypes[i]);
                VisIt_MeshMetaData_setTopologicalDimension(mmd, dims[i]);
                VisIt_MeshMetaData_setSpatialDimension(mmd, dims[i]);
                VisIt_MeshMetaData_setNumDomains(mmd, 1);
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

#define NX 5
#define NY 4
#define NZ 3

float coords2d[NY][NX][2] = {
{{0,0}, {1,0}, {2,0}, {3,0}, {4,0}},
{{0,1}, {1,1}, {2,1}, {3,1}, {4,1}},
{{0,2}, {1,2}, {2,2}, {3,2}, {4,2}},
{{0,3}, {1,3}, {2,3}, {3,3}, {4,3}}
};

float coords3d[NZ][NY][NX][3] = {
{ /* plane 1 */
{{0,0,0}, {1,0,0}, {2,0,0}, {3,0,0}, {4,0,0}},
{{0,1,0}, {1,1,0}, {2,1,0}, {3,1,0}, {4,1,0}},
{{0,2,0}, {1,2,0}, {2,2,0}, {3,2,0}, {4,2,0}},
{{0,3,0}, {1,3,0}, {2,3,0}, {3,3,0}, {4,3,0}}
},
{ /* plane 2 */
{{0,0,1}, {1,0,1}, {2,0,1}, {3,0,1}, {4,0,1}},
{{0,1,1}, {1,1,1}, {2,1,1}, {3,1,1}, {4,1,1}},
{{0,2,1}, {1,2,1}, {2,2,1}, {3,2,1}, {4,2,1}},
{{0,3,1}, {1,3,1}, {2,3,1}, {3,3,1}, {4,3,1}}
},
{ /* plane 3 */
{{0,0,2}, {1,0,2}, {2,0,2}, {3,0,2}, {4,0,2}},
{{0,1,2}, {1,1,2}, {2,1,2}, {3,1,2}, {4,1,2}},
{{0,2,2}, {1,2,2}, {2,2,2}, {3,2,2}, {4,2,2}},
{{0,3,2}, {1,3,2}, {2,3,2}, {3,3,2}, {4,3,2}}
}
};

/******************************************************************************
 *
 * Purpose: This function copies data into the coordinate variable data object
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar  3 14:37:57 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/


void
SetData(visit_handle c, int owner, int dataType, int nTuples, int nComps, float *f)
{
    if(dataType == VISIT_DATATYPE_FLOAT)
    {
        if(owner == VISIT_OWNER_SIM)
            VisIt_VariableData_setDataF(c, owner, nTuples, nComps, f);
        else
        {
            float *d = (float *)malloc(nTuples * nComps * sizeof(float));
            memcpy(d, f, nTuples * nComps * sizeof(float));
            VisIt_VariableData_setDataF(c, owner, nTuples, nComps, d);
        }
    }
    else
    {
        /* Convert the data to double and let VisIt own it. */
        int i;
        double *d = (double *)malloc(nTuples * nComps * sizeof(double));
        for(i = 0; i < nTuples * nComps; ++i)
            d[i] = (double)f[i];
        VisIt_VariableData_setDataD(c, VISIT_OWNER_VISIT, nTuples, nComps, d);
    }
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar  3 14:37:57 PST 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    visit_handle c = VISIT_INVALID_HANDLE;
    int owner = VISIT_OWNER_SIM;
    int dt = VISIT_DATATYPE_FLOAT;

    if(strstr(name, "double") != NULL)
        dt = VISIT_DATATYPE_DOUBLE;
    if(strstr(name, "dynamic") != NULL)
        owner = VISIT_OWNER_VISIT;

    if(strstr(name, "curv") != NULL)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&c) == VISIT_OKAY)
        {
            if(strstr(name, "curv2d") != NULL)
            {
                int dims[2];
                dims[0] = NX;
                dims[1] = NY;
                SetData(c, owner, dt, 2, NX*NY, (float*)coords2d);
                VisIt_CurvilinearMesh_setCoords2(h, dims, c);
            }
            else
            {
                int dims[3];
                dims[0] = NX;
                dims[1] = NY;
                dims[2] = NZ;
                SetData(c, owner, dt, 3, NX*NY*NZ, (float*)coords3d);
                VisIt_CurvilinearMesh_setCoords3(h, dims, c);
            }
        }
    }
    else if(strstr(name, "point") != NULL)
    {
        if(VisIt_PointMesh_alloc(&h) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&c) == VISIT_OKAY)
        {
            if(strstr(name, "point2d") != NULL)
                SetData(c, owner, dt, 2, NX*NY, (float*)coords2d);
            else
                SetData(c, owner, dt, 3, NX*NY*NZ, (float*)coords3d);

            VisIt_PointMesh_setCoords(h, c);
        }
    }
    else if(strstr(name, "ucd") != NULL)
    {
        visit_handle hc;
        if(VisIt_UnstructuredMesh_alloc(&h) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&c) == VISIT_OKAY &&
           VisIt_VariableData_alloc(&hc) == VISIT_OKAY)
        {
            int nzones = 0;
            if(strstr(name, "ucd2d") != NULL)
            {
                int i, j, lconnectivity;
                int *connectivity = NULL, *conn = NULL;

                nzones = (NX-1)*(NY-1);
                lconnectivity = 5*nzones;
                conn = connectivity = (int*)malloc(lconnectivity * sizeof(int));
                for(j = 0; j < NY-1; ++j)
                    for(i = 0; i < NX-1; ++i)
                    {
                        *conn++ = VISIT_CELL_QUAD;
                        *conn++ = j*NX + i;
                        *conn++ = j*NX + (i+1);
                        *conn++ = (j+1)*NX + (i+1);
                        *conn++ = (j+1)*NX + i;
                    }
                SetData(c, owner, dt, 2, NX*NY, (float*)coords2d);
                VisIt_VariableData_setDataI(hc, VISIT_OWNER_VISIT, 1, lconnectivity,
                    connectivity);
            }
            else
            {
                int i, j, k, lconnectivity;
                int *connectivity = NULL, *conn = NULL;

                nzones = (NX-1)*(NY-1)*(NZ-1);
                lconnectivity = 9*nzones;
                conn = connectivity = (int*)malloc(lconnectivity * sizeof(int));
                for(k = 0; k < NZ-1; ++k)
                  for(j = 0; j < NY-1; ++j)
                    for(i = 0; i < NX-1; ++i)
                    {
                        *conn++ = VISIT_CELL_HEX;
                        *conn++ = k*NX*NY + j*NX + i;
                        *conn++ = k*NX*NY + j*NX + (i+1);
                        *conn++ = k*NX*NY + (j+1)*NX + (i+1);
                        *conn++ = k*NX*NY + (j+1)*NX + i;
                        *conn++ = (k+1)*NX*NY + j*NX + i;
                        *conn++ = (k+1)*NX*NY + j*NX + (i+1);
                        *conn++ = (k+1)*NX*NY + (j+1)*NX + (i+1);
                        *conn++ = (k+1)*NX*NY + (j+1)*NX + i;
                    }
                SetData(c, owner, dt, 3, NX*NY*NZ, (float*)coords3d);
                VisIt_VariableData_setDataI(hc, VISIT_OWNER_VISIT, 1, lconnectivity,
                    connectivity);
            }

            VisIt_UnstructuredMesh_setCoords(h, c);
            VisIt_UnstructuredMesh_setConnectivity(h, nzones, hc);
        }
    }
    return h;
}

