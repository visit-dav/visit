// ************************************************************************* //
//                                VisWinAxes.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_AXES_H
#define VIS_WIN_AXES_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkHankAxisActor2D;

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
// ****************************************************************************

class VISWINDOW_API VisWinAxes : public VisWinColleague
{
  public:
                              VisWinAxes(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxes();

    virtual void              SetForegroundColor(float, float, float);
    virtual void              UpdateView(void);
    virtual void              SetViewport(float, float, float, float);
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

  protected:
    vtkHankAxisActor2D       *xAxis, *yAxis;
 
    int                       lastXPow, lastYPow;
    int                       lastXAxisDigits, lastYAxisDigits;
    bool                      addedAxes;
    char                      xTitle[256];
    char                      yTitle[256];
    int                       powX;
    int                       powY;
    char                      unitsX[256];
    char                      unitsY[256];
    
    void                      AdjustValues(float, float, float, float);
    void                      AdjustRange(float, float, float, float);
    void                      GetRange(float &, float &, float &, float &);
    void                      AddAxesToWindow(void);
    void                      RemoveAxesFromWindow(void);
    bool                      ShouldAddAxes(void);
    void                      SetTitle(void);
};


#endif


