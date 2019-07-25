// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_ACTION_UI_SINGLE_H
#define VIEWER_ACTION_UI_SINGLE_H
#include <viewer_exports.h>
#include <ViewerActionUI.h>

class QAction;
class QIcon;
class QMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerActionUISingle
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
//
// ****************************************************************************

class VIEWER_API ViewerActionUISingle : public ViewerActionUI
{
    Q_OBJECT
public:
    ViewerActionUISingle(ViewerActionLogic *L);
    virtual ~ViewerActionUISingle();
    
    virtual void Update();

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QMenu *menu);
    virtual void RemoveFromMenu(QMenu *menu);
    virtual void ConstructToolbar(QToolBar *toolbar);
    virtual void RemoveFromToolbar(QToolBar *toolbar);
    virtual void UpdateConstruction();
protected:
    // Methods to set the action's attributes.
    virtual void SetText(const QString &text);
    virtual void SetMenuText(const QString &text);
    virtual void SetToolTip(const QString &text);
    virtual void SetIcon(const QIcon &icons);
    virtual void SetToggleAction(bool val);
protected:
    QAction *action;
};

#endif
