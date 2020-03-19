// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisWinAxesArray.h                            //
// ************************************************************************* //

#ifndef VIS_WIN_AXES_ARRAY_H
#define VIS_WIN_AXES_ARRAY_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkVisItAxisActor2D;
class VisWindowColleagueProxy;

// ****************************************************************************
//  Class: VisWinAxesArray
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It puts
//      updating axes around plots with arrays of parallel axes, like the
//      ParallelCoordinates plot.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 31, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Feb  7 17:59:55 EST 2008
//    Added support for array variables and bin-defined x positions.
//
//    Jeremy Meredith, Tue Nov 18 15:50:49 EST 2008
//    Added support for a few missing features and new font attributes.
//
//    Eric Brugger, Tue Jan 20 11:33:46 PST 2009
//    I removed SetGridVisibility since it doesn't make sense.
//
//    Burlen Loring, Wed Oct  7 16:33:07 PDT 2015
//    Fix a compiler warning
//
// ****************************************************************************

class VISWINDOW_API VisWinAxesArray : public VisWinColleague
{
  public:
                              VisWinAxesArray(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxesArray();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              UpdateView(void);
    virtual void              SetViewport(double, double, double, double);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    virtual void              StartAxisArrayMode(void);
    virtual void              StopAxisArrayMode(void);

    void                      SetTitles();

    void                      SetLabelVisibility(int);
    void                      SetTitleVisibility(int);
    void                      SetVisibility(int);
    void                      SetTickVisibility(bool,bool);
    void                      SetTickLocation(int);
    void                      SetAutoSetTicks(int);
    void                      SetMajorTickMinimum(double);
    void                      SetMajorTickMaximum(double);
    void                      SetMajorTickSpacing(double);
    void                      SetMinorTickSpacing(double);
    void                      SetLabelFontHeight(double);
    void                      SetTitleFontHeight(double);
    void                      SetLineWidth(int);
    void                      SetLabelScaling(bool, int);
    void                      SetTitleTextAttributes(
                                  const VisWinTextAttributes &atts);
    void                      SetLabelTextAttributes(
                                  const VisWinTextAttributes &atts);
  protected:
    struct AxisInfo {
        vtkVisItAxisActor2D *axis;
        int lastPow;
        int lastAxisDigits;
        int pow;
        char units[256];
        char title[256];
        double range[2];
        double xpos;
        AxisInfo()
             : axis(NULL), lastPow(0), lastAxisDigits(3), pow(0)
        {
            xpos = 0;
            units[0] = '\0';
            title[0] = '\0';
        }
        AxisInfo(vtkVisItAxisActor2D *a, int lp, int lad, int p, int up)
            : axis(a), lastPow(lp), lastAxisDigits(lad), pow(p)
        {
            (void)up;
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
    int                       tickLocation;
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
    int                       userPow;

    void                      SetNumberOfAxes(int);
    bool                      AdjustValues(int, double, double);
    void                      AdjustRange(int, double, double);
    void                      GetRange(double &, double &, double &, double &);
    void                      AddAxesToWindow(void);
    void                      RemoveAxesFromWindow(void);
    bool                      ShouldAddAxes(void);
    void UpdateTitleTextAttributes(double fr, double fg, double fb);
    void UpdateLabelTextAttributes(double fr, double fg, double fb);
};


#endif


