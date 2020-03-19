// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_RECTILINEAR_MESH_H
#define VISIT_RECTILINEAR_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_RectilinearMesh_alloc(visit_handle *obj);
int VisIt_RectilinearMesh_free(visit_handle obj);
int VisIt_RectilinearMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_RectilinearMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_RectilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3]);
int VisIt_RectilinearMesh_setBaseIndex(visit_handle obj, int base_index[3]);
int VisIt_RectilinearMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_RectilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn);

int VisIt_RectilinearMesh_getCoords(visit_handle obj, int *ndims, visit_handle *x, visit_handle *y, visit_handle *z);
int VisIt_RectilinearMesh_getRealIndices(visit_handle obj, int min[3], int max[3]);
int VisIt_RectilinearMesh_getBaseIndex(visit_handle obj, int base_index[3]);
int VisIt_RectilinearMesh_getGhostCells(visit_handle h, visit_handle *gz);
int VisIt_RectilinearMesh_getGhostNodes(visit_handle h, visit_handle *gn);

#ifdef __cplusplus
}
#endif

#endif
