// ************************************************************************* //
//                            VisWinAxes3D.h                                 //
// ************************************************************************* //

#ifndef VIS_WIN_AXES3D_H
#define VIS_WIN_AXES3D_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkKatCubeAxesActor;
class vtkPolyDataMapper;
class vtkOutlineSource;

// ****************************************************************************
//  Class: VisWinAxes3D
//
//  Purpose:
//      This is a concrete colleague for the mediator VisWindow.  It places
//      a 3D axes around the bounding box for the plots.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 28, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Aug  3 14:55:59 PDT 2001
//    Changed from using a 2d cube axes actor to using a 3d version.
//
//    Kathleen Bonnell, Tue Oct 30 10:30:10 PST 2001 
//    Moved AdjustValues, AdjustRange and related members to the
//    more appropriate location of vtkKatCubeAxesActor
//
//    Kathleen Bonnell, Wed May 28 15:52:32 PDT 2003   
//    Added method 'ReAddToWindow'. 
//    
//    Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003 
//    Added method 'SetLabelScaling'.
//    
// ****************************************************************************

class VISWINDOW_API VisWinAxes3D : public VisWinColleague
{
  public:
                              VisWinAxes3D(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxes3D();

    virtual void              SetForegroundColor(float, float, float);
    virtual void              UpdateView(void);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              Start3DMode(void);
    virtual void              Stop3DMode(void);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);
    virtual void              ReAddToWindow(void);

    void                      SetBounds(float [6]);
    void                      SetXTickVisibility(int, int);
    void                      SetYTickVisibility(int, int);
    void                      SetZTickVisibility(int, int);
    void                      SetXLabelVisibility(int);
    void                      SetYLabelVisibility(int);
    void                      SetZLabelVisibility(int);
    void                      SetXGridVisibility(int);
    void                      SetYGridVisibility(int);
    void                      SetZGridVisibility(int);
    void                      SetVisibility(int);
    void                      SetBBoxVisibility(int);
    void                      SetFlyMode(int);
    void                      SetTickLocation(int);
    void                      SetLabelScaling(bool, int, int, int);

  protected:
    vtkKatCubeAxesActor      *axes;
    vtkOutlineSource         *axesBoxSource;
    vtkPolyDataMapper        *axesBoxMapper;
    vtkActor                 *axesBox;

    bool                      addedAxes3D;
    float                     currentBounds[6];
    bool                      visibility;

    void                      AddAxes3DToWindow(void);
    void                      RemoveAxes3DFromWindow(void);
    bool                      ShouldAddAxes3D(void);
};


#endif


