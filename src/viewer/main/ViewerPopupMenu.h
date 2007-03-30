#ifndef VIEWER_POPUP_MENU_H
#define VIEWER_POPUP_MENU_H
#include <viewer_exports.h>
#include <string>
#include <map>

// Forward declares.
class QPopupMenu;
class ViewerActionBase;
class ViewerWindow;

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
// ****************************************************************************

class VIEWER_API ViewerPopupMenu
{
    struct SubMenuInfo
    {
        SubMenuInfo();
        SubMenuInfo(const SubMenuInfo &);
        SubMenuInfo(QPopupMenu *, int);
        virtual ~SubMenuInfo();
        void operator = (const SubMenuInfo &);

        QPopupMenu *menu;
        int         menuId;
    };

    typedef std::map<std::string, SubMenuInfo> MenuMap;
public:
    ViewerPopupMenu(ViewerWindow *win);
    virtual ~ViewerPopupMenu();

    void ShowMenu();
    void HideMenu();
    void SetEnabled(bool val);

    void AddAction(ViewerActionBase *action);
    void AddAction(const std::string &menuName, ViewerActionBase *action);
    void RemoveAction(ViewerActionBase *action);
    void EnableMenu(const std::string &menuName);
    void DisableMenu(const std::string &menuName);
private:
    QPopupMenu   *CreateMenu(const std::string &name);

    QPopupMenu   *popup;
    ViewerWindow *window;
    MenuMap       menus;
};

#endif
