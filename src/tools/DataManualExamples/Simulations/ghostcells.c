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

#define FREE(ptr) if(ptr != NULL){free(ptr); ptr = NULL;}

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetCurve(const char *name, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Mesh functions
 ******************************************************************************/

/* Quad mesh */
typedef struct
{
    int   dims[2];
    float extents[4];
    float *x;
    float *y;
    int   *ghostCells;
    float *data;
} quadmesh_2d;

void
quadmesh_2d_ctor(quadmesh_2d *m)
{
    memset(m, 0, sizeof(quadmesh_2d));
}

void
quadmesh_2d_dtor(quadmesh_2d *m)
{
    FREE(m->x);
    FREE(m->y);
    FREE(m->ghostCells);
    FREE(m->data);
}

typedef quadmesh_2d rectmesh_2d;
typedef quadmesh_2d curvmesh_2d;

/* Quad mesh with real min/max indices for multidomain index-based ghost cells. */
typedef struct
{
    rectmesh_2d m;
    int         minReal[3];
    int         maxReal[3];
} rectmesh_idx_2d;

void
rectmesh_idx_2d_ctor(rectmesh_idx_2d *m)
{
    memset(m, 0, sizeof(rectmesh_idx_2d));
}

void
rectmesh_idx_2d_dtor(rectmesh_idx_2d *m)
{
    quadmesh_2d_dtor(&m->m);
}

/* Unstructured mesh */
typedef struct
{
    int   nnodes;
    int   ncells;
    float extents[4];
    float *xyz;
    int   connectivityLen;
    int   *connectivity;
    int   *ghostCells;
    float *data;
} ucdmesh_2d;

void
ucdmesh_2d_ctor(ucdmesh_2d *m)
{
    memset(m, 0, sizeof(ucdmesh_2d));
}

void
ucdmesh_2d_dtor(ucdmesh_2d *m)
{
    FREE(m->xyz);
    FREE(m->connectivity);
    FREE(m->ghostCells);
    FREE(m->data);
}

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

    rectmesh_2d     blankRectMesh;
    curvmesh_2d     blankCurvMesh;
    ucdmesh_2d      blankUcdMesh;
    rectmesh_idx_2d multidomain[9];
} simulation_data;

void
simulation_data_ctor(simulation_data *sim)
{
    int i;

    sim->par_rank = 0;
    sim->par_size = 1;
    sim->cycle = 0;
    sim->time = 0.;
    sim->runMode = SIM_STOPPED;
    sim->done = 0;

    quadmesh_2d_ctor(&sim->blankRectMesh);
    quadmesh_2d_ctor(&sim->blankCurvMesh);
    ucdmesh_2d_ctor(&sim->blankUcdMesh);

    for(i = 0; i < 9; ++i)
        rectmesh_idx_2d_ctor(&sim->multidomain[i]);
}

void
simulation_data_dtor(simulation_data *sim)
{
    int i;

    quadmesh_2d_dtor(&sim->blankRectMesh);
    quadmesh_2d_dtor(&sim->blankCurvMesh);
    ucdmesh_2d_dtor(&sim->blankUcdMesh);

    for(i = 0; i < 9; ++i)
        rectmesh_idx_2d_dtor(&sim->multidomain[i]);
}

const char *cmd_names[] = {"halt", "step", "run"};

/******************************************************************************
 * Functions to really populate data
 ******************************************************************************/

/******************************************************************************
 *
 * Purpose: Initialize a rectiliner mesh, saving its dims, create its coords.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
rectmesh_2d_create(rectmesh_2d *m, int nx, int ny, 
    float xmin, float xmax, float ymin, float ymax)
{
    int i;
    float t;

    m->dims[0] = nx;
    m->dims[1] = ny;
    m->extents[0] = xmin;
    m->extents[1] = xmax;
    m->extents[2] = ymin;
    m->extents[3] = ymax;
    m->x = (float *)malloc(m->dims[0] * sizeof(float));
    m->y = (float *)malloc(m->dims[1] * sizeof(float));
    for(i = 0; i < m->dims[0]; ++i)
    {
        t = ((float)i) / ((float)(m->dims[0]-1));
        m->x[i] = m->extents[0] + 
            t * (m->extents[1] - m->extents[0]);
    }
    for(i = 0; i < m->dims[1]; ++i)
    {
        t = ((float)i) / ((float)(m->dims[1]-1));
        m->y[i] = m->extents[2] + 
            t * (m->extents[3] - m->extents[2]);
    }
}

/******************************************************************************
 *
 * Purpose: Initialize a curviliner mesh, saving its dims, create its coords.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
curvmesh_2d_create(curvmesh_2d *m, int nx, int ny, 
    float xmin, float xmax, float ymin, float ymax)
{
    int i, j, idx = 0;

    m->dims[0] = nx;
    m->dims[1] = ny;
    m->extents[0] = xmin;
    m->extents[1] = xmax;
    m->extents[2] = ymin;
    m->extents[3] = ymax;
    m->x = (float *)malloc(m->dims[0] * m->dims[1] * sizeof(float));
    m->y = (float *)malloc(m->dims[0] * m->dims[1] * sizeof(float));
    for(j = 0; j < m->dims[1]; ++j)
    {
        float ty, y;
        ty = ((float)j) / ((float)(m->dims[1]-1));
        y = m->extents[2] + 
            ty * (m->extents[3] - m->extents[2]);
        for(i = 0; i < m->dims[0]; ++i, idx++)
        {
            float tx, x;
            tx = ((float)i) / ((float)(m->dims[0]-1));
            x = m->extents[0] + 
                tx * (m->extents[1] - m->extents[0]);
            m->x[idx] = x;
            m->y[idx] = y;
        }
    }
}

/******************************************************************************
 *
 * Purpose: Create a grid as an unstructured mesh of quads.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
ucdmesh_2d_create(ucdmesh_2d *m, int nx, int ny, 
    float xmin, float xmax, float ymin, float ymax)
{
    int i, j, idx = 0;

    m->nnodes = nx * ny;
    m->ncells = (nx-1) * (ny-1);
    m->extents[0] = xmin;
    m->extents[1] = xmax;
    m->extents[2] = ymin;
    m->extents[3] = ymax;
    m->xyz = (float *)malloc(3 * m->nnodes * sizeof(float));
    for(j = 0; j < ny; ++j)
    {
        float ty, y;
        ty = ((float)j) / ((float)(ny-1));
        y = m->extents[2] + 
            ty * (m->extents[3] - m->extents[2]);
        for(i = 0; i < nx; ++i, idx += 3)
        {
            float tx, x;
            tx = ((float)i) / ((float)(nx-1));
            x = m->extents[0] + 
                tx * (m->extents[1] - m->extents[0]);
            m->xyz[idx]   = x;
            m->xyz[idx+1] = y;
            m->xyz[idx+2] = 0.f;
        }
    }

    idx = 0;
    m->connectivityLen = 5 * m->ncells;
    m->connectivity = (int *)malloc(m->connectivityLen * sizeof(int));
    for(j = 0; j < ny-1; ++j)
    {
        for(i = 0; i < nx-1; ++i, idx += 5)
        {
            m->connectivity[idx] = VISIT_CELL_QUAD;
            m->connectivity[idx+1] = j * nx + i;
            m->connectivity[idx+2] = j * nx + i + 1;
            m->connectivity[idx+3] = (j+1) * nx + i + 1;
            m->connectivity[idx+4] = (j+1) * nx + i;
        }
    }
}

/******************************************************************************
 *
 * Purpose: Create some radial data based on the mesh coordinates.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
quadmesh_2d_create_radial_wave(quadmesh_2d *m, double time)
{
    int i, j;
    /* treat time as an angle */
    double ax, ay;
    ax = 2. * cos(time);
    ay = 2. * sin(time);
    for(j = 0; j < m->dims[1]-1; ++j)
    {
        double cy = (m->y[j] + m->y[j+1]) * 0.5;
        for(i = 0; i < m->dims[0]-1; ++i)
        {
            double cx, dist;
            cx = (m->x[i] + m->x[i+1]) * 0.5;
            dist = sqrt((cx-ax)*(cx-ax) + (cy-ay)*(cy-ay));
            m->data[j * (m->dims[0]-1) + i] = sin(dist / (2. * M_PI));
        }
    }
}

/******************************************************************************
 *
 * Purpose: Look for nodes that are only used by ghost cells and set their
 *          values to bogus values that will skew any extents calculation in
 *          VisIt if the node coordinates are used. This simulates having
 *          invalid coordinates for ghost nodes, which some codes have.
 *
 *          We will combat this by setting the mesh's extents in the metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
mess_up_ghost_coordinates(curvmesh_2d *m)
{
    int i,j;
    for(j = 0; j < m->dims[1]-1-1; ++j)
        for(i = 0; i < m->dims[0]-1-1; ++i)
        {
            int cellidx = j * (m->dims[0]-1) + i;
            if(m->ghostCells[cellidx] != 0 &&
               m->ghostCells[cellidx + 1] != 0)
            {
                int top = cellidx + (m->dims[0]-1);
                if(m->ghostCells[top] != 0)
                {
                    /* This cell abuts other ghosts to the right and top.
                     * Mess up its coordinates.
                     */
                    int nidx = (j+1) * m->dims[0] + (i+1);
                    m->y[nidx] = m->x[nidx] = 1.e8;
                }
            }
        }
}

/******************************************************************************
 *
 * Purpose: Create the meshes that we'll blank out using ghost cells.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
create_blanked_meshes(simulation_data *sim)
{
    int NX, NY, i, j, ncells;

    NX = 101;
    NY = 151;

    rectmesh_2d_create(&sim->blankRectMesh, NX, NY, 0.f, 10.f, 0.f, 15.f);
    /* Blank out some cells in the center/right of the mesh.*/
    ncells = (sim->blankRectMesh.dims[0]-1)*(sim->blankRectMesh.dims[1]-1);
    sim->blankRectMesh.ghostCells = (int *)malloc(ncells * sizeof(int));
    memset(sim->blankRectMesh.ghostCells, 0, ncells * sizeof(int));
    for(j = NY/3; j < (2*NY/3); ++j)
        for(i = NX/2; i < NX-1; ++i)
        {
            int idx = j * (sim->blankRectMesh.dims[0]-1) + i;
            sim->blankRectMesh.ghostCells[idx] = VISIT_GHOSTCELL_BLANK;
        }
    /*Create some data*/
    sim->blankRectMesh.data = (float *)malloc(ncells * sizeof(float));
    quadmesh_2d_create_radial_wave(&sim->blankRectMesh, sim->time);

    curvmesh_2d_create(&sim->blankCurvMesh, NX, NY, 0.f, 10.f, 0.f, 15.f);
    /* Blank out some cells in the center/right of the mesh.*/
    ncells = (sim->blankCurvMesh.dims[0]-1)*(sim->blankCurvMesh.dims[1]-1);
    sim->blankCurvMesh.ghostCells = (int *)malloc(ncells * sizeof(int));
    memset(sim->blankCurvMesh.ghostCells, 0, ncells * sizeof(int));
    for(j = NY/3; j < NY-1; ++j)
        for(i = NX/3; i < (2*NX/3); ++i)
        {
            int idx = j * (sim->blankCurvMesh.dims[0]-1) + i;
            sim->blankCurvMesh.ghostCells[idx] = VISIT_GHOSTCELL_BLANK;
        }
    /*Create some data*/
    sim->blankCurvMesh.data = (float *)malloc(ncells * sizeof(float));
    quadmesh_2d_create_radial_wave(&sim->blankCurvMesh, -sim->time);
    mess_up_ghost_coordinates(&sim->blankCurvMesh);

    ucdmesh_2d_create(&sim->blankUcdMesh, NX, NY, 0.f, 10.f, 0.f, 15.f);
    /* Blank out some cells in the lower left of the mesh.*/
    sim->blankUcdMesh.ghostCells = (int *)malloc(sim->blankUcdMesh.ncells * sizeof(int));
    memset(sim->blankUcdMesh.ghostCells, 0, sim->blankUcdMesh.ncells * sizeof(int));
    for(j = 5; j < NY/3; ++j)
        for(i = 5; i < NX/2; ++i)
        {
            int idx = j * (NX-1) + i;
            sim->blankUcdMesh.ghostCells[idx] = VISIT_GHOSTCELL_BLANK;
        }
    /*Create some data*/
    sim->blankUcdMesh.data = (float *)malloc(sim->blankUcdMesh.ncells * sizeof(float));
    memcpy(sim->blankUcdMesh.data, sim->blankRectMesh.data, sim->blankUcdMesh.ncells * sizeof(float));
}

/******************************************************************************
 *
 * Purpose: Create a mesh with 9 rectilinear domains. We expand the submeshes
 *          along their boundaries (where appropriate) to create a layer of
 *          ghost cells. We then create ghost cells 2 different ways so we can
 *          expose those as different meshes to test ghost cells via indexing
 *          and ghost cells via a ghost cell array.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
create_multidomain_mesh(simulation_data *sim)
{
    int i, ii, jj, *gc = NULL;

    /* Whether to add ghosts along a side {bottom,right,top,left} */
    static const int addGhosts[9][4] = {
        {0,1,1,0},{0,1,1,1},{0,0,1,1},
        {1,1,1,0},{1,1,1,1},{1,0,1,1},
        {1,1,0,0},{1,1,0,1},{1,0,0,1}
    };
    /*Per-domain extents {xmin,xmax,ymin,ymax} */
    static const float extents[9][4] = {
        {0.f,10.f,0.f,15.f}, {10.f,20.f,0.f,15.f}, {20.f,30.f,0.f,15.f},
        {0.f,10.f,15.f,30.f}, {10.f,20.f,15.f,30.f}, {20.f,30.f,15.f,30.f},
        {0.f,10.f,30.f,45.f}, {10.f,20.f,30.f,45.f}, {20.f,30.f,30.f,45.f}
    };
    const int NX = 21;
    const int NY = 31;

    for(i = 0; i < 9; ++i)
    {
        int ncells;
        float dX, dY;

        dX = ((extents[i][1] - extents[i][0]) / ((float)(NX-1))) / 3.f;
        dY = ((extents[i][3] - extents[i][2]) / ((float)(NY-1))) / 3.f;

        /* Create the domain mesh, adding a layer of cells where appropriate. */
        rectmesh_2d_create(&sim->multidomain[i].m, 
            NX + addGhosts[i][1] + addGhosts[i][3],
            NY + addGhosts[i][0] + addGhosts[i][2], 
            extents[i][0]/3.f - addGhosts[i][3] * dX,
            extents[i][1]/3.f + addGhosts[i][1] * dX,
            extents[i][2]/3.f - addGhosts[i][0] * dY,
            extents[i][3]/3.f + addGhosts[i][2] * dY);

        /* Ghost Cell Method I: Create ghost cells using indices to mark which 
         *                      cells in the arrays are real. Ghost cells are
         *                      the cells that are outside of the inclusive 
         *                      real cell boundaries we set here.
         */
        sim->multidomain[i].minReal[0] = 0 + addGhosts[i][3];
        sim->multidomain[i].minReal[1] = 0 + addGhosts[i][0];
        sim->multidomain[i].minReal[2] = 0;
        sim->multidomain[i].maxReal[0] = (sim->multidomain[i].m.dims[0]-1) - addGhosts[i][1];
        sim->multidomain[i].maxReal[1] = (sim->multidomain[i].m.dims[1]-1) - addGhosts[i][2];
        sim->multidomain[i].maxReal[2] = 0;

        /* Ghost Cell Method II: Create ghost cells along the edges using a 
         *                       cell-centered data array. All real cells are
         *                       zeroes and ghosts are interior boundaries.
         */
        ncells = (sim->multidomain[i].m.dims[0]-1)*(sim->multidomain[i].m.dims[1]-1);
        gc = sim->multidomain[i].m.ghostCells = (int *)malloc(ncells * sizeof(int));
        memset(sim->multidomain[i].m.ghostCells, 0, ncells * sizeof(int));
        for(jj = 0; jj < sim->multidomain[i].m.dims[1]-1; ++jj)
        {
            if((jj == 0 && addGhosts[i][0] == 1) ||
               (jj == sim->multidomain[i].m.dims[1]-1-1 && addGhosts[i][2] == 1))
            {
                for(ii = 0; ii < sim->multidomain[i].m.dims[0]-1; ++ii)
                    *gc++ = VISIT_GHOSTCELL_INTERIOR_BOUNDARY;
            }
            else
            {
                if(addGhosts[i][3] == 1)
                    gc[0] = VISIT_GHOSTCELL_INTERIOR_BOUNDARY;

                if(addGhosts[i][1] == 1)
                    gc[sim->multidomain[i].m.dims[0]-1-1] = VISIT_GHOSTCELL_INTERIOR_BOUNDARY;

                gc += sim->multidomain[i].m.dims[0]-1;
            }
        }

        /*Create some data*/
        sim->multidomain[i].m.data = (float *)malloc(ncells * sizeof(float));
        quadmesh_2d_create_radial_wave(&sim->multidomain[i].m, sim->time); 
    }
}

/******************************************************************************
 *
 * Purpose: Create the data that the simulation will use.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 14:59:38 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

void
read_input_deck(simulation_data *sim)
{
    create_blanked_meshes(sim);
    create_multidomain_mesh(sim);
}

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

void simulate_one_timestep(simulation_data *sim)
{
    int i, ncells;
    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%lg\n", sim->cycle, sim->time);

    quadmesh_2d_create_radial_wave(&sim->blankRectMesh, sim->time);
    quadmesh_2d_create_radial_wave(&sim->blankCurvMesh, -sim->time);
    memcpy(sim->blankUcdMesh.data, sim->blankRectMesh.data, sim->blankUcdMesh.ncells * sizeof(float));
    for(i = 0; i < 9; ++i)
        quadmesh_2d_create_radial_wave(&sim->multidomain[i].m, sim->time); 

    VisItTimeStepChanged();
    VisItUpdatePlots();
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
 * Date:       Fri Aug 12 15:05:36 PDT 2011
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
    char *env = NULL;
    simulation_data sim;
    simulation_data_ctor(&sim);

#ifdef PARALLEL
    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &sim.par_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &sim.par_size);
#endif

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction(visit_broadcast_int_callback);
    VisItSetBroadcastStringFunction(visit_broadcast_string_callback);
    /* Tell libsim whether the simulation is parallel. */
    VisItSetParallel(sim.par_size > 1);
    VisItSetParallelRank(sim.par_rank);
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

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(sim.par_rank == 0)
    {
        /* Write out .sim file that VisIt uses to connect. */
        VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "ghostcells_par",
#else
            "ghostcells",
#endif
            "Demonstrate ghost cells on different mesh types",
            "/path/to/where/sim/was/started",
            NULL, NULL, NULL);
    }

    /* Read input problem setup, geometry, data.*/
    read_input_deck(&sim);

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
 * Date:       Fri Aug 12 14:39:11 PDT 2011
 *
 * Modifications:
 *  Brad Whitlock, Wed Aug 17 12:26:56 PDT 2011
 *  Set spatial extents for curv_blank mesh.
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
        double extents[6];

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
            VisIt_MeshMetaData_setName(mmd, "rect_blank");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "curv_blank");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_CURVILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            /* This mesh's coordinates contain bad values for some of cells
             * that are ghosted out. Set the extents to make it so VisIt
             * will use these extents rather than looking over the coordinates.
             */
            extents[0] = sim->blankCurvMesh.extents[0];
            extents[1] = sim->blankCurvMesh.extents[1];
            extents[2] = sim->blankCurvMesh.extents[2];
            extents[3] = sim->blankCurvMesh.extents[3];
            extents[4] = 0.;
            extents[5] = 0.;
            VisIt_MeshMetaData_setSpatialExtents(mmd, extents);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "ucd_blank");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 1);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "multi_domain_index");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 9);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "multi_domain");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_RECTILINEAR);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 2);
            VisIt_MeshMetaData_setNumDomains(mmd, 9);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "rect_var");
            VisIt_VariableMetaData_setMeshName(vmd, "rect_blank");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "curv_var");
            VisIt_VariableMetaData_setMeshName(vmd, "curv_blank");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "ucd_var");
            VisIt_VariableMetaData_setMeshName(vmd, "ucd_blank");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "multi_var");
            VisIt_VariableMetaData_setMeshName(vmd, "multi_domain");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "multi_var_index");
            VisIt_VariableMetaData_setMeshName(vmd, "multi_domain_index");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

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
 * Date:       Fri Aug 12 14:44:54 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "rect_blank") == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int ncells;
            visit_handle hx, hy, gc;

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, sim->blankRectMesh.dims[0], sim->blankRectMesh.x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, sim->blankRectMesh.dims[1], sim->blankRectMesh.y);
            VisIt_RectilinearMesh_setCoordsXY(h, hx, hy);

            /* Attach ghost cells to blank out certain cells*/
            VisIt_VariableData_alloc(&gc);
            ncells = (sim->blankRectMesh.dims[0]-1) * (sim->blankRectMesh.dims[1]-1);
            VisIt_VariableData_setDataI(gc, VISIT_OWNER_SIM, 1, ncells, sim->blankRectMesh.ghostCells);
            VisIt_RectilinearMesh_setGhostCells(h, gc);
        }
    }
    else if(strcmp(name, "curv_blank") == 0)
    {
        if(VisIt_CurvilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int ncells, nnodes;
            visit_handle hx, hy, gc;

            nnodes = sim->blankCurvMesh.dims[0] * sim->blankCurvMesh.dims[1];
            ncells = (sim->blankCurvMesh.dims[0]-1) * (sim->blankCurvMesh.dims[1]-1);

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, nnodes, sim->blankCurvMesh.x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, nnodes, sim->blankCurvMesh.y);
            VisIt_CurvilinearMesh_setCoordsXY(h, sim->blankCurvMesh.dims, hx, hy);

            /* Attach ghost cells to blank out certain cells*/
            VisIt_VariableData_alloc(&gc);
            VisIt_VariableData_setDataI(gc, VISIT_OWNER_SIM, 1, ncells, sim->blankCurvMesh.ghostCells);
            VisIt_CurvilinearMesh_setGhostCells(h, gc);
        }
    }
    else if(strcmp(name, "ucd_blank") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxyz, hc, gc;

            VisIt_VariableData_alloc(&hxyz);
            VisIt_VariableData_setDataF(hxyz, VISIT_OWNER_SIM, 3, sim->blankUcdMesh.nnodes, sim->blankUcdMesh.xyz);
            VisIt_UnstructuredMesh_setCoords(h, hxyz);

            VisIt_VariableData_alloc(&hc);
            VisIt_VariableData_setDataI(hc, VISIT_OWNER_SIM, 1, sim->blankUcdMesh.connectivityLen,
                sim->blankUcdMesh.connectivity);
            VisIt_UnstructuredMesh_setConnectivity(h, sim->blankUcdMesh.ncells, hc);

            /* Attach ghost cells to blank out certain cells*/
            VisIt_VariableData_alloc(&gc);
            VisIt_VariableData_setDataI(gc, VISIT_OWNER_SIM, 1, sim->blankUcdMesh.ncells, sim->blankUcdMesh.ghostCells);
            VisIt_UnstructuredMesh_setGhostCells(h, gc);
        }
    }
    else if(strncmp(name, "multi_domain", 12) == 0)
    {
        if(VisIt_RectilinearMesh_alloc(&h) != VISIT_ERROR)
        {
            int ncells;
            visit_handle hx, hy, gc;

            VisIt_VariableData_alloc(&hx);
            VisIt_VariableData_alloc(&hy);
            VisIt_VariableData_setDataF(hx, VISIT_OWNER_SIM, 1, sim->multidomain[domain].m.dims[0], sim->multidomain[domain].m.x);
            VisIt_VariableData_setDataF(hy, VISIT_OWNER_SIM, 1, sim->multidomain[domain].m.dims[1], sim->multidomain[domain].m.y);
            VisIt_RectilinearMesh_setCoordsXY(h, hx, hy);

            if(strcmp(name, "multi_domain") == 0)
            {
                /* Do ghost cells using a ghost cells array. */
                VisIt_VariableData_alloc(&gc);
                ncells = (sim->multidomain[domain].m.dims[0]-1) * (sim->multidomain[domain].m.dims[1]-1);
                VisIt_VariableData_setDataI(gc, VISIT_OWNER_SIM, 1, ncells, sim->multidomain[domain].m.ghostCells);
                VisIt_RectilinearMesh_setGhostCells(h, gc);
            }
            else
            {
                /* Do ghost cells using indices. Note: other mesh types can also 
                 * designate ghost cells using indices.
                 */
                VisIt_RectilinearMesh_setRealIndices(h, sim->multidomain[domain].minReal, 
                    sim->multidomain[domain].maxReal);
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
 * Date:       Fri Aug 12 11:22:47 PDT 2011
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "rect_var") == 0)
    {
        int ncells;
        ncells = (sim->blankRectMesh.dims[0]-1) * (sim->blankRectMesh.dims[1]-1);
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            ncells, sim->blankRectMesh.data);
    }
    else if(strcmp(name, "curv_var") == 0)
    {
        int ncells;
        ncells = (sim->blankCurvMesh.dims[0]-1) * (sim->blankCurvMesh.dims[1]-1);
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            ncells, sim->blankCurvMesh.data);
    }
    else if(strcmp(name, "ucd_var") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            sim->blankUcdMesh.ncells, sim->blankUcdMesh.data);
    }
    else if(strncmp(name, "multi_var", 9) == 0)
    {
        int ncells;
        ncells = (sim->multidomain[domain].m.dims[0]-1) * (sim->multidomain[domain].m.dims[1]-1);
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            ncells, sim->multidomain[domain].m.data);
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Aug 12 11:22:47 PDT 2011
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
        int i, *iptr = NULL, dcount = 0;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(9 * sizeof(int));
        memset(iptr, 0, 9 * sizeof(int));

        for(i = 0; i < 9; i++)
        {
            int owner_of_domain = i % sim->par_size;
            if(sim->par_rank == owner_of_domain)
                iptr[dcount++] = i;
        }

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, dcount, iptr);
        VisIt_DomainList_setDomains(h, dcount, hdl);
    }
    return h;
}
