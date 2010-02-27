#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"

int
VisIt_RectilinearMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_RectilinearMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_RectilinearMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

int
VisIt_RectilinearMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setCoordsXYZ,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y,z));
}

int
VisIt_RectilinearMesh_setBaseIndex(visit_handle obj, int base_index[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setCoords,
                    int (*)(visit_handle,int[3]), 
                    int (*cb)(visit_handle,int[3]), 
                    (*cb)(obj,base_index));
}


int
VisIt_RectilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3])
{
    VISIT_DYNAMIC_EXECUTE(RectilinearMesh_setRealIndices,
                    int (*)(visit_handle,int[3], int[3]), 
                    int (*cb)(visit_handle,int[3], int[3]), 
                    (*cb)(obj,min,max));
}

/************************** Fortran callable routines *************************/
#define F_VISITRECTILINEARMESHALLOC           F77_ID(visitrectilinearmeshalloc_,visitrectilinearmeshalloc,VISITRECTILINEARMESHALLOC)
#define F_VISITRECTILINEARMESHFREE            F77_ID(visitrectilinearmeshfree_,visitrectilinearmeshfree,VISITRECTILINEARMESHFREE)
#define F_VISITRECTILINEARMESHSETCOORDSXY     F77_ID(visitrectilinearmeshsetcoordsxy_,visitrectilinearmeshsetcoordsxy,VISITRECTILINEARMESHSETCOORDSXY)
#define F_VISITRECTILINEARMESHSETCOORDSXYZ    F77_ID(visitrectilinearmeshsetcoordsxyz_,visitrectilinearmeshsetcoordsxyz,VISITRECTILINEARMESHSETCOORDSXYZ)
#define F_VISITRECTILINEARMESHSETBASEINDEX    F77_ID(visitrectilinearmeshsetbaseindex_,visitrectilinearmeshsetbaseindex,VISITRECTILINEARMESHSETBASEINDEX)
#define F_VISITRECTILINEARMESHSETREALINDICES  F77_ID(visitrectilinearmeshsetrealindices_,visitrectilinearmeshsetrealindices,VISITRECTILINEARMESHSETREALINDICES)

int
F_VISITRECTILINEARMESHALLOC(visit_handle *obj)
{
    return VisIt_RectilinearMesh_alloc(obj);
}

int
F_VISITRECTILINEARMESHFREE(visit_handle *obj)
{
    return VisIt_RectilinearMesh_free(*obj);
}

int
F_VISITRECTILINEARMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_RectilinearMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITRECTILINEARMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_RectilinearMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITRECTILINEARMESHSETBASEINDEX(visit_handle *obj, int *base_index)
{
    int tmp[3];
    tmp[0] = base_index[0];
    tmp[1] = base_index[1];
    tmp[2] = base_index[2];
    return VisIt_RectilinearMesh_setBaseIndex(*obj, tmp);
}

int
F_VISITRECTILINEARMESHSETREALINDICES(visit_handle *obj, int *mins, int *maxs)
{
    int tmpMin[3], tmpMax[3];
    tmpMin[0] = mins[0];
    tmpMin[1] = mins[1];
    tmpMin[2] = mins[2];
    tmpMax[0] = maxs[0];
    tmpMax[1] = maxs[1];
    tmpMax[2] = maxs[2];
    return VisIt_RectilinearMesh_setRealIndices(*obj, tmpMin, tmpMax);
}

