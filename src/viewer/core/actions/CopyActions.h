// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef COPY_ACTIONS_H
#define COPY_ACTIONS_H
#include <ViewerActionLogic.h>

// ****************************************************************************
// Class: CopyAnnotationsToWindowAction
//
// Purpose:
//   Handles ViewerRPC::CopyAnnotationsToWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CopyAnnotationsToWindowAction : public ViewerActionLogic
{
public:
    CopyAnnotationsToWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CopyAnnotationsToWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CopyLightingToWindowAction
//
// Purpose:
//   Handles ViewerRPC::CopyLightingToWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CopyLightingToWindowAction : public ViewerActionLogic
{
public:
    CopyLightingToWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CopyLightingToWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CopyPlotsToWindowAction
//
// Purpose:
//   Handles ViewerRPC::CopyPlotsToWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CopyPlotsToWindowAction : public ViewerActionLogic
{
public:
    CopyPlotsToWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CopyPlotsToWindowAction() {}

    virtual void Execute();
};

// ****************************************************************************
// Class: CopyViewToWindowAction
//
// Purpose:
//   Handles ViewerRPC::CopyViewToWindowRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class VIEWERCORE_API CopyViewToWindowAction : public ViewerActionLogic
{
public:
    CopyViewToWindowAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~CopyViewToWindowAction() {}

    virtual void Execute();
};

#endif
