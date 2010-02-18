#ifndef SIMV2_DOMAINNESTING_H
#define SIMV2_DOMAINNESTING_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_DomainNesting_alloc(visit_handle*);
SIMV2_API int simv2_DomainNesting_free(visit_handle);
SIMV2_API int simv2_DomainNesting_set_dimensions(visit_handle,int,int,int);
SIMV2_API int simv2_DomainNesting_set_levelRefinement(visit_handle, int, int[3]);
SIMV2_API int simv2_DomainNesting_set_nestingForPatch(visit_handle, int, int, const int *, int, int[6]);

/* This function is only available in the runtime. */
SIMV2_API void *simv2_DomainNesting_avt(visit_handle);

#ifdef __cplusplus
}
#endif

#endif
