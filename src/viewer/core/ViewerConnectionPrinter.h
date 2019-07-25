// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CONNECTION_PRINTER_H
#define VIEWER_CONNECTION_PRINTER_H
#include <viewercore_exports.h>

class Connection;

// ****************************************************************************
// Class: ViewerConnectionPrinter
//
// Purpose:
//   Base class for objects that print VCL console output.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 21 15:14:14 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue May 27 14:17:46 PDT 2008
//   Removed name.
//
// ****************************************************************************

class VIEWERCORE_API ViewerConnectionPrinter
{
public:
    ViewerConnectionPrinter();
    virtual ~ViewerConnectionPrinter();
    virtual void SetConnection(Connection *);
protected:
    Connection *conn;
};

#endif
