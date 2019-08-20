// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MDSERVER_ACTIONS_H
#define MDSERVER_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: OpenMDServerAction
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

class VIEWERCORE_API OpenMDServerAction : public ViewerActionLogic
{
public:
    OpenMDServerAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~OpenMDServerAction() {}

    virtual void Execute();
};

#endif
