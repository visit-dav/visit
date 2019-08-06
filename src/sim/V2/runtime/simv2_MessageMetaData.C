// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_MessageMetaData.h"

struct VisIt_MessageMetaData : public VisIt_ObjectBase
{
    VisIt_MessageMetaData();
    virtual ~VisIt_MessageMetaData();

    std::string name;
};

VisIt_MessageMetaData::VisIt_MessageMetaData() : VisIt_ObjectBase(VISIT_MESSAGEMETADATA)
{
    name = "";
}

VisIt_MessageMetaData::~VisIt_MessageMetaData()
{
}

static VisIt_MessageMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[150];
    VisIt_MessageMetaData *obj = (VisIt_MessageMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_MESSAGEMETADATA)
        {
            snprintf(tmp, 150, "%s: The provided handle does not point to "
                "a MessageMetaData object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        snprintf(tmp, 150, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_MessageMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_MessageMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_MessageMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_MessageMetaData *obj = GetObject(h, "simv2_MessageMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MessageMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MessageMetaData *obj = GetObject(h, "simv2_MessageMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MessageMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MessageMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MessageMetaData *obj = GetObject(h, "simv2_MessageMetaData_getName");
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
simv2_MessageMetaData_check(visit_handle h)
{
    VisIt_MessageMetaData *obj = GetObject(h, "simv2_MessageMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("MessageMetaData needs a string");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

