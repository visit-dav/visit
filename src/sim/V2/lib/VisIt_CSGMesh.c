// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_CSGMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_CSGMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_CSGMesh_setRegions(visit_handle obj, visit_handle boolops, visit_handle leftids, 
    visit_handle rightids)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setRegions,
                    int, (visit_handle,visit_handle,visit_handle,visit_handle), 
                    (obj,boolops,leftids,rightids));
}

int
VisIt_CSGMesh_setZonelist(visit_handle obj, visit_handle zl)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setZonelist,
                    int, (visit_handle,visit_handle), 
                    (obj,zl));
}

int
VisIt_CSGMesh_setBoundaryTypes(visit_handle obj, visit_handle boundaryTypes)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setBoundaryTypes,
                    int, (visit_handle,visit_handle), 
                    (obj,boundaryTypes));
}

int
VisIt_CSGMesh_setBoundaryCoeffs(visit_handle obj, visit_handle boundaryCoeffs)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setBoundaryCoeffs,
                    int, (visit_handle,visit_handle), 
                    (obj,boundaryCoeffs));
}

int
VisIt_CSGMesh_setExtents(visit_handle obj, double min[3], double max[3])
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_setExtents,
                    int, (visit_handle,double[3], double[3]), 
                    (obj,min,max));
}

int
VisIt_CSGMesh_getRegions(visit_handle obj, visit_handle *boolops, visit_handle *leftids, 
    visit_handle *rightids)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_getRegions,
                    int, (visit_handle,visit_handle*,visit_handle*,visit_handle*), 
                    (obj,boolops,leftids,rightids));
}

int
VisIt_CSGMesh_getZonelist(visit_handle obj, visit_handle *zl)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_getZonelist,
                    int, (visit_handle,visit_handle*), 
                    (obj,zl));
}

int
VisIt_CSGMesh_getBoundaryTypes(visit_handle obj, visit_handle *boundaryTypes)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_getBoundaryTypes,
                    int, (visit_handle,visit_handle*), 
                    (obj,boundaryTypes));
}

int
VisIt_CSGMesh_getBoundaryCoeffs(visit_handle obj, visit_handle *boundaryCoeffs)
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_getBoundaryCoeffs,
                    int, (visit_handle,visit_handle*), 
                    (obj,boundaryCoeffs));
}

int
VisIt_CSGMesh_getExtents(visit_handle obj, double min[3], double max[3])
{
    VISIT_DYNAMIC_EXECUTE(CSGMesh_getExtents,
                    int, (visit_handle,double[3], double[3]), 
                    (obj,min,max));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITCSGMESHALLOC              F77_ID(visitcsgmeshalloc_,visitcsgmeshalloc,VISITCSGMESHALLOC)
#define F_VISITCSGMESHFREE               F77_ID(visitcsgmeshfree_,visitcsgmeshfree,VISITCSGMESHFREE)
#define F_VISITCSGMESHSETREGIONS         F77_ID(visitcsgmeshsetregions_,visitcsgmeshsetregions,VISITCSGMESHSETREGIONS)
#define F_VISITCSGMESHSETZONELIST        F77_ID(visitcsgmeshsetzonelist_,visitcsgmeshsetzonelist,VISITCSGMESHSETZONELIST)
#define F_VISITCSGMESHSETBOUNDARYTYPES   F77_ID(visitcsgmeshsetboundarytypes_,visitcsgmeshsetboundarytypes,VISITCSGMESHSETBOUNDARYTYPES)
#define F_VISITCSGMESHSETBOUNDARYCOEFFS  F77_ID(visitcsgmeshsetboundarycoeffs_,visitcsgmeshsetboundarycoeffs,VISITCSGMESHSETBOUNDARYCOEFFS)
#define F_VISITCSGMESHSETEXTENTS         F77_ID(visitcsgmeshsetextents_,visitcsgmeshsetextents,VISITCSGMESHSETEXTENTS)
#define F_VISITCSGMESHGETREGIONS         F77_ID(visitcsgmeshgetregions_,visitcsgmeshgetregions,VISITCSGMESHGETREGIONS)
#define F_VISITCSGMESHGETZONELIST        F77_ID(visitcsgmeshgetzonelist_,visitcsgmeshgetzonelist,VISITCSGMESHGETZONELIST)
#define F_VISITCSGMESHGETBOUNDARYTYPES   F77_ID(visitcsgmeshgetboundarytypes_,visitcsgmeshgetboundarytypes,VISITCSGMESHGETBOUNDARYTYPES)
#define F_VISITCSGMESHGETBOUNDARYCOEFFS  F77_ID(visitcsgmeshgetboundarycoeffs_,visitcsgmeshgetboundarycoeffs,VISITCSGMESHGETBOUNDARYCOEFFS)
#define F_VISITCSGMESHGETEXTENTS         F77_ID(visitcsgmeshgetextents_,visitcsgmeshgetextents,VISITCSGMESHGETEXTENTS)

int
F_VISITCSGMESHALLOC(visit_handle *obj)
{
    return VisIt_CSGMesh_alloc(obj);
}

int
F_VISITCSGMESHFREE(visit_handle *obj)
{
    return VisIt_CSGMesh_free(*obj);
}

int
F_VISITCSGMESHSETREGIONS(visit_handle *obj, visit_handle *boolops, visit_handle *leftids, visit_handle *rightids)
{
    return VisIt_CSGMesh_setRegions(*obj, *boolops, *leftids, *rightids);
}

int
F_VISITCSGMESHSETZONELIST(visit_handle *obj, visit_handle *zl)
{
    return VisIt_CSGMesh_setZonelist(*obj, *zl);
}

int
F_VISITCSGMESHSETBOUNDARYTYPES(visit_handle *obj, visit_handle *boundaryTypes)
{
    return VisIt_CSGMesh_setBoundaryTypes(*obj, *boundaryTypes);
}

int
F_VISITCSGMESHSETBOUNDARYCOEFFS(visit_handle *obj, visit_handle *boundaryCoeffs)
{
    return VisIt_CSGMesh_setBoundaryCoeffs(*obj, *boundaryCoeffs);
}

int
F_VISITCSGMESHSETEXTENTS(visit_handle *obj, double *mins, double *maxs)
{
    double tmpMin[3], tmpMax[3];
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = mins[2];
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = maxs[2];
    return VisIt_CSGMesh_setExtents(*obj, tmpMin, tmpMax);
}

int
F_VISITCSGMESHGETREGIONS(visit_handle *obj, visit_handle *boolops, visit_handle *leftids, visit_handle *rightids)
{
    return VisIt_CSGMesh_getRegions(*obj, boolops, leftids, rightids);
}

int
F_VISITCSGMESHGETZONELIST(visit_handle *obj, visit_handle *zl)
{
    return VisIt_CSGMesh_getZonelist(*obj, zl);
}

int
F_VISITCSGMESHGETBOUNDARYTYPES(visit_handle *obj, visit_handle *boundaryTypes)
{
    return VisIt_CSGMesh_getBoundaryTypes(*obj, boundaryTypes);
}

int
F_VISITCSGMESHGETBOUNDARYCOEFFS(visit_handle *obj, visit_handle *boundaryCoeffs)
{
    return VisIt_CSGMesh_getBoundaryCoeffs(*obj, boundaryCoeffs);
}

int
F_VISITCSGMESHGETEXTENTS(visit_handle *obj, double *mins, double *maxs)
{
    return VisIt_CSGMesh_getExtents(*obj, mins, maxs);
}
