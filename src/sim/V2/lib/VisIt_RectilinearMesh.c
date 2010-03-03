#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

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
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITRECTMESHALLOC             F77_ID(visitrectmeshalloc_,visitrectmeshalloc,VISITRECTMESHALLOC)
#define F_VISITRECTMESHFREE              F77_ID(visitrectmeshfree_,visitrectmeshfree,VISITRECTMESHFREE)
#define F_VISITRECTMESHSETCOORDSXY       F77_ID(visitrectmeshsetcoordsxy_,visitrectmeshsetcoordsxy,VISITRECTMESHSETCOORDSXY)
#define F_VISITRECTMESHSETCOORDSXYZ      F77_ID(visitrectmeshsetcoordsxyz_,visitrectmeshsetcoordsxyz,VISITRECTMESHSETCOORDSXYZ)
#define F_VISITRECTMESHSETBASEINDEX      F77_ID(visitrectmeshsetbaseindex_,visitrectmeshsetbaseindex,VISITRECTMESHSETBASEINDEX)
#define F_VISITRECTMESHSETREALINDICES    F77_ID(visitrectmeshsetrealindices_,visitrectmeshsetrealindices,VISITRECTMESHSETREALINDICES)

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
    tmp[0] = base_index[0];
    tmp[1] = base_index[1];
    tmp[2] = base_index[2];
    return VisIt_RectilinearMesh_setBaseIndex(*obj, tmp);
}

int
F_VISITRECTMESHSETREALINDICES(visit_handle *obj, int *mins, int *maxs)
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

