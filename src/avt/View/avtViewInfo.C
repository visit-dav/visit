/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
// ****************************************************************************

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
    vtkcam->SetFocalPoint(focus);
    vtkcam->SetPosition(camera);
    vtkcam->SetViewUp(viewUp);
    vtkcam->SetWindowCenter(2.0*imagePan[0], 2.0*imagePan[1]);
    vtkcam->SetFocalDisk(imageZoom);
}


