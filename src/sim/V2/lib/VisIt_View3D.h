// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VIEW3D_H
#define VISIT_VIEW3D_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_View3D_alloc(visit_handle *obj);
int VisIt_View3D_free(visit_handle obj);
int VisIt_View3D_setViewNormal(visit_handle h, double [3]);
int VisIt_View3D_getViewNormal(visit_handle h, double [3]);
int VisIt_View3D_setFocus(visit_handle h, double [3]);
int VisIt_View3D_getFocus(visit_handle h, double [3]);
int VisIt_View3D_setViewUp(visit_handle h, double [3]);
int VisIt_View3D_getViewUp(visit_handle h, double [3]);
int VisIt_View3D_setViewAngle(visit_handle h, double);
int VisIt_View3D_getViewAngle(visit_handle h, double*);
int VisIt_View3D_setParallelScale(visit_handle h, double);
int VisIt_View3D_getParallelScale(visit_handle h, double*);
int VisIt_View3D_setNearPlane(visit_handle h, double);
int VisIt_View3D_getNearPlane(visit_handle h, double*);
int VisIt_View3D_setFarPlane(visit_handle h, double);
int VisIt_View3D_getFarPlane(visit_handle h, double*);
int VisIt_View3D_setImagePan(visit_handle h, double [2]);
int VisIt_View3D_getImagePan(visit_handle h, double [2]);
int VisIt_View3D_setImageZoom(visit_handle h, double);
int VisIt_View3D_getImageZoom(visit_handle h, double*);
int VisIt_View3D_setPerspective(visit_handle h, int);
int VisIt_View3D_getPerspective(visit_handle h, int*);
int VisIt_View3D_setEyeAngle(visit_handle h, double);
int VisIt_View3D_getEyeAngle(visit_handle h, double*);
int VisIt_View3D_setCenterOfRotationSet(visit_handle h, int);
int VisIt_View3D_getCenterOfRotationSet(visit_handle h, int*);
int VisIt_View3D_setCenterOfRotation(visit_handle h, double [3]);
int VisIt_View3D_getCenterOfRotation(visit_handle h, double [3]);
int VisIt_View3D_setAxis3DScaleFlag(visit_handle h, int);
int VisIt_View3D_getAxis3DScaleFlag(visit_handle h, int*);
int VisIt_View3D_setAxis3DScales(visit_handle h, double [3]);
int VisIt_View3D_getAxis3DScales(visit_handle h, double [3]);
int VisIt_View3D_setShear(visit_handle h, double [3]);
int VisIt_View3D_getShear(visit_handle h, double [3]);
int VisIt_View3D_setWindowValid(visit_handle h, int);
int VisIt_View3D_getWindowValid(visit_handle h, int*);

int VisIt_View3D_copy(visit_handle dest, visit_handle src);

#ifdef __cplusplus
}
#endif

#endif
