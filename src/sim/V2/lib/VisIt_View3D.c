// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <VisItDataInterface_V2.h>
#include "VisItDynamic.h"
#include "VisItFortran.h"

int
VisIt_View3D_alloc(visit_handle *obj)
{
    VISIT_DYNAMIC_EXECUTE(View3D_alloc,
                    int, (visit_handle*),
                    (obj))
}

int
VisIt_View3D_free(visit_handle obj)
{
    VISIT_DYNAMIC_EXECUTE(View3D_free,
                    int, (visit_handle), 
                    (obj));
}

int
VisIt_View3D_setViewNormal(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setViewNormal,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getViewNormal(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getViewNormal,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setFocus(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setFocus,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getFocus(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getFocus,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setViewUp(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setViewUp,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getViewUp(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getViewUp,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setViewAngle(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setViewAngle,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getViewAngle(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getViewAngle,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setParallelScale(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setParallelScale,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getParallelScale(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getParallelScale,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setNearPlane(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setNearPlane,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getNearPlane(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getNearPlane,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setFarPlane(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setFarPlane,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getFarPlane(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getFarPlane,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setImagePan(visit_handle h, double val[2])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setImagePan,
        int, (visit_handle, double [2]),
        (h, val));
}

int
VisIt_View3D_getImagePan(visit_handle h, double val[2])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getImagePan,
        int, (visit_handle, double [2]),
        (h, val));
}

int
VisIt_View3D_setImageZoom(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setImageZoom,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getImageZoom(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getImageZoom,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setPerspective(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setPerspective,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View3D_getPerspective(visit_handle h, int*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getPerspective,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View3D_setEyeAngle(visit_handle h, double val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setEyeAngle,
        int, (visit_handle, double),
        (h, val));
}

int
VisIt_View3D_getEyeAngle(visit_handle h, double*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getEyeAngle,
        int, (visit_handle, double*),
        (h, val));
}

int
VisIt_View3D_setCenterOfRotationSet(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setCenterOfRotationSet,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View3D_getCenterOfRotationSet(visit_handle h, int*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getCenterOfRotationSet,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View3D_setCenterOfRotation(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setCenterOfRotation,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getCenterOfRotation(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getCenterOfRotation,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setAxis3DScaleFlag(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setAxis3DScaleFlag,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View3D_getAxis3DScaleFlag(visit_handle h, int*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getAxis3DScaleFlag,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View3D_setAxis3DScales(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setAxis3DScales,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getAxis3DScales(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getAxis3DScales,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setShear(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_setShear,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_getShear(visit_handle h, double val[3])
{
    VISIT_DYNAMIC_EXECUTE(View3D_getShear,
        int, (visit_handle, double [3]),
        (h, val));
}

int
VisIt_View3D_setWindowValid(visit_handle h, int val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_setWindowValid,
        int, (visit_handle, int),
        (h, val));
}

int
VisIt_View3D_getWindowValid(visit_handle h, int*val)
{
    VISIT_DYNAMIC_EXECUTE(View3D_getWindowValid,
        int, (visit_handle, int*),
        (h, val));
}

int
VisIt_View3D_copy(visit_handle dest, visit_handle src)
{
    VISIT_DYNAMIC_EXECUTE(View3D_copy,
                    int, (visit_handle,visit_handle), 
                    (dest,src));
}

/************************** Fortran callable routines *************************/
/* maxlen 012345678901234567890123456789                                      */
#define F_VISITVIEW3DALLOC                F77_ID(visitview3dalloc_,visitview3dalloc,VISITVIEW3DALLOC)
#define F_VISITVIEW3DFREE                 F77_ID(visitview3dfree_,visitview3dfree,VISITVIEW3DFREE)
#define F_VISITVIEW3DSETVIEWNORMAL        F77_ID(visitview3dsetviewnormal_,visitview3dsetviewnormal, VISITVIEW3DSETVIEWNORMAL)
#define F_VISITVIEW3DGETVIEWNORMAL        F77_ID(visitview3dgetviewnormal_,visitview3dgetviewnormal, VISITVIEW3DGETVIEWNORMAL)
#define F_VISITVIEW3DSETFOCUS             F77_ID(visitview3dsetfocus_,visitview3dsetfocus, VISITVIEW3DSETFOCUS)
#define F_VISITVIEW3DGETFOCUS             F77_ID(visitview3dgetfocus_,visitview3dgetfocus, VISITVIEW3DGETFOCUS)
#define F_VISITVIEW3DSETVIEWUP            F77_ID(visitview3dsetviewup_,visitview3dsetviewup, VISITVIEW3DSETVIEWUP)
#define F_VISITVIEW3DGETVIEWUP            F77_ID(visitview3dgetviewup_,visitview3dgetviewup, VISITVIEW3DGETVIEWUP)
#define F_VISITVIEW3DSETVIEWANGLE         F77_ID(visitview3dsetviewangle_,visitview3dsetviewangle, VISITVIEW3DSETVIEWANGLE)
#define F_VISITVIEW3DGETVIEWANGLE         F77_ID(visitview3dgetviewangle_,visitview3dgetviewangle, VISITVIEW3DGETVIEWANGLE)
#define F_VISITVIEW3DSETPARALLELSCALE     F77_ID(visitview3dsetparallelscale_,visitview3dsetparallelscale, VISITVIEW3DSETPARALLELSCALE)
#define F_VISITVIEW3DGETPARALLELSCALE     F77_ID(visitview3dgetparallelscale_,visitview3dgetparallelscale, VISITVIEW3DGETPARALLELSCALE)
#define F_VISITVIEW3DSETNEARPLANE         F77_ID(visitview3dsetnearplane_,visitview3dsetnearplane, VISITVIEW3DSETNEARPLANE)
#define F_VISITVIEW3DGETNEARPLANE         F77_ID(visitview3dgetnearplane_,visitview3dgetnearplane, VISITVIEW3DGETNEARPLANE)
#define F_VISITVIEW3DSETFARPLANE          F77_ID(visitview3dsetfarplane_,visitview3dsetfarplane, VISITVIEW3DSETFARPLANE)
#define F_VISITVIEW3DGETFARPLANE          F77_ID(visitview3dgetfarplane_,visitview3dgetfarplane, VISITVIEW3DGETFARPLANE)
#define F_VISITVIEW3DSETIMAGEPAN          F77_ID(visitview3dsetimagepan_,visitview3dsetimagepan, VISITVIEW3DSETIMAGEPAN)
#define F_VISITVIEW3DGETIMAGEPAN          F77_ID(visitview3dgetimagepan_,visitview3dgetimagepan, VISITVIEW3DGETIMAGEPAN)
#define F_VISITVIEW3DSETIMAGEZOOM         F77_ID(visitview3dsetimagezoom_,visitview3dsetimagezoom, VISITVIEW3DSETIMAGEZOOM)
#define F_VISITVIEW3DGETIMAGEZOOM         F77_ID(visitview3dgetimagezoom_,visitview3dgetimagezoom, VISITVIEW3DGETIMAGEZOOM)
#define F_VISITVIEW3DSETPERSPECTIVE       F77_ID(visitview3dsetperspective_,visitview3dsetperspective, VISITVIEW3DSETPERSPECTIVE)
#define F_VISITVIEW3DGETPERSPECTIVE       F77_ID(visitview3dgetperspective_,visitview3dgetperspective, VISITVIEW3DGETPERSPECTIVE)
#define F_VISITVIEW3DSETEYEANGLE          F77_ID(visitview3dseteyeangle_,visitview3dseteyeangle, VISITVIEW3DSETEYEANGLE)
#define F_VISITVIEW3DGETEYEANGLE          F77_ID(visitview3dgeteyeangle_,visitview3dgeteyeangle, VISITVIEW3DGETEYEANGLE)
#define F_VISITVIEW3DSETCENTEROFROTATIONSET  F77_ID(visitview3dsetcenterofrotationset_,visitview3dsetcenterofrotationset, VISITVIEW3DSETCENTEROFROTATIONSET)
#define F_VISITVIEW3DGETCENTEROFROTATIONSET  F77_ID(visitview3dgetcenterofrotationset_,visitview3dgetcenterofrotationset, VISITVIEW3DGETCENTEROFROTATIONSET)
#define F_VISITVIEW3DSETCENTEROFROTATION  F77_ID(visitview3dsetcenterofrotation_,visitview3dsetcenterofrotation, VISITVIEW3DSETCENTEROFROTATION)
#define F_VISITVIEW3DGETCENTEROFROTATION  F77_ID(visitview3dgetcenterofrotation_,visitview3dgetcenterofrotation, VISITVIEW3DGETCENTEROFROTATION)
#define F_VISITVIEW3DSETAXIS3DSCALEFLAG   F77_ID(visitview3dsetaxis3dscaleflag_,visitview3dsetaxis3dscaleflag, VISITVIEW3DSETAXIS3DSCALEFLAG)
#define F_VISITVIEW3DGETAXIS3DSCALEFLAG   F77_ID(visitview3dgetaxis3dscaleflag_,visitview3dgetaxis3dscaleflag, VISITVIEW3DGETAXIS3DSCALEFLAG)
#define F_VISITVIEW3DSETAXIS3DSCALES      F77_ID(visitview3dsetaxis3dscales_,visitview3dsetaxis3dscales, VISITVIEW3DSETAXIS3DSCALES)
#define F_VISITVIEW3DGETAXIS3DSCALES      F77_ID(visitview3dgetaxis3dscales_,visitview3dgetaxis3dscales, VISITVIEW3DGETAXIS3DSCALES)
#define F_VISITVIEW3DSETSHEAR             F77_ID(visitview3dsetshear_,visitview3dsetshear, VISITVIEW3DSETSHEAR)
#define F_VISITVIEW3DGETSHEAR             F77_ID(visitview3dgetshear_,visitview3dgetshear, VISITVIEW3DGETSHEAR)
#define F_VISITVIEW3DSETWINDOWVALID       F77_ID(visitview3dsetwindowvalid_,visitview3dsetwindowvalid, VISITVIEW3DSETWINDOWVALID)
#define F_VISITVIEW3DGETWINDOWVALID       F77_ID(visitview3dgetwindowvalid_,visitview3dgetwindowvalid, VISITVIEW3DGETWINDOWVALID)

#define F_VISITVIEW3DCOPY                 F77_ID(visitview3dcopy_,visitview3dcopy, VISITVIEW3DCOPY)

int
F_VISITVIEW3DALLOC(visit_handle *h)
{
    return VisIt_View3D_alloc(h);
}

int
F_VISITVIEW3DFREE(visit_handle *h)
{
    return VisIt_View3D_free(*h);
}

int
F_VISITVIEW3DSETVIEWNORMAL(visit_handle *h, double val[3])
{
    return VisIt_View3D_setViewNormal(*h, val);
}

int
F_VISITVIEW3DGETVIEWNORMAL(visit_handle *h, double val[3])
{
    return VisIt_View3D_getViewNormal(*h, val);
}

int
F_VISITVIEW3DSETFOCUS(visit_handle *h, double val[3])
{
    return VisIt_View3D_setFocus(*h, val);
}

int
F_VISITVIEW3DGETFOCUS(visit_handle *h, double val[3])
{
    return VisIt_View3D_getFocus(*h, val);
}

int
F_VISITVIEW3DSETVIEWUP(visit_handle *h, double val[3])
{
    return VisIt_View3D_setViewUp(*h, val);
}

int
F_VISITVIEW3DGETVIEWUP(visit_handle *h, double val[3])
{
    return VisIt_View3D_getViewUp(*h, val);
}

int
F_VISITVIEW3DSETVIEWANGLE(visit_handle *h, double *val)
{
    return VisIt_View3D_setViewAngle(*h, *val);
}

int
F_VISITVIEW3DGETVIEWANGLE(visit_handle *h, double *val)
{
    return VisIt_View3D_getViewAngle(*h, val);
}

int
F_VISITVIEW3DSETPARALLELSCALE(visit_handle *h, double *val)
{
    return VisIt_View3D_setParallelScale(*h, *val);
}

int
F_VISITVIEW3DGETPARALLELSCALE(visit_handle *h, double *val)
{
    return VisIt_View3D_getParallelScale(*h, val);
}

int
F_VISITVIEW3DSETNEARPLANE(visit_handle *h, double *val)
{
    return VisIt_View3D_setNearPlane(*h, *val);
}

int
F_VISITVIEW3DGETNEARPLANE(visit_handle *h, double *val)
{
    return VisIt_View3D_getNearPlane(*h, val);
}

int
F_VISITVIEW3DSETFARPLANE(visit_handle *h, double *val)
{
    return VisIt_View3D_setFarPlane(*h, *val);
}

int
F_VISITVIEW3DGETFARPLANE(visit_handle *h, double *val)
{
    return VisIt_View3D_getFarPlane(*h, val);
}

int
F_VISITVIEW3DSETIMAGEPAN(visit_handle *h, double val[2])
{
    return VisIt_View3D_setImagePan(*h, val);
}

int
F_VISITVIEW3DGETIMAGEPAN(visit_handle *h, double val[2])
{
    return VisIt_View3D_getImagePan(*h, val);
}

int
F_VISITVIEW3DSETIMAGEZOOM(visit_handle *h, double *val)
{
    return VisIt_View3D_setImageZoom(*h, *val);
}

int
F_VISITVIEW3DGETIMAGEZOOM(visit_handle *h, double *val)
{
    return VisIt_View3D_getImageZoom(*h, val);
}

int
F_VISITVIEW3DSETPERSPECTIVE(visit_handle *h, int *val)
{
    return VisIt_View3D_setPerspective(*h, *val);
}

int
F_VISITVIEW3DGETPERSPECTIVE(visit_handle *h, int *val)
{
    return VisIt_View3D_getPerspective(*h, val);
}

int
F_VISITVIEW3DSETEYEANGLE(visit_handle *h, double *val)
{
    return VisIt_View3D_setEyeAngle(*h, *val);
}

int
F_VISITVIEW3DGETEYEANGLE(visit_handle *h, double *val)
{
    return VisIt_View3D_getEyeAngle(*h, val);
}

int
F_VISITVIEW3DSETCENTEROFROTATIONSET(visit_handle *h, int *val)
{
    return VisIt_View3D_setCenterOfRotationSet(*h, *val);
}

int
F_VISITVIEW3DGETCENTEROFROTATIONSET(visit_handle *h, int *val)
{
    return VisIt_View3D_getCenterOfRotationSet(*h, val);
}

int
F_VISITVIEW3DSETCENTEROFROTATION(visit_handle *h, double val[3])
{
    return VisIt_View3D_setCenterOfRotation(*h, val);
}

int
F_VISITVIEW3DGETCENTEROFROTATION(visit_handle *h, double val[3])
{
    return VisIt_View3D_getCenterOfRotation(*h, val);
}

int
F_VISITVIEW3DSETAXIS3DSCALEFLAG(visit_handle *h, int *val)
{
    return VisIt_View3D_setAxis3DScaleFlag(*h, *val);
}

int
F_VISITVIEW3DGETAXIS3DSCALEFLAG(visit_handle *h, int *val)
{
    return VisIt_View3D_getAxis3DScaleFlag(*h, val);
}

int
F_VISITVIEW3DSETAXIS3DSCALES(visit_handle *h, double val[3])
{
    return VisIt_View3D_setAxis3DScales(*h, val);
}

int
F_VISITVIEW3DGETAXIS3DSCALES(visit_handle *h, double val[3])
{
    return VisIt_View3D_getAxis3DScales(*h, val);
}

int
F_VISITVIEW3DSETSHEAR(visit_handle *h, double val[3])
{
    return VisIt_View3D_setShear(*h, val);
}

int
F_VISITVIEW3DGETSHEAR(visit_handle *h, double val[3])
{
    return VisIt_View3D_getShear(*h, val);
}

int
F_VISITVIEW3DSETWINDOWVALID(visit_handle *h, int *val)
{
    return VisIt_View3D_setWindowValid(*h, *val);
}

int
F_VISITVIEW3DGETWINDOWVALID(visit_handle *h, int *val)
{
    return VisIt_View3D_getWindowValid(*h, val);
}

int
F_VISITVIEW3DCOPY(visit_handle *dest, visit_handle *src)
{
    return VisIt_View3D_copy(*dest, *src);
}
