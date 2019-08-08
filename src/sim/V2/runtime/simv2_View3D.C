// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <cstring>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include <View3DAttributes.h>

#include "simv2_View3D.h"

struct VisIt_View3D : public VisIt_ObjectBase
{
    VisIt_View3D();
    virtual ~VisIt_View3D();

    View3DAttributes view;
};

VisIt_View3D::VisIt_View3D() : VisIt_ObjectBase(VISIT_VIEW3D), view()
{
}

VisIt_View3D::~VisIt_View3D()
{
}

static VisIt_View3D *
GetObject(visit_handle h, const char *fname)
{
    char tmp[150];
    VisIt_View3D *obj = (VisIt_View3D *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_VIEW3D)
        {
            snprintf(tmp, 150, "%s: The provided handle does not point to "
                "a View3D object. (type=%d)", fname, obj->objectType());
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        snprintf(tmp, 150, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_View3D_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_View3D);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_View3D_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_setViewNormal(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setViewNormal");
    if(obj != NULL)
    {
        obj->view.SetViewNormal(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getViewNormal(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getViewNormal: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getViewNormal");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetViewNormal(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = 0.; val[2] = 1;
    }
    return retval;
}

int
simv2_View3D_setFocus(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setFocus");
    if(obj != NULL)
    {
        obj->view.SetFocus(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getFocus(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getFocus: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getFocus");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetFocus(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = val[2] = 0.;
    }
    return retval;
}

int
simv2_View3D_setViewUp(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setViewUp");
    if(obj != NULL)
    {
        obj->view.SetViewUp(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getViewUp(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getViewUp: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getViewUp");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetViewUp(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = val[2] = 0.;
    }
    return retval;
}

int
simv2_View3D_setViewAngle(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setViewAngle");
    if(obj != NULL)
    {
        obj->view.SetViewAngle(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getViewAngle(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getViewAngle: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getViewAngle");
    if(obj != NULL)
    {
        *val = obj->view.GetViewAngle();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setParallelScale(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setParallelScale");
    if(obj != NULL)
    {
        obj->view.SetParallelScale(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getParallelScale(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getParallelScale: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getParallelScale");
    if(obj != NULL)
    {
        *val = obj->view.GetParallelScale();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setNearPlane(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setNearPlane");
    if(obj != NULL)
    {
        obj->view.SetNearPlane(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getNearPlane(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getNearPlane: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getNearPlane");
    if(obj != NULL)
    {
        *val = obj->view.GetNearPlane();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setFarPlane(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setFarPlane");
    if(obj != NULL)
    {
        obj->view.SetFarPlane(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getFarPlane(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getFarPlane: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getFarPlane");
    if(obj != NULL)
    {
        *val = obj->view.GetFarPlane();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setImagePan(visit_handle h, double val[2])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setImagePan");
    if(obj != NULL)
    {
        obj->view.SetImagePan(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getImagePan(visit_handle h, double  val[2])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getImagePan: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getImagePan");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetImagePan(), 2 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = 0.;
    }
    return retval;
}

int
simv2_View3D_setImageZoom(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setImageZoom");
    if(obj != NULL)
    {
        obj->view.SetImageZoom(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getImageZoom(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getImageZoom: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getImageZoom");
    if(obj != NULL)
    {
        *val = obj->view.GetImageZoom();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setPerspective(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setPerspective");
    if(obj != NULL)
    {
        obj->view.SetPerspective(val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getPerspective(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getPerspective: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getPerspective");
    if(obj != NULL)
    {
        *val = obj->view.GetPerspective() ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setEyeAngle(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setEyeAngle");
    if(obj != NULL)
    {
        obj->view.SetEyeAngle(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getEyeAngle(visit_handle h, double* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getEyeAngle: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getEyeAngle");
    if(obj != NULL)
    {
        *val = obj->view.GetEyeAngle();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setCenterOfRotationSet(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setCenterOfRotationSet");
    if(obj != NULL)
    {
        obj->view.SetCenterOfRotationSet(val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getCenterOfRotationSet(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getCenterOfRotationSet: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getCenterOfRotationSet");
    if(obj != NULL)
    {
        *val = obj->view.GetCenterOfRotationSet() ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setCenterOfRotation(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setCenterOfRotation");
    if(obj != NULL)
    {
        obj->view.SetCenterOfRotation(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getCenterOfRotation(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getCenterOfRotation: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getCenterOfRotation");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetCenterOfRotation(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = val[2] = 0.;
    }
    return retval;
}

int
simv2_View3D_setAxis3DScaleFlag(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setAxis3DScaleFlag");
    if(obj != NULL)
    {
        obj->view.SetAxis3DScaleFlag(val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getAxis3DScaleFlag(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getAxis3DScaleFlag: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getAxis3DScaleFlag");
    if(obj != NULL)
    {
        *val = obj->view.GetAxis3DScaleFlag() ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View3D_setAxis3DScales(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setAxis3DScales");
    if(obj != NULL)
    {
        obj->view.SetAxis3DScales(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getAxis3DScales(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getAxis3DScales: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getAxis3DScales");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetAxis3DScales(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = val[2] = 0.;
    }
    return retval;
}

int
simv2_View3D_setShear(visit_handle h, double val[3])
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setShear");
    if(obj != NULL)
    {
        obj->view.SetShear(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getShear(visit_handle h, double  val[3])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getShear: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getShear");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetShear(), 3 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = val[2] = 0.;
    }
    return retval;
}

int
simv2_View3D_setWindowValid(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_setWindowValid");
    if(obj != NULL)
    {
        obj->view.SetWindowValid(val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View3D_getWindowValid(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View3D_getWindowValid: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_getWindowValid");
    if(obj != NULL)
    {
        *val = obj->view.GetWindowValid() ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

View3DAttributes *
simv2_View3D_GetAttributes(visit_handle h)
{
    View3DAttributes *retval = NULL;
    VisIt_View3D *obj = GetObject(h, "simv2_View3D_GetAttributes");
    if(obj != NULL)
    {
        retval = &obj->view;
    }
    return retval;
}

int
simv2_View3D_copy(visit_handle dest, visit_handle src)
{
    int retval = VISIT_ERROR;
    VisIt_View3D *srcobj = GetObject(src, "simv2_View3D_copy");
    VisIt_View3D *destobj = GetObject(dest, "simv2_View3D_copy");
    if(srcobj != NULL && destobj != NULL)
    {
        destobj->view = srcobj->view;
        retval = VISIT_OKAY;
    }
    return retval;
}
