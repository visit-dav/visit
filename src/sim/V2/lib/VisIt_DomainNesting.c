#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"

int
VisIt_DomainNesting_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_DomainNesting_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_DomainNesting_set_dimensions(visit_handle obj, int nPatches, int nLevels, int nDimensions)
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_dimensions,
                    int (*)(visit_handle,int,int,int),
                    int (*cb)(visit_handle,int,int,int),
                    (*cb)(obj, nPatches, nLevels, nDimensions))
}

int
VisIt_DomainNesting_set_levelRefinement(visit_handle obj, int level, int ratios[3])
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_levelRefinement,
                    int (*)(visit_handle,int,int[3]), 
                    int (*cb)(visit_handle,int,int[3]), 
                    (*cb)(obj, level, ratios));
}

int
VisIt_DomainNesting_set_nestingForPatch(visit_handle obj, int patch, int level, 
    const int *childPatches, int nChildPatches, int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainNesting_set_nestingForPatch,
                    int (*)(visit_handle,int,int,const int*,int,int[6]), 
                    int (*cb)(visit_handle,int,int,const int*,int,int[6]), 
                    (*cb)(obj, patch, level, childPatches, nChildPatches, extents));
}

/************************** Fortran callable routines *************************/
#define F_VISITDALLOC                           F77_ID(visitdnestingalloc_,visitdnestingalloc,VISITDNESTINGALLOC)
#define F_VISITDNESTINGFREE                     F77_ID(visitdnestingfree_,visitdnestingfree,VISITDNESTINGFREE)
#define F_VISITDNESTINGSETDIMENSIONS            F77_ID(visitdnestingsetdimensions_,visitdnestingsetdimensions,VISITDNESTINGSETDIMENSIONS)
#define F_VISITDNESTINGSETLEVELREFINEMENTRATIOS F77_ID(visitdnestingsetlevelrefinementratios_,visitdnestingsetlevelrefinmentratios,VISITDNESTINGSETLEVELREFINEMENTRATIOS)
#define F_VISITDNESTINGSETNESTINGFORPATCH       F77_ID(visitdnestingsetnestingforpatch_,visitdnestingsetnestingforpatch,VISITDNESTINGSETNESTINGFORPATCH)

int
F_VISITDNESTINGALLOC(visit_handle *obj)
{
    return VisIt_DomainNesting_alloc(obj);
}

int
F_VISITDNESTINGFREE(visit_handle *obj)
{
    return VisIt_DomainNesting_free(*obj);
}

int
F_VISITDNESTINGSETDIMENSIONS(visit_handle *obj, int *nPatches, int *nLevels, int *nDimensions)
{
    return VisIt_DomainNesting_set_dimensions(*obj, *nPatches, *nLevels, *nDimensions);
}

int
F_VISITDNESTINGSETLEVELREFINEMENT(visit_handle *obj, int *level, const int *ratios)
{
    int r[3];
    r[0] = ratios[0];
    r[1] = ratios[1];
    r[2] = ratios[2];
    return VisIt_DomainNesting_set_levelRefinement(*obj, *level, r);
}

int
F_VISITDNESTINGSETNESTINGFORPATCH(visit_handle *obj, int *patch, int *level, 
    const int *childPatches, int *nChildPatches, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainNesting_set_nestingForPatch(*obj, *patch, *level, 
           childPatches, *nChildPatches, e);
}

