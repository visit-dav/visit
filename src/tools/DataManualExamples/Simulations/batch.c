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

#include <VisItControlInterface_V2.h>
#include <VisItDataInterface_V2.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "extract.h"

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Communication Function prototypes */
int SimBroadcastInt(int *value, int sender, void *cbdata);
int SimBroadcastString(char *str, int len, int sender, void *cbdata);
void SimSlaveProcessCallback(void *);

/* Data Adaptor Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/* Expression names and definitions. */
static const char *exprNames[] = {"nid", "zid", "xc", "yc", "zc", "radius"};
static const char *exprDefinitions[] = {"nodeid(mesh)", "zoneid(mesh)",
"coord(mesh)[0]", "coord(mesh)[1]", "coord(mesh)[2]", "sqrt(xc*xc+yc*yc+zc*zc)"};

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

typedef struct
{
#ifdef PARALLEL
    MPI_Comm par_comm;
#endif
    int      par_rank;
    int      par_size;
    int      cycle;
    double   time;
    int      dims[3];
    float    extents[6];
    float   *x;
    float   *y;
    float   *z;
    float   *q;
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->dims[0] = 50;
    sim->dims[1] = 50;
    sim->dims[2] = 50;
    sim->extents[0] = 0.f;
    sim->extents[1] = 10.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = 10.f;
    sim->extents[4] = 0.f;
    sim->extents[5] = 10.f;
    sim->x = NULL;
    sim->y = NULL;
    sim->z = NULL;
    sim->q = NULL;
}

void
simulation_data_dtor(simulation_data *sim)
{
    if(sim->x != NULL)
    {
        free(sim->x);
        sim->x = NULL;
    }
    if(sim->y != NULL)
    {
        free(sim->y);
        sim->y = NULL;
    }
    if(sim->z != NULL)
    {
        free(sim->z);
        sim->z = NULL;
    }
    if(sim->q != NULL)
    {
        free(sim->q);
        sim->q = NULL;
    }
}

void 
simulation_data_update(simulation_data *sim)
{
    float x,y,z,tx,ty,tz,offset;
    int index, i,j,k, npts, ncells;
    npts = sim->dims[0]*sim->dims[1]*sim->dims[2];
    ncells = (sim->dims[0]-1)*(sim->dims[1]-1)*(sim->dims[2]-1);

    if(sim->par_rank == 0)
    {
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);
        fflush(stdout);
    }

    /* Allocate the arrays */
    if(sim->x == NULL)
    {
        sim->x = (float *)malloc(sizeof(float) * npts);
        sim->y = (float *)malloc(sizeof(float) * npts);
        sim->z = (float *)malloc(sizeof(float) * npts);
        sim->q = (float *)malloc(sizeof(float) * ncells);

        /* Init coordinates. */
        offset = (sim->extents[1] - sim->extents[0]) * sim->par_rank;
        index = 0;
        for(k = 0; k < sim->dims[2]; ++k)
        {
            tz = ((float)k) / ((float)(sim->dims[2] - 1));
            z = (1.f-tz)*sim->extents[4] + tz*sim->extents[5];
            for(j = 0; j < sim->dims[1]; ++j)
            {
                float ty = ((float)j) / ((float)(sim->dims[1] - 1));
                y = (1.f-ty)*sim->extents[2] + ty*sim->extents[3];
                for(i = 0; i < sim->dims[0]; ++i)
                {
                    float tx = ((float)i) / ((float)(sim->dims[0] - 1));
                    x = (1.f-tx)*sim->extents[0] + tx*sim->extents[1];
                    sim->x[index] = x + offset;
                    sim->y[index] = y;
                    sim->z[index] = z;
                    ++index;
                }
            }
        }
    }
    /* Update q. */
    for(k = 0; k < sim->dims[2]-1; ++k)
    {
        for(j = 0; j < sim->dims[1]-1; ++j)
        {
            for(i = 0; i < sim->dims[0]-1; ++i)
            {
                int srcIndex, destIndex;
                srcIndex = k*(sim->dims[0])*(sim->dims[1]) +
                           j*(sim->dims[0]) +
                           i;
                destIndex = k*(sim->dims[0]-1)*(sim->dims[1]-1) +
                            j*(sim->dims[0]-1) +
                            i;

                sim->q[destIndex] = sin(sim->x[srcIndex] + sim->time);
            }
        }
    }
}

/******************************************************************************
 *
 * Function: mainloop_batch
 *
 * Purpose: The batch version of the main loop. It iterates and makes extracts.
 *
 * Programmer: Brad Whitlock
 * Date:      Tue Feb  3 13:10:23 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

void mainloop_batch(simulation_data *sim)
{
    char filebase[100];
    const char *extractvars[] = {"q", "xc", "radius", NULL};
    double origin[] = {5., 5., 5.}, normal[] = {0., 0.707, 0.707};
    double isos[] = {0.2, 0.5, 0.8};
    double v0[] = {1.,1.,1.}, v1[] = {5., 1.5, 7.}, v2[] = {8., 2., 5.};
    
    /* Explicitly load VisIt runtime functions and install callbacks. */
    VisItInitializeRuntime();
    VisItSetSlaveProcessCallback2(SimSlaveProcessCallback, (void*)sim);
    VisItSetGetMetaData(SimGetMetaData, (void*)sim);
    VisItSetGetMesh(SimGetMesh, (void*)sim);
    VisItSetGetVariable(SimGetVariable, (void*)sim);
    VisItSetGetDomainList(SimGetDomainList, (void*)sim);

    while(1)
    {  
        /* Update the simulation data for this iteration.*/     
        simulation_data_update(sim);

        /* Tell VisIt that some metadata changed.*/
        VisItTimeStepChanged();

        /* Make some extracts. */
        sprintf(filebase, "slice3v_%04d", sim->cycle);
        extract_slice_3v(filebase, v0, v1, v2, extractvars);

        sprintf(filebase, "sliceON_%04d", sim->cycle);
        extract_slice_origin_normal(filebase, origin, normal, extractvars);

        sprintf(filebase, "sliceX_%04d", sim->cycle);
        extract_slice(filebase, 0, 0.5, extractvars);

        sprintf(filebase, "sliceY_%04d", sim->cycle);
        extract_slice(filebase, 1, 2.5, extractvars);

        sprintf(filebase, "sliceZ_%04d", sim->cycle);
        extract_slice(filebase, 2, 5., extractvars);

        sprintf(filebase, "iso_%04d", sim->cycle);
        extract_iso(filebase, "q", isos, 3, extractvars);

        ++sim->cycle;
        sim->time += (M_PI / 10.);
    }
}

/******************************************************************************
 *
 * Purpose: This is the main function for the program.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:53:45 PST 2015
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
    int i;
    char *env = NULL;
    simulation_data sim;
    simulation_data_ctor(&sim);

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);

    /* Create a new communicator. */
    if (MPI_Comm_dup(MPI_COMM_WORLD, &sim.par_comm) != MPI_SUCCESS)
        sim.par_comm = MPI_COMM_WORLD;

    MPI_Comm_rank (sim.par_comm, &sim.par_rank);
    MPI_Comm_size (sim.par_comm, &sim.par_size);
#endif

    /* Check for command line arguments. */
    for(i = 1; i < argc; ++i)
    {
        if((i+1) < argc)
        {
            if(strcmp(argv[i], "-nx") == 0)
            {
                sscanf(argv[i+1], "%d", &sim.dims[0]);
                i++;
            }
            else if(strcmp(argv[i], "-ny") == 0)
            {
                sscanf(argv[i+1], "%d", &sim.dims[1]);
                i++;
            }
            else if(strcmp(argv[i], "-nz") == 0)
            {
                sscanf(argv[i+1], "%d", &sim.dims[2]);
                i++;
            }
            else if(strcmp(argv[i], "-dir") == 0)
            {
                /* Specify the path to VisIt installation. */
                VisItSetDirectory(argv[i+1]);
                i++;
            }
        }
    }

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction2(SimBroadcastInt, (void*)&sim);
    VisItSetBroadcastStringFunction2(SimBroadcastString, (void*)&sim);

    /* Tell libsim whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);

    /* Tell libsim which communicator to use. You must pass the address of
     * an MPI_Comm object.
     */
    VisItSetMPICommunicator((void *)&sim.par_comm);
#endif

    /* Only read the environment on rank 0. This could happen before MPI_Init if
     * we are using an MPI that does not like to let us spawn processes but we
     * would not know our processor rank.
     */
    if(sim.par_rank == 0)
        env = VisItGetEnvironment();

    /* Pass the environment to all other processors collectively. */
    VisItSetupEnvironment2(env);
    if(env != NULL)
        free(env);

    /* Call the main loop. */
    mainloop_batch(&sim);

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/*******************************************************************************
********************************************************************************
*****                           ADAPTOR FUNCTIONS
********************************************************************************
*******************************************************************************/

/* COMMUNICATION ADAPTOR FUNCTIONS. */

#ifdef PARALLEL
int SimBroadcastInt(int *value, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(value, 1, MPI_INT, sender, sim->par_comm);
}

int SimBroadcastString(char *str, int len, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(str, len, MPI_CHAR, sender, sim->par_comm);
}
#endif

void SimSlaveProcessCallback(void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    int command = VISIT_COMMAND_PROCESS;
#ifdef PARALLEL
    MPI_Bcast(&command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* DATA ADAPTOR FUNCTIONS */

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
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
        VisIt_SimulationMetaData_setMode(md, VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "mesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
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
            VisIt_VariableMetaData_setName(vmd, "q");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "xc");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "radius");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);
            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
#if 0
        /* Add expressions for VisIt to calculate. */
        for(i = 0; i < 6; ++i)
        {
            if(VisIt_ExpressionMetaData_alloc(&emd) == VISIT_OKAY)
            {
                VisIt_ExpressionMetaData_setName(emd, exprNames[i]);
                VisIt_ExpressionMetaData_setDefinition(emd, exprDefinitions[i]);
                VisIt_ExpressionMetaData_setType(emd, VISIT_VARTYPE_SCALAR);
                VisIt_SimulationMetaData_addExpression(md, emd);
            }
        }
#endif
    }

    return md;
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "mesh") == 0)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int npts;
            visit_handle hx, hy, hz;
            npts = sim->dims[0]*sim->dims[1]*sim->dims[2];
            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_alloc(&hz);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, npts, sim->x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, npts, sim->y);
            VisIt_VariableData_setDataF(hz, VISIT_OWNER_SIM, 1, npts, sim->z);
            VisIt_CurvilinearMesh_setCoordsXYZ(h, sim->dims, hx, hy, hz);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    int ncells, npts;
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    npts = sim->dims[0] * sim->dims[1] * sim->dims[2];
    ncells = (sim->dims[0]-1) * (sim->dims[1]-1) * (sim->dims[2]-1);

    if(strcmp(name, "q") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            ncells, sim->q);
    }
    else if(strcmp(name, "xc") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            npts, sim->x);
    }
    else if(strcmp(name, "radius") == 0)
    {
        int index;
        double *rad = NULL;
        VisIt_VariableData_alloc(&h);
        /* On the fly data generation. We donate the array to VisIt. */
        rad = (double *)malloc(npts * sizeof(double));
        for(index = 0; index < npts; ++index)
            rad[index] = sqrt(sim->x[index]*sim->x[index] + 
                              sim->y[index]*sim->y[index] + 
                              sim->z[index]*sim->z[index]);
        VisIt_VariableData_setDataD(h, VISIT_OWNER_VISIT, 1,
            npts, rad);
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list (the list of domains
 *          owned by this processor).
 *
 * Programmer: Brad Whitlock
 * Date:       Tue Feb  3 14:47:22 PST 2015
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
