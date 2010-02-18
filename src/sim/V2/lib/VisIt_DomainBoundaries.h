#ifndef VISIT_DOMAINBOUNDARIES_H
#define VISIT_DOMAINBOUNDARIES_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainBoundaries_alloc(visit_handle*);
int VisIt_DomainBoundaries_free(visit_handle);
int VisIt_DomainBoundaries_set_type(visit_handle, int);
int VisIt_DomainBoundaries_set_numDomains(visit_handle, int);
int VisIt_DomainBoundaries_set_rectIndices(visit_handle, int dom, const int e[6]);
int VisIt_DomainBoundaries_set_amrIndices(visit_handle, int patch, int level, const int e[6]);

#ifdef __cplusplus
}
#endif

#endif
