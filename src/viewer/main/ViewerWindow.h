// ************************************************************************* //
//                               ViewerWindow.h                              //
// ************************************************************************* //

#ifndef VIEWER_WINDOW_H
#define VIEWER_WINDOW_H
#include <viewer_exports.h>
#include <avtActor.h>
#include <avtDataset.h>
#include <avtImage.h>
#include <avtTypes.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <avtViewCurve.h>
#include <VisCallback.h>
#include <VisWindowTypes.h>
#include <AnnotationAttributes.h>
#include <WindowAttributes.h>
#include <vectortypes.h>

class DataNode;
class Line;
class PickAttributes;
class VisWindow;
class ViewerActionManager;
class ViewerAnimation;
class ViewerPopupMenu;
class ViewerToolbar;
class AnimationAttributes;
class AnnotationAttributes;
class AttributeSubject;
class AttributeSubjectMap;
class avtToolInterface;
class LightList;

struct ExternalRenderRequestInfo
{
    std::vector<const char*>              pluginIDsList;
    stringVector                          hostsList;
    intVector                             plotIdsList;
    std::vector<const AttributeSubject *> attsList;
    WindowAttributes                      winAtts;
    AnnotationAttributes                  annotAtts;
};

// ****************************************************************************
//  Class: ViewerWindow
//
//  Purpose:
//    This class contains a VisWindow, an animation and extra things
//    like the popup menu and the toolbar.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 7 09:29:55 PDT 2000
//
//  Modifications:
//    Hank Childs, Fri Nov 10 13:03:38 PST 2000
//    Add Get/SetPerspectiveProjection.
//
//    Eric Brugger, Fri Dec 22 14:17:15 PST 2000
//    I added methods for controlling the view.
//
//    Eric Brugger, Tue Dec 20 16:23:49 PST 2001
//    I added the methods AddPlot, ClearPlots, EnablePlots,
//    DisablePlots and GetViewInfo.
//
//    Eric Brugger, Fri Feb 23 12:44:59 PST 2001
//    I overloaded RecenterView and added GetAutoCenterMode.
//
//    Brad Whitlock, Thu Apr 19 11:16:56 PDT 2001
//    I added methods to handle window iconification.
//
//    Eric Brugger, Tue Apr 24 14:33:06 PDT 2001
//    I removed the methods Set/GetCamerPosition, Set/GetFocalPoint,
//    Set/GetViewUp and GetViewInfo.  I added the method UpdateViewInfo.
//
//    Eric Brugger, Tue Apr 24 16:49:27 PDT 2001
//    I modified the class to keep seperate 2d and 3d views.
//
//    Brad Whitlock, Thu Jun 14 16:26:33 PST 2001
//    I added the UpdateColorTable, RedrawWindow methods.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    I added Get/Set AnnotationAttributes.
//
//    Eric Brugger, Mon Aug 20 11:59:46 PDT 2001
//    I modified GetView2D and SetView2D to use avtView2D instead of
//    avtViewInfo.  I also modified GetView3D and SetView3D to use
//    avtView3D instead of avtViewInfo.
//
//    Brad Whitlock, Wed Aug 22 11:49:31 PDT 2001
//    I added the SendDeleteMessage method.
//
//    Brad Whitlock, Mon Aug 26 23:56:32 PST 2001
//    I added the InvertBackgroundColor method. I also modified the way
//    annotation state is handled.
//
//    Kathleen Bonnell, Wed Sep 26 11:49:18 PDT 2001 
//    Added SetPlotColors method, to send background and foreground color
//    to the plots. 
//
//    Brad Whitlock, Mon Oct 1 16:17:05 PST 2001
//    Added methods for handling tools.
//
//    Brad Whitlock, Fri Sep 14 15:27:44 PST 2001
//    I added a method for setting the light list.
//
//    Eric Brugger, Fri Nov  2 14:57:28 PST 2001
//    I added a const qualifier to the argument to SetAnnotationAttributes.
//    A also added the methods CopyGlobalAttributes, CopyViewAttributes,
//    CopyAnnotationAttributes, and CopyLightList.
//
//    Brad Whitlock, Fri Nov 2 10:50:21 PDT 2001
//    Added methods for returning the size, location of the window.
//
//    Eric Brugger, Wed Nov 21 13:38:45 PST 2001
//    I added methods for setting animation attributes.
//
//    Brad Whitlock, Wed Dec 5 11:35:51 PDT 2001
//    Added a method to return window attributes.
//
//    Brad Whitlock, Wed Sep 19 16:13:26 PST 2001
//    I added the UpdatesEnabled method.
//
//    Kathleen Bonnell, Thu Nov 22 16:03:00 PST 2001 
//    I added methods for pick mode. 
//
//    Brad Whitlock, Tue Feb 12 09:24:56 PDT 2002
//    Added SetActivePlots. Moved it from a higher level. Added a method to
//    update tools.
//
//    Brad Whitlock, Thu Apr 11 17:34:59 PST 2002
//    Addded an argument to HandleTool.
//
//    Eric Brugger, Mon Apr 15 14:54:39 PDT 2002
//    Added centeringValid2d and centeringValid3d to help handle the case
//    where the view is recentered when the window has no plots.
//
//    Kathleen Bonnell, Fri May 10 16:27:40 PDT 2002 
//    Added methods in support of avtViewCurve and lineout. 
//
//    Sean Ahern, Mon May 20 13:25:47 PDT 2002
//    Added ability to raise/lower windows.
//    
//    Hank Childs, Wed May 29 08:56:16 PDT 2002
//    Added Get/Set SpinMode.
//
//    Kathleen Bonnell, Mon Jun 10 17:22:57 PDT 2002 
//    Added UpdateQuery, DeleteQuery.
//
//    Kathleen Bonnell, Fri Jul 12 17:28:31 PDT 2002  
//    Added ScalePlots. 
//
//    Hank Childs, Mon Jul 15 10:50:24 PDT 2002
//    Added SetViewExtentsType.
//
//    Brad Whitlock, Tue Jul 23 16:17:58 PST 2002
//    Added DisableAllTools method.
//
//    Kathleen Bonnell, Tue Aug 13 15:15:37 PDT 2002   
//    Change Set/Get Light methods to use LightList instead of avtLightList. 
//
//    Brad Whitlock, Mon Sep 16 15:06:17 PST 2002
//    I added a GetInteractionMode method.
//
//    Brad Whitlock, Thu Sep 19 14:00:07 PST 2002
//    I added methods to get/set rendering options like antialiasing, stereo.
//
//    Brad Whitlock, Fri Sep 6 15:21:57 PST 2002
//    I added some pick and lineout methods.
//
//    Kathleen Bonnell, Tue Oct  1 16:25:50 PDT 2002 
//    Changed argument of ValidateQuery to Line* to convey more information
//    than just color. 
//
//    Brad Whitlock, Tue Oct 15 16:50:55 PST 2002
//    Added CopyPlotList method.
//
//    Brad Whitlock, Wed Oct 30 15:02:46 PST 2002
//    Added SetPopupEnabled method.
//
//    Brad Whitlock, Mon Nov 11 11:14:35 PDT 2002
//    I added support for locking tools.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Removed Set/Get/Copy methods for PickAttributes.
// 
//    Kathleen Bonnell, Wed Dec  4 17:38:27 PST 2002
//    Remove antialiasing frames, no longer needed.  
// 
//    Kathleen Bonnell, Thu Dec 19 16:52:45 PST 2002  
//    Add argument to ValidateQuery. 
// 
//    Kathleen Bonnell, Mon Dec 23 13:13:15 PST 2002 
//    Remove Lineout. 
// 
//    Eric Brugger, Mon Jan  6 12:58:35 PST 2003
//    I added support for keyframing views.  This involved adding
//    ClearViewKeyframes, DeleteViewKeyframe, SetViewKeyframe,
//    GetCameraViewMode, SetCameraViewMode and UpdateCameraView.
//
//    Mark C. Miller, Mon Jan 13 16:52:33 PST 2003
//    Added method to forward request to register external render callback 
//    to the VisWindow
//
//    Eric Brugger, Tue Jan 14 07:59:05 PST 2003
//    I added the argument nDimensions to GetExtents.
//
//    Eric Brugger, Wed Jan 29 10:52:40 PST 2003
//    I added MoveViewKeyframe.
//
//    Brad Whitlock, Wed Jan 29 11:52:17 PDT 2003
//    I added toolbar support.
//
//    Kathleen Bonnell, Fri Jan 31 09:36:54 PST 2003 
//    Modified ValidateQuery arguments. 
//
//    Eric Brugger, Tue Feb  4 12:39:59 PST 2003
//    I added GetViewKeyframeIndices.
//
//    Brad Whitlock, Wed Feb 12 11:50:47 PDT 2003
//    I renamed a few methods that copy attributes.
//
//    Brad Whitlock, Wed Mar 12 09:37:13 PDT 2003
//    I added IsVisible, HideCallback, and ShowCallback.
//
//    Brad Whitlock, Fri Apr 11 07:41:24 PDT 2003
//    I removed SetActivePlots since it is now an action.
//
//    Eric Brugger, Fri Apr 18 12:18:38 PDT 2003
//    I replaced Set/GetAutoCenterMode with Set/GetMaintainViewMode.  I
//    added SetViewModified2d.  I added SetMergeViewLimits.
//
//    Eric Brugger, Fri May  9 13:39:35 PDT 2003
//    I added AdjustView3d.
//
//    Kathleen Bonnell, Thu May 15 13:09:41 PDT 2003 
//    Added fullFrame member and Set/Get methods.  Added Compute2DScaleFactor.
//
//    Kathleen Bonnell, Fri Jun  6 16:06:25 PDT 2003
//    Added GetScaleFactorAndType method. 
//
//    Kathleen Bonnell, Wed Jun 25 16:50:48 PDT 2003 
//    Removed Start/StopPickMode.  Added INTERACTION_MODE argument to Pick.
//
//    Brad Whitlock, Mon Jun 30 13:03:48 PST 2003
//    Added CreateNode, SetFromNode.
//
//    Mark C. Miller, 07Jul03
//    Added GetWindowSize method
//
// ****************************************************************************

class VIEWER_API ViewerWindow
{
    friend class ViewerToolbar;
public:
    ViewerWindow(int id = 0);
    ~ViewerWindow();

    ViewerPopupMenu     *GetPopupMenu() const;
    ViewerToolbar       *GetToolbar() const;
    ViewerAnimation     *GetAnimation() const;
    ViewerActionManager *GetActionManager() const;
    int  GetWindowId() const;

    void CreateNode(DataNode *parentNode);
    void SetFromNode(DataNode *parentNode);

    void SetSize(const int width, const int height);
    void GetSize(int &width, int &height);
    void GetWindowSize(int &width, int &height);
    void SetLocation(const int x, const int y);
    void GetLocation(int &x, int &y);
    void SetTitle(const char *title);
    void Realize();

    void SetInteractionMode(const INTERACTION_MODE mode);
    INTERACTION_MODE GetInteractionMode() const;
    void SetToolEnabled(int toolId, bool enabled);
    void DisableAllTools();
    void ClearViewKeyframes();
    void DeleteViewKeyframe(const int frame);
    void MoveViewKeyframe(int oldFrame, int newFrame);
    void SetViewKeyframe();
    const int *GetViewKeyframeIndices(int &) const;
    void RecenterView();
    void RecenterView(const double *limits);
    void ResetView();
    void SetViewExtentsType(const avtExtentType);
    avtExtentType GetViewExtentsType() const;
    void GetExtents(int nDimensions, double *extents);
    void SetBoundingBoxMode(const bool mode);
    bool GetBoundingBoxMode() const;
    void SetSpinMode(const bool mode);
    bool GetSpinMode() const;
    void SetCameraViewMode(const bool mode);
    bool GetCameraViewMode() const;
    void SetMaintainViewMode(const bool mode);
    bool GetMaintainViewMode() const;
    void SetViewIsLocked(const bool);
    bool GetViewIsLocked() const;
    void SetTimeLock(const bool);
    bool GetTimeLock() const;
    void SetToolLock(const bool);
    bool GetToolLock() const;
    void SetPerspectiveProjection(const bool mode);
    bool GetPerspectiveProjection() const;
    void SetPopupEnabled(bool val);
    void UpdateColorTable(const char *ctName);
    void RedrawWindow();
    void ClearWindow();
    avtImage_p ScreenCapture();
    avtDataset_p GetAllDatasets();
    void InvertBackgroundColor();
    void CopyGeneralAttributes(const ViewerWindow *);
    WindowAttributes GetWindowAttributes() const;

    void AddPlot(avtActor_p &);
    void ClearPlots();
    void CopyAnimation(const ViewerWindow *, bool copyplots=true);
    void EnableUpdates();
    void DisableUpdates();

    void SendRedrawMessage();
    void SendUpdateMessage();
    void SendDeleteMessage();

    bool IsTheSameWindow(VisWindow *);

    void UpdateView(const int dimension, const double *limits);
    void SetView2D(const avtView2D &v);
    void SetView3D(const avtView3D &v);
    void SetViewCurve(const avtViewCurve &v);
    const avtView2D &GetView2D() const;
    const avtView3D &GetView3D() const;
    const avtViewCurve &GetViewCurve() const;
    int  GetViewDimension() const { return viewDimension; }
    void SetViewModified2d() { viewModified2d = true; }
    void SetMergeViewLimits(bool mode) { mergeViewLimits = mode; }
    void CopyViewAttributes(const ViewerWindow *);
    void UpdateCameraView();

    void SetAnimationAttributes(const AnimationAttributes *);
    const AnimationAttributes *GetAnimationAttributes() const;

    void SetAnnotationAttributes(const AnnotationAttributes *);
    const AnnotationAttributes *GetAnnotationAttributes() const;
    void CopyAnnotationAttributes(const ViewerWindow *);

    void SetLightList(const LightList *);
    const LightList *GetLightList() const;
    void CopyLightList(const ViewerWindow *);

    bool UpdatesEnabled() const;

    void ShowMenu();
    void HideMenu();

    void Pick(int x, int y, const INTERACTION_MODE mode);
    void ClearPickPoints();

    void ValidateQuery(const PickAttributes *, const Line *);
    void UpdateQuery(const Line *);
    void DeleteQuery(const Line *);
    void ClearRefLines();

    void Iconify();
    void DeIconify();
    void Show();
    void Hide();
    void Raise();
    void Lower();
    bool GetRealized();
    void SetVisible(bool);
    bool IsVisible() const;
    void SetTypeIsCurve(const bool);
    bool GetTypeIsCurve() const;

    void SetPlotColors(const double *bg, const double *fg);

    void HandleTool(const avtToolInterface &ti, bool applyToAll=false);
    void UpdateTools();
    int  GetNumTools() const;
    bool GetToolAvailable(int index) const;
    bool GetToolEnabled(int index) const;
    std::string GetToolName(int index) const;

    static void SetNoWinMode(bool);
    static bool GetNoWinMode();

    void SetFullFrameMode(const bool mode);
    bool GetFullFrameMode() const;
    void Compute2DScaleFactor(double &s, int &t);
    void GetScaleFactorAndType(double &s, int &t);


    void ScalePlots(const float [3]);

    // Rendering options.
    void SetAntialiasing(bool enabled);
    bool GetAntialiasing() const;
    void GetRenderTimes(float times[3]) const;
    void SetStereoRendering(bool enabled, int type);
    bool GetStereo() const;
    int  GetStereoType() const;
    void SetImmediateModeRendering(bool mode);
    bool GetImmediateModeRendering() const;
    void SetSurfaceRepresentation(int rep);
    int  GetSurfaceRepresentation() const;
    int  GetNumTriangles() const;
    void SetNotifyForEachRender(bool val);
    bool GetNotifyForEachRender() const;
    void SetScalableRendering(bool mode, bool update = false);
    bool GetScalableRendering() const;
    void SetScalableThreshold(int threshold);
    int  GetScalableThreshold() const;

private:
    void RecenterView2d(const double *limits);
    void RecenterView3d(const double *limits);
    void RecenterViewCurve(const double *limits);
    void ResetView2d();
    void ResetView3d();
    void ResetViewCurve();
    void AdjustView3d(const double *limits);
    void UpdateView2d(const double *limits);
    void UpdateView3d(const double *limits);
    void UpdateViewCurve(const double *limits);

    static void ShowCallback(void *);
    static void HideCallback(void *);
    static void ShowMenuCallback(void *);
    static void HideMenuCallback(void *);
    static void CloseCallback(void *);
    static void ExternalRenderCallback(void *, avtDataObject_p&);

    static void PerformPickCallback(void *);
    static void PerformLineoutCallback(void *);

    void *CreateToolbar(const std::string &name);

    VisWindow           *visWindow;
    ViewerPopupMenu     *popupMenu;
    ViewerToolbar       *toolbar;
    ViewerAnimation     *animation;
    ViewerActionManager *actionMgr;

    ViewAttributes      *curView;
    AttributeSubjectMap *view2DAtts;
    AttributeSubjectMap *view3DAtts;

    ExternalRenderRequestInfo lastExternalRenderRequest;

    bool            cameraView;
    bool            maintainView;
    bool            viewIsLocked;
    int             viewDimension;
    avtExtentType   plotExtentsType;
    bool            timeLocked;
    bool            toolsLocked;
    int             windowId;
    bool            isVisible;

    double          boundingBox2d[4];
    bool            boundingBoxValid2d;
    bool            haveRenderedIn2d;
    bool            viewModified2d;
    bool            mergeViewLimits;

    double          boundingBox3d[6];
    bool            boundingBoxValid3d;
    bool            haveRenderedIn3d;

    bool            centeringValid2d;
    bool            centeringValid3d;

    bool            fullFrame;

    static bool     doNoWinMode;

};

#endif
