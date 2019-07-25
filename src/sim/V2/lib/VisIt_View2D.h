// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VISIT_VIEW2D_H
#define VISIT_VIEW2D_H

#ifdef __cplusplus
extern "C" {
#endif

int VisIt_View2D_alloc(visit_handle *obj);
int VisIt_View2D_free(visit_handle obj);
int VisIt_View2D_setWindowCoords(visit_handle h, double [4]);
int VisIt_View2D_getWindowCoords(visit_handle h, double [4]);
int VisIt_View2D_setViewportCoords(visit_handle h, double [4]);
int VisIt_View2D_getViewportCoords(visit_handle h, double [4]);
int VisIt_View2D_setFullFrameActivationMode(visit_handle h, int);
int VisIt_View2D_getFullFrameActivationMode(visit_handle h, int *);
int VisIt_View2D_setFullFrameAutoThreshold(visit_handle h, double);
int VisIt_View2D_getFullFrameAutoThreshold(visit_handle h, double*);
int VisIt_View2D_setXScale(visit_handle h, int);
int VisIt_View2D_getXScale(visit_handle h, int*);
int VisIt_View2D_setYScale(visit_handle h, int);
int VisIt_View2D_getYScale(visit_handle h, int*);
int VisIt_View2D_setWindowValid(visit_handle h, int);
int VisIt_View2D_getWindowValid(visit_handle h, int*);

int VisIt_View2D_copy(visit_handle dest, visit_handle src);

#ifdef __cplusplus
}
#endif

#endif
