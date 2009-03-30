#ifndef VISIT_DOMAINNESTING_H
#define VISIT_DOMAINNESTING_H

#ifdef __cplusplus
extern "C"
{
#endif

int VISIT_DECORATE(DomainNesting_alloc)(visit_handle*);
int VISIT_DECORATE(DomainNesting_free)(visit_handle);
int VISIT_DECORATE(DomainNesting_set_dimensions)(visit_handle,int,int,int);
int VISIT_DECORATE(DomainNesting_set_levelRefinement)(visit_handle, int, int[3]);
int VISIT_DECORATE(DomainNesting_set_nestingForPatch)(visit_handle, int, int, const int *, int, int[6]);

/* This function is only available in the runtime. */
void *VISIT_DECORATE(DomainNesting_avt)(visit_handle);

#ifdef __cplusplus
}
#endif

#endif
