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
avtCurveLegend::ChangePosition(float x, float y)
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
avtCurveLegend::ChangeFontHeight(float fh)
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
avtCurveLegend::SetColor(const float col[3])
{
    float r = col[0];
    float g = col[1];
    float b = col[2];
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
avtCurveLegend::SetColor(float r, float g, float b)
{
    lineLegend->GetLineProperty()->SetColor(r, g, b);
}

