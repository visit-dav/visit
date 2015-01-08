/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

#include "simv2_ExpressionMetaData.h"

struct VisIt_ExpressionMetaData : public VisIt_ObjectBase
{
    VisIt_ExpressionMetaData();
    virtual ~VisIt_ExpressionMetaData();

    std::string name;
    std::string definition;
    int         type;
};

VisIt_ExpressionMetaData::VisIt_ExpressionMetaData() : VisIt_ObjectBase(VISIT_EXPRESSIONMETADATA)
{
    name = "";
    definition = "";
    type = VISIT_VARTYPE_SCALAR;
}

VisIt_ExpressionMetaData::~VisIt_ExpressionMetaData()
{
}

static VisIt_ExpressionMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_ExpressionMetaData *obj = (VisIt_ExpressionMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_EXPRESSIONMETADATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a ExpressionMetaData object.", fname);
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
simv2_ExpressionMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_ExpressionMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_ExpressionMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_ExpressionMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_ExpressionMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_ExpressionMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_getName");
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
simv2_ExpressionMetaData_setDefinition(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for definition");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_setDefinition");
    if(obj != NULL)
    {
        obj->definition = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_ExpressionMetaData_getDefinition(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_ExpressionMetaData_getDefinition: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_getDefinition");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->definition.size() + 1);
        strcpy(*val, obj->definition.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_ExpressionMetaData_setType(visit_handle h, int val)
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
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_setType");
    if(obj != NULL)
    {
        obj->type = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_ExpressionMetaData_getType(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_ExpressionMetaData_getType: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_getType");
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
simv2_ExpressionMetaData_check(visit_handle h)
{
    VisIt_ExpressionMetaData *obj = GetObject(h, "simv2_ExpressionMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("ExpressionMetaData needs a name");
            return VISIT_ERROR;
        }
        if(obj->definition == "")
        {
            VisItError("ExpressionMetaData needs a definition");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

