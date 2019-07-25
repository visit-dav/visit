// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef WINDOW_ACTIONS_H
#define WINDOW_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerActionUISingle.h>
#include <ViewerActionUIToggle.h>
#include <ViewerActionUIMultiple.h>

// ****************************************************************************
// Class: AddWindowActionUI
//
// Purpose:
//   Handles the add window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:00 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API AddWindowActionUI : public ViewerActionUISingle
{
public:
    AddWindowActionUI(ViewerActionLogic *L);
    virtual ~AddWindowActionUI(){}
};

// ****************************************************************************
// Class: CloneWindowActionUI
//
// Purpose:
//   Handles the clone window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:03 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API CloneWindowActionUI : public ViewerActionUISingle
{
public:
    CloneWindowActionUI(ViewerActionLogic *L);
    virtual ~CloneWindowActionUI() {}
};

// ****************************************************************************
// Class: DeleteWindowActionUI
//
// Purpose:
//   Handles the delete window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:22 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API DeleteWindowActionUI : public ViewerActionUISingle
{
public:
    DeleteWindowActionUI(ViewerActionLogic *L);
    virtual ~DeleteWindowActionUI(){}

    virtual void Update();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ClearWindowActionUI
//
// Purpose:
//   Handles the clear window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:16:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearWindowActionUI : public ViewerActionUISingle
{
public:
    ClearWindowActionUI(ViewerActionLogic *L);
    virtual ~ClearWindowActionUI(){}

    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ClearAllWindowsActionUI
//
// Purpose:
//   Handles the clear all windows action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:01 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearAllWindowsActionUI : public ViewerActionUISingle
{
public:
    ClearAllWindowsActionUI(ViewerActionLogic *L);
    virtual ~ClearAllWindowsActionUI(){}

    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ClearPickPointsActionUI
//
// Purpose:
//   Handles clearing of pick points.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:16:19 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearPickPointsActionUI : public ViewerActionUISingle
{
public:
    ClearPickPointsActionUI(ViewerActionLogic *L);
    virtual ~ClearPickPointsActionUI() { };

    virtual bool Enabled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: RemovePicksActionUI
//
// Purpose:
//    Handles removing a list of picks. 
//
// Notes:      
//
// Programmer: Alister Maguire
// Creation:   Mon Oct 16 15:41:23 PDT 2017
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API RemovePicksActionUI : public ViewerActionUISingle
{
public:
    RemovePicksActionUI(ViewerActionLogic *L);
    virtual ~RemovePicksActionUI() { };

    virtual bool Enabled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: ClearReferenceLinesActionUI
//
// Purpose:
//   Clears reference lines.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 16:16:39 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ClearReferenceLinesActionUI : public ViewerActionUISingle
{
public:
    ClearReferenceLinesActionUI(ViewerActionLogic *L);
    virtual ~ClearReferenceLinesActionUI() { };

    virtual bool Enabled() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SetActiveWindowActionUI
//
// Purpose:
//   Handles the set active window action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API SetActiveWindowActionUI : public ViewerActionUIToggle
{
public:
    SetActiveWindowActionUI(ViewerActionLogic *L);
    virtual ~SetActiveWindowActionUI(){}

    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ToggleSpinModeActionUI
//
// Purpose:
//   Handles the toggle spin mode action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleSpinModeActionUI : public ViewerActionUIToggle
{
public:
    ToggleSpinModeActionUI(ViewerActionLogic *L);
    virtual ~ToggleSpinModeActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};


// ****************************************************************************
// Class: SetWindowLayoutActionUI
//
// Purpose:
//   Handles the set window layout action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:18:44 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API SetWindowLayoutActionUI : public ViewerActionUIMultiple
{
public:
    SetWindowLayoutActionUI(ViewerActionLogic *L);
    virtual ~SetWindowLayoutActionUI() { }

    virtual bool ChoiceChecked(int i) const;
};

// ****************************************************************************
// Class: InvertBackgroundActionUI
//
// Purpose:
//   Handles the invert background color action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:19:10 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API InvertBackgroundActionUI : public ViewerActionUISingle
{
public:
    InvertBackgroundActionUI(ViewerActionLogic *L);
    virtual ~InvertBackgroundActionUI() { }
};

// ****************************************************************************
// Class: SetWindowModeActionUI
//
// Purpose:
//   Handles the set window mode action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:19:23 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Tue Jul 22 16:28:08 PDT 2003
//   Added Setup.
//   
//   Brad Whitlock, Wed May 28 15:50:34 PDT 2008
//   Removed CanHaveOwnToolbar.
//
// ****************************************************************************

class VIEWER_API SetWindowModeActionUI : public ViewerActionUIMultiple
{
public:
    SetWindowModeActionUI(ViewerActionLogic *L);
    virtual ~SetWindowModeActionUI() { }

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;

    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: EnableToolActionUI
//
// Purpose:
//   Handles the enable tool action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:20:03 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed May 28 15:50:34 PDT 2008
//   Removed CanHaveOwnToolbar.
//
// ****************************************************************************

class VIEWER_API EnableToolActionUI : public ViewerActionUIMultiple
{
public:
    EnableToolActionUI(ViewerActionLogic *L);
    virtual ~EnableToolActionUI() { }

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;

    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: SetToolUpdateModeActionUI
//
// Purpose:
//   Handles the enable tool action.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   February  2, 2010
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API SetToolUpdateModeActionUI : public ViewerActionUIMultiple
{
public:
    SetToolUpdateModeActionUI(ViewerActionLogic *L);
    virtual ~SetToolUpdateModeActionUI() { }

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;

    virtual bool MenuTopLevel() const { return true; }
    virtual bool AllowInToolbar() const { return false; }
};

#endif
