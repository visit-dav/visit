/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                             avtCurveLegend.C                              //
// ************************************************************************* //

#include <avtCurveLegend.h>

#include <vtkLineLegend.h>
#include <vtkProperty2D.h>


// ****************************************************************************
//  Method: avtCurveLegend constructor
//
//  Programmer:  Kathleen Bonnell
//  Creation:    October 25, 2005
//
//  Modifications:
//
// ****************************************************************************

avtCurveLegend::avtCurveLegend()
{
    lineLegend = vtkLineLegend::New();
    lineLegend->SetShadow(0);

    size[0] = 0.26;
    size[1] = 0.26;
    lineLegend->SetPosition2(size[0], size[1]);

    SetLegendPosition(0.05, 0.72);

    legend = lineLegend;
    legend->Register(NULL);
}


// ****************************************************************************
//  Method: avtCurveLegend destructor
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 25, 2005
//
//  Modifications:
//
// ****************************************************************************

avtCurveLegend::~avtCurveLegend()
{
    if (lineLegend != NULL)
    {
        lineLegend->Delete();
        lineLegend = NULL;
    }
}


// ****************************************************************************
//  Method: avtCurveLegend::ChangePosition
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's position.
//
//  Arguments:
//      x       The new x-position.
//      y       The new y-position.
//
//  Programmer: Kathleen Bonnell
//  Creation:   October 25, 2005
//
// ****************************************************************************

void
avtCurveLegend::ChangePosition(double x, double y)
{
    lineLegend->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    lineLegend->GetPositionCoordinate()->SetValue(x, y, 0.);
}


// ****************************************************************************
//  Method: avtCurveLegend::ChangeTitle
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's title.
//
//  Arguments:
//      t       The new title.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 25, 2005 
// 
//  Modifications:
//
// ****************************************************************************

void
avtCurveLegend::ChangeTitle(const char *t)
{
    lineLegend->SetTitle(t);
}


// ****************************************************************************
//  Method: avtCurveLegend::ChangeFontHeight
//
//  Purpose:
//      Because the base type doesn't know what kind of 2D actor we have used,
//      this is the hook that allows it to do the bookkeeping and the derived
//      type to do the actual work of changing the legend's font height.
//
//  Arguments:
//      fh      The new font height.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 25, 2005 
// 
//  Modifications:
//
// ****************************************************************************

void
avtCurveLegend::ChangeFontHeight(double fh)
{
    lineLegend->SetFontHeight(fh);
}


// ****************************************************************************
//  Method: avtCurveLegend::SetLineWidth
//
//  Purpose:
//    Sets the line width of the legend.
//
//  Arguments:
//    lw         The new line width
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 25, 2005
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveLegend::SetLineWidth(_LineWidth lw)
{
    lineLegend->GetLineProperty()->SetLineWidth(LineWidth2Int(lw));
}


// ****************************************************************************
//  Method: avtCurveLegend::SetLineStyle
//
//  Purpose:
//    Sets the line style for the legend.
//
//  Arguments:
//    ls         The new line style
//
//  Programmer:  Kathleen Bonnell
//  Creation:    October 25, 2005 
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveLegend::SetLineStyle(_LineStyle ls)
{
    lineLegend->GetLineProperty()->SetLineStipplePattern(LineStyle2StipplePattern(ls));
}


// ****************************************************************************
//  Method: avtCurveLegend::SetColor
//
//  Purpose:
//    Sets the main color of the legend.  
//
//  Arguments:
//    col         The new color.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 25, 2005 
//
// ****************************************************************************

void                  
avtCurveLegend::SetColor(const double col[3])
{
    double r = col[0];
    double g = col[1];
    double b = col[2];
    lineLegend->GetLineProperty()->SetColor(r, g, b);
}

// ****************************************************************************
//  Method: avtCurveLegend::SetColor
//
//  Purpose:
//    Sets the main color of the legend.  
//
//  Arguments:
//    r          The new value for red.
//    g          The new value for green.
//    b          The new value for blue.
//
//  Programmer:   Kathleen Bonnell 
//  Creation:     October 25, 2005 
//
// ****************************************************************************

void                  
avtCurveLegend::SetColor(double r, double g, double b)
{
    lineLegend->GetLineProperty()->SetColor(r, g, b);
}

