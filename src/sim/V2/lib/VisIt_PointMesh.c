#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_PointMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_alloc,
                    int (*)(visit_handle*),
                    int (*cb)(visit_handle*),
                    (*cb)(obj))
}

int
VisIt_PointMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_free,
                    int (*)(visit_handle), 
                    int (*cb)(visit_handle), 
                    (*cb)(obj));
}

int
VisIt_PointMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoordsXY,
                    int (*)(visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y));
}

int
VisIt_PointMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoordsXYZ,
                    int (*)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle,visit_handle,visit_handle), 
                    (*cb)(obj,x,y,z));
}

int
VisIt_PointMesh_setCoords(visit_handle obj, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(PointMesh_setCoords,
                    int (*)(visit_handle,visit_handle), 
                    int (*cb)(visit_handle,visit_handle), 
                    (*cb)(obj,c));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITPOINTMESHALLOC            F77_ID(visitpointmeshalloc_,visitpointmeshalloc,VISITPOINTMESHALLOC)
#define F_VISITPOINTMESHFREE             F77_ID(visitpointmeshfree_,visitpointmeshfree,VISITPOINTMESHFREE)
#define F_VISITPOINTMESHSETCOORDSXY      F77_ID(visitpointmeshsetcoordsxy_,visitpointmeshsetcoordsxy,VISITPOINTMESHSETCOORDSXY)
#define F_VISITPOINTMESHSETCOORDSXYZ     F77_ID(visitpointmeshsetcoordsxyz_,visitpointmeshsetcoordsxyz,VISITPOINTMESHSETCOORDSXYZ)
#define F_VISITPOINTMESHSETCOORDS        F77_ID(visitpointmeshsetcoords_,visitpointmeshsetcoords,VISITPOINTMESHSETCOORDS)

int
F_VISITPOINTMESHALLOC(visit_handle *obj)
{
    return VisIt_PointMesh_alloc(obj);
}

int
F_VISITPOINTMESHFREE(visit_handle *obj)
{
    return VisIt_PointMesh_free(*obj);
}

int
F_VISITPOINTMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_PointMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITPOINTMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_PointMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITPOINTMESHSETCOORDS(visit_handle *obj, visit_handle *c)
{
    return VisIt_PointMesh_setCoords(*obj, *c);
}


