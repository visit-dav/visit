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

#include <VisitInteractiveTool.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>

// ****************************************************************************
//  Method:  VisitInteractiveTool::ComputeDisplayToWorld
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
VisitInteractiveTool::ComputeDisplayToWorld(double x, double y, double z,
    double *worldPt)
{
    vtkRenderer *ren = proxy.GetCanvas();
    ren->SetDisplayPoint(x, y, z);
    ren->DisplayToWorld();
    ren->GetWorldPoint(worldPt);
    if(worldPt[3])
    {
        worldPt[0] /= worldPt[3];
        worldPt[1] /= worldPt[3];
        worldPt[2] /= worldPt[3];
        worldPt[3] = 1.0;
    }
}

// ****************************************************************************
//  Method:  VisitInteractiveTool::ComputeWorldToDisplay
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Brad Whitlock
//  Creation:    October  9, 2001
//
// ****************************************************************************
void
VisitInteractiveTool::ComputeWorldToDisplay(double x, double y, double z,
    double *displayPt)
{
    vtkRenderer *ren = proxy.GetCanvas();
    ren->SetWorldPoint(x, y, z, 1.0);
    ren->WorldToDisplay();
    ren->GetDisplayPoint(displayPt);
}

// ****************************************************************************
//  Method:  VisitInteractiveTool::ComputeDisplayToWorld
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
avtVector
VisitInteractiveTool::ComputeDisplayToWorld(const avtVector &v)
{
    double p[4];
    ComputeDisplayToWorld(v.x, v.y, v.z, p);
    return avtVector(p[0], p[1], p[2]);
}

// ****************************************************************************
//  Method:  VisitInteractiveTool::ComputeWorldToDisplay
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
avtVector
VisitInteractiveTool::ComputeWorldToDisplay(const avtVector &v)
{
    double p[4];
    ComputeWorldToDisplay(v.x, v.y, v.z, p);
    return avtVector(p[0], p[1], p[2]);
}

// ****************************************************************************
//  Method:  VisitInteractiveTool::DisplayToUnit
//
//  Purpose:
//    
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2001
//
// ****************************************************************************
avtVector
VisitInteractiveTool::DisplayToUnit(const avtVector &d)
{
    avtVector u;

    vtkRenderer *ren = proxy.GetCanvas();
    int *size = ren->GetSize();

    u.x = (2.0 * double(d.x)) / size[0]  -  1.0;
    u.y = (2.0 * double(d.y)) / size[1]  -  1.0;
    return u;
}

// ****************************************************************************
// Method: VisitInteractiveTool::FacingAxis
//
// Purpose: 
//   Returns the axis that is most facing the camera.
//
// Returns:    0,1 = X-axis, 2,3 = Y-axis, 4,5 = Z-axis
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 19 13:44:37 PST 2002
//
// Modifications:
//   
// ****************************************************************************

int
VisitInteractiveTool::FacingAxis()
{
    // Create a camera vector
    vtkCamera *camera = proxy.GetCanvas()->GetActiveCamera();
    const double *pos = camera->GetPosition();
    const double *focus = camera->GetFocalPoint();
    avtVector camvec(pos[0]-focus[0],pos[1]-focus[1],pos[2]-focus[2]);
    camvec.normalize();

    double dots[6];
    // X-axis
    dots[0] = camvec * avtVector(1., 0., 0.);
    dots[1] = camvec * avtVector(-1., 0., 0.);
    // Y-axis
    dots[2] = camvec * avtVector(0., 1., 0.);
    dots[3] = camvec * avtVector(0., -1., 0.);
    // Z-axis
    dots[4] = camvec * avtVector(0., 0., 1.);
    dots[5] = camvec * avtVector(0., 0., -1.);

    // Find the index of the largest dot product.
    int largestDotIndex = 0;
    for(int i = 1; i < 6; ++i)
    {
        if(dots[i] > dots[largestDotIndex])
            largestDotIndex = i;
    }

    return largestDotIndex;
}

// ****************************************************************************
// Method: VisitInteractiveTool::ComputeDepthTranslationDistance
//
// Purpose: 
//   Computes a good distance to depth translate.
//
// Returns:    A vector that is the increment that we can translate an object
//             so we can move it in and out along the current depth axis.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 30 09:34:38 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

avtVector
VisitInteractiveTool::ComputeDepthTranslationDistance()
{
    avtVector retval;
    int *size = proxy.GetCanvas()->GetSize();
    double bounds[6];
    proxy.GetBounds(bounds);
    double dX = bounds[1] - bounds[0];
    double dY = bounds[3] - bounds[2];
    double dZ = bounds[5] - bounds[4];

    switch(FacingAxis())
    {
    case 0:
        retval = avtVector(-1.,0.,0.) * (dX / double(size[1]));
        break;
    case 1:
        retval = avtVector(1.,0.,0.) * (dX / double(size[1]));
        break;
    case 2:
        retval = avtVector(0.,-1.,0.) * (dY / double(size[1]));
        break;
    case 3:
        retval = avtVector(0.,1.,0.) * (dY / double(size[1]));
        break;
    case 4:
        retval = avtVector(0.,0.,-1.) * (dZ / double(size[1]));
        break;
    case 5:
    default:
        retval = avtVector(0.,0.,1.) * (dZ / double(size[1]));
        break;
    }

    return retval;
}
