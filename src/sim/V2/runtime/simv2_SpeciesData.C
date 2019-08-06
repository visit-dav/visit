// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_SpeciesData.h"
#include "simv2_VariableData.h"

struct VisIt_SpeciesData : public VisIt_ObjectBase
{
    VisIt_SpeciesData();
    virtual ~VisIt_SpeciesData();

    std::vector<visit_handle> speciesNames;
    visit_handle species;
    visit_handle speciesMF;
    visit_handle mixedSpecies;
};

VisIt_SpeciesData::VisIt_SpeciesData() : VisIt_ObjectBase(VISIT_SPECIES_DATA)
{
}

VisIt_SpeciesData::~VisIt_SpeciesData()
{
    for(size_t i = 0; i < speciesNames.size(); ++i)
    {
        if(speciesNames[i] != VISIT_INVALID_HANDLE)
            simv2_FreeObject(speciesNames[i]);
    }
    if(species != VISIT_INVALID_HANDLE)
        simv2_FreeObject(species);
    if(speciesMF != VISIT_INVALID_HANDLE)
        simv2_FreeObject(speciesMF);
    if(mixedSpecies != VISIT_INVALID_HANDLE)
        simv2_FreeObject(mixedSpecies);
}

static VisIt_SpeciesData *
GetObject(visit_handle h, const char *fname)
{
    char tmp[100];
    VisIt_SpeciesData *obj = (VisIt_SpeciesData *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_SPECIES_DATA)
        {
            snprintf(tmp, 100, "%s: The provided handle does not point to "
                "a SpeciesData object.", fname);
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
simv2_SpeciesData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_SpeciesData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_SpeciesData_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_addSpeciesName(visit_handle h, visit_handle val)
{
    int retval = VISIT_ERROR;
    if(simv2_ObjectType(val) != VISIT_NAMELIST)
    {
        VisItError("simv2_SpeciesData_addSpeciesName: Namelist object required");
        return VISIT_ERROR;
    }
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_addSpeciesName");
    if(obj != NULL)
    {
        obj->speciesNames.push_back(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_setSpecies(visit_handle h, visit_handle val)
{
    // How many arrays make up the variable.
    int nArr = 1;
    if(simv2_VariableData_getNumArrays(val, &nArr) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(nArr != 1)
    {
        VisItError("species must have 1 component.");
        return VISIT_ERROR;
    }

    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("simv2_SpeciesData_setSpecies: species require int data");
        return VISIT_ERROR;        
    }

    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_setSpecies");
    if(obj != NULL)
    {
        obj->species = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_setSpeciesMF(visit_handle h, visit_handle val)
{
    // How many arrays make up the variable.
    int nArr = 1;
    if(simv2_VariableData_getNumArrays(val, &nArr) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(nArr != 1)
    {
        VisItError("speciesMF must have 1 component.");
        return VISIT_ERROR;
    }

    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_FLOAT &&
       dataType != VISIT_DATATYPE_DOUBLE)
    {
        VisItError("simv2_SpeciesData_setSpeciesMF: speciesMF requires float or double data");
        return VISIT_ERROR;        
    }

    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_setSpeciesMF");
    if(obj != NULL)
    {
        obj->speciesMF = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_setMixedSpecies(visit_handle h, visit_handle val)
{
    // How many arrays make up the variable.
    int nArr = 1;
    if(simv2_VariableData_getNumArrays(val, &nArr) == VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(nArr != 1)
    {
        VisItError("mixedSpecies must have 1 component.");
        return VISIT_ERROR;
    }

    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("simv2_SpeciesData_setMixedSpecies: mixedSpecies requires int data");
        return VISIT_ERROR;        
    }

    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_setMixedSpecies");
    if(obj != NULL)
    {
        obj->mixedSpecies = val;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_getData(visit_handle h, 
    std::vector<visit_handle> &speciesNames, visit_handle &species,
    visit_handle &speciesMF, visit_handle &mixedSpecies)
{
    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_getMixedSpecies");
    if(obj != NULL)
    {
        speciesNames = obj->speciesNames;
        species = obj->species;
        speciesMF = obj->speciesMF;
        mixedSpecies = obj->mixedSpecies;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SpeciesData_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_getMixedSpecies");
    if(obj != NULL)
    {
        if(obj->speciesNames.empty())
        {
            VisItError("The species names were not set");
            return VISIT_ERROR;
        }
        if(obj->species == VISIT_INVALID_HANDLE)
        {
            VisItError("The species values were not set");
            return VISIT_ERROR;
        }
        if(obj->speciesMF == VISIT_INVALID_HANDLE)
        {
            VisItError("The speciesMF values were not set");
            return VISIT_ERROR;
        }
        if(obj->mixedSpecies == VISIT_INVALID_HANDLE)
        {
            VisItError("The mixedSpecies values were not set");
            return VISIT_ERROR;
        }

        retval = VISIT_OKAY;
    }
    return retval;

}
