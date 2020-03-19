// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_UNSTRUCTUREDMESH_H
#define SIMV2_UNSTRUCTUREDMESH_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_UnstructuredMesh_alloc(visit_handle*);
SIMV2_API int simv2_UnstructuredMesh_free(visit_handle);

SIMV2_API int simv2_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
SIMV2_API int simv2_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
SIMV2_API int simv2_UnstructuredMesh_setCoords(visit_handle obj, visit_handle xyz);
SIMV2_API int simv2_UnstructuredMesh_setConnectivity(visit_handle obj, int nzones,
                                                     visit_handle conn);
SIMV2_API int simv2_UnstructuredMesh_setRealIndices(visit_handle obj, int,int);
SIMV2_API int simv2_UnstructuredMesh_setGhostCells(visit_handle h, visit_handle gz);
SIMV2_API int simv2_UnstructuredMesh_setGhostNodes(visit_handle h, visit_handle gn);
SIMV2_API int simv2_UnstructuredMesh_setGlobalCellIds(visit_handle obj, visit_handle glz);
SIMV2_API int simv2_UnstructuredMesh_setGlobalNodeIds(visit_handle obj, visit_handle gln);

SIMV2_API int simv2_UnstructuredMesh_getCoords(visit_handle h,
                                               int *ndims, int *coordMode,
                                               visit_handle *x, 
                                               visit_handle *y, 
                                               visit_handle *z, 
                                               visit_handle *coords);
SIMV2_API int simv2_UnstructuredMesh_getConnectivity(visit_handle h, int *nzones,
                                                     visit_handle *conn);
SIMV2_API int simv2_UnstructuredMesh_getRealIndices(visit_handle obj, int *, int *);
SIMV2_API int simv2_UnstructuredMesh_getGhostCells(visit_handle h, visit_handle *gz);
SIMV2_API int simv2_UnstructuredMesh_getGhostNodes(visit_handle h, visit_handle *gn);
SIMV2_API int simv2_UnstructuredMesh_getGlobalCellIds(visit_handle obj, visit_handle *glz);
SIMV2_API int simv2_UnstructuredMesh_getGlobalNodeIds(visit_handle obj, visit_handle *gln);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_UnstructuredMesh_check(visit_handle h);

#endif
