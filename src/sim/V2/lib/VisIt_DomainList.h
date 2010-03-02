#ifndef VISIT_DOMAINLIST_H
#define VISIT_DOMAINLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_DomainList_alloc(visit_handle *obj);
int VisIt_DomainList_free(visit_handle obj);
int VisIt_DomainList_setDomains(visit_handle obj, int alldoms,
                                visit_handle mydoms);

#ifdef __cplusplus
}
#endif

#endif
