#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"

int
VisIt_UnstructuredMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_UnstructuredMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

int
VisIt_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXYZ,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y,z));
}

int
VisIt_UnstructuredMesh_setCoords(visit_handle obj, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoords,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,c));
}

int
VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int nzones, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setConnectivity,
                    int (*)(visit_handle,int,visit_handle), 
                    int (*cb)(visit_handle,int,visit_handle), 
                    (*cb)(obj,nzones,c));
}

int
VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setRealIndices,
                    int (*)(visit_handle,int,int), 
                    int (*cb)(visit_handle,int,int), 
                    (*cb)(obj,min,max));
}

/************************** Fortran callable routines *************************/
#define F_VISITUNSTRUCTUREDMESHALLOC           F77_ID(visitunstructuredmeshalloc_,visitunstructuredmeshalloc,VISITUNSTRUCTUREDMESHALLOC)
#define F_VISITUNSTRUCTUREDMESHFREE            F77_ID(visitunstructuredmeshfree_,visitunstructuredmeshfree,VISITUNSTRUCTUREDMESHFREE)
#define F_VISITUNSTRUCTUREDMESHSETCOORDSXY     F77_ID(visitunstructuredmeshsetcoordsxy_,visitunstructuredmeshsetcoordsxy,VISITUNSTRUCTUREDMESHSETCOORDSXY)
#define F_VISITUNSTRUCTUREDMESHSETCOORDSXYZ    F77_ID(visitunstructuredmeshsetcoordsxyz_,visitunstructuredmeshsetcoordsxyz,VISITUNSTRUCTUREDMESHSETCOORDSXYZ)
#define F_VISITUNSTRUCTUREDMESHSETCOORDS       F77_ID(visitunstructuredmeshsetcoords_,visitunstructuredmeshsetcoords,VISITUNSTRUCTUREDMESHSETCOORDS)
#define F_VISITUNSTRUCTUREDMESHSETCONNECTIVITY F77_ID(visitunstructuredmeshsetconnectivity_,visitunstructuredmeshsetconnectivity,VISITUNSTRUCTUREDMESHSETCONNECTIVITY)
#define F_VISITUNSTRUCTUREDMESHSETREALINDICES  F77_ID(visitunstructuredmeshsetrealindices_,visitunstructuredmeshsetrealindices,VISITUNSTRUCTUREDMESHSETREALINDICES)

int
F_VISITUNSTRUCTUREDMESHALLOC(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_alloc(obj);
}

int
F_VISITUNSTRUCTUREDMESHFREE(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_free(*obj);
}

int
F_VISITUNSTRUCTUREDMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_UnstructuredMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITUNSTRUCTUREDMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_UnstructuredMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITUNSTRUCTUREDMESHSETCOORDS(visit_handle *obj, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setCoords(*obj, *c);
}

int
F_VISITUNSTRUCTUREDMESHSETCONNECTIVITY(visit_handle *obj, int *nzones, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setConnectivity(*obj, *nzones, *c);
}

int
F_VISITUNSTRUCTUREDMESHSETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_setRealIndices(*obj, *min, *max);
}


