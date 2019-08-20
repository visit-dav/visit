// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PRINT_WINDOW_ACTION_H
#define PRINT_WINDOW_ACTION_H
#include <ViewerActionLogic.h>

class ViewerWindow;

// ****************************************************************************
// Class: PrintWindowAction
//
// Purpose:
//   Handles ViewerRPC::OpenMDServerRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class PrintWindowAction : public ViewerActionLogic
{
public:
    PrintWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~PrintWindowAction() {}

    virtual void Execute();
};

#endif
