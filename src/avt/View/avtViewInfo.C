// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                               avtViewInfo.C                               //
// ************************************************************************* //

#include <avtViewInfo.h>

#include <vtkCamera.h>


// ****************************************************************************
//  Method: avtViewInfo constructor
//
//  Programmer: Hank Childs
//  Creation:   June 18, 2001
//
// ****************************************************************************

avtViewInfo::avtViewInfo()
{
    SetToDefault();
}


// ****************************************************************************
//  Method: avtViewInfo operator =
//
//  Arguments:
//    vi      The view info to copy.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:30:49 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//    Added eye angle.
//
//    Jeremy Meredith, Mon Aug  2 13:47:52 EDT 2010
//    Added shear.
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Add useOSPRay.
//
// ****************************************************************************

avtViewInfo &
avtViewInfo::operator=(const avtViewInfo &vi)
{
    camera[0]    = vi.camera[0];
    camera[1]    = vi.camera[1];
    camera[2]    = vi.camera[2];
    focus[0]     = vi.focus[0];
    focus[1]     = vi.focus[1];
    focus[2]     = vi.focus[2];
    viewUp[0]    = vi.viewUp[0];
    viewUp[1]    = vi.viewUp[1];
    viewUp[2]    = vi.viewUp[2];
    viewAngle    = vi.viewAngle;
    eyeAngle     = vi.eyeAngle;
    parallelScale= vi.parallelScale;
    setScale     = vi.setScale;
    orthographic = vi.orthographic;
    nearPlane    = vi.nearPlane;
    farPlane     = vi.farPlane;
    imagePan[0]  = vi.imagePan[0];
    imagePan[1]  = vi.imagePan[1];
    imageZoom    = vi.imageZoom;
    shear[0]     = vi.shear[0];
    shear[1]     = vi.shear[1];
    shear[2]     = vi.shear[2];
    useOSPRay    = vi.useOSPRay;
    useAnari     = vi.useAnari;

    return *this;
}


// ****************************************************************************
//  Method: avtViewInfo operator ==
//
//  Arguments:
//    vi      The view info to compare to.
//
//  Programmer: Hank Childs
//  Creation:   November 27, 2000
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:30:49 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//    Added eye angle.
//
//    Jeremy Meredith, Mon Aug  2 13:47:52 EDT 2010
//    Added shear.
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Add useOSPRay.
//
// ****************************************************************************

bool
avtViewInfo::operator==(const avtViewInfo &vi)
{
    if (camera[0] != vi.camera[0] || camera[1] != vi.camera[1]
        || camera[2] != vi.camera[2])
    {
        return false;
    }

    if (focus[0] != vi.focus[0] || focus[1] != vi.focus[1]
        || focus[2] != vi.focus[2])
    {
        return false;
    }

    if (viewUp[0] != vi.viewUp[0] || viewUp[1] != vi.viewUp[1]
        || viewUp[2] != vi.viewUp[2])
    {
        return false;
    }

    if (viewAngle != vi.viewAngle)
    {
        return false;
    }

    if (eyeAngle != vi.eyeAngle)
    {
        return false;
    }

    if (setScale != vi.setScale || parallelScale != vi.parallelScale)
    {
        return false;
    }

    if (orthographic != vi.orthographic)
    {
        return false;
    }

    if (nearPlane != vi.nearPlane || farPlane != vi.farPlane)
    {
        return false;
    }

    if (imagePan[0] != vi.imagePan[0] || imagePan[1] != vi.imagePan[1] ||
        imageZoom != vi.imageZoom)
    {
        return false;
    }

    if (shear[0] != vi.shear[0] || shear[1] != vi.shear[1]
        || shear[2] != vi.shear[2])
    {
        return false;
    }

    if (useOSPRay != vi.useOSPRay)
    {
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtViewInfo::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 08, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:30:49 PDT 2003
//    I added image pan and image zoom.
//
//    Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//    Added eye angle.
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Add useOSPRay.
//
// ****************************************************************************

void
avtViewInfo::SetToDefault()
{
    camera[0]    =  0.;
    camera[1]    =  0.;
    camera[2]    = -1.;
    focus[0]     =  0.;
    focus[1]     =  0.;
    focus[2]     =  0.;
    viewUp[0]    =  0.;
    viewUp[1]    =  1.;
    viewUp[2]    =  0.;
    viewAngle    = 30.;
    eyeAngle     = 2.;
    setScale     = false;
    parallelScale= 1.;
    nearPlane    = 0.001;
    farPlane     = 100;
    orthographic = true;
    imagePan[0]  = 0.;
    imagePan[1]  = 0.;
    imageZoom    = 1.;
    shear[0]     =  0.;
    shear[1]     =  0.;
    shear[2]     =  1.;
    useOSPRay    = false;
}

// ****************************************************************************
//  Method: avtViewInfo::SetViewFromCamera
//
//  Arguments:
//    vtkcam      The camera from which to set the info.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 04, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//    Added eye angle.
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
// ****************************************************************************

void
avtViewInfo::SetViewFromCamera(vtkCamera *vtkcam)
{
    vtkcam->GetPosition(camera);
    vtkcam->GetFocalPoint(focus);
    vtkcam->GetViewUp(viewUp);

    double clipRange[2];
    vtkcam->GetClippingRange(clipRange);
    nearPlane = clipRange[0];
    farPlane  = clipRange[1];

    viewAngle = vtkcam->GetViewAngle();
    eyeAngle  = vtkcam->GetEyeAngle();
    parallelScale = vtkcam->GetParallelScale();
    orthographic = (vtkcam->GetParallelProjection() != 0 ? true : false);
    vtkcam->GetViewShear(shear);
}


// ****************************************************************************
//  Method: avtViewInfo::SetCameraFromView
//
//  Arguments:
//    vtkcam      The camera in which to store the view info.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    January 08, 2001
//
//  Modifications:
//    Eric Brugger, Fri Jun  6 15:30:49 PDT 2003
//    I added image pan and image zoom.
//
//    Eric Brugger, Wed Jun 18 17:46:36 PDT 2003
//    I modified the call to SetWindowCenter since the meaning of its
//    arguments changed.
//
//    Hank Childs, Wed Oct 15 13:09:03 PDT 2003
//    Added eye angle.
//
//    Jeremy Meredith, Mon Aug  2 14:23:08 EDT 2010
//    Add shear for oblique projection support.
//
//    Kathleen Bonnell, Wed Jun  8 14:17:36 PDT 2011
//    Set user transform matrix when zooming (used to be done by hack inside
//    vtkCamera).
//
//    Kathleen Biagas, Wed Aug 17, 2022
//    Test useOSPRay to determine if ospray-path should be used.
//    It will be true only if HAVE_OSPRAY is true and ospray rendering is
//    currently being used.
//
// ****************************************************************************
#include<vtkMatrix4x4.h>
#include<vtkTransform.h>

void
avtViewInfo::SetCameraFromView(vtkCamera *vtkcam) const
{
    vtkcam->SetViewAngle(viewAngle);
    vtkcam->SetEyeAngle(eyeAngle);
    if (setScale)
    {
        vtkcam->SetParallelScale(parallelScale);
    }
    vtkcam->SetParallelProjection(orthographic ? 1 : 0);
    vtkcam->SetClippingRange(nearPlane, farPlane);
    vtkcam->SetViewShear(shear[0],shear[1],shear[2]);
    vtkcam->SetFocalPoint(focus);
    vtkcam->SetPosition(camera);
    vtkcam->SetViewUp(viewUp);
    vtkcam->SetWindowCenter(2.0*imagePan[0], 2.0*imagePan[1]);
    vtkcam->SetFocalDisk(imageZoom);

    if (useOSPRay || useAnari)
    {
        // Currently the SetWindowCenter and SetUserTransform do not get
        // used in the vtkOSPRayCameraNode so instead use the Zoom here and
        // in the Navigate3D.C and Zoom3D.C pan the camera rather than the
        // image.
        vtkcam->Zoom(imageZoom);
    }
    else
    {
        if (imageZoom != 1.0)
        {
            double matrix[4][4];
            vtkMatrix4x4::Identity(*matrix);

            matrix[0][0] = imageZoom;
            matrix[1][1] = imageZoom;
            vtkTransform *trans = vtkTransform::New();
            trans->SetMatrix(*matrix);
            vtkcam->SetUserTransform(trans);
            trans->Delete();
        }
        else
        {
            vtkcam->SetUserTransform(NULL);
        }
   }
}
