// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtBoxToolInterface.h>
#include <BoxExtents.h>

avtBoxToolInterface::avtBoxToolInterface(const VisWindow *v) : 
    avtToolInterface(v)
{
    atts = new BoxExtents;
    initialized = false;
}

avtBoxToolInterface::~avtBoxToolInterface()
{
    // nothing
}

void
avtBoxToolInterface::SetExtents(const double *d)
{
    BoxExtents *ext = (BoxExtents *)atts;
    ext->SetExtents(d);
    initialized = true;
}

const double *
avtBoxToolInterface::GetExtents() const
{
    BoxExtents *ext = (BoxExtents *)atts;
    return ext->GetExtents();
}

