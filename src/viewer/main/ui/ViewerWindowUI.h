// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_WINDOW_UI_H
#define VIEWER_WINDOW_UI_H
#include <viewer_exports.h>
#include <ViewerWindow.h>

class ViewerPopupMenu;
class ViewerToolbar;

// ****************************************************************************
// Class: ViewerWindowUI
//
// Purpose:
//   ViewerWindow subclass that has UI elements.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:38:41 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerWindowUI : public ViewerWindow
{
public:
    static ViewerWindowUI *SafeDownCast(ViewerWindow *);

    ViewerWindowUI(int windowIndex);
    virtual ~ViewerWindowUI();

    void SetVisWindow(VisWindow *); // override

    ViewerPopupMenu *GetPopupMenu() { return popupMenu; }
    ViewerToolbar   *GetToolbar()   { return toolbar; }

    void ShowMenu();
    void HideMenu();

    void SetLargeIcons(bool);

    void *CreateToolbar(const std::string &name);
protected:
    static void ShowCallback(void *);
    static void HideCallback(void *);
    static void ShowMenuCallback(void *);
    static void HideMenuCallback(void *);
    static void CloseCallback(void *);

    ViewerPopupMenu     *popupMenu;
    ViewerToolbar       *toolbar;
};

#endif
