#ifndef SIMV2_DOMAINLIST_H
#define SIMV2_DOMAINLIST_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_DomainList_alloc(visit_handle*);
SIMV2_API int simv2_DomainList_free(visit_handle);

SIMV2_API int simv2_DomainList_setDomains(visit_handle obj, int alldoms,
                                          visit_handle mydoms);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_DomainList_getData(visit_handle h, 
                                       int &alldoms,
                                       visit_handle &mydoms);

SIMV2_API int simv2_DomainList_check(visit_handle h);

#endif
