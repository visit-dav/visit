/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
