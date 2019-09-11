// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VARIABLEMETADATA_H
#define VISIT_VARIABLEMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_VariableMetaData_alloc(visit_handle *obj);
int VisIt_VariableMetaData_free(visit_handle obj);
int VisIt_VariableMetaData_setName(visit_handle h, const char *);
int VisIt_VariableMetaData_getName(visit_handle h, char **);
int VisIt_VariableMetaData_setMeshName(visit_handle h, const char *);
int VisIt_VariableMetaData_getMeshName(visit_handle h, char **);
int VisIt_VariableMetaData_setUnits(visit_handle h, const char *);
int VisIt_VariableMetaData_getUnits(visit_handle h, char **);
int VisIt_VariableMetaData_setCentering(visit_handle h, int);
int VisIt_VariableMetaData_getCentering(visit_handle h, int*);
int VisIt_VariableMetaData_setType(visit_handle h, int);
int VisIt_VariableMetaData_getType(visit_handle h, int*);
int VisIt_VariableMetaData_setTreatAsASCII(visit_handle h, int);
int VisIt_VariableMetaData_getTreatAsASCII(visit_handle h, int*);
int VisIt_VariableMetaData_setHideFromGUI(visit_handle h, int);
int VisIt_VariableMetaData_getHideFromGUI(visit_handle h, int*);
int VisIt_VariableMetaData_setNumComponents(visit_handle h, int);
int VisIt_VariableMetaData_getNumComponents(visit_handle h, int*);
int VisIt_VariableMetaData_addMaterialName(visit_handle h, const char *);
int VisIt_VariableMetaData_getNumMaterialNames(visit_handle h, int *);
int VisIt_VariableMetaData_getMaterialName(visit_handle h, int, char **);
int VisIt_VariableMetaData_setEnumerationType(visit_handle h, int);
int VisIt_VariableMetaData_getEnumerationType(visit_handle h, int*);
int VisIt_VariableMetaData_addEnumGraphEdge(visit_handle h, int, int, const char *);
int VisIt_VariableMetaData_addEnumNameValue(visit_handle h, const char *, double, int *);
int VisIt_VariableMetaData_addEnumNameRange(visit_handle h, const char *, double, double, int *);
int VisIt_VariableMetaData_setEnumAlwaysExcludeValue(visit_handle h, double);
int VisIt_VariableMetaData_setEnumAlwaysExcludeRange(visit_handle h, double, double);
int VisIt_VariableMetaData_getEnumAlwaysExcludeRange(visit_handle h, double *, double *);  
int VisIt_VariableMetaData_setEnumAlwaysIncludeValue(visit_handle h, double);
int VisIt_VariableMetaData_setEnumAlwaysIncludeRange(visit_handle h, double, double);
int VisIt_VariableMetaData_getEnumAlwaysIncludeRange(visit_handle h, double *, double *);
  
#ifdef __cplusplus
}
#endif

#endif
