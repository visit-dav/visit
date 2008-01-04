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
//                             avtViewCurve.C                                //
// ************************************************************************* //

#include <avtViewCurve.h>

#include <avtViewInfo.h>
#include <DebugStream.h>
#include <ViewCurveAttributes.h>


// ****************************************************************************
//  Method: avtViewCurve constructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
// ****************************************************************************

avtViewCurve::avtViewCurve()
{
    SetToDefault();
}

// ****************************************************************************
//  Method: avtViewCurve operator =
//
//  Arguments:
//    vi        The view info to copy.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added havePerformedLogDomain/Range.
//
// ****************************************************************************

avtViewCurve &
avtViewCurve::operator=(const avtViewCurve &vi)
{
    viewport[0]  = vi.viewport[0];
    viewport[1]  = vi.viewport[1];
    viewport[2]  = vi.viewport[2];
    viewport[3]  = vi.viewport[3];
    domain[0]    = vi.domain[0];
    domain[1]    = vi.domain[1];
    range[0]     = vi.range[0];
    range[1]     = vi.range[1];

    domainScale  = vi.domainScale;
    rangeScale   = vi.rangeScale;
    havePerformedLogDomain = vi.havePerformedLogDomain;
    havePerformedLogRange  = vi.havePerformedLogRange;

    return *this;
}


// ****************************************************************************
//  Method: avtViewCurve operator ==
//
//  Arguments:
//    vi        The view info to compare to.
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added havePerformedLogDomain/Range.
//
// ****************************************************************************

bool
avtViewCurve::operator==(const avtViewCurve &vi)
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

    if (domainScale != vi.domainScale)
    {
        return false;
    }
    if (rangeScale != vi.rangeScale)
    {
        return false;
    }

    if (havePerformedLogDomain != vi.havePerformedLogDomain ||
        havePerformedLogRange  != vi.havePerformedLogRange)
    {
        return false;
    }
    return true;
}


// ****************************************************************************
//  Method: avtViewCurve::SetToDefault
//
//  Purpose:
//    Sets up with default values.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I removed yScale and replaced window with domain and range.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
//    Kathleen Bonnell, Fri May 11 09:20:06 PDT 2007 
//    Added havePerformedLogDomain/Range.
//
// ****************************************************************************

void
avtViewCurve::SetToDefault()
{
    viewport[0] = 0.;
    viewport[1] = 1.;
    viewport[2] = 0.;
    viewport[3] = 1.;
    domain[0]   = 0.;
    domain[1]   = 1.;
    range[0]    = 0.;
    range[1]    = 1.;
    domainScale = LINEAR;
    rangeScale = LINEAR;
    havePerformedLogDomain = false;
    havePerformedLogRange  = false;
}

// ****************************************************************************
//  Method: avtViewCurve::SetViewInfoFromView
//
//  Purpose:
//    Set the avtViewInfo, which is used to set the view within avt and
//    ultimately vtk, based on the 3d view.
//
//  Arguments:
//    viewInfo   The avtViewInfo in which to store the Curve view. 
//    size       The size of the window.
//
//  Notes:
//    Taken from avtView2D.
//
//  Programmer:  Kathleen Bonnell
//  Creation:    April 30, 2002
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I replaced window with domain and range.  I added a window size argument
//    so that the routine could handle non-square windows and viewports.
//
//    Eric Brugger, Wed Oct  8 16:28:41 PDT 2003
//    I Modified the routine to set the z camera position and near and far
//    clipping plane positions independent of the coordinate extents.
//
//    Eric Brugger, Fri Oct 10 12:45:11 PDT 2003
//    Add code to handle degenerate windows.
//
//    Eric Brugger, Tue Nov 18 09:23:44 PST 2003
//    I replaced GetValidDomainRange with CheckAndCorrectDomainRange.
//
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
// ****************************************************************************

void
avtViewCurve::SetViewInfoFromView(avtViewInfo &viewInfo, int *size)
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
    // projection mode is always parallel for Curve.  The parallel scale is
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
//  Method: avtViewCurve::SetViewport
//
//  Purpose: Sets the window's viewport 
//
//  Programmer: Mark Blair
//  Creation:   Mon Jul 16 17:16:29 PDT 2007
//
// ****************************************************************************

void
avtViewCurve::SetViewport(double *newViewport)
{
    viewport[0] = newViewport[0];
    viewport[1] = newViewport[1];
    viewport[2] = newViewport[2];
    viewport[3] = newViewport[3];
}

// ****************************************************************************
//  Method: avtViewCurve::GetViewport
//
//  Purpose:
//    Gets the window's viewport 
//
//  Programmer: Mark C. Miller 
//  Creation:   November 4, 2003
//
// ****************************************************************************

void
avtViewCurve::GetViewport(double *winViewport) const
{
    winViewport[0] = viewport[0];
    winViewport[1] = viewport[1];
    winViewport[2] = viewport[2];
    winViewport[3] = viewport[3];
}

// ****************************************************************************
//  Method: avtViewCurve::GetScaleFactor
//
//  Purpose:
//    Gets the window's scale factor.
//
//  Arguments:
//    size      The size of the renderable area.
//
//  Programmer: Eric Brugger
//  Creation:   October 10, 2003
//
//  Modifications:
//    Eric Brugger, Tue Nov 18 09:23:44 PST 2003
//    I replaced GetValidDomainRange with CheckAndCorrectDomainRange.
//
// ****************************************************************************

double
avtViewCurve::GetScaleFactor(int *size)
{
    double s;

    CheckAndCorrectDomainRange();

    s = ((domain[1] - domain[0]) / (range[1]  - range[0])) *
        ((viewport[3] - viewport[2]) / (viewport[1] - viewport[0])) *
        ((double) size[1] / (double) size[0]);

    return s;
}

// ****************************************************************************
//  Method: avtViewCurve::SetFromViewCurveAttributes
//
//  Purpose: 
//    Sets the avtCurveView from the ViewCurveAttributes object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewCurveAttributes object to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 13:00:39 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I renamed this method.  I replaced window with domain and range.
//
//    Kathleen Bonnell, Tue Apr  3 14:28:18 PDT 2007 
//    Added domainScale and rangeScale. 
//   
// ****************************************************************************

void
avtViewCurve::SetFromViewCurveAttributes(const ViewCurveAttributes *viewAtts)
{
    for(int i = 0; i < 4; ++i)
    {
        viewport[i] = viewAtts->GetViewportCoords()[i];
    }
    domain[0] = viewAtts->GetDomainCoords()[0];
    domain[1] = viewAtts->GetDomainCoords()[1];
    range[0]  = viewAtts->GetRangeCoords()[0];
    range[1]  = viewAtts->GetRangeCoords()[1];
    domainScale = (ScaleMode)viewAtts->GetDomainScale();
    rangeScale  = (ScaleMode)viewAtts->GetRangeScale();
}

// ****************************************************************************
//  Method: avtViewCurve::SetToViewCurveAttributes
//
//  Purpose: 
//    Sets the ViewCurveAttributes from the avtViewCurve object.
//
//  Arguments:
//    viewAtts : A pointer to the ViewCurveAttributes object to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Jul 1 13:00:39 PST 2003
//
//  Modifications:
//    Eric Brugger, Wed Aug 20 09:42:38 PDT 2003
//    I renamed this method.  I replaced window with domain and range.
//   
//    Kathleen Bonnell, Thu Mar 22 20:12:04 PDT 2007 
//    Added domainScale, rangeScale. 
//
// ****************************************************************************

void
avtViewCurve::SetToViewCurveAttributes(ViewCurveAttributes *viewAtts) const
{
    viewAtts->SetViewportCoords(viewport);
    viewAtts->SetDomainCoords(domain);
    viewAtts->SetRangeCoords(range);
    viewAtts->SetDomainScale(domainScale);
    viewAtts->SetRangeScale(rangeScale);
}

// ****************************************************************************
//  Method: avtViewCurve::CheckAndCorrectDomainRange
//
//  Purpose:
//    Checks the window parameters and corrects them if they are invalid.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2003
//
// ****************************************************************************

void
avtViewCurve::CheckAndCorrectDomainRange()
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

