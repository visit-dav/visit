// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_TOOLBAR_H
#define VIEWER_TOOLBAR_H
#include <viewer_exports.h>
#include <ViewerBaseUI.h>
#include <string>
#include <map>

class QToolBar;
class ViewerActionUI;
class ViewerWindowUI;

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

class VIEWER_API ViewerToolbar : public ViewerBaseUI
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
    ViewerToolbar(ViewerWindowUI *win);
    virtual ~ViewerToolbar();

    void Show(const std::string &toolBarName);
    void Hide(const std::string &toolBarName);
    void ShowAll();
    void HideAll();

    void AddAction(const std::string &toolBarName, ViewerActionUI *action);
    void RemoveAction(ViewerActionUI *action);
private:
    ToolbarItem CreateToolbar(const std::string &toolBarName);

    ViewerWindowUI *window;
    bool            hidden;
    ToolbarMap      toolbars;
};

#endif
