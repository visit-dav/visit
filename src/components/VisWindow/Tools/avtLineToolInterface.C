#include <avtLineToolInterface.h>
#include <Line.h>
#include <VisWindow.h>

avtLineToolInterface::avtLineToolInterface(const VisWindow *v) :
    avtToolInterface(v)
{
    atts = new Line;
}

avtLineToolInterface::~avtLineToolInterface()
{
    // nothing
}

// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Fri Jun  6 15:47:14 PDT 2003 
//    Added support for full-frame mode.
//
// ****************************************************************************

void
avtLineToolInterface::SetPoint1(double x, double y, double z)
{
    Line *p = (Line *)atts;
    double d[3] = {x, y, z};
    if (const_cast<VisWindow*>(visWindow)->GetFullFrameMode())
    {
        //
        // If in full-frame mode, the point was computed in the scaled 
        // full-frame space.  Reverse the scaling to get the correct point. 
        //
        double scale;
        int type;
        const_cast<VisWindow*>(visWindow)->GetScaleFactorAndType(scale, type);
        if ( type == 0 && scale > 0.)
            d[0] /= scale;
        else if ( type == 1 && scale > 0.)
            d[1] /= scale;
    }
    p->SetPoint1(d);
}


// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Fri Jun  6 15:47:14 PDT 2003 
//    Added support for full-frame mode.
//
// ****************************************************************************

void
avtLineToolInterface::SetPoint2(double x, double y, double z)
{
    Line *p = (Line *)atts;
    double d[3] = {x, y, z};
    if (const_cast<VisWindow*>(visWindow)->GetFullFrameMode())
    {
        //
        // If in full-frame mode, the point was computed in the scaled 
        // full-frame space.  Reverse the scaling to get the correct point. 
        //
        double scale;
        int type;
        const_cast<VisWindow*>(visWindow)->GetScaleFactorAndType(scale, type);
        if ( type == 0 && scale > 0.)
            d[0] /= scale;
        else if ( type == 1 && scale > 0.)
            d[1] /= scale;
    }
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
