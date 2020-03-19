// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_VIEW2D_H
#define SIMV2_VIEW2D_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_View2D_alloc(visit_handle *obj);
SIMV2_API int simv2_View2D_free(visit_handle obj);
SIMV2_API int simv2_View2D_setWindowCoords(visit_handle h, double [4]);
SIMV2_API int simv2_View2D_getWindowCoords(visit_handle h, double [4]);
SIMV2_API int simv2_View2D_setViewportCoords(visit_handle h, double [4]);
SIMV2_API int simv2_View2D_getViewportCoords(visit_handle h, double [4]);
SIMV2_API int simv2_View2D_setFullFrameActivationMode(visit_handle h, int);
SIMV2_API int simv2_View2D_getFullFrameActivationMode(visit_handle h, int);
SIMV2_API int simv2_View2D_setFullFrameAutoThreshold(visit_handle h, double);
SIMV2_API int simv2_View2D_getFullFrameAutoThreshold(visit_handle h, double*);
SIMV2_API int simv2_View2D_setXScale(visit_handle h, int);
SIMV2_API int simv2_View2D_getXScale(visit_handle h, int*);
SIMV2_API int simv2_View2D_setYScale(visit_handle h, int);
SIMV2_API int simv2_View2D_getYScale(visit_handle h, int*);
SIMV2_API int simv2_View2D_setWindowValid(visit_handle h, int);
SIMV2_API int simv2_View2D_getWindowValid(visit_handle h, int*);

SIMV2_API int simv2_View2D_copy(visit_handle dest, visit_handle src);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
class View2DAttributes;
View2DAttributes *simv2_View2D_GetAttributes(visit_handle h);

#endif
