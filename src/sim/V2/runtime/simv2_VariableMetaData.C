/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include "simv2_VariableMetaData.h"

struct VisIt_VariableMetaData : public VisIt_ObjectBase
{
    VisIt_VariableMetaData();
    virtual ~VisIt_VariableMetaData();

    std::string  name;
    std::string  meshName;
    std::string  units;
    int          centering;
    int          type;
    bool         treatAsASCII;
    bool         hideFromGUI;
    int          numComponents;
    stringVector materialNames;
};

VisIt_VariableMetaData::VisIt_VariableMetaData() : VisIt_ObjectBase(VISIT_VARIABLEMETADATA)
{
    name = "";
    meshName = "";
    units = "";
    centering = VISIT_VARCENTERING_ZONE;
    type = VISIT_VARTYPE_SCALAR;
    treatAsASCII = false;
    hideFromGUI = false;
    numComponents = 1;
}

VisIt_VariableMetaData::~VisIt_VariableMetaData()
{
}

static VisIt_VariableMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[150];
    VisIt_VariableMetaData *obj = (VisIt_VariableMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_VARIABLEMETADATA)
        {
            SNPRINTF(tmp, 150, "%s: The provided handle does not point to "
                "a VariableMetaData object. (type=%d)", fname, obj->type);
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
simv2_VariableMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_VariableMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_VariableMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getName");
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
simv2_VariableMetaData_setMeshName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for meshName");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setMeshName");
    if(obj != NULL)
    {
        obj->meshName = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getMeshName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getMeshName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getMeshName");
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
simv2_VariableMetaData_setUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for units");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setUnits");
    if(obj != NULL)
    {
        obj->units = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getUnits");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->units.size() + 1);
        strcpy(*val, obj->units.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_VariableMetaData_setCentering(visit_handle h, int val)
{
    if(val != VISIT_VARCENTERING_NODE &&
       val != VISIT_VARCENTERING_ZONE)
    {
        VisItError("The value for centering must be one of: VISIT_VARCENTERING_NODE, VISIT_VARCENTERING_ZONE");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setCentering");
    if(obj != NULL)
    {
        obj->centering = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getCentering(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getCentering: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getCentering");
    if(obj != NULL)
    {
        *val = obj->centering;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_setType(visit_handle h, int val)
{
    if(val != VISIT_VARTYPE_SCALAR &&
       val != VISIT_VARTYPE_VECTOR &&
       val != VISIT_VARTYPE_TENSOR &&
       val != VISIT_VARTYPE_SYMMETRIC_TENSOR &&
       val != VISIT_VARTYPE_MATERIAL &&
       val != VISIT_VARTYPE_MATSPECIES &&
       val != VISIT_VARTYPE_LABEL &&
       val != VISIT_VARTYPE_ARRAY &&
       val != VISIT_VARTYPE_MESH &&
       val != VISIT_VARTYPE_CURVE)
    {
        VisItError("The value for type must be one of: VISIT_VARTYPE_SCALAR, VISIT_VARTYPE_VECTOR, VISIT_VARTYPE_TENSOR, VISIT_VARTYPE_SYMMETRIC_TENSOR, VISIT_VARTYPE_MATERIAL, VISIT_VARTYPE_MATSPECIES, VISIT_VARTYPE_LABEL, VISIT_VARTYPE_ARRAY, VISIT_VARTYPE_MESH, VISIT_VARTYPE_CURVE");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setType");
    if(obj != NULL)
    {
        obj->type = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getType(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getType: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getType");
    if(obj != NULL)
    {
        *val = obj->type;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_setTreatAsASCII(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setTreatAsASCII");
    if(obj != NULL)
    {
        obj->treatAsASCII = (val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getTreatAsASCII(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getTreatAsASCII: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getTreatAsASCII");
    if(obj != NULL)
    {
        *val = obj->treatAsASCII ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_setHideFromGUI(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setHideFromGUI");
    if(obj != NULL)
    {
        obj->hideFromGUI = (val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getHideFromGUI(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getHideFromGUI: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getHideFromGUI");
    if(obj != NULL)
    {
        *val = obj->hideFromGUI ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_setNumComponents(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setNumComponents");
    if(obj != NULL)
    {
        obj->numComponents = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getNumComponents(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumComponents: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumComponents");
    if(obj != NULL)
    {
        *val = obj->numComponents;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_addMaterialName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for materialNames");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_addMaterialName");
    if(obj != NULL)
    {
        obj->materialNames.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getNumMaterialName(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumMaterialName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumMaterialName");
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
simv2_VariableMetaData_getMaterialName(visit_handle h, int i, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getMaterialName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getMaterialName");
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
simv2_VariableMetaData_check(visit_handle h)
{
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("VariableMetaData needs a name");
            return VISIT_ERROR;
        }
        if(obj->meshName == "")
        {
            VisItError("VariableMetaData needs a mesh name");
            return VISIT_ERROR;
        }
        // Check/override the number of components.
        switch(obj->type)
        {
        case VISIT_VARTYPE_SCALAR:
        case VISIT_VARTYPE_MATERIAL:
        case VISIT_VARTYPE_MATSPECIES:
        case VISIT_VARTYPE_MESH:
        case VISIT_VARTYPE_CURVE:
            obj->numComponents = 1;
            break;
        case VISIT_VARTYPE_VECTOR:
            obj->numComponents = 3;
            break;
        case VISIT_VARTYPE_TENSOR:
            obj->numComponents = 9;
            break;
        case VISIT_VARTYPE_SYMMETRIC_TENSOR:
            obj->numComponents = 6;
            break;
        case VISIT_VARTYPE_LABEL:
        case VISIT_VARTYPE_ARRAY:
            if(obj->numComponents < 1)
            {
                VisItError("VariableMetaData needs numComponents >= 1 for labels and arrays.");
                return VISIT_ERROR;
            }
            break;
        }

        retval = VISIT_OKAY;
    }
    return retval;
}

