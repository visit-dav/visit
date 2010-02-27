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

#ifdef __cplusplus
}
#endif

#endif
