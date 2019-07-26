// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLIENT_ACTIONS_H
#define CLIENT_ACTIONS_H
#include <ViewerActionLogic.h>
#include <ViewerWindow.h>
#include <ViewerMethods.h>
#include <InstallationFunctions.h>

// ****************************************************************************
// Class: OpenGUIClientAction
//
// Purpose:
//   This action opens the VisIt GUI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenGUIClientAction : public ViewerActionLogic
{
public:
    OpenGUIClientAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~OpenGUIClientAction() { }

    virtual void Execute()
    {
        stringVector args;
        GetViewerMethods()->OpenClient("GUI", GetVisItLauncher(), args);
    }
};

// ****************************************************************************
// Class: OpenCLIClientAction
//
// Purpose:
//   This action opens the VisIt CLI.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API OpenCLIClientAction : public ViewerActionLogic
{
public:
    OpenCLIClientAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~OpenCLIClientAction() { }

    virtual void Execute()
    {
        stringVector args;
        args.push_back("-cli");
        args.push_back("-newconsole");
        GetViewerMethods()->OpenClient("CLI", GetVisItLauncher(), args);
    }
};

// ****************************************************************************
// Class: MenuQuitAction
//
// Purpose:
//   This action quits the viewer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 14 09:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API MenuQuitAction : public ViewerActionLogic
{
public:
    MenuQuitAction(ViewerWindow *win) : ViewerActionLogic(win) { }
    virtual ~MenuQuitAction() { }

    virtual void Execute()
    {
        GetViewerMethods()->Close();
        EXCEPTION1(VisItException, "Bypass action manager update. This is safe and by design");
    }
};

#endif
