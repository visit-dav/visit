// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_UnstructuredMesh_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_UnstructuredMesh_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXY,
                    int, (visit_handle,visit_handle,visit_handle), 
                    (obj,x,y));
}

int
VisIt_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoordsXYZ,
                    int, (visit_handle,visit_handle,visit_handle,visit_handle), 
                    (obj,x,y,z));
}

int
VisIt_UnstructuredMesh_setCoords(visit_handle obj, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setCoords,
                    int, (visit_handle,visit_handle), 
                    (obj,c));
}

int
VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int ncells, visit_handle c)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setConnectivity,
                    int, (visit_handle,int,visit_handle), 
                    (obj,ncells,c));
}

int
VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setRealIndices,
                    int, (visit_handle,int,int), 
                    (obj,min,max));
}

int
VisIt_UnstructuredMesh_setGhostCells(visit_handle obj, visit_handle gz)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setGhostCells,
                    int, (visit_handle,visit_handle), 
                    (obj,gz));
}

int
VisIt_UnstructuredMesh_setGhostNodes(visit_handle obj, visit_handle gn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setGhostNodes,
                    int, (visit_handle,visit_handle), 
                    (obj,gn));
}

int
VisIt_UnstructuredMesh_setGlobalCellIds(visit_handle obj, visit_handle gz)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setGlobalCellIds,
                    int, (visit_handle,visit_handle), 
                    (obj,gz));
}

int
VisIt_UnstructuredMesh_setGlobalNodeIds(visit_handle obj, visit_handle gn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_setGlobalNodeIds,
                    int, (visit_handle,visit_handle), 
                    (obj,gn));
}

int
VisIt_UnstructuredMesh_getCoords(visit_handle obj,
    int *ndims, int *coordMode,
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *coords)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getCoords,
                    int, (visit_handle,int*,int*,visit_handle*,visit_handle*,visit_handle*,visit_handle*), 
                    (obj,ndims,coordMode,x,y,z,coords));
}

int
VisIt_UnstructuredMesh_getConnectivity(visit_handle obj, int *ncells,
    visit_handle *conn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getConnectivity,
                    int, (visit_handle,int*,visit_handle*), 
                    (obj,ncells, conn));
}

int
VisIt_UnstructuredMesh_getRealIndices(visit_handle obj, int *min, int *max)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getRealIndices,
                    int, (visit_handle,int*,int*), 
                    (obj,min,max));
}

int
VisIt_UnstructuredMesh_getGhostCells(visit_handle obj, visit_handle *gz)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getGhostCells,
                    int, (visit_handle,visit_handle*), 
                    (obj,gz));
}

int
VisIt_UnstructuredMesh_getGhostNodes(visit_handle obj, visit_handle *gn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getGhostNodes,
                    int, (visit_handle,visit_handle*), 
                    (obj,gn));
}

int
VisIt_UnstructuredMesh_getGlobalCellIds(visit_handle obj, visit_handle *gz)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getGlobalCellIds,
                    int, (visit_handle,visit_handle*), 
                    (obj,gz));
}

int
VisIt_UnstructuredMesh_getGlobalNodeIds(visit_handle obj, visit_handle *gn)
{
    VISIT_DYNAMIC_EXECUTE(UnstructuredMesh_getGlobalNodeIds,
                    int, (visit_handle,visit_handle*), 
                    (obj,gn));
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
#define F_VISITUCDMESHSETGHOSTCELLS      F77_ID(visitucdmeshsetghostcells_,visitucdmeshsetghostcells,VISITUCDMESHSETGHOSTCELLS)
#define F_VISITUCDMESHSETGHOSTNODES      F77_ID(visitucdmeshsetghostnodes_,visitucdmeshsetghostnodes,VISITUCDMESHSETGHOSTNODES)
#define F_VISITUCDMESHSETGLOBALCELLIDS   F77_ID(visitucdmeshsetglobalcellids_,visitucdmeshsetglobalcellids,VISITUCDMESHSETGLOBALCELLIDS)
#define F_VISITUCDMESHSETGLOBALNODEIDS   F77_ID(visitucdmeshsetglobalnodeids_,visitucdmeshsetglobalnodeids,VISITUCDMESHSETGLOBALNODEIDS)

#define F_VISITUCDMESHGETCOORDS          F77_ID(visitucdmeshgetcoords_,visitucdmeshgetcoords,VISITUCDMESHGETCOORDS)
#define F_VISITUCDMESHGETCONNECTIVITY    F77_ID(visitucdmeshgetconnectivity_,visitucdmeshgetconnectivity,VISITUCDMESHGETCONNECTIVITY)
#define F_VISITUCDMESHGETREALINDICES     F77_ID(visitucdmeshgetrealindices_,visitucdmeshgetrealindices,VISITUCDMESHGETREALINDICES)
#define F_VISITUCDMESHGETGHOSTCELLS      F77_ID(visitucdmeshgetghostcells_,visitucdmeshgetghostcells,VISITUCDMESHGETGHOSTCELLS)
#define F_VISITUCDMESHGETGHOSTNODES      F77_ID(visitucdmeshgetghostnodes_,visitucdmeshgetghostnodes,VISITUCDMESHGETGHOSTNODES)
#define F_VISITUCDMESHGETGLOBALCELLIDS   F77_ID(visitucdmeshgetglobalcellids_,visitucdmeshgetglobalcellids,VISITUCDMESHGETGLOBALCELLIDS)
#define F_VISITUCDMESHGETGLOBALNODEIDS   F77_ID(visitucdmeshgetglobalnodeids_,visitucdmeshgetglobalnodeids,VISITUCDMESHGETGLOBALNODEIDS)

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
F_VISITUCDMESHSETCONNECTIVITY(visit_handle *obj, int *ncells, visit_handle *c)
{
    return VisIt_UnstructuredMesh_setConnectivity(*obj, *ncells, *c);
}

int
F_VISITUCDMESHSETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_setRealIndices(*obj, *min, *max);
}

int
F_VISITUCDMESHSETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_UnstructuredMesh_setGhostCells(*obj, *gz);
}

int
F_VISITUCDMESHSETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_UnstructuredMesh_setGhostNodes(*obj, *gn);
}

int
F_VISITUCDMESHSETGLOBALCELLIDS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_UnstructuredMesh_setGlobalCellIds(*obj, *gz);
}

int
F_VISITUCDMESHSETGLOBALNODEIDS(visit_handle *obj, visit_handle *gn)
{
    return VisIt_UnstructuredMesh_setGlobalNodeIds(*obj, *gn);
}

int
F_VISITUCDMESHGETCOORDS(visit_handle *obj, int *ndims, int *coordMode, 
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c)
{
    return VisIt_UnstructuredMesh_getCoords(*obj, ndims, coordMode, x, y, z, c);
}

int
F_VISITUCDMESHGETCONNECTIVITY(visit_handle *obj, int *ncells, visit_handle *c)
{
    return VisIt_UnstructuredMesh_getConnectivity(*obj, ncells, c);
}

int
F_VISITUCDMESHGETREALINDICES(visit_handle *obj, int *min, int *max)
{
    return VisIt_UnstructuredMesh_getRealIndices(*obj, min, max);
}

int
F_VISITUCDMESHGETGHOSTCELLS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_UnstructuredMesh_getGhostCells(*obj, gz);
}

int
F_VISITUCDMESHGETGHOSTNODES(visit_handle *obj, visit_handle *gn)
{
    return VisIt_UnstructuredMesh_getGhostNodes(*obj, gn);
}

int
F_VISITUCDMESHGETGLOBALCELLIDS(visit_handle *obj, visit_handle *gz)
{
    return VisIt_UnstructuredMesh_getGlobalCellIds(*obj, gz);
}

int
F_VISITUCDMESHGETGLOBALNODEIDS(visit_handle *obj, visit_handle *gn)
{
    return VisIt_UnstructuredMesh_getGlobalNodeIds(*obj, gn);
}
