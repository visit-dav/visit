// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            VisWinParallelAxes.h                           //
// ************************************************************************* //

#ifndef VIS_WIN_PARALLEL_AXES_H
#define VIS_WIN_PARALLEL_AXES_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkVisItAxisActor2D;
class VisWindowColleagueProxy;

// ****************************************************************************
//  Class: VisWinParallelAxes
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It puts
//      updating axes around plots with arrays of parallel axes, like the
//      Parallel Curves plot.
//
//  Programmer: Eric Brugger
//  Creation:   December 9, 2008
//
//  Modifications:
//    Eric Brugger, Tue Jan 20 11:33:00 PST 2009
//    I implemented autoSetTicks, labelVisibility, titleVisibility,
//    tickVisibility and setting the major and minor tick locations.
//
//    Eric Brugger, Mon Nov  5 15:19:24 PST 2012
//    I added the ability to display the parallel axes either horizontally
//    or vertically.
//
// ****************************************************************************

class VISWINDOW_API VisWinParallelAxes : public VisWinColleague
{
  public:
                              VisWinParallelAxes(VisWindowColleagueProxy &);
    virtual                  ~VisWinParallelAxes();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              UpdateView(void);
    virtual void              SetViewport(double, double, double, double);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    virtual void              StartParallelAxesMode(void);
    virtual void              StopParallelAxesMode(void);

    void                      SetTitles();

    void                      SetLabelVisibility(int);
    void                      SetTitleVisibility(int);
    void                      SetVisibility(int);
    void                      SetTickVisibility(bool,bool);
    void                      SetAutoSetTicks(int);
    void                      SetMajorTickMinimum(double);
    void                      SetMajorTickMaximum(double);
    void                      SetMajorTickSpacing(double);
    void                      SetMinorTickSpacing(double);
    void                      SetLabelFontHeight(double);
    void                      SetTitleFontHeight(double);
    void                      SetLineWidth(int);
    void                      SetLabelScaling(bool, int);
    void                      SetLabelTextAttributes(
                                  const VisWinTextAttributes &atts);
    void                      SetTitleTextAttributes(
                                  const VisWinTextAttributes &atts);

    enum Orientation {
        Horizontal,
        Vertical
    };
    void                      SetAxisOrientation(Orientation);
  protected:
    struct AxisInfo {
        vtkVisItAxisActor2D *axis;
        vtkVisItAxisActor2D *axisCap1;
        vtkVisItAxisActor2D *axisCap2;
        int lastPow;
        int lastAxisDigits;
        char units[256];
        char title[256];
        double range[2];
        double xpos;
        AxisInfo()
            : axis(NULL), lastPow(0), lastAxisDigits(3)
        {
            xpos = 0;
            units[0] = '\0';
            title[0] = '\0';
        }
        AxisInfo(vtkVisItAxisActor2D *a, int lp, int lad)
            : axis(a), lastPow(lp), lastAxisDigits(lad)
        {
            xpos = 0;
            units[0] = '\0';
            title[0] = '\0';
        }
    };
    std::vector<AxisInfo> axes;

    bool                      addedAxes;
    bool                      autolabelScaling;
    double                    vl, vr;
    double                    vb, vt;

    double                    fr, fg, fb;
    int                       axisVisibility;
    bool                      labelVisibility;
    bool                      titleVisibility;
    bool                      tickVisibility;
    bool                      tickLabelVisibility;
    bool                      autoSetTicks;
    double                    majorTickMinimum;
    double                    majorTickMaximum;
    double                    majorTickSpacing;
    double                    minorTickSpacing;
    double                    labelFontHeight;
    double                    titleFontHeight; 
    int                       lineWidth;
    VisWinTextAttributes      titleTextAttributes;
    VisWinTextAttributes      labelTextAttributes;
    int                       axisPow;
    int                       userPow;
    Orientation               axisOrientation;

    void                      SetNumberOfAxes(int);
    bool                      AdjustValues(double, double);
    void                      AdjustRange(double, double);
    void                      GetRange(double &, double &, double &, double &);
    void                      AddAxesToWindow(void);
    void                      RemoveAxesFromWindow(void);
    bool                      ShouldAddAxes(void);
    void                      UpdateLabelTextAttributes(double, double,
                                  double);
    void                      UpdateTitleTextAttributes(double, double,
                                  double);
};


#endif


