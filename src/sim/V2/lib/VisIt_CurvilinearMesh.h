// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_CURVILINEAR_MESH_H
#define VISIT_CURVILINEAR_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_CurvilinearMesh_alloc(visit_handle *obj);
int VisIt_CurvilinearMesh_free(visit_handle obj);
int VisIt_CurvilinearMesh_setCoordsXY(visit_handle obj, int dims[2], visit_handle x, visit_handle y);
int VisIt_CurvilinearMesh_setCoordsXYZ(visit_handle obj, int dims[3], visit_handle x, visit_handle y, visit_handle z);
int VisIt_CurvilinearMesh_setCoords2(visit_handle obj, int dims[2], visit_handle c);
int VisIt_CurvilinearMesh_setCoords3(visit_handle obj, int dims[3], visit_handle c);
int VisIt_CurvilinearMesh_setRealIndices(visit_handle obj, int min[3], int max[3]);
int VisIt_CurvilinearMesh_setBaseIndex(visit_handle obj, int base_index[3]);
int VisIt_CurvilinearMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_CurvilinearMesh_setGhostNodes(visit_handle obj, visit_handle gn);

int VisIt_CurvilinearMesh_getCoords(visit_handle h, int *ndims, int dims[3], int *coordMode, visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *c);
int VisIt_CurvilinearMesh_getRealIndices(visit_handle h, int min[3], int max[3]);
int VisIt_CurvilinearMesh_getBaseIndex(visit_handle h, int base_index[3]);
int VisIt_CurvilinearMesh_getGhostCells(visit_handle h, visit_handle *gz);
int VisIt_CurvilinearMesh_getGhostNodes(visit_handle h, visit_handle *gn);

#ifdef __cplusplus
}
#endif

#endif
