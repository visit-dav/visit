// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_POPUP_MENU_H
#define VIEWER_POPUP_MENU_H
#include <viewer_exports.h>
#include <string>
#include <map>

#include <ViewerBaseUI.h>

// Forward declares.
class QAction;
class QMenu;
class ViewerActionUI;
class ViewerWindowUI;

// ****************************************************************************
// Class: ViewerPopupMenu
//
// Purpose:
//   This is the popup menu class for the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 4 15:37:09 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:09:39 PDT 2003
//   I added RemoveAction.
//
//   Brad Whitlock, Tue May 27 14:10:57 PDT 2008
//   Qt 4.
//  
//   Marc Durant, Tue Dec 27 13:53:00 MDT 2011
//   Added IsEnabled accessor
//
// ****************************************************************************

class VIEWER_API ViewerPopupMenu : public ViewerBaseUI
{
    struct SubMenuInfo
    {
        SubMenuInfo();
        SubMenuInfo(const SubMenuInfo &);
        SubMenuInfo(QMenu *, QAction *);
        virtual ~SubMenuInfo();
        void operator = (const SubMenuInfo &);

        QMenu   *menu;
        QAction *action;
    };

    typedef std::map<std::string, SubMenuInfo> MenuMap;
public:
    ViewerPopupMenu(ViewerWindowUI *win);
    virtual ~ViewerPopupMenu();

    void ShowMenu();
    void HideMenu();
    void SetEnabled(bool val);
    bool IsEnabled();

    void AddAction(ViewerActionUI *action);
    void AddAction(const std::string &menuName, ViewerActionUI *action);
    void RemoveAction(ViewerActionUI *action);
    void EnableMenu(const std::string &menuName);
    void DisableMenu(const std::string &menuName);
    QMenu* GetPopup() { return popup; }
private:
    QMenu   *CreateMenu(const std::string &name);

    QMenu          *popup;
    ViewerWindowUI *window;
    MenuMap         menus;
};

#endif
