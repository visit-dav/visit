/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                         VisWinInteractions.C                              //
// ************************************************************************* //

#include <VisWindow.h>
#include <VisWindowColleagueProxy.h>
#include <VisWinInteractions.h>

#include <BadInteractorException.h>
#include <VisitHotPointInteractor.h>


// ****************************************************************************
//  Method: VisWinInteractions constructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Brad Whitlock, Mon Oct 1 10:00:41 PDT 2001
//    Modified the code so it only has a hotpoint interactor.
//
//    Hank Childs, Wed May 29 09:06:05 PDT 2002
//    Initialized spinMode.
//
//    Brad Whitlock, Wed Sep 10 15:28:12 PST 2003
//    Initialized spinModeSuspended.
//
// ****************************************************************************

VisWinInteractions::VisWinInteractions(VisWindowColleagueProxy &c,
                                       VisWindowInteractorProxy &i) 
    : VisWinColleague(c)
{
    mode = NAVIGATE;
    bboxMode = true;
    spinMode = false;
    spinModeSuspended = false;
    hotPointInteractor = new VisitHotPointInteractor(i);
    mediator.SetInteractor(hotPointInteractor);
}


// ****************************************************************************
//  Method: VisWinInteractions destructor
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Wed Aug 15 15:05:18 PDT 2001
//    Use VTK's memory management system rather than blindly deleting 
//    interactor styles.
//
//    Brad Whitlock, Mon Oct 1 09:16:19 PDT 2001
//    Changed code since there is now only one interactor style to delete.
//
// ****************************************************************************

VisWinInteractions::~VisWinInteractions()
{
    if (hotPointInteractor != NULL)
    {
        hotPointInteractor->Delete();
        hotPointInteractor = NULL;
    }
}


// ****************************************************************************
//  Method: VisWinInteractions::SetInteractionMode
//
//  Purpose:
//      Sets the interaction mode for the vis window.
//
//  Arguments:
//      m       The new interaction mode.
//
//  Programmer: Hank Childs
//  Creation:   May 30, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Sep 18 10:41:30 PDT 2001
//    Added cases to switch statement to fix compiler warning.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002  
//    Added support for curve mode. 
//
// ****************************************************************************

void
VisWinInteractions::SetInteractionMode(INTERACTION_MODE m)
{
    switch (mediator.GetMode())
    {
      case WINMODE_2D:
        Stop2DMode();
        break;
      case WINMODE_3D:
        Stop3DMode();
        break;
      case WINMODE_CURVE:
        StopCurveMode();
        break;
      case WINMODE_NONE:
      default:
        break;
    }

    mode = m;

    switch (mediator.GetMode())
    {
      case WINMODE_2D:
        Start2DMode();
        break;
      case WINMODE_3D:
        Start3DMode();
        break;
      case WINMODE_CURVE:
        StartCurveMode();
        break;
      case WINMODE_NONE:
      default:
        break;
    }
}


// ****************************************************************************
// Method: VisWinInteractions::GetInteractionMode
//
// Purpose: 
//   Returns the current interaction mode.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 7 16:12:21 PST 2000
//
// Modifications:
//   
// ****************************************************************************

INTERACTION_MODE
VisWinInteractions::GetInteractionMode() const
{
    return mode;
}


// ****************************************************************************
//  Method: VisWinInteractions::Start2DMode
//
//  Purpose:
//      Sets the appropriate interactor for 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Wed Aug 15 15:59:43 PDT 2001
//    Don't start up an interaction mode if there are no plots.
//
//    Brad Whitlock, Mon Oct 1 09:20:46 PDT 2001
//    I moved all of the behavior into the hot point interactor.
//
// ****************************************************************************

void
VisWinInteractions::Start2DMode(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->Start2DMode(mode);
    }
}

// ****************************************************************************
//  Method: VisWinInteractions::Start3DMode
//
//  Purpose:
//      Sets the appropriate interactor for 3D mode.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Hank Childs, Wed Aug 15 15:59:43 PDT 2001
//    Don't start up an interaction mode if there are no plots.
//
//    Brad Whitlock, Mon Oct 1 09:25:34 PDT 2001
//    I moved all of the behavior into the hot point interactor.
//
// ****************************************************************************

void
VisWinInteractions::Start3DMode(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->Start3DMode(mode);
    }
}

// ****************************************************************************
//  Method: VisWinInteractions::StartCurveMode
//
//  Purpose:
//      Sets the appropriate interactor for Curve mode.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002
//
// ****************************************************************************

void
VisWinInteractions::StartCurveMode(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->StartCurveMode(mode);
    }
}

// ****************************************************************************
//  Method: VisWinInteractions::Stop2DMode
//
//  Purpose:
//      Stops the 2D interactions.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Oct 1 09:25:34 PDT 2001
//    I moved all of the behavior into the hot point interactor.
//
// ****************************************************************************

void
VisWinInteractions::Stop2DMode()
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->Stop2DMode();
    }
}


// ****************************************************************************
//  Method: VisWinInteractions::Stop3DMode
//
//  Purpose:
//      Stops the 3D interactions.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Oct 1 09:25:34 PDT 2001
//    I moved all of the behavior into the hot point interactor.
//
//    Kathleen Bonnell, Thu Nov 21 09:04:22 PST 2002 
//    Fixed type ( ->Stop2DMode).
//
// ****************************************************************************

void
VisWinInteractions::Stop3DMode(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->Stop3DMode();
    }
}


// ****************************************************************************
//  Method: VisWinInteractions::StopCurveMode
//
//  Purpose:
//      Stops the Curve interactions.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 8, 2002
//
// ****************************************************************************

void
VisWinInteractions::StopCurveMode(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->StopCurveMode();
    }
}


// ****************************************************************************
//  Method: VisWinInteractions::SetBoundingBoxMode
//
//  Purpose:
//      Sets the bounding box mode.
//
//  Returns:
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Nov 9 15:52:59 PST 2000
//
// ****************************************************************************

void
VisWinInteractions::SetBoundingBoxMode(bool val)
{
    bboxMode = val;
}


// ****************************************************************************
//  Method: VisWinInteractions::GetBoundingBoxMode
//
//  Purpose:
//      Determines whether the bounding box mode is enabled or disabled.
//
//  Returns:    true if bounding box mode is enabled, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   November 8, 2000
//
// ****************************************************************************

bool
VisWinInteractions::GetBoundingBoxMode() const
{
    return bboxMode;
}


// ****************************************************************************
//  Method: VisWinInteractions::SetSpinMode
//
//  Purpose:
//      Sets the spin mode.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

void
VisWinInteractions::SetSpinMode(bool val)
{
    spinMode = val;
}


// ****************************************************************************
//  Method: VisWinInteractions::GetSpinMode
//
//  Purpose:
//      Determines whether the spin mode is enabled or disabled.
//
//  Returns:    true if spin mode is enabled, false otherwise.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2002
//
// ****************************************************************************

bool
VisWinInteractions::GetSpinMode() const
{
    return spinMode;
}

// ****************************************************************************
//  Method: VisWinInteractions::SetSpinModeSuspended
//
//  Purpose:
//      Sets whether spin mode is temporarily suspended.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 10 15:29:02 PST 2003
//
// ****************************************************************************

void
VisWinInteractions::SetSpinModeSuspended(bool val)
{
    spinModeSuspended = val;
}


// ****************************************************************************
//  Method: VisWinInteractions::GetSpinModeSuspdended
//
//  Purpose:
//      Determines whether the spin mode is suspended.
//
//  Returns:    true if spin mode is suspended, false otherwise.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Sep 10 15:59:18 PST 2003
//
// ****************************************************************************

bool
VisWinInteractions::GetSpinModeSuspended() const
{
    return spinModeSuspended;
}


// ****************************************************************************
//  Method: VisWinInteractions::NoPlots
//
//  Purpose:
//      Makes the current interactor be the NULL interactor, in response to
//      having no plots.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Oct 1 09:25:34 PDT 2001
//    I moved all of the behavior into the hot point interactor.
//
// ****************************************************************************

void
VisWinInteractions::NoPlots(void)
{
    if(hotPointInteractor != NULL)
    {
        hotPointInteractor->SetNullInteractor();
    }
}


// ****************************************************************************
//  Method: VisWinInteractions::HasPlots
//
//  Purpose:
//      Notifies the module that we now have plots in the window.  This
//      sends the interaction mode back in, kickstarting it off with the
//      null interactor.
//
//  Programmer: Hank Childs
//  Creation:   August 15, 2001
//
//  Modifications:
//    Kathleen Bonnell, Thu May 16 08:54:34 PDT 2002  
//    Lineout not valid for 3D, so test and set mode to Navigate if necessary. 
//
// ****************************************************************************

void
VisWinInteractions::HasPlots(void)
{
    if (!(mediator.GetMode() == WINMODE_3D && mode == LINEOUT))
    {
        SetInteractionMode(mode);
    }
    else 
    {
        SetInteractionMode(NAVIGATE);
    }
}


