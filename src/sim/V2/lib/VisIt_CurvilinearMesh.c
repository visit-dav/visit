#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_CurvilinearMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_CurvilinearMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_CurvilinearMesh_setCoordsXY(visit_handle obj, int dims[2], visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoordsXY,
                    int (*)(visit_handle,int[2],visit_handle,visit_handle), 
                    int (*cb)(visit_handle,int[2],visit_handle,visit_handle), 
                    (*cb)(obj,dims,x,y));
}

int
VisIt_CurvilinearMesh_setCoordsXYZ(visit_handle obj, int dims[3], visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoordsXYZ,
                    int (*)(visit_handle,int[3],visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,int[3],visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,dims,x,y,z));
}

int
VisIt_CurvilinearMesh_setCoords2(visit_handle obj, int dims[2], visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoords2,
                    int (*)(visit_handle,int[2],visit_handle), 
                    int (*cb)(visit_handle,int[2],visit_handle), 
                    (*cb)(obj,dims,c));
}

int
VisIt_CurvilinearMesh_setCoords3(visit_handle obj, int dims[3], visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoords3,
                    int (*)(visit_handle,int[3],visit_handle), 
                    int (*cb)(visit_handle,int[3],visit_handle), 
                    (*cb)(obj,dims,c));
}

int
VisIt_CurvilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setCoords,
                    int (*)(visit_handle,int[3], int[3]), 
                    int (*cb)(visit_handle,int[3], int[3]), 
                    (*cb)(obj,min,max));
}

int
VisIt_CurvilinearMesh_setBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(CurvilinearMesh_setBaseIndex,
                    int (*)(visit_handle,int[3]), 
                    int (*cb)(visit_handle,int[3]), 
                    (*cb)(obj,base_index));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITCURVMESHALLOC             F77_ID(visitcurvmeshalloc_,visitcurvmeshalloc,VISITCURVMESHALLOC)
#define F_VISITCURVMESHFREE              F77_ID(visitcurvmeshfree_,visitcurvmeshfree,VISITCURVMESHFREE)
#define F_VISITCURVMESHSETCOORDSXY       F77_ID(visitcurvmeshsetcoordsxy_,visitcurvmeshsetcoordsxy,VISITCURVMESHSETCOORDSXY)
#define F_VISITCURVMESHSETCOORDSXYZ      F77_ID(visitcurvmeshsetcoordsxyz_,visitcurvmeshsetcoordsxyz,VISITCURVMESHSETCOORDSXYZ)
#define F_VISITCURVMESHSETCOORDS2        F77_ID(visitcurvmeshsetcoords2_,visitcurvmeshsetcoords2,VISITCURVMESHSETCOORDS2)
#define F_VISITCURVMESHSETCOORDS3        F77_ID(visitcurvmeshsetcoords3_,visitcurvmeshsetcoords3,VISITCURVMESHSETCOORDS3)
#define F_VISITCURVMESHSETBASEINDEX      F77_ID(visitcurvmeshsetbaseindex_,visitcurvmeshsetbaseindex,VISITCURVMESHSETBASEINDEX)
#define F_VISITCURVMESHSETREALINDICES    F77_ID(visitcurvmeshsetrealindices_,visitcurvmeshsetrealindices,VISITCURVMESHSETREALINDICES)

int
F_VISITCURVMESHALLOC(visit_handle *obj)
{
    return VisIt_CURVMESH_alloc(obj);
}

int
F_VISITCURVMESHFREE(visit_handle *obj)
{
    return VisIt_CURVMESH_free(*obj);
}

int
F_VISITCURVMESHSETCOORDSXY(visit_handle *obj, int *dims, visit_handle *x, visit_handle *y)
{
    return VisIt_CURVMESH_setCoordsXY(*obj, dims, *x, *y);
}

int
F_VISITCURVMESHSETCOORDSXYZ(visit_handle *obj, int *dims, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_CURVMESH_setCoordsXYZ(*obj, dims, *x, *y, *z);
}

int
F_VISITCURVMESHSETCOORDS2(visit_handle *obj, int *dims, visit_handle *c)
{
    return VisIt_CURVMESH_setCoords2(*obj, dims, *c);
}

int
F_VISITCURVMESHSETCOORDS3(visit_handle *obj, int *dims, visit_handle *c)
{
    return VisIt_CURVMESH_setCoords3(*obj, dims, *c);
}

int
F_VISITCURVMESHSETBASEINDEX(visit_handle *obj, int *base_index)
{
    int tmp[3];
    tmp[0] = base_index[0];
    tmp[1] = base_index[1];
    tmp[2] = base_index[2];
    return VisIt_CURVMESH_setBaseIndex(*obj, tmp);
}

int
F_VISITCURVMESHSETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    int tmpMin[3], tmpMax[3];
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = mins[2];
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = maxs[2];
    return VisIt_CURVMESH_setRealIndices(*obj, tmpMin, tmpMax);
}

