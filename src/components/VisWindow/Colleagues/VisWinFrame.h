// ************************************************************************* //
//                               VisWinFrame.h                               //
// ************************************************************************* //

#ifndef VIS_WIN_FRAME_H
#define VIS_WIN_FRAME_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkHankAxisActor2D;


// ****************************************************************************
//  Class: VisWinFrame
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It creates
//      a frame around the viewport when the VisWindow is in 2D mode.
//
//  Programmer: Hank Childs
//  Creation:   July 7, 2000
//
//  Modifications:
//
//    Hank Childs, Tue Jul 25 13:30:59 PDT 2000
//    Added SetViewport method and put frame on background instead of canvas.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    Added methods related to user-interaction: UpdateView, SetVisibility,
//    SetTopRightTickVisibility, SetTickLocation, GetRange. 
//
//    Eric Brugger, Tue Nov  6 12:38:05 PST 2002
//    Added methods SetAutoSetTicks, SetMajorTickMinimum, SetMajorTickMaximum,
//    SetMajorTickSpacing and SetMinorTickSpacing.
//    
//    Eric Brugger, Wed Jun 25 15:36:16 PDT 2003
//    I added SetLineWidth.
//
// ****************************************************************************

class VISWINDOW_API VisWinFrame : public VisWinColleague
{
  public:
                                 VisWinFrame(VisWindowColleagueProxy &);
    virtual                     ~VisWinFrame();

    virtual void                 SetForegroundColor(float, float, float);
    virtual void                 SetViewport(float, float, float, float);
    virtual void                 UpdateView(void);

    virtual void                 Start2DMode(void);
    virtual void                 Stop2DMode(void);

    virtual void                 HasPlots(void);
    virtual void                 NoPlots(void);

    void                         SetVisibility(int);
    void                         SetTopRightTickVisibility(int);
    void                         SetTickLocation(int);
    void                         SetAutoSetTicks(int);
    void                         SetMajorTickMinimum(double, double);
    void                         SetMajorTickMaximum(double, double);
    void                         SetMajorTickSpacing(double, double);
    void                         SetMinorTickSpacing(double, double);
    void                         SetLineWidth(int);

  protected:
    vtkHankAxisActor2D          *leftBorder, *rightBorder, *topBorder,
                                *bottomBorder;

    bool                         addedFrame;

    void                         AddFrameToWindow(void);
    void                         RemoveFrameFromWindow(void);
    void                         GetRange(float &, float &, float &, float &); 
    bool                         ShouldAddFrame(void);
};


#endif


