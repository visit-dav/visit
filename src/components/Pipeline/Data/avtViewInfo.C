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
    parallelScale= vi.parallelScale;
    setScale     = vi.setScale;
    orthographic = vi.orthographic;
    nearPlane    = vi.nearPlane;
    farPlane     = vi.farPlane;

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
    setScale     = false;
    parallelScale= 1.;
    nearPlane    = 0.001;
    farPlane     = 100;
    orthographic = true;
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
    parallelScale = vtkcam->GetParallelScale();
    orthographic = (vtkcam->GetParallelProjection() != 0 ? true : false);
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
// ****************************************************************************

void
avtViewInfo::SetCameraFromView(vtkCamera *vtkcam) const
{
    vtkcam->SetViewAngle(viewAngle);
    if (setScale)
    {
        vtkcam->SetParallelScale(parallelScale);
    }
    vtkcam->SetParallelProjection(orthographic ? 1 : 0);
    vtkcam->SetClippingRange(nearPlane, farPlane);
    vtkcam->SetFocalPoint(focus);
    vtkcam->SetPosition(camera);
    vtkcam->SetViewUp(viewUp);
}


