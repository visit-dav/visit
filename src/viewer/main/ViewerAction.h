#ifndef VIEWER_ACTION_H
#define VIEWER_ACTION_H
#include <viewer_exports.h>
#include <ViewerActionBase.h>

class QAction;
class QIconSet;
class QPopupMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerAction
//
// Purpose:
//   This class defines an action that the viewer can perform. An action is
//   special in that it can appear in the menu or the toolbar besides being
//   callable from the viewer's clients.
//
// Notes:      This class is abstract so it forces derived classes to
//             define their own Execute method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 09:44:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:03:30 PDT 2003
//   I added the RemoveFromMenu and RemoveFromToolbar methods.
//
// ****************************************************************************

class VIEWER_API ViewerAction : public ViewerActionBase
{
    Q_OBJECT
public:
    ViewerAction(ViewerWindow *win, const char *name = 0);
    virtual ~ViewerAction();
    
    virtual void Setup();
    virtual void Execute() = 0;
    virtual void Update();

    virtual bool Enabled() const { return true;  }
    virtual bool Toggled() const { return false; }

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QPopupMenu *menu);
    virtual void RemoveFromMenu(QPopupMenu *menu);
    virtual void ConstructToolbar(QToolBar *toolbar);
    virtual void RemoveFromToolbar(QToolBar *toolbar);

    // Methods to set the action's attributes.
    virtual void SetAllText(const char *text);
    virtual void SetText(const char *text);
    virtual void SetMenuText(const char *text);
    virtual void SetToolTip(const char *text);
    virtual void SetIconSet(const QIconSet &icons);
    virtual void SetToggleAction(bool val);
protected slots:
    virtual void HandleToggle(bool);
protected:
    QAction *action;
};

#endif
