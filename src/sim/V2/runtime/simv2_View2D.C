/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <cstring>
#include <snprintf.h>
#include <vectortypes.h>
#include "VisItDataInterfaceRuntime.h"
#include "VisItDataInterfaceRuntimeP.h"

#include <View2DAttributes.h>

#include "simv2_View2D.h"

struct VisIt_View2D : public VisIt_ObjectBase
{
    VisIt_View2D();
    virtual ~VisIt_View2D();

    View2DAttributes view;
};

VisIt_View2D::VisIt_View2D() : VisIt_ObjectBase(VISIT_VIEW2D), view()
{
}

VisIt_View2D::~VisIt_View2D()
{
}

static VisIt_View2D *
GetObject(visit_handle h, const char *fname)
{
    char tmp[150];
    VisIt_View2D *obj = (VisIt_View2D *)VisItGetPointer(h);
    if(obj != NULL)
    {
        if(obj->objectType() != VISIT_VIEW2D)
        {
            SNPRINTF(tmp, 150, "%s: The provided handle does not point to "
                "a View2D object. (type=%d)", fname, obj->objectType());
            VisItError(tmp);
            obj = NULL;
        }
    }
    else
    {
        SNPRINTF(tmp, 150, "%s: An invalid handle was provided.", fname);
        VisItError(tmp);
    }

    return obj;
}

/*******************************************************************************
 * Public functions, available to C 
 ******************************************************************************/

int
simv2_View2D_alloc(visit_handle *h)
{
    *h = VisItStorePointer(new VisIt_View2D);
    return (*h != VISIT_INVALID_HANDLE) ? VISIT_OKAY : VISIT_ERROR;
}

int
simv2_View2D_free(visit_handle h)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_free");
    if(obj != NULL)
    {
        delete obj;
        VisItFreePointer(h);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_setWindowCoords(visit_handle h, double val[4])
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setWindowCoords");
    if(obj != NULL)
    {
        obj->view.SetWindowCoords(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getWindowCoords(visit_handle h, double  val[4])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getWindowCoords: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getWindowCoords");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetWindowCoords(), 4 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = 1.; val[2] = 0.; val[3] = 1.;
    }
    return retval;
}

int
simv2_View2D_setViewportCoords(visit_handle h, double val[4])
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setViewportCoords");
    if(obj != NULL)
    {
        obj->view.SetViewportCoords(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getViewportCoords(visit_handle h, double  val[4])
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getViewportCoords: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getViewportCoords");
    if(obj != NULL)
    {
        memcpy(val, obj->view.GetViewportCoords(), 4 * sizeof(double));
        retval = VISIT_OKAY;
    }
    else
    {
        val[0] = val[1] = 1.; val[2] = 0.; val[3] = 1.;
    }
    return retval;
}

int
simv2_View2D_setFullFrameActivationMode(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setFullFrameActivationMode");
    if(obj != NULL)
    {
        if(val == 0)
        {
            obj->view.SetFullFrameActivationMode(View2DAttributes::On);
            retval = VISIT_OKAY;
        }
        else if(val == 1)
        {
            obj->view.SetFullFrameActivationMode(View2DAttributes::Off);
            retval = VISIT_OKAY;
        }
        else if(val == 2)
        {
            obj->view.SetFullFrameActivationMode(View2DAttributes::Auto);
            retval = VISIT_OKAY;
        }
    }
    return retval;
}

int
simv2_View2D_getFullFrameActivationMode(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getFullFrameActivationMode: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getFullFrameActivationMode");
    if(obj != NULL)
    {
        *val = obj->view.GetFullFrameActivationMode();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View2D_setFullFrameAutoThreshold(visit_handle h, double val)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setFullFrameAutoThreshold");
    if(obj != NULL)
    {
        obj->view.SetFullFrameAutoThreshold(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getFullFrameAutoThreshold(visit_handle h, double *val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getFullFrameAutoThreshold: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getFullFrameAutoThreshold");
    if(obj != NULL)
    {
        *val = obj->view.GetFullFrameAutoThreshold();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View2D_setXScale(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setXScale");
    if(obj != NULL)
    {
        obj->view.SetXScale(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getXScale(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getXScale: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getXScale");
    if(obj != NULL)
    {
        *val = obj->view.GetXScale();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View2D_setYScale(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setYScale");
    if(obj != NULL)
    {
        obj->view.SetYScale(val);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getYScale(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getYScale: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getYScale");
    if(obj != NULL)
    {
        *val = obj->view.GetYScale();
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

int
simv2_View2D_setWindowValid(visit_handle h, int val)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_setWindowValid");
    if(obj != NULL)
    {
        obj->view.SetWindowValid(val > 0);
        retval = VISIT_OKAY;
    }
    return retval;
}

int
simv2_View2D_getWindowValid(visit_handle h, int* val)
{
    int retval = VISIT_ERROR;
    if(val == NULL)
    {
        VisItError("simv2_View2D_getWindowValid: Invalid address.");
        return VISIT_ERROR;
    }
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_getWindowValid");
    if(obj != NULL)
    {
        *val = obj->view.GetWindowValid() ? 1 : 0;
        retval = VISIT_OKAY;
    }
    else
        *val = 0;
    return retval;
}

View2DAttributes *
simv2_View2D_GetAttributes(visit_handle h)
{
    View2DAttributes *retval = NULL;
    VisIt_View2D *obj = GetObject(h, "simv2_View2D_GetAttributes");
    if(obj != NULL)
    {
        retval = &obj->view;
    }
    return retval;
}

int
simv2_View2D_copy(visit_handle dest, visit_handle src)
{
    int retval = VISIT_ERROR;
    VisIt_View2D *srcobj = GetObject(src, "simv2_View2D_copy");
    VisIt_View2D *destobj = GetObject(dest, "simv2_View2D_copy");
    if(srcobj != NULL && destobj != NULL)
    {
        destobj->view = srcobj->view;
        retval = VISIT_OKAY;
    }
    return retval;
}
