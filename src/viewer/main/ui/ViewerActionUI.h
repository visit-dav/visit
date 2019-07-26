// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_ACTION_UI_H
#define VIEWER_ACTION_UI_H
#include <viewer_exports.h>
#include <ViewerBaseUI.h>

class ViewerActionLogic;

class QIcon;
class QMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerActionUI
//
// Purpose:
//   This is an abstract base class for action UI's.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 14:46:56 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerActionUI : public ViewerBaseUI
{
    Q_OBJECT
public:
    ViewerActionUI(ViewerActionLogic *);
    virtual ~ViewerActionUI();

    virtual void Update() = 0;

    virtual bool Enabled() const           { return true;  }
    virtual bool Checked() const           { return false; }
    virtual bool MenuTopLevel() const      { return false; }
    virtual bool CanHaveOwnToolbar() const { return false; }
    virtual bool AllowInToolbar() const    { return true; }

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QMenu *menu) = 0;
    virtual void RemoveFromMenu(QMenu *menu) = 0;
    virtual void ConstructToolbar(QToolBar *toolbar) = 0;
    virtual void RemoveFromToolbar(QToolBar *toolbar) = 0;
    virtual void UpdateConstruction() = 0;

    ViewerActionLogic *GetLogic() const;
protected slots:
    virtual void Activate();
    virtual void ActivateEx(bool interactive);
protected:
    virtual void PreExecute();
    virtual void PostponeExecute();

    // Methods to set the action's attributes.
    virtual void SetText(const QString &text) = 0;
    virtual void SetMenuText(const QString &text) = 0;
    virtual void SetToolTip(const QString &text) = 0;
    virtual void SetIcon(const QIcon &icons) = 0;
    virtual void SetToggleAction(bool val);

    void         SetAllText(const QString &text);

    void         SetActiveAction(int);
    int          GetActiveAction() const;
private:
    ViewerActionLogic *logic;
    int                activeAction;
};

#endif
