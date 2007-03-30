#include <avtPlaneToolInterface.h>
#include <PlaneAttributes.h>

avtPlaneToolInterface::avtPlaneToolInterface(const VisWindow *v) :
    avtToolInterface(v)
{
    atts = new PlaneAttributes;
}

avtPlaneToolInterface::~avtPlaneToolInterface()
{
    // nothing
}

void
avtPlaneToolInterface::SetOrigin(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetOrigin(d);
}

void
avtPlaneToolInterface::SetNormal(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetNormal(d);
}

void
avtPlaneToolInterface::SetUpAxis(double x, double y, double z)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    double d[3] = {x, y, z};
    p->SetUpAxis(d);
}

void
avtPlaneToolInterface::SetRadius(const double radius)
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    p->SetRadius(radius);
}

const double *
avtPlaneToolInterface::GetOrigin() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetOrigin();
}

const double *
avtPlaneToolInterface::GetNormal() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetNormal();
}

const double *
avtPlaneToolInterface::GetUpAxis() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetUpAxis();
}

double
avtPlaneToolInterface::GetRadius() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetRadius();
}

bool
avtPlaneToolInterface::GetHaveRadius() const
{
    PlaneAttributes *p = (PlaneAttributes *)atts;
    return p->GetHaveRadius();
}
