// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_MATERIALMETADATA_H
#define VISIT_MATERIALMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_MaterialMetaData_alloc(visit_handle *obj);
int VisIt_MaterialMetaData_free(visit_handle obj);
int VisIt_MaterialMetaData_setName(visit_handle h, const char *);
int VisIt_MaterialMetaData_getName(visit_handle h, char **);
int VisIt_MaterialMetaData_setMeshName(visit_handle h, const char *);
int VisIt_MaterialMetaData_getMeshName(visit_handle h, char **);
int VisIt_MaterialMetaData_addMaterialName(visit_handle h, const char *);
int VisIt_MaterialMetaData_getNumMaterialNames(visit_handle h, int *);
int VisIt_MaterialMetaData_getMaterialName(visit_handle h, int, char **);

#ifdef __cplusplus
}
#endif

#endif
