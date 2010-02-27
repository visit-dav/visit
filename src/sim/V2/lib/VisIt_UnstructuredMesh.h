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
int VisIt_UnstructuredMesh_setConnectivity(visit_handle obj, int nzones, visit_handle c);
int VisIt_UnstructuredMesh_setRealIndices(visit_handle obj, int min, int max);

#ifdef __cplusplus
}
#endif

#endif
