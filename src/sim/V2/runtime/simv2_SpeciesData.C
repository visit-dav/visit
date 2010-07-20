/*****************************************************************************
*
* Copyright (c) 2000 - 2010, Lawrence Livermore National Security, LLC
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
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to "
                "a SpeciesData object.", fname);
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
        VisItError("simv2_SpeciesData_setSpeciesName: Namelist object required");
        return VISIT_ERROR;
    }
    VisIt_SpeciesData *obj = GetObject(h, "simv2_SpeciesData_setSpeciesName");
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
    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("simv2_SpeciesData_setSpeciesNames: species require int data");
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
    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_FLOAT)
    {
        VisItError("simv2_SpeciesData_setSpeciesNames: speciesMF require float data");
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
    int owner, dataType, nComps, nTuples;
    void *data = NULL;
    if(simv2_VariableData_getData(val, owner, dataType, nComps, nTuples, data) ==
       VISIT_ERROR)
    {
        return VISIT_ERROR;
    }
    if(dataType != VISIT_DATATYPE_INT)
    {
        VisItError("simv2_SpeciesData_setSpeciesNames: mixedSpecies require int data");
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
