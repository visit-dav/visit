/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
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
 * Date: Thu Nov 2 17:32:48 PST 2006
 *
 * Build command:
 *   gcc -o writeback -I. -I/usr/gapps/visit/1.5.4/linux-intel/libsim/V1/include -DPARALLEL -I/misc/gapps/mpich/1.2.4/Linux/serial/64/debug/include -L/usr/gapps/visit/1.5.4/linux-intel/libsim/V1/lib -L/misc/gapps/mpich/1.2.4/Linux/serial/64/debug/lib writeback.c -Wl,--export-dynamic -lsim -ldl -lmpich
 *
 * Modifications:
 *
 ******************************************************************************/

#include <VisItControlInterface_V1.h>
#include <VisItDataInterface_V1.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>

/* Is the simulation in run mode (not waiting for VisIt input) */
static int    runFlag = 0;
static int    simcycle = 0;
static double simtime = 0.;
static int    simDone = 0;

static int par_rank = 0;
static int par_size = 1;

#define VISIT_COMMAND_PROCESS 0
#define VISIT_COMMAND_SUCCESS 1
#define VISIT_COMMAND_FAILURE 2

#define VISIT_OKAY 0

int  simulation_done(void) { return simDone; }

/* SIMULATE ONE TIME STEP */
#include <unistd.h>
void simulate_one_timestep(void)
{
    ++simcycle;
    simtime += 0.0134;

    if(par_rank == 0)
    {
        printf("Simulating time step: cycle=%d, time=%lg\n", simcycle, simtime);
        sleep(1);
    }
}

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
 *** Functions to deal with structures from VisItDataInterface_V1.h
 ***
 *****************************************************************************
 *****************************************************************************/

static void FreeDataArray(VisIt_DataArray da)
{
    if (da.owner != VISIT_OWNER_SIM)
        return;

    switch (da.dataType)
    {
      case VISIT_DATATYPE_CHAR:
        free(da.cArray);
        da.cArray = NULL;
        break;
      case VISIT_DATATYPE_INT:
        free(da.iArray);
        da.iArray = NULL;
        break;
      case VISIT_DATATYPE_FLOAT:
        free(da.fArray);
        da.fArray = NULL;
        break;
      case VISIT_DATATYPE_DOUBLE:
        free(da.dArray);
        da.dArray = NULL;
        break;
    }
}

static void *
Mesh_Copy(int mesh_type, void *mesh)
{
    void *mc = NULL;
    size_t sz = 0;

    if(mesh_type == VISIT_MESHTYPE_RECTILINEAR)
    {
        VisIt_RectilinearMesh *orig = (VisIt_RectilinearMesh *)mesh;
        sz = sizeof(VisIt_RectilinearMesh);
        /* Steal the data from VisIt so the new copy will own the data. */
        orig->xcoords.owner = VISIT_OWNER_SIM;
        orig->ycoords.owner = VISIT_OWNER_SIM;
        orig->zcoords.owner = VISIT_OWNER_SIM;
    }
    else if(mesh_type == VISIT_MESHTYPE_CURVILINEAR)
    {
        VisIt_CurvilinearMesh *orig = (VisIt_CurvilinearMesh *)mesh;
        sz = sizeof(VisIt_CurvilinearMesh);
        /* Steal the data from VisIt so the new copy will own the data. */
        orig->xcoords.owner = VISIT_OWNER_SIM;
        orig->ycoords.owner = VISIT_OWNER_SIM;
        orig->zcoords.owner = VISIT_OWNER_SIM;
    }
    else if(mesh_type == VISIT_MESHTYPE_UNSTRUCTURED)
    {
        VisIt_UnstructuredMesh *orig = (VisIt_UnstructuredMesh *)mesh;
        sz = sizeof(VisIt_UnstructuredMesh);
        /* Steal the data from VisIt so the new copy will own the data. */
        orig->xcoords.owner = VISIT_OWNER_SIM;
        orig->ycoords.owner = VISIT_OWNER_SIM;
        orig->zcoords.owner = VISIT_OWNER_SIM;
        orig->connectivity.owner = VISIT_OWNER_SIM;
    }
    else if(mesh_type == VISIT_MESHTYPE_POINT)
    {
        VisIt_PointMesh *orig = (VisIt_PointMesh *)mesh;
        sz = sizeof(VisIt_PointMesh);
        /* Steal the data from VisIt so the new copy will own the data. */
        orig->xcoords.owner = VISIT_OWNER_SIM;
        orig->ycoords.owner = VISIT_OWNER_SIM;
        orig->zcoords.owner = VISIT_OWNER_SIM;
    }
    else
        return 0;

    /* Return a copy of the mesh.*/
    mc = malloc(sz);
    memcpy(mc, mesh, sz);
    return mc;
}

static void
Mesh_Free(int mesh_type, void *mesh)
{
    if(mesh_type == VISIT_MESHTYPE_RECTILINEAR)
    {
        VisIt_RectilinearMesh *m = (VisIt_RectilinearMesh *)mesh;
        FreeDataArray(m->xcoords);
        FreeDataArray(m->ycoords);
        FreeDataArray(m->zcoords);
    }
    else if(mesh_type == VISIT_MESHTYPE_CURVILINEAR)
    {
        VisIt_CurvilinearMesh *m = (VisIt_CurvilinearMesh *)mesh;
        FreeDataArray(m->xcoords);
        FreeDataArray(m->ycoords);
        FreeDataArray(m->zcoords);
    }
    else if(mesh_type == VISIT_MESHTYPE_UNSTRUCTURED)
    {
        VisIt_UnstructuredMesh *m = (VisIt_UnstructuredMesh *)mesh;
        FreeDataArray(m->xcoords);
        FreeDataArray(m->ycoords);
        FreeDataArray(m->zcoords);
        FreeDataArray(m->connectivity);
    }
    else if(mesh_type == VISIT_MESHTYPE_POINT)
    {
        VisIt_PointMesh *m = (VisIt_PointMesh *)mesh;
        FreeDataArray(m->xcoords);
        FreeDataArray(m->ycoords);
        FreeDataArray(m->zcoords);
    }
    free(mesh);
}

VisIt_MeshMetaData *
MeshMetaData_Copy(const VisIt_MeshMetaData *mmd)
{
    VisIt_MeshMetaData *newmd = (VisIt_MeshMetaData*)malloc(sizeof(VisIt_MeshMetaData));
    memcpy(newmd, mmd, sizeof(VisIt_MeshMetaData));

#define SAFE_STRDUP(ptr) (((ptr) != NULL) ? strdup(ptr) : NULL)

    /* Duplicate the strings */
    newmd->name = SAFE_STRDUP(mmd->name);
    newmd->blockTitle = SAFE_STRDUP(mmd->blockTitle);
    newmd->blockPieceName = SAFE_STRDUP(mmd->blockPieceName);
    newmd->groupTitle = SAFE_STRDUP(mmd->groupTitle);
    newmd->units = SAFE_STRDUP(mmd->units);
    newmd->xLabel = SAFE_STRDUP(mmd->xLabel);
    newmd->yLabel = SAFE_STRDUP(mmd->yLabel);
    newmd->zLabel = SAFE_STRDUP(mmd->zLabel);

    return newmd;
}


void
MeshMetaData_Free(VisIt_MeshMetaData *mmd)
{
    // Free the mesh metadata.
    free((void*)mmd->name);
    free((void*)mmd->blockTitle);
    free((void*)mmd->blockPieceName);
    free((void*)mmd->groupTitle);
    free((void*)mmd->units);
    free((void*)mmd->xLabel);
    free((void*)mmd->yLabel);
    free((void*)mmd->zLabel);
    free(mmd);
}

VisIt_ScalarMetaData *
ScalarMetaData_Copy(const VisIt_ScalarMetaData *smd)
{
    VisIt_ScalarMetaData *newmd = (VisIt_ScalarMetaData*)malloc(sizeof(VisIt_ScalarMetaData));
    memcpy(newmd, smd, sizeof(VisIt_ScalarMetaData));

    /* Duplicate the strings */
    newmd->name = strdup(smd->name);
    newmd->meshName = strdup(smd->meshName);

    return newmd;
}
void
ScalarMetaData_Free(VisIt_ScalarMetaData *smd)
{
    // Free the scalar metadata.
    free((void*)smd->name);
    free((void*)smd->meshName);
    free(smd);
}

/*****************************************************************************
 *****************************************************************************
 ***
 *** Functions to manage the mesh and variable caches that we use to store
 *** data that we give to VisIt and get from VisIt.
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

parsim_map_t mesh_cache;
parsim_map_t var_cache;

typedef struct
{
    VisIt_MeshMetaData *metadata;
    parsim_map_t        domain_meshes;
} parsim_mesh_record_t;

typedef struct
{
    void               *mesh;
    VisIt_MeshMetaData *metadata;
} MeshAndMetaData;

void
MeshCache_Init()
{
    parsim_map_create(&mesh_cache);
}

void
MeshCache_add_mesh(const char *name, int domain, MeshAndMetaData m_mmd)
{
    void *mesh_copy;
    parsim_mesh_record_t *mesh_entry;

    /* Look for an entry in the map with the same mesh name. */
    mesh_entry = (parsim_mesh_record_t*)parsim_map_get(&mesh_cache, name, key_compare_strings);
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
        parsim_map_add(&mesh_cache, key_newstring(name), mesh_entry);
    }
    else
    {
        /* Replace the existing metadata. */
        MeshMetaData_Free(mesh_entry->metadata);
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
MeshCache_get_nmeshes()
{
    return mesh_cache.nelems;
}

int
MeshCache_get_mesh_info(int i, char **name, VisIt_MeshMetaData **mmd)
{
    int ret = 0;
    if(i >= 0 && i < mesh_cache.nelems)
    {
        *name = mesh_cache.elems[i].key;
        *mmd = ((parsim_mesh_record_t *)mesh_cache.elems[i].value)->metadata;
        ret = 1;
    }
    return ret;
}

int
MeshCache_get_mesh_info2(const char *name, VisIt_MeshMetaData **mmd)
{
    int ret = 0;
    parsim_mesh_record_t *info = parsim_map_get(&mesh_cache, name, key_compare_strings);
    if(info != 0)
    {
        *mmd = info->metadata;
        ret = 1;
    }
    return ret;
}

void *
MeshCache_get_mesh(const char *name, int domain)
{
    void *ret = NULL;

    /* Look for an entry in the map with the same mesh name. */
    parsim_mesh_record_t *mesh_entry = (parsim_mesh_record_t*)parsim_map_get(
        &mesh_cache, name, key_compare_strings);

    if(mesh_entry != NULL)
    {
        ret = parsim_map_get(&mesh_entry->domain_meshes, (void*)&domain, 
            key_compare_ints);
    }

    return ret;
}

typedef struct
{
    const char *meshName;
    int   dataType;
    int   centering;
    int   ntuples;
    int   ncomps;
    void *values;
} parsim_var_t;

typedef struct
{
    VisIt_ScalarMetaData *metadata;
    parsim_map_t          domain_vars;
} parsim_var_record_t;

void
VarCache_Init()
{
    parsim_map_create(&var_cache);
}

void
VarCache_add_var(const char *name, int domain, const char *meshName, 
    int dataType, int centering, 
    int ntuples, int ncomps, void *values, VisIt_ScalarMetaData *smd)
{
    parsim_var_record_t *var_entry = NULL;

    /* Copy the incoming data. */
    size_t sz = 0;
    parsim_var_t *newvar_rec = (parsim_var_t*)malloc(sizeof(parsim_var_t));
    newvar_rec->meshName = strdup(meshName);
    newvar_rec->dataType = dataType;
    newvar_rec->centering = centering;
    newvar_rec->ntuples = ntuples;
    newvar_rec->ncomps = ncomps;
    if(dataType == VISIT_DATATYPE_CHAR)
        sz = ntuples * ncomps * sizeof(char);
    else if(dataType == VISIT_DATATYPE_INT)
        sz = ntuples * ncomps * sizeof(int);
    else if(dataType == VISIT_DATATYPE_FLOAT)
        sz = ntuples * ncomps * sizeof(float);
    else if(dataType == VISIT_DATATYPE_DOUBLE)
        sz = ntuples * ncomps * sizeof(double);
    newvar_rec->values = malloc(sz);
    memcpy(newvar_rec->values, values, sz);

    /* Look for an entry in the map with the same var name. */
    var_entry = (parsim_var_record_t*)parsim_map_get(&var_cache, 
        name, key_compare_strings);
    if(var_entry == NULL)
    {
        /* The var cache did not have an entry for the variable. Add one. */
        var_entry = (parsim_var_record_t*)malloc(sizeof(parsim_var_record_t));
        var_entry->metadata = smd;
        parsim_map_create(&var_entry->domain_vars);

        /* Add the domain to the domain map. */
        parsim_map_add(&var_entry->domain_vars, key_newint(domain), newvar_rec);

        /* Add the domain map to the var cache. */
        parsim_map_add(&var_cache, key_newstring(name), var_entry);
    }
    else
    {
        /* Replace the metadata. */
        ScalarMetaData_Free(var_entry->metadata);
        var_entry->metadata = smd;

        /* We found a var with the given name. Look for the specified domain. */
        void *domain_var = parsim_map_get(&var_entry->domain_vars, 
            (void*)&domain, key_compare_ints);

        if(domain_var == NULL)
        {
            /* Add the domain var. */
            parsim_map_add(&var_entry->domain_vars, key_newint(domain), newvar_rec);
        }
        else
        {
            /* Copy over the value in the map with the new var. */
            int i;
            for(i = 0; i < var_entry->domain_vars.nelems; ++i)
            {
                if(var_entry->domain_vars.elems[i].value == domain_var)
                {
                    parsim_var_t *var = (parsim_var_t *)var_entry->domain_vars.elems[i].value;
                    free(var->values);
                    free(var);
                    var_entry->domain_vars.elems[i].value = newvar_rec;
                    break;
                }
            }
        }
    }
}

int
VarCache_get_nvars()
{
    return var_cache.nelems;
}

int
VarCache_get_var_info(int i, char **name, VisIt_ScalarMetaData **smd)
{
    int ret = 0;
    if(i >= 0 && i < var_cache.nelems)
    {
        parsim_var_record_t *var_entry;
        var_entry = (parsim_var_record_t *)var_cache.elems[i].value;
      
        *name = (char *)var_cache.elems[i].key;
        *smd = var_entry->metadata;

        ret = 1;
    }

    return ret;
}

VisIt_ScalarData *
VarCache_get_var(const char *name, int domain)
{
    VisIt_ScalarData *scalar = NULL;

    /* Look for an entry in the map with the same mesh name. */
    parsim_var_record_t *var_entry = (parsim_var_record_t*)parsim_map_get(
        &var_cache, name, key_compare_strings);

    if(var_entry != NULL)
    {
        parsim_var_t *var = (parsim_var_t *)parsim_map_get(&var_entry->domain_vars,
           (void*)&domain, key_compare_ints);

        if(var != NULL)
        {
            scalar = (VisIt_ScalarData*)malloc(sizeof(VisIt_ScalarData));
            memset(scalar, 0, sizeof(VisIt_ScalarData));

            scalar->len = var->ntuples * var->ncomps;
            if(var->dataType == VISIT_DATATYPE_CHAR)
                scalar->data = VisIt_CreateDataArrayFromChar(VISIT_OWNER_SIM, (char*)var->values);
            else if(var->dataType == VISIT_DATATYPE_INT)
                scalar->data = VisIt_CreateDataArrayFromInt(VISIT_OWNER_SIM, (int*)var->values);
            else if(var->dataType == VISIT_DATATYPE_FLOAT)
                scalar->data = VisIt_CreateDataArrayFromFloat(VISIT_OWNER_SIM, (float*)var->values);
            else if(var->dataType == VISIT_DATATYPE_DOUBLE)
                scalar->data = VisIt_CreateDataArrayFromDouble(VISIT_OWNER_SIM, (double*)var->values);
        }
    }

    return scalar;
}

/******************************************************************************
 ******************************************************************************
 ***
 *** DATA ACCESS READER FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: parsim_GetMetaData
 *
 * Purpose: Returns metadata to VisIt based on the metadata that we have stored
 *          in our program's mesh and variable caches.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_SimulationMetaData *
parsim_GetMetaData(void)
{
    int i, index;

    /* Create a metadata object with no variables. */
    size_t sz = sizeof(VisIt_SimulationMetaData);
    VisIt_SimulationMetaData *md = 
        (VisIt_SimulationMetaData *)malloc(sz);
    memset(md, 0, sz);

    /* Set the simulation state. */
    md->currentMode = runFlag ? VISIT_SIMMODE_RUNNING : VISIT_SIMMODE_STOPPED;
    md->currentCycle = simcycle;
    md->currentTime = simtime;

    /* Look through the mesh cache and return information about
     * meshes that we know about.
     */
    md->numMeshes = MeshCache_get_nmeshes();
    sz = sizeof(VisIt_MeshMetaData) * md->numMeshes;
    md->meshes = (VisIt_MeshMetaData *)malloc(sz);
    memset(md->meshes, 0, sz);
    index = 0;
    for(i = 0; i < md->numMeshes; ++i)
    {
        char *name;
        VisIt_MeshMetaData *mmd;

        if(MeshCache_get_mesh_info(i, &name, &mmd))
        {
            /* Create a copy of mmd and copy it straight into the md->meshes 
             * array so that copy gets the pointers. Then, just free the husk
             * of mmd2.
             */
            VisIt_MeshMetaData *mmd2 = MeshMetaData_Copy(mmd);
            memcpy(&md->meshes[index], mmd2, sizeof(VisIt_MeshMetaData));
            free(mmd2);

            md->meshes[index].numBlocks = par_size;

            ++index;
        }
    }
    md->numMeshes = index;

    /* Look through the variables that we know about and expose them. */
    md->numScalars = VarCache_get_nvars();
    if(md->numScalars > 0)
    {
        sz = md->numScalars * sizeof(VisIt_ScalarMetaData);
        md->scalars = (VisIt_ScalarMetaData *)malloc(sz);
        memset(md->scalars, 0, sz);

        index = 0;
        for(i = 0; i < md->numScalars; ++i)
        {
            char *name = 0;
            VisIt_ScalarMetaData *smd;
            if(VarCache_get_var_info(i, &name, &smd))
            {
                /* Create a copy of smd and copy it straight into the md->scalars 
                 * array so that copy gets the pointers. Then, just free the husk
                 * of smd2.
                 */
                VisIt_ScalarMetaData *smd2 = ScalarMetaData_Copy(smd);
                memcpy(&md->scalars[index], smd2, sizeof(VisIt_ScalarMetaData));
                free(smd2);

                ++index;
            }
        }
    }

    /* Add some custom commands. */
    md->numGenericCommands = 3;
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

    return md;
}

/******************************************************************************
 * Function: parsim_GetMeta
 *
 * Purpose: Returns the specified mesh and domain to VisIt.
 * 
 * Notes: Since we're storing meshes in our mesh cache in VisIt's SimV1 format,
 *        we just return the meshes that we have in the cache. Most applications
 *        will need to translate from their internal data representation into
 *        the SimV1 format.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_MeshData *
parsim_GetMesh(int domain, const char *name)
{
    VisIt_MeshData *mesh = NULL;
    VisIt_MeshMetaData *mmd = NULL;
    size_t sz = sizeof(VisIt_MeshData);
    

    /* Look up the mesh name in our mesh cache to get some information
     * about the mesh.
     */
    if(MeshCache_get_mesh_info2(name, &mmd))
    {
        void *mesh_data = MeshCache_get_mesh(name, domain);
        if(mesh_data != NULL)
        {
            mesh = (VisIt_MeshData*)malloc(sizeof(VisIt_MeshData));
            memset(mesh, 0, sizeof(VisIt_MeshData));
            mesh->meshType = mmd->meshType;

            /* We found the mesh/domain that we were looking for.
             * Here's where we would translate from the sim's format
             * into the mesh formats provided by VisItDataInterface_V1.h.
             * Note that we're just returning pointers in this case
             * because this simulation uses the formats provided
             * in the header for its own internal format.
             */
            if(mmd->meshType == VISIT_MESHTYPE_RECTILINEAR)
                mesh->rmesh = (VisIt_RectilinearMesh*)mesh_data;
            else if(mmd->meshType == VISIT_MESHTYPE_CURVILINEAR)
                mesh->cmesh = (VisIt_CurvilinearMesh* )mesh_data;
            else if(mmd->meshType == VISIT_MESHTYPE_UNSTRUCTURED)
                mesh->umesh = (VisIt_UnstructuredMesh *)mesh_data;
            else if(mmd->meshType == VISIT_MESHTYPE_POINT)
                mesh->pmesh = (VisIt_PointMesh *)mesh_data;
        }
    }

    return mesh;
}

/******************************************************************************
 * Function: parsim_GetScalar
 *
 * Purpose: Returns the specified scalar from the right domain.
 *
 * Notes: This program stores its variables in VisIt's SimV1 format so we only
 *        have to return the data. Most applications will translate from their
 *        internal representation to a format that VisIt likes.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_ScalarData *
parsim_GetScalar(int domain, const char *name)
{
    return VarCache_get_var(name, domain);
}

/******************************************************************************
 * Function: parsim_GetDomainList
 *
 * Purpose: Returns the domain list to VisIt.
 *
 * Notes: This function must be provide in order for VisIt to run in parallel
 *        without crashing. This function returns a list of domains to VisIt
 *        so VisIt knows the processor where domains are allocated. This 
 *        example assumes that each processor will have 1 domain.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_DomainList *
parsim_GetDomainList()
{
    int i;
    VisIt_DomainList *dl = malloc(sizeof(VisIt_DomainList));

    dl->nTotalDomains = par_size;

    dl->nMyDomains = 1;
    dl->myDomains = VisIt_CreateDataArrayFromInt(VISIT_OWNER_VISIT,
                                                 malloc(sizeof(int)));
    dl->myDomains.iArray[0] = par_rank;

    return dl;
}

/*
 * visitCallbacks structure lets SimV1 know how to access your 
 * application's data access functions.
 */
VisIt_SimulationCallback visitCallbacks =
{
    parsim_GetMetaData,  /* GetMetaData */
    parsim_GetMesh,      /* GetMesh */
    NULL,                /* GetMaterial */
    NULL,                /* GetSpecies */
    parsim_GetScalar,    /* GetScalar */
    NULL,                /* GetCurve */
    NULL,                /* GetMixedScalar */
    parsim_GetDomainList /* GetDomainList */
};

/******************************************************************************
 ******************************************************************************
 ***
 *** DATA ACCESS WRITER FUNCTIONS
 ***
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Function: parsim_WriteBegin
 *
 * Purpose: This function is called before any data is sent to the application.
 *
 * Notes: Use this function if you need to prepare your application before VisIt
 *        sends it data.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteBegin(const char *objName)
{
    if(par_rank == 0)
    { 
        printf("Simulation preparing for %s from VisIt.\n", objName);
    }
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WriteEnd
 *
 * Purpose: This function is called after VisIt has sent all data to the application.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteEnd(const char *objName)
{
    if(par_rank == 0)
    { 
        printf("Simulation handled %s from VisIt.\n", objName);
    }
    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WriteCurvilinearMesh
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          curvilinear mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteCurvilinearMesh(const char *objName, int chunk, VisIt_CurvilinearMesh *m, 
    const VisIt_MeshMetaData *mmd)
{
    /* Create a copy of the mesh. Note that the Mesh_Copy function that we've
     * defined will steal the data arrays from the input mesh, which is okay 
     * since VisIt's writer class would only have deleted them anyway. Other
     * clients using this writer interface might translate the VisIt_CurvilinearMesh
     * into an internal representation convenient for the client.
     */
    MeshAndMetaData m_mmd;
    m_mmd.mesh = Mesh_Copy(VISIT_MESHTYPE_CURVILINEAR, m);
    m_mmd.metadata = MeshMetaData_Copy(mmd);
    if(par_rank == 0)
    { 
        printf("Simulation received domain %d of curvilinear mesh called %s from VisIt.\n",
               chunk, objName);
    }
    MeshCache_add_mesh(objName, chunk, m_mmd);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WriteRectilinearMesh
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          rectilinear mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteRectilinearMesh(const char *objName, int chunk, VisIt_RectilinearMesh *m,
    const VisIt_MeshMetaData *mmd)
{
    /* Create a copy of the mesh. Note that the Mesh_Copy function that we've
     * defined will steal the data arrays from the input mesh, which is okay 
     * since VisIt's writer class would only have deleted them anyway. Other
     * clients using this writer interface might translate the VisIt_RectilinearMesh
     * into an internal representation convenient for the client.
     */
    MeshAndMetaData m_mmd;
    m_mmd.mesh = Mesh_Copy(VISIT_MESHTYPE_RECTILINEAR, m);
    m_mmd.metadata = MeshMetaData_Copy(mmd);
    if(par_rank == 0)
    { 
        printf("Simulation received domain %d of rectilinear mesh called %s from VisIt.\n",
               chunk, objName);
    }
    MeshCache_add_mesh(objName, chunk, m_mmd);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WritePointMesh
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          point mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WritePointMesh(const char *objName, int chunk, VisIt_PointMesh *m,
    const VisIt_MeshMetaData *mmd)
{
    /* Create a copy of the mesh. Note that the Mesh_Copy function that we've
     * defined will steal the data arrays from the input mesh, which is okay 
     * since VisIt's writer class would only have deleted them anyway. Other
     * clients using this writer interface might translate the VisIt_PointMesh
     * into an internal representation convenient for the client.
     */
    MeshAndMetaData m_mmd;
    m_mmd.mesh = Mesh_Copy(VISIT_MESHTYPE_POINT, m);
    m_mmd.metadata = MeshMetaData_Copy(mmd);
    if(par_rank == 0)
    { 
        printf("Simulation received domain %d of point mesh called %s from VisIt.\n",
               chunk, objName);
    }
    MeshCache_add_mesh(objName, chunk, m_mmd);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WriteUnstructuredMesh
 *
 * Purpose: This function is called when VisIt wants to send the application an
 *          unstructured mesh.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteUnstructuredMesh(const char *objName, int chunk, VisIt_UnstructuredMesh *m,
    const VisIt_MeshMetaData *mmd)
{
    /* Create a copy of the mesh. Note that the Mesh_Copy function that we've
     * defined will steal the data arrays from the input mesh, which is okay 
     * since VisIt's writer class would only have deleted them anyway. Other
     * clients using this writer interface might translate the VisIt_UnstructuredMesh
     * into an internal representation convenient for the client.
     */
    MeshAndMetaData m_mmd;
    m_mmd.mesh = Mesh_Copy(VISIT_MESHTYPE_UNSTRUCTURED, m);
    m_mmd.metadata = MeshMetaData_Copy(mmd);
    if(par_rank == 0)
    { 
        printf("Simulation received domain %d of unstructured mesh called %s from VisIt.\n",
               chunk, objName);
    }
    MeshCache_add_mesh(objName, chunk, m_mmd);

    return VISIT_OKAY;
}

/******************************************************************************
 * Function: parsim_WriteDataArray
 *
 * Purpose: This function is called when VisIt wants to send the application a
 *          data array for the mesh that was sent previously.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

int parsim_WriteDataArray(const char *objName, const char *arrName, int chunk,
    int dataType, void *values, int ntuples, int ncomponents,
    const VisIt_ScalarMetaData *smd)
{
    VisIt_ScalarMetaData *smd2 = NULL;

    /* Create a name that VisIt will treat like a subdirectory in the GUI. This also
     * prevents the original data from getting clobbered if only the mesh changed.
     * If you want to clobber the original data then by all means do so.
     */
    char varname[200];
    sprintf(varname, "%s/%s", objName, arrName);

    if(par_rank == 0)
    { 
        printf("Simulation getting new data array %s:%s:%d from VisIt.\n", objName, arrName, chunk);
    }

    smd2 = ScalarMetaData_Copy(smd);
    free((void*)smd2->name);
    smd2->name = strdup(varname);

    /* Store the variable coming in from VisIt on the "objName" mesh. Since we are
     * adding the variable to our sim's variable cache, we're adding it to the list
     * of variables that the simulation knows about and can perform calculations on.
     * Other clients using the writer interface could translate this incoming data
     * into a representation convenient for the client.
     */

    /* Add the data to the variable cache. */
    VarCache_add_var(varname, chunk, objName, dataType, smd->centering, 
                     ntuples, ncomponents, values, smd2);

    return VISIT_OKAY;
}

/*
 * visitWriterCallbacks lets you tell VisIt which functions to call when you want
 * to export data back to your application.
 */
VisIt_SimulationWriterCallback visitWriterCallbacks =
{
    parsim_WriteBegin,            /* WriteBegin */
    parsim_WriteEnd,              /* WriteEnd */
    parsim_WriteCurvilinearMesh,  /* WriteCurvilinearMesh */
    parsim_WriteRectilinearMesh,  /* WriteRectilinearMesh */
    parsim_WritePointMesh,        /* WritePointMesh */
    parsim_WriteUnstructuredMesh, /* WriteUnstructuredMesh */
    parsim_WriteDataArray,        /* WriteDataArray */
};

/******************************************************************************/

/******************************************************************************
 * Function: CreateRectMesh
 *
 * Purpose: Creates a rectilinear mesh so the application will have a mesh to
 *          give to VisIt.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

VisIt_RectilinearMesh *
CreateRectMesh(int domain)
{
    int i;
    size_t sz;
    float *x, *y;
    /* Rectilinear mesh */
    VisIt_RectilinearMesh *rmesh = NULL;
    const float rmesh_x[] = {0., 1., 2.5, 5.};
    const float rmesh_y[] = {0., 2., 2.25, 2.55,  5.};
    const int   rmesh_dims[] = {4, 5, 1};
    const int   rmesh_ndims = 2;

    /* Make VisIt_MeshData contain a VisIt_RectilinearMesh. */
    sz = sizeof(VisIt_RectilinearMesh);
    rmesh = (VisIt_RectilinearMesh *)malloc(sz);
    memset(rmesh, 0, sz);

    /* Set the mesh's number of dimensions. */
    rmesh->ndims = rmesh_ndims;

    /* Set the mesh dimensions. */
    rmesh->dims[0] = rmesh_dims[0];
    rmesh->dims[1] = rmesh_dims[1];
    rmesh->dims[2] = rmesh_dims[2];

    rmesh->baseIndex[0] = 0;
    rmesh->baseIndex[1] = 0;
    rmesh->baseIndex[2] = 0;

    rmesh->minRealIndex[0] = 0;
    rmesh->minRealIndex[1] = 0;
    rmesh->minRealIndex[2] = 0;
    rmesh->maxRealIndex[0] = rmesh_dims[0]-1;
    rmesh->maxRealIndex[1] = rmesh_dims[1]-1;
    rmesh->maxRealIndex[2] = rmesh_dims[2]-1;

    /*Change mesh coords based on the domain #.*/
    x = (float*)malloc(4 * sizeof(float));
    for(i = 0; i < 4; ++i)
    {
        x[i] = rmesh_x[i];
        if(domain > 0)
            x[i] += (par_rank * 5.f);
    }
    y = (float*)malloc(5 * sizeof(float));
    for(i = 0; i < 5; ++i)
        y[i] = rmesh_y[i];
      
    /* Give VisIt a copy of the mesh coordinates. Since we're letting
     * VisIt own the data arrays here, it will delete when no longer needed.
     */
    rmesh->xcoords = VisIt_CreateDataArrayFromFloat(
        VISIT_OWNER_SIM, x);
    rmesh->ycoords = VisIt_CreateDataArrayFromFloat(
        VISIT_OWNER_SIM, y);

    return rmesh;
}

/******************************************************************************
 * Function: read_input_deck
 *
 * Purpose: Sets up the initial data that VisIt knows how to plot as if the 
 *          data were read from an input deck.
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:10:53 PST 2006
 *
 * Modifications:
 *
 *****************************************************************************/

void
read_input_deck()
{
    MeshAndMetaData m_mmd;
    VisIt_ScalarMetaData *smd;
    double rmesh_zc_var[] = {0.,1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,11.};
    int domain = par_rank;

    MeshCache_Init();
    VarCache_Init();

    /* Create the mesh and mesh metadata that we will report to VisIt 
     * and cache it.
     */
    m_mmd.mesh = CreateRectMesh(domain);
    m_mmd.metadata = (VisIt_MeshMetaData *)malloc(sizeof(VisIt_MeshMetaData));
    memset(m_mmd.metadata, 0, sizeof(VisIt_MeshMetaData));
    m_mmd.metadata->name = strdup("mesh");
    m_mmd.metadata->meshType = VISIT_MESHTYPE_RECTILINEAR;
    m_mmd.metadata->topologicalDimension = 2;
    m_mmd.metadata->spatialDimension = 2;
    m_mmd.metadata->numBlocks = par_size;
    m_mmd.metadata->blockTitle = strdup("domains");
    m_mmd.metadata->blockPieceName = strdup("domain");
    m_mmd.metadata->numGroups = 0;
    m_mmd.metadata->groupTitle = strdup("groups");
    m_mmd.metadata->groupPieceName = strdup("group");
    m_mmd.metadata->groupIds = NULL;
    m_mmd.metadata->units = strdup("cm");
    m_mmd.metadata->xLabel = strdup("Width");
    m_mmd.metadata->yLabel = strdup("Height");
    m_mmd.metadata->zLabel = strdup("Depth");
    MeshCache_add_mesh("mesh", domain, m_mmd);

    /* Add a variable to the variable cache. */
    smd = (VisIt_ScalarMetaData *)malloc(sizeof(VisIt_ScalarMetaData));
    smd->name = strdup("zc");
    smd->meshName = strdup("mesh");
    smd->centering = VISIT_VARCENTERING_ZONE;
    smd->treatAsASCII = 0;
    VarCache_add_var("zc", domain, "mesh",VISIT_DATATYPE_DOUBLE, VISIT_VARCENTERING_ZONE, 
                     12, 1, rmesh_zc_var, smd);
}

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
        simDone = 1;
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
                fprintf(stderr, "VisIt connected\n");
                VisItSetCommandCallback(ControlCommandCallback);
                VisItSetSlaveProcessCallback(SlaveProcessCallback);
            }
            else
                fprintf(stderr, "VisIt did not connect\n");
            break;
        case 2:
            /* VisIt wants to tell the engine something. */
            runFlag = 0;
            if(!ProcessVisItCommand())
            {
                /* Disconnect on an error or closed connection. */
                VisItDisconnect();
                /* Start running again if VisIt closes. */
                runFlag = 1;
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
 * Function:  main
 *
 * Programmer: Brad Whitlock
 * Date:       Thu Nov 2 17:27:14 PST 2006
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
        VisItInitializeSocketAndDumpSimFile("parsim",
        "Parallel C prototype simulation connects to VisIt",
        "/path/to/where/sim/was/started", NULL, NULL, NULL);
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


