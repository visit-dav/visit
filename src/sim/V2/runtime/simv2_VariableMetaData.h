// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_VARIABLEMETADATA_H
#define SIMV2_VARIABLEMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_VariableMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_VariableMetaData_free(visit_handle obj);
SIMV2_API int simv2_VariableMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setMeshName(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getMeshName(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setUnits(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getUnits(visit_handle h, char **);
SIMV2_API int simv2_VariableMetaData_setCentering(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getCentering(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setType(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getType(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setTreatAsASCII(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getTreatAsASCII(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setHideFromGUI(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getHideFromGUI(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_setNumComponents(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getNumComponents(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_addMaterialName(visit_handle h, const char *);
SIMV2_API int simv2_VariableMetaData_getNumMaterialNames(visit_handle h, int *);
SIMV2_API int simv2_VariableMetaData_getMaterialName(visit_handle h, int, char **);
SIMV2_API int simv2_VariableMetaData_setEnumerationType(visit_handle h, int);
SIMV2_API int simv2_VariableMetaData_getEnumerationType(visit_handle h, int*);
SIMV2_API int simv2_VariableMetaData_getNumEnumNames(visit_handle h, int *);
SIMV2_API int simv2_VariableMetaData_getEnumName(visit_handle h, int, char **);
SIMV2_API int simv2_VariableMetaData_addEnumNameValue(visit_handle h, const char *, double, int *);
SIMV2_API int simv2_VariableMetaData_addEnumNameRange(visit_handle h, const char *, double, double, int *);
SIMV2_API int simv2_VariableMetaData_getNumEnumNameRanges(visit_handle h, int *);
SIMV2_API int simv2_VariableMetaData_getEnumNameRange(visit_handle h, int, double *, double *);
SIMV2_API int simv2_VariableMetaData_addEnumGraphEdge(visit_handle h, int, int, const char *);
SIMV2_API int simv2_VariableMetaData_getNumEnumGraphEdges(visit_handle h, int *);
SIMV2_API int simv2_VariableMetaData_getEnumGraphEdge(visit_handle h, int, int *, int *, char **);
SIMV2_API int simv2_VariableMetaData_setEnumAlwaysIncludeValue(visit_handle h, double);
SIMV2_API int simv2_VariableMetaData_setEnumAlwaysIncludeRange(visit_handle h, double, double);
SIMV2_API int simv2_VariableMetaData_getEnumAlwaysIncludeRange(visit_handle h, double *, double *);
SIMV2_API int simv2_VariableMetaData_setEnumAlwaysExcludeValue(visit_handle h, double);
SIMV2_API int simv2_VariableMetaData_setEnumAlwaysExcludeRange(visit_handle h, double, double);
SIMV2_API int simv2_VariableMetaData_getEnumAlwaysExcludeRange(visit_handle h, double *, double *);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_VariableMetaData_check(visit_handle);

#endif
