#ifndef VIEWER_TOOLBAR_H
#define VIEWER_TOOLBAR_H
#include <viewer_exports.h>
#include <string>
#include <map>

class QToolBar;
class ViewerActionBase;
class ViewerWindow;

// ****************************************************************************
// Class: ViewerToolbar
//
// Purpose:
//   Encapsulates ViewerWindow's toolbars.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 29 11:48:16 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Feb 25 10:25:32 PDT 2003
//   I added the RemoveAction method.
//
// ****************************************************************************

class VIEWER_API ViewerToolbar
{
    struct ToolbarItem
    {
        ToolbarItem();
        ToolbarItem(const ToolbarItem &);
        ToolbarItem(QToolBar *, bool);
        virtual ~ToolbarItem();
        void operator = (const ToolbarItem &);
        QToolBar *toolbar;
        bool     visible;
    };
        
    typedef std::map<std::string, ToolbarItem> ToolbarMap;
public:
    ViewerToolbar(ViewerWindow *win);
    virtual ~ViewerToolbar();

    void Show(const std::string &toolBarName);
    void Hide(const std::string &toolBarName);
    void ShowAll();
    void HideAll();

    void AddAction(const std::string &toolBarName, ViewerActionBase *action);
    void RemoveAction(ViewerActionBase *action);
private:
    ToolbarItem CreateToolbar(const std::string &toolBarName);

    ViewerWindow *window;
    bool          hidden;
    ToolbarMap    toolbars;
};

#endif
