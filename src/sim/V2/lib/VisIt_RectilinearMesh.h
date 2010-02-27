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

#ifdef __cplusplus
}
#endif

#endif
