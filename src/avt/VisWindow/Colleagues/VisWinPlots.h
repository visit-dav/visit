// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              VisWinPlots.h                                //
// ************************************************************************* //

#ifndef VIS_WIN_PLOTS_H
#define VIS_WIN_PLOTS_H

#include <viswindow_exports.h>

#include <vector>

#include <avtDataset.h>
#include <VisCallback.h>
#include <VisWinColleague.h>


class vtkActor;
class vtkOutlineSource;
class vtkPolyDataMapper;
class vtkScalarBarActor;
class vtkCamera;

class avtExternallyRenderedImagesActor;
class avtTransparencyActor;


// ****************************************************************************
//  Class: VisWinPlots
//
//  Purpose:
//      Handles the plots (actors) that are added to the VisWindow.  This
//      includes adding and positioning scalar bars and making sure that
//      2D and 3D plots are never in the same window.
//
//  Programmer: Hank Childs
//  Creation:   May 19, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Jul  5 15:11:23 PDT 2000
//    Added a picker to the object and changed the interface so a picker is
//    no longer passed in.
//
//    Hank Childs, Thu Jul  6 12:59:26 PDT 2000
//    Added ability to clear all the actors.
//
//    Hank Childs, Wed Aug  2 16:32:08 PDT 2000
//    Add GetBounds routine.
//
//    Hank Childs, Sun Aug  6 13:49:42 PDT 2000
//    Added SetBounds, UnsetBounds, AdjustCamera, and SetCamera as well
//    as supporting data members, usetSetCamera, currentBounds, and setBounds.
//
//    Hank Childs, Thu Sep 28 21:20:38 PDT 2000
//    Add avtPlots.
//
//    Hank Childs, Thu Dec 28 10:27:44 PST 2000
//    Replace avtPlot with avtActor and remove hooks for vtkActor.
//
//    Kathleen Bonnell, Tue Apr  3 15:27:13 PDT 2001 
//    Added method OrderPlots. 
//
//    Kathleen Bonnell, Tue Sep  4 14:07:15 PDT 2001
//    Made bbox use vtkOutlineSource instead of vtkRectlinearGrid in
//    wireframe mode. 
//
//    Eric Brugger, Mon Oct 22 09:37:26 PDT 2001
//    I added the protected member bboxMode to keep track of the bounding
//    box mode.
//
//    Kathleen Bonnell, Wed Nov 28 08:25:55 PST 2001
//    Added methods/members in support of pick operation. 
//
//    Hank Childs, Thu Mar 14 18:22:59 PST 2002
//    Removed legend support and put it in (new colleague) VisWinLegends.
//
//    Kathleen Bonnell, Fri Mar 15 14:10:49 PST 2002 
//    Moved Pick related operations to VisWinQuery.
//
//    Kathleen Bonnell, Wed May  8 14:06:50 PDT 2002  
//    Added method to retrieve data range. 
//
//    Hank Childs, Sun Jul  7 12:48:30 PDT 2002
//    Added support for transparency.
//
//    Hank Childs, Thu Jul 11 17:51:34 PDT 2002
//    Added support for motion tracking (so transparency can be perfectly
//    sorted after rotating).
//
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002 
//    Added method for scaling a plot's actors.
//    
//    Hank Childs, Mon Jul 15 11:22:59 PDT 2002
//    Added support for getting bounds based on different flavor of extents.
//
//    Jeremy Meredith, Fri Jul 26 14:26:07 PDT 2002
//    Added MotionBegin to better keep track of transparency needs.
//
//    Kathleen Bonnell, Fri Jul 26 15:01:55 PDT 2002   
//    Added UpdateScaleFactor. 
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002   
//    Added TurnLightingOn, TurnLightingOff, and SetAmbientCoefficient. 
//
//    Brad Whitlock, Mon Sep 23 15:47:19 PST 2002
//    Added SetSurfaceRepresentation and SetImmediateModeRendering.
//
//    Mark C. Miller, Thu Dec 19 2002
//    Added externally rendered images actor data member
//
//    Mark C. Miller, Mon Jan 13 22:54:26 PST 2003
//    Added method to forward request to register an external rendering
//    callback to the externally rendered images actor
//
//    Kathleen Bonnell, Fri Jun  6 15:23:05 PDT 2003  
//    Added FullFrameOn/Off methods. 
//    
//    Kathleen Bonnell, Mon Sep 29 13:21:12 PDT 2003
//    Added bool arg to OrderPlots. 
//    
//    Jeremy Meredith, Fri Nov 14 17:15:21 PST 2003
//    Added specular properties.
//
//    Kathleen Bonnell, Wed Dec  3 16:48:23 PST 2003 
//    Added method TransparenciesExist.
//
//    Chris Wojtan, Mon Jul 26 16:22:56 PDT 2004
//    Added 4 functions for suspending and resuming opaque and
//    translucent geometry.
//
//    Kathleen Bonnell, Thu Nov  4 16:46:31 PST 2004 
//    Added MakeAllPickable, MakeAllUnPickable. 
//
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Removed bool for antialiasing arg from OrderPlots
//
//    Kathleen Bonnell, Mon Jun 27 14:54:36 PDT 2005 
//    Added GetMaxZShift.
//
//    Mark Blair, Wed Aug 30 14:19:00 PDT 2006
//    Added GetPlotListIndex.
//
//    Brad Whitlock, Mon Sep 18 11:13:40 PDT 2006
//    Added SetColorTexturingFlag.
//
//    Mark Blair, Wed Oct 25 15:12:55 PDT 2006
//    Added GetPlotInfoAtts.
//
//    Mark C. Miller, Wed Mar 28 15:56:15 PDT 2007
//    Added IsMakingExternalRenderRequests, GetAverageExternalRenderingTime
//    and DoNextExternalRenderAsVisualQueue to support the 'in-progress'
//    visual queue for SR mode.
//
//    Brad Whitlock, Wed Jan  7 14:38:44 PST 2009
//    I changed the plot info atts method.
//
//    Tom Fogal, Mon May 25 18:20:46 MDT 2009
//    Added GetTransparencyActor method.
//
//    Jeremy Meredith, Wed May 19 14:15:58 EDT 2010
//    Account for 3D axis scaling (3D equivalent of full-frame mode).
//
//    Burlen Loring, Tue Aug 18 11:25:24 PDT 2015
//    Added api to retreive the camera.
//
//    Kathleen Biagas, Thu Aug  2 13:03:41 MST 2018
//    Removed AdjustCamera, an ancient code path.
//
// ****************************************************************************

class VISWINDOW_API VisWinPlots : public VisWinColleague
{
  public:
                                  VisWinPlots(VisWindowColleagueProxy &);
    virtual                      ~VisWinPlots();

    void                          AddPlot(avtActor_p &);
    void                          RemovePlot(avtActor_p &);

    void                          ClearPlots(void);
    void                          OrderPlots();
    
    void                          GetBounds(double [6]);
    void                          GetDataRange(double &, double &);
    void                          SetViewExtentsType(avtExtentType);

    void                          EndBoundingBox(void);
    void                          StartBoundingBox(void);

    virtual void                  SetForegroundColor(double, double, double);
    virtual void                  UpdateView(void);

    virtual void                  Start3DMode(void);
    virtual void                  Stop3DMode(void);
    virtual void                  Start2DMode(void);
    virtual void                  Stop2DMode(void);

    virtual void                  MotionBegin(void);
    virtual void                  MotionEnd(void);

    void                          SetBounds(const double [6]);
    void                          UnsetBounds(void);

    void                          TriggerPlotListUpdate(void);

    avtDataset_p                  GetAllDatasets(void);

    void                          ScalePlots(const double [3]);
    void                          TurnLightingOn(void);
    void                          TurnLightingOff(void);
    void                          SetAmbientCoefficient(const double);

    virtual void                  SetSurfaceRepresentation(int rep);

    virtual void                  SetSpecularProperties(bool,double,double,
                                                        const ColorAttribute&);
    virtual void                  SetColorTexturingFlag(bool);

    void                          SetExternalRenderCallback(
                                      VisCallbackWithDob *cb, void *data);
    bool                          EnableExternalRenderRequests(void);
    bool                          DisableExternalRenderRequests(bool bClearImage = false);
    bool                          IsMakingExternalRenderRequests(void) const;
    double                        GetAverageExternalRenderingTime(void) const;
    void                          DoNextExternalRenderAsVisualQueue(int w, int h,
                                      const double *color);

    virtual void                  FullFrameOn(const double, const int);
    virtual void                  FullFrameOff(void);
    virtual void                  Set3DAxisScalingFactors(bool scale,
                                                          const double s[3]);
    bool                          DoAllPlotsAxesHaveSameUnits();

    bool                          TransparenciesExist(void);
    avtTransparencyActor*         GetTransparencyActor();
    vtkCamera*                    GetCamera();

    void                          SuspendOpaqueGeometry(void);
    void                          SuspendTranslucentGeometry(void);
    void                          ResumeOpaqueGeometry(void);
    void                          ResumeTranslucentGeometry(void);

    void                          MakeAllPickable(void);
    void                          MakeAllUnPickable(void);
    double                         GetMaxZShift(void);

  protected:
    std::vector< avtActor_p >     plots;
    avtTransparencyActor         *transparencyActor;
    avtExternallyRenderedImagesActor *extRenderedImagesActor;

    vtkOutlineSource             *bboxGrid;
    vtkPolyDataMapper            *bboxMapper;
    vtkActor                     *bbox;
    bool                          bboxMode;
    avtExtentType                 spatialExtentType;

    bool                          userSetBounds;
    double                         currentBounds[6];
    double                         setBounds[6];

    bool                          sceneHasChanged;

    void                          CheckPlot(avtActor_p &);
    void                          GetRealBounds(double [6]);
    void                          SetBoundingBox(double *);
    void                          ShiftPlots(double [3]);
    void                          UpdateScaleFactor(void);
};

#endif
