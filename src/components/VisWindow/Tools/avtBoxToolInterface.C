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

