/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
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
//    Brad Whitlock, Thu Mar 22 00:12:00 PDT 2007
//    Initialize scale and titleVisibility.
//
// ****************************************************************************

avtCurveLegend::avtCurveLegend()
{
    lineLegend = vtkLineLegend::New();
    lineLegend->SetShadow(0);

    scale[0] = 1.;
    scale[1] = 1.;
    size[0] = 0.08;
    size[1] = 0.26;
    titleVisibility = true;

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
//  Method: avtCurveLegend::GetLegendSize
//
//  Purpose:
//      Gets the legend's size.
//
//  Arguments:
//      w        The legend's width.
//      h        The legend's height.
//
//  Programmer:  Brad Whitlock
//  Creation:    Thu Mar 22 00:12:50 PDT 2007
//
//  Modifications:
//
// ****************************************************************************

void
avtCurveLegend::GetLegendSize(double, double &w, double &h)
{
    w = 0.08 * scale[0];

    double nLines = 0.51;

    if (title != NULL)        nLines += 0.8;
    if (databaseInfo != NULL) nLines += 1.6;
    if (varName != NULL)      nLines += 0.8;
    if (varUnits != NULL)     nLines += 0.8;

    h = nLines * fontHeight * scale[1];

    size[0] = w;
    size[1] = h;
}

// ****************************************************************************
// Method: avtCurveLegend::SetTitleVisibility
//
// Purpose: 
//   Sets whether Titles are visible.
//
// Arguments:
//   val : True if Titles are to be visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtCurveLegend::SetTitleVisibility(bool val)
{
    titleVisibility = val;
    lineLegend->SetTitleVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtCurveLegend::GetTitleVisibility
//
// Purpose: 
//   Returns whether Titles are visible.
//
// Returns:    Whether Titles are visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:31:48 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
avtCurveLegend::GetTitleVisibility() const
{
    return titleVisibility;
}

// ****************************************************************************
// Method: avtCurveLegend::SetLegendScale
//
// Purpose: 
//   Set the legend scale.
//
// Arguments:
//   xScale : The scale factor in X.
//   yScale : The scale factor in Y.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:33:20 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtCurveLegend::SetLegendScale(double xScale, double yScale)
{
    // Scale the color bar
    double colorBarScale = lineLegend->GetBarWidth() / scale[0];
    colorBarScale *= xScale;
    lineLegend->SetBarWidth(colorBarScale);

    // Save the scales.
    scale[0] = xScale;
    scale[1] = yScale;
}

// ****************************************************************************
// Method: avtCurveLegend::SetBoundingBoxVisibility
//
// Purpose: 
//   Sets whether the bounding box around the legend will be visible.
//
// Arguments:
//   val : True to make the box visible.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:03 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtCurveLegend::SetBoundingBoxVisibility(bool val)
{
    lineLegend->SetBoundingBoxVisibility(val?1:0);
}

// ****************************************************************************
// Method: avtCurveLegend::SetBoundingBoxColor
//
// Purpose: 
//   Set the bounding box color.
//
// Arguments:
//   color : An rgba tuple of colors.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:34:36 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtCurveLegend::SetBoundingBoxColor(const double *color)
{
    lineLegend->SetBoundingBoxColor((double*)color);
}

// ****************************************************************************
// Method: avtCurveLegend::SetFont
//
// Purpose: 
//   Set the font properties for the legend.
//
// Arguments:
//   family : VTK_ARIAL, VTK_COURIER, VTK_TIMES
//   bold   : True to make the text bold.
//   italic : True to make the text italic.
//   shadow : True to make the text shadowed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 21:35:31 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
avtCurveLegend::SetFont(int family, bool bold, bool italic, bool shadow)
{
    lineLegend->SetFontFamily(family);
    lineLegend->SetBold(bold?1:0);
    lineLegend->SetItalic(italic?1:0);
    lineLegend->SetShadow(shadow?1:0);
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
//  Modifications:
//    Brad Whitlock, Thu Mar 22 00:15:01 PDT 2007
//    Set the position2.
//
// ****************************************************************************

void
avtCurveLegend::ChangePosition(double x, double y)
{
    lineLegend->GetPositionCoordinate()->SetCoordinateSystemToNormalizedViewport();
    lineLegend->GetPositionCoordinate()->SetValue(x, y, 0.);

    // Set the position 2, incorporating the scale.
    double tmp = 0., w, h;
    GetLegendSize(tmp, w, h);
    lineLegend->SetPosition2(w, h);  
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
//    Brad Whitlock, Thu Mar 22 00:42:12 PDT 2007
//    Take the scale into account.
//
// ****************************************************************************

void
avtCurveLegend::ChangeFontHeight(double fh)
{
    double minScale = (scale[0] < scale[1]) ? scale[0] : scale[1];
    lineLegend->SetFontHeight(fh * minScale);
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

