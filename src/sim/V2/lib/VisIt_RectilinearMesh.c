// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_RectilinearMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_RectilinearMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_RectilinearMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setCoordsXY,
                    int, (visit_handle,visit_handle,visit_handle), 
                    (obj,x,y));
}

int
VisIt_RectilinearMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setCoordsXYZ,
                    int, (visit_handle,visit_handle,visit_handle,visit_handle), 
                    (obj,x,y,z));
}

int
VisIt_RectilinearMesh_setBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setBaseIndex,
                    int, (visit_handle,int[3]), 
                    (obj,base_index));
}


int
VisIt_RectilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setRealIndices,
                    int, (visit_handle,int[3], int[3]), 
                    (obj,min,max));
}

int
VisIt_RectilinearMesh_setGhostCells(visit_handle obj, visit_handle gz)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setGhostCells,
                    int, (visit_handle,visit_handle), 
                    (obj,gz));
}

int
VisIt_RectilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setGhostNodes,
                    int, (visit_handle,visit_handle), 
                    (obj,gn));
}

int
VisIt_RectilinearMesh_getCoords(visit_handle obj, int *ndims, 
                  visit_handle *x, visit_handle *y, visit_handle *z)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_getCoords,
                    int, (visit_handle,int*,visit_handle*,visit_handle*,visit_handle*), 
                    (obj,ndims,x,y,z));
}

int
VisIt_RectilinearMesh_getRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_getRealIndices,
                    int, (visit_handle,int[3], int[3]), 
                    (obj,min,max));
}

int
VisIt_RectilinearMesh_getBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_getBaseIndex,
                    int, (visit_handle,int[3]), 
                    (obj,base_index));
}

static int
VisIt_RectilinearMesh_getNumDimensions(visit_handle obj)
{
    int ndims = 3;
    visit_handle x,y,z;
    VisIt_RectilinearMesh_getCoords(obj, &ndims, &x, &y, &z);
    return ndims;
}

int
VisIt_RectilinearMesh_getGhostCells(visit_handle obj, visit_handle *gz)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_getGhostCells,
                    int, (visit_handle,visit_handle*), 
                    (obj,gz));
}

int
VisIt_RectilinearMesh_getGhostNodes(visit_handle obj, visit_handle *gn)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_getGhostNodes,
                    int, (visit_handle,visit_handle*), 
                    (obj,gn));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITRECTMESHALLOC             F77_ID(visitrectmeshalloc_,visitrectmeshalloc,VISITRECTMESHALLOC)
#define F_VISITRECTMESHFREE              F77_ID(visitrectmeshfree_,visitrectmeshfree,VISITRECTMESHFREE)
#define F_VISITRECTMESHSETCOORDSXY       F77_ID(visitrectmeshsetcoordsxy_,visitrectmeshsetcoordsxy,VISITRECTMESHSETCOORDSXY)
#define F_VISITRECTMESHSETCOORDSXYZ      F77_ID(visitrectmeshsetcoordsxyz_,visitrectmeshsetcoordsxyz,VISITRECTMESHSETCOORDSXYZ)
#define F_VISITRECTMESHSETBASEINDEX      F77_ID(visitrectmeshsetbaseindex_,visitrectmeshsetbaseindex,VISITRECTMESHSETBASEINDEX)
#define F_VISITRECTMESHSETREALINDICES    F77_ID(visitrectmeshsetrealindices_,visitrectmeshsetrealindices,VISITRECTMESHSETREALINDICES)
#define F_VISITRECTMESHSETGHOSTCELLS     F77_ID(visitrectmeshsetghostcells_,visitrectmeshsetghostcells,VISITRECTMESHSETGHOSTCELLS)
#define F_VISITRECTMESHSETGHOSTNODES     F77_ID(visitrectmeshsetghostnodes_,visitrectmeshsetghostnodes,VISITRECTMESHSETGHOSTNODES)

#define F_VISITRECTMESHGETCOORDS         F77_ID(visitrectmeshgetcoords_,visitrectmeshgetcoords,VISITRECTMESHGETCOORDS)
#define F_VISITRECTMESHGETBASEINDEX      F77_ID(visitrectmeshgetbaseindex_,visitrectmeshgetbaseindex,VISITRECTMESHGETBASEINDEX)
#define F_VISITRECTMESHGETREALINDICES    F77_ID(visitrectmeshgetrealindices_,visitrectmeshgetrealindices,VISITRECTMESHGETREALINDICES)
#define F_VISITRECTMESHGETGHOSTCELLS     F77_ID(visitrectmeshgetghostcells_,visitrectmeshgetghostcells,VISITRECTMESHGETGHOSTCELLS)
#define F_VISITRECTMESHGETGHOSTNODES     F77_ID(visitrectmeshgetghostnodes_,visitrectmeshgetghostnodes,VISITRECTMESHGETGHOSTNODES)

int
F_VISITRECTMESHALLOC(visit_handle *obj)
{
    return VisIt_RectilinearMesh_alloc(obj);
}

int
F_VISITRECTMESHFREE(visit_handle *obj)
{
    return VisIt_RectilinearMesh_free(*obj);
}

int
F_VISITRECTMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_RectilinearMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITRECTMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_RectilinearMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITRECTMESHSETBASEINDEX(visit_handle *obj, int *base_index)
{
    int tmp[3];
    int ndims;
    ndims = VisIt_RectilinearMesh_getNumDimensions(*obj);
    tmp[0] = base_index[0];
    tmp[1] = base_index[1];
    tmp[2] = (ndims == 3) ? base_index[2] : 0;
    return VisIt_RectilinearMesh_setBaseIndex(*obj, tmp);
}

int
F_VISITRECTMESHSETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    int tmpMin[3], tmpMax[3];
    int ndims;
    ndims = VisIt_RectilinearMesh_getNumDimensions(*obj);
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = (ndims == 3) ? mins[2] : 0;
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = (ndims == 3) ? maxs[2] : 0;
    return VisIt_RectilinearMesh_setRealIndices(*obj, tmpMin, tmpMax);
}

int
F_VISITRECTMESHSETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_RectilinearMesh_setGhostCells(*obj, *gz);
}

int
F_VISITRECTMESHSETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_RectilinearMesh_setGhostNodes(*obj, *gn);
}

int
F_VISITRECTMESHGETCOORDS(visit_handle *obj, int *ndims, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_RectilinearMesh_getCoords(*obj, ndims, x, y, z);
}

int
F_VISITRECTMESHGETBASEINDEX(visit_handle *obj, int *base_index)
{
    return VisIt_RectilinearMesh_getBaseIndex(*obj, base_index);
}

int
F_VISITRECTMESHGETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    return VisIt_RectilinearMesh_getRealIndices(*obj, mins, maxs);
}

int
F_VISITRECTMESHGETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_RectilinearMesh_getGhostCells(*obj, gz);
}

int
F_VISITRECTMESHGETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_RectilinearMesh_getGhostNodes(*obj, gn);
}
