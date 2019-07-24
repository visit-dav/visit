// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerConnectionPrinter.h>
#include <Connection.h>

ViewerConnectionPrinter::ViewerConnectionPrinter() : conn(NULL)
{
}

ViewerConnectionPrinter::~ViewerConnectionPrinter()
{
}

void
ViewerConnectionPrinter::SetConnection(Connection *c)
{
    conn = c;
}
