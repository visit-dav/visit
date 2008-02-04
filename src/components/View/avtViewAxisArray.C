/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                             avtViewAxisArray.C                                //
// ************************************************************************* //

#include <avtViewAxisArray.h>

#include <avtViewInfo.h>
#include <DebugStream.h>
#include <ViewAxisArrayAttributes.h>


// ****************************************************************************
//  Method: avtViewAxisArray constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

avtViewAxisArray::avtViewAxisArray()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtViewAxisArray operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

avtViewAxisArray &
avtViewAxisArray::operator=(const avtViewAxisArray &vi)
{
    viewport[0]  = vi.viewport[0];
    viewport[1]  = vi.viewport[1];
    viewport[2]  = vi.viewport[2];
    viewport[3]  = vi.viewport[3];
    domain[0]    = vi.domain[0];
    domain[1]    = vi.domain[1];
    range[0]     = vi.range[0];
    range[1]     = vi.range[1];

    return *this;
}


// ****************************************************************************
//  Method: avtViewAxisArray operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

bool
avtViewAxisArray::operator==(const avtViewAxisArray &vi)
{
    if (viewport[0] != vi.viewport[0] || viewport[1] != vi.viewport[1] ||
        viewport[2] != vi.viewport[2] || viewport[3] != vi.viewport[3])
    {
        return false;
    }

    if (domain[0] != vi.domain[0] || domain[1] != vi.domain[1])
    {
        return false;
    }

    if (range[0] != vi.range[0] || range[1] != vi.range[1])
    {
        return false;
    }

    return true;
}


// ****************************************************************************
//  Method: avtViewAxisArray::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Mon Feb  4 13:29:03 EST 2008
//    Set a more reasonable default for the viewport (and one that
//    matches the defaults in the attributes).
//
// ****************************************************************************

void
avtViewAxisArray::SetToDefault()
{
    viewport[0] = 0.15;
    viewport[1] = 0.9;
    viewport[2] = 0.10;
    viewport[3] = 0.85;
    domain[0]   = 0.;
    domain[1]   = 1.;
    range[0]    = 0.;
    range[1]    = 1.;
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3d view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the AxisArray view. 
//    size       The size of the window.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtViewAxisArray::SetViewInfoFromView(avtViewInfo &viewInfo, int *size)
{
    CheckAndCorrectDomainRange();

    //
    // Calculate a new range so that we get a 1 to 1 aspect ration.
    //
    double    viewScale;
    double    realRange[2];

    viewScale = ((domain[1] - domain[0]) / (range[1] -  range[0])) *
                ((viewport[3] - viewport[2]) / (viewport[1] - viewport[0])) *
                ((double) size[1] / (double) size[0]) ;

    realRange[0] = range[0] * viewScale;
    realRange[1] = range[1] * viewScale;

    //
    // Reset the view up vector, the focal point and the camera position.
    // The width is set based on the y window dimension.
    //
    double    width;

    width = realRange[1] - realRange[0];

    viewInfo.viewUp[0] = 0.;
    viewInfo.viewUp[1] = 1.;
    viewInfo.viewUp[2] = 0.;

    viewInfo.focus[0] = (domain[1] + domain[0]) / 2.;
    viewInfo.focus[1] = (realRange[1]  + realRange[0]) / 2.;
    viewInfo.focus[2] = 0.;

    viewInfo.camera[0] = viewInfo.focus[0];
    viewInfo.camera[1] = viewInfo.focus[1];
    viewInfo.camera[2] = 1.;

    //
    // Set the projection mode, parallel scale and view angle.  The
    // projection mode is always parallel for AxisArray.  The parallel scale is
    // off.
    //
    viewInfo.orthographic = true;
    viewInfo.setScale = true;
    viewInfo.parallelScale = width / 2.;
    viewInfo.viewAngle = 30.;

    //
    // Set the near and far clipping planes.  They are set independent of
    // the coordinate extents, since it doesn't matter.  Setting the values
    // too tight around the focus causes problems.
    //
    viewInfo.nearPlane = 0.5;
    viewInfo.farPlane  = 1.5;
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetViewport
//
//  Purpose: Sets the window's viewport 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::SetViewport(double *newViewport)
{
    viewport[0] = newViewport[0];
    viewport[1] = newViewport[1];
    viewport[2] = newViewport[2];
    viewport[3] = newViewport[3];
}

// ****************************************************************************
//  Method: avtViewAxisArray::GetViewport
//
//  Purpose:
//    Gets the window's viewport 
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::GetViewport(double *winViewport) const
{
    winViewport[0] = viewport[0];
    winViewport[1] = viewport[1];
    winViewport[2] = viewport[2];
    winViewport[3] = viewport[3];
}

// ****************************************************************************
//  Method: avtViewAxisArray::GetScaleFactor
//
//  Purpose:
//    Gets the window's scale factor.
//
//  Arguments:
//    size      The size of the renderable area.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

double
avtViewAxisArray::GetScaleFactor(int *size)
{
    double s;

    CheckAndCorrectDomainRange();

    s = ((domain[1] - domain[0]) / (range[1]  - range[0])) *
        ((viewport[3] - viewport[2]) / (viewport[1] - viewport[0])) *
        ((double) size[1] / (double) size[0]);

    return s;
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetFromViewAxisArrayAttributes
//
//  Purpose: 
//    Sets the avtAxisArrayView from the ViewAxisArrayAttributes object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewAxisArrayAttributes object to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//   
// ****************************************************************************

void
avtViewAxisArray::SetFromViewAxisArrayAttributes(const ViewAxisArrayAttributes *viewAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        viewport[i] = viewAtts->GetViewportCoords()[i];
    }
    domain[0] = viewAtts->GetDomainCoords()[0];
    domain[1] = viewAtts->GetDomainCoords()[1];
    range[0]  = viewAtts->GetRangeCoords()[0];
    range[1]  = viewAtts->GetRangeCoords()[1];
}

// ****************************************************************************
//  Method: avtViewAxisArray::SetToViewAxisArrayAttributes
//
//  Purpose: 
//    Sets the ViewAxisArrayAttributes from the avtViewAxisArray object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewAxisArrayAttributes object to use.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//
// ****************************************************************************

void
avtViewAxisArray::SetToViewAxisArrayAttributes(ViewAxisArrayAttributes *viewAtts) const
{
    viewAtts->SetViewportCoords(viewport);
    viewAtts->SetDomainCoords(domain);
    viewAtts->SetRangeCoords(range);
}

// ****************************************************************************
//  Method: avtViewAxisArray::CheckAndCorrectDomainRange
//
//  Purpose:
//    Checks the window parameters and corrects them if they are invalid.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
// ****************************************************************************

void
avtViewAxisArray::CheckAndCorrectDomainRange()
{
    //
    // Account for degenerate views.
    //
    double width  = domain[1] - domain[0];
    double height = range[1]  - range[0];
    if (width <= 0. && height <= 0.)
    {
        if (domain[0] == 0. && range[0] == 0.)
        {
            domain[0] = -1.;
            domain[1] =  1.;
            range[0]  = -1.;
            range[1]  =  1.;
        }
        else if (domain[0] == 0.)
        {
            domain[0] -= range[0];
            domain[1] += range[1];
            range[0]  -= range[0];
            range[1]  += range[1];
        }
        else if (range[0] == 0.)
        {
            range[0]  -= domain[0];
            range[1]  += domain[1];
            domain[0] -= domain[0];
            domain[1] += domain[1];
        }
        else
        {
            domain[0] -= domain[0];
            domain[1] += domain[1];
            range[0]  -= range[0];
            range[1]  += range[1];
        }
    }
    else if (width <= 0.)
    {
        domain[0] -= height / 2.;
        domain[1] += height / 2.;
    }
    else if (height <= 0.)
    {
        range[0] -= width / 2.;
        range[1] += width / 2.;
    }
}

