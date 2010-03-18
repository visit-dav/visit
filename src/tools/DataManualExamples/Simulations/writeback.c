/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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

/******************************************************************************
 * writeback.c
 * 
 * Purpose: This application demonstrates how one might use libsim to 
 *          instrument a parallel application that listens to VisIt and to
 *          commands typed via node 0's stdin. Most of this application is
 *          devoted to showing how to use the writer interface for libsim's
 *          data access functions which allows the application to accept
 *          data that VisIt has modified. When the application gets data
 *          back from VisIt, it can store it away in its internal representation
 *          for use in further application-specific calculations.
 *
 * Programmer: Brad Whitlock
 * Date: Wed Mar 17 16:07:34 PDT 2010
 *
 * Build command:
 *   gcc -o writeback -I. -I/usr/gapps/visit/1.5.4/linux-intel/libsim/V1/include -DPARALLEL -I/misc/gapps/mpich/1.2.4/Linux/serial/64/debug/include -L/usr/gapps/visit/1.5.4/linux-intel/libsim/V1/lib -L/misc/gapps/mpich/1.2.4/Linux/serial/64/debug/lib writeback.c -Wl,--export-dynamic -lsim -ldl -lmpich
 *
 * Modifications:
 *
 ******************************************************************************/

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

#ifndef FREE
#define FREE(ptr) if((ptr) != NULL){free(ptr);}
#endif

/*****************************************************************************
 *****************************************************************************
 ***
 *** A crude map object.
 ***
 *****************************************************************************
 *****************************************************************************/

typedef struct
{
    void *key;
    void *value;
} parsim_map_elem_t;

typedef struct
{
    int                nelems;
    parsim_map_elem_t *elems;
} parsim_map_t;

void
parsim_map_create(parsim_map_t *m)
{
    m->nelems = 0;
    m->elems = NULL;
}

void
parsim_map_destroy(parsim_map_t *m, 
   void (*key_destruct)(void *), void (*elem_destruct)(void *))
{
    int i;
    for(i = 0; i < m->nelems; ++i)
    {
        (*key_destruct)(m->elems[i].key);
        (*elem_destruct)(m->elems[i].value);
    }
    free(m->elems);
}

void
parsim_map_add(parsim_map_t *m, void *key, void *value)
{
    parsim_map_elem_t *newelems = (parsim_map_elem_t *)
        malloc(sizeof(parsim_map_elem_t) * (m->nelems+1));
    if(m->nelems > 0)
    {
        memcpy(newelems, m->elems, sizeof(parsim_map_elem_t) * m->nelems);
        free(m->elems);
    }
    m->elems = newelems;

    m->elems[m->nelems].key = key;
    m->elems[m->nelems].value = value;
    ++m->nelems;
}

void *
parsim_map_get(parsim_map_t *m, const void *key, 
    int (*key_compare)(const void *, const void *))
{
    int i;
    void *ret = NULL;
    for(i = 0; i < m->nelems; ++i)
    {
        if((*key_compare)(key, m->elems[i].key) == 0)
        {
            ret = m->elems[i].value;
            break;
        }
    }
    return ret;
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Simulation data model.
 ***
 *****************************************************************************
 *****************************************************************************/

typedef struct SimDataArray
{
    void *data;
    int   dataType;
    int   nComps;
    int   nTuples;
} SimDataArray;

typedef struct SimRectilinearMesh
{
    int          ndims;
    SimDataArray coords[3];
} SimRectilinearMesh;

typedef struct SimCurvilinearMesh
{
    int          ndims;
    int          dims[3];
    SimDataArray coords;
} SimCurvilinearMesh;

typedef struct SimPointMesh
{
    SimDataArray coords;
    int          ndims;
} SimPointMesh;

typedef struct SimUnstructuredMesh
{
    SimDataArray coords;
    SimDataArray conn;
    int          nzones;
    int          ndims;
} SimUnstructuredMesh;

typedef struct SimMeshMetaData
{
    char *name;
 
    int   meshType;
    int   topologicalDimension;
    int   spatialDimension;

    int   numDomains;
    char *domainTitle;
    char *domainPieceName;

    int   numGroups;
    char *groupTitle;
    char *groupPieceName;

    char *xUnits;
    char *yUnits;
    char *zUnits;

    char *xLabel;
    char *yLabel;
    char *zLabel;
} SimMeshMetaData;

typedef struct SimVariableMetaData
{
    char *name;
    char *meshName;
    int   centering;
    int   type;
    char *units;
} SimVariableMetaData;

void
SimDataArray_free(SimDataArray *ptr)
{
    if(ptr != NULL)
    { 
        FREE(ptr->data);
    }
}

void
SimRectilinearMesh_free(SimRectilinearMesh *ptr)
{
    if(ptr != NULL)
    {
        if(ptr->ndims > 0)
            SimDataArray_free(&ptr->coords[0]);
        if(ptr->ndims > 1)
            SimDataArray_free(&ptr->coords[1]);
        if(ptr->ndims > 2)
            SimDataArray_free(&ptr->coords[2]);
    }
}

void
SimCurvilinearMesh_free(SimCurvilinearMesh *ptr)
{
    if(ptr != NULL)
        SimDataArray_free(&ptr->coords);
}

void
SimPointMesh_free(SimPointMesh *ptr)
{
    if(ptr != NULL)
        SimDataArray_free(&ptr->coords);
}

void
SimUnstructuredMesh_free(SimUnstructuredMesh *ptr)
{
    if(ptr != NULL)
    {
        SimDataArray_free(&ptr->coords);
        SimDataArray_free(&ptr->conn);
    }
}

void
SimMeshMetaData_free(SimMeshMetaData *ptr)
{
    if(ptr != NULL)
    {
        FREE((void*)ptr->name);
        FREE((void*)ptr->domainTitle);
        FREE((void*)ptr->domainPieceName);
        FREE((void*)ptr->groupTitle);
        FREE((void*)ptr->groupPieceName);
        FREE((void*)ptr->xUnits);
        FREE((void*)ptr->yUnits);
        FREE((void*)ptr->zUnits);
        FREE((void*)ptr->xLabel);
        FREE((void*)ptr->yLabel);
        FREE((void*)ptr->zLabel);
    }
}

void
SimVariableMetaData_free(SimVariableMetaData *ptr)
{
    if(ptr != NULL)
    {
        FREE((void*)ptr->name);
        FREE((void*)ptr->meshName);
        FREE((void*)ptr->units);
    }
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Convert to and from Simulation data model and SimV2.
 ***
 *****************************************************************************
 *****************************************************************************/

void
CombineCoordinates_From_Handles(SimDataArray *coords, int ndims, 
    visit_handle x, visit_handle y, visit_handle z)
{
    int owner[3], dataTypes[3], nComps[3], nTuples[3];
    void *data[3] = {NULL,NULL,NULL}, *ptr;
    int i = 0;
    VisIt_VariableData_getData(x, &owner[i], &dataTypes[i], &nComps[i], &nTuples[i], &data[i]);
    i = 1;
    VisIt_VariableData_getData(y, &owner[i], &dataTypes[i], &nComps[i], &nTuples[i], &data[i]);
    if(ndims > 2)
    {
        i = 2;
        VisIt_VariableData_getData(z, &owner[i], &dataTypes[i], &nComps[i], &nTuples[i], &data[i]);
        if(dataTypes[0] == VISIT_DATATYPE_DOUBLE)
        {
            double *dest;
            ptr = malloc(nTuples[0] * 3 * sizeof(double));
            dest = (double *)ptr;
            for(i = 0; i < nTuples[0]; ++i)
            {
                *dest++ = ((double*)data[0])[i];
                *dest++ = ((double*)data[1])[i];
                *dest++ = ((double*)data[2])[i];
            }
        }
        else
        {
            float *dest;
            ptr = malloc(nTuples[0] * 3 * sizeof(float));
            dest = (float *)ptr;
            for(i = 0; i < nTuples[0]; ++i)
            {
                *dest++ = ((float*)data[0])[i];
                *dest++ = ((float*)data[1])[i];
                *dest++ = ((float*)data[2])[i];
            }
        }
    }
    else if(dataTypes[0] == VISIT_DATATYPE_DOUBLE)
    {
        double *dest;
        ptr = (double *)malloc(nTuples[0] * 3 * sizeof(double));
        dest = (double *)ptr;
        for(i = 0; i < nTuples[0]; ++i)
        {
            *dest++ = ((double*)data[0])[i];
            *dest++ = ((double*)data[1])[i];
            *dest++ = 0.;
        }
    }
    else
    {
        float *dest;
        ptr = (float *)malloc(nTuples[0] * 3 * sizeof(float));
        dest = (float *)ptr;
        for(i = 0; i < nTuples[0]; ++i)
        {
            *dest++ = ((float*)data[0])[i];
            *dest++ = ((float*)data[1])[i];
            *dest++ = 0.;
        }
    }

    coords->data = ptr;
    coords->dataType = VISIT_DATATYPE_DOUBLE;
    coords->nComps = 3;
    coords->nTuples = nTuples[0];
}

void
SimDataArray_From_Handle(SimDataArray *ptr, visit_handle h)
{
    if(ptr != NULL)
    {
        int owner;
        void *data = NULL;

        memset(ptr, 0, sizeof(SimDataArray));

        if(VisIt_VariableData_getData(h, &owner, &ptr->dataType, &ptr->nComps, 
            &ptr->nTuples, &data) == VISIT_OKAY)
        {
            size_t sz = 1;
            if(ptr->dataType == VISIT_DATATYPE_CHAR)
                sz = ptr->nComps * ptr->nTuples * sizeof(char);
            else if(ptr->dataType == VISIT_DATATYPE_INT)
                sz = ptr->nComps * ptr->nTuples * sizeof(int);
            else if(ptr->dataType == VISIT_DATATYPE_FLOAT)
                sz = ptr->nComps * ptr->nTuples * sizeof(float);
            else if(ptr->dataType == VISIT_DATATYPE_DOUBLE)
                sz = ptr->nComps * ptr->nTuples * sizeof(double);
            ptr->data = malloc(sz);
            memcpy(ptr->data, data, sz);
        }
    }
}

visit_handle
Handle_From_SimDataArray(SimDataArray *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_VariableData_alloc(&h) == VISIT_OKAY)
    {
        switch(ptr->dataType)
        {
        case VISIT_DATATYPE_CHAR:
            VisIt_VariableData_setDataC(h, VISIT_OWNER_SIM, ptr->nComps,
                ptr->nTuples, (char *)ptr->data);
            break;
        case VISIT_DATATYPE_INT:
            VisIt_VariableData_setDataI(h, VISIT_OWNER_SIM, ptr->nComps,
                ptr->nTuples, (int *)ptr->data);
            break;
        case VISIT_DATATYPE_FLOAT:
            VisIt_VariableData_setDataF(h, VISIT_OWNER_SIM, ptr->nComps,
                ptr->nTuples, (float *)ptr->data);
            break;
        case VISIT_DATATYPE_DOUBLE:
            VisIt_VariableData_setDataD(h, VISIT_OWNER_SIM, ptr->nComps,
                ptr->nTuples, (double *)ptr->data);
            break;
        }
    }
    return h;
}

SimRectilinearMesh *
SimRectilinearMesh_From_Handle(visit_handle h)
{
    SimRectilinearMesh *ptr = (SimRectilinearMesh *)malloc(sizeof(SimRectilinearMesh));
    if(ptr != NULL)
    {
        visit_handle x,y,z;

        memset(ptr, 0, sizeof(SimRectilinearMesh));

        if(VisIt_RectilinearMesh_getCoords(h, &ptr->ndims, &x, &y, &z) == VISIT_OKAY)
        {
            SimDataArray_From_Handle(&ptr->coords[0], x);
            SimDataArray_From_Handle(&ptr->coords[1], y);
            if(ptr->ndims > 2)
                SimDataArray_From_Handle(&ptr->coords[2], z);
        }
    }
    return ptr;
}

visit_handle
Handle_From_SimRectilinearMesh(SimRectilinearMesh *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_RectilinearMesh_alloc(&h) == VISIT_OKAY)
    {
        visit_handle x,y,z;
        x = Handle_From_SimDataArray(&ptr->coords[0]);
        y = Handle_From_SimDataArray(&ptr->coords[1]);
        if(ptr->ndims > 2)
        {
            z = Handle_From_SimDataArray(&ptr->coords[2]);
            VisIt_RectilinearMesh_setCoordsXYZ(h, x,y,z);
        }
        else
            VisIt_RectilinearMesh_setCoordsXY(h,x,y);
    }
    return h;
}

SimCurvilinearMesh *
SimCurvilinearMesh_From_Handle(visit_handle h)
{
    SimCurvilinearMesh *ptr = (SimCurvilinearMesh *)malloc(sizeof(SimCurvilinearMesh));
    if(ptr != NULL)
    {
        int coordMode;
        visit_handle x,y,z,c;

        memset(ptr, 0, sizeof(SimCurvilinearMesh));

        if(VisIt_CurvilinearMesh_getCoords(h, &ptr->ndims, ptr->dims, &coordMode,
           &x, &y, &z, &c) == VISIT_OKAY)
        {
            if(coordMode == VISIT_COORD_MODE_INTERLEAVED)
                SimDataArray_From_Handle(&ptr->coords, c);
            else
                CombineCoordinates_From_Handles(&ptr->coords, ptr->ndims, x,y,z);
        }
    }
    return ptr;
}

visit_handle
Handle_From_SimCurvilinearMesh(SimCurvilinearMesh *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_CurvilinearMesh_alloc(&h) == VISIT_OKAY)
    {
        visit_handle c;
        c = Handle_From_SimDataArray(&ptr->coords);
        VisIt_CurvilinearMesh_setCoords3(h, ptr->dims, c);
    }
    return h;
}

SimPointMesh *
SimPointMesh_From_Handle(visit_handle h)
{
    SimPointMesh *ptr = (SimPointMesh *)malloc(sizeof(SimPointMesh));
    if(ptr != NULL)
    {
        int coordMode;
        visit_handle x,y,z,c;

        memset(ptr, 0, sizeof(SimPointMesh));

        if(VisIt_PointMesh_getCoords(h, &ptr->ndims, &coordMode,
           &x, &y, &z, &c) == VISIT_OKAY)
        {
            if(coordMode == VISIT_COORD_MODE_INTERLEAVED)
                SimDataArray_From_Handle(&ptr->coords, c);
            else
                CombineCoordinates_From_Handles(&ptr->coords, ptr->ndims, x,y,z);
        }
    }
    return ptr;
}

visit_handle
Handle_From_SimPointMesh(SimPointMesh *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_PointMesh_alloc(&h) == VISIT_OKAY)
    {
        visit_handle c;
        c = Handle_From_SimDataArray(&ptr->coords);
        VisIt_PointMesh_setCoords(h, c);
    }
    return h;
}

SimUnstructuredMesh *
SimUnstructuredMesh_From_Handle(visit_handle h)
{
    SimUnstructuredMesh *ptr = (SimUnstructuredMesh *)malloc(sizeof(SimUnstructuredMesh));
    if(ptr != NULL)
    {
        int coordMode;
        visit_handle x,y,z,c,conn;

        memset(ptr, 0, sizeof(SimUnstructuredMesh));

        if(VisIt_UnstructuredMesh_getCoords(h, &ptr->ndims, &coordMode,
           &x, &y, &z, &c) == VISIT_OKAY)
        {
            if(coordMode == VISIT_COORD_MODE_INTERLEAVED)
                SimDataArray_From_Handle(&ptr->coords, c);
            else
                CombineCoordinates_From_Handles(&ptr->coords, ptr->ndims, x,y,z);
        }

        if(VisIt_UnstructuredMesh_getConnectivity(h, &ptr->nzones, &conn) == VISIT_OKAY)
        {
            SimDataArray_From_Handle(&ptr->conn, conn);
        }
    }
    return ptr;
}

visit_handle
Handle_From_SimUnstructuredMesh(SimUnstructuredMesh *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_UnstructuredMesh_alloc(&h) == VISIT_OKAY)
    {
        visit_handle c, conn;
        c = Handle_From_SimDataArray(&ptr->coords);
        VisIt_UnstructuredMesh_setCoords(h, c);

        conn = Handle_From_SimDataArray(&ptr->conn);
        VisIt_UnstructuredMesh_setConnectivity(h, ptr->nzones, conn);
    }
    return h;
}

SimMeshMetaData *
SimMeshMetaData_From_Handle(visit_handle h)
{
    SimMeshMetaData *ptr = (SimMeshMetaData *)malloc(sizeof(SimMeshMetaData));
    if(ptr != NULL)
    {
        memset(ptr, 0, sizeof(SimMeshMetaData));

        VisIt_MeshMetaData_getName(h, &ptr->name);
        VisIt_MeshMetaData_getMeshType(h, &ptr->meshType);
        VisIt_MeshMetaData_getTopologicalDimension(h, &ptr->topologicalDimension);
        VisIt_MeshMetaData_getSpatialDimension(h, &ptr->spatialDimension);
        VisIt_MeshMetaData_getNumDomains(h, &ptr->numDomains);
        VisIt_MeshMetaData_getDomainTitle(h, &ptr->domainTitle);
        VisIt_MeshMetaData_getDomainPieceName(h, &ptr->domainPieceName);
        VisIt_MeshMetaData_getNumGroups(h, &ptr->numGroups);
        VisIt_MeshMetaData_getGroupTitle(h, &ptr->groupTitle);
        VisIt_MeshMetaData_getXUnits(h, &ptr->xUnits);
        VisIt_MeshMetaData_getYUnits(h, &ptr->yUnits);
        VisIt_MeshMetaData_getZUnits(h, &ptr->zUnits);
        VisIt_MeshMetaData_getXLabel(h, &ptr->xLabel);
        VisIt_MeshMetaData_getYLabel(h, &ptr->yLabel);
        VisIt_MeshMetaData_getZLabel(h, &ptr->zLabel);
    }
    return ptr;
}

visit_handle
Handle_From_SimMeshMetaData(SimMeshMetaData *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_MeshMetaData_alloc(&h) == VISIT_OKAY)
    {
        VisIt_MeshMetaData_setName(h, ptr->name);
        VisIt_MeshMetaData_setMeshType(h, ptr->meshType);
        VisIt_MeshMetaData_setTopologicalDimension(h, ptr->topologicalDimension);
        VisIt_MeshMetaData_setSpatialDimension(h, ptr->spatialDimension);
        VisIt_MeshMetaData_setNumDomains(h, ptr->numDomains);
        VisIt_MeshMetaData_setDomainTitle(h, ptr->domainTitle);
        VisIt_MeshMetaData_setDomainPieceName(h, ptr->domainPieceName);
        VisIt_MeshMetaData_setNumGroups(h, ptr->numGroups);
        VisIt_MeshMetaData_setGroupTitle(h, ptr->groupTitle);
        VisIt_MeshMetaData_setXUnits(h, ptr->xUnits);
        VisIt_MeshMetaData_setYUnits(h, ptr->yUnits);
        VisIt_MeshMetaData_setZUnits(h, ptr->zUnits);
        VisIt_MeshMetaData_setXLabel(h, ptr->xLabel);
        VisIt_MeshMetaData_setYLabel(h, ptr->yLabel);
        VisIt_MeshMetaData_setZLabel(h, ptr->zLabel);
    }
    return h;
}

SimVariableMetaData *
SimVariableMetaData_From_Handle(visit_handle h)
{
    SimVariableMetaData *ptr = (SimVariableMetaData *)malloc(sizeof(SimVariableMetaData));
    if(ptr != NULL)
    {
        memset(ptr, 0, sizeof(SimVariableMetaData));

        VisIt_VariableMetaData_getName(h, &ptr->name);
        VisIt_VariableMetaData_getMeshName(h, &ptr->meshName);
        VisIt_VariableMetaData_getCentering(h, &ptr->centering);
        VisIt_VariableMetaData_getType(h, &ptr->type);
        VisIt_VariableMetaData_getUnits(h, &ptr->units);
    }
    return ptr;
}

visit_handle
Handle_From_SimVariableMetaData(SimVariableMetaData *ptr)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    if(ptr != NULL &&
       VisIt_VariableMetaData_alloc(&h) == VISIT_OKAY)
    {
        VisIt_VariableMetaData_setName(h, ptr->name);
        VisIt_VariableMetaData_setMeshName(h, ptr->meshName);
        VisIt_VariableMetaData_setCentering(h, ptr->centering);
        VisIt_VariableMetaData_setType(h, ptr->type);
        VisIt_VariableMetaData_setUnits(h, ptr->units);
    }
    return h;
}

static void
Mesh_Free(int mesh_type, void *mesh)
{
    if(mesh_type == VISIT_MESHTYPE_RECTILINEAR)
        SimRectilinearMesh_free((SimRectilinearMesh *)mesh);
    else if(mesh_type == VISIT_MESHTYPE_CURVILINEAR)
        SimCurvilinearMesh_free((SimCurvilinearMesh *)mesh);
    else if(mesh_type == VISIT_MESHTYPE_UNSTRUCTURED)
        SimUnstructuredMesh_free((SimUnstructuredMesh *)mesh);
    else if(mesh_type == VISIT_MESHTYPE_POINT)
        SimPointMesh_free((SimPointMesh *)mesh);
    free(mesh);
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Functions to manage the mesh cache.
 ***
 *****************************************************************************
 *****************************************************************************/

static int
key_compare_strings(const void *k0, const void *k1)
{
    return strcmp((const char *)k0, (const char *)k1);
}

static int
key_compare_ints(const void *k0, const void *k1)
{
    int i0, i1, ret = 0;
    i0 = *((int *)k0);
    i1 = *((int *)k1);
    if(i0 < i1)
        ret = -1;
    else if(i0 > i1)
        ret = 1;
    return ret;
}

static void *
key_newint(int i)
{
    int *iptr = (int *)malloc(sizeof(int));
    *iptr = i;
    return (void*)iptr;
}

static void *
key_newstring(const char *s)
{
    return (void*)strdup(s);
}

typedef struct
{
    SimMeshMetaData *metadata;
    parsim_map_t     domain_meshes;
} parsim_mesh_record_t;

typedef struct
{
    void            *mesh;
    SimMeshMetaData *metadata;
} MeshAndMetaData;

void
mesh_dtor_0(void *ptr)
{
    SimRectilinearMesh_free((SimRectilinearMesh *)ptr);
    free(ptr);
}

void
mesh_dtor_1(void *ptr)
{
    SimCurvilinearMesh_free((SimCurvilinearMesh *)ptr);
    free(ptr);
}

void
mesh_dtor_2(void *ptr)
{
    SimPointMesh_free((SimPointMesh *)ptr);
    free(ptr);
}

void
mesh_dtor_3(void *ptr)
{
    SimUnstructuredMesh_free((SimUnstructuredMesh *)ptr);
    free(ptr);
}

void
parsim_mesh_record_t_free(void *ptr)
{
    parsim_mesh_record_t *rec = (parsim_mesh_record_t *)ptr;
    if(rec->metadata->meshType == VISIT_MESHTYPE_RECTILINEAR)
        parsim_map_destroy(&rec->domain_meshes, free, mesh_dtor_0);
    if(rec->metadata->meshType == VISIT_MESHTYPE_CURVILINEAR)
        parsim_map_destroy(&rec->domain_meshes, free, mesh_dtor_1);
    if(rec->metadata->meshType == VISIT_MESHTYPE_POINT)
        parsim_map_destroy(&rec->domain_meshes, free, mesh_dtor_2);
    if(rec->metadata->meshType == VISIT_MESHTYPE_UNSTRUCTURED)
        parsim_map_destroy(&rec->domain_meshes, free, mesh_dtor_3);

    SimMeshMetaData_free((SimMeshMetaData *)rec->metadata);
    free(rec->metadata);

    free(ptr);
}

void
MeshCache_add_mesh(parsim_map_t *mesh_cache, const char *name, int domain, 
    MeshAndMetaData m_mmd)
{
    void *mesh_copy;
    parsim_mesh_record_t *mesh_entry;

    /* Look for an entry in the map with the same mesh name. */
    mesh_entry = (parsim_mesh_record_t*)parsim_map_get(mesh_cache, name, key_compare_strings);
    if(mesh_entry == NULL)
    {
        /* No domain list with the given name was found. Add one. */
        mesh_entry = (parsim_mesh_record_t*)
            malloc(sizeof (parsim_mesh_record_t));

        /* Add the mesh copy to the domain mesh map. */
        mesh_entry->metadata = m_mmd.metadata;
        parsim_map_create(&mesh_entry->domain_meshes);
        parsim_map_add(&mesh_entry->domain_meshes, key_newint(domain), m_mmd.mesh);

        /* Add the new domain map to the mesh cache.*/
        parsim_map_add(mesh_cache, key_newstring(name), mesh_entry);
    }
    else
    {
        /* Replace the existing metadata. */
        SimMeshMetaData_free(mesh_entry->metadata);
        mesh_entry->metadata = m_mmd.metadata;

        /* We found a mesh with the given name. Look for the specified domain. */
        void *domain_mesh = parsim_map_get(&mesh_entry->domain_meshes, 
            (void*)&domain, key_compare_ints);

        if(domain_mesh == NULL)
        {
            /* Add the domain mesh. */
            parsim_map_add(&mesh_entry->domain_meshes, key_newint(domain), m_mmd.mesh);
        }
        else
        {
            /* Copy over the value in the map with the new mesh. */
            int i;
            parsim_map_t *mr = &mesh_entry->domain_meshes;
            for(i = 0; i < mesh_entry->domain_meshes.nelems; ++i)
            {
                if(mesh_entry->domain_meshes.elems[i].value == domain_mesh)
                {
                    Mesh_Free(mesh_entry->metadata->meshType, 
                              mesh_entry->domain_meshes.elems[i].value);
                    mesh_entry->domain_meshes.elems[i].value = m_mmd.mesh;
                    break;
                }
            }
        }
    }
}

int
MeshCache_get_nmeshes(parsim_map_t *mesh_cache)
{
    return mesh_cache->nelems;
}

int
MeshCache_get_mesh_info(parsim_map_t *mesh_cache, int i, char **name, 
    SimMeshMetaData **mmd)
{
    int ret = 0;
    if(i >= 0 && i < mesh_cache->nelems)
    {
        *name = mesh_cache->elems[i].key;
        *mmd = ((parsim_mesh_record_t *)mesh_cache->elems[i].value)->metadata;
        ret = 1;
    }
    return ret;
}

int
MeshCache_get_mesh_info2(parsim_map_t *mesh_cache, const char *name, 
    SimMeshMetaData **mmd)
{
    int ret = 0;
    parsim_mesh_record_t *info = parsim_map_get(mesh_cache, name, key_compare_strings);
    if(info != 0)
    {
        *mmd = info->metadata;
        ret = 1;
    }
    return ret;
}

void *
MeshCache_get_mesh(parsim_map_t *mesh_cache, const char *name, int domain)
{
    void *ret = NULL;

    /* Look for an entry in the map with the same mesh name. */
    parsim_mesh_record_t *mesh_entry = (parsim_mesh_record_t*)parsim_map_get(
        mesh_cache, name, key_compare_strings);

    if(mesh_entry != NULL)
    {
        ret = parsim_map_get(&mesh_entry->domain_meshes, (void*)&domain, 
            key_compare_ints);
    }

    return ret;
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Functions to manage the variable cache.
 ***
 *****************************************************************************
 *****************************************************************************/


typedef struct
{
    SimVariableMetaData *metadata;
    parsim_map_t         domain_vars;
} parsim_var_record_t;

void
SimDataArray_free2(void *ptr)
{
    SimDataArray_free((SimDataArray *)ptr);
    free(ptr);
}

void
parsim_var_record_t_free(void *ptr)
{
    parsim_var_record_t *rec = (parsim_var_record_t *)ptr;
    SimVariableMetaData_free((SimVariableMetaData *)rec->metadata);
    free(rec->metadata);
    parsim_map_destroy(&rec->domain_vars, free, SimDataArray_free2);
    free(ptr);
}

void
VarCache_add_var(parsim_map_t *var_cache, const char *name, int domain, 
    const char *meshName, SimDataArray *data, SimVariableMetaData *metadata)
{
    parsim_var_record_t *var_entry = NULL;

    /* Look for an entry in the map with the same var name. */
    var_entry = (parsim_var_record_t*)parsim_map_get(var_cache, 
        name, key_compare_strings);
    if(var_entry == NULL)
    {
        /* The var cache did not have an entry for the variable. Add one. */
        var_entry = (parsim_var_record_t*)malloc(sizeof(parsim_var_record_t));
        var_entry->metadata = metadata;
        parsim_map_create(&var_entry->domain_vars);

        /* Add the domain data to the domain map. */
        parsim_map_add(&var_entry->domain_vars, key_newint(domain), data);

        /* Add the domain map to the var cache. */
        parsim_map_add(var_cache, key_newstring(name), var_entry);
    }
    else
    {
        /* Replace the metadata. */
        SimVariableMetaData_free(var_entry->metadata);
        var_entry->metadata = metadata;

        /* We found a var with the given name. Look for the specified domain. */
        void *domain_var = parsim_map_get(&var_entry->domain_vars, 
            (void*)&domain, key_compare_ints);

        if(domain_var == NULL)
        {
            /* Add the domain var. */
            parsim_map_add(&var_entry->domain_vars, key_newint(domain), data);
        }
        else
        {
            /* Copy over the value in the map with the new var. */
            int i;
            for(i = 0; i < var_entry->domain_vars.nelems; ++i)
            {
                if(var_entry->domain_vars.elems[i].value == domain_var)
                {
                    SimDataArray *var = (SimDataArray *)var_entry->domain_vars.elems[i].value;
                    SimDataArray_free(var);
                    var_entry->domain_vars.elems[i].value = data;
                    break;
                }
            }
        }
    }
}

int
VarCache_get_nvars(parsim_map_t *var_cache)
{
    return var_cache->nelems;
}

int
VarCache_get_var_info(parsim_map_t *var_cache, int i, char **name, 
    SimVariableMetaData **smd)
{
    int ret = 0;
    if(i >= 0 && i < var_cache->nelems)
    {
        parsim_var_record_t *var_entry;
        var_entry = (parsim_var_record_t *)var_cache->elems[i].value;
      
        *name = (char *)var_cache->elems[i].key;
        *smd = var_entry->metadata;

        ret = 1;
    }

    return ret;
}

visit_handle
VarCache_get_var(parsim_map_t *var_cache, const char *name, int domain)
{
    visit_handle scalar = VISIT_INVALID_HANDLE;

    /* Look for an entry in the map with the same mesh name. */
    parsim_var_record_t *var_entry = (parsim_var_record_t*)parsim_map_get(
        var_cache, name, key_compare_strings);

    if(var_entry != NULL)
    {
        SimDataArray *var = (SimDataArray *)parsim_map_get(&var_entry->domain_vars,
           (void*)&domain, key_compare_ints);

        if(var != NULL)
            scalar = Handle_From_SimDataArray(var);
    }

    return scalar;
}

/******************************************************************************
 ******************************************************************************
 ***
 *** Simulation data and functions
 ***
 ******************************************************************************
 *****************************************************************************/
#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

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

    /* The real data */
    parsim_map_t mesh_cache;
    parsim_map_t var_cache;
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

    parsim_map_create(&sim->mesh_cache);
    parsim_map_create(&sim->var_cache);
}


void
free_mesh_cache_item(void *ptr)
{
    SimDataArray_free((SimDataArray *)ptr);
    free(ptr);
}

void
simulation_data_dtor(simulation_data *sim)
{
   parsim_map_destroy(&sim->var_cache, free, parsim_var_record_t_free);
   parsim_map_destroy(&sim->mesh_cache, free, parsim_mesh_record_t_free);
}

const char *cmd_names[] = {"halt", "step", "run", "update"};


/******************************************************************************
 ******************************************************************************
 ***
 *** DATA ACCESS READER FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: SimGetMetaData
 *
 * Purpose: Returns metadata to VisIt based on the metadata that we have stored
 *          in our program's mesh and variable caches.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 15:47:08 PDT 2010
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
        int i, n;
        visit_handle mmd = VISIT_INVALID_HANDLE;
        visit_handle vmd = VISIT_INVALID_HANDLE;
        visit_handle cmd = VISIT_INVALID_HANDLE;
        visit_handle emd = VISIT_INVALID_HANDLE;

        /* Set the simulation state. */
        VisIt_SimulationMetaData_setMode(md, (sim->runMode == SIM_STOPPED) ?
            VISIT_SIMMODE_STOPPED : VISIT_SIMMODE_RUNNING);
        VisIt_SimulationMetaData_setCycleTime(md, sim->cycle, sim->time);

        /* Look through the mesh cache and return information about
         * meshes that we know about.
         */
        n = MeshCache_get_nmeshes(&sim->mesh_cache);
        for(i = 0; i < n; ++i)
        {
            char *name = NULL;
            SimMeshMetaData *mmd = NULL;

            if(MeshCache_get_mesh_info(&sim->mesh_cache, i, &name, &mmd))
                VisIt_SimulationMetaData_addMesh(md, Handle_From_SimMeshMetaData(mmd));
        }

        /* Look through the variables that we know about and expose them. */
        n = VarCache_get_nvars(&sim->var_cache);
        for(i = 0; i < n; ++i)
        {
            char *name = NULL;
            SimVariableMetaData *smd = NULL;
            if(VarCache_get_var_info(&sim->var_cache, i, &name, &smd))
                VisIt_SimulationMetaData_addVariable(md, Handle_From_SimVariableMetaData(smd));
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
 * Function: SimGetMesh
 *
 * Purpose: Returns the specified mesh and domain to VisIt.
 * 
 * Notes: This function returns the requested mesh from the mesh cache.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetMesh(int domain, const char *name, void *cbdata)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    SimMeshMetaData *mmd = NULL;  
    simulation_data *sim = (simulation_data *)cbdata;

    /* Look up the mesh name in our mesh cache to get some information
     * about the mesh.
     */
    if(MeshCache_get_mesh_info2(&sim->mesh_cache, name, &mmd))
    {
        void *mesh_data = MeshCache_get_mesh(&sim->mesh_cache, name, domain);
        if(mesh_data != NULL)
        {
            /* We found the mesh/domain that we were looking for.
             * Wrap up our sim's data into objects that we return to VisIt.
             */
            if(mmd->meshType == VISIT_MESHTYPE_RECTILINEAR)
                h = Handle_From_SimRectilinearMesh((SimRectilinearMesh*)mesh_data);
            else if(mmd->meshType == VISIT_MESHTYPE_CURVILINEAR)
                h = Handle_From_SimCurvilinearMesh((SimCurvilinearMesh* )mesh_data);
            else if(mmd->meshType == VISIT_MESHTYPE_UNSTRUCTURED)
                h = Handle_From_SimUnstructuredMesh((SimUnstructuredMesh *)mesh_data);
            else if(mmd->meshType == VISIT_MESHTYPE_POINT)
                h = Handle_From_SimPointMesh((SimPointMesh *)mesh_data);
        }
    }

    return h;
}

/******************************************************************************
 * Function: SimGetVariable
 *
 * Purpose: Returns the specified scalar from the right domain.
 *
 * Notes: This function returns the requested variable from the variable cache.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

visit_handle
SimGetVariable(int domain, const char *name, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    return VarCache_get_var(&sim->var_cache, name, domain);
}

/******************************************************************************
 * Function: SimGetDomainList
 *
 * Purpose: Returns the domain list to VisIt.
 *
 * Notes: This function must be provide in order for VisIt to run in parallel
 *        without crashing. This function returns a list of domains to VisIt
 *        so VisIt knows the processor where domains are allocated. This 
 *        example assumes that each processor will have 1 domain.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
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
        int i, *iptr = NULL;
        simulation_data *sim = (simulation_data *)cbdata;

        iptr = (int *)malloc(sizeof(int));
        *iptr = sim->par_rank;

        VisIt_VariableData_alloc(&hdl);
        VisIt_VariableData_setDataI(hdl, VISIT_OWNER_VISIT, 1, 1, iptr);
        VisIt_DomainList_setDomains(h, sim->par_size, hdl);
    }
    return h;
}

/******************************************************************************
 ******************************************************************************
 ***
 *** DATA ACCESS WRITER FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: SimWriteBegin
 *
 * Purpose: This function is called before any data is sent to the application.
 *
 * Notes: Use this function if you need to prepare your application before VisIt
 *        sends it data.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int SimWriteBegin(const char *objName, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    if(sim->par_rank == 0)
    { 
        printf("Simulation preparing for %s from VisIt.\n", objName);
    }
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: SimWriteEnd
 *
 * Purpose: This function is called after VisIt has sent all data to the application.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int SimWriteEnd(const char *objName, void *cbdata)
{
    simulation_data *sim = (simulation_data *)cbdata;
    if(sim->par_rank == 0)
    { 
        printf("Simulation handled %s from VisIt.\n", objName);
    }
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: SimWriteMesh
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          curvilinear mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int SimWriteMesh(const char *objName, int chunk, int meshType, 
    visit_handle m, visit_handle mmd, void *cbdata)
{
    MeshAndMetaData m_mmd = {NULL, NULL};
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create a copy of the mesh. */
    if(meshType == VISIT_MESHTYPE_CURVILINEAR)
        m_mmd.mesh = (void *)SimCurvilinearMesh_From_Handle(m);
    else if(meshType == VISIT_MESHTYPE_RECTILINEAR)
        m_mmd.mesh = (void *)SimRectilinearMesh_From_Handle(m);
    else if(meshType == VISIT_MESHTYPE_POINT)
        m_mmd.mesh = (void *)SimPointMesh_From_Handle(m);
    else if(meshType == VISIT_MESHTYPE_UNSTRUCTURED)
        m_mmd.mesh = (void *)SimUnstructuredMesh_From_Handle(m);
    /* Create a copy of the mesh metadata. */
    m_mmd.metadata = SimMeshMetaData_From_Handle(mmd);

    if(sim->par_rank == 0)
    { 
        printf("Simulation received domain %d of mesh %s from VisIt.\n",
               chunk, objName);
    }

    MeshCache_add_mesh(&sim->mesh_cache, objName, chunk, m_mmd);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: SimWriteVariable
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          data array for the mesh that was sent previously.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int SimWriteVariable(const char *objName, const char *arrName, int chunk,
    visit_handle data, visit_handle metadata, void *cbdata)
{
    char varname[200];
    SimDataArray *d = NULL;
    SimVariableMetaData *md = NULL;
    simulation_data *sim = (simulation_data *)cbdata;

    /* Create a name that VisIt will treat like a subdirectory in the GUI. This also
     * prevents the original data from getting clobbered if only the mesh changed.
     * If you want to clobber the original data then by all means do so.
     */
    sprintf(varname, "%s/%s", objName, arrName);
    if(sim->par_rank == 0)
    { 
        printf("Simulation getting new data array %s:%s:%d from VisIt.\n", objName, arrName, chunk);
    }

    /* Package up the data into SimDataArray */
    d = (SimDataArray *)malloc(sizeof(SimDataArray));
    SimDataArray_From_Handle(d, data);

    /* Convert the metadata and replace the variable name. */
    md = SimVariableMetaData_From_Handle(metadata);
    free((void*)md->name);
    md->name = strdup(varname);

    /* Store the variable coming in from VisIt on the "objName" mesh. Since we are
     * adding the variable to our sim's variable cache, we're adding it to the list
     * of variables that the simulation knows about and can perform calculations on.
     * Other clients using the writer interface could translate this incoming data
     * into a representation convenient for the client.
     */

    /* Add the data to the variable cache. */
    VarCache_add_var(&sim->var_cache, varname, chunk, objName, d, md);

    return VISIT_OKAY;
}

/******************************************************************************/

/******************************************************************************
 * Function: CreateRectMesh
 *
 * Purpose: Creates a rectilinear mesh so the application will have a mesh to
 *          give to VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

void
CreateRectMesh(int domain, SimRectilinearMesh **rmesh, SimDataArray **dist)
{
    int i,j,k, dims[3] = {101, 151, 201};
    float x_offset, y_offset, z_offset = 0.;
    float *x, *y, *z, *distvar;

    /* Offset the coordinates based on the domain. */
    x_offset = (float)(domain % 4);
    y_offset = (float)(domain / 4);

    /* Create coordinates. */
    x = (float*)malloc(dims[0] * sizeof(float));
    for(i = 0; i < dims[0]; ++i)
    {
        float t = (float)i / (float)(dims[0] - 1);
        x[i] = t + x_offset;
    }
    y = (float*)malloc(dims[1] * sizeof(float));
    for(i = 0; i < dims[1]; ++i)
    {
        float t = (float)i / (float)(dims[1] - 1);
        y[i] = t + y_offset;
    }
    z = (float*)malloc(dims[2] * sizeof(float));
    for(i = 0; i < dims[2]; ++i)
    {
        float t = (float)i / (float)(dims[2] - 1);
        z[i] = t + z_offset;
    }

    *rmesh = (SimRectilinearMesh *)malloc(sizeof(SimRectilinearMesh));
    memset(*rmesh, 0, sizeof(SimRectilinearMesh));
    (*rmesh)->ndims = 3;

    (*rmesh)->coords[0].data = (void *)x;
    (*rmesh)->coords[0].dataType = VISIT_DATATYPE_FLOAT;
    (*rmesh)->coords[0].nComps = 1;
    (*rmesh)->coords[0].nTuples = dims[0];
      
    (*rmesh)->coords[1].data = (void *)y;
    (*rmesh)->coords[1].dataType = VISIT_DATATYPE_FLOAT;
    (*rmesh)->coords[1].nComps = 1;
    (*rmesh)->coords[1].nTuples = dims[1];
      
    (*rmesh)->coords[2].data = (void *)z;
    (*rmesh)->coords[2].dataType = VISIT_DATATYPE_FLOAT;
    (*rmesh)->coords[2].nComps = 1;
    (*rmesh)->coords[2].nTuples = dims[2];

    /* Create distance variable. */
    distvar = (float*)malloc((dims[0]-1) * (dims[1]-1) * (dims[2]-1) * sizeof(float));
    for(k = 0; k < dims[2]-1; ++k)
    for(j = 0; j < dims[1]-1; ++j)
    for(i = 0; i < dims[0]-1; ++i)
    {
        distvar[k*(dims[1]-1)*(dims[0]-1) + j*(dims[0]-1) + i] = 
           sqrt(x[i]*x[i] + y[j]*y[j] + z[k]*z[k]);
    }
    *dist = (SimDataArray *)malloc(sizeof(SimDataArray));
    (*dist)->data = (void *)distvar;
    (*dist)->dataType = VISIT_DATATYPE_FLOAT;
    (*dist)->nComps = 1;
    (*dist)->nTuples = (dims[0]-1) * (dims[1]-1) * (dims[2]-1);
}

/******************************************************************************
 * Function: read_input_deck
 *
 * Purpose: Sets up the initial data that VisIt knows how to plot as if the 
 *          data were read from an input deck.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

void
read_input_deck(simulation_data *sim)
{
    MeshAndMetaData m_mmd = {NULL, NULL};
    SimRectilinearMesh *rmesh = NULL;
    SimDataArray *rdata = NULL;
    SimVariableMetaData *smd = NULL;
    int domain = sim->par_rank;

    /* Create the data for this domain */
    CreateRectMesh(domain, &rmesh, &rdata);

    /* Create the mesh metadata that we will report to VisIt. */
    m_mmd.mesh = rmesh;
    m_mmd.metadata = (SimMeshMetaData *)malloc(sizeof(SimMeshMetaData));
    memset(m_mmd.metadata, 0, sizeof(SimMeshMetaData));
    m_mmd.metadata->name = strdup("mesh");
    m_mmd.metadata->meshType = VISIT_MESHTYPE_RECTILINEAR;
    m_mmd.metadata->topologicalDimension = 3;
    m_mmd.metadata->spatialDimension = 3;
    m_mmd.metadata->numDomains = sim->par_size;
    m_mmd.metadata->domainTitle = strdup("domains");
    m_mmd.metadata->domainPieceName = strdup("domain");
    m_mmd.metadata->numGroups = 0;
    m_mmd.metadata->groupTitle = strdup("groups");
    m_mmd.metadata->groupPieceName = strdup("group");
    m_mmd.metadata->xUnits = strdup("cm");
    m_mmd.metadata->yUnits = strdup("cm");
    m_mmd.metadata->zUnits = strdup("cm");
    m_mmd.metadata->xLabel = strdup("Width");
    m_mmd.metadata->yLabel = strdup("Height");
    m_mmd.metadata->zLabel = strdup("Depth");
    MeshCache_add_mesh(&sim->mesh_cache, "mesh", domain, m_mmd);

    /* Add a variable to the variable cache. */
    smd = (SimVariableMetaData *)malloc(sizeof(SimVariableMetaData));
    memset(smd, 0, sizeof(SimVariableMetaData));
    smd->name = strdup("distance");
    smd->meshName = strdup("mesh");
    smd->type = VISIT_VARTYPE_SCALAR;
    smd->centering = VISIT_VARCENTERING_ZONE;
    smd->units = strdup("cm");
    VarCache_add_var(&sim->var_cache, "distance", domain, "mesh", rdata, smd);
}

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
 * Date:       Wed Mar 17 16:11:39 PDT 2010
 *
 * Modifications:
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
    else if(strcmp(cmd, "update") == 0)
    {
        VisItTimeStepChanged();
        VisItUpdatePlots();
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
void SlaveProcessCallback()
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
        int iseof = (fgets(cmd, 1000, stdin) == NULL);
        if (iseof)
        {
            sprintf(cmd, "quit");
            printf("quit\n");
        }

        if (strlen(cmd)>0 && cmd[strlen(cmd)-1] == '\n')
            cmd[strlen(cmd)-1] = '\0';
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
}

/******************************************************************************
 *
 * Function: mainloop
 *
 * Purpose: Handles the program's main event loop and dispatches events to 
 *          other functions for processing.
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 16:07:34 PDT 2010
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

                /* Read functions */
                VisItSetGetMetaData(SimGetMetaData, (void*)sim);
                VisItSetGetMesh(SimGetMesh, (void*)sim);
                VisItSetGetVariable(SimGetVariable, (void*)sim);
                VisItSetGetDomainList(SimGetDomainList, (void*)sim);

                /* Write functions */
                VisItSetWriteBegin(SimWriteBegin, (void *)sim);
                VisItSetWriteEnd(SimWriteEnd, (void *)sim);
                VisItSetWriteMesh(SimWriteMesh, (void *)sim);
                VisItSetWriteVariable(SimWriteVariable, (void*)sim);
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
 * Function:  main
 *
 * Programmer: Brad Whitlock
 * Date:       Wed Mar 17 15:57:56 PDT 2010
 *
 * Modifications:
 *
 *****************************************************************************/

int main(int argc, char **argv)
{
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
    VisItSetupEnvironment();

#ifdef PARALLEL
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
        VisItInitializeSocketAndDumpSimFile("writeback",
        "Accept data from VisIt and expose that data as new data sources",
        "/path/to/where/sim/was/started", NULL, NULL, NULL);
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


