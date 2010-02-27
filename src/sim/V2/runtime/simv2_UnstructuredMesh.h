#ifndef SIMV2_UNSTRUCTUREDMESH_H
#define SIMV2_UNSTRUCTUREDMESH_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_UnstructuredMesh_alloc(visit_handle*);
SIMV2_API int simv2_UnstructuredMesh_free(visit_handle);

SIMV2_API int simv2_UnstructuredMesh_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);
SIMV2_API int simv2_UnstructuredMesh_setCoordsXYZ(visit_handle obj, visit_handle y, visit_handle y, visit_handle z);
SIMV2_API int simv2_UnstructuredMesh_setCoords(visit_handle obj, visit_handle xyz);
SIMV2_API int simv2_UnstructuredMesh_setConnectivity(visit_handle obj, int nzones,
                                                     visit_handle conn);
SIMV2_API int simv2_UnstructuredMesh_setRealIndices(visit_handle obj, int,int);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_UnstructuredMesh_getData(visit_handle h, 
                                             int &ndims,
                                             int &coordMode,
                                             visit_handle &x,
                                             visit_handle &y,
                                             visit_handle &z,
                                             visit_handle &c,
                                             int &nzones,
                                             int &firstRealZone,
                                             int &lastRealZone,
                                             visit_handle &connectivity);

SIMV2_API int simv2_UnstructuredMesh_check(visit_handle h);

#endif
