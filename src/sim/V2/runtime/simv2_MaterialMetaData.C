/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
simv2_MaterialMetaData_getNumMaterialName(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_MaterialMetaData_getNumMaterialName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MaterialMetaData *obj = GetObject(h, "simv2_MaterialMetaData_getNumMaterialName");
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
    if(obj != NULL && i >= 0 && i < obj->materialNames.size())
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

