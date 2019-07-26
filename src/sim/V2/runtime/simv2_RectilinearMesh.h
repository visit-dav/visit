// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_RECTILINEARMESH_H
#define SIMV2_RECTILINEARMESH_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_RectilinearMesh_alloc(visit_handle*);
SIMV2_API int simv2_RectilinearMesh_free(visit_handle);

SIMV2_API int simv2_RectilinearMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
SIMV2_API int simv2_RectilinearMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
SIMV2_API int simv2_RectilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3]);
SIMV2_API int simv2_RectilinearMesh_setBaseIndex(visit_handle obj, int base_index[3]);
SIMV2_API int simv2_RectilinearMesh_setGhostCells(visit_handle h, visit_handle gz);
SIMV2_API int simv2_RectilinearMesh_setGhostNodes(visit_handle h, visit_handle gn);

SIMV2_API int simv2_RectilinearMesh_getCoords(visit_handle h, int *ndims, 
                  visit_handle *x, visit_handle *y, visit_handle *z);
SIMV2_API int simv2_RectilinearMesh_getRealIndices(visit_handle h, int min[3], int max[3]);
SIMV2_API int simv2_RectilinearMesh_getBaseIndex(visit_handle h, int base_index[3]);
SIMV2_API int simv2_RectilinearMesh_getGhostCells(visit_handle h, visit_handle *gz);
SIMV2_API int simv2_RectilinearMesh_getGhostNodes(visit_handle h, visit_handle *gn);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_RectilinearMesh_check(visit_handle h);

#endif
