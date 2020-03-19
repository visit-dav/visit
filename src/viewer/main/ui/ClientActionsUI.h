// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLIENT_ACTIONS_UI_H
#define CLIENT_ACTIONS_UI_H
#include <ViewerActionUISingle.h>

class OpenGUIClientActionUI : public ViewerActionUISingle
{
public:
    OpenGUIClientActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
    {
        SetAllText(tr("Open GUI"));
    }
    virtual ~OpenGUIClientActionUI() { }
    virtual bool AllowInToolbar() const { return false; }
};

class OpenCLIClientActionUI : public ViewerActionUISingle
{
public:
    OpenCLIClientActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
    {
        SetAllText(tr("Open CLI"));
    }
    virtual ~OpenCLIClientActionUI() { }
    virtual bool AllowInToolbar() const { return false; }
};

class MenuQuitActionUI : public ViewerActionUISingle
{
public:
    MenuQuitActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
    {
        SetAllText(tr("Quit"));
    }
    virtual ~MenuQuitActionUI() { }

    virtual bool AllowInToolbar() const { return false; }
};

#endif
