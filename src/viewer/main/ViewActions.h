#ifndef VIEW_ACTIONS_H
#define VIEW_ACTIONS_H
#include <viewer_exports.h>
#include <ViewerAction.h>
#include <ViewerToggleAction.h>
#include <ViewerMultipleAction.h>
#include <vector>

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

class VIEWER_API TogglePerspectiveViewAction : public ViewerToggleAction
{
public:
    TogglePerspectiveViewAction(ViewerWindow *win);
    virtual ~TogglePerspectiveViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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

class VIEWER_API ResetViewAction : public ViewerAction
{
public:
    ResetViewAction(ViewerWindow *win);

    virtual ~ResetViewAction(){}

    virtual void Execute();
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

class VIEWER_API RecenterViewAction : public ViewerAction
{
public:
    RecenterViewAction(ViewerWindow *win);
    virtual ~RecenterViewAction(){}

    virtual void Execute();
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

class VIEWER_API UndoViewAction : public ViewerAction
{
public:
    UndoViewAction(ViewerWindow *win);
    virtual ~UndoViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
};

// ****************************************************************************
// Class: ToggleLockViewAction
//
// Purpose:
//   Handles the lock view action.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 5 16:44:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class VIEWER_API ToggleLockViewAction : public ViewerToggleAction
{
public:
    ToggleLockViewAction(ViewerWindow *win);
    virtual ~ToggleLockViewAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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

class VIEWER_API ToggleFullFrameAction : public ViewerToggleAction
{
public:
    ToggleFullFrameAction(ViewerWindow *win);
    virtual ~ToggleFullFrameAction(){}

    virtual void Execute();
    virtual bool Enabled() const;
    virtual bool Toggled() const;
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
//   Brad Whitlock, Thu Feb 27 14:35:57 PST 2003
//   I added VIEWCurve.
//
// ****************************************************************************

class VIEWER_API SaveViewAction : public ViewerMultipleAction
{
    static const int VIEWCurve;
    static const int VIEW2D;
    static const int VIEW3D;
    struct ViewInfo
    {
        int  viewType;
        void *view;
    };
    typedef std::vector<ViewInfo> ViewInfoVector;
public:
    SaveViewAction(ViewerWindow *win);
    virtual ~SaveViewAction();

    virtual void Execute(int);

    virtual bool Enabled() const;
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceToggled(int i) const { return false;}
private:
    void DeleteViews();
    void SaveCurrentView();
    void UseSavedView(int index);

    ViewInfoVector views;
};
#endif
