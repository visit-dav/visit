#include <snprintf.h>
#include <string.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_VariableData.h"
#include "simv2_TypeTraits.hxx"
#include <stdio.h>

struct VisIt_VariableData : public VisIt_ObjectBase
{
    VisIt_VariableData();
    virtual ~VisIt_VariableData();

    int owner;
    int dataType;
    int nComponents;
    int nTuples;
    void *data;
    void (*callback)(void *);
    void *callbackData;
};

VisIt_VariableData::VisIt_VariableData() : VisIt_ObjectBase(VISIT_VARIABLE_DATA)
{
    owner = VISIT_OWNER_VISIT;
    dataType = VISIT_DATATYPE_FLOAT;
    nComponents = 1;
    nTuples = 0;
    data = NULL;
    callback = NULL;
    callbackData = NULL;
}

VisIt_VariableData::~VisIt_VariableData()
{
    if ((owner == VISIT_OWNER_VISIT) && (data != NULL))
    {
        free(data);
    }
    else
    if ((owner == VISIT_OWNER_VISIT_EX) && (callback != NULL))
    {
        callback(callbackData);
    }
}

static VisIt_VariableData *
GetVariableDataObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_VariableData *obj = (VisIt_VariableData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_VARIABLE_DATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to a "
                "VariableData object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 100, "%s: An invalid handle was provided for a "
            "VariableData object.", fname);
        VisItError(tmp);
    }

    return obj;
}


/*******************************************************************************
 * Public functions, available to C
 ******************************************************************************/

int
simv2_VariableData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_VariableData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_VariableData_free(visit_handle h)
{
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_free");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableData_setData(visit_handle h, int owner, int dataType, int nComps,
    int nTuples, void *data)
{
    if(owner != VISIT_OWNER_SIM &&
       owner != VISIT_OWNER_VISIT &&
       owner != VISIT_OWNER_VISIT_EX &&
       owner != VISIT_OWNER_COPY)
    {
        VisItError("VariableData's owner must be set to VISIT_OWNER_SIM"
            ", VISIT_OWNER_VISIT, VISIT_OWNER_VISIT_EX, or VISIT_OWNER_COPY.");
        return VISIT_ERROR;
    }

    if(!simV2_ValidDataType(dataType))
    {
        VisItError("VariableData's data type must be set to one of: "
            "VISIT_DATATYPE_CHAR, VISIT_DATATYPE_INT, VISIT_DATATYPE_FLOAT, "
            "VISIT_DATATYPE_DOUBLE, VISIT_DATATYPE_LONG");
        return VISIT_ERROR;
    }

    if(nComps <= 0)
    {
        VisItError("VariableData's number of components must be greater than 0.");
        return VISIT_ERROR;
    }

    if(nTuples <= 0)
    {
        VisItError("VariableData's number of tuples must be greater than 0.");
        return VISIT_ERROR;
    }

    if(data == NULL)
    {
        VisItError("VariableData's data must not be NULL.");
        return VISIT_ERROR;
    }

    int realOwner = owner;
    void *realData = data;
    if(owner == VISIT_OWNER_COPY)
    {
        switch (dataType)
        {
        simV2TemplateMacro(
            size_t sz = nComps*nTuples*sizeof(simV2_TT::cppType);
            realData = malloc(sz);
            if(realData != NULL)
            {
                memcpy(realData, data, sz);
                realOwner = VISIT_OWNER_VISIT;
            }
            else
            {
                VisItError("Could not allocate memory to copy data");
                return VISIT_ERROR;
            }
            );
        }
    }

    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_setData");
    if(obj != NULL)
    {
        obj->owner = realOwner;
        obj->dataType = dataType;
        obj->nComponents = nComps;
        obj->nTuples = nTuples;
        obj->data = realData;
        if (owner != VISIT_OWNER_VISIT_EX)
        {
            obj->callback = NULL;
            obj->callbackData = NULL;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableData_setDataEx(visit_handle h, int owner, int dataType,
            int nComps, int nTuples, void *data, void (*callback)(void*),
            void *callbackData)
{

    if (owner == VISIT_OWNER_VISIT_EX)
    {
        if (callback == NULL)
        {
            VisItError("VISIT_OWNER_VISIT_EX specified "
                       "but a callback was not provided.");
            return VISIT_ERROR;
        }

        VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_setData");
        if(obj == NULL)
        {
            VisItError("Failed to locate the object from the given handle.");
            return VISIT_ERROR;
        }

        obj->callback = callback;
        obj->callbackData = callbackData;
        }
    return simv2_VariableData_setData(h, owner, dataType, nComps, nTuples, data);
}

int
simv2_VariableData_getData2(visit_handle h, int *owner, int *dataType, int *nComps,
    int *nTuples, void **data)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getData");
    if(obj != NULL)
    {
        if(obj->data == NULL)
        {
            VisItError("The data array does not contain any data");
            return VISIT_ERROR;
        }

        *owner = obj->owner;
        *dataType = obj->dataType;
        *nComps = obj->nComponents;
        *nTuples = obj->nTuples;
        *data = obj->data;

        retval = VISIT_OKAY;
    }
    return retval;
}

/*******************************************************************************
 * C++ code callable from the SimV2 plugin and within the runtime
 ******************************************************************************/

int
simv2_VariableData_getData(visit_handle h, int &owner, int &dataType, int &nComps,
    int &nTuples, void *&data)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getData");
    if(obj != NULL)
    {
        if(obj->data == NULL)
        {
            VisItError("The data array does not contain any data");
            return VISIT_ERROR;
        }

        owner = obj->owner;
        dataType = obj->dataType;
        nComps = obj->nComponents;
        nTuples = obj->nTuples;
        data = obj->data;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableData_getDataEx(visit_handle h, int &owner, int &dataType, int &nComps,
    int &nTuples, void *&data, void (*&callback)(void*), void *&callbackData)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_getDataEx");
    if(obj != NULL)
    {
        if(obj->data == NULL)
        {
            VisItError("The data array does not contain any data");
            return VISIT_ERROR;
        }

        owner = obj->owner;
        dataType = obj->dataType;
        nComps = obj->nComponents;
        nTuples = obj->nTuples;
        data = obj->data;
        callback = obj->callback;
        callbackData = obj->callbackData;

        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableData_nullData(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_VariableData *obj = GetVariableDataObject(h, "simv2_VariableData_nullData");
    if(obj != NULL)
    {
        obj->data = NULL;
        obj->nTuples = 0;
        obj->nComponents = 0;
        obj->callback = NULL;
        obj->callbackData = NULL;

        retval = VISIT_OKAY;
    }
    return retval;
}
