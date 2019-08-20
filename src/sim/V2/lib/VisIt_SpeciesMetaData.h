// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SPECIESMETADATA_H
#define VISIT_SPECIESMETADATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_SpeciesMetaData_alloc(visit_handle *obj);
int VisIt_SpeciesMetaData_free(visit_handle obj);
int VisIt_SpeciesMetaData_setName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_getName(visit_handle h, char **);
int VisIt_SpeciesMetaData_setMeshName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_getMeshName(visit_handle h, char **);
int VisIt_SpeciesMetaData_setMaterialName(visit_handle h, const char *);
int VisIt_SpeciesMetaData_getMaterialName(visit_handle h, char **);
int VisIt_SpeciesMetaData_addSpeciesName(visit_handle h, visit_handle);
int VisIt_SpeciesMetaData_getNumSpeciesName(visit_handle h, visit_handle *);
int VisIt_SpeciesMetaData_getSpeciesName(visit_handle h, int, visit_handle*);

#ifdef __cplusplus
}
#endif

#endif
