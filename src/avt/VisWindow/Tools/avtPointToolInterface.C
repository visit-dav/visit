// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <avtPointToolInterface.h>
#include <PointAttributes.h>
#include <VisWindow.h>

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
    p->SetPoint(d);
}

const double *
avtPointToolInterface::GetPoint() const
{
    PointAttributes *p = (PointAttributes *)atts;
    return p->GetPoint();
}
