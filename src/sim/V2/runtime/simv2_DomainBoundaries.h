#ifndef SIMV2_DOMAINBOUNDARIES_H
#define SIMV2_DOMAINBOUNDARIES_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_DomainBoundaries_alloc(visit_handle*);
SIMV2_API int simv2_DomainBoundaries_free(visit_handle);
SIMV2_API int simv2_DomainBoundaries_set_type(visit_handle, int);
SIMV2_API int simv2_DomainBoundaries_set_numDomains(visit_handle, int);
SIMV2_API int simv2_DomainBoundaries_set_rectIndices(visit_handle, int dom, const int e[6]);
SIMV2_API int simv2_DomainBoundaries_set_amrIndices(visit_handle, int patch, int level, const int e[6]);

/* This function is only available in the runtime. */
SIMV2_API void *simv2_DomainBoundaries_avt(visit_handle);

#ifdef __cplusplus
}
#endif

#endif
