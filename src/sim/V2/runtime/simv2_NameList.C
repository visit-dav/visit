// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_NameList.h"

struct VisIt_NameList : public VisIt_ObjectBase
{
    VisIt_NameList();
    virtual ~VisIt_NameList();

    stringVector names;
};

VisIt_NameList::VisIt_NameList() : VisIt_ObjectBase(VISIT_NAMELIST)
{
}

VisIt_NameList::~VisIt_NameList()
{
}

static VisIt_NameList *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_NameList *obj = (VisIt_NameList *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_NAMELIST)
        {
            snprintf(tmp, 100, "%s: The provided handle does not point to "
                "a NameList object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        snprintf(tmp, 100, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_NameList_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_NameList);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_NameList_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_NameList *obj = GetObject(h, "simv2_NameList_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_NameList_addName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for names");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_NameList *obj = GetObject(h, "simv2_NameList_addName");
    if(obj != NULL)
    {
        obj->names.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_NameList_getNumName(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_NameList_getNumName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_NameList *obj = GetObject(h, "simv2_NameList_getNumName");
    if(obj != NULL)
    {
        *val = obj->names.size();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_NameList_getName(visit_handle h, int i, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_NameList_getName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_NameList *obj = GetObject(h, "simv2_NameList_getName");
    if(obj != NULL && i >= 0 && i < (int)obj->names.size())
    {
        *val = (char *)malloc(obj->names[i].size() + 1);
        strcpy(*val, obj->names[i].c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

