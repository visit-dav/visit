#include "VisItDataInterfaceRuntime.h"

#include <stdlib.h>
#include <string>
#include <snprintf.h>

#include <ImproperUseException.h>

#define ALLOC(T) (T*)calloc(1, sizeof(T))

//
// Define a structure that we can use to contain all of the callback function
// pointers that the user supplied.
//
typedef struct
{
    /* Reader functions */
    int  (*cb_GetMetaData)(VisIt_SimulationMetaData *, void *);
    void  *cbdata_GetMetaData;

    int  (*cb_GetMesh)(int, const char *, VisIt_MeshData *, void *);
    void  *cbdata_GetMesh;

    int  (*cb_GetMaterial)(int, const char *, VisIt_MaterialData *, void *);
    void  *cbdata_GetMaterial;

    int  (*cb_GetSpecies)(int, const char *, VisIt_SpeciesData *, void *);
    void  *cbdata_GetSpecies;

    int  (*cb_GetVariable)(int, const char *, VisIt_VariableData *, void *);
    void  *cbdata_GetVariable;

    int  (*cb_GetMixedVariable)(int, const char *, VisIt_MixedVariableData *, void *);
    void  *cbdata_GetMixedVariable;

    int  (*cb_GetCurve)(const char *, VisIt_CurveData *, void *);
    void  *cbdata_GetCurve;

    int  (*cb_GetDomainList)(VisIt_DomainList *, void *);
    void  *cbdata_GetDomainList;

    /* Writer functions */
    int (*cb_WriteBegin)(const char *, void *);
    void *cbdata_WriteBegin;

    int (*cb_WriteEnd)(const char *, void *);
    void *cbdata_WriteEnd;

    int (*cb_WriteMesh)(const char *, int, const VisIt_MeshData *, const VisIt_MeshMetaData *, void *);
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

extern "C" 
{
void visit_set_GetMetaData(int (*cb) (VisIt_SimulationMetaData *, void *), void *cbdata);
void visit_set_GetMesh(int (*cb) (int, const char *, VisIt_MeshData *, void *), void *cbdata);
void visit_set_GetMaterial(int (*cb) (int, const char *, VisIt_MaterialData *, void *), void *cbdata);
void visit_set_GetSpecies(int (*cb) (int, const char *, VisIt_SpeciesData *, void *), void *cbdata);
void visit_set_GetVariable(int (*cb) (int, const char *, VisIt_VariableData *, void *), void *cbdata);
void visit_set_GetMixedVariable(int (*cb) (int, const char *, VisIt_MixedVariableData *, void *), void *cbdata);
void visit_set_GetCurve(int (*cb) (const char *, VisIt_CurveData *, void *), void *cbdata);
void visit_set_GetDomainList(int (*cb) (VisIt_DomainList *, void *), void *cbdata);

void visit_set_WriteBegin(int (*cb)(const char *, void *), void *cbdata);
void visit_set_WriteEnd(int (*cb)(const char *, void *), void *cbdata);
void visit_set_WriteMesh(int (*cb)(const char *, int, const VisIt_MeshData *, const VisIt_MeshMetaData *, void *), void *cbdata);
void visit_set_WriteVariable(int (*cb)(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *, void *), void *cbdata);
}

void
visit_set_GetMetaData(int (*cb) (VisIt_SimulationMetaData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMetaData = cb;
        callbacks->cbdata_GetMetaData = cbdata;
    }
}

void
visit_set_GetMesh(int (*cb) (int, const char *, VisIt_MeshData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMesh = cb;
        callbacks->cbdata_GetMesh = cbdata;
    }
}

void
visit_set_GetMaterial(int (*cb) (int, const char *, VisIt_MaterialData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMaterial = cb;
        callbacks->cbdata_GetMaterial = cbdata;
    }
}

void
visit_set_GetSpecies(int (*cb) (int, const char *, VisIt_SpeciesData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetSpecies = cb;
        callbacks->cbdata_GetSpecies = cbdata;
    }
}

void
visit_set_GetVariable(int (*cb) (int, const char *, VisIt_VariableData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetVariable = cb;
        callbacks->cbdata_GetVariable = cbdata;
    }
}

void
visit_set_GetMixedVariable(int (*cb) (int, const char *, VisIt_MixedVariableData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetMixedVariable = cb;
        callbacks->cbdata_GetMixedVariable = cbdata;
    }
}

void
visit_set_GetCurve(int (*cb) (const char *, VisIt_CurveData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetCurve = cb;
        callbacks->cbdata_GetCurve = cbdata;
    }
}

void
visit_set_GetDomainList(int (*cb) (VisIt_DomainList *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_GetDomainList = cb;
        callbacks->cbdata_GetDomainList = cbdata;
    }
}

/* Write functions */
void
visit_set_WriteBegin(int (*cb)(const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteBegin = cb;
        callbacks->cbdata_WriteBegin = cbdata;
    }
}

void
visit_set_WriteEnd(int (*cb)(const char *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteEnd = cb;
        callbacks->cbdata_WriteEnd = cbdata;
    }
}

void
visit_set_WriteMesh(int (*cb)(const char *, int, const VisIt_MeshData *, const VisIt_MeshMetaData *, void *), void *cbdata)
{
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL)
    {
        callbacks->cb_WriteMesh = cb;
        callbacks->cbdata_WriteMesh = cbdata;
    }
}

void
visit_set_WriteVariable(int (*cb)(const char *, const char *, int, int, void *, int, int, const VisIt_VariableMetaData *, void *), void *cbdata)
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
VisIt_VariableMetaData_print(VisIt_VariableMetaData *obj, FILE *f, const char *indent)
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
VisIt_SimulationMetaData_print(VisIt_SimulationMetaData *md, FILE *f)
{
    fprintf(f, "VisIt_SimulationMetaData\n{\n");
    fprintf(f, "    numVariables = %d\n", md->numVariables);
    for(int i = 0; i < md->numVariables; ++i)
    {
        fprintf(f, "    variables[%d] = ", i);
        VisIt_VariableMetaData_print(&md->variables[i], f, "    ");
    }
    fprintf(f, "}\n");
}

/******************** START CHECKER FUNCTIONS *********************************/
int
VisIt_VariableMetaData_check(const VisIt_VariableMetaData *obj, std::string &err)
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
VisIt_SimulationMetaData_check(const VisIt_SimulationMetaData *obj, std::string &err)
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
        if(VisIt_VariableMetaData_check(&obj->variables[i], suberr) == VISIT_ERROR)
        {
            SNPRINTF(tmp, 100, "Metadata for variables[%d] contains errors:\n", i);
            err += (std::string(tmp) + suberr + "\n");
            ret = VISIT_ERROR;
        }
    }

    return ret;
}

int
VisIt_RectilinearMesh_check(const VisIt_RectilinearMesh *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    return ret;
}

int
VisIt_CurvilinearMesh_check(const VisIt_CurvilinearMesh *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    return ret;
}

int
VisIt_UnstructuredMesh_check(const VisIt_UnstructuredMesh *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    return ret;
}

int
VisIt_PointMesh_check(const VisIt_PointMesh *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    return ret;
}

int
VisIt_CSGMesh_check(const VisIt_CSGMesh *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    return ret;
}

int
VisIt_MeshData_check(const VisIt_MeshData *obj, std::string &err)
{
    int ret = VISIT_OKAY;
    std::string suberr;

    switch(obj->meshType)
    {
    case VISIT_MESHTYPE_RECTILINEAR:
        if(obj->rmesh == NULL)
        {
            err += "The rmesh pointer is NULL.\n";
            ret = VISIT_ERROR;
        }
        else
            ret = VisIt_RectilinearMesh_check(obj->rmesh, suberr);
        break;
    case VISIT_MESHTYPE_CURVILINEAR:
        if(obj->cmesh == NULL)
        {
            err += "The cmesh pointer is NULL.\n";
            ret = VISIT_ERROR;
        }
        else
            ret = VisIt_CurvilinearMesh_check(obj->cmesh, suberr);
        break;
    case VISIT_MESHTYPE_UNSTRUCTURED:
        if(obj->umesh == NULL)
        {
            err += "The umesh pointer is NULL.\n";
            ret = VISIT_ERROR;
        }
        else
            ret = VisIt_UnstructuredMesh_check(obj->umesh, suberr);
        break;
    case VISIT_MESHTYPE_POINT:
        if(obj->pmesh == NULL)
        {
            err += "The pmesh pointer is NULL.\n";
            ret = VISIT_ERROR;
        }
        else
            ret = VisIt_PointMesh_check(obj->pmesh, suberr);
        break;
    case VISIT_MESHTYPE_CSG:
        if(obj->csgmesh == NULL)
        {
            err += "The csgmesh pointer is NULL.\n";
            ret = VISIT_ERROR;
        }
        else
            ret = VisIt_CSGMesh_check(obj->csgmesh, suberr);
        break;
    default:
        err += "The mesh type is invalid.\n";
        ret = VISIT_ERROR;
        break;
    }

    if(ret == VISIT_ERROR)
        err += suberr;

    return ret;
}

/******************************************************************************/

// ****************************************************************************
// Method: visit_invoke_GetMetaData
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
visit_invoke_GetMetaData()
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
                if(VisIt_SimulationMetaData_check(obj, err) == VISIT_ERROR)
                {
                    VisIt_SimulationMetaData_free(obj);
                    EXCEPTION1(ImproperUseException, err);
                }
            }
            else
            {
                VisIt_SimulationMetaData_free(obj);
                obj = NULL;
            }
        }
#if 0
        VisIt_SimulationMetaData_print(obj, stdout);
#endif
    }
    return obj;
}

VisIt_MeshData *
visit_invoke_GetMesh(int dom, const char *name)
{
    VisIt_MeshData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMesh != NULL)
    {
        obj = ALLOC(VisIt_MeshData);
        if(obj != NULL)
        {
            std::string err("The simulation returned mesh data with errors that "
                "need to be fixed before VisIt will accept the data: ");
            if((*callbacks->cb_GetMesh)(dom, name, obj, callbacks->cbdata_GetMesh) == VISIT_OKAY)
            {
                if(VisIt_MeshData_check(obj, err) == VISIT_ERROR)
                {
                    VisIt_MeshData_free(obj);
                    EXCEPTION1(ImproperUseException, err);
                }
            }
            else
            {
                VisIt_MeshData_free(obj);
                obj = NULL;
            }
        }
    }
    return obj;
}

VisIt_MaterialData *
visit_invoke_GetMaterial(int dom, const char *name)
{
    VisIt_MaterialData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMaterial != NULL)
    {
        obj = ALLOC(VisIt_MaterialData);
        if(obj != NULL && (*callbacks->cb_GetMaterial)(dom, name, obj, callbacks->cbdata_GetMaterial) == VISIT_ERROR)
        {
            VisIt_MaterialData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_SpeciesData *
visit_invoke_GetSpecies(int dom, const char *name)
{
    VisIt_SpeciesData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetSpecies != NULL)
    {
        obj = ALLOC(VisIt_SpeciesData);
        if(obj != NULL && (*callbacks->cb_GetSpecies)(dom, name, obj, callbacks->cbdata_GetSpecies) == VISIT_ERROR)
        {
            VisIt_SpeciesData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_VariableData *
visit_invoke_GetVariable(int dom, const char *name)
{
    VisIt_VariableData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetVariable != NULL)
    {
        obj = ALLOC(VisIt_VariableData);
        if(obj != NULL)
        {
            obj->nComponents = 1; /* Default the number of components to 1. */
            if((*callbacks->cb_GetVariable)(dom, name, obj, callbacks->cbdata_GetVariable) == VISIT_ERROR)
            {
                VisIt_VariableData_free(obj);
                obj = NULL;
            }
        }
    }
    return obj;
}

VisIt_MixedVariableData *
visit_invoke_GetMixedVariable(int dom, const char *name)
{
    VisIt_MixedVariableData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetMixedVariable != NULL)
    {
        obj = ALLOC(VisIt_MixedVariableData);
        obj->nComponents = 1; /* Default the number of components to 1. */
        if(obj != NULL && (*callbacks->cb_GetMixedVariable)(dom, name, obj, callbacks->cbdata_GetMixedVariable) == VISIT_ERROR)
        {
            VisIt_MixedVariableData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_CurveData *
visit_invoke_GetCurve(const char *name)
{
    VisIt_CurveData *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetCurve != NULL)
    {
        obj = ALLOC(VisIt_CurveData);
        if(obj != NULL && (*callbacks->cb_GetCurve)(name, obj, callbacks->cbdata_GetCurve) == VISIT_ERROR)
        {
            VisIt_CurveData_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

VisIt_DomainList *
visit_invoke_GetDomainList()
{
    VisIt_DomainList *obj = NULL;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_GetDomainList != NULL)
    {
        obj = ALLOC(VisIt_DomainList);
        if(obj != NULL && (*callbacks->cb_GetDomainList)(obj, callbacks->cbdata_GetDomainList) == VISIT_ERROR)
        {
            VisIt_DomainList_free(obj);
            obj = NULL;
        }
    }
    return obj;
}

/* Writer functions. */

int
visit_invoke_WriteBegin(const char *name)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteBegin != NULL)
        ret = (*callbacks->cb_WriteBegin)(name, callbacks->cbdata_WriteBegin);
    return ret;
}

int
visit_invoke_WriteEnd(const char *name)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteEnd != NULL)
        ret = (*callbacks->cb_WriteEnd)(name, callbacks->cbdata_WriteEnd);
    return ret;
}

int
visit_invoke_WriteMesh(const char *name, int chunk, const VisIt_MeshData *md, const VisIt_MeshMetaData *mmd)
{
    int ret = VISIT_ERROR;
    data_callback_t *callbacks = GetDataCallbacks();
    if(callbacks != NULL && callbacks->cb_WriteMesh != NULL)
        ret = (*callbacks->cb_WriteMesh)(name, chunk, md, mmd, callbacks->cbdata_WriteMesh);
    return ret;
}

int
visit_invoke_WriteVariable(const char *name, const char *arrName, int chunk,
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
