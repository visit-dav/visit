// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SET_APPEARANCE_ACTION_H
#define SET_APPEARANCE_ACTION_H
#include <ViewerActionLogic.h>

class ViewerWindow;

// ****************************************************************************
// Class: SetAppearanceAction
//
// Purpose:
//   Handles ViewerRPC::SetAppearanceRPC
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:48:37 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

class SetAppearanceAction : public ViewerActionLogic
{
public:
    SetAppearanceAction(ViewerWindow *win) : ViewerActionLogic(win) {}
    virtual ~SetAppearanceAction() {}

    virtual void Execute();
};

#endif
