#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "VisItDataInterface_V2P.h"
#include "simv2_DomainBoundaries.h"
#include "simv2_DomainNesting.h"
#include "simv2_VariableData.h"
#include "simv2_CSGMesh.h"
#include "simv2_CurvilinearMesh.h"
#include "simv2_PointMesh.h"
#include "simv2_RectilinearMesh.h"
#include "simv2_UnstructuredMesh.h"

#include <stdlib.h>
#include <string>
#include <snprintf.h>

#include <ImproperUseException.h>

#define ALLOC(T) (T*)calloc(1, sizeof(T))
#define FREE(PTR) if(PTR != NULL) free(PTR)

//
// Define a structure that we can use to contain all of the callback function
// pointers that the user supplied.
//
typedef struct
{
    /* Reader functions */
    int  (*cb_ActivateTimestep)(void *);
    void  *cbdata_ActivateTimestep;

    int  (*cb_GetMetaData)(VisIt_SimulationMetaData *, void *);
    void  *cbdata_GetMetaData;

    visit_handle (*cb_GetMesh)(int, const char *, void *);
    void  *cbdata_GetMesh;

    int  (*cb_GetMaterial)(int, const char *, VisIt_MaterialData *, void *);
    void  *cbdata_GetMaterial;

    int  (*cb_GetSpecies)(int, const char *, VisIt_SpeciesData *, void *);
    void  *cbdata_GetSpecies;

    visit_handle  (*cb_GetVariable)(int, const char *, void *);
    void  *cbdata_GetVariable;

    visit_handle  (*cb_GetMixedVariable)(int, const char *, void *);
    void  *cbdata_GetMixedVariable;

    int  (*cb_GetCurve)(const char *, VisIt_CurveData *, void *);
    void  *cbdata_GetCurve;

    int  (*cb_GetDomainList)(VisIt_DomainList *, void *);
    void  *cbdata_GetDomainList;

    visit_handle  (*cb_GetDomainBoundaries)(const char *, void *);
    void  *cbdata_GetDomainBoundaries;

    visit_handle  (*cb_GetDomainNesting)(const char *, void *);
    void  *cbdata_GetDomainNesting;

    /* Writer functions */
    int (*cb_WriteBegin)(const char *, void *);
    void *cbdata_WriteBegin;

    int (*cb_WriteEnd)(const char *, void *);
    void *cbdata_WriteEnd;

    int (*cb_WriteMesh)(const char *, int, visit_handle, const VisIt_MeshMetaData *, void *);
    void *cbdata_WriteMesh;

    int (*cb_WriteVariable)(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *, void *);
    void *cbdata_WriteVariable;

} data_callback_t;

static data_callback_t *visit_data_callbacks = NULL;

static data_callback_t *GetDataCallbacks()
{
    if(visit_data_callbacks == NULL)
        visit_data_callbacks = ALLOC(data_callback_t);
    return visit_data_callbacks;
}

void
DataCallbacksCleanup(void)
{
    if(visit_data_callbacks != NULL)
    {
        free(visit_data_callbacks);
        visit_data_callbacks = NULL;
    }
}

// *****************************************************************************
// Data Interface functions (Callable from libsimV2)
// *****************************************************************************

void
simv2_set_ActivateTimestep(int (*cb) (void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_ActivateTimestep = cb;
        callbacks->cbdata_ActivateTimestep = cbdata;
    }
}

void
simv2_set_GetMetaData(int (*cb) (VisIt_SimulationMetaData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMetaData = cb;
        callbacks->cbdata_GetMetaData = cbdata;
    }
}

void
simv2_set_GetMesh(visit_handle (*cb) (int, const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMesh = cb;
        callbacks->cbdata_GetMesh = cbdata;
    }
}

void
simv2_set_GetMaterial(int (*cb) (int, const char *, VisIt_MaterialData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMaterial = cb;
        callbacks->cbdata_GetMaterial = cbdata;
    }
}

void
simv2_set_GetSpecies(int (*cb) (int, const char *, VisIt_SpeciesData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetSpecies = cb;
        callbacks->cbdata_GetSpecies = cbdata;
    }
}

void
simv2_set_GetVariable(visit_handle (*cb) (int, const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetVariable = cb;
        callbacks->cbdata_GetVariable = cbdata;
    }
}

void
simv2_set_GetMixedVariable(visit_handle (*cb) (int, const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMixedVariable = cb;
        callbacks->cbdata_GetMixedVariable = cbdata;
    }
}

void
simv2_set_GetCurve(int (*cb) (const char *, VisIt_CurveData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetCurve = cb;
        callbacks->cbdata_GetCurve = cbdata;
    }
}

void
simv2_set_GetDomainList(int (*cb) (VisIt_DomainList *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetDomainList = cb;
        callbacks->cbdata_GetDomainList = cbdata;
    }
}

void
simv2_set_GetDomainBoundaries(visit_handle (*cb) (const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetDomainBoundaries = cb;
        callbacks->cbdata_GetDomainBoundaries = cbdata;
    }
}

void
simv2_set_GetDomainNesting(visit_handle (*cb) (const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetDomainNesting = cb;
        callbacks->cbdata_GetDomainNesting = cbdata;
    }
}

/* Write functions */
void
simv2_set_WriteBegin(int (*cb)(const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteBegin = cb;
        callbacks->cbdata_WriteBegin = cbdata;
    }
}

void
simv2_set_WriteEnd(int (*cb)(const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteEnd = cb;
        callbacks->cbdata_WriteEnd = cbdata;
    }
}

void
simv2_set_WriteMesh(int (*cb)(const char *, int, visit_handle, const VisIt_MeshMetaData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteMesh = cb;
        callbacks->cbdata_WriteMesh = cbdata;
    }
}

void
simv2_set_WriteVariable(int (*cb)(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteVariable = cb;
        callbacks->cbdata_WriteVariable = cbdata;
    }
}

// *****************************************************************************
// Callable from SimV2 reader
// *****************************************************************************
#include <stdio.h>
void
simv2_VariableMetaData_print(VisIt_VariableMetaData *obj, FILE *f, const char *indent)
{
    fprintf(f, "%s{\n", indent);
    fprintf(f, "%s    name = %s\n", indent, obj->name);
    fprintf(f, "%s    meshName = %s\n", indent, obj->meshName);
    fprintf(f, "%s    centering = %d\n", indent, obj->centering);
    fprintf(f, "%s    type = %d\n", indent, obj->type);
    fprintf(f, "%s    treatAsASCII = %d\n", indent, obj->treatAsASCII);
    fprintf(f, "%s}\n", indent);
}

void
simv2_SimulationMetaData_print(VisIt_SimulationMetaData *md, FILE *f)
{
    fprintf(f, "VisIt_SimulationMetaData\n{\n");
    fprintf(f, "    numVariables = %d\n", md->numVariables);
    for(int i = 0; i < md->numVariables; ++i)
    {
        fprintf(f, "    variables[%d] = ", i);
        simv2_VariableMetaData_print(&md->variables[i], f, "    ");
    }
    fprintf(f, "}\n");
}

/******************** START CHECKER FUNCTIONS *********************************/
int
simv2_VariableMetaData_check(const VisIt_VariableMetaData *obj, std::string &err)
{
    char tmp[100];
    int ret = VISIT_OKAY;
    if(obj->name == NULL)
    {
        err += "The name field was not set. ";
        ret = VISIT_ERROR;
    }
    if(obj->meshName == NULL)
    {
        err += "The meshName field was not set. ";
        ret = VISIT_ERROR;
    }
    if(obj->centering != VISIT_VARCENTERING_NODE &&
       obj->centering != VISIT_VARCENTERING_ZONE)
    {
        SNPRINTF(tmp, 100, "Invalid centering (%d). ", (int)obj->centering);
        err += tmp;
        ret = VISIT_ERROR;
    }
    if(!(obj->type >= VISIT_VARTYPE_SCALAR &&
         obj->type <= VISIT_VARTYPE_ARRAY))
    {
        SNPRINTF(tmp, 100, "Invalid type (%d). ", (int)obj->type);
        err += tmp;
        ret = VISIT_ERROR;
    }
    return ret;
}

int
simv2_SimulationMetaData_check(const VisIt_SimulationMetaData *obj, std::string &err)
{
    int i, ret = VISIT_OKAY;
    char tmp[100];

    if(obj->numVariables < 0)
    {
        err += "The number of variables is less than zero.\n";
        ret = VISIT_ERROR;
    }
    if(obj->numVariables > 0 && obj->variables == NULL)
    {
        err += "The number of variables is non-zero, yet no variable metadata has been allocated.\n";
        ret = VISIT_ERROR;
    }
    for(i = 0; i < obj->numVariables; ++i)
    {
        std::string suberr;
        if(simv2_VariableMetaData_check(&obj->variables[i], suberr) == VISIT_ERROR)
        {
            SNPRINTF(tmp, 100, "Metadata for variables[%d] contains errors:\n", i);
            err += (std::string(tmp) + suberr + "\n");
            ret = VISIT_ERROR;
        }
    }

    return ret;
}

/******************************************************************************/

// ****************************************************************************
// Method: simv2_invoke_ActivateTimeStep
//
// Purpose: 
//   This function invokes the simulation's ActivateTimeStep callback function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 16 16:12:44 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

int
simv2_invoke_ActivateTimestep(void)
{
    int retval = VISIT_OKAY;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_ActivateTimestep != NULL)
    {
        retval = (*callbacks->cb_ActivateTimestep)(callbacks->cbdata_ActivateTimestep);
    }
    return retval;
}

// ****************************************************************************
// Method: simv2_invoke_GetMetaData
//
// Purpose: 
//   This function invokes the simulation's GetMetaData callback function and
//   returns a VisIt_SimulationMetaData structure that was populated by the
//   simulation.
//
// Arguments:
//
// Returns:    
//
// Note:       This encapsulates the code to create an object ourselves,
//             pass it to the sim along with any user callback data that was
//             provided, and check the results for errors that could threaten
//             VisIt's operation.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 16:03:44 PST 2009
//
// Modifications:
//   
// ****************************************************************************

VisIt_SimulationMetaData *
simv2_invoke_GetMetaData(void)
{
    VisIt_SimulationMetaData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMetaData != NULL)
    {
        obj = ALLOC(VisIt_SimulationMetaData);
        if(obj != NULL)
        {
            std::string err("The simulation returned metadata with errors that "
                "need to be fixed before VisIt will accept the metadata. ");
            if((*callbacks->cb_GetMetaData)(obj, callbacks->cbdata_GetMetaData) == VISIT_OKAY)
            {
                if(simv2_SimulationMetaData_check(obj, err) == VISIT_ERROR)
                {
                    simv2_SimulationMetaData_free(obj);
                    EXCEPTION1(ImproperUseException, err);
                }
            }
            else
            {
                simv2_SimulationMetaData_free(obj);
                obj = NULL;
            }
        }
#if 0
        simv2_SimulationMetaData_print(obj, stdout);
#endif
    }
    return obj;
}

visit_handle
simv2_invoke_GetMesh(int dom, const char *name)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMesh != NULL)
    {
        h = (*callbacks->cb_GetMesh)(dom, name, callbacks->cbdata_GetMesh);

        if(h != VISIT_INVALID_HANDLE)
        {
            int msgno = 0, err = VISIT_ERROR;
            int objType = simv2_ObjectType(h);
            switch (objType)
            {
            case VISIT_CSG_MESH:
                err = simv2_CSGMesh_check(h);
                break;
            case VISIT_CURVILINEAR_MESH:
                err = simv2_RectilinearMesh_check(h);
                break;
            case VISIT_RECTILINEAR_MESH:
                err = simv2_RectilinearMesh_check(h);
                break;
            case VISIT_POINT_MESH:
                err = simv2_PointMesh_check(h);
                break;
            case VISIT_UNSTRUCTURED_MESH:
                err = simv2_UnstructuredMesh_check(h);
                break;
            default:
                msgno = 1;
            }

            if(err == VISIT_ERROR)
            {
                simv2_FreeObject(h);
                EXCEPTION1(ImproperUseException, 
                    (msgno == 0) ? 
                    "The mesh returned by the simulation did not pass "
                    "a consistency check."
                    : 
                    "The simulation returned a handle for an object other "
                    "than a mesh."
                );
            }
        }
    }
    return h;
}

VisIt_MaterialData *
simv2_invoke_GetMaterial(int dom, const char *name)
{
    VisIt_MaterialData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMaterial != NULL)
    {
        obj = ALLOC(VisIt_MaterialData);
        if(obj != NULL && (*callbacks->cb_GetMaterial)(dom, name, obj, callbacks->cbdata_GetMaterial) == VISIT_ERROR)
        {
            simv2_MaterialData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_SpeciesData *
simv2_invoke_GetSpecies(int dom, const char *name)
{
    VisIt_SpeciesData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetSpecies != NULL)
    {
        obj = ALLOC(VisIt_SpeciesData);
        if(obj != NULL && (*callbacks->cb_GetSpecies)(dom, name, obj, callbacks->cbdata_GetSpecies) == VISIT_ERROR)
        {
            simv2_SpeciesData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

visit_handle
simv2_invoke_GetVariable(int dom, const char *name)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetVariable != NULL)
    {
        h = (*callbacks->cb_GetVariable)(dom, name, callbacks->cbdata_GetVariable);

        if(h != VISIT_INVALID_HANDLE)
        {
            if(simv2_ObjectType(h) != VISIT_VARIABLE_DATA)
            {
                simv2_FreeObject(h);
                EXCEPTION1(ImproperUseException, 
                    "The simulation returned a handle for an object other "
                    "than a variable."
                );
            }
        }
    }
    return h;
}

visit_handle
simv2_invoke_GetMixedVariable(int dom, const char *name)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMixedVariable != NULL)
    {
        h = (*callbacks->cb_GetMixedVariable)(dom, name, callbacks->cbdata_GetMixedVariable);

        if(h != VISIT_INVALID_HANDLE)
        {
            if(simv2_ObjectType(h) != VISIT_VARIABLE_DATA)
            {
                simv2_FreeObject(h);
                EXCEPTION1(ImproperUseException, 
                    "The simulation returned a handle for an object other "
                    "than a variable."
                );
            }
        }
    }
    return h;
}

VisIt_CurveData *
simv2_invoke_GetCurve(const char *name)
{
    VisIt_CurveData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetCurve != NULL)
    {
        obj = ALLOC(VisIt_CurveData);
        if(obj != NULL && (*callbacks->cb_GetCurve)(name, obj, callbacks->cbdata_GetCurve) == VISIT_ERROR)
        {
            simv2_CurveData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_DomainList *
simv2_invoke_GetDomainList(void)
{
    VisIt_DomainList *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetDomainList != NULL)
    {
        obj = ALLOC(VisIt_DomainList);
        if(obj != NULL && (*callbacks->cb_GetDomainList)(obj, callbacks->cbdata_GetDomainList) == VISIT_ERROR)
        {
            simv2_DomainList_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

visit_handle 
simv2_invoke_GetDomainBoundaries(const char *name)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetDomainBoundaries != NULL)
    {
        h = (*callbacks->cb_GetDomainBoundaries)(name, callbacks->cbdata_GetDomainBoundaries);
    }
    return h;
}

visit_handle 
simv2_invoke_GetDomainNesting(const char *name)
{
    visit_handle h = VISIT_INVALID_HANDLE;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetDomainNesting != NULL)
    {
        h = (*callbacks->cb_GetDomainNesting)(name, callbacks->cbdata_GetDomainNesting);
    }
    return h;
}

/* Writer functions. */

int
simv2_invoke_WriteBegin(const char *name)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteBegin != NULL)
        ret = (*callbacks->cb_WriteBegin)(name, callbacks->cbdata_WriteBegin);
    return ret;
}

int
simv2_invoke_WriteEnd(const char *name)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteEnd != NULL)
        ret = (*callbacks->cb_WriteEnd)(name, callbacks->cbdata_WriteEnd);
    return ret;
}

int
simv2_invoke_WriteMesh(const char *name, int chunk, visit_handle md, const VisIt_MeshMetaData *mmd)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteMesh != NULL)
        ret = (*callbacks->cb_WriteMesh)(name, chunk, md, mmd, callbacks->cbdata_WriteMesh);
    return ret;
}

int
simv2_invoke_WriteVariable(const char *name, const char *arrName, int chunk,
    int dataType, void *values, int ntuples, int ncomponents,
    const VisIt_VariableMetaData *smd)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteVariable != NULL)
        ret = (*callbacks->cb_WriteVariable)(name, arrName, chunk, dataType, values, 
            ntuples, ncomponents, smd, callbacks->cbdata_WriteVariable);
    return ret;
}

int
simv2_ObjectType(visit_handle h)
{
    VisIt_ObjectBase *obj = VisItGetPointer(h);
    int t = -1;
    if(obj != NULL)
        t = obj->type;
    return t;
}

int
simv2_FreeObject(visit_handle h)
{
    VisIt_ObjectBase *obj = VisItGetPointer(h);
    if(obj != NULL)
    {
        // Rely on the virtual destructor
        delete obj;
        VisItFreePointer(h);
    }
}
