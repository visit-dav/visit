#ifndef VISIT_CSG_MESH_H
#define VISIT_CSG_MESH_H

#ifdef __cplusplus
extern "C"
{
#endif

int VisIt_CSGMesh_alloc(visit_handle *obj);
int VisIt_CSGMesh_free(visit_handle obj);
int VisIt_CSGMesh_setRegions(visit_handle obj, visit_handle boolops, visit_handle leftids, visit_handle rightids);
int VisIt_CSGMesh_setZonelist(visit_handle obj, visit_handle zl);
int VisIt_CSGMesh_setBoundaryTypes(visit_handle obj, visit_handle csgtypes);
int VisIt_CSGMesh_setBoundaryCoeffs(visit_handle obj, visit_handle coeffs);
int VisIt_CSGMesh_setExtents(visit_handle obj, double min[3], double max[3]);

#ifdef __cplusplus
}
#endif

#endif
