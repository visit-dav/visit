// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <iostream>
#include <cstring>
#include <float.h>
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

    int          enumerationType;
    stringVector enumNames;
    doubleVector enumRanges;
    double       enumAlwaysExclude[2];
    double       enumAlwaysInclude[2];
    intVector    enumGraphEdges;
    stringVector enumGraphEdgeNames;
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
    enumerationType = VISIT_ENUMTYPE_NONE;
    enumAlwaysExclude[0] = +DBL_MAX;
    enumAlwaysExclude[1] = -DBL_MAX;
    enumAlwaysInclude[0] = +DBL_MAX;
    enumAlwaysInclude[1] = -DBL_MAX;
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
            snprintf(tmp, 150, "%s: The provided handle does not point to "
                "a VariableMetaData object. (type=%d)", fname, obj->type);
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
simv2_VariableMetaData_getNumMaterialNames(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumMaterialNames: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumMaterialNames");
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
simv2_VariableMetaData_setEnumerationType(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumerationType");
    if(obj != NULL)
    {
        obj->enumerationType = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getEnumerationType(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumerationType: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumerationType");
    if(obj != NULL)
    {
        *val = obj->enumerationType;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int simv2_VariableMetaData_addEnumNameValue(visit_handle h, const char * name, double val, int * index)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_addEnumNameValue");
    if(obj != NULL)
    {
        *index = obj->enumNames.size();
        obj->enumNames.push_back(name);
        obj->enumRanges.push_back(val);
        obj->enumRanges.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_addEnumNameRange(visit_handle h, const char * name, double minVal, double maxVal, int * index)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumerationType");
    if(obj != NULL)
    {
        if (minVal > maxVal)
        {
            VisItError("simv2_VariableMetaData_addEnumNameRange: minVal > maxVal.");
            return VISIT_ERROR;
        }
        
        *index = obj->enumNames.size();
        obj->enumNames.push_back(name);
        obj->enumRanges.push_back(minVal);
        obj->enumRanges.push_back(maxVal);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getNumEnumNames(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumEnumNames: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumEnumName");
    if(obj != NULL)
    {
        *val = obj->enumNames.size();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_getEnumName(visit_handle h, int i, char **val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumName: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumName");
    if(obj != NULL && i >= 0 && i < static_cast<int>(obj->enumNames.size()))
    {
        *val = (char *)malloc(obj->enumNames[i].size() + 1);
        strcpy(*val, obj->enumNames[i].c_str());
        retval = VISIT_OKAY;
    }
    else
        *val = NULL;
    return retval;
}

int
simv2_VariableMetaData_getNumEnumNameRanges(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumEnumNameRanges: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumEnumNameRanges");
    if(obj != NULL)
    {
        *val = obj->enumRanges.size() / 2;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_getEnumNameRange(visit_handle h, int i, double * minVal, double * maxVal)
{
    if(minVal == NULL || maxVal == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumNameRange: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumNameRange");
    if(obj != NULL && i >= 0 && i < static_cast<int>(obj->enumRanges.size()/2))
    {
        *minVal = obj->enumRanges[i*2  ];
        *maxVal = obj->enumRanges[i*2+1];

        retval = VISIT_OKAY;
    }
    else
    {
        *minVal = 0;
        *maxVal = 0;
    }
    return retval;
}

int
simv2_VariableMetaData_addEnumGraphEdge(visit_handle h, int head, int tail, const char *edgeName)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_addEnumGraphEdge");
    if(obj != NULL)
    {
        obj->enumGraphEdges.push_back(head);
        obj->enumGraphEdges.push_back(tail);
        obj->enumGraphEdgeNames.push_back(edgeName);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getNumEnumGraphEdges(visit_handle h, int *val)
{
    if(val == NULL)
    {
        VisItError("simv2_VariableMetaData_getNumEnumGraphEdges: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getNumEnumGraphEdges");
    if(obj != NULL)
    {
        *val = obj->enumGraphEdges.size() / 2;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_VariableMetaData_getEnumGraphEdge(visit_handle h, int i, int * head, int * tail, char **edgeName)
{
    if(head == NULL || tail == NULL || edgeName == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumGraphEdge: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumGraphEdge");
    if(obj != NULL && i >= 0 && i < static_cast<int>(obj->enumGraphEdges.size() /2))
    {
        *head = obj->enumGraphEdges[i*2  ];
        *tail = obj->enumGraphEdges[i*2+1];
        *edgeName = (char *)malloc(obj->enumGraphEdgeNames[i].size() + 1);
        strcpy(*edgeName, obj->enumGraphEdgeNames[i].c_str());
        retval = VISIT_OKAY;
    }
    else
    {
        *head = 0;
        *tail = 0;
        *edgeName = 0;
    }    
    return retval;
}

int
simv2_VariableMetaData_setEnumAlwaysIncludeValue(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumAlwaysIncludeValue");
    if(obj != NULL)
    {
        obj->enumAlwaysInclude[0] = val;
        obj->enumAlwaysInclude[1] = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_setEnumAlwaysIncludeRange(visit_handle h, double minVal, double maxVal)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumAlwaysIncludeRange");
    if(obj != NULL)
    {
        if (minVal > maxVal)
        {
            VisItError("simv2_VariableMetaData_setEnumAlwaysIncludeRange: minVal > maxVal.");
            return VISIT_ERROR;
        }
        
        obj->enumAlwaysInclude[0] = minVal;
        obj->enumAlwaysInclude[1] = maxVal;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getEnumAlwaysIncludeRange(visit_handle h, double * minVal, double * maxVal)
{
    if(minVal == NULL || maxVal == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumAlwaysIncludeRange: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumAlwaysIncludeRange");
    if(obj != NULL )
    {
        *minVal = obj->enumAlwaysInclude[0];
        *maxVal = obj->enumAlwaysInclude[1];

        retval = VISIT_OKAY;
    }
    else
    {
        *minVal = 0;
        *maxVal = 0;
    }
    return retval;
}

int
simv2_VariableMetaData_setEnumAlwaysExcludeValue(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumAlwaysExcludeValue");
    if(obj != NULL)
    {
        obj->enumAlwaysExclude[0] = val;
        obj->enumAlwaysExclude[1] = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_setEnumAlwaysExcludeRange(visit_handle h, double minVal, double maxVal)
{
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_setEnumAlwaysExcludeRange");
    if(obj != NULL)
    {
        if (minVal > maxVal)
        {
            VisItError("simv2_VariableMetaData_setEnumAlwaysExcludeRange: minVal > maxVal.");
            return VISIT_ERROR;
        }
        
        obj->enumAlwaysExclude[0] = minVal;
        obj->enumAlwaysExclude[1] = maxVal;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_VariableMetaData_getEnumAlwaysExcludeRange(visit_handle h, double * minVal, double * maxVal)
{
    if(minVal == NULL || maxVal == NULL)
    {
        VisItError("simv2_VariableMetaData_getEnumAlwaysExcludeRange: Invalid address");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_VariableMetaData *obj = GetObject(h, "simv2_VariableMetaData_getEnumAlwaysExcludeRange");
    if(obj != NULL )
    {
        *minVal = obj->enumAlwaysExclude[0];
        *maxVal = obj->enumAlwaysExclude[1];

        retval = VISIT_OKAY;
    }
    else
    {
        *minVal = 0;
        *maxVal = 0;
    }
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
