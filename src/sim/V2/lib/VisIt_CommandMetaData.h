// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_COMMANDMETADATA_H
#define VISIT_COMMANDMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_CommandMetaData_alloc(visit_handle *obj);
int VisIt_CommandMetaData_free(visit_handle obj);
int VisIt_CommandMetaData_setName(visit_handle h, const char *);
int VisIt_CommandMetaData_getName(visit_handle h, char **);
int VisIt_CommandMetaData_setEnabled(visit_handle h, int);
int VisIt_CommandMetaData_getEnabled(visit_handle h, int*);

#ifdef __cplusplus
}
#endif

#endif
