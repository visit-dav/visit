// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

    virtual void               SetTitleVisibility(bool);
    virtual bool               GetTitleVisibility() const;

    virtual void               SetLegendScale(double xScale, double yScale);
    virtual void               SetBoundingBoxVisibility(bool);
    virtual void               SetBoundingBoxColor(const double *);
    virtual void               SetFont(int family, bool bold, bool italic, bool shadow);

    void                       SetLineWidth(_LineWidth lw);
    void                       SetColor(const double[3]);
    void                       SetColor(double, double, double);

  protected:
    vtkLineLegend             *lineLegend;
    double                     scale[2];
    bool                       titleVisibility;

    virtual void               ChangePosition(double, double);
    virtual void               ChangeTitle(const char *);
    virtual void               ChangeFontHeight(double);
};


#endif


