#include <avtLineToolInterface.h>
#include <Line.h>

avtLineToolInterface::avtLineToolInterface(const VisWindow *v) :
    avtToolInterface(v)
{
    atts = new Line;
}

avtLineToolInterface::~avtLineToolInterface()
{
    // nothing
}

void
avtLineToolInterface::SetPoint1(double x, double y, double z)
{
    Line *p = (Line *)atts;
    double d[3] = {x, y, z};
    p->SetPoint1(d);
}

void
avtLineToolInterface::SetPoint2(double x, double y, double z)
{
    Line *p = (Line *)atts;
    double d[3] = {x, y, z};
    p->SetPoint2(d);
}

const double *
avtLineToolInterface::GetPoint1() const
{
    Line *p = (Line *)atts;
    return p->GetPoint1();
}

const double *
avtLineToolInterface::GetPoint2() const
{
    Line *p = (Line *)atts;
    return p->GetPoint2();
}
