// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DOMAINNESTING_H
#define VISIT_DOMAINNESTING_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainNesting_alloc(visit_handle*);
int VisIt_DomainNesting_free(visit_handle);
int VisIt_DomainNesting_set_dimensions(visit_handle,int,int,int);
int VisIt_DomainNesting_set_levelRefinement(visit_handle, int, int[3]);
int VisIt_DomainNesting_set_nestingForPatch(visit_handle, int, int, const int *, int, int[6]);

#ifdef __cplusplus
}
#endif

#endif
