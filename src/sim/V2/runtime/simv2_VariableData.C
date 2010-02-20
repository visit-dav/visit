#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"

typedef struct
{
    VISIT_OBJECT_HEAD

    int owner;
    int dataType;
    int nComponents;
    int nTuples;
    void *data;
} VisIt_VariableData;

static VisIt_VariableData *
GetObject(visit_handle h)
{
    VisIt_VariableData *obj = (VisIt_VariableData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(!VISIT_OBJECT_CHECK_TYPE(obj, VISIT_VARIABLE_DATA))
        {
            VisItError("The provided handle does not point to a VariableData object.");
            obj = NULL;
        }
    }
    else
    {
        VisItError("An invalid handle was provided.");
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/
int
simv2_VariableData_alloc(visit_handle *h)
{
    VisIt_VariableData *obj = VISIT_OBJECT_ALLOCATE(VisIt_VariableData);
    VISIT_OBJECT_INITIALIZE(obj, VISIT_VARIABLE_DATA);
    obj->owner = VISIT_OWNER_VISIT;
    obj->nComponents = 1;
    *h = VisItStorePointer(obj);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_VariableData_free(visit_handle h)
{
    VisIt_VariableData *obj = GetObject(h);
    if(obj != NULL)
    {
        if(obj->owner == VISIT_OWNER_VISIT &&
           obj->data != NULL)
        {
            free(obj->data);
        }
        if(obj != NULL)
            free(obj);
        VisItFreePointer(h);
    }
}

int
simv2_VariableData_setData(visit_handle h, int owner, int dataType, int nComps,
    int nTuples, void *data)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetObject(h);
    if(obj != NULL)
    {
        obj->owner = owner;
        obj->dataType = dataType; 
        obj->nComponents = nComps;
        obj->nTuples = nTuples;
        obj->data = data;

        retval = VISIT_OKAY;
    }
    return retval;
}

// C++ code that exists in the runtime that we can use in the SimV2 reader
int
simv2_VariableData_getData(visit_handle h, int &owner, int &dataType, int &nComps,
    int &nTuples, void *&data)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetObject(h);
    if(obj != NULL)
    {
        owner = obj->owner;
        dataType = obj->dataType;
        nComps = obj->nComponents;
        nTuples = obj->nTuples;
        data = obj->data;

        retval = VISIT_OKAY;
    }
    return retval;
}
