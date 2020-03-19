// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_EXPRESSIONMETADATA_H
#define SIMV2_EXPRESSIONMETADATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_ExpressionMetaData_alloc(visit_handle *obj);
SIMV2_API int simv2_ExpressionMetaData_free(visit_handle obj);
SIMV2_API int simv2_ExpressionMetaData_setName(visit_handle h, const char *);
SIMV2_API int simv2_ExpressionMetaData_getName(visit_handle h, char **);
SIMV2_API int simv2_ExpressionMetaData_setDefinition(visit_handle h, const char *);
SIMV2_API int simv2_ExpressionMetaData_getDefinition(visit_handle h, char **);
SIMV2_API int simv2_ExpressionMetaData_setType(visit_handle h, int);
SIMV2_API int simv2_ExpressionMetaData_getType(visit_handle h, int*);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_ExpressionMetaData_check(visit_handle);

#endif
