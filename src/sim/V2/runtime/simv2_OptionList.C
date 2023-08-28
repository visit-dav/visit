// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_OptionList.h"

struct SimV2Variant
{
    int         type;
    union
    {
        unsigned char valueC;
        int     valueI;
        long    valueL;
        float   valueF;
        double  valueD;
        int     valueE;
    } storage;
    std::string valueS;
};

void
SimV2Variant_setValue(SimV2Variant &var, int type, void *value)
{
    var.type = type;
    switch(type)
    {
    case VISIT_DATATYPE_CHAR:
        var.storage.valueC = *((unsigned char *)value);
        break;
    case VISIT_DATATYPE_INT:
        var.storage.valueI = *((int *)value);
        break;
    case VISIT_DATATYPE_LONG:
        var.storage.valueL = *((long *)value);
        break;
    case VISIT_DATATYPE_FLOAT:
        var.storage.valueF = *((float *)value);
        break;
    case VISIT_DATATYPE_DOUBLE:
        var.storage.valueD = *((double *)value);
        break;
    case VISIT_DATATYPE_STRING:
        var.valueS = std::string((const char *)value);
        break;
    case VISIT_DATATYPE_ENUM:
        var.storage.valueE = *((int *)value);
        break;
    }
}

void *
SimV2Variant_getValue(SimV2Variant &var)
{
    void *ptr = NULL;
    switch(var.type)
    {
    case VISIT_DATATYPE_CHAR:
        ptr = (void*)&var.storage.valueC;
        break;
    case VISIT_DATATYPE_INT:
        ptr = (void*)&var.storage.valueI;
        break;
    case VISIT_DATATYPE_LONG:
        ptr = (void*)&var.storage.valueL;
        break;
    case VISIT_DATATYPE_FLOAT:
        ptr = (void*)&var.storage.valueF;
        break;
    case VISIT_DATATYPE_DOUBLE:
        ptr = (void*)&var.storage.valueD;
        break;
    case VISIT_DATATYPE_STRING:
        ptr = (void*)var.valueS.c_str();
        break;
    case VISIT_DATATYPE_ENUM:
        ptr = (void*)&var.storage.valueE;
        break;
    }
    return ptr;
}

struct VisIt_OptionList : public VisIt_ObjectBase
{
    VisIt_OptionList();
    virtual ~VisIt_OptionList();

    int  index(const std::string &k) const;
    void setValue(const std::string &key, int type, void *);

    std::vector<std::string>  keys;
    std::vector<SimV2Variant> values;
};

VisIt_OptionList::VisIt_OptionList() : VisIt_ObjectBase(VISIT_OPTIONLIST), 
    keys(), values()
{
}

VisIt_OptionList::~VisIt_OptionList()
{
}

int 
VisIt_OptionList::index(const std::string &k) const
{
    for(size_t i = 0; i < keys.size(); ++i)
    {
        if(keys[i] == k)
             return i;
    }
    return -1;
}

void
VisIt_OptionList::setValue(const std::string &key, int type, void *value)
{
    int idx = index(key);
    if(idx == -1)
    {
        SimV2Variant var;
        SimV2Variant_setValue(var, type, value);
        keys.push_back(key);
        values.push_back(var);
    }
    else
    {
        SimV2Variant_setValue(values[idx], type, value);
    }
}

static VisIt_OptionList *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_OptionList *obj = (VisIt_OptionList *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_OPTIONLIST)
        {
            snprintf(tmp, 100, "%s: The provided handle does not point to "
                "a OptionList object.", fname);
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
simv2_OptionList_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_OptionList);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_OptionList_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_OptionList_setValue(visit_handle h, const char *name, int type, void *value)
{
    if(name == NULL)
    {
        VisItError("An invalid key string was provided");
        return VISIT_ERROR;
    }
    if(value == NULL)
    {
        VisItError("An invalid value was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_setValue");
    if(obj != NULL)
    {
        obj->setValue(name, type, value);
        retval = VISIT_OKAY;
    }
    return retval;  
}

int
simv2_OptionList_getNumValues(visit_handle h, int *nvalues)
{
    if(nvalues == NULL)
    {
        VisItError("An invalid return variable was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_getNumValues");
    if(obj != NULL)
    {
        *nvalues = static_cast<int>(obj->keys.size());
        retval = VISIT_OKAY;
    }
    return retval;  
}

int
simv2_OptionList_getIndex(visit_handle h, const char *name, int *index)
{
    if(name == NULL || index == NULL)
    {
        VisItError("An invalid return variable was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_getType");
    if(obj != NULL)
    {
        *index = obj->index(name);
        retval = VISIT_OKAY;
    }
    return retval; 
}

int
simv2_OptionList_getType(visit_handle h, int index, int *type)
{
    if(type == NULL)
    {
        VisItError("An invalid return variable was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_getType");
    if(obj != NULL)
    {
        if(index >= 0 && index < static_cast<int>(obj->values.size()))
        {
            *type = obj->values[index].type;
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("An invalid index was provided");
        }
    }
    return retval; 
}

int
simv2_OptionList_getName(visit_handle h, int index, char **name)
{
    if(name == NULL)
    {
        VisItError("An invalid return variable was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_getName");
    if(obj != NULL)
    {
        if(index >= 0 && index < static_cast<int>(obj->keys.size()))
        {
            *name = strdup(obj->keys[index].c_str());
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("An invalid index was provided");
        }
    }
    return retval; 
}

int
simv2_OptionList_getValue(visit_handle h, int index, void **value)
{
    if(value == NULL)
    {
        VisItError("An invalid return variable was provided");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_OptionList *obj = GetObject(h, "simv2_OptionList_getName");
    if(obj != NULL)
    {
        if(index >= 0 && index < static_cast<int>(obj->values.size()))
        {
            *value = SimV2Variant_getValue(obj->values[index]);
            retval = VISIT_OKAY;
        }
        else
        {
            VisItError("An invalid index was provided");
        }
    }
    return retval; 
}
