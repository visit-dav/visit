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
//                                VisWinTriad.C                              //
// ************************************************************************* //

#include <vtkTriad2D.h>
#include <vtkProperty2D.h>
#include <vtkRenderer.h>

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>
#include <VisWinTriad.h>


// ****************************************************************************
//  Method: VisWinTriad constructor
//
//  Arguments:
//      p      A proxy that allows more access to the VisWindow for this 
//             colleague.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:01:24 PDT 2000
//    Initialized data member addedTriad.
//
//    Hank Childs, Mon Jul 10 16:58:06 PDT 2000
//    Removed axis actors in favor of vtkTriad2D class.
//
//    Kathleen Bonnell, Mon Nov 26 9:16:32 PST 2001
//    Make the triad un-pickable.
//
// ****************************************************************************

VisWinTriad::VisWinTriad(VisWindowColleagueProxy &p) : VisWinColleague(p)
{
    triad = vtkTriad2D::New();
    triad->PickableOff();
    addedTriad = false;
}


// ****************************************************************************
//  Method: VisWinTriad destructor
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 10 16:58:06 PDT 2000
//    Removed deletion of former data members for axes; added deletion of 
//    triad.
//
// ****************************************************************************

VisWinTriad::~VisWinTriad()
{
    if (triad != NULL)
    {
        triad->Delete();
        triad = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinTriad::SetForegroundColor
//
//  Purpose:
//      Sets the color of the triad.
//
//  Arguments:
//      fr        The red component of the foreground color.
//      fg        The green component of the foreground color.
//      fb        The blue component of the foreground color.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 10 16:58:06 PDT 2000
//    Changed colors of triad's axes instead of former data members.
//
// ****************************************************************************

void
VisWinTriad::SetForegroundColor(double fr, double fg, double fb)
{
    triad->GetXAxis()->GetProperty()->SetColor(fr, fg, fb);
    triad->GetYAxis()->GetProperty()->SetColor(fr, fg, fb);
    triad->GetZAxis()->GetProperty()->SetColor(fr, fg, fb);
}


// ****************************************************************************
//  Method: VisWinTriad::Start3DMode
//
//  Purpose:
//      We are about to enter 3D mode, so the triad should be added to the
//      background, as long as there are plots.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:01:24 PDT 2000
//    Pushed logic for adding the triad into its own routine.
//
// ****************************************************************************

void
VisWinTriad::Start3DMode(void)
{
    if (ShouldAddTriad())
    {
        AddTriadToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinTriad::Stop3DMode
//
//  Purpose:
//      We are about to leave 3D mode, so the triad should be removed from the
//      background.
//
//  Programmer: Hank Childs
//  Creation:   June 8, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Jul  6 11:01:24 PDT 2000
//    Pushed logic for removing the triad into its own routine.
//
// ****************************************************************************

void
VisWinTriad::Stop3DMode(void)
{
    RemoveTriadFromWindow();
}


// ****************************************************************************
//  Method: VisWinTriad::HasPlots
//
//  Purpose:
//      This routine is how the vis window tells this colleague that it now
//      has plots.  We can now add the triad.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinTriad::HasPlots(void)
{
    if (ShouldAddTriad())
    {
        AddTriadToWindow();
    }
}


// ****************************************************************************
//  Method: VisWinTriad::NoPlots
//
//  Purpose:
//      This routine is how the vis window tells this colleague that it no
//      longer has plots.  Remove the triad.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

void
VisWinTriad::NoPlots(void)
{
    RemoveTriadFromWindow();
}


// ****************************************************************************
//  Method: VisWinTriad::ShouldAddTriad
//
//  Purpose:
//      There are two constraints on whether the triad should be added to the
//      VisWindow - whether we are in 3D mode and whether we have plots.  This
//      buffers the logic for that so the individual operations don't need to
//      know about each other.
//
//  Returns:    true if the triad should be added to the window, false 
//              otherwise.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
// ****************************************************************************

bool
VisWinTriad::ShouldAddTriad()
{
    return (mediator.GetMode() == WINMODE_3D && mediator.HasPlots());
}


// ****************************************************************************
//  Method: VisWinTriad::AddTriadToWindow
//
//  Purpose:
//      Adds the triad to the window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 10 16:58:06 PDT 2000
//    Added triad to the background instead of former axes data members.
//    Registered the active camera with the triad.
//
//    Hank Childs, Fri Aug  4 14:44:32 PDT 2000
//    Put triad on foreground instead of background.
//
// ****************************************************************************

void
VisWinTriad::AddTriadToWindow(void)
{
    if (addedTriad)
    {
        return;
    }

    //
    // Get the camera of the canvas and register it with the triad.
    //
    vtkRenderer *canvas = mediator.GetCanvas();
    triad->SetCamera(canvas->GetActiveCamera());

    //
    // Add the triad to the background (note that we are using a different
    // renderer's camera -- the canvas').
    //
    vtkRenderer *foreground = mediator.GetForeground();
    foreground->AddActor2D(triad);

    addedTriad = true;
}


// ****************************************************************************
//  Method: VisWinTriad::RemoveTriadFromWindow
//
//  Purpose:
//      Removes the triad from the window.
//
//  Programmer: Hank Childs
//  Creation:   July 6, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Jul 10 16:58:06 PDT 2000
//    Removed the triad from the background instead of former axes data
//    members.
//
//    Hank Childs, Fri Aug  4 14:44:32 PDT 2000
//    Put triad on foreground instead of background.
//
// ****************************************************************************

void
VisWinTriad::RemoveTriadFromWindow(void)
{
    if (! addedTriad)
    {
        return;
    }

    vtkRenderer *foreground = mediator.GetForeground();
    foreground->RemoveActor2D(triad);

    addedTriad = false;
}


// ****************************************************************************
//  Method: VisWinTriad::SetVisibility
//
//  Purpose:
//      Sets the visibility of the triad.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 27, 2001 
//
// ****************************************************************************

void
VisWinTriad::SetVisibility(bool vis)
{
    triad->SetVisibility((int)vis);
}


