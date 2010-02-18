#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"

int
VisIt_DomainBoundaries_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_DomainBoundaries_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_DomainBoundaries_set_type(visit_handle obj, int type)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_set_type,
                    int (*)(visit_handle,int), 
                    int (*cb)(visit_handle,int), 
                    (*cb)(obj, type));
}

int
VisIt_DomainBoundaries_set_numDomains(visit_handle obj, int numDomains)
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_set_numDomains,
                    int (*)(visit_handle,int), 
                    int (*cb)(visit_handle,int), 
                    (*cb)(obj, numDomains));
}

int
VisIt_DomainBoundaries_set_amrIndices(visit_handle obj, int patch, int level, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_set_amrIndices,
                    int (*)(visit_handle,int,int,const int[6]), 
                    int (*cb)(visit_handle,int,int,const int[6]), 
                    (*cb)(obj, patch, level, extents));
}

int
VisIt_DomainBoundaries_set_rectIndices(visit_handle obj, int dom, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(simv2_DomainBoundaries_set_rectIndices,
                    int (*)(visit_handle,int,const int[6]), 
                    int (*cb)(visit_handle,int,const int[6]), 
                    (*cb)(obj, dom, extents));
}

/************************** Fortran callable routines *************************/
#define F_VISITDBOUNDSALLOC          F77_ID(visitdboundsalloc_,visitdboundsalloc,VISITDBOUNDSALLOC)
#define F_VISITDBOUNDSFREE           F77_ID(visitdboundsfree_,visitdboundsfree,VISITDBOUNDSFREE)
#define F_VISITDBOUNDSSETTYPE        F77_ID(visitdboundssettype_,visitdboundssettype,VISITDBOUNDSSETTYPE)
#define F_VISITDBOUNDSSETNUMDOMAINS  F77_ID(visitdboundssetnumdomains_,visitdboundssetnumdomains,VISITDBOUNDSSETNUMDOMAINS)
#define F_VISITDBOUNDSSETAMRINDICES  F77_ID(visitdboundssetamrindices_,visitdboundssetamrindices,VISITDBOUNDSSETAMRINDICES)
#define F_VISITDBOUNDSSETRECTINDICES F77_ID(visitdboundssetrectindices_,visitdboundssetrectindices,VISITDBOUNDSSETRECTINDICES)

int
F_VISITDBOUNDSALLOC(visit_handle *obj)
{
    return VisIt_DomainBoundaries_alloc(obj);
}

int
F_VISITDBOUNDSFREE(visit_handle *obj)
{
    return VisIt_DomainBoundaries_free(*obj);
}

int
F_VISITDBOUNDSSETTYPE(visit_handle *obj, int *type)
{
    return VisIt_DomainBoundaries_set_type(*obj, *type);
}

int
F_VISITDBOUNDSSETNUMDOMAINS(visit_handle *obj, int *numDomains)
{
    return VisIt_DomainBoundaries_set_numDomains(*obj, *numDomains);
}

int
F_VISITDBOUNDSSETAMRINDICES(visit_handle *obj, int *patch, int *level, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_set_amrIndices(*obj, *patch, *level, e);
}

int
F_VISITDBOUNDSSETRECTINDICES(visit_handle *obj, int *domain, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_set_rectIndices(*obj, *domain, e);
}
