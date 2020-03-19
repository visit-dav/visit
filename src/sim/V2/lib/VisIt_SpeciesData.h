// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_SPECIESDATA_H
#define VISIT_SPECIESDATA_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_SpeciesData_alloc(visit_handle *obj);
int VisIt_SpeciesData_free(visit_handle obj);
int VisIt_SpeciesData_addSpeciesName(visit_handle h, visit_handle);
int VisIt_SpeciesData_setSpecies(visit_handle h, visit_handle);
int VisIt_SpeciesData_setSpeciesMF(visit_handle h, visit_handle);
int VisIt_SpeciesData_setMixedSpecies(visit_handle h, visit_handle);

#ifdef __cplusplus
}
#endif

#endif
