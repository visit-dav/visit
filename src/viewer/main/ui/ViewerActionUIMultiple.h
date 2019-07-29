// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_ACTION_UI_MULTIPLE_H
#define VIEWER_ACTION_UI_MULTIPLE_H
#include <viewer_exports.h>
#include <ViewerActionUI.h>

#include <QIcon>
#include <QPixmap>
#include <QString>

#include <vector>

class QAction;
class QActionGroup;
class QMenu;
class QToolBar;

// ****************************************************************************
// Class: ViewerActionUIMultiple
//
// Purpose:
//   This is a base class for actions that multiple toolbar buttons or menu
//   options but still service a single RPC.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 30 13:49:49 PST 2003
//
// Modifications:
//   Sean Ahern, Thu Feb 20 01:51:35 America/Los_Angeles 2003
//   Added the ability to set large and small icons.
//
//   Brad Whitlock, Tue Aug 26 17:12:02 PST 2003
//   I added the isExclusive flag.
//
//   Brad Whitlock, Tue Apr 29 11:16:17 PDT 2008
//   Converted to QString for menu items.
//
//   Brad Whitlock, Thu May 22 13:44:11 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class VIEWER_API ViewerActionUIMultiple : public ViewerActionUI
{
    Q_OBJECT

    typedef std::vector<QAction *> ActionPointerVector;
public:
    ViewerActionUIMultiple(ViewerActionLogic *L);
    virtual ~ViewerActionUIMultiple();
    
    virtual void Update();

    virtual bool Enabled() const;

    // Methods to add the action to the menu and toolbar.
    virtual void ConstructMenu(QMenu *menu);
    virtual void RemoveFromMenu(QMenu *menu);
    virtual void ConstructToolbar(QToolBar *toolbar);
    virtual void RemoveFromToolbar(QToolBar *toolbar);
    virtual void UpdateConstruction();

protected:
    virtual void PostponeExecute();

    // Methods to set the action's attributes.
    virtual void SetText(const QString &text);
    virtual void SetMenuText(const QString &text);
    virtual void SetToolTip(const QString &text);
    virtual void SetIcon(const QIcon &icon);
    virtual bool ChoiceEnabled(int i) const;
    virtual bool ChoiceChecked(int i) const;
    virtual void HideChoice(int i);
    virtual void AddChoice(const QString &menuText);
    virtual void AddChoice(const QString &menuText, const QString &toolTip, const
                           QPixmap &icon);
    virtual void AddChoice(const QString &menuText, const QString &toolTip,
                           const QPixmap &small_icon,
                           const QPixmap &large_icon);
    virtual void SetExclusive(bool val);
protected slots:
    void ActivateHelper(QAction*);
protected:
    bool                 iconSpecified;
    bool                 toggled;
    QString              text;
    QString              menuText;
    QString              toolTip;
    QIcon                icon;
    QActionGroup        *action;
    QMenu               *actionMenu;
    bool                 isExclusive;
    ActionPointerVector  children;
};

#endif
