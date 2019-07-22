// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                VisWinAxes.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_AXES_H
#define VIS_WIN_AXES_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkAxesActor2D;

class VisWindowColleagueProxy;

// ****************************************************************************
//  Class: VisWinAxes
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It puts
//      updating axes around 2D plots.
//
//  Programmer: Hank Childs
//  Creation:   June 9, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Jul  7 10:43:43 PDT 2000
//    Added HasPlots, NoPlots, Add/RemoveAxesTo/FromWindow.
//
//    Hank Childs, Wed Jul 12 09:12:02 PDT 2000
//    Added data members and methods to adjust the precision and put the
//    numbers in scientific notation.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001
//    Added methods for user-control of axes appearance.
//
//    Kathleen Bonnell, Fri Jul  6 14:09:00 PDT 2001
//    Added SetXGridVisibility, SetYGridVisibility.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002
//    Added members xTitle and yTitle, methods StartCurveMode, StopCurveMode.
//
//    Eric Brugger, Wed Nov  6 07:58:25 PST 2002
//    Added SetTitleVisibility, SetAutoSetTicks, SetMajorTickMinimum,
//    SetMajorTickMaximum, SetMajorTickSpacing, SetMinorTickSpacing and
//    SetFontScaleFactor.
//
//    Eric Brugger, Fri Jan 24 09:03:00 PST 2003
//    Replaced SetFontScaleFactor with SetXLabelFontHeight,
//    SetYLabelFontHeight, SetXTitleFontHeight and SetYTitleFontHeight.
//
//    Eric Brugger, Fri Feb 28 11:32:39 PST 2003
//    Modified AdjustValues so that the arguments aren't passed by reference
//    because they are no longer modified.
//
//    Eric Brugger, Wed Jun 25 14:09:15 PDT 2003
//    I added SetLineWidth.
//
//    Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003
//    Added SetLabelScaling, autolabelscalng, userPowX, userPowY.
//
//    Brad Whitlock, Thu Jul 28 08:52:38 PDT 2005
//    Added the ability to specify the axis titles and units.
//
//    Kathleen Bonnell, Thu Mar 29 10:30:41 PDT 2007
//    Added AdjustLabelFormatForLogScale.
//
//    Kathleen Bonnell, Wed May  9 10:54:12 PDT 2007
//    Added bool args to AdjustLabelFormatForLogScale.
//
//    Kathleen Biagas, Wed Jan  8 14:16:04 PST 2014
//    Utilize new vtkAxisActor2D.
//
// ****************************************************************************

class VISWINDOW_API VisWinAxes : public VisWinColleague
{
  public:
                              VisWinAxes(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxes();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              UpdateView(void);
    virtual void              SetViewport(double, double, double, double);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              Start2DMode(void);
    virtual void              Stop2DMode(void);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);

    virtual void              StartCurveMode(void);
    virtual void              StopCurveMode(void);

    void                      SetXTickVisibility(int, int);
    void                      SetYTickVisibility(int, int);
    void                      SetLabelsVisibility(int, int);
    void                      SetTitleVisibility(int, int);
    void                      SetVisibility(int);
    void                      SetTickLocation(int);
    void                      SetXGridVisibility(int);
    void                      SetYGridVisibility(int);
    void                      SetAutoSetTicks(int);
    void                      SetMajorTickMinimum(double, double);
    void                      SetMajorTickMaximum(double, double);
    void                      SetMajorTickSpacing(double, double);
    void                      SetMinorTickSpacing(double, double);
    void                      SetXLabelFontHeight(double);
    void                      SetYLabelFontHeight(double);
    void                      SetXTitleFontHeight(double);
    void                      SetYTitleFontHeight(double);
    void                      SetLineWidth(int);
    void                      SetLabelScaling(bool, int, int);

    void                      SetXTitle(const std::string &, bool);
    void                      SetXUnits(const std::string &, bool);
    void                      SetYTitle(const std::string &, bool);
    void                      SetYUnits(const std::string &, bool);

    void                      SetTitleTextAttributes(
                                  const VisWinTextAttributes &xAxis,
                                  const VisWinTextAttributes &yAxis);
    void                      SetLabelTextAttributes(
                                  const VisWinTextAttributes &xAxis,
                                  const VisWinTextAttributes &yAxis);
  protected:
    vtkAxesActor2D   *axes;

    int                       lastXPow, lastYPow;
    int                       lastXAxisDigits, lastYAxisDigits;
    bool                      addedAxes;
    char                      xTitle[256];
    char                      yTitle[256];
    int                       powX;
    int                       powY;
    char                      unitsX[256];
    char                      unitsY[256];
    bool                      autolabelScaling;
    int                       userPowX;
    int                       userPowY;
    bool                      userXTitle;
    bool                      userXUnits;
    bool                      userYTitle;
    bool                      userYUnits;
    VisWinTextAttributes      titleTextAttributes[2];
    VisWinTextAttributes      labelTextAttributes[2];

    void                      AdjustValues(double, double, double, double);
    void                      AdjustRange(double, double, double, double);
    void                      GetRange(double &, double &, double &, double &);
    void                      AddAxesToWindow(void);
    void                      RemoveAxesFromWindow(void);
    bool                      ShouldAddAxes(void);
    void                      SetTitle(void);

    void AdjustLabelFormatForLogScale(double, double, double, double, bool[2]);
    void UpdateTitleTextAttributes(double fr, double fg, double fb);
    void UpdateLabelTextAttributes(double fr, double fg, double fb);
};


#endif


