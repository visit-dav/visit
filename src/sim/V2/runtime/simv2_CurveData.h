#ifndef SIMV2_CURVEDATA_H
#define SIMV2_CURVEDATA_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C"
{
#endif

SIMV2_API int simv2_CurveData_alloc(visit_handle*);
SIMV2_API int simv2_CurveData_free(visit_handle);

SIMV2_API int simv2_CurveData_setCoordsXY(visit_handle obj, visit_handle x, visit_handle y);

#ifdef __cplusplus
}
#endif

// Callable from within the runtime and SimV2
SIMV2_API int simv2_CurveData_getData(visit_handle h, 
                                      visit_handle &x,
                                      visit_handle &y);

SIMV2_API int simv2_CurveData_check(visit_handle h);

#endif
