// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerSubjectProxyFactory.h>
#include <ViewerActionManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>

ViewerSubjectProxyFactory::ViewerSubjectProxyFactory() : ViewerFactoryUI()
{
    createWindowCB = NULL;
    createWindowCBData = NULL;
}

ViewerSubjectProxyFactory::~ViewerSubjectProxyFactory()
{
}

ViewerWindow *
ViewerSubjectProxyFactory::CreateViewerWindow(int windowIndex)
{
    ViewerWindow *win = ViewerFactoryUI::CreateViewerWindow(windowIndex);

    // Ensure the action UI's get created before we call the window creation callback.
    win->GetActionManager()->EnableActions(ViewerWindowManager::Instance()->GetWindowAtts());

    if(createWindowCB != NULL)
        (*createWindowCB)(win, createWindowCBData);

    return win;
}

void
ViewerSubjectProxyFactory::SetCreateViewerWindowCallback(
    void (*cb)(ViewerWindow *, void *), void *cbdata)
{
    createWindowCB = cb;
    createWindowCBData = cbdata;
}
