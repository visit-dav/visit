// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SIMULATIONMETADATA_H
#define VISIT_SIMULATIONMETADATA_H
#include <VisItInterfaceTypes_V2.h>

/* C-callable implementation of front end functions */
#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_SimulationMetaData_alloc(visit_handle*);
int VisIt_SimulationMetaData_free(visit_handle);

int VisIt_SimulationMetaData_setMode(visit_handle h, int mode);
int VisIt_SimulationMetaData_setCycleTime(visit_handle h, int cycle, double time);
int VisIt_SimulationMetaData_addMesh(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addVariable(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addMaterial(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addCurve(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addExpression(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addSpecies(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addGenericCommand(visit_handle h, visit_handle obj);
int VisIt_SimulationMetaData_addCustomCommand(visit_handle h, visit_handle obj);

int VisIt_SimulationMetaData_addMessage(visit_handle h, visit_handle obj);

#ifdef __cplusplus
}
#endif

#endif
