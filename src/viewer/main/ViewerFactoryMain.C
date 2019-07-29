// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerFactoryMain.h>

#include <ViewerFileServer.h>
#include <ViewerEngineManager.h>

// ****************************************************************************
// Method: ViewerFactoryMain::ViewerFactoryMain
//
// Purpose:
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 00:18:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactoryMain::ViewerFactoryMain() : ViewerFactory()
{
}

// ****************************************************************************
// Method: ViewerFactoryMain::~ViewerFactoryMain
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 00:18:38 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFactoryMain::~ViewerFactoryMain()
{
}

// ****************************************************************************
// Method: ViewerFactoryMain::CreateFileServerInterface
//
// Purpose:
//   Creates the file server interface we'll use.
//
// Returns:    A pointer to the new file server interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 21 15:02:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerFileServerInterface *
ViewerFactoryMain::CreateFileServerInterface()
{
    return new ViewerFileServer;
}

// ****************************************************************************
// Method: ViewerFactoryMain::CreateEngineManagerInterface
//
// Purpose:
//   Creates the engine manager interface we'll use.
//
// Returns:    A pointer to the new engine manager interface.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 21 15:02:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerEngineManagerInterface *
ViewerFactoryMain::CreateEngineManagerInterface()
{
    return new ViewerEngineManager;
}
