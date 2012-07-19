/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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
visit_handle SimGetMaterial(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetMixedVariable(int, const char *, void *);

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
 * Date:       Thu Jul 19 14:54:30 PDT 2012
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
    if(VisItReadConsole(1000, cmd) == VISIT_ERROR)
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
 * Date:       Thu Jul 19 14:54:30 PDT 2012
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
                VisItSetGetMaterial(SimGetMaterial, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetMixedVariable(SimGetMixedVariable, (void*)sim);
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
 * Date:       Thu Jul 19 14:54:30 PDT 2012
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
    VisItInitializeSocketAndDumpSimFile("materialvar",
        "Demonstrates variables restricted to certain materials",
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

/* Values to match the Fortran example. */
#define NX 5
#define NY 4
#define XMIN 0.f
#define XMAX 4.f
#define YMIN 0.f
#define YMAX 3.f
const char *matNames[] = {"Water", "Membrane", "Air"};

#define DX (XMAX - XMIN)

/* Rectilinear mesh */
static float rmesh_x[NX];
static float rmesh_y[NY];
static const int   rmesh_dims[] = {NX, NY, 1};
static const int   rmesh_ndims = 2;

/* The matlist table indicates the material numbers that are found in
 * each cell. Every 3 numbers indicates the material numbers in a cell.
 * A material number of 0 means that the material entry is not used.
 */
int matlist[NY-1][NX-1][3] = {
    {{3,0,0},{2,3,0},{1,2,0},{1,0,0}},
    {{3,0,0},{2,3,0},{1,2,0},{1,0,0}},
    {{3,0,0},{2,3,0},{1,2,3},{1,2,0}}
};

/* The mat_vf table indicates the material volume fractions that are
 * found in a cell.
 */
float mat_vf[NY-1][NX-1][3] = {
    {{1.,0.,0.},{0.75,0.25,0.},  {0.8125,0.1875, 0.},{1.,0.,0.}},
    {{1.,0.,0.},{0.625,0.375,0.},{0.5625,0.4375,0.}, {1.,0.,0.}},
    {{1.,0.,0.},{0.3,0.7,0.},    {0.2,0.4,0.4},      {0.55,0.45,0.}}
};

/******************************************************************************
 *
 * Purpose: Return whether the cell contains the specified materials.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int
cell_contains_mats(const int *cellmats, const int *mats, int nmat)
{
    int i,j;
    for(i = 0; i < 3; ++i)
        for(j = 0; j < nmat; ++j)
            if(cellmats[i] == mats[j])
                return 1;
    return 0;
}

/******************************************************************************
 *
 * Purpose: Return whether the cell is mixed.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int
cell_is_mixed(const int *cellmats)
{
    /* Mixed cells have 1 or 0 zeroes in matlist */
    int m, zeroes = 0;
    for(m = 0; m < 3; ++m)
        zeroes += (cellmats[m] == 0) ? 1 : 0;
    return (zeroes <= 1) ? 1 : 0;
}

/******************************************************************************
 *
 * Purpose: Return an array containing the cell numbers of all cells that contain
 *          the specified material.
 *
 * Notes: We use this method to return per-material variables so we can check
 *        that the expanded variable that VisIt creates gets mapped to the
 *        right cell in the mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int *
cell_number_for_cells_with_mat(int mat, int *nTuples)
{
    int i,j, cell = 0, idx = 0;
    int *f = (int*)malloc(sizeof(int)*(NX-1)*(NY-1));
    for(j = 0; j < NY-1; ++j)
    {
        for(i = 0; i < NX-1; ++i, ++cell)
        {
            if(cell_contains_mats(matlist[j][i], &mat, 1))
                f[idx++] = cell;
        }
    }
    *nTuples = idx;
    return f;
}

/******************************************************************************
 *
 * Purpose: Return an array containing the node numbers of all cells that contain
 *          the specified material. This is a nodal variable.
 *
 * Notes: We use this method to return per-material variables so we can check
 *        that the expanded variable that VisIt creates gets mapped to the
 *        right node in the mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int *
node_number_for_cells_with_mats(const int *mats, int nmats, int *nTuples)
{
    int i,j,n,m, idx = 0;
    int *f = (int*)malloc(sizeof(int)*NX*NY);
    for(j = 0; j < NY-1; ++j)
    {
        for(i = 0; i < NX-1; ++i)
        {
            if(cell_contains_mats(matlist[j][i], mats, nmats))
            {
                /* node numbers for this cell. This cell ordering matches
                   VTK_PIXEL, which is the cell type that this cell seems
                   to make in VTK.
                 
                   nodes[2] *-----* nodes[3]
                            |     |
                            |     |
                   nodes[0] *-----* nodes[1]

                   For node ordering, see pages 9-10 of
                       http://www.vtk.org/VTK/img/file-formats.pdf
                 */
                int nodes[4];
                nodes[0] = j * NX + i;
                nodes[1] = j * NX + i + 1;
                nodes[2] = (j+1) * NX + i;
                nodes[3] = (j+1) * NX + i + 1;

                /* Add nodes to the list if they are not already in it. */
                for(n = 0; n < 4; ++n)
                {
                    int found = 0;
                    for(m = 0; m < idx; ++m)
                    {
                        if(f[m] == nodes[n])
                        {
                            found = 1;
                            break;
                        }
                    }
                    if(found == 0)
                        f[idx++] = nodes[n];
                }
            }
        }
    }
    *nTuples = idx;
    return f;
}

/******************************************************************************
 *
 * Purpose: Return the largest material for all cells that contain any
 *          of the specified materials.
 *
 * Notes: We use this function to return the material numbers as a per-material
 *        variable so we can make sure that the expanded variable that VisIt
 *        creates has the largest volume fraction.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int *
mat_number_for_cells_with_mats(const int *mats, int nmats, int *nTuples)
{
    int i,j,m, idx = 0;
    float vf;
    int *f = (int*)malloc(sizeof(int)*(NX-1)*(NY-1));
    for(j = 0; j < NY-1; ++j)
    {
        for(i = 0; i < NX-1; ++i)
        {
            if(cell_contains_mats(matlist[j][i], mats, nmats))
            {
                /* Save the material with the largest VF. */
                m = matlist[j][i][0];
                vf = mat_vf[j][i][0];
                if(mat_vf[j][i][1] > vf)
                {
                    m = matlist[j][i][1];
                    vf = mat_vf[j][i][1];
                }
                if(mat_vf[j][i][2] > vf)
                    m = matlist[j][i][2];
                f[idx++] = m;
            }
        }
    }
    *nTuples = idx;
    return f;
}

/******************************************************************************
 *
 * Purpose: Return the material numbers for all cells that contain any of the 
 *          specified materials.
 *
 * Notes: This function returns the per-material values for the variable. In
 *        this case, we return the material numbers. This makes sure that when
 *        we force MIR, the proper material numbers will appear on the 
 *        reconstructed pieces.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Jul 18 16:49:26 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int *
mat_numbers_for_cells_with_mats(const int *mats, int nmats, int *nTuples)
{
    int i,j,m, idx = 0;
    int *f = (int*)malloc(sizeof(int)*(NX-1)*(NY-1)*3);
    for(j = 0; j < NY-1; ++j)
    {
        for(i = 0; i < NX-1; ++i)
        {
            if(cell_contains_mats(matlist[j][i], mats, nmats))
            {
                if(cell_is_mixed(matlist[j][i]))
                {
                    for(m = 0; m < 3; ++m)
                    {
                        if(matlist[j][i][m] != 0)
                            f[idx++] = matlist[j][i][m];
                    }
                }
            }
        }
    }
    *nTuples = idx;
    return f;
}

/******************************************************************************
 *
 * Purpose: Create a vector from a scalar, copying each component.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

int *
makevector(const int *data, int nTuples, int nComponents)
{
    int i, j, *vec, *ptr;
    ptr = vec = (int *)malloc(nTuples * nComponents * sizeof(int));
    for(i = 0; i < nTuples; ++i)
    {
        for(j = 0; j < nComponents; ++j)
        {
            *ptr++ = data[i];
            printf("%d,", data[i]);
        }
    }
    return vec;
}

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
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
        visit_handle mat = VISIT_INVALID_HANDLE;

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
            VisIt_MeshMetaData_setNumDomains(mmd, 1);
            VisIt_MeshMetaData_setDomainTitle(mmd, "Domains");
            VisIt_MeshMetaData_setDomainPieceName(mmd, "domain");
            VisIt_MeshMetaData_setXUnits(mmd, "cm");
            VisIt_MeshMetaData_setYUnits(mmd, "cm");
            VisIt_MeshMetaData_setXLabel(mmd, "Width");
            VisIt_MeshMetaData_setYLabel(mmd, "Height");

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
       
        /* Add a material */
        if(VisIt_MaterialMetaData_alloc(&mat) == VISIT_OKAY)
        {
            VisIt_MaterialMetaData_setName(mat, "Material");
            VisIt_MaterialMetaData_setMeshName(mat, "mesh2d");
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[0]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[1]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[2]);

            VisIt_SimulationMetaData_addMaterial(md, mat);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "scalar");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add some scalars that only exist on certain materials.*/
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar_on_mat1");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[0]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar_on_mat2");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[1]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar_on_mat3");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[2]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_scalar_on_mats12");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[0]);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[1]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "zonal_vector_on_mats12");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_VECTOR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[0]);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[1]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_scalar_on_mat1");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[0]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_scalar_on_mat2");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[1]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_scalar_on_mat3");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[2]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "nodal_scalar_on_mats23");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh2d");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[1]);
            VisIt_VariableMetaData_addMaterialName(vmd, matNames[2]);

            VisIt_SimulationMetaData_addVariable(md, vmd);
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

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
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
        if(VisIt_RectilinearMesh_alloc(&h) == VISIT_OKAY)
        {
            int i;
            visit_handle x,y;

            /* Initialize X coords. */
            for(i = 0; i < NX; ++i)
            {
                float t = (float)i / (float)(NX-1);
                rmesh_x[i] = (1.f-t)*XMIN + t*XMAX;
            }
            /* Initialize Y coords. */
            for(i = 0; i < NY; ++i) 
            {
                float t = (float)i / (float)(NY-1);
                rmesh_y[i] = (1.f-t)*YMIN + t*YMAX;
            }

            VisIt_VariableData_alloc(&x);
            VisIt_VariableData_alloc(&y);
            VisIt_VariableData_setDataF(x, VISIT_OWNER_SIM, 1, NX, rmesh_x);
            VisIt_VariableData_setDataF(y, VISIT_OWNER_SIM, 1, NY, rmesh_y);
            VisIt_RectilinearMesh_setCoordsXY(h, x, y);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns material data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMaterial(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    /* Allocate a VisIt_MaterialData */
    if(VisIt_MaterialData_alloc(&h) == VISIT_OKAY)
    {
        int i, j, m, cell = 0, arrlen = 0;
        int nmats, cellmat[10], matnos[3]={0,0,0};
        float cellmatvf[10];

        /* Tell the object we'll be adding cells to it using add*Cell functions */
        VisIt_MaterialData_appendCells(h, (NX-1)*(NY-1));

        /* Fill in the VisIt_MaterialData */
        VisIt_MaterialData_addMaterial(h, matNames[0], &matnos[0]);
        VisIt_MaterialData_addMaterial(h, matNames[1], &matnos[1]);
        VisIt_MaterialData_addMaterial(h, matNames[2], &matnos[2]);

        for(j = 0; j < NY-1; ++j)
        {
            for(i = 0; i < NX-1; ++i, ++cell)
            {
                nmats = 0;
                for(m = 0; m < 3; ++m)
                {
                    if(matlist[j][i][m] > 0)
                    {
                        cellmat[nmats] = matnos[matlist[j][i][m] - 1];
                        cellmatvf[nmats] = mat_vf[j][i][m];
                        nmats++;
                    }
                }        
                if(nmats > 1)
                    VisIt_MaterialData_addMixedCell(h, cell, cellmat, cellmatvf, nmats);
                else
                    VisIt_MaterialData_addCleanCell(h, cell, cellmat[0]);
            }
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

/* Make a constant field of 1. on the mesh. We'll break up the 1.0 into
 * pieces in the mixvar.
 */
#define C1 1.f
#define C2 2.f
#define C3 3.f
#define C4 4.f
float zonal_scalar[NY-1][NX-1] = {
{C1*1.f, C2*1.f, C3*1.f, C4*1.f},
{C1*1.f, C2*1.f, C3*1.f, C4*1.f},
{C1*1.f, C2*1.f, C3*1.f, C4*1.f}
};

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        if(strcmp(name, "scalar") == 0)
        {
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                (NX-1) * (NY-1), &zonal_scalar[0][0]); 
        }
        else if(strstr(name, "_scalar_on") != NULL)
        {
            int nTuples = 0;
            int *data = NULL;
            if(strcmp(name, "zonal_scalar_on_mat1") == 0)
                data = cell_number_for_cells_with_mat(1, &nTuples);
            else if(strcmp(name, "zonal_scalar_on_mat2") == 0)
                data = cell_number_for_cells_with_mat(2, &nTuples);
            else if(strcmp(name, "zonal_scalar_on_mat3") == 0)
                data = cell_number_for_cells_with_mat(3, &nTuples);
            else if(strcmp(name, "zonal_scalar_on_mats12") == 0)
            {
                int mats[] = {1,2};
                data = mat_number_for_cells_with_mats(mats, 2, &nTuples);
            }
            else if(strcmp(name, "nodal_scalar_on_mat1") == 0)
            {
                int mats = 1;
                data = node_number_for_cells_with_mats(&mats, 1, &nTuples);
            }
            else if(strcmp(name, "nodal_scalar_on_mat2") == 0)
            {
                int mats = 2;
                data = node_number_for_cells_with_mats(&mats, 1, &nTuples);
            }
            else if(strcmp(name, "nodal_scalar_on_mat3") == 0)
            {
                int mats = 3;
                data = node_number_for_cells_with_mats(&mats, 1, &nTuples);
            }
            else if(strcmp(name, "nodal_scalar_on_mats23") == 0)
            {
                int mats[] = {2,3};
                data = node_number_for_cells_with_mats(mats, 2, &nTuples);
            }
            VisIt_VariableData_setDataI(h, VISIT_OWNER_VISIT, 1, nTuples, data);
        }
        else if(strcmp(name, "zonal_vector_on_mats12") == 0)
        {
            int mats[] = {1,2};
            int i, *vec = NULL;
            int nTuples = 0, nComps = 2;
            int *data = mat_number_for_cells_with_mats(mats, 2, &nTuples);
            vec = makevector(data, nTuples, nComps);
            free(data);
            data = vec;
            VisIt_VariableData_setDataI(h, VISIT_OWNER_VISIT, nComps, nTuples, data);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns mixed scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Jul 19 14:54:30 PDT 2012
 *
 * Modifications:
 *
 *****************************************************************************/

/* The mesh has some mixed cells. Mixed scalars are only defined for
 * those mixed cells. That means that clean cells have no entries
 * in the mixed scalar array. For this example, we're reproducing the
 * volume fractions for the mixed cells as the mixvar data. All cells in
 * mesh have a comment indicating their location, even if they add no data.
 * Cells that have data provide their data after the comment. See the
 * mat_vf array in the GetMaterial function to draw comparisons.
 */
float mixvar[] = {
/*cell 0,0*/ /*cell 0,1*/ C2*0.75,C2*0.25,  /*cell 0,2*/ C3*0.8125,C3*0.1875, /*cell 0,3*/
/*cell 1,0*/ /*cell 1,1*/ C2*0.625,C2*0.375,/*cell 1,2*/ C3*0.5625,C3*0.4375, /*cell 1,3*/
/*cell 2,0*/ /*cell 2,1*/ C2*0.3,C2*0.7,    /*cell 2,2*/ C3*0.2,C3*0.4,C3*0.4,   /*cell 2,3*/C4*0.55,C4*0.45
};

visit_handle
SimGetMixedVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "scalar") == 0)
    {
        if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
        {
            int nTuples = sizeof(mixvar) / sizeof(float);
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
                nTuples, mixvar);
        }
    }
    else if(strcmp(name, "zonal_scalar_on_mats12") == 0)
    {
        if(VisIt_VariableData_alloc(&h) == VISIT_OKAY)
        {
            int mats[] = {1,2};
            int nTuples = 0;
            int *data = mat_numbers_for_cells_with_mats(mats, 2, &nTuples);
            VisIt_VariableData_setDataI(h, VISIT_OWNER_VISIT, 1,
                nTuples, data);
        }
    }

    return h;
}

