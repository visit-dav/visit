// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtToolInterface.h>
#include <AttributeSubject.h>

avtToolInterfaceCallback avtToolInterface::callback = 0;

avtToolInterface::avtToolInterface(const VisWindow *v)
{
    visWindow = v;
    atts = NULL;
}

avtToolInterface::~avtToolInterface()
{
    if(atts != NULL)
        delete atts;
}

