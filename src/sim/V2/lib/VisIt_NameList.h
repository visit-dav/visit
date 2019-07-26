// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_NAMELIST_H
#define VISIT_NAMELIST_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_NameList_alloc(visit_handle *obj);
int VisIt_NameList_free(visit_handle obj);
int VisIt_NameList_addName(visit_handle h, const char *);
int VisIt_NameList_getNumName(visit_handle h, int *);
int VisIt_NameList_getName(visit_handle h, int, char **);

#ifdef __cplusplus
}
#endif

#endif
