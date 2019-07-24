// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <snprintf.h>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_CommandMetaData.h"

struct VisIt_CommandMetaData : public VisIt_ObjectBase
{
    VisIt_CommandMetaData();
    virtual ~VisIt_CommandMetaData();

    std::string name;
    int         enabled;
};

VisIt_CommandMetaData::VisIt_CommandMetaData() : VisIt_ObjectBase(VISIT_COMMANDMETADATA)
{
    name = "";
    enabled = 1;
}

VisIt_CommandMetaData::~VisIt_CommandMetaData()
{
}

static VisIt_CommandMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[150];
    VisIt_CommandMetaData *obj = (VisIt_CommandMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_COMMANDMETADATA)
        {
            SNPRINTF(tmp, 150, "%s: The provided handle does not point to "
                "a CommandMetaData object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 150, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_CommandMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_CommandMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_CommandMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CommandMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CommandMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CommandMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_getName");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->name.size() + 1);
        strcpy(*val, obj->name.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_CommandMetaData_setEnabled(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_setEnabled");
    if(obj != NULL)
    {
        obj->enabled = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CommandMetaData_getEnabled(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_CommandMetaData_getEnabled: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_getEnabled");
    if(obj != NULL)
    {
        *val = obj->enabled;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_CommandMetaData_check(visit_handle h)
{
    VisIt_CommandMetaData *obj = GetObject(h, "simv2_CommandMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("CommandMetaData needs a name");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

