// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Kathleen Biagas, Tue Jan 27, 2026
//    Handle fudged z coordinate in 2D that enables line tool to appear on
//    top of 2D plots.
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

    // in order to get the line tool to appear on top of the 2d plot
    // we fudge the z coordinate to be 0.00001, but it should be set
    // '0.0' for LineAtts.
    if(GetVisWindow()->GetWindowMode() == WINMODE_2D)
    {
        d[2] = 0.;
    }

    p->SetPoint1(d);
}


// ****************************************************************************
//  Modifications:
//    Kathleen Bonnell, Fri Jun  6 15:47:14 PDT 2003
//    Added support for full-frame mode.
//
//    Kathleen Biagas, Tue Jan 27, 2026
//    Handle fudged z coordinate in 2D that enables line tool to appear on
//    top of 2D plots.
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

    // in order to get the line tool to appear on top of the 2d plot
    // we fudge the z coordinate to be 0.00001, but it should be set
    // '0.0' for LineAtts.
    if(GetVisWindow()->GetWindowMode() == WINMODE_2D)
    {
        d[2] = 0.;
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
