// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            VisWinAxes3D.h                                 //
// ************************************************************************* //

#ifndef VIS_WIN_AXES3D_H
#define VIS_WIN_AXES3D_H
#include <viswindow_exports.h>


#include <VisWinColleague.h>


class vtkVisItCubeAxesActor;
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
//    more appropriate location of vtkVisItCubeAxesActor
//
//    Kathleen Bonnell, Wed May 28 15:52:32 PDT 2003
//    Added method 'ReAddToWindow'.
//
//    Kathleen Bonnell, Tue Dec 16 11:34:33 PST 2003
//    Added method 'SetLabelScaling'.
//
//    Brad Whitlock, Thu Jul 28 10:10:40 PDT 2005
//    Added methods to set the units and title.
//
//    Brad Whitlock, Tue Mar 25 16:27:40 PDT 2008
//    Added methods for line width, font, font size.
//
//    Eric Brugger, Wed Oct 15 13:05:33 PDT 2008
//    Added SetAutoSetTicks, SetMajorTickMinimum, SetMajorTickMaximum,
//    SetMajorTickSpacing and SetMinorTickSpacing.
//
//    Jeremy Meredith, Wed May  5 14:32:23 EDT 2010
//    Added support for title visibility separate from label visibility.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//    Hank Childs, Mon May 23 13:26:09 PDT 2011
//    Add method for overriding bounding box location.
//
//    Burlen Loring, Wed Oct 21 15:23:16 PDT 2015
//    I added a get method to query actor visibility.
//
//    Kathleen Biagas, Fri Mar 25, 2022
//    Added currentScaleFlag. Useful for knowing when it is turned off so
//    the bounding box can be redone.
//
// ****************************************************************************

class VISWINDOW_API VisWinAxes3D : public VisWinColleague
{
  public:
                              VisWinAxes3D(VisWindowColleagueProxy &);
    virtual                  ~VisWinAxes3D();

    virtual void              SetForegroundColor(double, double, double);
    virtual void              UpdateView(void);
    virtual void              UpdatePlotList(std::vector<avtActor_p> &);

    virtual void              Start3DMode(void);
    virtual void              Stop3DMode(void);

    virtual void              HasPlots(void);
    virtual void              NoPlots(void);
    virtual void              ReAddToWindow(void);

    void                      SetBBoxLocation(bool, const double *);
    void                      SetBounds(double [6], double scales[3]);
    void                      SetXTickVisibility(int, int);
    void                      SetYTickVisibility(int, int);
    void                      SetZTickVisibility(int, int);
    void                      SetXLabelVisibility(int);
    void                      SetYLabelVisibility(int);
    void                      SetZLabelVisibility(int);
    void                      SetXTitleVisibility(int);
    void                      SetYTitleVisibility(int);
    void                      SetZTitleVisibility(int);
    void                      SetXGridVisibility(int);
    void                      SetYGridVisibility(int);
    void                      SetZGridVisibility(int);
    void                      SetVisibility(int);
    int                       GetVisibility(){ return visibility; }
    void                      SetAutoSetTicks(int);
    void                      SetMajorTickMinimum(double, double, double);
    void                      SetMajorTickMaximum(double, double, double);
    void                      SetMajorTickSpacing(double, double, double);
    void                      SetMinorTickSpacing(double, double, double);
    void                      SetBBoxVisibility(int);
    void                      SetFlyMode(int);
    void                      SetTickLocation(int);
    void                      SetLabelScaling(bool, int, int, int);

    void                      SetXTitle(const std::string &, bool);
    void                      SetXUnits(const std::string &, bool);
    void                      SetYTitle(const std::string &, bool);
    void                      SetYUnits(const std::string &, bool);
    void                      SetZTitle(const std::string &, bool);
    void                      SetZUnits(const std::string &, bool);

    void                      SetLineWidth(int);
    void                      SetTitleTextAttributes(
                                  const VisWinTextAttributes &xAxis,
                                  const VisWinTextAttributes &yAxis,
                                  const VisWinTextAttributes &zAxis);
    void                      SetLabelTextAttributes(
                                  const VisWinTextAttributes &xAxis,
                                  const VisWinTextAttributes &yAxis,
                                  const VisWinTextAttributes &zAxis);

    void                      Set3DAxisScalingFactors(bool scale,
                                                      const double s[3]);

    bool                      GetBoundsOverridden() const;
    void                      GetOverrideBounds( double *bounds ) const;
 
  protected:
    void UpdateTitleTextAttributes(double fr, double fg, double fb);
    void UpdateLabelTextAttributes(double fr, double fg, double fb);

    vtkVisItCubeAxesActor    *axes;
    vtkOutlineSource         *axesBoxSource;
    vtkPolyDataMapper        *axesBoxMapper;
    vtkActor                 *axesBox;

    bool                      addedAxes3D;
    double                    currentBounds[6];
    bool                      currentScaleFlag;
    double                    currentScaleFactors[3];
    bool                      visibility;

    bool                      boundsOverridden;
    double                    overrideBounds[6];

    std::string               userXTitle;
    std::string               userYTitle;
    std::string               userZTitle;
    std::string               userXUnits;
    std::string               userYUnits;
    std::string               userZUnits;
    bool                      userXTitleFlag;
    bool                      userYTitleFlag;
    bool                      userZTitleFlag;
    bool                      userXUnitsFlag;
    bool                      userYUnitsFlag;
    bool                      userZUnitsFlag;

    VisWinTextAttributes      titleTextAttributes[3];
    VisWinTextAttributes      labelTextAttributes[3];

    void                      AddAxes3DToWindow(void);
    void                      RemoveAxes3DFromWindow(void);
    bool                      ShouldAddAxes3D(void);
};
#endif
