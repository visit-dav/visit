/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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

// ************************************************************************* //
//                                avtView3D.C                                //
// ************************************************************************* //

#include <avtView3D.h>

#include <math.h>

#include <cmath>
#include <algorithm>

#include <avtViewInfo.h>
#include <View3DAttributes.h>

#include <vtkCamera.h>
#include <vtkMatrix4x4.h>


// ****************************************************************************
//  Method: avtView3D constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
// ****************************************************************************

avtView3D::avtView3D()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtView3D operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 16:02:13 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Wed Jan  8 16:46:42 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
// ****************************************************************************

avtView3D &
avtView3D::operator=(const avtView3D &vi)
{
    normal[0]           = vi.normal[0];
    normal[1]           = vi.normal[1];
    normal[2]           = vi.normal[2];
    focus[0]            = vi.focus[0];
    focus[1]            = vi.focus[1];
    focus[2]            = vi.focus[2];
    viewUp[0]           = vi.viewUp[0];
    viewUp[1]           = vi.viewUp[1];
    viewUp[2]           = vi.viewUp[2];
    viewAngle           = vi.viewAngle;
    parallelScale       = vi.parallelScale;
    nearPlane           = vi.nearPlane;
    farPlane            = vi.farPlane;
    imagePan[0]         = vi.imagePan[0];
    imagePan[1]         = vi.imagePan[1];
    imageZoom           = vi.imageZoom;
    perspective         = vi.perspective;
    eyeAngle            = vi.eyeAngle;
    centerOfRotationSet = vi.centerOfRotationSet;
    centerOfRotation[0] = vi.centerOfRotation[0];
    centerOfRotation[1] = vi.centerOfRotation[1];
    centerOfRotation[2] = vi.centerOfRotation[2];
    axis3DScaleFlag     = vi.axis3DScaleFlag;
    axis3DScales[0]     = vi.axis3DScales[0];
    axis3DScales[1]     = vi.axis3DScales[1];
    axis3DScales[2]     = vi.axis3DScales[2];
    shear[0]            = vi.shear[0];
    shear[1]            = vi.shear[1];
    shear[2]            = vi.shear[2];
    windowValid         = vi.windowValid;

    return *this;
}

// ****************************************************************************
//  Method: avtView3D operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 16:02:13 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Wed Jan  8 16:46:42 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
// ****************************************************************************

bool
avtView3D::operator==(const avtView3D &vi)
{
    if (normal[0] != vi.normal[0] || normal[1] != vi.normal[1] ||
        normal[2] != vi.normal[2])
    {
        return false;
    }

    if (focus[0] != vi.focus[0] || focus[1] != vi.focus[1] ||
        focus[2] != vi.focus[2])
    {
        return false;
    }

    if (viewUp[0] != vi.viewUp[0] || viewUp[1] != vi.viewUp[1] ||
        viewUp[2] != vi.viewUp[2])
    {
        return false;
    }

    if (centerOfRotation[0] != vi.centerOfRotation[0] ||
        centerOfRotation[1] != vi.centerOfRotation[1] ||
        centerOfRotation[2] != vi.centerOfRotation[2])
    {
        return false;
    }

    if (axis3DScales[0] != vi.axis3DScales[0] ||
        axis3DScales[1] != vi.axis3DScales[1] ||
        axis3DScales[2] != vi.axis3DScales[2])
    {
        return false;
    }

    if (shear[0] != vi.shear[0] || shear[1] != vi.shear[1] ||
        shear[2] != vi.shear[2])
    {
        return false;
    }

    if (viewAngle != vi.viewAngle || parallelScale != vi.parallelScale ||
        nearPlane != vi.nearPlane || farPlane != vi.farPlane ||
        imagePan[0] != vi.imagePan[0] || imagePan[1] != vi.imagePan[1] ||
        imageZoom != vi.imageZoom || perspective != vi.perspective ||
        eyeAngle != vi.eyeAngle ||
        centerOfRotationSet != vi.centerOfRotationSet ||
        axis3DScaleFlag != vi.axis3DScaleFlag ||
        windowValid != vi.windowValid)
    {
        return false;
    }

    return true;
}

// ****************************************************************************
//  Method: avtView3D::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:12:41 PST 2002
//    I changed the default values for the near and far clipping planes
//    since I changed their definition to be the distance from the focus
//    instead of the camera.
//
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 16:02:13 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Wed Jan  8 16:46:42 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
// ****************************************************************************

void
avtView3D::SetToDefault()
{
    normal[0]           = 0.;
    normal[1]           = 0.;
    normal[2]           = 1.;
    focus[0]            = 0.;
    focus[1]            = 0.;
    focus[2]            = 0.;
    viewUp[0]           = 0.;
    viewUp[1]           = 1.;
    viewUp[2]           = 0.;
    viewAngle           = 30.;
    parallelScale       = 0.5;
    nearPlane           = -0.5;
    farPlane            =  0.5;
    imagePan[0]         = 0.;
    imagePan[1]         = 0.;
    imageZoom           = 1.;
    eyeAngle            = 2.0;
    perspective         = false;
    centerOfRotationSet = false;
    centerOfRotation[0] = 0.;
    centerOfRotation[1] = 0.;
    centerOfRotation[2] = 0.;
    axis3DScaleFlag     = false;
    axis3DScales[0]     = 1.0;
    axis3DScales[1]     = 1.0;
    axis3DScales[2]     = 1.0;
    shear[0]            = 0.;
    shear[1]            = 0.;
    shear[2]            = 1.;
    windowValid         = false;
}

// ****************************************************************************
//  Method: avtView3D::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3D view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the 3D view. 
//
//  Programmer:  Eric Brugger
//  Creation:    August 17, 2001
//
//  Modifications:
//    Eric Brugger, Fri Mar 29 15:12:41 PST 2002
//    I changed the definition of the near and far clipping planes to be
//    the distance from the focus instead of the camera.
//
//    Eric Brugger, Tue Apr  2 16:25:51 PST 2002
//    I added code to set a minimum value for the vtk near clipping distance.
//
//    Eric Brugger, Mon Apr 15 12:30:01 PDT 2002
//    I modified the setting of the minimum value for the vtk near clipping
//    distance to be adaptive based on the distance between the avt near and
//    far clipping distances.
//
//    Brad Whitlock, Tue May 7 14:50:54 PST 2002
//    Fixed a bug I introduced when porting to MS Windows.
//
//    Eric Brugger, Wed Jan  8 13:34:01 PST 2003
//    I modified the routine to normalize the normal before using it.
//
//    Eric Brugger, Fri Jun  6 15:24:17 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Mark C. Miller, Tue Nov 16 17:25:30 PST 2004
//    Added '- distance' to second case in max to compute nearPlane 
//
//    Mark C. Miller, Thu Nov 18 21:25:36 PST 2004
//    Undid previous change. It could result in negative nearPlane values
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
// ****************************************************************************

void
avtView3D::SetViewInfoFromView(avtViewInfo &viewInfo) const
{
    double    distance;
    double    normal2[3];

    //
    // Calculate a unit length normal.
    //
    distance = sqrt(normal[0] * normal[0] + normal[1] * normal[1] +
                    normal[2] * normal[2]);
    distance = (distance != 0) ? distance : 1.;
    normal2[0] = normal[0] / distance;
    normal2[1] = normal[1] / distance;
    normal2[2] = normal[2] / distance;

    //
    // The view up vector and focal point are the same.  The distance from the
    // camera to the focal point can be calculated from the parallel scale and
    // view angle.  The camera position is then found by moving along the view
    // plane normal from the focal point by the distance.
    //
    viewInfo.viewUp[0] = viewUp[0];
    viewInfo.viewUp[1] = viewUp[1];
    viewInfo.viewUp[2] = viewUp[2];

    viewInfo.focus[0] = focus[0];
    viewInfo.focus[1] = focus[1];
    viewInfo.focus[2] = focus[2];

    viewInfo.eyeAngle = eyeAngle;

    distance = parallelScale / tan(viewAngle * 3.1415926535 / 360.);
    viewInfo.camera[0] = focus[0] + normal2[0] * distance;
    viewInfo.camera[1] = focus[1] + normal2[1] * distance;
    viewInfo.camera[2] = focus[2] + normal2[2] * distance;

    //
    // Orthographic is the opposite of perspective, setScale is always true.
    // It forces vtk to use the parallel scale.
    //
    viewInfo.orthographic  = !perspective;
    viewInfo.setScale      = true;
    viewInfo.parallelScale = parallelScale;
    viewInfo.viewAngle     = viewAngle;

    //
    // The minimum near clipping distance must be adaptive to make good use
    // of the zbuffer.  The distance between the near and far planes seemed
    // like a good choice, another possibility could have been the distance
    // between the camera and focus.  The 5000. is a magic number.  The
    // number should be as large as possible.  10000 would probably also
    // work, but 100000 would start showing z buffering artifacts.
    //
    viewInfo.nearPlane = std::max(nearPlane + distance, (farPlane - nearPlane) / 5000.);
    viewInfo.farPlane = farPlane + distance;

    //
    // Set the image pan and image zoom.
    //
    viewInfo.imagePan[0] = -imagePan[0];
    viewInfo.imagePan[1] = -imagePan[1];
    viewInfo.imageZoom   = imageZoom;

    //
    // Set the vew shear.
    //
    viewInfo.shear[0] = shear[0];
    viewInfo.shear[1] = shear[1];
    viewInfo.shear[2] = shear[2];
}

// ****************************************************************************
//  Method: avtView3D::SetFromView3DAttributes
//
//  Purpose: 
//    Sets the avtView3D from a View3DAttributes object.
//
//  Arguments:
//    view3DAtts : The View3DAttributes to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:00:50 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:39:11 PDT 2003
//    I renamed this routine.
//   
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 16:02:13 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Wed Jan  8 16:46:42 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
// ****************************************************************************

void
avtView3D::SetFromView3DAttributes(const View3DAttributes *view3DAtts)
{
    for(int i = 0; i < 3; ++i)
    {
        normal[i] = view3DAtts->GetViewNormal()[i];
        focus[i]  = view3DAtts->GetFocus()[i];
        viewUp[i] = view3DAtts->GetViewUp()[i];
        centerOfRotation[i] = view3DAtts->GetCenterOfRotation()[i];
        axis3DScales[i] = view3DAtts->GetAxis3DScales()[i];
        shear[i] = view3DAtts->GetShear()[i];
    }

    viewAngle = view3DAtts->GetViewAngle();
    parallelScale = view3DAtts->GetParallelScale();
    nearPlane = view3DAtts->GetNearPlane();
    farPlane = view3DAtts->GetFarPlane();
    imagePan[0] = view3DAtts->GetImagePan()[0];
    imagePan[1] = view3DAtts->GetImagePan()[1];
    imageZoom = view3DAtts->GetImageZoom();
    perspective = view3DAtts->GetPerspective();
    eyeAngle = view3DAtts->GetEyeAngle();
    centerOfRotationSet = view3DAtts->GetCenterOfRotationSet();
    axis3DScaleFlag = view3DAtts->GetAxis3DScaleFlag();
    windowValid = view3DAtts->GetWindowValid();
}

// ****************************************************************************
//  Method: avtView3D::SetToView3DAttributes
//
//  Purpose: 
//    Sets a View3DAttributes from the avtView3D.
//
//  Arguments:
//    view3DAtts : The View3DAttributes object to set.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 14:01:23 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:39:11 PDT 2003
//    I renamed this routine.
//   
//    Hank Childs, Wed Oct 15 13:05:33 PDT 2003
//    Added eye angle.
//
//    Eric Brugger, Mon Feb  9 16:02:13 PST 2004
//    Added centerOfRotationSet and centerOfRotation.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Support 3D axis scaling (3D equivalent of full-frame mode).
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Eric Brugger, Wed Jan  8 16:46:42 PST 2014
//    I added windowValid to support adding a multi resolution display
//    capability.
//
// ****************************************************************************

void
avtView3D::SetToView3DAttributes(View3DAttributes *view3DAtts) const
{
    view3DAtts->SetViewNormal(normal);
    view3DAtts->SetFocus(focus);
    view3DAtts->SetViewUp(viewUp);
    view3DAtts->SetViewAngle(viewAngle);
    view3DAtts->SetParallelScale(parallelScale);
    view3DAtts->SetNearPlane(nearPlane);
    view3DAtts->SetFarPlane(farPlane);
    view3DAtts->SetImagePan(imagePan);
    view3DAtts->SetImageZoom(imageZoom);
    view3DAtts->SetPerspective(perspective);
    view3DAtts->SetEyeAngle(eyeAngle);
    view3DAtts->SetCenterOfRotationSet(centerOfRotationSet);
    view3DAtts->SetCenterOfRotation(centerOfRotation);
    view3DAtts->SetAxis3DScaleFlag(axis3DScaleFlag);
    view3DAtts->SetAxis3DScales(axis3DScales);
    view3DAtts->SetShear(shear);
    view3DAtts->SetWindowValid(windowValid);
}

// ****************************************************************************
//  Method: avtView3D::GetCompositeProjectionTransformMatrix
//
//  Purpose: 
//    Returns the composite projection transform matrix given the aspect
//    ratio of the window width to height.
//
//  Arguments:
//    matrix     : The returned composite projection transform matrix.
//    aspect     : The aspect ratio of window width to height.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Jan 23 16:25:23 PST 2014
//
// ****************************************************************************

void
avtView3D::GetCompositeProjectionTransformMatrix(double *matrix, double aspect)
    const
{
    //
    // Get the inverse of the composite projection transform matrix.
    //
    avtViewInfo viewInfo;
    SetViewInfoFromView(viewInfo);
    vtkCamera *vtkcam = vtkCamera::New();
    viewInfo.SetCameraFromView(vtkcam);
    vtkMatrix4x4::DeepCopy(
        matrix,
        vtkcam->GetCompositeProjectionTransformMatrix(aspect, -1, +1));
}

// ****************************************************************************
//  Method: avtView3D::CalculateExtentsAndArea
//
//  Purpose: 
//    Calculate the 3d view extents and its area from the composite
//    projection transform matrix.
//
//  Arguments:
//    extents    : The returned extents.
//    area       : The returned area.
//    matrix     : The composite projection transform matrix.
//
//  Programmer: Eric Brugger
//  Creation:   Thu Jan 23 16:25:23 PST 2014
//
// ****************************************************************************

void
avtView3D::CalculateExtentsAndArea(double *extents, double &area,
    double *matrix)
{
    //
    // Invert the matrix.
    //
    double matrix2[16];
    vtkMatrix4x4::Invert(matrix, matrix2);

    //
    // Transform the corners of the back plane, front plane and view plane
    // in normalized device coordinates to world coordinates.
    //
    double c[12][4];
    c[0][0]  = -1.; c[0][1]  = -1.; c[0][2]  = -1.; c[0][3]  = 1.;
    c[1][0]  = +1.; c[1][1]  = -1.; c[1][2]  = -1.; c[1][3]  = 1.;
    c[2][0]  = +1.; c[2][1]  = +1.; c[2][2]  = -1.; c[2][3]  = 1.;
    c[3][0]  = -1.; c[3][1]  = +1.; c[3][2]  = -1.; c[3][3]  = 1.;
    c[4][0]  = -1.; c[4][1]  = -1.; c[4][2]  = +1.; c[4][3]  = 1.;
    c[5][0]  = +1.; c[5][1]  = -1.; c[5][2]  = +1.; c[5][3]  = 1.;
    c[6][0]  = +1.; c[6][1]  = +1.; c[6][2]  = +1.; c[6][3]  = 1.;
    c[7][0]  = -1.; c[7][1]  = +1.; c[7][2]  = +1.; c[7][3]  = 1.;
    c[8][0]  = -1.; c[8][1]  = -1.; c[8][2]  =  0.; c[8][3]  = 1.;
    c[9][0]  = +1.; c[9][1]  = -1.; c[9][2]  =  0.; c[9][3]  = 1.;
    c[10][0] = +1.; c[10][1] = +1.; c[10][2] =  0.; c[10][3] = 1.;
    c[11][0] = -1.; c[11][1] = +1.; c[11][2] =  0.; c[11][3] = 1.;
    for (int i = 0; i < 12; i++)
    {
        vtkMatrix4x4::MultiplyPoint(matrix2, c[i], c[i]);
        c[i][0] /= c[i][3]; c[i][1] /= c[i][3]; c[i][2] /= c[i][3];
    }

    //
    // Calculate the extents from the corners.
    //
    double xmin, xmax, ymin, ymax, zmin, zmax; 
    xmin = c[0][0]; xmax = c[0][0];
    ymin = c[0][1]; ymax = c[0][1];
    zmin = c[0][2]; zmax = c[0][2];
    for (int i = 1; i < 8; i++)
    {
        xmin = std::min(xmin, c[i][0]);
        xmax = std::max(xmax, c[i][0]);
        ymin = std::min(ymin, c[i][1]);
        ymax = std::max(ymax, c[i][1]);
        zmin = std::min(zmin, c[i][2]);
        zmax = std::max(zmax, c[i][2]);
    }
    
    extents[0] = xmin;
    extents[1] = xmax;
    extents[2] = ymin;
    extents[3] = ymax;
    extents[4] = zmin;
    extents[5] = zmax;

    //
    // Calculate the area from the corners of the view plane.
    //
    double v1[3], v2[3], v3[3];
    v1[0] = c[11][0] - c[8][0];
    v1[1] = c[11][1] - c[8][1];
    v1[2] = c[11][2] - c[8][2];
    v2[0] = c[9][0] - c[8][0];
    v2[1] = c[9][1] - c[8][1];
    v2[2] = c[9][2] - c[8][2];
    v3[0] = v1[1] * v2[2] - v1[2] * v2[1];
    v3[1] = v1[2] * v2[0] - v1[0] * v2[2];
    v3[2] = v1[0] * v2[1] - v1[1] * v2[0];
    area = std::sqrt(v3[0] * v3[0] + v3[1] * v3[1] + v3[2] * v3[2]);
}

#if 0
    //
    // Code that gets the plane equations of the 6 frustum planes.
    // I'm leaving it here in case I decide I later want to use
    // that instead of an AABB for the frustum.
    avtViewInfo viewInfo;
    SetViewInfoFromView(viewInfo);
    vtkCamera *vtkcam = vtkCamera::New();
    viewInfo.SetCameraFromView(vtkcam);
    double planes[24];
    vtkcam->GetFrustumPlanes(aspect, planes);
    vtkcam->Delete();
#endif
