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

// ************************************************************************* //
//                            avtCurveLegend.h                               //
// ************************************************************************* //

#ifndef AVT_CURVE_LEGEND_H
#define AVT_CURVE_LEGEND_H
#include <plotter_exports.h>


#include <avtLegend.h>
#include <LineAttributes.h>

class  vtkLineLegend;


// ****************************************************************************
//  Class: avtCurveLegend
//
//  Purpose:
//    The legend for curve based plots.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 25, 2005
//
//  Modifications:
//    Brad Whitlock, Thu Mar 22 00:03:26 PDT 2007
//    Added methods for controlling some more properties.
//
// ****************************************************************************

class PLOTTER_API avtCurveLegend : public avtLegend
{
  public:
                               avtCurveLegend();
    virtual                   ~avtCurveLegend();

    virtual void               GetLegendSize(double, double &, double &);

    virtual void               SetLabelVisibility(bool);
    virtual bool               GetLabelVisibility() const;

    virtual void               SetLegendScale(double xScale, double yScale);
    virtual void               SetBoundingBoxVisibility(bool);
    virtual void               SetBoundingBoxColor(const double *);
    virtual void               SetFont(int family, bool bold, bool italic, bool shadow);

    void                       SetLineWidth(_LineWidth lw);
    void                       SetLineStyle(_LineStyle ls);
    void                       SetColor(const double[3]);
    void                       SetColor(double, double, double);

  protected:
    vtkLineLegend             *lineLegend;
    double                     scale[2];
    bool                       labelVisibility;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


