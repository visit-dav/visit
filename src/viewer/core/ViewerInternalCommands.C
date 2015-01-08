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
#include <ViewerInternalCommands.h>

#include <ViewerMessaging.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerQueryManager.h>
#include <ViewerMethods.h>

//////////////////////////////////////////////////////////////////////////////

ViewerCommandUpdateWindow::ViewerCommandUpdateWindow(ViewerWindow *w) : 
    ViewerInternalCommand(), window(w)
{
}

void
ViewerCommandUpdateWindow::Execute()
{
    window->EnableUpdates();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandRedrawWindow::ViewerCommandRedrawWindow(ViewerWindow *w) : 
    ViewerInternalCommand(), window(w)
{
}

void
ViewerCommandRedrawWindow::Execute()
{
    window->RedrawWindow();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandDeleteWindow::ViewerCommandDeleteWindow(ViewerWindow *w) : 
    ViewerInternalCommand(), window(w)
{
}

void
ViewerCommandDeleteWindow::Execute()
{
    ViewerWindowManager::Instance()->DeleteWindow(window);
    ViewerWindowManager::Instance()->UpdateActions();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandActivateTool::ViewerCommandActivateTool(ViewerWindow *w, int id) :
    ViewerInternalCommand(), window(w), toolId(id)
{
}

void
ViewerCommandActivateTool::Execute()
{
    // Tell the viewer window manager to activate the tool.
    window->SetToolEnabled(toolId, true);
    ViewerWindowManager::Instance()->UpdateActions();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandSetInteractionMode::ViewerCommandSetInteractionMode(
    ViewerWindow *w, INTERACTION_MODE m) : ViewerInternalCommand(),
    window(w), mode(m)
{
}

void
ViewerCommandSetInteractionMode::Execute()
{
    // Tell the window to set its interaction mode.
    window->SetInteractionMode(mode);
    ViewerWindowManager::Instance()->UpdateActions();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandSetToolUpdateMode::ViewerCommandSetToolUpdateMode(
    ViewerWindow *w, TOOLUPDATE_MODE m) : ViewerInternalCommand(),
    window(w), mode(m)
{
}

void
ViewerCommandSetToolUpdateMode::Execute()
{
    // Tell the window to set its toolupdate mode.
    window->SetToolUpdateMode(mode);
    ViewerWindowManager::Instance()->UpdateActions();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandUpdateFrame::ViewerCommandUpdateFrame(ViewerWindow *w) : 
    ViewerInternalCommand(), window(w)
{
}

void
ViewerCommandUpdateFrame::Execute()
{
    // Tell the window's animation to update.
    window->GetPlotList()->UpdateFrame();
    ViewerWindowManager::Instance()->UpdateActions();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandSetScalableRenderingMode::ViewerCommandSetScalableRenderingMode(
    ViewerWindow *w, bool m) : ViewerInternalCommand(), window(w), mode(m)
{
}

void
ViewerCommandSetScalableRenderingMode::Execute()
{
    // Tell the window to change scalable rendering modes, if necessary 
    if (window->GetScalableRendering() != mode)
        window->ChangeScalableRenderingMode(mode);
}

//////////////////////////////////////////////////////////////////////////////

void
ViewerCommandFinishLineout::Execute()
{
    ViewerQueryManager::Instance()->FinishLineout(); 
}

//////////////////////////////////////////////////////////////////////////////

void
ViewerCommandFinishLineQuery::Execute()
{
    ViewerQueryManager::Instance()->FinishLineQuery();
    GetViewerMessaging()->ClearStatus();
}

//////////////////////////////////////////////////////////////////////////////

void
ViewerCommandUpdateAnnotationObjectList::Execute()
{
    ViewerWindowManager::Instance()->UpdateAnnotationObjectList();
}

//////////////////////////////////////////////////////////////////////////////

ViewerCommandUpdateNamedSelection::ViewerCommandUpdateNamedSelection(
    const std::string &s) : ViewerInternalCommand(), selectionName(s)
{
}

void
ViewerCommandUpdateNamedSelection::Execute()
{
    GetViewerMethods()->UpdateNamedSelection(selectionName, true, false);
}

