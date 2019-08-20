// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_SIMULATIONMETADATA_H
#define SIMV2_SIMULATIONMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_SimulationMetaData_alloc(visit_handle*);
SIMV2_API int simv2_SimulationMetaData_free(visit_handle);

SIMV2_API int simv2_SimulationMetaData_setMode(visit_handle h, int mode);
SIMV2_API int simv2_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time);
SIMV2_API int simv2_SimulationMetaData_addMesh(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addVariable(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addMaterial(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addCurve(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addExpression(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addSpecies(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addGenericCommand(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addCustomCommand(visit_handle h, visit_handle obj);
SIMV2_API int simv2_SimulationMetaData_addMessage(visit_handle h, visit_handle obj);

#ifdef __cplusplus
}
#endif

SIMV2_API int simv2_SimulationMetaData_check(visit_handle h);

// Callable from within the runtime and SimV2
SIMV2_API int simv2_SimulationMetaData_getData(visit_handle, 
                  int &mode, int &cycle, double &time);
SIMV2_API int simv2_SimulationMetaData_getNumMeshes(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getMesh(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumVariables(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getVariable(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumMaterials(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getMaterial(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumCurves(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getCurve(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumExpressions(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getExpression(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumSpecies(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getSpecies(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumGenericCommands(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getGenericCommand(visit_handle h, int i, visit_handle &obj);
SIMV2_API int simv2_SimulationMetaData_getNumCustomCommands(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getCustomCommand(visit_handle h, int i, visit_handle &obj);

SIMV2_API int simv2_SimulationMetaData_getNumMessages(visit_handle h, int &n);
SIMV2_API int simv2_SimulationMetaData_getMessage(visit_handle h, int i, visit_handle &obj);

#endif
