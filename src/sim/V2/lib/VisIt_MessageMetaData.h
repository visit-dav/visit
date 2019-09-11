// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MESSAGEMETADATA_H
#define VISIT_MESSAGEMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_MessageMetaData_alloc(visit_handle *obj);
int VisIt_MessageMetaData_free(visit_handle obj);
int VisIt_MessageMetaData_setName(visit_handle h, const char *);
int VisIt_MessageMetaData_getName(visit_handle h, char **);

#ifdef __cplusplus
}
#endif

#endif
