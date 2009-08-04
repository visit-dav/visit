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
//                            ViewerWindowManager.h                          //
// ************************************************************************* //

#ifndef VIEWER_WINDOW_MANAGER_H
#define VIEWER_WINDOW_MANAGER_H
#include <viewer_exports.h>
#include <ViewerBase.h>
#include <VisWindowTypes.h>
#include <avtTypes.h>
#include <avtImage.h>
#include <avtDataset.h>
#include <avtViewCurve.h>
#include <avtView2D.h>
#include <avtView3D.h>
#include <avtViewAxisArray.h>
#include <vectortypes.h>
#include <map>

//
// Flags to use in calls to the UpdateWindowInformation method.
//

// Use when the active source could have changed
#define WINDOWINFO_SOURCE           1
// Use when the list of time slider names or the active time slider could
// have changed.
#define WINDOWINFO_TIMESLIDERS      2
// Use when the time slider states or the animation mode could have changed.
#define WINDOWINFO_ANIMATION        4
// Use when the window flag could have changed.
#define WINDOWINFO_WINDOWFLAGS      8
// Use when the window's rendering information could have changed.
#define WINDOWINFO_RENDERINFO       16
// Use when the window size could have changed.
#define WINDOWINFO_WINDOWSIZE       32
// Use when the window mode (WINMODE) needs to be updated
#define WINDOWINFO_WINMODEONLY      64

class AnimationAttributes;
class AnnotationAttributes;
class AnnotationObjectList;
class DatabaseCorrelation;
class DataNode;
class EngineKey;
class GlobalAttributes;
class InteractorAttributes;
class KeyframeAttributes;
class LightList;
class Line;
class PrinterAttributes;
class RenderingAttributes;
class SaveWindowAttributes;
class ViewerWindow;
class ViewerAnimation;
class ViewCurveAttributes;
class View2DAttributes;
class View3DAttributes;
class ViewerWindowManagerAttributes;
class WindowInformation;
class VisWindow;
class avtFileWriter;
class avtToolInterface;
class QTimer;

typedef struct {
   int       x;
   int       y;
   int       width;
   int       height;
} WindowLimits;

// ****************************************************************************
//  Class: ViewerWindowManager
//
//  Purpose:
//    ViewerWindowManager is a class that manages viewer windows.  It has
//    methods for adding and deleting windows, specifying window layouts,
//    and setting the active window.  It is modeled after the singleton
//    class which means that applications can't directly instantiate the
//    class but can only get a reference to the one instance through the
//    Instance method.
//
//  Note:  The class currently supports a maximum of 16 windows.
//
//  Programmer: Eric Brugger
//  Creation:   September 7, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Nov 7 10:08:37 PDT 2000
//    Changed to use ViewerWindow.
//
//    Hank Childs, Fri Nov 10 11:20:19 PST 2000
//    Added TogglePerspective.
//
//    Brad Whitlock, Mon Nov 27 14:28:41 PST 2000
//    I made it inherit from QObject so I could add some signals that are
//    emitted when creating and deleting windows.
//
//    Eric Brugger, Fri Feb 23 12:33:00 PST 2001
//    I added RecenterView and ToggleAutoCenterMode.
//
//    Brad Whitlock, Thu Apr 19 11:10:12 PDT 2001
//    I added methods to handle window iconification.
//
//    Brad Whitlock, Thu Jun 14 16:22:21 PST 2001
//    Added a method to update the color table.
//
//    Jeremy Meredith, Fri Jul 20 11:23:42 PDT 2001
//    Added SetShift and shiftX/Y for correcting some window managers.
//
//    Brad Whitlock, Thu Jul 19 16:13:24 PST 2001
//    Added methods to set the view.
//
//    Eric Brugger, Tue Aug 21 10:16:06 PDT 2001
//    Added the ViewCallback method.  Removed an argument from UpdateVeiwAtts.
//
//    Brad Whitlock, Wed Aug 22 11:25:07 PDT 2001
//    Added a second DeleteWindow method.
//
//    Brad Whitlock, Mon, Aug 26 23:44:23 PST 2001
//    I added a new slot function and converted some functions to slots. I
//    also moved the annotation attributes into this class.
//
//    Jeremy Meredith, Fri Sep 14 13:49:38 PDT 2001
//    Added SetPreshift and preshiftX/Y for further correction of some
//    window managers.
//
//    Kathleen Bonnell, Wed Sep 26 11:49:18 PDT 2001 
//    Added SetPlotColors to send foreground/background color information
//    to the plots.
//
//    Brad Whitlock, Mon Oct 1 16:11:38 PST 2001
//    Added methods to handle tools.
//
//    Brad Whitlock, Fri Sep 14 14:19:57 PST 2001
//    Added methods to deal with lighting.
//
//    Eric Brugger, Fri Oct 26 14:31:59 PDT 2001
//    I added a timer for playing animations along with the method
//    UpdateAnimationTimer to turn it on and off appropriately.  I also
//    added an animation argument to UpdateAnimationState so that it would
//    only update the state if the animation was the active animation.
//
//    Eric Brugger, Fri Nov  2 12:37:44 PST 2001
//    I added a data member to keep track of whether the window has been
//    referenced before.
//
//    Brad Whitlock, Fri Nov 2 10:28:46 PDT 2001
//    Added a method to get the viewer window manager attributes.
//
//    Eric Brugger, Wed Nov 21 12:11:35 PST 2001
//    I added methods to deal with animation attributes.
//
//    Kathleen Bonnell, Tue Nov 27 16:03:00 PST 2001 
//    Added pick attributes and related methods.
//    Added method 'ClearPickPoints'. 
//
//    Brad Whitlock, Mon Jan 28 16:27:14 PST 2002
//    Added support for a stack of views.
//
//    Brad Whitlock, Fri Feb 8 10:29:32 PDT 2002
//    Added a method to query all plots in all windows to see if a file
//    is being used for the plot.
//
//    Brad Whitlock, Mon Feb 11 17:26:50 PST 2002
//    Added a method to set the active plots for a window.
//
//    Brad Whitlock, Wed Feb 20 14:34:42 PST 2002
//    I added printing support.
//
//    Sean Ahern, Tue Apr 16 12:36:24 PDT 2002
//    Added the ability to show/hide all windows and have deferred creation.
//
//    Kathleen Bonnell, Thu May  9 09:57:56 PDT 2002  
//    Added methods in support of Lineout: ClearRefLines, AddToLineoutWindow,
//    ResetLineoutDesignation.
//
//    Hank Childs, Thu May 23 17:33:41 PDT 2002
//    Added a more generic file writer that can take care of files and images.
//
//    Hank Childs, Wed May 29 08:42:29 PDT 2002
//    Added ToggleSpinMode.
//
//    Kathleen Bonnell, Wed May 29 11:24:14 PDT 2002  
//    Changed arguments to method AddtoLineoutWindow. 
//
//    Kathleen Bonnell, Mon Jun 10 17:22:57 PDT 2002 
//    Replace 'AddToLineoutWindow' with 'GetLineoutWindow'. 
//
//    Brad Whitlock, Thu Jun 27 16:37:21 PST 2002
//    Added methods to copy attributes from one window to another window.
//
//    Hank Childs, Mon Jul 15 09:58:27 PDT 2002
//    Added SetViewExtentsType.
//
//    Brad Whitlock, Tue Jul 23 16:15:07 PST 2002
//    I added a method to disable all tools.
//
//    Brad Whitlock, Thu Sep 19 13:27:06 PST 2002
//    I added a new state object.
//
//    Brad Whitlock, Fri Sep 6 15:17:26 PST 2002
//    I added methods to do pick and lineout.
//
//    Brad Whitlock, Tue Oct 15 16:36:41 PST 2002
//    I added methods to clone a window and copy plots to a window.
//
//    Brad Whitlock, Wed Oct 30 14:53:55 PST 2002
//    I added methods to tell the client and the viewer windows when an
//    engine is busy executing.
//
//    Brad Whitlock, Mon Nov 11 11:55:57 PDT 2002
//    I added methods to toggle locktime and locktools.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Removed PickAttributes and related Set/Get methods. Now handled
//    by ViewerQueryManager. 
//
//    Brad Whitlock, Tue Nov 19 14:44:36 PST 2002
//    I added more default arguments to UpdateViewAtts.
//
//    Eric Brugger, Mon Nov 25 09:58:47 PST 2002
//    I added GetKeyframeClientAtts and SetKeyframeAttsFromClient.
//
//    Kathleen Bonnell, Mon Dec 23 13:14:18 PST 2002 
//    Remove Lineout.
//
//    Eric Brugger, Mon Jan  6 12:27:06 PST 2003
//    I added ClearViewKeyframes, DeleteViewKeyframe, SetViewKeyframe and
//    ToggleCameraViewMode.
//
//    Eric Brugger, Wed Jan 29 11:08:28 PST 2003
//    I added MoveViewKeyframe.
//
//    Eric Brugger, Fri Jan 31 13:43:37 PST 2003 
//    I removed the state argument from UpdateAnimationState.
//
//    Brad Whitlock, Fri Jan 31 12:39:24 PDT 2003
//    I added a method to get the number of windows and another method to
//    update actions.
//
//    Brad Whitlock, Thu Feb 27 12:01:39 PDT 2003
//    I removed an overloaded version of UpdateAnimationState.
//
//    Brad Whitlock, Tue Apr 1 10:40:36 PDT 2003
//    I added a method to hide toolbars for all windows.
//
//    Eric Brugger, Fri Apr 11 14:01:53 PDT 2003
//    I added the method SimpleAddWindow.
//
//    Eric Brugger, Fri Apr 18 12:36:15 PDT 2003
//    I replaced ToggleAutoCenterMode with ToggleMaintainViewMode.
//
//    Kathleen Bonnell, Wed May 14 17:07:07 PDT 2003  
//    Removed 'Pick' method. 
//
//    Kathleen Bonnell, Thu May 15 10:00:02 PDT 2003 
//    Added ToggleFullFrameMode. 
//
//    Brad Whitlock, Mon Jun 30 12:39:19 PDT 2003
//    Added methods to save/read state.
//
//    Brad Whitlock, Wed Jul 23 13:56:03 PST 2003
//    Removed AddInitialWindows method.
//
//    Eric Brugger, Wed Aug 20 11:39:45 PDT 2003
//    I implemented curve view as a first class view type.  I split the
//    view attributes into 2d and 3d parts.
//
//    Brad Whitlock, Wed Oct 15 13:44:38 PST 2003
//    Added ReplaceDatabase method.
//
//    Brad Whitlock, Mon Nov 3 10:13:19 PDT 2003
//    Added more arguments to the ReplaceDatabase method.
//
//    Brad Whitlock, Wed Oct 29 11:13:55 PDT 2003
//    I added method to deal with advanced annotation features.
//
//    Brad Whitlock, Wed Jan 7 10:02:39 PDT 2004
//    I added methods for setting the center of rotation.
//
//    Brad Whitlock, Tue Feb 3 22:41:31 PST 2004
//    Added some methods to deal with time sliders and database correlations.
//    I also added a method to close a database.
//
//    Kathleen Bonnell,  Fri Mar 19 07:26:27 PST 2004
//    Added GetEmptyWindow and GetWindow methods. 
//
//    Eric Brugger, Mon Mar 29 14:53:52 PST 2004
//    I added ToggleMaintainDataMode.
//
//    Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004
//    Added GetTimeQueryWindow and ResetTimeQuery methods.
//
//    Mark C. Miller, Mon Jul 12 19:46:32 PDT 2004
//    Added optional bool to GetLineoutWindow to return NULL if window
//    doesn't already exist
//
//    Kathleen Bonnell, Tue Jul 20 10:47:26 PDT 2004 
//    Added optional int arg to GetTimeQueryWindow. 
//
//    Kathleen Bonnell, Thu Jul 22 09:23:25 PDT 200 
//    Added optional int arg to GetLineoutWindow, removed bool arg.
//
//    Brad Whitlock, Tue Jul 27 10:22:15 PDT 2004
//    Added CheckForNewStates.
//
//    Kathleen Bonnell, Wed Aug 18 09:28:51 PDT 2004 
//    Added interactorAtts and related methods.
//
//    Brad Whitlock, Thu Feb 3 14:34:45 PST 2005
//    Added AlterTimeSlider.
//
//    Brad Whitlock, Wed Mar 16 16:33:04 PST 2005
//    Added GetTimeLockedWindowIndices and made CreateMultiWindowCorrelation
//    public.
//
//    Mark C. Miller, Tue May  3 21:49:22 PDT 2005
//    Added bool arg to GetDatasets
//
//    Brad Whitlock, Thu Nov 17 17:12:05 PST 2005
//    Added methods to move and resize windows.
//
//    Brad Whitlock, Tue Mar 7 17:33:54 PST 2006
//    I removed the view stacks.
//
//    Brad Whitlock, Thu Nov 9 16:41:47 PST 2006
//    Made some interface changes to allow db parameterized sessions.
//
//    Brad Whitlock, Mon Feb 12 17:37:57 PST 2007
//    Changed base class.
//
//    Brad Whitlock, Tue Mar 20 09:59:08 PDT 2007
//    Name annotation objects.
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added methods to SetTryHarderCyclesTimes and SetTreatAllDBsAsTimeVarying
//    since it is the VWM that managers the GlobalAttributes where these
//    are defined.
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//    Added methods to SetCreateMeshQualityExpressions and 
//    SetCreateTimeDerivativeExpressions. 
//
//    Dave Bremer, Wed Oct 31 15:48:16 PDT 2007
//    Added flag to clear the external renderer's cached image if disabling it.
//
//    Cyrus Harrison, Wed Nov 28 12:01:16 PST 2007
//    Added SetCreateVectorMagnitudeExpressions.
//
//    Ellen Tarwater, Wed Dec 26, 2007
//    Added SetPlotFollowsTime
//
//    Brad Whitlock, Wed Jan 23 10:40:12 PST 2008
//    Added TurnOffAllLocks.
//
//    Jeremy Meredith, Thu Jan 31 14:56:06 EST 2008
//    Added new axis array window mode.
//
//    Hank Childs, Thu Feb  7 11:04:00 PST 2008
//    Added method SynchronizeTimeLockedWindows.
//
//    Brad Whitlock, Wed Feb 13 14:15:27 PST 2008
//    Added argument to SetFromNode.
//
//    Kathleen Bonnell, Thu Mar  6 09:21:02 PST 2008 
//    Made return type of SetPlotFollowsTime be 'void'.
//
//    Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//    Changed the interface to ReplaceDatabase, adding option to replace
//    only active plots.
//
// ****************************************************************************

class VIEWER_API ViewerWindowManager : public ViewerBase
{
    Q_OBJECT
  public:
    virtual ~ViewerWindowManager();

    static ViewerWindowManager *Instance();

    void SetBorders(const char *windowBorders);
    void SetShift(const char *windowShift);
    void SetPreshift(const char *windowPreshift);
    void SetGeometry(const char *windowGeometry);

    void AddWindow(bool copyAtts = false);
    void CloneWindow();
    void ClearAllWindows();
    void ClearWindow(int windowIndex = -1);
    void CopyAnnotationsToWindow(int from, int to);
    void CopyLightingToWindow(int from, int to);
    void CopyViewToWindow(int from, int to);
    void CopyPlotListToWindow(int from, int to);
    void DeleteWindow();
    void DeleteWindow(ViewerWindow *win);
    void DisableRedraw(int windowIndex = -1);
    void MoveWindow(int windowIndex, int x, int y);
    void MoveAndResizeWindow(int windowIndex, int x, int y, int w, int h);
    void ResizeWindow(int windowIndex, int w, int h);
    bool FileInUse(const std::string &host,
                   const std::string &dbName) const;
    void IconifyAllWindows();
    void DeIconifyAllWindows();
    void ShowAllWindows();
    void HideAllWindows();
    bool GetWindowsHidden() const { return windowsHidden; };
    void PrintWindow(int windowIndex = -1);
    void RecenterView(int windowIndex = -1);
    void RedrawWindow(int windowIndex = -1);
    void ResetView(int windowIndex = -1);
    void SaveWindow(int windowIndex = -1);
    void SetActiveWindow(const int windowId);
    void SetAnimationAttsFromClient();

    void SetAnnotationAttsFromClient();
    void SetAnnotationAttsFromDefault();
    void AddAnnotationObject(int annotType, const std::string &annotName);
    void HideActiveAnnotationObjects();
    void DeleteActiveAnnotationObjects();
    void RaiseActiveAnnotationObjects();
    void LowerActiveAnnotationObjects();
    void SetAnnotationObjectOptions();
    void UpdateAnnotationObjectList(bool delay = false);

    void SetFrameIndex(int frame, int windowIndex = -1);
    void NextFrame(int windowIndex = -1);
    void PrevFrame(int windowIndex = -1);
    void Stop(int windowIndex = -1);
    void Play(int windowIndex = -1);
    void ReversePlay(int windowIndex = -1);
    void SetActiveTimeSlider(const std::string &ts, int windowIndex = -1);
    void AlterTimeSlider(const std::string &ts);

    void SetKeyframeAttsFromClient();
    void UpdateKeyframeAttributes();

    void SetInteractionMode(INTERACTION_MODE m, int windowIndex = -1);
    void SetLightListFromClient();
    void SetLightListFromDefault();
    void SetViewCurveFromClient();
    void SetView2DFromClient();
    void SetView3DFromClient();
    void SetViewAxisArrayFromClient();
    void ClearViewKeyframes();
    void DeleteViewKeyframe(const int frame);
    void MoveViewKeyframe(int oldFrame, int newFrame);
    void SetViewKeyframe();
    void SetCenterOfRotation(int, double x, double y, double z);
    void ChooseCenterOfRotation(int, double sx, double sy);
    void SetWindowLayout(const int windowLayout);

    void SetRenderingAttributes(int windowIndex = -1);
    void ToggleBoundingBoxMode(int windowIndex = -1);
    void TogglePerspective(int windowIndex = -1);
    void ToggleMaintainViewMode(int windowIndex = -1);
    void ToggleMaintainDataMode(int windowIndex = -1);
    void ToggleSpinMode(int windowIndex = -1);
    void ToggleCameraViewMode(int windowIndex = -1);
    void ToggleFullFrameMode(int windowIndex = -1);

    void ToggleLockTime(int windowIndex = -1);
    void ToggleLockTools(int windowIndex = -1);
    void ToggleLockViewMode(int windowIndex = -1);
    void TurnOffAllLocks();

    void SetViewExtentsType(avtExtentType, int windowIndex = -1);
    void UpdateColorTable(const char *ctName);
    void ResetLineoutDesignation(int windowIndex = -1); 
    void ResetTimeQueryDesignation(int windowIndex = -1);
    void ResetNetworkIds(const EngineKey &k);
    int             *GetWindowIndices(int *nwin) const;
    void             GetTimeLockedWindowIndices(intVector &ids) const;
    int              GetNumWindows() const;
    ViewerWindow    *GetActiveWindow() const;
    ViewerWindow    *GetLineoutWindow(int useThisId = -1);
    ViewerWindow    *GetTimeQueryWindow(int useThisId = -1);
    ViewerWindow    *GetEmptyWindow() ;
    ViewerWindow    *GetWindow(int windowIndex) ;
    int              GetWindowLayout() const { return layout; };
    void UpdateActions();
    void HideToolbarsForAllWindows();
    void ShowToolbarsForAllWindows();
    void SetLargeIcons(bool);
    bool UsesLargeIcons() const;

    void UpdateAnimationTimer();
    void StopTimer();

    void UpdateGlobalAtts() const;
    void UpdateViewAtts(int windowIndex = -1, bool updateCurve = true,
                        bool update2d = true, bool update3d = true,
                        bool updateAxisArray = true);
    void UpdateAnimationAtts();
    void UpdateAnnotationAtts();
    void UpdateLightListAtts();
    void UpdateWindowAtts();

    void UpdateWindowInformation(int flags, int windowIndex = -1);
    void UpdateWindowRenderingInformation(int windowIndex = -1);
    void UpdateViewKeyframeInformation();

    void UpdateRenderingAtts(int windowIndex = -1);
    void UpdateAllAtts();

    void BeginEngineExecute();
    void EndEngineExecute();

    void DisableExternalRenderRequestsAllWindows(std::vector<bool>& oldAble, bool bClearImage = false);
    void EnableExternalRenderRequestsAllWindows(const std::vector<bool> oldAble);

    bool SetTreatAllDBsAsTimeVarying(bool val);
    bool SetTryHarderCyclesTimes(bool val);

    bool SetCreateMeshQualityExpressions(bool val);
    bool SetCreateTimeDerivativeExpressions(bool val);
    bool SetCreateVectorMagnitudeExpressions(bool val);
    void SetPlotFollowsTime();

    void ReplaceDatabase(const EngineKey &key,
                         const std::string &database,
                         int timeState,
                         bool setTimeState,
                         bool onlyReplaceSame,
                         bool onlyReplaceActive);
    void CheckForNewStates(const std::string &hostDatabase);
    void CreateDatabaseCorrelation(const std::string &name,
                                   const stringVector &dbs,
                                   int method, int initialState,
                                   int nStates = -1);
    void AlterDatabaseCorrelation(const std::string &name,
                                  const stringVector &dbs,
                                  int method, int nStates = -1);
    void DeleteDatabaseCorrelation(const std::string &name);
    DatabaseCorrelation *CreateMultiWindowCorrelation(const intVector &windowIds);
    void CloseDatabase(const std::string &dbName);

    void CreateNode(DataNode *parentNode, 
                    const std::map<std::string, std::string> &, bool detailed);
    void SetFromNode(DataNode *parentNode,
                     const std::map<std::string, std::string> &,
                     const std::string &);
    static bool SessionContainsErrors(DataNode *);

    static GlobalAttributes              *GetClientAtts();
    static SaveWindowAttributes          *GetSaveWindowClientAtts();
    static ViewCurveAttributes           *GetViewCurveClientAtts();
    static ViewAxisArrayAttributes       *GetViewAxisArrayClientAtts();
    static View2DAttributes              *GetView2DClientAtts();
    static View3DAttributes              *GetView3DClientAtts();
    static AnimationAttributes           *GetAnimationClientAtts();
    static AnnotationAttributes          *GetAnnotationClientAtts();
    static AnnotationAttributes          *GetAnnotationDefaultAtts();
    static PrinterAttributes             *GetPrinterClientAtts();
    static ViewerWindowManagerAttributes *GetWindowAtts();
    static WindowInformation             *GetWindowInformation();
    static void                          SetClientAnnotationAttsFromDefault();
    static void                          SetDefaultAnnotationAttsFromClient();
    static KeyframeAttributes            *GetKeyframeClientAtts();
    static LightList                     *GetLightListClientAtts();
    static LightList                     *GetLightListDefaultAtts();
    static void                          SetClientLightListFromDefault();
    static void                          SetDefaultLightListFromClient();
    static void                          ViewCallback(VisWindow *);
    static RenderingAttributes           *GetRenderingAttributes();
    static void                          RenderInformationCallback(void *data);
    static AnnotationObjectList          *GetAnnotationObjectList();
    static AnnotationObjectList          *GetDefaultAnnotationObjectList();
    static void                          SetDefaultAnnotationObjectListFromClient();

    void SetInteractorAttsFromClient();
    void SetInteractorAttsFromDefault();
    void UpdateInteractorAtts();
    static InteractorAttributes          *GetInteractorClientAtts();
    static InteractorAttributes          *GetInteractorDefaultAtts();
    static void                          SetClientInteractorAttsFromDefault();
    static void                          SetDefaultInteractorAttsFromClient();

    void GetDatabasesForWindows(const intVector &,stringVector &, bool) const;

  signals:
    void createWindow(ViewerWindow *);
    void deleteWindow(ViewerWindow *);

  public slots:
    void InvertBackgroundColor(int windowIndex = -1);
    void SetToolEnabled(int toolId, bool enabled, int windowIndex = -1);
    void DisableAllTools(int windowIndex = -1);
    void UndoView(int windowIndex = -1);
    void RedoView(int windowIndex = -1);

  protected:
    ViewerWindowManager();
    avtImage_p CreateSingleImage(int windowIndex, int width, int height,
                                 bool screenCapture, bool leftEye);
    avtImage_p CreateTiledImage(int width, int height, bool leftEye);
    avtDataset_p GetDataset(int windowIndex, bool&);
    bool AskForCorrelationPermission(const QString &, const QString &,
                                     const stringVector &dbs) const;
    DatabaseCorrelation *CreateMultiWindowCorrelationHelper(const stringVector &dbs);
    static void ToolCallback(const avtToolInterface &);
    void       SynchronizeTimeLockedWindows(int, int);

  private:
    void InitWindowLimits();
    int  SimpleAddWindow();
    void CreateVisWindow(const int windowIndex, const int width,
                         const int height,
                         const int x, const int y);
    void SetWindowAttributes(int windowIndex, bool copyAtts);

    void SetPlotColors(const AnnotationAttributes *);

    static ViewerWindowManager           *instance;
    static ViewerWindowManagerAttributes *windowAtts;
    static WindowInformation             *windowInfo;
    static const int                     maxWindows;
    static const int                     maxLayouts;
    static const int                     validLayouts[];
    static GlobalAttributes              *clientAtts;
    static SaveWindowAttributes          *saveWindowClientAtts;
    static ViewCurveAttributes           *viewCurveClientAtts;
    static View2DAttributes              *view2DClientAtts;
    static View3DAttributes              *view3DClientAtts;
    static ViewAxisArrayAttributes       *viewAxisArrayClientAtts;
    static AnimationAttributes           *animationClientAtts;
    static AnnotationAttributes          *annotationClientAtts;
    static AnnotationAttributes          *annotationDefaultAtts;
    static KeyframeAttributes            *keyframeClientAtts;
    static LightList                     *lightListClientAtts;
    static LightList                     *lightListDefaultAtts;
    static PrinterAttributes             *printerAtts;
    static RenderingAttributes           *renderAtts;
    static AnnotationObjectList          *annotationObjectList;
    static AnnotationObjectList          *defaultAnnotationObjectList;
    static InteractorAttributes          *interactorClientAtts;
    static InteractorAttributes          *interactorDefaultAtts;

    int               layout;
    int               layoutIndex;

    int               activeWindow;

    int               screenWidth;
    int               screenHeight;
    int               screenX;
    int               screenY;
    int               borderTop;
    int               borderBottom;
    int               borderLeft;
    int               borderRight;
    int               shiftX;
    int               shiftY;
    int               preshiftX;
    int               preshiftY;

    int               filesWritten;
    avtFileWriter    *fileWriter;

    int               nWindows;
    ViewerWindow      **windows;
    int               *x_locations;
    int               *y_locations;
    bool              *referenced;

    bool              windowsHidden;
    bool              windowsIconified;

    int               lineoutWindow;
    int               timeQueryWindow;

    WindowLimits      **windowLimits;

    QTimer            *timer;
    int               animationTimeout;
    int               lastAnimation;

    bool              viewStacking;
private slots:
    void AnimationCallback();

};

#endif
