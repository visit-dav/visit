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
