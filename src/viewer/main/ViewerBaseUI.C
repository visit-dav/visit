// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerBaseUI.h>

// ****************************************************************************
// Method: ViewerBaseUI::ViewerBaseUI
//
// Purpose:
//   Constructor
//
// Arguments:
//   parent : The parent object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 16:26:14 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerBaseUI::ViewerBaseUI(QObject *parent) : QObject(parent), ViewerBase()
{
}

// ****************************************************************************
// Method: ViewerBaseUI::~ViewerBaseUI
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 16:26:33 PDT 2014
//
// Modifications:
//
// ****************************************************************************

ViewerBaseUI::~ViewerBaseUI()
{
}
