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
#ifdef PARALLEL
#include <mpi.h>
#endif

#include "SimulationExample.h"

#define FREE(ptr) if(ptr != NULL){free(ptr); ptr = NULL;}

#define INT_CAST(VAL) ((int)(VAL))
#define FLOAT_CAST(VAL) ((float)(VAL))

#define MAXPARTS 3

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

#define SIM_STOPPED       0
#define SIM_RUNNING       1

/* Data Access Function prototypes */
visit_handle SimGetMetaData(void *);
visit_handle SimGetMesh(int, const char *, void *);
visit_handle SimGetMaterial(int, const char *, void *);
visit_handle SimGetVariable(int, const char *, void *);
visit_handle SimGetDomainList(const char *, void *);

/******************************************************************************
 * Mesh functions
 ******************************************************************************/

/* Unstructured mesh */
typedef struct
{
    int   nnodes;
    int   ncells;
    int   nodeDataSize;
    int   cellDataSize;

    float extents[6];
    float *xyz;
    int   connectivitySize;
    int   connectivityLen;
    int   *connectivity;
    int   *globalNodeIds;
    int   *globalCellIds;
    float *data;
    float *data2;
} ucdmesh;

void
ucdmesh_ctor(ucdmesh *m)
{
    memset(m, 0, sizeof(ucdmesh));
}

void
ucdmesh_dtor(ucdmesh *m)
{
    FREE(m->xyz);
    FREE(m->connectivity);
    FREE(m->globalNodeIds);
    FREE(m->globalCellIds);
    FREE(m->data);
    FREE(m->data2);
}

/******************************************************************************
 * Surface Mesh functions
 ******************************************************************************/

typedef ucdmesh surfacemesh;

#define SURFACEMESH_ALLOC_INCR 1000

void
surfacemesh_ctor(surfacemesh *m)
{
    memset(m, 0, sizeof(surfacemesh));

    m->nodeDataSize = SURFACEMESH_ALLOC_INCR;
    m->xyz = (float *)malloc(3 * sizeof(float) * m->nodeDataSize);
    m->globalNodeIds = (int *)malloc(sizeof(float) * m->nodeDataSize);

    m->connectivitySize = SURFACEMESH_ALLOC_INCR;
    m->connectivity = (int *)malloc(sizeof(int) * m->connectivitySize);

    m->cellDataSize = SURFACEMESH_ALLOC_INCR;
    m->globalCellIds = (int *)malloc(sizeof(int) * m->cellDataSize);
    m->data = (float *)malloc(sizeof(float) * m->cellDataSize);
    m->data2 = (float *)malloc(sizeof(float) * m->cellDataSize);
}

void
surfacemesh_dtor(surfacemesh *m)
{
    ucdmesh_dtor(m);
}

int
surfacemesh_addpoint(surfacemesh *m, float x, float y, float z, int id)
{
    if(m->nnodes+1 >= m->nodeDataSize)
    {
        m->nodeDataSize += SURFACEMESH_ALLOC_INCR;
        m->xyz = (float *)realloc(m->xyz, 3 * sizeof(float) * m->nodeDataSize);
        m->globalNodeIds = (int *)realloc(m->globalNodeIds, sizeof(int) * m->nodeDataSize);
    }

    m->xyz[m->nnodes*3  ] = x;
    m->xyz[m->nnodes*3+1] = y;
    m->xyz[m->nnodes*3+2] = z;
    m->globalNodeIds[m->nnodes] = id;
    m->nnodes++;

    return m->nnodes-1;
}

int
surfacemesh_addcell(surfacemesh *m, const int *conn, int nconn, float value, float value2, int id)
{
    int i, *ptr;

    if((m->connectivityLen + nconn) >= m->connectivitySize)
    {
        m->connectivitySize += SURFACEMESH_ALLOC_INCR;
        m->connectivity = (int *)realloc(m->connectivity, 3 * sizeof(int) * m->connectivitySize);
    }

    if(m->ncells+1 >= m->cellDataSize)
    {
        m->cellDataSize += SURFACEMESH_ALLOC_INCR;
        m->globalCellIds = (int *)realloc(m->globalCellIds, sizeof(int) * m->cellDataSize);
        m->data = (float *)realloc(m->data, sizeof(float) * m->cellDataSize);
        m->data2 = (float *)realloc(m->data2, sizeof(float) * m->cellDataSize);
    }

    ptr = m->connectivity + m->connectivityLen;
    for(i = 0; i < nconn; ++i)
        ptr[i] = conn[i];
    m->connectivityLen += nconn;

    m->globalCellIds[m->ncells] = id;
    m->data[m->ncells] = value;   
    m->data2[m->ncells] = value2;   
    m->ncells++;

    return m->ncells-1;
}

void
surfacemesh_extract(surfacemesh *m, surfacemesh *output, 
    int(*keepcell)(surfacemesh *, int, int *, int, void*), void *cbdata)
{
    int i, *conn, lconn, cell = 0, npts;
    int *old2new = (int *)malloc(sizeof(int) * m->nnodes);
    for(i = 0; i < m->nnodes; ++i)
        old2new[i] = -1;

    conn = m->connectivity;
    for(cell = 0; cell < m->ncells; ++cell)
    {
        if(conn[0] == VISIT_CELL_TRI)
            npts = 3;
        else if(conn[0] == VISIT_CELL_QUAD)
            npts = 4;
        else
            break;
        lconn = npts+1;
        if((*keepcell)(m, cell, conn, lconn, cbdata) > 0)
        {
            int newconn[5];
            newconn[0] = conn[0];
            for(i = 1; i < lconn; ++i)
            {
                int a = conn[i];
                if(old2new[a] == -1)
                {
                    old2new[a] = surfacemesh_addpoint(output,
                                     m->xyz[3*a],
                                     m->xyz[3*a+1],
                                     m->xyz[3*a+2],
                                     m->globalNodeIds[a]);
                }
                newconn[i] = old2new[a];
            }
            surfacemesh_addcell(output, newconn, lconn, m->data[cell], m->data2[cell], m->globalCellIds[cell]);
        }
        conn += lconn;
    }

    FREE(old2new);
}

void
surfacemesh_cell_center(surfacemesh *m, const int *conn, int lconn, float *X, float *Y, float *Z)
{
    float x = 0.f,y = 0.f,z = 0.f, npts;
    int i;
    /* Cell center */
    npts = FLOAT_CAST(lconn-1);
    for(i = 1; i < lconn; ++i)
    {
        x += m->xyz[3*conn[i]+0];
        y += m->xyz[3*conn[i]+1];
        z += m->xyz[3*conn[i]+2];
    }
    *X = x / npts;
    *Y = y / npts;
    *Z = z / npts;
}

void
surfacemesh_data_extents(surfacemesh *m, float *e0, float *e1)
{
    int i;
    float ext[2];
    ext[0] = ext[1] = (m->ncells>0) ? m->data[0] : 0.f;
    for(i = 1; i < m->ncells; ++i)
    {
        ext[0] = (m->data[i] < ext[0]) ? m->data[i] : ext[0];
        ext[1] = (m->data[i] > ext[1]) ? m->data[i] : ext[1];
    }
    *e0 = ext[0];
    *e1 = ext[1];
}

void
surfacemesh_print(surfacemesh *m)
{
    int i;
    printf("nnodes=%d\n", m->nnodes);
    printf("ncells=%d\n", m->ncells);
    printf("nodeDataSize=%d\n", m->nodeDataSize);
    printf("cellDataSize=%d\n", m->cellDataSize);
    printf("extents={%f,%f,%f,%f,%f,%f}\n", 
        m->extents[0], m->extents[1], m->extents[2], 
        m->extents[3], m->extents[4], m->extents[5]);
    printf("xyz = {\n");
    for(i = 0; i < m->nnodes; ++i)
        printf("%d: %f, %f, %f\n", i, m->xyz[3*i],m->xyz[3*i+1],m->xyz[3*i+2]);
    printf("}\n");
    printf("connectivitySize=%d\n", m->connectivitySize);
    printf("connectivityLen=%d\n", m->connectivityLen);
    printf("connectivity = {");
    for(i = 0; i < m->connectivityLen; ++i)
       printf("%d, ", m->connectivity[i]);
    printf("}\n");
    printf("globalNodeIds = {");
    for(i = 0; i < m->nnodes; ++i)
       printf("%d, ", m->globalNodeIds[i]);
    printf("}\n");
    printf("globalCellIds = {");
    for(i = 0; i < m->ncells; ++i)
       printf("%d, ", m->globalCellIds[i]);
    printf("}\n");
    printf("data = {");
    for(i = 0; i < m->ncells; ++i)
       printf("%f, ", m->data[i]);
    printf("}\n");
    printf("data2 = {");
    for(i = 0; i < m->ncells; ++i)
       printf("%f, ", m->data2[i]);
    printf("}\n");
}

/******************************************************************************
 * Functions to create 6.14 20-bit fixed point numbers that we can use to
 * encode XYZ points with less precision to make them more likely to match.
 ******************************************************************************/
int
float_to_fixed6_14(float x)
{
    return INT_CAST(x * pow(2.,14.)) & 0xfffff;
}

float
fixed6_14_to_float(int x)
{
    float x1;
    if((x & 0x80000) > 0)
    {
        int x2 = x | 0xfff80000;
        x1 = (FLOAT_CAST(x2) * pow(2, -14.));
    }
    else
        x1 = (FLOAT_CAST(x) * pow(2, -14.));
    return x1;
}

typedef unsigned long point_key;

point_key
encode_point(float x, float y, float z)
{
    unsigned long X = (unsigned long)float_to_fixed6_14(x);
    unsigned long Y = (unsigned long)float_to_fixed6_14(y);
    unsigned long Z = (unsigned long)float_to_fixed6_14(z);
    return (X << 40) | (Y << 20) | Z;
}

void
decode_point(point_key pt, float *x, float *y, float *z)
{
    *x = fixed6_14_to_float( (int)((pt >> 40) & 0xfffff) );
    *y = fixed6_14_to_float( (int)((pt >> 20) & 0xfffff) );
    *z = fixed6_14_to_float( (int)((pt)       & 0xfffff) );
}

int
point_key_equal(point_key k0, point_key k1)
{
    return k0 == k1;
}

/******************************************************************************
 * Functions to make a set of unique points
 ******************************************************************************/

#define UNIQUEPOINTS_ALLOC_INCR 1000

typedef struct
{
    int        capacity;
    int        size;
    point_key *data;
} unique_points;

void
unique_points_ctor(unique_points *p)
{
    p->capacity = UNIQUEPOINTS_ALLOC_INCR;
    p->size = 0;
    p->data = (point_key *)malloc(p->capacity * sizeof(point_key));
}

void
unique_points_dtor(unique_points *p)
{
    FREE(p->data);
}

int
unique_points_lookup(unique_points *p, point_key k)
{
    int i;
    for(i = 0; i < p->size; ++i)
    {
        if(point_key_equal(k, p->data[i]))
            return i;
    }
    return -1;
}

int
unique_points_insert(unique_points *p, point_key k)
{
    if(p->size+1 > p->capacity)
    {
        p->capacity += UNIQUEPOINTS_ALLOC_INCR;
        p->data = (point_key *)realloc(p->data, p->capacity * sizeof(point_key));
    }
    p->data[p->size++] = k;
    return p->size-1;
}

/******************************************************************************
 * Functions to make cylinder geometry
 ******************************************************************************/

void
ConstructDisk(surfacemesh *m, unique_points *p, const float center[3],
    float radius,  float degrees, int radialDiv, int verticalDiv, float height)
{
    int r,r1,r2,a,i,centerPt,nAngles,nRadii,ring1;
    float *tr, minRadius, mat;

    /* Add points */
    unique_points_insert(p, encode_point(center[0], center[1], height));
    surfacemesh_addpoint(m, center[0], center[1], height, m->nnodes);

    centerPt = m->nnodes-1;
    nAngles = INT_CAST(360. / degrees);
    nRadii = radialDiv;
    ring1 = m->nnodes;

    tr = (float *)malloc(nRadii * sizeof(float));
    for(r = 0; r < nRadii; ++r)
        tr[r] = FLOAT_CAST(r+1) / FLOAT_CAST(nRadii);
    minRadius = 0.;

    for(r = 0; r < nRadii; ++r)
    {
        float rad = (1.-tr[r])*minRadius + tr[r] * radius;

        for(a = 0; a < nAngles; ++a)
        {
            float ta,angle,x,y,z;
            ta = FLOAT_CAST(a) / FLOAT_CAST(nAngles);
            angle = ta * 2. * M_PI;
            x = center[0] + rad*cos(angle);
            y = center[1] + rad*sin(angle);
            z = height;

            unique_points_insert(p, encode_point(x,y,z));
            surfacemesh_addpoint(m, x,y,z, m->nnodes);
        } 
    }
    FREE(tr);

    /* Connect the points into triangles. */
    for(i = 0; i < nAngles; ++i)
    {
        int conn[4];
        conn[0] = VISIT_CELL_TRI;
        conn[1] = centerPt;
        conn[2] = centerPt+1+i;
        conn[3] = (i==nAngles-1) ? (centerPt+1) : (centerPt+2+i);
        surfacemesh_addcell(m, conn, 4, 0.f, 2.f, m->ncells);
    }

    /* Make quads */
    for(r = 0; r < nRadii-1; ++r)
    {
        r1 = nRadii/3;
        r2 = 2*nRadii/3;
        if(r < r2)
            mat = (r < r1) ? 2.f : 1.f;
        else 
            mat = 0.f;
        for(a = 0; a < nAngles; ++a)
        {
            int conn[5], nextA;

            nextA = (a == nAngles-1) ? 0 : (a+1);
            conn[0] = VISIT_CELL_QUAD;
            conn[1] = ring1 + (r * nAngles + a);
            conn[2] = ring1 + ((r+1) * nAngles + a);
            conn[3] = ring1 + ((r+1) * nAngles + nextA);
            conn[4] = ring1 + (r * nAngles + nextA);

            surfacemesh_addcell(m, conn, 5, 0.f, mat, m->ncells);
        }
    }
}

void
ConstructWalls(surfacemesh *m, unique_points *p, const float center[3],
    float radius, float degrees, int radialDiv, int verticalDiv,
    float z0, float z1)
{
    /* Add points */
    int r,a,nAngles,nDiv,*ptids,idx,id;
    float th,h,ta,angle,x,y,z;

    nAngles = (int)(360. / degrees);
    nDiv = verticalDiv;
    ptids = (int *)malloc(nAngles * nDiv * sizeof(int));
    idx = 0;
    for(r = 0; r < nDiv; ++r)
    {
        th = FLOAT_CAST(r) / FLOAT_CAST(nDiv-1);
        h = (1.-th)*z0 + th*z1;
        for(a = 0; a < nAngles; ++a)
        {
            point_key k;

            ta = FLOAT_CAST(a) / FLOAT_CAST(nAngles);
            angle = ta * 2. * M_PI;
            x = center[0] + radius*cos(angle);
            y = center[1] + radius*sin(angle);
            z = h;

            /* Make sure that points are unique */
            k = encode_point(x,y,z);
            id = unique_points_lookup(p, k);
            if(id == -1)
            {
                id = unique_points_insert(p,k);
                surfacemesh_addpoint(m, x,y,z, m->nnodes);
            }

            ptids[idx++] = id;
        } 
    }

    for(r = 0; r < nDiv-1; ++r)
    {
        for(a = 0; a < nAngles; ++a)
        {
            int conn[5], nextA;
            nextA = (a == nAngles-1) ? 0 : (a+1);
            conn[0] = VISIT_CELL_QUAD;
            conn[1] = ptids[ (r * nAngles + a) ];
            conn[2] = ptids[ ((r+1) * nAngles + a) ];
            conn[3] = ptids[ ((r+1) * nAngles + nextA) ];
            conn[4] = ptids[ (r * nAngles + nextA) ];
            surfacemesh_addcell(m, conn, 5, 0.f, 0.f, m->ncells);
        }
    }

    FREE(ptids);
}

float
compute_angle(float xyz[3], float center[3])
{
    float x,y,r,cosphi,angle;
    x = xyz[0]-center[0];
    y = xyz[1]-center[1];
    r = sqrt(x * x + y * y);
    cosphi = (r >= 1.e-10) ? x/r : 1.;
    angle  = acos(cosphi);
    if (y < 0.)
        angle = 2. * M_PI - angle;
    return angle;
}

void
compute_cell_angles(surfacemesh *m, float center[3])
{
    int *conn, lconn, cell = 0, npts;
    float xyz[3];

    conn = m->connectivity;
    for(cell = 0; cell < m->ncells; ++cell)
    {
        if(conn[0] == VISIT_CELL_TRI)
            npts = 3;
        else if(conn[0] == VISIT_CELL_QUAD)
            npts = 4;
        else
            break;
        lconn = npts+1;

        surfacemesh_cell_center(m, conn, lconn, &xyz[0], &xyz[1], &xyz[2]);
        m->data[cell] = compute_angle(xyz, center);
        conn += lconn;
    }
}

void
create_cylinder_surface(surfacemesh *m, const float *extents)
{
    float degrees = 5.f;
    int radialDiv = 20;
    int verticalDiv = 20;
    float z0,z1, center[3], radius, eps = 0.05f;

    unique_points p;
    unique_points_ctor(&p);

    center[0] = (extents[0] + extents[3]) / 2.f;
    center[1] = (extents[1] + extents[4]) / 2.f;
    center[2] = (extents[2] + extents[5]) / 2.f;
    z0 = extents[2] + eps;
    z1 = extents[5] - eps;

    radius = (extents[3] - center[0]) * 0.8f;

    /* Create ground*/
    ConstructDisk(m,&p, center, radius, degrees, radialDiv, verticalDiv, z0);

    /* Create ceiling*/
    ConstructDisk(m,&p, center, radius, degrees, radialDiv, verticalDiv, z1);

    /* Create wall*/
    ConstructWalls(m,&p, center, radius, degrees, radialDiv, verticalDiv, z0, z1);

    /* Compute an angle for each cell. */
    compute_cell_angles(m, center);

    unique_points_dtor(&p);
}

/******************************************************************************
 * Simulation data and functions
 ******************************************************************************/

typedef struct
{
#ifdef PARALLEL
    MPI_Comm        par_comm;
#endif
    int             par_rank;
    int             par_size;
    int             cycle;
    double          time;
    int             runMode;
    int             done;
    int             echo;

    int             nx,ny,nz;
    float           extents[6];
    int             ijk_split[3];
    int             owns[8];
    ucdmesh         domains[8];

    int             surface_npartitions;
    int             surface_partitions[8];
    surfacemesh    *surfaces;
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
    sim->echo = 0;

    sim->nx = 51;
    sim->ny = 51;
    sim->nz = 51;
    sim->extents[0] = 0.f;
    sim->extents[1] = 0.f;
    sim->extents[2] = 0.f;
    sim->extents[3] = 1.f;
    sim->extents[4] = 1.f;
    sim->extents[5] = 1.f;
    sim->ijk_split[0] = sim->nx / 2;
    sim->ijk_split[1] = sim->ny / 2;
    sim->ijk_split[2] = sim->nz / 2;

    for(i = 0; i < 8; ++i)
        ucdmesh_ctor(&sim->domains[i]);

    sim->surface_npartitions = 0;
    sim->surfaces = NULL;
}

void
simulation_data_dtor(simulation_data *sim)
{
    int i;

    for(i = 0; i < 8; ++i)
        ucdmesh_dtor(&sim->domains[i]);

    if(sim->surfaces != NULL)
    {
        for(i = 0; i < sim->surface_npartitions; ++i)
            surfacemesh_dtor(&sim->surfaces[i]);
        FREE(sim->surfaces);
    }
}

const int dom2i[8] = {0,1,0,1,0,1,0,1};
const int dom2j[8] = {0,0,1,1,0,0,1,1};
const int dom2k[8] = {0,0,0,0,1,1,1,1};

/* Update the "mesh" ucdmesh. The mesh gets split into octants, forming 8
 * domains. The split point moves around so we get domains that shrink and
 * grow.
 */
void
simulation_data_update_ucdmesh(simulation_data *sim)
{
    float angle,rad,x,y,z,cx,cy,cz,dx,dy,dz,sx,sy,sz;
    int d,i,j,k, idx, ijk_start[3], ijk_end[3],nx,ny,nz;

    /* Get rid of the old meshes. */
    for(i = 0; i < 8; ++i)
        ucdmesh_dtor(&sim->domains[i]);

    angle = sim->time;

    /* We're going to move the ijk_split node around to determine where the
     * domain split occurs in the mesh.
     */
    dx = sim->extents[3] - sim->extents[0];
    dy = sim->extents[4] - sim->extents[1];
    dz = sim->extents[5] - sim->extents[2];
    rad = sqrt(dx*dx + dz*dz) / 10.;
    sx = ((sim->extents[0] + sim->extents[3]) / 2) + rad * cos(angle);
    sy = ((sim->extents[1] + sim->extents[4]) / 2) + rad * sin(angle);
    sz = ((sim->extents[2] + sim->extents[5]) / 2);

    /* We have a new x,y,z. Figure out where that is in ijk. */
    i = (int)(sim->nx * ((sx - sim->extents[0]) / dx));
    j = (int)(sim->ny * ((sy - sim->extents[1]) / dy));
    k = (int)(sim->nz * ((sz - sim->extents[2]) / dz));
    sim->ijk_split[0] = i;
    sim->ijk_split[1] = j;
    sim->ijk_split[2] = k;

    /* Now, make new domains according to the ijk split. */
    cx = dx / ((float)(sim->nx-1));
    cy = dy / ((float)(sim->ny-1));
    cz = dz / ((float)(sim->nz-1));
    for(d = 0; d < 8; ++d)
    {
        ucdmesh *dom = &sim->domains[d];

        /* See if this processor owns the domain. */
        if(sim->owns[d] == 0)
            continue;

        /* Figure node boundaries for this domain within "whole".*/
        ijk_start[0] = (dom2i[d] == 0) ? 0 : sim->ijk_split[0];
        ijk_start[1] = (dom2j[d] == 0) ? 0 : sim->ijk_split[1];
        ijk_start[2] = (dom2k[d] == 0) ? 0 : sim->ijk_split[2];
        ijk_end[0]   = (dom2i[d] == 0) ? sim->ijk_split[0] : (sim->nx-1);
        ijk_end[1]   = (dom2j[d] == 0) ? sim->ijk_split[1] : (sim->ny-1);
        ijk_end[2]   = (dom2k[d] == 0) ? sim->ijk_split[2] : (sim->nz-1);

        /* Extents for this domain. */
        dom->extents[0] = ((float)ijk_start[0]) * cx;
        dom->extents[1] = ((float)ijk_start[1]) * cy;
        dom->extents[2] = ((float)ijk_start[2]) * cz;
        dom->extents[3] = ((float)ijk_end[0]) * cx;
        dom->extents[4] = ((float)ijk_end[1]) * cy;
        dom->extents[5] = ((float)ijk_end[2]) * cz;

        /* nnodes for this domain*/
        nx = ijk_end[0] - ijk_start[0] + 1;
        ny = ijk_end[1] - ijk_start[1] + 1;
        nz = ijk_end[2] - ijk_start[2] + 1;

        /* Populate mesh and data for this domain. */
        dom->nnodes = nx * ny * nz;
        dom->ncells = (nx-1)*(ny-1)*(nz-1);
        dom->xyz = (float *)malloc(3 * dom->nnodes * sizeof(float));
        dom->connectivityLen = 9 * dom->ncells;
        dom->connectivity = (int *)malloc(dom->connectivityLen * sizeof(int));
        dom->globalNodeIds = (int *)malloc(dom->nnodes * sizeof(int));
        dom->globalCellIds = (int *)malloc(dom->ncells * sizeof(int));
        dom->data = (float *)malloc(dom->nnodes * sizeof(float));
        idx = 0;
        for(k = ijk_start[2]; k <= ijk_end[2]; ++k)
        {
            z = sim->extents[2] + ((float)k) * cz;
            for(j = ijk_start[1]; j <= ijk_end[1]; ++j)
            {
                y = sim->extents[1] + ((float)j) * cy;
                for(i = ijk_start[0]; i <= ijk_end[0]; ++i)
                {
                    x = sim->extents[0] + ((float)i) * cx;

                    dom->globalNodeIds[idx] = k*sim->nx*sim->ny + j*sim->nx + i;

                    dom->xyz[3*idx+0] = x;
                    dom->xyz[3*idx+1] = y;
                    dom->xyz[3*idx+2] = z;

                    dom->data[idx] = sqrt((sx-x)*(sx-x) + (sy-y)*(sy-y) + (sz-z)*(sz-z));

                    ++idx;
                }
            }
        }

#define LOCAL_NODE(I,J,K) ((K)*nx*ny + (J)*nx + (I))
        /* Connectivity, etc */
        idx = 0;
        for(k = 0; k < nz-1; ++k)
        {
            for(j = 0; j < ny-1; ++j)
            {
                for(i = 0; i < nx-1; ++i)
                {
                    /*local cell connectivity*/
                    int ii,jj,kk, idx9 = idx*9;
                    dom->connectivity[idx9+0] = VISIT_CELL_HEX;
                    dom->connectivity[idx9+1] = LOCAL_NODE(i  ,j  ,k+1);
                    dom->connectivity[idx9+2] = LOCAL_NODE(i+1,j  ,k+1);
                    dom->connectivity[idx9+3] = LOCAL_NODE(i+1,j  ,k);
                    dom->connectivity[idx9+4] = LOCAL_NODE(i  ,j  ,k);
                    dom->connectivity[idx9+5] = LOCAL_NODE(i  ,j+1,k+1);
                    dom->connectivity[idx9+6] = LOCAL_NODE(i+1,j+1,k+1);
                    dom->connectivity[idx9+7] = LOCAL_NODE(i+1,j+1,k);
                    dom->connectivity[idx9+8] = LOCAL_NODE(i  ,j+1,k);

                    /* global cell id */
                    ii = i + ijk_start[0];
                    jj = j + ijk_start[1];
                    kk = k + ijk_start[2];
                    dom->globalCellIds[idx] = kk*(sim->nx-1)*(sim->ny-1) + jj*(sim->nx-1) + ii;

                    idx++;
                }
            }
        }
    }
}

/* Select a cell if it is in a box. */
int
cell_in_box(surfacemesh *m, int cell, int *conn, int lconn, void *args)
{
    float x = 0.f,y = 0.f,z = 0.f,*extents;
    int valid;
    surfacemesh_cell_center(m, conn, lconn, &x, &y, &z);
    extents = (float *)args;
    valid = (x >= extents[0] && x < extents[3]) &&
            (y >= extents[1] && y < extents[4]) &&
            (z >= extents[2] && z < extents[5]);

    return valid;
}

/* Select a cell if its angle is within range. */
int
cell_in_angle_range(surfacemesh *m, int cell, int *conn, int lconn, void *args)
{
    float *angles;
    int valid;
    angles = (float *)args;
    valid = (m->data[cell] >= angles[0] && m->data[cell] < angles[1]);

    return valid;
}

/* Make a cylinder surface and divide it up into pieces using the octants from
 * the other mesh. We then further divide up each octant using angular values
 * stored on the cells in order to make a variable number of domains per
 * octant. We end up forcing the surface domains within an octant to be assigned
 * to the same processor. We can enforce this mapping within VisIt via the
 * domain list.
 */

void
simulation_data_update_surfacemesh(simulation_data *sim)
{
    int i, j, p, idx, idx2, np, *ncells, needclean;
    float t, angle[2], angle_ranges[MAXPARTS+1];
    surfacemesh surface, partial;

    /* Clean up previous surfaces */
    for(i = 0; i < sim->surface_npartitions; ++i)
        surfacemesh_dtor(&sim->surfaces[i]);
    FREE(sim->surfaces);

    /* Determine the number of partitions for each surface.*/
    p = (sim->cycle % MAXPARTS) + 1;
    sim->surface_npartitions = 0;
    for(i = 0; i < 8; ++i)
    {
        sim->surface_partitions[i] = p;
        sim->surface_npartitions += p;
        if(p+1 > MAXPARTS)
            p = 1;
        else
            p++;
    }

    /* Allocate new storage */
    sim->surfaces = (surfacemesh *)malloc(sizeof(surfacemesh) * sim->surface_npartitions);
    memset(sim->surfaces, 0, sizeof(surfacemesh) * sim->surface_npartitions);

    /* Create initial surface */
    surfacemesh_ctor(&surface);
    create_cylinder_surface(&surface, sim->extents);

    /* Partition spatially. */
    idx = 0;
    ncells = (int *)malloc(sizeof(int) * sim->surface_npartitions);
    memset(ncells, 0, sizeof(int) * sim->surface_npartitions);
    for(i = 0; i < 8; ++i)
    {
        if(sim->owns[i])
        {
            surfacemesh_ctor(&partial);
            surfacemesh_extract(&surface, &partial,
                                cell_in_box, sim->domains[i].extents);

            if(sim->surface_partitions[i] == 1)
            {
                /* Steal the partial mesh.*/
                memcpy(&sim->surfaces[idx], &partial, sizeof(surfacemesh));
                memset(&partial, 0, sizeof(surfacemesh));
                ncells[idx] = sim->surfaces[idx].ncells;
                idx++;
            }
            else
            {
                /* Partition among angles. */
                surfacemesh_data_extents(&partial, &angle[0], &angle[1]);
                for(j = 0; j < sim->surface_partitions[i]+1; ++j)
                {
                    t = FLOAT_CAST(j) / FLOAT_CAST(sim->surface_partitions[i]);
                    angle_ranges[j] = (1.-t)*angle[0] + t*angle[1];
                }
                angle_ranges[0] -= M_PI/2.f;
                angle_ranges[sim->surface_partitions[i]] += M_PI/2.f;
                np = sim->surface_partitions[i];
                for(j = 0; j < np; ++j)
                {
                    surfacemesh_ctor(&sim->surfaces[idx]);
                    surfacemesh_extract(&partial, &sim->surfaces[idx],
                                        cell_in_angle_range, &angle_ranges[j]);

                    ncells[idx] = sim->surfaces[idx].ncells;
                    idx++;
                }

                surfacemesh_dtor(&partial);
            }
        }
        else
        {
            for(j = 0; j < sim->surface_partitions[i]; ++j)
                ncells[idx++] = 0;
        }
    }

    surfacemesh_dtor(&surface);

#ifdef PARALLEL
    /* Sum the ncells array across processors. */
    {
    int *ncells2 = (int *)malloc(sizeof(int) * sim->surface_npartitions);
    MPI_Allreduce(ncells, ncells2, sim->surface_npartitions, MPI_INT, MPI_SUM, sim->par_comm);
    FREE(ncells);
    ncells = ncells2;
    }
#endif

    /* Get rid of any empty meshes. */
    needclean = 0;
    for(i = 0; i < sim->surface_npartitions; ++i)
        needclean |= (ncells[i] == 0);
    if(needclean)
    {
        surfacemesh *newsurf = (surfacemesh *)malloc(sizeof(surfacemesh) * sim->surface_npartitions);
        memset(newsurf, 0, sizeof(surfacemesh) * sim->surface_npartitions);

        idx = 0; idx2 = 0;
        for(i = 0; i < 8; ++i)
        {
            np = sim->surface_partitions[i];
            for(j = 0; j < np; ++j)
            {
                if(ncells[idx] == 0)
                {
                    sim->surface_partitions[i]--;
                    sim->surface_npartitions--;
                    surfacemesh_dtor(&sim->surfaces[idx]);
                }
                else
                {
                    memcpy(&newsurf[idx2], &sim->surfaces[idx], sizeof(surfacemesh));
                    ++idx2;
                }
                ++idx;
            }
        }

        FREE(sim->surfaces);
        sim->surfaces = newsurf;
    }

    FREE(ncells);
}

/* Update the simulation data for the current time step. */
void
simulation_data_update(simulation_data *sim)
{
    int i;
    /* Determine which meshes we'll work on based on the rank. */
    for(i = 0; i < 8; i++)
    {
        int owner_of_domain = i % sim->par_size;
        if(sim->par_rank == owner_of_domain)
            sim->owns[i] = 1;
        else
            sim->owns[i] = 0;
    }

    simulation_data_update_ucdmesh(sim);
    simulation_data_update_surfacemesh(sim);
}


const char *cmd_names[] = {"halt", "step", "run"};

/******************************************************************************
 ******************************************************************************
 ***
 *** EVENT PROCESSING FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

void simulate_one_timestep(simulation_data *sim)
{
    ++sim->cycle;
    sim->time += (M_PI / 10.);

    if(sim->par_rank == 0)
        printf("Simulating time step: cycle=%d, time=%g\n", sim->cycle, sim->time);

    simulation_data_update(sim);

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
static int visit_broadcast_int_callback(int *value, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(value, 1, MPI_INT, sender, sim->par_comm);
}

static int visit_broadcast_string_callback(char *str, int len, int sender, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return MPI_Bcast(str, len, MPI_CHAR, sender, sim->par_comm);
}
#endif


/* Helper function for ProcessVisItCommand */
static void BroadcastSlaveCommand(int *command, simulation_data *sim)
{
#ifdef PARALLEL
    MPI_Bcast(command, 1, MPI_INT, 0, sim->par_comm);
#endif
}

/* Callback involved in command communication. */
void SlaveProcessCallback(void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    int command = VISIT_COMMAND_PROCESS;
    BroadcastSlaveCommand(&command, sim);
}

/* Process commands from viewer on all processors. */
int ProcessVisItCommand(simulation_data *sim)
{
    int command = VISIT_COMMAND_FAILURE;
    if (sim->par_rank==0)
    {  
        int success = VisItProcessEngineCommand();

        if (success == VISIT_OKAY)
        {
            command = VISIT_COMMAND_SUCCESS;
            BroadcastSlaveCommand(&command, sim);
            return 1;
        }
        else
        {
            command = VISIT_COMMAND_FAILURE;
            BroadcastSlaveCommand(&command, sim);
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
            BroadcastSlaveCommand(&command, sim);
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
    MPI_Bcast(cmd, 1000, MPI_CHAR, 0, sim->par_comm);
#endif

    if(strcmp(cmd, "quit") == 0)
        sim->done = 1;
    else if(strcmp(cmd, "halt") == 0)
        sim->runMode = SIM_STOPPED;
    else if(strcmp(cmd, "step") == 0)
        simulate_one_timestep(sim);
    else if(strcmp(cmd, "run") == 0)
        sim->runMode = SIM_RUNNING;

    if(sim->echo && sim->par_rank == 0)
    {
        fprintf(stderr, "Command %s completed.\n", cmd);
        fflush(stderr);
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
    int blocking, visitstate = 0, err = 0;

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
        MPI_Bcast(&visitstate, 1, MPI_INT, 0, sim->par_comm);
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
                VisItSetSlaveProcessCallback2(SlaveProcessCallback, (void*)sim);

                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetMaterial(SimGetMaterial, (void*)sim);
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
 * Date:       Mon Jun 23 15:28:41 PDT 2014
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

    /* Initialize environment variables. */
    SimulationArguments(argc, argv);

    for(i = 1; i < argc; ++i)
    {
        if(strcmp(argv[i], "-echo") == 0)
            sim.echo = 1;
        else if(strcmp(argv[i], "-nx") == 0 && (i+1) < argc)
        {
            sim.nx = atoi(argv[i+1]);
            ++i;
        }
        else if(strcmp(argv[i], "-ny") == 0 && (i+1) < argc)
        {
            sim.ny = atoi(argv[i+1]);
            ++i;
        }
        else if(strcmp(argv[i], "-nz") == 0 && (i+1) < argc)
        {
            sim.nz = atoi(argv[i+1]);
            ++i;
        }
    }

#ifdef PARALLEL
    /* Install callback functions for global communication. */
    VisItSetBroadcastIntFunction2(visit_broadcast_int_callback, (void*)&sim);
    VisItSetBroadcastStringFunction2(visit_broadcast_string_callback, (void*)&sim);

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

    /* Write out .sim file that VisIt uses to connect. Only do it
     * on processor 0.
     */
    /* CHANGE 3 */
    if(sim.par_rank == 0)
    {
        /* Write out .sim file that VisIt uses to connect. */
        VisItInitializeSocketAndDumpSimFile(
#ifdef PARALLEL
            "globalids_par",
#else
            "globalids",
#endif
            "Demonstrate global node and cell ids on an unstructured mesh",
            "/path/to/where/sim/was/started",
            NULL, NULL, SimulationFilename());
    }

    /* Call the main loop. */
    mainloop(&sim);

    simulation_data_dtor(&sim);
#ifdef PARALLEL
    MPI_Finalize();
#endif

    return 0;
}

/* DATA ACCESS FUNCTIONS */
const char *matNames[] = {"outer", "middle", "inner"};

/******************************************************************************
 *
 * Purpose: This callback function returns simulation metadata.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 13 13:47:15 PDT 2014
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
            VisIt_MeshMetaData_setName(mmd, "mesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 8);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "radial");
            VisIt_VariableMetaData_setMeshName(vmd, "mesh");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_NODE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }

        /* Add mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "tetmesh");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 3);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, 8);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add surface mesh metadata. */
        if(VisIt_MeshMetaData_alloc(&mmd) == VISIT_OKAY)
        {
            /* Set the mesh's properties.*/
            VisIt_MeshMetaData_setName(mmd, "surface");
            VisIt_MeshMetaData_setMeshType(mmd, VISIT_MESHTYPE_UNSTRUCTURED);
            VisIt_MeshMetaData_setTopologicalDimension(mmd, 2);
            VisIt_MeshMetaData_setSpatialDimension(mmd, 3);
            VisIt_MeshMetaData_setNumDomains(mmd, sim->surface_npartitions);

            VisIt_SimulationMetaData_addMesh(md, mmd);
        }

        /* Add a surface mesh variable. */
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "angle");
            VisIt_VariableMetaData_setMeshName(vmd, "surface");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        if(VisIt_VariableMetaData_alloc(&vmd) == VISIT_OKAY)
        {
            VisIt_VariableMetaData_setName(vmd, "matid");
            VisIt_VariableMetaData_setMeshName(vmd, "surface");
            VisIt_VariableMetaData_setType(vmd, VISIT_VARTYPE_SCALAR);
            VisIt_VariableMetaData_setCentering(vmd, VISIT_VARCENTERING_ZONE);

            VisIt_SimulationMetaData_addVariable(md, vmd);
        }
        /* Add a material */
        if(VisIt_MaterialMetaData_alloc(&mat) == VISIT_OKAY)
        {
            VisIt_MaterialMetaData_setName(mat, "surfacemat");
            VisIt_MaterialMetaData_setMeshName(mat, "surface");
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[0]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[1]);
            VisIt_MaterialMetaData_addMaterialName(mat, matNames[2]);

            VisIt_SimulationMetaData_addMaterial(md, mat);
        }

        /* Add some commands. */
        for(i = 0; i < (int) (sizeof(cmd_names)/sizeof(const char *)); ++i)
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
 * Purpose: Make tet connectivity from hex connectivity.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Sep  4 10:57:00 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

void
MakeTetConnectivity(const int *conn, int lconn, int **newconn, int *lnewconn)
{
    const int tets[][4] = {
        {0,1,3,7},{1,6,3,7},{1,2,3,6},{4,5,0,7},{0,5,1,7},{1,6,5,7}};

    /* We know that the input connectivity is all hexes. */
    int i, j, nelem, *nc;
    const int *c;

    c = conn + 1;
    nelem = lconn / 9;
    *lnewconn = nelem * 6 * (4 + 1);
    nc = (int *)malloc(*lnewconn * sizeof(int));
    *newconn = nc;
    for(i = 0; i < nelem; ++i)
    {
        for(j = 0; j < 6; ++j)
        {
            *nc++ = VISIT_CELL_TET;
            *nc++ = c[tets[j][0]];
            *nc++ = c[tets[j][1]];
            *nc++ = c[tets[j][2]];
            *nc++ = c[tets[j][3]];
        }
        c += 9;
    }
}

/******************************************************************************
 *
 * Purpose: This callback function returns meshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 13 13:49:52 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "mesh") == 0 || strcmp(name, "tetmesh") == 0)
    {
        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxyz, hc, gln, glc;

            VisIt_VariableData_alloc(&hxyz);
            VisIt_VariableData_setDataF(hxyz, VISIT_OWNER_SIM, 3, sim->domains[domain].nnodes, sim->domains[domain].xyz);
            VisIt_UnstructuredMesh_setCoords(h, hxyz);

            VisIt_VariableData_alloc(&hc);
            if(strcmp(name, "tetmesh") == 0)
            {
                int *tetconn = NULL, ltetconn = 0;
                MakeTetConnectivity(sim->domains[domain].connectivity, 
                                    sim->domains[domain].connectivityLen,
                                    &tetconn, &ltetconn);
                VisIt_VariableData_setDataI(hc, VISIT_OWNER_VISIT, 1, ltetconn, tetconn);
                VisIt_UnstructuredMesh_setConnectivity(h, ltetconn / 5, hc);
            }
            else
            {
                VisIt_VariableData_setDataI(hc, VISIT_OWNER_SIM, 1, sim->domains[domain].connectivityLen,
                    sim->domains[domain].connectivity);
                VisIt_UnstructuredMesh_setConnectivity(h, sim->domains[domain].ncells, hc);

                /* Global Cell Ids */
                VisIt_VariableData_alloc(&glc);
                VisIt_VariableData_setDataI(glc, VISIT_OWNER_SIM, 1, 
                    sim->domains[domain].ncells, sim->domains[domain].globalCellIds);
                VisIt_UnstructuredMesh_setGlobalCellIds(h, glc);
            }

            /* Global Node Ids */
            VisIt_VariableData_alloc(&gln);
            VisIt_VariableData_setDataI(gln, VISIT_OWNER_SIM, 1, 
                sim->domains[domain].nnodes, sim->domains[domain].globalNodeIds);
            VisIt_UnstructuredMesh_setGlobalNodeIds(h, gln);
        }
    }
    else if(strcmp(name, "surface") == 0)
    {
        /* Check for invalid requests. */
        if(sim->surfaces[domain].nnodes == 0)
        {
            fprintf(stderr, "Rank %d was asked for domain %d that it does not own.\n", sim->par_rank, domain);
            return h;
        }

        if(VisIt_UnstructuredMesh_alloc(&h) != VISIT_ERROR)
        {
            visit_handle hxyz, hc, gln, glc;

            VisIt_VariableData_alloc(&hxyz);
            VisIt_VariableData_setDataF(hxyz, VISIT_OWNER_SIM, 3, sim->surfaces[domain].nnodes, sim->surfaces[domain].xyz);
            VisIt_UnstructuredMesh_setCoords(h, hxyz);

            VisIt_VariableData_alloc(&hc);
            VisIt_VariableData_setDataI(hc, VISIT_OWNER_SIM, 1, sim->surfaces[domain].connectivityLen,
                sim->surfaces[domain].connectivity);
            VisIt_UnstructuredMesh_setConnectivity(h, sim->surfaces[domain].ncells, hc);

            /* Global Node Ids */
            VisIt_VariableData_alloc(&gln);
            VisIt_VariableData_setDataI(gln, VISIT_OWNER_SIM, 1, 
                sim->surfaces[domain].nnodes, sim->surfaces[domain].globalNodeIds);
            VisIt_UnstructuredMesh_setGlobalNodeIds(h, gln);

            /* Global Cell Ids */
            VisIt_VariableData_alloc(&glc);
            VisIt_VariableData_setDataI(glc, VISIT_OWNER_SIM, 1, 
                sim->surfaces[domain].ncells, sim->surfaces[domain].globalCellIds);
            VisIt_UnstructuredMesh_setGlobalCellIds(h, glc);
        }
    }
    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns material data.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 20 14:43:48 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMaterial(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Allocate a VisIt_MaterialData */
    if(strcmp(name, "surfacemat") == 0)
    {
        int cell, matnos[3];
        VisIt_MaterialData_alloc(&h);

        /* Tell the object we'll be adding cells to it using add*Cell functions */
        VisIt_MaterialData_appendCells(h, sim->surfaces[domain].ncells);

        /* Fill in the VisIt_MaterialData */
        VisIt_MaterialData_addMaterial(h, matNames[0], &matnos[0]);
        VisIt_MaterialData_addMaterial(h, matNames[1], &matnos[1]);
        VisIt_MaterialData_addMaterial(h, matNames[2], &matnos[2]);

        for(cell = 0; cell < sim->surfaces[domain].ncells; ++cell)
        {
            int idx = (int)sim->surfaces[domain].data2[cell];
            VisIt_MaterialData_addCleanCell(h, cell, matnos[idx]);
        }
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns scalars.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 13 13:49:52 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    simulation_data *sim = (simulation_data *)cbdata;

    if(strcmp(name, "radial") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            sim->domains[domain].nnodes, sim->domains[domain].data);
    }
    else if(strcmp(name, "angle") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            sim->surfaces[domain].ncells, sim->surfaces[domain].data);
    }
    else if(strcmp(name, "matid") == 0)
    {
        VisIt_VariableData_alloc(&h);
        VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, 1,
            sim->surfaces[domain].ncells, sim->surfaces[domain].data2);
    }

    return h;
}

/******************************************************************************
 *
 * Purpose: This callback function returns a domain list.
 *
 * Notes: This callback demonstrates that we can return unique domain 
 *        decompositions for different multimeshes.
 *
 * Programmer: Brad Whitlock
 * Date:       Fri Jun 20 17:37:09 PDT 2014
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetDomainList(const char *name, void *cbdata)
{
    visit_handle h, hdl;
    int i, j, *iptr = NULL, dcount, ndoms, dom;
    simulation_data *sim = (simulation_data *)cbdata;
    h = VISIT_INVALID_HANDLE;

    if(strcmp(name, "surface") == 0)
    {
        if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
        {
            dcount = 0;
            ndoms = sim->surface_npartitions;

            iptr = (int *)malloc(ndoms * sizeof(int));
            memset(iptr, 0, ndoms * sizeof(int));

            dom = 0;
            for(i = 0; i < 8; i++)
            {
                if(sim->owns[i] == 1)
                {
                    for(j = 0; j < sim->surface_partitions[i]; ++j)
                        iptr[dcount++] = dom++;
                }
                else
                {
                    dom += sim->surface_partitions[i];
                }
            }

#if 0
printf("rank %d owns: ", sim->par_rank);
for(i = 0; i < dcount; i++)
   printf("%d, ", iptr[i]);
printf("\n");
#endif
            VisIt_VariableData_alloc(&hdl);
            VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, dcount, iptr);
            VisIt_DomainList_setDomains(h, ndoms, hdl);
        }
    }
    else if(strcmp(name, "mesh") == 0)
    {
        if(VisIt_DomainList_alloc(&h) != VISIT_ERROR)
        {
            dcount = 0;
            ndoms = 8;

            iptr = (int *)malloc(ndoms * sizeof(int));
            memset(iptr, 0, ndoms * sizeof(int));

            for(i = 0; i < ndoms; i++)
            {
                if(sim->owns[i] == 1)
                    iptr[dcount++] = i;
            }

            VisIt_VariableData_alloc(&hdl);
            VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, dcount, iptr);
            VisIt_DomainList_setDomains(h, ndoms, hdl);
        }
    }
    else
    {
        printf("SimGetDomainList: unknown mesh=%s\n", name);
    }
    return h;
}
