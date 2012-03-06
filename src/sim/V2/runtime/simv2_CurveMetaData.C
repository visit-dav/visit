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

#include "simv2_CurveMetaData.h"

struct VisIt_CurveMetaData : public VisIt_ObjectBase
{
    VisIt_CurveMetaData();
    virtual ~VisIt_CurveMetaData();

    std::string name;
    std::string xUnits;
    std::string yUnits;
    std::string xLabel;
    std::string yLabel;
};

VisIt_CurveMetaData::VisIt_CurveMetaData() : VisIt_ObjectBase(VISIT_CURVEMETADATA)
{
    name = "";
    xUnits = "";
    yUnits = "";
    xLabel = "";
    yLabel = "";
}

VisIt_CurveMetaData::~VisIt_CurveMetaData()
{
}

static VisIt_CurveMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_CurveMetaData *obj = (VisIt_CurveMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_CURVEMETADATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a CurveMetaData object.", fname);
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
simv2_CurveMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_CurveMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_CurveMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CurveMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_getName");
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
simv2_CurveMetaData_setXUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for xUnits");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_setXUnits");
    if(obj != NULL)
    {
        obj->xUnits = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_getXUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CurveMetaData_getXUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_getXUnits");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->xUnits.size() + 1);
        strcpy(*val, obj->xUnits.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_CurveMetaData_setYUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for yUnits");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_setYUnits");
    if(obj != NULL)
    {
        obj->yUnits = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_getYUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CurveMetaData_getYUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_getYUnits");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->yUnits.size() + 1);
        strcpy(*val, obj->yUnits.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_CurveMetaData_setXLabel(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for xLabel");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_setXLabel");
    if(obj != NULL)
    {
        obj->xLabel = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_getXLabel(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CurveMetaData_getXLabel: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_getXLabel");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->xLabel.size() + 1);
        strcpy(*val, obj->xLabel.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_CurveMetaData_setYLabel(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for yLabel");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_setYLabel");
    if(obj != NULL)
    {
        obj->yLabel = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_CurveMetaData_getYLabel(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_CurveMetaData_getYLabel: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_getYLabel");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->yLabel.size() + 1);
        strcpy(*val, obj->yLabel.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_CurveMetaData_check(visit_handle h)
{
    VisIt_CurveMetaData *obj = GetObject(h, "simv2_CurveMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("CurveMetaData needs a name");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

