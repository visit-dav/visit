#include <avtPointToolInterface.h>
#include <PointAttributes.h>

avtPointToolInterface::avtPointToolInterface(const VisWindow *v) :
    avtToolInterface(v)
{
    atts = new PointAttributes;
}

avtPointToolInterface::~avtPointToolInterface()
{
    // nothing
}

void
avtPointToolInterface::SetPoint(double x, double y, double z)
{
    PointAttributes *p = (PointAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetPoint(d);
}

const double *
avtPointToolInterface::GetPoint() const
{
    PointAttributes *p = (PointAttributes *)atts;
    return p->GetPoint();
}
