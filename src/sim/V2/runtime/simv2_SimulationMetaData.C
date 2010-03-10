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

#include <vector>
#include <snprintf.h>
#include <algorithm>

#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include "simv2_SimulationMetaData.h"
#include "simv2_MeshMetaData.h"
#include "simv2_VariableMetaData.h"
#include "simv2_MaterialMetaData.h"
#include "simv2_CurveMetaData.h"
#include "simv2_ExpressionMetaData.h"
#include "simv2_SpeciesMetaData.h"
#include "simv2_CommandMetaData.h"

struct VisIt_SimulationMetaData : public VisIt_ObjectBase
{
    VisIt_SimulationMetaData();
    virtual ~VisIt_SimulationMetaData();

    int                       simulationMode;
    int                       cycle;
    double                    time;

    std::vector<visit_handle> meshes;
    std::vector<visit_handle> variables;
    std::vector<visit_handle> materials;
    std::vector<visit_handle> curves;
    std::vector<visit_handle> expressions;
    std::vector<visit_handle> species;

    std::vector<visit_handle> genericCommands;
    std::vector<visit_handle> customCommands;
};

VisIt_SimulationMetaData::VisIt_SimulationMetaData() : 
    VisIt_ObjectBase(VISIT_SIMULATION_METADATA),
    simulationMode(0), 
    cycle(0), time(0.),
    meshes(), variables(), materials(), curves(), expressions(), species(),
    genericCommands(), customCommands()
{
}

VisIt_SimulationMetaData::~VisIt_SimulationMetaData()
{
    size_t i;
    for(i = 0; i < meshes.size(); ++i)
        simv2_FreeObject(meshes[i]);
    for(i = 0; i < variables.size(); ++i)
        simv2_FreeObject(variables[i]);
    for(i = 0; i < materials.size(); ++i)
        simv2_FreeObject(materials[i]);
    for(i = 0; i < curves.size(); ++i)
        simv2_FreeObject(curves[i]);
    for(i = 0; i < expressions.size(); ++i)
        simv2_FreeObject(expressions[i]);
    for(i = 0; i < species.size(); ++i)
        simv2_FreeObject(species[i]);

    for(i = 0; i < genericCommands.size(); ++i)
        simv2_FreeObject(genericCommands[i]);
    for(i = 0; i < customCommands.size(); ++i)
        simv2_FreeObject(customCommands[i]);
}

static VisIt_SimulationMetaData *
GetObject(visit_handle h, const char *fname)
{
    VisIt_SimulationMetaData *obj = (VisIt_SimulationMetaData *)VisItGetPointer(h);
    char tmp[100];
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_SIMULATION_METADATA)
        {
            SNPRINTF(tmp, 100, "%s: The provided handle does not point to a SimulationMetaData object.", fname);
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
simv2_SimulationMetaData_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_SimulationMetaData);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_SimulationMetaData_free(visit_handle h)
{
    VisIt_SimulationMetaData *obj = GetObject(h, "simv2_SimulationMetaData_free");
    int retval = VISIT_ERROR;
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SimulationMetaData_setMode(visit_handle h, int mode)
{
    if(mode != VISIT_SIMMODE_UNKNOWN &&
       mode != VISIT_SIMMODE_RUNNING &&
       mode != VISIT_SIMMODE_STOPPED)
    {
        VisItError("Simulation mode must be one of:VISIT_SIMMODE_UNKNOWN, "
            "VISIT_SIMMODE_RUNNING, VISIT_SIMMODE_STOPPED");
        return VISIT_ERROR;
    }
    int retval = VISIT_ERROR;
    VisIt_SimulationMetaData *obj = GetObject(h, "simv2_SimulationMetaData_setMode");
    if(obj != NULL)
    {
        obj->simulationMode = mode;
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time)
{
    int retval = VISIT_ERROR;
    VisIt_SimulationMetaData *obj = GetObject(h, "simv2_SimulationMetaData_setCycleTime");
    if(obj != NULL)
    {
        obj->cycle = cycle;
        obj->time = time; 
        retval = VISIT_OKAY;
    }
    return retval;
}

#define ADD_METADATA(FUNC, EXPECTED_TYPE, OBJNAME, VEC) \
int \
FUNC(visit_handle h, visit_handle obj)\
{\
    int retval = VISIT_ERROR; \
    VisIt_SimulationMetaData *md = GetObject(h, #FUNC); \
    if(md != NULL) \
    { \
        if(simv2_ObjectType(obj) != EXPECTED_TYPE)\
        {\
            VisItError("An attempt was made to add a handle of a type other than " OBJNAME " to SimulationMetaData"); \
            return VISIT_ERROR;\
        }\
        if(std::find(md->VEC.begin(), md->VEC.end(), obj) == md->VEC.end()) \
            md->VEC.push_back(obj);\
        retval = VISIT_OKAY;\
    }\
    return retval; \
}

ADD_METADATA(simv2_SimulationMetaData_addMesh, VISIT_MESHMETADATA, "MeshMetaData", meshes)
ADD_METADATA(simv2_SimulationMetaData_addVariable, VISIT_VARIABLEMETADATA, "VariableMetaData", variables)
ADD_METADATA(simv2_SimulationMetaData_addMaterial, VISIT_MATERIALMETADATA, "MaterialMetaData", materials)
ADD_METADATA(simv2_SimulationMetaData_addCurve, VISIT_CURVEMETADATA, "CurveMetaData", curves)
ADD_METADATA(simv2_SimulationMetaData_addExpression, VISIT_EXPRESSIONMETADATA, "ExpressionMetaData", expressions)
ADD_METADATA(simv2_SimulationMetaData_addSpecies, VISIT_SPECIESMETADATA, "SpeciesMetaData", species)
ADD_METADATA(simv2_SimulationMetaData_addGenericCommand, VISIT_COMMANDMETADATA, "CommandMetaData", genericCommands)
ADD_METADATA(simv2_SimulationMetaData_addCustomCommand, VISIT_COMMANDMETADATA, "CommandMetaData", customCommands)


// C++ code that exists in the runtime that we can use in the SimV2 reader
int
simv2_SimulationMetaData_getData(visit_handle h, int &simulationMode, int &cycle, double &time)
{
    int retval = VISIT_ERROR;
    VisIt_SimulationMetaData *obj = GetObject(h, "simv2_SimulationMetaData_getData");
    if(obj != NULL)
    {
        simulationMode = obj->simulationMode;
        cycle = obj->cycle;
        time = obj->time;
        retval = VISIT_OKAY;
    }
    return retval;
}

#define GET_METADATA_ITEM(FUNC1, FUNC2, VEC) \
int \
FUNC1(visit_handle h, int &n) \
{ \
    int retval = VISIT_ERROR; \
    VisIt_SimulationMetaData *obj = GetObject(h, #FUNC1); \
    if(obj != NULL) \
    { \
        n = obj->VEC.size(); \
        retval = VISIT_OKAY; \
    } \
    return retval; \
} \
int \
FUNC2(visit_handle h, int i, visit_handle &val) \
{ \
    int retval = VISIT_ERROR; \
    VisIt_SimulationMetaData *obj = GetObject(h, #FUNC2); \
    if(obj != NULL) \
    { \
        if(i < 0 || i >= obj->VEC.size()) \
        { \
            VisItError("An invalid index was provided"); \
            return VISIT_ERROR; \
        } \
        val = obj->VEC[i]; \
        retval = VISIT_OKAY; \
    } \
    return retval; \
}

GET_METADATA_ITEM(simv2_SimulationMetaData_getNumMeshes, simv2_SimulationMetaData_getMesh, meshes)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumVariables, simv2_SimulationMetaData_getVariable, variables)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumMaterials, simv2_SimulationMetaData_getMaterial, materials)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumCurves, simv2_SimulationMetaData_getCurve, curves)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumExpressions, simv2_SimulationMetaData_getExpression, expressions)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumSpecies, simv2_SimulationMetaData_getSpecies, species)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumGenericCommands, simv2_SimulationMetaData_getGenericCommand, genericCommands)
GET_METADATA_ITEM(simv2_SimulationMetaData_getNumCustomCommands, simv2_SimulationMetaData_getCustomCommand, customCommands)

int
simv2_SimulationMetaData_check(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_SimulationMetaData *obj = GetObject(h, "simv2_SimulationMetaData_check"); 
    if(obj != NULL) 
    { 
        size_t i;
        retval = VISIT_OKAY;
        for(i = 0; i < obj->meshes.size(); ++i)
        {
            if(simv2_MeshMetaData_check(obj->meshes[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->variables.size(); ++i)
        {
            if(simv2_VariableMetaData_check(obj->variables[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->materials.size(); ++i)
        {
            if(simv2_MaterialMetaData_check(obj->materials[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->curves.size(); ++i)
        {
            if(simv2_CurveMetaData_check(obj->curves[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->expressions.size(); ++i)
        {
            if(simv2_ExpressionMetaData_check(obj->expressions[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->species.size(); ++i)
        {
            if(simv2_SpeciesMetaData_check(obj->species[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->genericCommands.size(); ++i)
        {
            if(simv2_CommandMetaData_check(obj->genericCommands[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
        for(i = 0; i < obj->customCommands.size(); ++i)
        {
            if(simv2_CommandMetaData_check(obj->customCommands[i]) == VISIT_ERROR)
                retval = VISIT_ERROR;
        }
    }
    return retval;
}
