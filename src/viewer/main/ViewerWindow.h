/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                               ViewerWindow.h                              //
// ************************************************************************* //

#ifndef VIEWER_WINDOW_H
#define VIEWER_WINDOW_H
#include <viewer_exports.h>
#include <ViewerBase.h>
#include <avtActor.h>
#include <avtDataset.h>
#include <avtImage.h>
#include <avtTypes.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <avtViewCurve.h>
#include <EngineKey.h>
#include <VisCallback.h>
#include <VisWindowTypes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <WindowAttributes.h>
#include <ViewStack.h>
#include <vectortypes.h>
#include <map>

#include <ExternalRenderRequestInfo.h>

class AnnotationObjectList;
class AttributeSubject;
class AttributeSubjectMap;
class DataNode;
class Line;
class LightList;
class PickAttributes;
class InteractorAttributes;

class avtToolInterface;

class VisWindow;
class ViewerActionManager;
class ViewerPlotList;
class ViewerPopupMenu;
class ViewerToolbar;

// ****************************************************************************
//  Class: ViewerWindow
//
//  Purpose:
//    This class contains a VisWindow, an plot list and extra things
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
//    Added CreateNode, SetFromNode, and a few new methods to send messages
//    to the main event loop.
//
//    Mark C. Miller, 07Jul03
//    Added GetWindowSize method
//
//    Eric Brugger, Wed Aug 20 11:14:04 PDT 2003
//    I implemented curve view as a first class view type.  I split the
//    view attributes into 2d and 3d parts.  I added the concept of a
//    window mode.
//
//    Eric Brugger, Thu Aug 28 12:16:23 PDT 2003
//    I added SetViewModifiedCurve.
//
//    Eric Brugger, Thu Oct  9 11:06:16 PDT 2003
//    I deleted Compute2DScaleFactor.  I deleted fullFrame.
//
//    Brad Whitlock, Wed Oct 29 12:06:40 PDT 2003
//    I added several methods to set annotation properties.
//
//    Mark C. Miller, Tue Nov 11 18:15:41 PST 2003
//    Introduced several new methods to refactored
//    ExternalRenderRequestCallback
//
//    Jeremy Meredith, Fri Nov 14 17:58:16 PST 2003
//    Added specular properties.
//
//    Eric Brugger, Mon Dec 22 10:22:09 PST 2003
//    Added SetInitialView3d.
//
//    Brad Whitlock, Tue Dec 30 10:47:42 PDT 2003
//    I added SetCenterOfRotation, ChooseCenterOfRotation and methods to
//    support more generalized picking.
//
//    Brad Whitlock, Sun Jan 25 23:47:06 PST 2004
//    Added support for multiple time sliders.
//
//    Mark C. Miller, Mon Mar 29 12:09:42 PST 2004
//    Refactored ExternalRenderCallback into methods support automatic
//    and manual external rendering
//
//    Jeremy Meredith, Thu Mar 25 15:39:33 PST 2004
//    Added an engine key used to index (and restart) engines.
//    This replaces hostName in ExternalRenderRequestInfo.
//
//    Eric Brugger, Mon Mar 29 15:33:13 PST 2004
//    I added Set/GetMaintainDataMode.
//
//    Mark C. Miller, Wed Apr 14 16:41:32 PDT 2004
//    Added data member for extents type string to render request info
//
//    Eric Brugger, Wed Apr 21 15:19:01 PDT 2004
//    Replaced haveRenderedInCurve with viewSetInCurve, haveRenderedIn2d
//    with viewSetIn2d and haveRenderedIn3d with viewSetIn3d.  Added
//    viewPartialSetIn3d.
//
//    Mark C. Miller, Tue Apr 27 14:41:35 PDT 2004
//    Removed preparingToChangeScalableRenderingMode
//
//    Hank Childs, Mon May 10 08:03:22 PDT 2004
//    Replaced "immediate mode" bool with "display list" enum.
//
//    Mark C. Miller, Tue May 11 20:21:24 PDT 2004
//    Added ShouldSendScalableRenderinModeChangeMessage, GetNumberOfCells
//    as well as methods to set/get scalable activation mode and scalable
//    auto threshold
//
//    Mark C. Miller, Wed Jun  9 17:44:38 PDT 2004
//    Added UpdateVisualCueInfo
//
//    Kathleen Bonnell, Fri Jul  9 16:22:50 PDT 2004 
//    Added Lineout.
//
//    Mark C. Miller, Tue Jul 27 15:11:11 PDT 2004
//    Added method to get frame and state info
//
//    Kathleen Bonnell, Wed Aug 18 09:39:29 PDT 2004 
//    Added SetInteracotrAtts, GetInteractorAtts, CopyInteractorAtts.
//
//    Kathleen Bonnell, Thu Sep  2 13:55:05 PDT 2004
//    Added bool arg to SetPickFunction, to indicate if pick shoul be
//    performed normally or as intersection-only. 
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Const qualified GetExtents
//
//    Mark C. Miller, Tue Oct 19 20:18:22 PDT 2004
//    Added data member for name of last color table to change
//
//    Kathleen Bonnell, Tue Oct 12 16:31:46 PDT 2004
//    Added GlyphPick method. 
//
//    Hank Childs, Sun Oct 24 13:39:57 PDT 2004
//    Added shading.
//
//    Mark C. Miller, Fri Mar  4 13:05:02 PST 2005
//    Changed GetNumTriangles to GetNumPrimitives
//
//    Brad Whitlock, Wed Apr 13 17:06:32 PST 2005
//    removed a method to set the popup menu's enabled state since we can
//    get at the menu directly.
//
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added 3 most recent rendering times to set of times returned from
//    GetRenderTimes. Added compression controls
//
//    Brad Whitlock, Wed Jan 11 14:55:54 PST 2006
//    Added SessionContainsErrors.
//
//    Brad Whitlock, Tue Mar 7 17:35:28 PST 2006
//    I added view stacks.
//    
//    Mark C. Miller, Sat Jul 22 23:21:09 PDT 2006
//    Added leftEye to external render methods. Added scalableStereoType
//    to store knowledge of stereo mode for cases where it is not appropriate
//    to actually put the visWindow into stereo.
//
//    Mark C. Miller, Wed Aug  9 16:35:25 PDT 2006
//    Removed scalableStereoType data member
//
//    Brad Whitlock, Thu Nov 9 16:45:04 PST 2006
//    Added argument to CreateNode.
//
//    Brad Whitlock, Mon Feb 12 17:41:58 PST 2007
//    Added ViewerBase base class.
//
//    Brad Whitlock, Tue Mar 20 10:02:18 PDT 2007
//    Name new annotation objects.
//
//    Kathleen Bonnell, Thu Mar 22 19:36:05 PDT 2007 
//    Added SetScaleMode method.
//
//    Brad Whitlock, Mon Mar 26 14:49:15 PST 2007
//    Added argument to CopyAnnotationObjectList.
//
//    Kathleen Bonnell, Wed Apr  4 10:08:27 PDT 2007 
//    Added GetScaleMode.
//
//    Kathleen Bonnell, Wed May  9 17:33:41 PDT 2007 
//    Added WINDOW_MODE arg to Set/GetScaleMode.
//
//    Jeremy Meredith, Tue Jul 17 16:37:04 EDT 2007
//    Added fullscreen support.
//
//    Jeremy Meredith, Wed Aug 29 15:21:38 EDT 2007
//    Added depth cueing properties.
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Brad Whitlock, Wed Feb 13 14:04:22 PST 2008
//    Added configVersion to SetFromNode.
//
//    Brad Whitlock, Wed Dec 10 15:25:15 PST 2008
//    I removed the AnimationAttribute methods since they can be accessed
//    via the plot list.
//
//    Brad Whitlock, Tue Apr 14 11:37:34 PDT 2009
//    I made it use ViewerProperties.
//
//    Jeremy Meredith, Tue Feb  2 11:16:01 EST 2010
//    Added tool update modes.
//
//    Jeremy Meredith, Wed Feb  3 15:35:08 EST 2010
//    Removed maintain data; moved maintain view from Global settings
//    (Main window) to per-window Window Information (View window).
//
// ****************************************************************************

class VIEWER_API ViewerWindow : public ViewerBase
{
    friend class ViewerToolbar;
public:
    ViewerWindow(int id = 0);
    virtual ~ViewerWindow();

    ViewerPopupMenu     *GetPopupMenu() const;
    ViewerToolbar       *GetToolbar() const;
    ViewerActionManager *GetActionManager() const;
    ViewerPlotList      *GetPlotList() const;
    int  GetWindowId() const;

    void CreateNode(DataNode *parentNode, 
                    const std::map<std::string, std::string> &, 
                    bool detailed);
    void SetFromNode(DataNode *parentNode,
                     const std::map<std::string, std::string> &,
                     const std::string &configVersion);
    static bool SessionContainsErrors(DataNode *parentNode);

    void SetSize(const int width, const int height);
    void GetSize(int &width, int &height);
    void GetWindowSize(int &width, int &height);
    void SetLocation(const int x, const int y);
    void GetLocation(int &x, int &y);
    void SetTitle(const char *title);
    void Realize();

    void SetInteractionMode(const INTERACTION_MODE mode);
    INTERACTION_MODE GetInteractionMode() const;
    void SetToolUpdateMode(TOOLUPDATE_MODE m);
    TOOLUPDATE_MODE GetToolUpdateMode() const;
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
    void SetCenterOfRotation(double x, double y, double z);
    void ChooseCenterOfRotation(double sx, double sy);
    void SetViewExtentsType(const avtExtentType);
    avtExtentType GetViewExtentsType() const;
    void GetExtents(int nDimensions, double *extents) const;
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
    void UpdateColorTable(const char *ctName);
    void RedrawWindow();
    void ClearWindow(bool clearAllPlots = true);
    avtImage_p ScreenCapture();
    avtDataset_p GetAllDatasets();
    void InvertBackgroundColor();
    void CopyGeneralAttributes(const ViewerWindow *);
    WindowAttributes GetWindowAttributes() const;
    bool SendWindowEnvironmentToEngine(const EngineKey &ek);

    void UndoView();
    void RedoView();
    void PushCurrentViews();
    bool UndoViewEnabled() const;
    bool RedoViewEnabled() const;

    void AddPlot(avtActor_p &);
    void ClearPlots();
    void EnableUpdates();
    void DisableUpdates();

    void SendRedrawMessage();
    void SendUpdateMessage();
    void SendDeleteMessage();
    void SendUpdateFrameMessage() const;
    void SendActivateToolMessage(const int toolId) const;
    void SendInteractionModeMessage(const INTERACTION_MODE m) const;
    void SendToolUpdateModeMessage(const TOOLUPDATE_MODE m) const;
    void SendScalableRenderingModeChangeMessage(bool newMode);
    bool ShouldSendScalableRenderingModeChangeMessage(bool *newMode) const;

    bool IsTheSameWindow(VisWindow *);

    void UpdateView(const WINDOW_MODE mode, const double *limits);
    void SetViewCurve(const avtViewCurve &v);
    void SetView2D(const avtView2D &v);
    void SetView3D(const avtView3D &v);
    void SetViewAxisArray(const avtViewAxisArray &v);
    const avtViewCurve &GetViewCurve() const;
    const avtView2D &GetView2D() const;
    const avtView3D &GetView3D() const;
    const avtViewAxisArray &GetViewAxisArray() const;
    void SetViewModifiedCurve() { viewModifiedCurve = true; }
    void SetViewModified2d() { viewModified2d = true; }
    void SetViewModifiedAxisArray() { viewModifiedAxisArray = true; }
    void SetMergeViewLimits(bool mode) { mergeViewLimits = mode; }
    void CopyViewAttributes(const ViewerWindow *);
    void UpdateCameraView();

    void SetAnnotationAttributes(const AnnotationAttributes *);
    const AnnotationAttributes *GetAnnotationAttributes() const;
    void CopyAnnotationAttributes(const ViewerWindow *);
    void CopyAnnotationObjectList(const ViewerWindow *, bool copyLegends);
    bool AddAnnotationObject(int annotType, const std::string &annotName);
    void HideActiveAnnotationObjects();
    void DeleteActiveAnnotationObjects();
    bool DeleteAnnotationObject(const std::string &name);
    void DeleteAllAnnotationObjects();
    void RaiseActiveAnnotationObjects();
    void LowerActiveAnnotationObjects();
    void SetAnnotationObjectOptions(const AnnotationObjectList &al);
    void CreateAnnotationObjectsFromList(const AnnotationObjectList &al);
    void UpdateAnnotationObjectList(AnnotationObjectList &al) const;
    void SetFrameAndState(int, int, int, int, int, int, int);
    void GetFrameAndState(int&, int&, int&, int&, int&, int&, int&);

    void SetLightList(const LightList *);
    const LightList *GetLightList() const;
    void CopyLightList(const ViewerWindow *);

    bool UpdatesEnabled() const;

    void ShowMenu();
    void HideMenu();

    void Pick(int x, int y, const INTERACTION_MODE mode);
    bool GetPickAttributesForScreenPoint(double sx, double sy,
                                         PickAttributes &pa);
    void SetPickFunction(void (*func)(void *, bool, const PickAttributes *),
                         void *data, bool);
    void ClearPickPoints();

    void ValidateQuery(const PickAttributes *, const Line *);
    void UpdateQuery(const Line *);
    void DeleteQuery(const Line *);
    void ClearRefLines();

    void UpdateVisualCueList(VisualCueList& visCues) const;

    void Iconify();
    void DeIconify();
    void Show();
    void Hide();
    void Raise();
    void Lower();
    bool GetRealized();
    void SetVisible(bool);
    bool IsVisible() const;
    WINDOW_MODE GetWindowMode() const;
    void SetLargeIcons(bool);

    void SetPlotColors(const double *bg, const double *fg);

    void HandleTool(const avtToolInterface &ti, bool applyToAll=false);
    void UpdateTools();
    int  GetNumTools() const;
    bool GetToolAvailable(int index) const;
    bool GetToolEnabled(int index) const;
    std::string GetToolName(int index) const;

    void SetFullFrameMode(const bool mode);
    bool GetFullFrameMode() const;
    void SetFullFrameActivationMode(const int mode);
    int  GetFullFrameActivationMode() const;
    void GetScaleFactorAndType(double &s, int &t);
    bool DoAllPlotsAxesHaveSameUnits();

    void ConvertFromLeftEyeToRightEye(void);
    void ConvertFromRightEyeToLeftEye(void);

    void ChangeScalableRenderingMode(bool newMode);
    bool IsChangingScalableRenderingMode(bool toMode = false) const;
    bool DisableExternalRenderRequests(bool bClearImage = false);
    bool EnableExternalRenderRequests();
    void ExternalRenderManual(avtDataObject_p& dob, int w, int h);

    // Rendering options.
    void SetAntialiasing(bool enabled);
    bool GetAntialiasing() const;
    void GetRenderTimes(double times[6]) const;
    void SetStereoRendering(bool enabled, int type);
    bool GetStereo() const;
    int  GetStereoType() const;
    void SetDisplayListMode(int);
    int  GetDisplayListMode(void) const;
    void SetSurfaceRepresentation(int rep);
    int  GetSurfaceRepresentation() const;
    int  GetNumPrimitives() const;
    int  GetNumberOfCells(bool polysOnly = false) const; 
    void SetNotifyForEachRender(bool val);
    bool GetNotifyForEachRender() const;
    bool GetScalableRendering() const;
    int  GetScalableThreshold() const;
    int  SetScalableActivationMode(int mode);
    int  GetScalableActivationMode() const;
    int  SetScalableAutoThreshold(int threshold);
    int  GetScalableAutoThreshold() const;
    void SetSpecularProperties(bool,double,double,const ColorAttribute&);
    bool  GetSpecularFlag() const;
    double GetSpecularCoeff() const;
    double GetSpecularPower() const;
    const ColorAttribute &GetSpecularColor() const;
    void SetShadingProperties(bool,double);
    bool GetDoShading() const;
    double GetShadingStrength() const;
    void SetDepthCueingProperties(bool,const double[3],const double[3]);
    bool GetDoDepthCueing() const;
    const double *GetStartCuePoint() const;
    const double *GetEndCuePoint() const;
    void SetColorTexturingFlag(bool);
    bool GetColorTexturingFlag() const;
    int GetCompressionActivationMode() const;
    int SetCompressionActivationMode(int);
    bool GetIsCompressingScalableImage() const;

    void Lineout(const bool);

    void SetInteractorAtts(const InteractorAttributes *);
    const InteractorAttributes *GetInteractorAtts() const;
    void CopyInteractorAtts(const ViewerWindow *);

    void GlyphPick(const double pt3[3], const double pt2[3], 
                   int &dom, int &elNum, bool &forCell);

    void SetScaleMode(ScaleMode ds, ScaleMode rs, WINDOW_MODE);
    void GetScaleMode(ScaleMode &ds, ScaleMode &rs, WINDOW_MODE);
    
    void SetPlotFollowsTime();

private:
    void RecenterViewCurve(const double *limits);
    void RecenterView2d(const double *limits);
    void RecenterView3d(const double *limits);
    void RecenterViewAxisArray(const double *limits);
    void ResetViewCurve();
    void ResetView2d();
    void ResetView3d();
    void ResetViewAxisArray();
    void AdjustView3d(const double *limits);
    void SetInitialView3d();
    void UpdateViewCurve(const double *limits);
    void UpdateView2d(const double *limits);
    void UpdateView3d(const double *limits);
    void UpdateViewAxisArray(const double *limits);

    void ClearExternalRenderRequestInfo(ExternalRenderRequestInfo&) const;
    void ClearLastExternalRenderRequestInfo();
    void UpdateLastExternalRenderRequestInfo(
             const ExternalRenderRequestInfo& newRequest);
    void GetExternalRenderRequestInfo(
             ExternalRenderRequestInfo& theRequest, bool leftEye) const;
    bool CanSkipExternalRender(
             const ExternalRenderRequestInfo& thisRequest) const;
    bool ExternalRender(const ExternalRenderRequestInfo& thisRequest,
             bool& shouldTurnOffScalableRendering, bool doAllAnnotations,
             avtDataObject_p& dob);
    void ExternalRenderAuto(avtDataObject_p& dob, bool leftEye);

    static void ShowCallback(void *);
    static void HideCallback(void *);
    static void ShowMenuCallback(void *);
    static void HideMenuCallback(void *);
    static void CloseCallback(void *);
    static void ExternalRenderCallback(void *, avtDataObject_p&);

    static void PerformPickCallback(void *);
    static void PerformLineoutCallback(void *);
    static void PickFunctionSetSuccessFlag(void *, bool, const PickAttributes *);
    void HandlePick(void *ppi);

    void *CreateToolbar(const std::string &name);

    VisWindow           *visWindow;
    ViewerPopupMenu     *popupMenu;
    ViewerToolbar       *toolbar;
    ViewerPlotList      *plotList;
    ViewerActionManager *actionMgr;

    ViewCurveAttributes     *curViewCurve;
    View2DAttributes        *curView2D;
    View3DAttributes        *curView3D;
    ViewAxisArrayAttributes *curViewAxisArray;
    AttributeSubjectMap     *viewCurveAtts;
    AttributeSubjectMap     *view2DAtts;
    AttributeSubjectMap     *view3DAtts;
    AttributeSubjectMap     *viewAxisArrayAtts;

    ExternalRenderRequestInfo lastExternalRenderRequest;

    void           (*pickFunction)(void *, bool, const PickAttributes *);
    void           *pickFunctionData;

    bool            isChangingScalableRenderingMode;
    bool            targetScalableRenderingMode;
    std::string     nameOfCtChangedSinceLastRender;
    bool            isCompressingScalableImage;
    int             compressionActivationMode;


    bool            cameraView;
    bool            maintainView;
    bool            viewIsLocked;
    WINDOW_MODE     windowMode;
    avtExtentType   plotExtentsType;
    bool            timeLocked;
    bool            toolsLocked;
    int             windowId;
    bool            isVisible;

    double          boundingBoxCurve[4];
    bool            boundingBoxValidCurve;
    bool            viewSetInCurve;
    bool            viewModifiedCurve;

    double          boundingBox2d[4];
    bool            boundingBoxValid2d;
    bool            viewSetIn2d;
    bool            viewModified2d;

    double          boundingBox3d[6];
    bool            boundingBoxValid3d;
    bool            viewSetIn3d;
    bool            viewPartialSetIn3d;

    double          boundingBoxAxisArray[4];
    bool            boundingBoxValidAxisArray;
    bool            viewSetInAxisArray;
    bool            viewModifiedAxisArray;

    bool            centeringValidCurve;
    bool            centeringValid2d;
    bool            centeringValid3d;
    bool            centeringValidAxisArray;

    bool            mergeViewLimits;

    ViewStack       undoViewStack;
    ViewStack       redoViewStack;

    // Note: since these aren't stored in VisWindow, this seems like a fine
    // place to put them.  If the shading ever is stored in VisWindow, this
    // should be removed.
    bool            doShading;
    double          shadingStrength;
    // Same for depth cueing
    bool            doDepthCueing;
    double          startCuePoint[3];
    double          endCuePoint[3];
};

#endif
