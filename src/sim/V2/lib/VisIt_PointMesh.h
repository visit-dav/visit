// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_POINTMESH_H
#define VISIT_POINTMESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_PointMesh_alloc(visit_handle *obj);
int VisIt_PointMesh_free(visit_handle obj);
int VisIt_PointMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_PointMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_PointMesh_setCoords(visit_handle obj, visit_handle c);

int VisIt_PointMesh_getCoords(visit_handle obj, int *ndims, int *coordMode, visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c);

#ifdef __cplusplus
}
#endif

#endif
