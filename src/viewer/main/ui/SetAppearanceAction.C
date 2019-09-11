// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SetAppearanceAction.h>
#include <ViewerWindow.h>
#include <ViewerState.h>
#include <AppearanceAttributes.h>

#include <Appearance.h>

#include <QApplication>

// ****************************************************************************
// Method: SetAppearanceAction::Execute
//
// Purpose: 
//   Execute ViewerRPC::SetAppearanceRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
SetAppearanceAction::Execute()
{
    SetAppearance(qApp, GetViewerState()->GetAppearanceAttributes());
}
