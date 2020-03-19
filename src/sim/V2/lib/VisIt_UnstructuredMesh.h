// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_UNSTRUCTURED_MESH_H
#define VISIT_UNSTRUCTURED_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_UnstructuredMesh_alloc(visit_handle *obj);
int VisIt_UnstructuredMesh_free(visit_handle obj);
int VisIt_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
int VisIt_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle x, visit_handle y, visit_handle z);
int VisIt_UnstructuredMesh_setCoords(visit_handle obj, visit_handle c);
int VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int nCells, visit_handle c);
int VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max);
int VisIt_UnstructuredMesh_setGhostCells(visit_handle obj, visit_handle gz);
int VisIt_UnstructuredMesh_setGhostNodes(visit_handle obj, visit_handle gn);
int VisIt_UnstructuredMesh_setGlobalCellIds(visit_handle obj, visit_handle glz);
int VisIt_UnstructuredMesh_setGlobalNodeIds(visit_handle obj, visit_handle gln);

int VisIt_UnstructuredMesh_getCoords(visit_handle h,int *ndims, int *coordMode,visit_handle *x, visit_handle *y, visit_handle *z, visit_handle *coords);
int VisIt_UnstructuredMesh_getConnectivity(visit_handle h, int *nCells, visit_handle *conn);
int VisIt_UnstructuredMesh_getRealIndices(visit_handle obj, int *, int *);
int VisIt_UnstructuredMesh_getGhostCells(visit_handle h, visit_handle *gz);
int VisIt_UnstructuredMesh_getGhostNodes(visit_handle h, visit_handle *gn);
int VisIt_UnstructuredMesh_getGlobalCellIds(visit_handle obj, visit_handle *glz);
int VisIt_UnstructuredMesh_getGlobalNodeIds(visit_handle obj, visit_handle *gln);

#ifdef __cplusplus
}
#endif

#endif
