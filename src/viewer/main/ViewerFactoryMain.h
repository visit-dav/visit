// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_FACTORY_MAIN_H
#define VIEWER_FACTORY_MAIN_H
#include <viewer_exports.h>
#include <ViewerFactory.h>

// ****************************************************************************
// Class: ViewerFactoryMain
//
// Purpose:
//   Creates various viewer objects.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 21 15:07:15 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerFactoryMain : public ViewerFactory
{
public:
    ViewerFactoryMain();
    virtual ~ViewerFactoryMain();

    virtual ViewerFileServerInterface *CreateFileServerInterface();

    virtual ViewerEngineManagerInterface *CreateEngineManagerInterface();
};

#endif
