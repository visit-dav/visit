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
// 
// ****************************************************************************

class PLOTTER_API avtCurveLegend : public avtLegend
{
  public:
                               avtCurveLegend();
    virtual                   ~avtCurveLegend();

    void                       SetLineWidth(_LineWidth lw);
    void                       SetLineStyle(_LineStyle ls);
    void                       SetColor(const double[3]);
    void                       SetColor(double, double, double);

  protected:
    vtkLineLegend              *lineLegend;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


