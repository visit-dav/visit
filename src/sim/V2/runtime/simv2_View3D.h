// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMV2_VIEW3D_H
#define SIMV2_VIEW3D_H
#include <VisItSimV2Exports.h>
#include <VisItInterfaceTypes_V2.h>

// C-callable implementation of front end functions
#ifdef __cplusplus
extern "C" {
#endif

SIMV2_API int simv2_View3D_alloc(visit_handle *obj);
SIMV2_API int simv2_View3D_free(visit_handle obj);
SIMV2_API int simv2_View3D_setViewNormal(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getViewNormal(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setFocus(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getFocus(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setViewUp(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getViewUp(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setViewAngle(visit_handle h, double);
SIMV2_API int simv2_View3D_getViewAngle(visit_handle h, double*);
SIMV2_API int simv2_View3D_setParallelScale(visit_handle h, double);
SIMV2_API int simv2_View3D_getParallelScale(visit_handle h, double*);
SIMV2_API int simv2_View3D_setNearPlane(visit_handle h, double);
SIMV2_API int simv2_View3D_getNearPlane(visit_handle h, double*);
SIMV2_API int simv2_View3D_setFarPlane(visit_handle h, double);
SIMV2_API int simv2_View3D_getFarPlane(visit_handle h, double*);
SIMV2_API int simv2_View3D_setImagePan(visit_handle h, double [2]);
SIMV2_API int simv2_View3D_getImagePan(visit_handle h, double [2]);
SIMV2_API int simv2_View3D_setImageZoom(visit_handle h, double);
SIMV2_API int simv2_View3D_getImageZoom(visit_handle h, double*);
SIMV2_API int simv2_View3D_setPerspective(visit_handle h, int);
SIMV2_API int simv2_View3D_getPerspective(visit_handle h, int*);
SIMV2_API int simv2_View3D_setEyeAngle(visit_handle h, double);
SIMV2_API int simv2_View3D_getEyeAngle(visit_handle h, double*);
SIMV2_API int simv2_View3D_setCenterOfRotationSet(visit_handle h, int);
SIMV2_API int simv2_View3D_getCenterOfRotationSet(visit_handle h, int*);
SIMV2_API int simv2_View3D_setCenterOfRotation(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getCenterOfRotation(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setAxis3DScaleFlag(visit_handle h, int);
SIMV2_API int simv2_View3D_getAxis3DScaleFlag(visit_handle h, int*);
SIMV2_API int simv2_View3D_setAxis3DScales(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getAxis3DScales(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setShear(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_getShear(visit_handle h, double [3]);
SIMV2_API int simv2_View3D_setWindowValid(visit_handle h, int);
SIMV2_API int simv2_View3D_getWindowValid(visit_handle h, int*);

SIMV2_API int simv2_View3D_copy(visit_handle dest, visit_handle src);

#ifdef __cplusplus
};
#endif

// Callable from within the runtime and SimV2
class View3DAttributes;
View3DAttributes *simv2_View3D_GetAttributes(visit_handle h);

#endif
