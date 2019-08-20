// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef TOOLBAR_ACTIONS_UI_H
#define TOOLBAR_ACTIONS_UI_H
#include <ViewerActionLogic.h>

#include <ViewerActionUIMultiple.h>
#include <ViewerActionUISingle.h>
#include <ViewerActionUIToggle.h>

// ****************************************************************************
// Class: EnableToolbarAction
//
// Purpose:
//   This action enables or disables toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:51:32 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:55:49 PDT 2008
//   Added tr()
//
// ****************************************************************************

class EnableToolbarAction : public ViewerActionLogic
{
public:
    EnableToolbarAction(ViewerWindow *win) : ViewerActionLogic(win)
    {
    }

    virtual ~EnableToolbarAction() { }

    virtual void Setup(int activeAction, bool toggled);
    virtual void Execute();
};

class EnableToolbarActionUI : public ViewerActionUIMultiple
{
public:
    EnableToolbarActionUI(ViewerActionLogic *L);
    virtual ~EnableToolbarActionUI() { }

    virtual bool ChoiceChecked(int i) const;

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: HideToolbarsAction
//
// Purpose:
//   This action temporarily hides toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:52:14 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be off.
//
//   Brad Whitlock, Tue Apr 29 11:55:58 PDT 2008
//   Added tr()
//
// ****************************************************************************

class HideToolbarsAction : public ViewerActionLogic
{
public:
    HideToolbarsAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~HideToolbarsAction() { }

    virtual void Execute();
};

class HideToolbarsActionUI : public ViewerActionUISingle
{
public:
    HideToolbarsActionUI(ViewerActionLogic *L);
    virtual ~HideToolbarsActionUI() { }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: HideToolbarsForAllWindowsAction
//
// Purpose:
//   This action temporarily hides toolbars in all viewer windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 1 10:34:13 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be off.
//
//   Brad Whitlock, Tue Apr 29 11:56:13 PDT 2008
//   Added tr()
//
// ****************************************************************************

class HideToolbarsForAllWindowsAction : public ViewerActionLogic
{
public:
    HideToolbarsForAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~HideToolbarsForAllWindowsAction() { }

    virtual void Execute();
};

class HideToolbarsForAllWindowsActionUI : public ViewerActionUISingle
{
public:
    HideToolbarsForAllWindowsActionUI(ViewerActionLogic *L);
    virtual ~HideToolbarsForAllWindowsActionUI() { }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ShowToolbarsAction
//
// Purpose:
//   This shows the toolbars that are marked as being visible.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:52:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be on.
//
//   Brad Whitlock, Tue Apr 29 11:56:27 PDT 2008
//   Added tr()
//
// ****************************************************************************

class ShowToolbarsAction : public ViewerActionLogic
{
public:
    ShowToolbarsAction(ViewerWindow *win) : ViewerActionLogic(win)
    {
    }
    virtual ~ShowToolbarsAction() { }

    virtual void Execute();
};

class ShowToolbarsActionUI : public ViewerActionUISingle
{
public:
    ShowToolbarsActionUI(ViewerActionLogic *L);
    virtual ~ShowToolbarsActionUI() { }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ShowToolbarsForAllWindowsAction
//
// Purpose:
//   This action shows toolbars in all viewer windows.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 1 10:34:13 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 24 13:30:24 PST 2004
//   I added code to record that toolbars should be on.
//
//   Brad Whitlock, Tue Apr 29 11:56:44 PDT 2008
//   Added tr()
//
// ****************************************************************************

class ShowToolbarsForAllWindowsAction : public ViewerActionLogic
{
public:
    ShowToolbarsForAllWindowsAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~ShowToolbarsForAllWindowsAction() { }

    virtual void Execute();
};

class ShowToolbarsForAllWindowsActionUI : public ViewerActionUISingle
{
public:
    ShowToolbarsForAllWindowsActionUI(ViewerActionLogic *L);
    virtual ~ShowToolbarsForAllWindowsActionUI() { }

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SetToolbarIconSizeAction
//
// Purpose:
//   This action tells the window manager to make all vis windows use either
//   large or small icons in the their toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 09:52:26 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 10:17:06 PDT 2004
//   Fixed to account for changes in ViewerToggleAction.
//
//   Brad Whitlock, Tue Apr 29 11:56:57 PDT 2008
//   Added tr()
//
//   Brad Whitlock, Fri May 23 10:26:52 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class SetToolbarIconSizeAction : public ViewerActionLogic
{
public:
    SetToolbarIconSizeAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~SetToolbarIconSizeAction() { }

    virtual void Execute();
protected:
    virtual void Setup(int activeAction, bool toggled);
};

class SetToolbarIconSizeActionUI : public ViewerActionUIToggle
{
public:
    SetToolbarIconSizeActionUI(ViewerActionLogic *L);
    virtual ~SetToolbarIconSizeActionUI() { }

    virtual void Update();
    virtual bool Checked() const;

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ToggleAllowPopupAction
//
// Purpose:
//   This action enables or disables the popup menu.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 15:51:32 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr 29 11:55:49 PDT 2008
//   Added tr()
//
// ****************************************************************************

class ToggleAllowPopupAction : public ViewerActionLogic
{
public:
    ToggleAllowPopupAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~ToggleAllowPopupAction() { }

    virtual void Setup(int activeAction, bool toggled);
    virtual void Execute();
};

#endif
