#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

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
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITUCDMESHALLOC              F77_ID(visitucdmeshalloc_,visitucdmeshalloc,VISITUCDMESHALLOC)
#define F_VISITUCDMESHFREE               F77_ID(visitucdmeshfree_,visitucdmeshfree,VISITUCDMESHFREE)
#define F_VISITUCDMESHSETCOORDSXY        F77_ID(visitucdmeshsetcoordsxy_,visitucdmeshsetcoordsxy,VISITUCDMESHSETCOORDSXY)
#define F_VISITUCDMESHSETCOORDSXYZ       F77_ID(visitucdmeshsetcoordsxyz_,visitucdmeshsetcoordsxyz,VISITUCDMESHSETCOORDSXYZ)
#define F_VISITUCDMESHSETCOORDS          F77_ID(visitucdmeshsetcoords_,visitucdmeshsetcoords,VISITUCDMESHSETCOORDS)
#define F_VISITUCDMESHSETCONNECTIVITY    F77_ID(visitucdmeshsetconnectivity_,visitucdmeshsetconnectivity,VISITUCDMESHSETCONNECTIVITY)
#define F_VISITUCDMESHSETREALINDICES     F77_ID(visitucdmeshsetrealindices_,visitucdmeshsetrealindices,VISITUCDMESHSETREALINDICES)

int
F_VISITUCDMESHALLOC(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_alloc(obj);
}

int
F_VISITUCDMESHFREE(visit_handle *obj)
{
    return VisIt_UnstructuredMesh_free(*obj);
}

int
F_VISITUCDMESHSETCOORDSXY(visit_handle *obj, visit_handle *x, visit_handle *y)
{
    return VisIt_UnstructuredMesh_setCoordsXY(*obj, *x, *y);
}

int
F_VISITUCDMESHSETCOORDSXYZ(visit_handle *obj, visit_handle *x, visit_handle *y, visit_handle *z)
{
    return VisIt_UnstructuredMesh_setCoordsXYZ(*obj, *x, *y, *z);
}

int
F_VISITUCDMESHSETCOORDS(visit_handle *obj, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setCoords(*obj, *c);
}

int
F_VISITUCDMESHSETCONNECTIVITY(visit_handle *obj, int *nzones, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setConnectivity(*obj, *nzones, *c);
}

int
F_VISITUCDMESHSETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_setRealIndices(*obj, *min, *max);
}


