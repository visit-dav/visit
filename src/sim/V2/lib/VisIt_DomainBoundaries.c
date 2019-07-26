// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_DomainBoundaries_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_DomainBoundaries_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_DomainBoundaries_set_type(visit_handle obj, int type)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_type,
                    int, (visit_handle,int), 
                    (obj, type));
}

int
VisIt_DomainBoundaries_set_numDomains(visit_handle obj, int numDomains)
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_numDomains,
                    int, (visit_handle,int), 
                    (obj, numDomains));
}

int
VisIt_DomainBoundaries_set_amrIndices(visit_handle obj, int patch, int level, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_amrIndices,
                    int, (visit_handle,int,int,const int[6]), 
                    (obj, patch, level, extents));
}

int
VisIt_DomainBoundaries_set_rectIndices(visit_handle obj, int dom, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_rectIndices,
                    int, (visit_handle,int,const int[6]), 
                    (obj, dom, extents));
}

int
VisIt_DomainBoundaries_set_extents(visit_handle obj, int dom, const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_set_extents,
                    int, (visit_handle,int,const int[6]), 
                    (obj, dom, extents));
}

int
VisIt_DomainBoundaries_add_neighbor(visit_handle obj, 
    int dom, int d, int mi, const int orientation[3], const int extents[6])
{
    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_add_neighbor,
                    int, (visit_handle,int,int,int,const int[3],const int[6]), 
                    (obj, dom, d, mi, orientation, extents));
}

int
VisIt_DomainBoundaries_finish(visit_handle obj, int dom)
{    VISIT_DYNAMIC_EXECUTE(DomainBoundaries_finish,
                    int, (visit_handle,int), 
                    (obj, dom));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITDBOUNDSALLOC              F77_ID(visitdboundsalloc_,visitdboundsalloc,VISITDBOUNDSALLOC)
#define F_VISITDBOUNDSFREE               F77_ID(visitdboundsfree_,visitdboundsfree,VISITDBOUNDSFREE)
#define F_VISITDBOUNDSSETTYPE            F77_ID(visitdboundssettype_,visitdboundssettype,VISITDBOUNDSSETTYPE)
#define F_VISITDBOUNDSSETNUMDOMAINS      F77_ID(visitdboundssetnumdomains_,visitdboundssetnumdomains,VISITDBOUNDSSETNUMDOMAINS)
#define F_VISITDBOUNDSSETAMRINDICES      F77_ID(visitdboundssetamrindices_,visitdboundssetamrindices,VISITDBOUNDSSETAMRINDICES)
#define F_VISITDBOUNDSSETRECTINDICES     F77_ID(visitdboundssetrectindices_,visitdboundssetrectindices,VISITDBOUNDSSETRECTINDICES)
#define F_VISITDBOUNDSSETEXTENTS         F77_ID(visitdboundssetextents_,visitdboundssetextents,VISITDBOUNDSSETEXTENTS)
#define F_VISITDBOUNDSADDNEIGHBOR        F77_ID(visitdboundsaddneighbor_, visitdboundsaddneighbor, VISITDBOUNDSADDNEIGHBOR)
#define F_VISITDBOUNDSFINISH             F77_ID(visitdboundsfinish_, visitdboundsfinish, VISITDBOUNDSFINISH)

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

int
F_VISITDBOUNDSSETEXTENTS(visit_handle *obj, int *domain, const int *extents)
{
    int i, e[6];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_set_extents(*obj, *domain, e);
}

int
F_VISITDBOUNDSADDNEIGHBOR(visit_handle *obj, int *domain, int *d, int *mi, const int *orientation, const int *extents)
{
    int i, o[3], e[6];
    for(i = 0; i < 3; ++i)
        o[i] = orientation[i];
    for(i = 0; i < 6; ++i)
        e[i] = extents[i];
    return VisIt_DomainBoundaries_add_neighbor(*obj, *domain, *d, *mi, o, e);
}

int
F_VISITDBOUNDSFINISH(visit_handle *obj, int *domain)
{
    return VisIt_DomainBoundaries_finish(*obj, *domain);
}
