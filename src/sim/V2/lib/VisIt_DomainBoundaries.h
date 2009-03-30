#ifndef VISIT_DOMAINBOUNDARIES_H
#define VISIT_DOMAINBOUNDARIES_H

#ifdef __cplusplus
extern "C"
{
#endif

int VISIT_DECORATE(DomainBoundaries_alloc)(visit_handle*);
int VISIT_DECORATE(DomainBoundaries_free)(visit_handle);
int VISIT_DECORATE(DomainBoundaries_set_type)(visit_handle, int);
int VISIT_DECORATE(DomainBoundaries_set_numDomains)(visit_handle, int);
int VISIT_DECORATE(DomainBoundaries_set_rectIndices)(visit_handle, int dom, const int e[6]);
int VISIT_DECORATE(DomainBoundaries_set_amrIndices)(visit_handle, int patch, int level, const int e[6]);

/* This function is only available in the runtime. */
void *VISIT_DECORATE(DomainBoundaries_avt)(visit_handle);

#ifdef __cplusplus
}
#endif

#endif
