// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_POINTMESH_H
#define SIMV2_POINTMESH_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_PointMesh_alloc(visit_handle*);
SIMV2_API int simv2_PointMesh_free(visit_handle);

SIMV2_API int simv2_PointMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
SIMV2_API int simv2_PointMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
SIMV2_API int simv2_PointMesh_setCoords(visit_handle obj, visit_handle xyz);

SIMV2_API int simv2_PointMesh_getCoords(visit_handle h,
    int *ndims, int *coordMode,
    visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *coords);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_PointMesh_check(visit_handle h);

#endif
