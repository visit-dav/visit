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

#include <avtExtentsToolInterface.h>
#include <ExtentsAttributes.h>


// ****************************************************************************
// Class: avtExtentsToolInterface
//
// Purpose: This class contains the information passed to users of the
//          extents tool.
//
// Programmer: Mark Blair
// Creation:   Mon Oct 31 16:11:00 PST 2005
//
// Modifications:
//
//     Mark Blair, Thu Sep 14 16:44:17 PDT 2006
//     Added methods to support slider change time ordinal lists.
//   
//     Mark Blair, Thu Nov  2 12:33:23 PST 2006
//     Added methods to support selective axis labeling in associated plot.
//
// ****************************************************************************


avtExtentsToolInterface::avtExtentsToolInterface(const VisWindow *v) : avtToolInterface(v)
{
    atts = new ExtentsAttributes;
}


avtExtentsToolInterface::~avtExtentsToolInterface()
{
    // nothing
}


void avtExtentsToolInterface::SetScalarNames(const stringVector &scalarNames_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarNames(scalarNames_);
}


void avtExtentsToolInterface::SetScalarMinima(const doubleVector &scalarMinima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarMinima(scalarMinima_);
}


void avtExtentsToolInterface::SetScalarMaxima(const doubleVector &scalarMaxima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetScalarMaxima(scalarMaxima_);
}


void avtExtentsToolInterface::SetMinima(const doubleVector &minima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMinima(minima_);
}


void avtExtentsToolInterface::SetMaxima(const doubleVector &maxima_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMaxima(maxima_);
}


void avtExtentsToolInterface::SetMinTimeOrdinals(const intVector &minTimeOrdinals_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMinTimeOrdinals(minTimeOrdinals_);
}


void avtExtentsToolInterface::SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetMaxTimeOrdinals(maxTimeOrdinals_);
}


void avtExtentsToolInterface::SetToolDrawsAxisLabels(bool toolDrawsAxisLabels_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetToolDrawsAxisLabels(toolDrawsAxisLabels_);
}


void avtExtentsToolInterface::SetAxisGroupNames(const stringVector &axisGroupNames_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetAxisGroupNames(axisGroupNames_);
}


void avtExtentsToolInterface::SetAxisLabelStates(const intVector &axisLabelStates_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetAxisLabelStates(axisLabelStates_);
}


void avtExtentsToolInterface::SetAxisXIntervals(const doubleVector &axisXIntervals_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetAxisXIntervals(axisXIntervals_);
}


void avtExtentsToolInterface::SetLeftSliderX(const double leftSliderX_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetLeftSliderX(leftSliderX_);
}


void avtExtentsToolInterface::SetRightSliderX(const double rightSliderX_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetRightSliderX(rightSliderX_);
}


void avtExtentsToolInterface::SetSlidersBottomY(const double slidersBottomY_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetSlidersBottomY(slidersBottomY_);
}


void avtExtentsToolInterface::SetSlidersTopY(const double slidersTopY_)
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    extAtts->SetSlidersTopY(slidersTopY_);
}


const stringVector &avtExtentsToolInterface::GetScalarNames() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarNames();
}


const doubleVector &avtExtentsToolInterface::GetScalarMinima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarMinima();
}


const doubleVector &avtExtentsToolInterface::GetScalarMaxima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetScalarMaxima();
}


const doubleVector &avtExtentsToolInterface::GetMinima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMinima();
}


const doubleVector &avtExtentsToolInterface::GetMaxima() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMaxima();
}


const intVector &avtExtentsToolInterface::GetMinTimeOrdinals() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMinTimeOrdinals();
}


const intVector &avtExtentsToolInterface::GetMaxTimeOrdinals() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetMaxTimeOrdinals();
}


bool avtExtentsToolInterface::GetToolDrawsAxisLabels() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetToolDrawsAxisLabels();
}


const stringVector &avtExtentsToolInterface::GetAxisGroupNames() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetAxisGroupNames();
}


const intVector &avtExtentsToolInterface::GetAxisLabelStates() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetAxisLabelStates();
}


const doubleVector &avtExtentsToolInterface::GetAxisXIntervals() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetAxisXIntervals();
}


double avtExtentsToolInterface::GetLeftSliderX() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetLeftSliderX();
}


double avtExtentsToolInterface::GetRightSliderX() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetRightSliderX();
}


double avtExtentsToolInterface::GetSlidersBottomY() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetSlidersBottomY();
}


double avtExtentsToolInterface::GetSlidersTopY() const
{
    ExtentsAttributes *extAtts = (ExtentsAttributes *)atts;
    return extAtts->GetSlidersTopY();
}
