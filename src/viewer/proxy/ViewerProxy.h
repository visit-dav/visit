// ************************************************************************* //
//                                ViewerProxy.h                              //
// ************************************************************************* //

#ifndef VIEWER_PROXY_H
#define VIEWER_PROXY_H
#include <viewerproxy_exports.h>
#include <SimpleObserver.h>
#include <avtSILRestriction.h>
#include <vectortypes.h>

class AnimationAttributes;
class AnnotationAttributes;
class AppearanceAttributes;
class AttributeSubject;
class ColorTableAttributes;
class Connection;
class ExpressionList;
class EngineList;
class GlobalAttributes;
class GlobalLineoutAttributes;
class HostProfileList;
class KeyframeAttributes;
class LightList;
class MaterialAttributes;
class MessageAttributes;
class PickAttributes;
class PlotList;
class PluginManagerAttributes;
class PrinterAttributes;
class QueryAttributes;
class QueryList;
class RenderingAttributes;
class RemoteProcess;
class SaveWindowAttributes;
class StatusAttributes;
class SyncAttributes;
class InternalSILObserver;
class ViewAttributes;
class ViewerRPC;
class WindowInformation;
class Xfer;

// ****************************************************************************
//  Class: ViewerProxy
//
//  Purpose:
//    ViewerProxy is a proxy class for creating and controlling a viewer.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Aug 25 11:09:32 PDT 2000
//    I replaced LocalProcess with RemoteProcess since it can now launch
//    local processes.
//
//    Brad Whitlock, Thu Aug 31 14:48:00 PST 2000
//    I made the state objects (PcAttributes) an instance instead of
//    a pointer. This is so observers can be set up before the proxy's
//    remote process is created.
//
//    Eric Brugger, Tue Sep  5 10:21:18 PDT 2000
//    I changed the PlotType enumerated type so that PC_PLOT would be zero.
//    This is currently necessary since it is the only type implemented.
//    Once they are all implemented this will no longer be the case.
//
//    Eric Brugger, Fri Sep 15 11:22:49 PDT 2000
//    I added the methods GetAsliceAttributes and GetGlobalAttributes.
//
//    Eric Brugger, Mon Sep 18 11:42:45 PDT 2000
//    I added MAX_PLOT and MAX_OPERATOR which give a count of the number
//    of plots and operators.
//
//    Brad Whitlock, Tue Sep 19 18:50:18 PST 2000
//    I added a HostProfileList state object.
//
//    Brad Whitlock, Mon Sep 25 11:59:09 PDT 2000
//    I removed PlotType and OperType and put them in include files.
//
//    Brad Whitlock, Thu Sep 28 11:49:47 PDT 2000
//    I added a WriteConfigFile rpc.
//
//    Kathleen Bonnell, Wed Oct 11 08:38:57 PDT 2000
//    I added OnionPeelAttributes.
//
//    Eric Brugger, Wed Oct 25 14:42:06 PDT 2000
//    I removed the argument "name" from the Create method.
//
//    Brad Whitlock, Fri Nov 10 15:01:31 PST 2000
//    I added MaterialAttributes.
//
//    Brad Whitlock, Tue Nov 21 11:25:14 PDT 2000
//    I added a ConnectToMetaDataServer rpc.
//
//    Brad Whitlock, Wed Dec 13 11:06:46 PDT 2000
//    I added MaterialSelectAttributes.
//
//    Hank Childs, Wed Jan 10 11:55:30 PST 2001
//    Add volume attributes.
//
//    Hank Childs, Sun Feb 11 19:00:06 PST 2001
//    Add save window rpc.
//
//    Brad Whitlock, Fri Feb 9 14:20:11 PST 2001
//    Added SaveImageAttributes.
//
//    Brad Whitlock, Fri Feb 16 13:34:34 PST 2001
//    Added ContourAttributes.
//
//    Kathleen Bonnell, Tue Mar  6 10:25:25 PST 2001 
//    Added SurfaceAttributes.
//
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    Modified to treat plots generically.
//
//    Brad Whitlock, Thu Apr 19 10:54:33 PDT 2001
//    Added methods to handle window iconification.
//
//    Brad Whitlock, Mon Apr 23 09:36:25 PDT 2001
//    Added state objects that can be observed to get error messages and
//    status information.
//
//    Brad Whitlock, Tue Apr 24 10:33:23 PDT 2001
//    Added consecutiveReadZeroes member.
//
//    Brad Whitlock, Mon Apr 30 12:17:31 PDT 2001
//    Added EngineList and StatusAttributes and methods for engine
//    interruption and termination.
//
//    Brad Whitlock, Mon Jun 11 14:18:06 PST 2001
//    Added colortable state object.
//
//    Brad Whitlock, Sun Jun 17 20:07:40 PST 2001
//    Added the AnnotationAttributes object.
//
//    Brad Whitlock, Thu Jun 21 13:00:35 PST 2001
//    Added methods to transfer SIL restrictions.
//
//    Hank Childs, Mon Jul 23 13:43:41 PDT 2001
//    Removed material select.
//
//    Jeremy Meredith, Thu Jul 26 03:11:41 PDT 2001
//    Removed all references to OperType.
//    Added support for real operator plugins.
//
//    Brad Whitlock, Thu Jul 19 15:56:39 PST 2001
//    Added methods to set the view.
//
//    Brad Whitlock, Tue Aug 14 14:52:39 PST 2001
//    Added methods to reset the plot and operator attributes.
//
//    Brad Whitlock, Thu Aug 30 09:52:12 PDT 2001
//    Added methods to set the default annotation attributes.
//
//    Jeremy Meredith, Wed Sep  5 14:06:21 PDT 2001
//    Added plugin manager attributes.
//
//    Brad Whitlock, Tue Sep 4 22:30:11 PST 2001
//    Added appearance attributes.
//
//    Brad Whitlock, Mon Sep 24 11:29:10 PDT 2001
//    Added a method to query the name of the local machine.
//
//    Jeremy Meredith, Fri Sep 28 13:41:57 PDT 2001
//    Added the LoadPlugins method.
//
//    Brad Whitlock, Fri Sep 14 13:45:59 PST 2001
//    Added a light list and RPC's to use the lights.
//
//    Eric Brugger, Mon Nov 19 13:29:49 PST 2001
//    Added animation attributes.
//
//    Brad Whitlock, Wed Sep 19 14:37:18 PST 2001
//    Added RPC's for disabling updates and redrawing the window.
//
//    Kathleen Bonnell, Wed Dec  5 13:42:07 PST 2001
//    Added pick attributes.
//
//    Brad Whitlock, Tue Jan 29 16:16:17 PST 2002
//    Added an RPC to set the window area for the vis windows.
//
//    Brad Whitlock, Wed Feb 20 13:58:29 PST 2002
//    Added printer attributes and RPCs for printing. Also added a method
//    to return the user name.
//
//    Sean Ahern, Tue Apr 16 12:31:39 PDT 2002
//    Added the ability to show/hide all windows.
//
//    Brad Whitlock, Mon May 6 16:32:43 PST 2002
//    Added a bunch of new methods to expose the functions in the
//    viswindow's popup menu.
//
//    Jeremy Meredith, Wed May  8 12:27:29 PDT 2002
//    Added keyframe attributes.
//
//    Hank Childs, Thu May 23 18:36:38 PDT 2002
//    Renamed saveImageAtts to saveWindowAtts.
//
//    Brad Whitlock, Thu Jun 27 16:13:55 PST 2002
//    Added copy window methods.
//
//    Brad Whitlock, Mon Jul 29 15:15:34 PST 2002
//    Added ReOpenDatabase method and ClearCache method.
//
//    Brad Whitlock, Mon Sep 16 12:37:42 PDT 2002
//    I added methods to clear reflines and set view extents. I also
//    added two new state objects.
//
//    Brad Whitlock, Fri Sep 6 13:52:50 PST 2002
//    I added query methods.
//
//    Brad Whitlock, Tue Oct 15 16:24:36 PST 2002
//    I added CloneWindow and CopyPlotsToWindow.
//
//    Jeremy Meredith, Thu Oct 24 16:03:25 PDT 2002
//    Added material options.
//
//    Brad Whitlock, Mon Nov 11 11:45:13 PDT 2002
//    I added methods to lock time and tools.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002 
//    Removed SetPickAttributes, SetDefaultPickAttributes, ResetPickAttributes.
//
//    Eric Brugger, Mon Nov 18 11:58:56 PST 2002
//    Added SetPlotFrameRange and DeletePlotKeyframe.
//
//    Brad Whitlock, Wed Nov 20 14:52:47 PST 2002
//    I changed some of the color table methods.
//
//    Hank Childs, Mon Dec  2 14:17:18 PST 2002
//    Used reference counted pointer with SIL Restriction to be consistent with
//    the rest of the code.
//
//    Eric Brugger, Mon Dec 30 11:15:26 PST 2002
//    Added SetPlotDatabaseState and DeletePlotDatabaseKeyframe.
//
//    Eric Brugger, Fri Jan  3 15:16:07 PST 2003
//    Added ClearViewKeyframes, DeleteViewKeyframe, SetViewKeyframe and
//    ToggleCameraViewMode.
//
//    Brad Whitlock, Thu Dec 19 11:41:48 PDT 2002
//    I added a security key argument to ConnectToMetaDataServer and made
//    changes to interface because ViewerRPC is now automatically generated.
//
//    Brad Whitlock, Mon Jan 13 08:42:12 PDT 2003
//    I added a method to open an mdserver.
//
//    Eric Brugger, Tue Jan 28 12:33:25 PST 2003
//    I added MovePlotKeyframe, MovePlotDatabaseKeyframe and MoveViewKeyframe.
//    I added a plotId argument to SetPlotDatabaseState,
//    DeletePlotDatabaseKeyframe, DeletePlotKeyframe and SetPlotFrameRange.
//
//    Brad Whitlock, Thu Feb 27 11:35:50 PDT 2003
//    I added the animationStopOpcode member.
//
//    Kathleen Bonnell, Tue Mar  4 13:27:11 PST 2003   
//    Added GlobalLineoutAttributes and Set/Get methods. 
//
//    Brad Whitlock, Wed Mar 12 10:45:35 PDT 2003
//    I added the iconifyOpcode member.
//
//    Brad Whitlock, Thu Apr 10 09:27:16 PDT 2003
//    I added PromoteOperator, DemoteOperator, RemoveOperator methods. I also
//    added an overloaded version of SetActivePlots.
//
//    Eric Brugger, Fri Apr 18 15:37:54 PDT 2003
//    I added ToggleMaintainViewMode and deleted ToggleAutoCenterMode.
//
//    Kathleen Bonnell, Thu May 15 10:00:02 PDT 2003  
//    I added ToggleFullFrameMode.
//
//    Brad Whitlock, Thu May 15 13:03:04 PST 2003
//    I added a default timeState argument to the OpenDatabase method.
//
//    Kathleen Bonnell, Tue Jul  1 09:34:37 PDT 2003  
//    Added SetPickAttributes.
//
//    Brad Whitlock, Tue Jul 1 16:45:31 PST 2003
//    I added a method to export color tables.
//
//    Brad Whitlock, Wed Jul 9 11:54:15 PDT 2003
//    I added methods to export and import the viewer's entire state.
//
//    Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//    Added 'samples' arg to LineQuery and Lineout methods.
//    Added int args to DatabaseQuery.
//    Added overloaded Pick and NodePick methods (accepting doubles).
//
//    Brad Whitlock, Wed Jul 30 14:44:34 PST 2003
//    Added an extra argument to ImportEntireState.
//
// ****************************************************************************

class VIEWER_PROXY_API ViewerProxy : public SimpleObserver
{
  public:
    ViewerProxy();
    virtual ~ViewerProxy();

    Connection *GetReadConnection() const;
    Connection *GetWriteConnection() const;
    const std::string &GetLocalHostName() const;
    const std::string &GetLocalUserName() const;
    void ProcessInput();

    void AddArgument(const std::string &arg);
    void Create();
    void Close();
    void LoadPlugins();

    void AddWindow();
    void CloneWindow();
    void DeleteWindow();
    void SetWindowLayout(int layout);
    void SetActiveWindow(int windowId);
    void IconifyAllWindows();
    void DeIconifyAllWindows();
    void ShowAllWindows();
    void HideAllWindows();
    void ClearWindow();
    void ClearAllWindows();
    void SaveWindow();
    void PrintWindow();
    void DisableRedraw();
    void RedrawWindow();

    void ConnectToMetaDataServer(const std::string &hostName, const stringVector &argv);
    void OpenMDServer(const std::string &hostName, const stringVector &argv);

    void OpenDatabase(const std::string &database, int timeState = 0);
    void ReOpenDatabase(const std::string &database, bool forceClose = true);
    void ReplaceDatabase(const std::string &database);
    void OverlayDatabase(const std::string &database);
    void ClearCache(const std::string &hostName);

    void OpenComputeEngine(const std::string &hostName, const stringVector &argv);
    void CloseComputeEngine(const std::string &hostName);
    void InterruptComputeEngine(const std::string &hostName);

    void AnimationSetNFrames(int nFrames);
    void AnimationPlay();
    void AnimationReversePlay();
    void AnimationStop();
    void AnimationNextFrame();
    void AnimationPreviousFrame();
    void AnimationSetFrame(int frame);

    void AddPlot(int type, const std::string &var);
    void SetPlotFrameRange(int plotId, int frame0, int frame1);
    void DeletePlotKeyframe(int plotId, int frame);
    void MovePlotKeyframe(int plotId, int oldFrame, int newFrame);
    void SetPlotDatabaseState(int plotId, int frame, int state);
    void DeletePlotDatabaseKeyframe(int plotId, int frame);
    void MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame);
    void DeleteActivePlots();
    void HideActivePlots();
    void DrawPlots();
    void SetActivePlots(const intVector &activePlotIds);
    void SetActivePlots(const intVector &activePlotIds,
                        const intVector &activeOperatorIds,
                        const intVector &expandedPlots);
    void ChangeActivePlotsVar(const std::string &var);

    void AddOperator(int oper);
    void PromoteOperator(int operatorId);
    void DemoteOperator(int operatorId);
    void RemoveOperator(int operatorId);
    void RemoveLastOperator();
    void RemoveAllOperators();

    void SetDefaultPlotOptions(int type);
    void SetPlotOptions(int type);
    void ResetPlotOptions(int type);
    void SetDefaultOperatorOptions(int oper);
    void SetOperatorOptions(int oper);
    void ResetOperatorOptions(int type);

    void SetActiveContinuousColorTable(const std::string &colorTableName);
    void SetActiveDiscreteColorTable(const std::string &colorTableName);
    void DeleteColorTable(const std::string &colorTableName);
    void UpdateColorTable(const std::string &colorTableName);
    void ExportColorTable(const std::string &colorTableName);
    void InvertBackgroundColor();

    void SetView2D();
    void SetView3D();
    void ClearViewKeyframes();
    void DeleteViewKeyframe(int frame);
    void MoveViewKeyframe(int oldFrame, int newFrame);
    void SetViewKeyframe();
    void ResetView();
    void RecenterView();
    void SetViewExtentsType(int t);
    void ToggleMaintainViewMode();
    void UndoView();
    void ToggleLockViewMode();
    void ToggleLockTime();
    void ToggleLockTools();
    void ToggleSpinMode();
    void ToggleCameraViewMode();
    void ToggleFullFrameMode();

    void SetWindowMode(int mode);
    void ToggleBoundingBoxMode();
    void EnableTool(int tool, bool enabled);

    void CopyViewToWindow(int from, int to);
    void CopyLightingToWindow(int from, int to);
    void CopyAnnotationsToWindow(int from, int to);
    void CopyPlotsToWindow(int from, int to);

    void SetAnnotationAttributes();
    void SetDefaultAnnotationAttributes();
    void ResetAnnotationAttributes();
    void SetKeyframeAttributes();

    void SetMaterialAttributes();
    void SetDefaultMaterialAttributes();
    void ResetMaterialAttributes();

    void SetLightList();
    void SetDefaultLightList();
    void ResetLightList();

    void SetAnimationAttributes();

    void SetAppearanceAttributes();
    void ProcessExpressions();

    void ClearPickPoints();
    void ClearReferenceLines();

    void SetRenderingAttributes();
    void SetWindowArea(int x, int y, int w, int h);

    void SetGlobalLineoutAttributes();
    void SetPickAttributes();

    void WriteConfigFile();
    void ExportEntireState(const std::string &filename);
    void ImportEntireState(const std::string &filename, bool inVisItDir);

    // Methods for dealing with plot SIL restrictions.
    avtSILRestriction_p GetPlotSILRestriction() 
                     { return internalSILRestriction; };
    avtSILRestriction_p GetPlotSILRestriction() const
                     { return new avtSILRestriction(internalSILRestriction); };
    void SetPlotSILRestriction(avtSILRestriction_p newRestriction);
    void SetPlotSILRestriction();

    // Methods for querying
    void DatabaseQuery(const std::string &queryName, const stringVector &vars,
                       const int arg1 = 0, const int arg2 = 0);
    void PointQuery(const std::string &queryName, const double pt[3],
                    const stringVector &vars);
    void LineQuery(const std::string &queryName, const double pt1[3],
                   const double pt2[3], const stringVector &vars,
                   const int samples);
    void Pick(int x, int y, const stringVector &vars);
    void Pick(double xyz[3], const stringVector &vars);
    void NodePick(int x, int y, const stringVector &vars);
    void NodePick(double xyz[3], const stringVector &vars);
    void Lineout(const double p0[3], const double p1[3],
                 const stringVector &vars, const int samples);

    // Methods for returning pointers to state obects.
    AnimationAttributes        *GetAnimationAttributes() const 
                                    {return animationAtts;};
    AnnotationAttributes       *GetAnnotationAttributes() const 
                                    {return annotationAtts;};
    AppearanceAttributes       *GetAppearanceAttributes() const 
                                    {return appearanceAtts;};
    ColorTableAttributes       *GetColorTableAttributes() const 
                                    {return colorTableAtts;};
    EngineList                 *GetEngineList() const 
                                    {return engineList;};
    ExpressionList             *GetExpressionList() const 
                                    {return exprList;};
    GlobalAttributes           *GetGlobalAttributes() const 
                                    {return globalAtts;};
    HostProfileList            *GetHostProfileList() const 
                                    {return hostProfiles;};
    KeyframeAttributes         *GetKeyframeAttributes() const
                                    {return keyframeAtts;}
    LightList                  *GetLightList() const 
                                    { return lightList; };
    MessageAttributes          *GetMessageAttributes() const 
                                    {return messageAtts;};
    AttributeSubject           *GetOperatorAttributes(int type) const;
    PickAttributes             *GetPickAttributes() const 
                                    {return pickAtts;};
    QueryAttributes            *GetQueryAttributes() const 
                                    {return queryAtts;};
    AttributeSubject           *GetPlotAttributes(int type) const;
    PlotList                   *GetPlotList() const 
                                    {return plotList;};
    PluginManagerAttributes    *GetPluginManagerAttributes() const 
                                    {return pluginManagerAttributes;};
    PrinterAttributes          *GetPrinterAttributes() const 
                                    {return printerAtts;};
    SaveWindowAttributes       *GetSaveWindowAttributes() const 
                                    {return saveWindowAtts;};
    SILRestrictionAttributes   *GetSILRestrictionAttributes() const 
                                    {return silRestrictionAtts;};
    StatusAttributes           *GetStatusAttributes() const 
                                    {return statusAtts;};
    SyncAttributes             *GetSyncAttributes() const
                                    { return syncAtts; };
    ViewAttributes             *GetView2DAttributes() const 
                                    {return view2DAttributes;};
    ViewAttributes             *GetView3DAttributes() const 
                                    {return view3DAttributes;};
    WindowInformation          *GetWindowInformation() const
                                    {return windowInfo; };
    RenderingAttributes        *GetRenderingAttributes() const
                                    {return renderAtts; };
    QueryList                  *GetQueryList() const
                                    {return queryList; };
    MaterialAttributes         *GetMaterialAttributes() const
                                    {return materialAtts;}
    GlobalLineoutAttributes    *GetGlobalLineoutAttributes() const 
                                    {return globalLineoutAtts;};
  protected:
    virtual void Update(Subject *subj);
  private:
    RemoteProcess              *viewer;
    Xfer                       *xfer;
    ViewerRPC                  *viewerRPC;

    int                        nPlots;
    int                        nOperators;
    int                        animationStopOpcode;
    int                        iconifyOpcode;

    // State objects
    SyncAttributes             *syncAtts;
    GlobalAttributes           *globalAtts;
    PlotList                   *plotList;
    ColorTableAttributes       *colorTableAtts;
    ExpressionList             *exprList;
    HostProfileList            *hostProfiles;
    MessageAttributes          *messageAtts;
    SaveWindowAttributes       *saveWindowAtts;
    StatusAttributes           *statusAtts;
    EngineList                 *engineList;
    AnnotationAttributes       *annotationAtts;
    SILRestrictionAttributes   *silRestrictionAtts;
    ViewAttributes             *view2DAttributes;
    ViewAttributes             *view3DAttributes;
    LightList                  *lightList;
    MaterialAttributes         *materialAtts;
    AnimationAttributes        *animationAtts;
    PluginManagerAttributes    *pluginManagerAttributes;
    AppearanceAttributes       *appearanceAtts;
    PickAttributes             *pickAtts;
    PrinterAttributes          *printerAtts;
    KeyframeAttributes         *keyframeAtts;
    WindowInformation          *windowInfo;
    RenderingAttributes        *renderAtts;
    QueryList                  *queryList;
    QueryAttributes            *queryAtts;
    GlobalLineoutAttributes    *globalLineoutAtts;

    AttributeSubject           **plotAtts;
    AttributeSubject           **operatorAtts;

    // Extra command line arguments to pass to the viewer.
    stringVector               argv;

    // Used to store the sil restriction in avt format.
    avtSILRestriction_p        internalSILRestriction;
};

#endif
