/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a NameList object.", fname);
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
    if(obj != NULL && i >= 0 && i < obj->names.size())
    {
        *val = (char *)malloc(obj->names[i].size() + 1);
        strcpy(*val, obj->names[i].c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

