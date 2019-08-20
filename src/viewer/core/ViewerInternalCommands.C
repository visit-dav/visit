// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

