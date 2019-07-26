// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_View2D_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(View2D_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_View2D_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(View2D_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_View2D_setWindowCoords(visit_handle h, double val[4])
{
    VISIT_DYNAMIC_EXECUTE(View2D_setWindowCoords,
        int, (visit_handle, double [4]),
        (h, val));
}

int
VisIt_View2D_getWindowCoords(visit_handle h, double val[4])
{
    VISIT_DYNAMIC_EXECUTE(View2D_getWindowCoords,
        int, (visit_handle, double [4]),
        (h, val));
}

int
VisIt_View2D_setViewportCoords(visit_handle h, double val[4])
{
    VISIT_DYNAMIC_EXECUTE(View2D_setViewportCoords,
        int, (visit_handle, double [4]),
        (h, val));
}

int
VisIt_View2D_getViewportCoords(visit_handle h, double val[4])
{
    VISIT_DYNAMIC_EXECUTE(View2D_getViewportCoords,
        int, (visit_handle, double [4]),
        (h, val));
}

int
VisIt_View2D_setFullFrameActivationMode(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_setFullFrameActivationMode,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View2D_getFullFrameActivationMode(visit_handle h, int *val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_getFullFrameActivationMode,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View2D_setFullFrameAutoThreshold(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_setFullFrameAutoThreshold,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View2D_getFullFrameAutoThreshold(visit_handle h, double* val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_getFullFrameAutoThreshold,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View2D_setXScale(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_setXScale,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View2D_getXScale(visit_handle h, int* val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_getXScale,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View2D_setYScale(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_setYScale,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View2D_getYScale(visit_handle h, int* val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_getYScale,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View2D_setWindowValid(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_setWindowValid,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View2D_getWindowValid(visit_handle h, int* val)
{
    VISIT_DYNAMIC_EXECUTE(View2D_getWindowValid,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View2D_copy(visit_handle dest, visit_handle src)
{
    VISIT_DYNAMIC_EXECUTE(View2D_copy,
                    int, (visit_handle,visit_handle), 
                    (dest,src));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITVIEW2DALLOC                F77_ID(visitview2dalloc_,visitview2dalloc,VISITVIEW2DALLOC)
#define F_VISITVIEW2DFREE                 F77_ID(visitview2dfree_,visitview2dfree,VISITVIEW2DFREE)
#define F_VISITVIEW2DCOPY                 F77_ID(visitview2dcopy_,visitview2dcopy, VISITVIEW2DCOPY)
#define F_VISITVIEW2DSETWINDOWCOORDS      F77_ID(visitview2dsetwindowcoords_,visitview2dsetwindowcoords,VISITVIEW2DSETWINDOWCOORDS)
#define F_VISITVIEW2DGETWINDOWCOORDS      F77_ID(visitview2dgetwindowcoords_,visitview2dgetwindowcoords,VISITVIEW2DGETWINDOWCOORDS)
#define F_VISITVIEW2DSETVIEWPORTCOORDS    F77_ID(visitview2dsetviewportcoords_,visitview2dsetviewportcoords,VISITVIEW2DSETVIEWPORTCOORDS)
#define F_VISITVIEW2DGETVIEWPORTCOORDS    F77_ID(visitview2dgetviewportcoords_,visitview2dgetviewportcoords,VISITVIEW2DGETVIEWPORTCOORDS)
#define F_VISITVIEW2DSETFFMODE            F77_ID(visitview2dsetffmode_,visitview2dsetffmode,VISITVIEW2DSETFFMODE)
#define F_VISITVIEW2DGETFFMODE            F77_ID(visitview2dgetffmode_,visitview2dgetffmode,VISITVIEW2DGETFFMODE)
#define F_VISITVIEW2DSETFFAUTOTHRESHOLD   F77_ID(visitview2dsetffautothreshold_,visitview2dsetffautothreshold,VISITVIEW2DSETAUTOTHRESHOLD)
#define F_VISITVIEW2DGETFFAUTOTHRESHOLD   F77_ID(visitview2dgetffautothreshold_,visitview2dgetffautothreshold,VISITVIEW2DGETAUTOTHRESHOLD)
#define F_VISITVIEW2DSETXSCALE            F77_ID(visitview2dsetxscale_,visitview2dsetxscale,VISITVIEW2DSETXSCALE)
#define F_VISITVIEW2DGETXSCALE            F77_ID(visitview2dgetxscale_,visitview2dgetxscale,VISITVIEW2DGETXSCALE)
#define F_VISITVIEW2DSETYSCALE            F77_ID(visitview2dsetyscale_,visitview2dsetyscale,VISITVIEW2DSETYSCALE)
#define F_VISITVIEW2DGETYSCALE            F77_ID(visitview2dgetyscale_,visitview2dgetyscale,VISITVIEW2DGETYSCALE)
#define F_VISITVIEW2DSETWINDOWVALID       F77_ID(visitview2dsetwindowvalid_,visitview2dsetwindowvalid,VISITVIEW2DSETWINDOWVALID)
#define F_VISITVIEW2DGETWINDOWVALID       F77_ID(visitview2dgetwindowvalid_,visitview2dgetwindowvalid,VISITVIEW2DGETWINDOWVALID)

int
F_VISITVIEW2DALLOC(visit_handle *h)
{
    return VisIt_View2D_alloc(h);
}

int
F_VISITVIEW2DFREE(visit_handle *h)
{
    return VisIt_View2D_free(*h);
}

int
F_VISITVIEW2DSETWINDOWCOORDS(visit_handle *h, double val[4])
{
    return VisIt_View2D_setWindowCoords(*h, val);
}

int
F_VISITVIEW2DGETWINDOWCOORDS(visit_handle *h, double val[4])
{
    return VisIt_View2D_getWindowCoords(*h, val);
}

int
F_VISITVIEW2DSETVIEWPORTCOORDS(visit_handle *h, double val[4])
{
    return VisIt_View2D_setViewportCoords(*h, val);
}

int
F_VISITVIEW2DGETVIEWPORTCOORDS(visit_handle *h, double val[4])
{
    return VisIt_View2D_getViewportCoords(*h, val);
}

int
F_VISITVIEW2DSETFFMODE(visit_handle *h, int *val)
{
    return VisIt_View2D_setFullFrameActivationMode(*h, *val);
}

int
F_VISITVIEW2DGETFFMODE(visit_handle *h, int *val)
{
    return VisIt_View2D_getFullFrameActivationMode(*h, val);
}

int
F_VISITVIEW2DSETFFAUTOTHRESHOLD(visit_handle *h, double *val)
{
    return VisIt_View2D_setFullFrameAutoThreshold(*h, *val);
}

int
F_VISITVIEW2DGETFFAUTOTHRESHOLD(visit_handle *h, double *val)
{
    return VisIt_View2D_getFullFrameAutoThreshold(*h, val);
}

int
F_VISITVIEW2DSETXSCALE(visit_handle *h, int *val)
{
    return VisIt_View2D_setXScale(*h, *val);
}

int
F_VISITVIEW2DGETXSCALE(visit_handle *h, int *val)
{
    return VisIt_View2D_getXScale(*h, val);
}

int
F_VISITVIEW2DSETYSCALE(visit_handle *h, int *val)
{
    return VisIt_View2D_setYScale(*h, *val);
}

int
F_VISITVIEW2DGETYSCALE(visit_handle *h, int *val)
{
    return VisIt_View2D_getYScale(*h, val);
}

int
F_VISITVIEW2DSETWINDOWVALID(visit_handle *h, int *val)
{
    return VisIt_View2D_setWindowValid(*h, *val);
}

int
F_VISITVIEW2DGETWINDOWVALID(visit_handle *h, int *val)
{
    return VisIt_View2D_getWindowValid(*h, val);
}

int
F_VISITVIEW2DCOPY(visit_handle *dest, visit_handle *src)
{
    return VisIt_View2D_copy(*dest, *src);
}
