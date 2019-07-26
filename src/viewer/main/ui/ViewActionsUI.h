// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEW_ACTIONS_UI_H
#define VIEW_ACTIONS_UI_H
#include <viewer_exports.h>
#include <ViewerActionUISingle.h>
#include <ViewerActionUIMultiple.h>
#include <ViewerActionUIToggle.h>
#include <VisWindowTypes.h>
#include <vector>

class PickAttributes;

// ****************************************************************************
// Class: TogglePerspectiveViewAction
//
// Purpose:
//   Handles the toggle perspective view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:17:58 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API TogglePerspectiveViewActionUI : public ViewerActionUIToggle
{
public:
    TogglePerspectiveViewActionUI(ViewerActionLogic *L);
    virtual ~TogglePerspectiveViewActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
};

// ****************************************************************************
// Class: ResetViewAction
//
// Purpose:
//   Handles the reset view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:43:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ResetViewActionUI : public ViewerActionUISingle
{
public:
    ResetViewActionUI(ViewerActionLogic *L);
    virtual ~ResetViewActionUI(){}

    virtual bool Enabled() const;
    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: RecenterViewAction
//
// Purpose:
//   Handles the recenter view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:43:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API RecenterViewActionUI : public ViewerActionUISingle
{
public:
    RecenterViewActionUI(ViewerActionLogic *L);
    virtual ~RecenterViewActionUI(){}

    virtual bool Enabled() const;
    virtual bool MenuTopLevel() const { return true; }
};

// ****************************************************************************
// Class: UndoViewAction
//
// Purpose:
//   Handles the undo view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:44:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API UndoViewActionUI : public ViewerActionUISingle
{
public:
    UndoViewActionUI(ViewerActionLogic *L);
    virtual ~UndoViewActionUI(){}

    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: RedoViewAction
//
// Purpose:
//   Handles the redo view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 7 17:55:11 PST 2006
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API RedoViewActionUI : public ViewerActionUISingle
{
public:
    RedoViewActionUI(ViewerActionLogic *L);
    virtual ~RedoViewActionUI(){}

    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ToggleFullFrameAction
//
// Purpose:
//   Handles the toggle full frame view action.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   May 13, 2003 
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 13:34:10 PST 2003
//   
// ****************************************************************************

class VIEWER_API ToggleFullFrameActionUI : public ViewerActionUIToggle
{
public:
    ToggleFullFrameActionUI(ViewerActionLogic *L);
    virtual ~ToggleFullFrameActionUI(){}

    virtual bool Enabled() const;
    virtual bool Checked() const;
    virtual bool AllowInToolbar() const { return false; }
};

// ****************************************************************************
// Class: SaveViewAction
//
// Purpose:
//   Saves the current view into the action or sets the view from stored views.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 25 07:23:26 PDT 2003
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API SaveViewActionUI : public ViewerActionUIMultiple
{
public:
    SaveViewActionUI(ViewerActionLogic *L);
    virtual ~SaveViewActionUI();

    virtual void Update();

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const { return false;}
private:
    int NumSavedViews() const;

    void DeleteViewsFromInterface();
};

// ****************************************************************************
// Class: ChooseCenterOfRotationAction
//
// Purpose:
//   Chooses the center of rotation.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Dec 29 09:49:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ChooseCenterOfRotationActionUI : public ViewerActionUISingle
{
public:
    ChooseCenterOfRotationActionUI(ViewerActionLogic *L);
    virtual ~ChooseCenterOfRotationActionUI();

    virtual bool Enabled() const;
    virtual bool MenuTopLevel() const { return true; }
};

#endif
