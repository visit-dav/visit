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

#ifndef VISIT_EXTENTS_TOOL_INTERFACE_H
#define VISIT_EXTENTS_TOOL_INTERFACE_H
#include <viswindow_exports.h>
#include <avtToolInterface.h>

#include <vectortypes.h>


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

class VISWINDOW_API avtExtentsToolInterface : public avtToolInterface
{
public:
    avtExtentsToolInterface(const VisWindow *v);
    virtual ~avtExtentsToolInterface();

    void SetScalarNames(const stringVector &scalarNames_);
    void SetScalarMinima(const doubleVector &scalarMinima_);
    void SetScalarMaxima(const doubleVector &scalarMaxima_);
    void SetMinima(const doubleVector &minima_);
    void SetMaxima(const doubleVector &maxima_);
    void SetMinTimeOrdinals(const intVector &minTimeOrdinals_);
    void SetMaxTimeOrdinals(const intVector &maxTimeOrdinals_);
    void SetToolDrawsAxisLabels(bool toolDrawsAxisLabels_);
    void SetAxisGroupNames(const stringVector &axisGroupNames_);
    void SetAxisLabelStates(const intVector &axisLabelStates_);
    void SetAxisXIntervals(const doubleVector &axisXIntervals_);
    void SetLeftSliderX(const double leftSliderX_);
    void SetRightSliderX(const double rightSliderX_);
    void SetSlidersBottomY(const double slidersBottomY_);
    void SetSlidersTopY(const double slidersTopY_);

    const stringVector &GetScalarNames() const;
    const doubleVector &GetScalarMinima() const;
    const doubleVector &GetScalarMaxima() const;
    const doubleVector &GetMinima() const;
    const doubleVector &GetMaxima() const;
    const intVector &GetMinTimeOrdinals() const;
    const intVector &GetMaxTimeOrdinals() const;
    bool GetToolDrawsAxisLabels() const;
    const stringVector &GetAxisGroupNames() const;
    const intVector &GetAxisLabelStates() const;
    const doubleVector &GetAxisXIntervals() const;
    double GetLeftSliderX() const;
    double GetRightSliderX() const;
    double GetSlidersBottomY() const;
    double GetSlidersTopY() const;
};

#endif
