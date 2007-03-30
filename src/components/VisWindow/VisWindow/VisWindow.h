// ************************************************************************* //
//                              VisWindow.h                                  //
// ************************************************************************* //

#ifndef VIS_WINDOW_H
#define VIS_WINDOW_H
#include <viswindow_exports.h>


#include <vector>

#include <VisWindowColleagueProxy.h>
#include <VisWindowInteractorProxy.h>
#include <VisWindowRenderProxy.h>
#include <VisWindowTypes.h>
#include <VisCallback.h>

#include <AnnotationAttributes.h>
#include <LightList.h>

#include <avtActor.h>
#include <avtDataset.h>
#include <avtImage.h>
#include <avtToolInterface.h>
#include <avtTypes.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <avtViewCurve.h>
#include <avtViewInfo.h>
#include <PickPointInfo.h>
#include <LineoutInfo.h>

class vtkRenderer;

class Line;
class PickAttributes;
class VisWinAxes;
class VisWinAxes3D;
class VisWinBackground;
class VisWinColleague;
class VisWinFrame;
class VisWinInteractions;
class VisWinLegends;
class VisWinLighting;
class VisWinPlots;
class VisWinQuery;
class VisWinRendering;
class VisWinTools;
class VisWinTriad;
class VisWinUserInfo;
class VisWinView;


class VisitInteractor;



// ****************************************************************************
//  Class: VisWindow
//
//  Purpose:
//    VisWindow creates and manages a visualization window.  It follows the 
//    mediator/colleague model outlined in "Design Patterns".  VisWindow
//    is the mediator and it has three primary colleagues, for the 
//    interactors, plots, and rendering.  In addition, more colleagues
//    can be added like a triad, updating axes, or user information.
//
//  Note:   To allow greater access than public (but not full access) to its
//          colleagues, the VisWindow is accessed through a proxy.
//
//  Programmer: Hank Childs
//  Creation:   May 4, 2000
//
//  Modifications:
//    Hank Childs, Wed Jul  5 15:11:23 PDT 2000
//    Changed interface for picking.
//
//    Hank Childs, Thu Jul  6 10:41:09 PDT 2000
//    Added functions to enable and disable updates and set the size and
//    location of the window.  Added new colleague, VisWinFrame.
//
//    Hank Childs, Sun Aug  6 13:59:37 PDT 2000
//    Added functions SetBounds and UnsetBounds so animations can specify
//    what the initial view should be.
//
//    Hank Childs, Thu Sep 28 21:15:30 PDT 2000
//    Make VisWindow accept avtPlots.
//
//    Brad Whitlock, Fri Nov 3 13:50:35 PST 2000
//    I added callbacks to show/hide the popup menu.
//
//    Hank Childs, Wed Nov  8 11:45:13 PST 2000
//    Added new colleague, VisWinView.
//
//    Brad Whitlock, Thu Nov 9 15:39:15 PST 2000
//    Added a method to set the bounding box mode.
//
//    Hank Childs, Fri Nov 10 10:10:30 PST 2000
//    Added perspective on/off.
// 
//    Hank Childs, Thu Dec 28 10:23:53 PST 2000
//    Replaced avtPlot with avtActor.  Removed hooks for vtkActors.
//
//    Eric Brugger, Mon Mar 12 14:44:19 PST 2001
//    I removed the Get/SetPerspectiveProjection methods.
//
//    Brad Whitlock, Thu Apr 19 11:33:04 PDT 2001
//    Added methods for window iconification.
//
//    Kathleen Bonnell, 
//    Added axes3D, annotationAtts, UpdateAxes2D, UpdateAxes3D,
//    and SetAnnotatationAttributes.
//
//    Eric Brugger, Fri Aug 17 09:32:12 PDT 2001
//    I changed the way the view is handled.
//
//    Brad Whitlock, Wed Aug 22 11:56:44 PDT 2001
//    Added the SetCloseCallback method.
//
//    Brad Whitlock, Tue Aug 27 12:19:34 PDT 2001
//    Added background colleague and methods to query the bg/fg colors.
//
//    Brad Whitlock, Mon Oct 1 12:46:17 PDT 2001
//    Added a tools colleague to manage the interactive tools.
//
//    Brad Whitlock, Fri Oct 19 22:27:20 PST 2001
//    Added set/get methods for a light list.
//
//    Eric Brugger, Fri Nov  2 14:02:47 PST 2001
//    I added a const qualifier to the argument to SetAnnotationAtts.
//
//    Brad Whitlock, Fri Nov 2 10:52:44 PDT 2001
//    Added methods to return the window size, location.
//
//    Brad Whitlock, Wed Sep 19 16:10:18 PST 2001
//    Added a method to query the enabled state.
//
//    Kathleen Bonnell, Fri Nov 9 14:54:20 PST 2001 
//    Added Pick. 
//
//    Brad Whitlock, Thu Jan 10 08:18:59 PDT 2002
//    Added an update method for text annotations.
//
//    Hank Childs, Fri Feb  1 09:57:52 PST 2002  
//    Added support for nowin mode.
//
//    Brad Whitlock, Mon Feb 11 14:30:14 PST 2002
//    Added a method to return a tool interface.
//
//    Hank Childs, Tue Mar 12 18:29:46 PST 2002
//    Added VisWinLegends.
//
//    Kathleen Bonnell, Fri Mar 15 14:16:28 PST 2002
//    Added VisWinQuery.
//
//    Kathleen Bonnell, Fri May 10 15:38:14 PDT 2002 
//    Added support for Lineout, avtViewCurve.. 
//
//    Sean Ahern, Mon May 20 13:29:13 PDT 2002
//    Added ability to raise/lower window.
//
//    Hank Childs, Wed May 29 09:09:45 PDT 2002
//    Added Get/SetSpin.
//
//    Kathleen Bonnell, Wed Jun 19 18:05:04 PDT 2002 
//    Added UpdateQuery, DeleteQuery. 
//
//    Hank Childs, Thu Jul 11 17:49:36 PDT 2002
//    Add support for tracking when motion begins and ends.
//
//    Kathleen Bonnell, Fri Jul 12 18:42:11 PDT 2002  
//    Add ScalePlots. 
//
//    Hank Childs, Mon Jul 15 11:03:29 PDT 2002
//    Added support for setting view based on different flavors of spatial
//    extents.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002
//    Added lighting colleague and methods to retrieve lighting coefficients.
//    Change Set/GetLightList to use LightList instead of avtLightList.  
//
//    Brad Whitlock, Thu Sep 19 14:06:13 PST 2002
//    I added methods to get/set rendering properties.
//
//    Brad Whitlock, Mon Sep 9 14:47:36 PST 2002
//    I made the pick and lineout methods public.
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002  
//    Changed argument of QueryIsValid to Line*, to convey more information
//    than just color. 
//
//    Kathleen Bonnell, Wed Dec  4 17:29:34 PST 2002   
//    Removed GetAntialiasingFrames, no longer needed. 
//    
//    Kathleen Bonnell, Thu Dec 19 13:32:47 PST 2002  
//    Added 'designator' argument to QueryIsValid. 
// 
//    Brad Whitlock, Wed Jan 29 14:32:10 PST 2003
//    I added a method to create and return a pointer to a toolbar.
//
//    Mark C. Miller, Mon Jan 13 22:15:26 PST 2003
//    Added support for external rendering callback
//
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003  
//    Replaced 'designator' argument from QueryIsValid with PickAttributes. 
//
//    Brad Whitlock, Wed Mar 12 08:43:59 PDT 2003
//    I added the IsVisible method.
//
//    Kathleen Bonnell, Thu May 15 10:00:02 PDT 2003  
//    Replaced GetCurveYScale with more generic GetScaleFactorAndType.
//
//    Kathleen Bonnell, Wed May 28 16:25:37 PDT 2003 
//    Added method ReAddColleaguesToRenderWindow.
//
//    Kathleen Bonnell,  Fri Jun  6 15:51:07 PDT 2003 
//    Added FullFrameOn/Off and GetFullFrameMode methods. 
//
// ****************************************************************************

class VISWINDOW_API VisWindow
{
    friend               class VisWindowProtectionProxy;
public:
                         VisWindow(bool = false);
    virtual             ~VisWindow();

    void                 AddPlot(avtActor_p &);
    void                 RemovePlot(avtActor_p &);
    void                 ClearPlots(void);

    void                 SetBounds(const float [6]);
    void                 UnsetBounds(void);

    void                 Realize(void);
    avtImage_p           ScreenCapture(bool doZBufferToo = false);
    avtDataset_p         GetAllDatasets(void);

    void                 SetSize(int, int);
    void                 GetSize(int &, int &) const;
    void                 SetLocation(int, int);
    void                 GetLocation(int &, int &) const;

    void                 EnableUpdates(void);
    void                 DisableUpdates(void);
    bool                 UpdatesEnabled() const;

    bool                 GetHotPoint(int, int, HotPoint &) const;
    void                 SetHighlightEnabled(bool);

    const char          *GetToolName(int i) const;
    int                  GetNumTools() const;
    void                 SetToolEnabled(int i, bool val);
    bool                 GetToolEnabled(int i) const;
    bool                 GetToolAvailable(int i) const;
    avtToolInterface    &GetToolInterface(int i) const;
    void                 UpdateTool(int i, bool redraw);

    void                 Iconify();
    void                 DeIconify();
    void                 Show();
    void                 Hide();
    void                 Raise();
    void                 Lower();
    bool                 GetRealized();
    bool                 IsVisible() const;

    void                 SetInteractionMode(INTERACTION_MODE m);
    INTERACTION_MODE     GetInteractionMode() const;

    void                 SetBackgroundColor(float, float, float);
    const float *        GetBackgroundColor() const;
    void                 SetGradientBackgroundColors(int, float, float, float,
                                                     float, float, float);
    void                 SetBackgroundMode(int);
    int                  GetBackgroundMode() const;
    void                 SetForegroundColor(float, float, float);
    const float *        GetForegroundColor() const;
    void                 InvertBackgroundColor();
    void                 SetViewport(float, float, float, float);

    void                 SetTitle(const char *);

    void                 SetBoundingBoxMode(bool);
    bool                 GetBoundingBoxMode() const;
    void                 SetSpinMode(bool);
    bool                 GetSpinMode() const;
    void                 SetViewExtentsType(avtExtentType);

    void                 SetTypeIsCurve(bool);
    bool                 GetTypeIsCurve() const;

    void                 ResetView(void);
    void                 SetView2D(const avtView2D &);
    const avtView2D     &GetView2D();
    void                 SetView3D(const avtView3D &);
    const avtView3D     &GetView3D(void);
    void                 SetViewCurve(const avtViewCurve &);
    const avtViewCurve  &GetViewCurve();

    void                 GetScaleFactorAndType(double &s, int &t);
    bool                 GetFullFrameMode(void);
    void                 FullFrameOn(const double, const int);
    void                 FullFrameOff(void);

    void                 ShowMenu(void);
    void                 HideMenu(void);
    void                 SetShowMenu(VisCallback *cb, void *data);
    void                 SetHideMenu(VisCallback *cb, void *data);
    void                 SetCloseCallback(VisCallback *cb, void *data);
    void                 SetHideCallback(VisCallback *cb, void *data);
    void                 SetShowCallback(VisCallback *cb, void *data);
    void                 SetPickCB(VisCallback *cb, void *data);
    void                 SetLineoutCB(VisCallback *cb, void *data);

    void                 SetAnnotationAtts(const AnnotationAttributes *);
    const AnnotationAttributes *GetAnnotationAtts() const;

    void                 SetLightList(const LightList *);
    const LightList     *GetLightList() const;

    void                 Render(void);

    void                 UpdateView();
    void                 ClearPickPoints();
    void                 ClearRefLines();
    void                 DrawRefLines(float pt1[3], float pt2[3]);
    QUERY_TYPE           GetQueryType() const;

    void                 QueryIsValid(const PickAttributes *, const Line *);
    void                 UpdateQuery(const Line *);
    void                 DeleteQuery(const Line *);
    void                 ScalePlots(const float [3]);

    void                 Pick(int, int);
    void                 Lineout(int, int, int, int);

    void                *CreateToolbar(const char *name);


    // external rendering controls
    void                 SetExternalRenderCallback(
                             VisCallbackWithDob *cb, void *data);
    bool                 EnableExternalRenderRequests(void);
    bool                 DisableExternalRenderRequests(void);

    // Rendering options.
    void                 SetRenderInfoCallback(VisCallback *cb, void *data);
    void                 SetAntialiasing(bool enabled);
    bool                 GetAntialiasing() const;
    void                 GetRenderTimes(float times[3]) const;
    void                 SetStereoRendering(bool enabled, int type);
    bool                 GetStereo() const;
    int                  GetStereoType() const;
    void                 SetImmediateModeRendering(bool mode);
    bool                 GetImmediateModeRendering() const;
    void                 SetSurfaceRepresentation(int rep);
    int                  GetSurfaceRepresentation() const;
    int                  GetNumTriangles() const;
    void                 SetNotifyForEachRender(bool val);
    bool                 GetNotifyForEachRender() const;
    bool                 GetScalableRendering() const;
    void                 SetScalableRendering(bool mode);
    int                  GetScalableThreshold() const;
    void                 SetScalableThreshold(int threshold);


protected:
    VisWindowColleagueProxy            colleagueProxy;
    VisWindowInteractorProxy           interactorProxy;
    VisWindowRenderProxy               renderProxy;

    std::vector< VisWinColleague * >   colleagues;

    VisWinBackground                  *windowBackground;
    VisWinAxes                        *axes;
    VisWinAxes3D                      *axes3D;
    VisWinFrame                       *frame;
    VisWinInteractions                *interactions;
    VisWinLegends                     *legends;
    VisWinLighting                    *lighting;
    VisWinPlots                       *plots;
    VisWinQuery                       *queries;
    VisWinRendering                   *rendering;
    VisWinTools                       *tools;
    VisWinTriad                       *triad;
    VisWinUserInfo                    *userInfo;
    VisWinView                        *view;

    AnnotationAttributes               annotationAtts;
    LightList                          lightList;

    float                              foreground[3];
    float                              background[3];
    float                              gradientBackground[2][3];
    int                                gradientBackgroundStyle;
    int                                backgroundMode;
    float                              viewportLeft, viewportRight,
                                       viewportBottom, viewportTop;

    WINDOW_MODE                        mode;
    bool                               updatesEnabled;
    bool                               hasPlots;
    bool                               typeIsCurve;

    avtView2D                          view2D;
    avtView3D                          view3D;
    avtViewCurve                       viewCurve;

    VisCallback                       *showMenuCallback;
    void                              *showMenuCallbackData;
    VisCallback                       *hideMenuCallback;
    void                              *hideMenuCallbackData;

    VisCallback                       *performPickCallback;
    PICK_POINT_INFO                   *ppInfo;

    VisCallback                       *performLineoutCallback;
    LINE_OUT_INFO                     *loInfo;

    void                 ChangeMode(WINDOW_MODE);
    void                 Start2DMode();
    void                 Start3DMode();
    void                 StartCurveMode();
    void                 Stop2DMode();
    void                 Stop3DMode();
    void                 StopCurveMode();
    void                 UpdatePlotList(std::vector<avtActor_p> &);

    void                 HasPlots(bool);
    void                 HasPlots();
    void                 NoPlots();

    void                 SetInteractor(VisitInteractor *);
    void                 GetViewport(float *);
    void                 GetForegroundColor(float *);

    void                 AddColleague(VisWinColleague *);

    void                 StartRender(void);
    void                 RecalculateRenderOrder(void);

    vtkRenderer         *GetCanvas(void);
    vtkRenderer         *GetBackground(void);
    vtkRenderer         *GetForeground(void);

    void                 GetBounds(float [6]);
    void                 StartBoundingBox(void);
    void                 EndBoundingBox(void);
    void                 UpdateAxes2D(void);
    void                 UpdateAxes3D(void);
    void                 UpdateTextAnnotations(void);

    void                 MotionBegin(void);
    void                 MotionEnd(void);

    bool                 GetAmbientOn(void);
    float                GetAmbientCoefficient(void);
    bool                 GetLighting(void);
    void                 UpdateLightPositions();

    float                ComputeVectorTextScaleFactor(const float *pos,
                                                      const float *vp = NULL); 

    static void          ProcessResizeEvent(void *);
    void                 ReAddColleaguesToRenderWindow(void);
};


#endif
