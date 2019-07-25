// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <snprintf.h>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_MaterialMetaData.h"

struct VisIt_MaterialMetaData : public VisIt_ObjectBase
{
    VisIt_MaterialMetaData();
    virtual ~VisIt_MaterialMetaData();

    std::string  name;
    std::string  meshName;
    stringVector materialNames;
};

VisIt_MaterialMetaData::VisIt_MaterialMetaData() : VisIt_ObjectBase(VISIT_MATERIALMETADATA)
{
    name = "";
    meshName = "";
}

VisIt_MaterialMetaData::~VisIt_MaterialMetaData()
{
}

static VisIt_MaterialMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_MaterialMetaData *obj = (VisIt_MaterialMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_MATERIALMETADATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a MaterialMetaData object.", fname);
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 100, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_MaterialMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_MaterialMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_MaterialMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MaterialMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_getName");
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
simv2_MaterialMetaData_setMeshName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for meshName");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_setMeshName");
    if(obj != NULL)
    {
        obj->meshName = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialMetaData_getMeshName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MaterialMetaData_getMeshName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_getMeshName");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->meshName.size() + 1);
        strcpy(*val, obj->meshName.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MaterialMetaData_addMaterialName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for materialNames");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_addMaterialName");
    if(obj != NULL)
    {
        obj->materialNames.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MaterialMetaData_getNumMaterialNames(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_MaterialMetaData_getNumMaterialNames: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_getNumMaterialNames");
    if(obj != NULL)
    {
        *val = obj->materialNames.size();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MaterialMetaData_getMaterialName(visit_handle h, int i, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MaterialMetaData_getMaterialName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_getMaterialName");
    if(obj != NULL && i >= 0 && i < static_cast<int>(obj->materialNames.size()))
    {
        *val = (char *)malloc(obj->materialNames[i].size() + 1);
        strcpy(*val, obj->materialNames[i].c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MaterialMetaData_check(visit_handle h)
{
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("MaterialMetaData needs a name");
            return VISIT_ERROR;
        }
        if(obj->meshName == "")
        {
            VisItError("MaterialMetaData needs a mesh name");
            return VISIT_ERROR;
        }
        if(obj->materialNames.empty())
        {
            VisItError("MaterialMetaData needs material names");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

