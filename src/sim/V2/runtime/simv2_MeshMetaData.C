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

#include "simv2_MeshMetaData.h"

struct VisIt_MeshMetaData : public VisIt_ObjectBase
{
    VisIt_MeshMetaData();
    virtual ~VisIt_MeshMetaData();

    std::string  name;
    int          meshType;
    int          topologicalDimension;
    int          spatialDimension;
    int          numDomains;
    std::string  domainTitle;
    std::string  domainPieceName;
    stringVector domainNames;
    int          numGroups;
    std::string  groupTitle;
    std::string  groupPieceName;
    intVector    groupIds;
    std::string  xUnits;
    std::string  yUnits;
    std::string  zUnits;
    std::string  xLabel;
    std::string  yLabel;
    std::string  zLabel;
    int          cellOrigin;
    int          nodeOrigin;
    bool         hasSpatialExtents;
    double       spatialExtents[6];
};

VisIt_MeshMetaData::VisIt_MeshMetaData() : VisIt_ObjectBase(VISIT_MESHMETADATA)
{
    name = "";
    meshType = VISIT_MESHTYPE_UNKNOWN;
    topologicalDimension = 2;
    spatialDimension = 2;
    numDomains = 1;
    domainTitle = "Domains";
    domainPieceName = "domain";
    numGroups = 0;
    groupTitle = "";
    groupPieceName = "group";
    xUnits = "";
    yUnits = "";
    zUnits = "";
    xLabel = "Width";
    yLabel = "Height";
    zLabel = "Depth";
    cellOrigin = 0;
    nodeOrigin = 0;
    hasSpatialExtents = false;
    spatialExtents[0] = 0.;
    spatialExtents[1] = 0.;
    spatialExtents[2] = 0.;
    spatialExtents[3] = 0.;
    spatialExtents[4] = 0.;
    spatialExtents[5] = 0.;
}

VisIt_MeshMetaData::~VisIt_MeshMetaData()
{
}

static VisIt_MeshMetaData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_MeshMetaData *obj = (VisIt_MeshMetaData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_MESHMETADATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a MeshMetaData object.", fname);
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
simv2_MeshMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_MeshMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_MeshMetaData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_setName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for name");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setName");
    if(obj != NULL)
    {
        obj->name = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getName");
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
simv2_MeshMetaData_setMeshType(visit_handle h, int val)
{
    if(val != VISIT_MESHTYPE_UNKNOWN &&
       val != VISIT_MESHTYPE_RECTILINEAR &&
       val != VISIT_MESHTYPE_CURVILINEAR &&
       val != VISIT_MESHTYPE_UNSTRUCTURED &&
       val != VISIT_MESHTYPE_POINT &&
       val != VISIT_MESHTYPE_CSG &&
       val != VISIT_MESHTYPE_AMR)
    {
        VisItError("The value for meshType must be one of: VISIT_MESHTYPE_UNKNOWN, VISIT_MESHTYPE_RECTILINEAR, VISIT_MESHTYPE_CURVILINEAR, VISIT_MESHTYPE_UNSTRUCTURED, VISIT_MESHTYPE_POINT, VISIT_MESHTYPE_CSG, VISIT_MESHTYPE_AMR");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setMeshType");
    if(obj != NULL)
    {
        obj->meshType = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getMeshType(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getMeshType: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getMeshType");
    if(obj != NULL)
    {
        *val = obj->meshType;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setTopologicalDimension(visit_handle h, int val)
{
    if(val < 0 || val > 3)
    {
        VisItError("Topological dimensions must be in [0,3]");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setTopologicalDimension");
    if(obj != NULL)
    {
        obj->topologicalDimension = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getTopologicalDimension(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getTopologicalDimension: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getTopologicalDimension");
    if(obj != NULL)
    {
        *val = obj->topologicalDimension;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setSpatialDimension(visit_handle h, int val)
{
    if(val < 0 || val > 3)
    {
        VisItError("Spatial dimensions must be in [0,3]");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setSpatialDimension");
    if(obj != NULL)
    {
        obj->spatialDimension = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getSpatialDimension(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getSpatialDimension: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getSpatialDimension");
    if(obj != NULL)
    {
        *val = obj->spatialDimension;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setNumDomains(visit_handle h, int val)
{
    if(val < 1)
    {
        VisItError("Number of domains must be greater than zero");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setNumDomains");
    if(obj != NULL)
    {
        obj->numDomains = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getNumDomains(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getNumDomains: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getNumDomains");
    if(obj != NULL)
    {
        *val = obj->numDomains;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setDomainTitle(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for domainTitle");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setDomainTitle");
    if(obj != NULL)
    {
        obj->domainTitle = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getDomainTitle(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getDomainTitle: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getDomainTitle");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->domainTitle.size() + 1);
        strcpy(*val, obj->domainTitle.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_setDomainPieceName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for domainPieceName");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setDomainPieceName");
    if(obj != NULL)
    {
        obj->domainPieceName = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getDomainPieceName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getDomainPieceName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getDomainPieceName");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->domainPieceName.size() + 1);
        strcpy(*val, obj->domainPieceName.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_addDomainName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for domainNames");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_addDomainName");
    if(obj != NULL)
    {
        obj->domainNames.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getNumDomainName(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getNumDomainName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getNumDomainName");
    if(obj != NULL)
    {
        *val = obj->domainNames.size();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_getDomainName(visit_handle h, int i, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getDomainName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getDomainName");
    if(obj != NULL && i >= 0 && i < obj->domainNames.size())
    {
        *val = (char *)malloc(obj->domainNames[i].size() + 1);
        strcpy(*val, obj->domainNames[i].c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_setNumGroups(visit_handle h, int val)
{
    if(val < 0)
    {
        VisItError("Number of groups must be greater than zero");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setNumGroups");
    if(obj != NULL)
    {
        obj->numGroups = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getNumGroups(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getNumGroups: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getNumGroups");
    if(obj != NULL)
    {
        *val = obj->numGroups;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setGroupTitle(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for groupTitle");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setGroupTitle");
    if(obj != NULL)
    {
        obj->groupTitle = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getGroupTitle(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getGroupTitle: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getGroupTitle");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->groupTitle.size() + 1);
        strcpy(*val, obj->groupTitle.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_setGroupPieceName(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for groupPieceName");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setGroupPieceName");
    if(obj != NULL)
    {
        obj->groupPieceName = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getGroupPieceName(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getGroupPieceName: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getGroupPieceName");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->groupPieceName.size() + 1);
        strcpy(*val, obj->groupPieceName.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_addGroupId(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_addGroupId");
    if(obj != NULL)
    {
        obj->groupIds.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getNumGroupId(visit_handle h, int *n)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getNumGroupId");
    if(obj != NULL && n != NULL)
    {
        *n = obj->groupIds.size();
        retval = VISIT_OKAY;
    }
    else
        *n = 0;
    return retval;
}

int
simv2_MeshMetaData_getGroupId(visit_handle h, int i, int *val)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getGroupId");
    if(obj != NULL && i >= 0 && i < obj->groupIds.size())
    {
        *val = obj->groupIds[i];
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setXUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for xUnits");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setXUnits");
    if(obj != NULL)
    {
        obj->xUnits = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getXUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getXUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getXUnits");
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
simv2_MeshMetaData_setYUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for yUnits");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setYUnits");
    if(obj != NULL)
    {
        obj->yUnits = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getYUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getYUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getYUnits");
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
simv2_MeshMetaData_setZUnits(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for zUnits");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setZUnits");
    if(obj != NULL)
    {
        obj->zUnits = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getZUnits(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getZUnits: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getZUnits");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->zUnits.size() + 1);
        strcpy(*val, obj->zUnits.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_setXLabel(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for xLabel");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setXLabel");
    if(obj != NULL)
    {
        obj->xLabel = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getXLabel(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getXLabel: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getXLabel");
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
simv2_MeshMetaData_setYLabel(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for yLabel");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setYLabel");
    if(obj != NULL)
    {
        obj->yLabel = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getYLabel(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getYLabel: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getYLabel");
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
simv2_MeshMetaData_setZLabel(visit_handle h, const char *val)
{
    if(val == NULL)
    {
        VisItError("An invalid string was provided for zLabel");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setZLabel");
    if(obj != NULL)
    {
        obj->zLabel = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getZLabel(visit_handle h, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getZLabel: Invalid address.");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getZLabel");
    if(obj != NULL)
    {
        *val = (char*)malloc(obj->zLabel.size() + 1);
        strcpy(*val, obj->zLabel.c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_MeshMetaData_setCellOrigin(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setCellOrigin");
    if(obj != NULL)
    {
        obj->cellOrigin = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getCellOrigin(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getCellOrigin: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getCellOrigin");
    if(obj != NULL)
    {
        *val = obj->cellOrigin;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setNodeOrigin(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setNodeOrigin");
    if(obj != NULL)
    {
        obj->nodeOrigin = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getNodeOrigin(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getNodeOrigin: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getNodeOrigin");
    if(obj != NULL)
    {
        *val = obj->nodeOrigin;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setHasSpatialExtents(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setHasSpatialExtents");
    if(obj != NULL)
    {
        obj->hasSpatialExtents = (val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getHasSpatialExtents(visit_handle h, int *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getHasSpatialExtents: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getHasSpatialExtents");
    if(obj != NULL)
    {
        *val = obj->hasSpatialExtents ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_setSpatialExtents(visit_handle h, double val[6])
{
    int retval = VISIT_ERROR;
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_setSpatialExtents");
    if(obj != NULL)
    {
        obj->hasSpatialExtents = true;
        memcpy((void *)obj->spatialExtents, (void *)val, 6 * sizeof(double));
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_MeshMetaData_getSpatialExtents(visit_handle h, double val[6])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_MeshMetaData_getSpatialExtents: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_getSpatialExtents");
    if(obj != NULL)
    {
        memcpy((void *)val, (void *)obj->spatialExtents, 6 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_MeshMetaData_check(visit_handle h)
{
    VisIt_MeshMetaData *obj = GetObject(h, "simv2_MeshMetaData_check");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        if(obj->name == "")
        {
            VisItError("MeshMetaData needs a name");
            return VISIT_ERROR;
        }
        if(obj->meshType == VISIT_MESHTYPE_UNKNOWN)
        {
            VisItError("MeshMetaData needs a valid mesh type");
            return VISIT_ERROR;
        }
        retval = VISIT_OKAY;
    }
    return retval;
}

