// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_DOMAINBOUNDARIES_H
#define VISIT_DOMAINBOUNDARIES_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainBoundaries_alloc(visit_handle*);
int VisIt_DomainBoundaries_free(visit_handle);
/* Pass 0=rectilinear, 1=curvilinear for the type. */
int VisIt_DomainBoundaries_set_type(visit_handle, int);
int VisIt_DomainBoundaries_set_numDomains(visit_handle, int);

/* Set extents for structured mesh. */
int VisIt_DomainBoundaries_set_rectIndices(visit_handle, int dom, const int e[6]);

/* Set extents for an AMR patch. */
int VisIt_DomainBoundaries_set_amrIndices(visit_handle, int patch, int level, const int e[6]);

/* Functions to add a custom number of neighbors for a domain. */
int VisIt_DomainBoundaries_set_extents(visit_handle, int dom, const int e[6]);
int VisIt_DomainBoundaries_add_neighbor(visit_handle, int dom, int d, int mi, 
                                        const int orientation[3], const int extents[6]);
int VisIt_DomainBoundaries_finish(visit_handle, int dom);

#ifdef __cplusplus
}
#endif

#endif
