#ifndef SIMV2_CSGMESH_H
#define SIMV2_CSGMESH_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_CSGMesh_alloc(visit_handle*);
SIMV2_API int simv2_CSGMesh_free(visit_handle);

SIMV2_API int simv2_CSGMesh_setRegions(visit_handle obj, visit_handle typeflags,
                                       visit_handle left, visit_handle right);

SIMV2_API int simv2_CSGMesh_setZonelist(visit_handle obj, visit_handle zl);
SIMV2_API int simv2_CSGMesh_setBoundaryTypes(visit_handle obj, visit_handle cshtypes);
SIMV2_API int simv2_CSGMesh_setBoundaryCoeffs(visit_handle obj, visit_handle coeffs);
SIMV2_API int simv2_CSGMesh_setExtents(visit_handle obj, double min[3], double max[3]);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_CSGMesh_getData(visit_handle h, 
                                    visit_handle &typeflags,
                                    visit_handle &left,
                                    visit_handle &right,
                                    visit_handle &zl,
                                    visit_handle &bndtypes,
                                    visit_handle &bndcoeff,
                                    double min[3],
                                    double max[3]);

SIMV2_API int simv2_CSGMesh_check(visit_handle h);

#endif
