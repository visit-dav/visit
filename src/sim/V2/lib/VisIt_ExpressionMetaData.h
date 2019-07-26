// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_EXPRESSIONMETADATA_H
#define VISIT_EXPRESSIONMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_ExpressionMetaData_alloc(visit_handle *obj);
int VisIt_ExpressionMetaData_free(visit_handle obj);
int VisIt_ExpressionMetaData_setName(visit_handle h, const char *);
int VisIt_ExpressionMetaData_getName(visit_handle h, char **);
int VisIt_ExpressionMetaData_setDefinition(visit_handle h, const char *);
int VisIt_ExpressionMetaData_getDefinition(visit_handle h, char **);
int VisIt_ExpressionMetaData_setType(visit_handle h, int);
int VisIt_ExpressionMetaData_getType(visit_handle h, int*);

#ifdef __cplusplus
}
#endif

#endif
