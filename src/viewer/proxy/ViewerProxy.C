// ************************************************************************* //
//                                ViewerProxy.C                              //
// ************************************************************************* //

#include <ViewerProxy.h>

#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>

#include <DebugStream.h>
#include <RemoteProcess.h>
#include <SocketConnection.h>
#include <ViewerRPC.h>
#include <VisItException.h>
#include <Xfer.h>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <ColorTableAttributes.h>
#include <ExpressionList.h>
#include <EngineList.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfile.h>
#include <HostProfileList.h>
#include <KeyframeAttributes.h>
#include <LightList.h>
#include <MaterialAttributes.h>
#include <MessageAttributes.h>
#include <PickAttributes.h>
#include <PlotList.h>
#include <PluginManagerAttributes.h>
#include <PrinterAttributes.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <StatusAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SyncAttributes.h>
#include <ViewCurveAttributes.h>
#include <View2DAttributes.h>
#include <View3DAttributes.h>
#include <WindowInformation.h>

#include <snprintf.h>

// ****************************************************************************
//  Method: ViewerProxy constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr 23 09:41:38 PDT 2001
//    Added MessageAttributes.
//
//    Brad Whitlock, Tue Apr 24 10:36:47 PDT 2001
//    Initialized read error counter.
//
//    Brad Whitlock, Mon Apr 30 14:05:04 PST 2001
//    Added statusAtts and engineList.
//
//    Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001 
//    Added the allocation of eraseAtts.
//
//    Brad Whitlock, Mon Jun 11 14:24:36 PST 2001
//    Added colorTableAtts.
//
//    Brad Whitlock, Sun Jun 17 20:11:42 PST 2001
//    Added AnnotationAttributes.
//
//    Brad Whitlock, Thu Jun 21 12:55:45 PDT 2001
//    Added methods, etc for setting SIL restrictions.
//
//    Hank Childs, Mon Jul 23 13:43:41 PDT 2001
//    Removed material selection.
//
//    Jeremy Meredith, Thu Jul 26 03:12:40 PDT 2001
//    Added support for real operator plugins.
//
//    Jeremy Meredith, Thu Jul 26 09:53:57 PDT 2001
//    Renamed plugin info to include the word "plot".
//
//    Brad Whitlock, Wed Jul 25 16:57:07 PST 2001
//    Added ViewAttributes.
//
//    Jeremy Meredith, Wed Sep  5 14:06:33 PDT 2001
//    Added plugin manager attributes initialization.
//
//    Brad Whitlock, Tue Sep 4 22:34:13 PST 2001
//    Added appearanceAtts.
//
//    Brad Whitlock, Mon Sep 17 10:55:43 PDT 2001
//    Added syncAtts.
//
//    Brad Whitlock, Fri Sep 14 13:48:14 PST 2001
//    Added lightList.
//
//    Sean Ahern, Fri Sep 14 15:30:51 PDT 2001
//    Added ExpressionList.
//
//    Jeremy Meredith, Fri Sep 28 13:43:37 PDT 2001
//    No longer load plugins here.
//
//    Eric Brugger, Mon Nov 19 13:31:48 PST 2001
//    Added AnimationAttributes.
//
//    Kathleen Bonnell, Wed Dec  5 13:42:07 PST 2001
//    Added pick attributes.
//
//    Brad Whitlock, Wed Feb 20 14:26:39 PST 2002
//    Added printer attributes.
//
//    Brad Whitlock, Mon Mar 25 12:52:32 PDT 2002
//    I removed the zero length read counter.
//
//    Hank Childs, Thu May 23 18:36:38 PDT 2002
//    Renamed saveImageAtts to saveWindowAtts.
//
//    Brad Whitlock, Mon Sep 16 14:28:50 PST 2002
//    I added WindowInformation and RenderingAttributes.
//
//    Brad Whitlock, Fri Sep 6 13:57:19 PST 2002
//    I added a query list.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Brad Whitlock, Thu Feb 27 11:36:51 PDT 2003
//    I initialized animationStopOpcode.
//
//    Kathleen Bonnell, Tue Mar  4 09:57:44 PST 2003 
//    Added globalLineoutAtts. 
//
//    Brad Whitlock, Wed Mar 12 10:45:56 PDT 2003
//    I added iconifyOpcode.
//
//    Eric Brugger, Wed Aug 20 10:45:18 PDT 2003
//    I added viewCurveAttributes.  I split the view attributes into 2d
//    and 3d parts.
//
//    Brad Whitlock, Wed Oct 29 10:57:21 PDT 2003
//    I added annotationObjectList.
//
// ****************************************************************************

ViewerProxy::ViewerProxy() : SimpleObserver(), argv()
{
    //
    // Create the state objects.
    //
    syncAtts             = new SyncAttributes;
    hostProfiles         = new HostProfileList;
    globalAtts           = new GlobalAttributes;
    plotList             = new PlotList;
    messageAtts          = new MessageAttributes;
    saveWindowAtts       = new SaveWindowAttributes;
    statusAtts           = new StatusAttributes;
    engineList           = new EngineList;
    colorTableAtts       = new ColorTableAttributes;
    exprList             = new ExpressionList;
    annotationAtts       = new AnnotationAttributes;
    silRestrictionAtts   = new SILRestrictionAttributes;
    viewCurveAttributes  = new ViewCurveAttributes;
    view2DAttributes     = new View2DAttributes;
    view3DAttributes     = new View3DAttributes;
    lightList            = new LightList;
    animationAtts        = new AnimationAttributes;
    pluginManagerAttributes = new PluginManagerAttributes;
    appearanceAtts       = new AppearanceAttributes;
    pickAtts             = new PickAttributes;
    printerAtts          = new PrinterAttributes;
    keyframeAtts         = new KeyframeAttributes;
    windowInfo           = new WindowInformation;
    renderAtts           = new RenderingAttributes;
    queryList            = new QueryList;
    queryAtts            = new QueryAttributes;
    materialAtts         = new MaterialAttributes;
    globalLineoutAtts    = new GlobalLineoutAttributes;
    annotationObjectList = new AnnotationObjectList;

    // Make the proxy observe the SIL restriction attributes.
    silRestrictionAtts->Attach(this);

    // We start with no plugins
    nPlots = 0;
    nOperators = 0;
    plotAtts = NULL;
    operatorAtts = NULL;

    animationStopOpcode = 0;
    iconifyOpcode = 0;
}

// ****************************************************************************
//  Method: ViewerProxy destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Sep 7 13:59:20 PST 2000
//    I added code to free the memory for the extra arguments.
//
//    Eric Brugger, Fri Sep 15 11:22:49 PDT 2000
//    Added the deletion of globalAtts and asliceAtts.
//
//    Kathleen Bonnell, Wed Oct 11 08:40:50 PDT 2000
//    I added onionpeelAtts.
//
//    Brad Whitlock, Fri Nov 10 15:04:44 PST 2000
//    I added materialAtts.
//
//    Brad Whitlock, Wed Dec 13 11:02:52 PDT 2000
//    I added matselAtts.
//
//    Hank Childs, Wed Jan 10 11:55:30 PST 2001
//    Added volume attributes.
//
//    Brad Whitlock, Fri Feb 9 14:22:56 PST 2001
//    Added save image attributes.
//
//    Brad Whitlock, Fri Feb 16 13:36:24 PST 2001
//    Added ContourAttributes.
//
//    Kathleen Bonnell, Tue Mar  6 10:25:25 PST 2001 
//    Added SurfaceAttributes.
//
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I modified the routine to handle plots generically.
//
//    Brad Whitlock, Mon Apr 23 09:42:03 PDT 2001
//    Added MessageAttributes.
//
//    Brad Whitlock, Mon Apr 30 14:06:16 PST 2001
//    Added statusAtts and engineList.
//
//    Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001 
//    Added the deletion of eraseAtts.
//
//    Brad Whitlock, Mon Jun 11 14:25:10 PST 2001
//    Added colorTableAtts.
//
//    Brad Whitlock, Sun Jun 17 20:12:32 PST 2001
//    Added AnnotationAttributes.
//
//    Brad Whitlock, Thu Jun 21 12:56:31 PDT 2001
//    Added silRestrictionAtts.
//
//    Hank Childs, Mon Jul 23 13:43:41 PDT 2001
//    Remove matsel attributes.
//
//    Jeremy Meredith, Thu Jul 26 03:13:02 PDT 2001
//    Added support for real operator plugins.
//
//    Brad Whitlock, Wed Jul 25 16:57:52 PST 2001
//    Added the view attributes.
//
//    Jeremy Meredith, Wed Sep  5 14:09:07 PDT 2001
//    Added plugin manager attributes.
//
//    Brad Whitlock, Tue Sep 4 22:35:33 PST 2001
//    Added appearance attributes.
//
//    Brad Whitlock, Fri Sep 14 13:49:15 PST 2001
//    Added the lightlist.
//
//    Sean Ahern, Fri Sep 14 15:31:19 PDT 2001
//    Added ExpressionList.
//
//    Brad Whitlock, Mon Sep 17 10:56:21 PDT 2001
//    Added syncAtts.
//
//    Eric Brugger, Mon Nov 19 13:31:48 PST 2001
//    Added AnimationAttributes.
//
//    Kathleen Bonnell, Wed Dec  5 13:42:07 PST 2001
//    Added pick attributes.
//
//    Brad Whitlock, Wed Feb 20 14:25:52 PST 2002
//    Added printerAtts;
//
//    Hank Childs, Thu May 23 18:36:38 PDT 2002
//    Renamed saveImageAtts to saveWindowAtts.
//
//    Brad Whitlock, Mon Sep 16 14:29:18 PST 2002
//    I added WindowInformation and RenderingAttributes.
//
//    Brad Whitlock, Fri Sep 6 13:57:48 PST 2002
//    I added a query list.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Brad Whitlock, Fri Dec 20 11:52:30 PDT 2002
//    Changed argv to a stringVector so we don't have to delete storage.
//
//    Kathleen Bonnell, Wed Feb 19 11:49:13 PST 2003  
//    Added globalLineoutAtts. 
//
//    Eric Brugger, Wed Aug 20 10:45:18 PDT 2003
//    I added viewCurveAttributes.
//
//    Brad Whitlock, Wed Oct 29 10:57:21 PDT 2003
//    I added annotationObjectList.
//
// ****************************************************************************

ViewerProxy::~ViewerProxy()
{
    int i;

    delete viewer;
    delete xfer;
    delete viewerRPC;

    //
    // Delete the state objects.
    //
    delete hostProfiles;
    delete globalAtts;
    delete plotList;
    delete messageAtts;
    delete saveWindowAtts;
    delete statusAtts;
    delete engineList;
    delete colorTableAtts;
    delete exprList;
    delete syncAtts;
    delete annotationAtts;
    delete silRestrictionAtts;
    delete viewCurveAttributes;
    delete view2DAttributes;
    delete view3DAttributes;
    delete lightList;
    delete animationAtts;
    delete pluginManagerAttributes;
    delete appearanceAtts;
    delete pickAtts;
    delete printerAtts;
    delete keyframeAtts;
    delete windowInfo;
    delete renderAtts;
    delete queryList;
    delete queryAtts;
    delete materialAtts;
    delete globalLineoutAtts;
    delete annotationObjectList;

    //
    // Delete the plot attribute state objects.
    //
    for (i = 0; i < nPlots; i++)
    {
        delete plotAtts[i];
    }
    delete [] plotAtts;
    nPlots   = 0;

    //
    // Delete the operator attribute state objects.
    //
    for (i = 0; i < nOperators; i++)
    {
        delete operatorAtts[i];
    }
    delete [] operatorAtts;
    nOperators   = 0;
}

// ****************************************************************************
// Method: ViewerProxy::Update
//
// Purpose: 
//   This method is called when objects that the ViewerProxy observes are
//   updated. This is an internal method and is primarily a means to update
//   the internal SIL restriction object.
//
// Arguments:
//   subj : A pointer to the subject that updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:23:50 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Create a new SIL restriction as a reference counted object from the heap.
//
// ****************************************************************************

void
ViewerProxy::Update(Subject *subj)
{
    // If the SIL restriction attributes have changed, update the internal
    // SIL restriction object internalSILRestriction. This is done so the
    // internalSILRestriction is available to the client and we have to
    // make the conversion far less often.
    if(subj == silRestrictionAtts)
    {
        internalSILRestriction = new avtSILRestriction(*silRestrictionAtts);
    }
}

// ****************************************************************************
//  Method: ViewerProxy::GetReadConnection
//
//  Purpose:
//    Get the connection that the viewer reads from.
//
//  Returns:    The connection the viewer reads from.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
// ****************************************************************************

Connection *
ViewerProxy::GetReadConnection() const
{
    if (viewer == 0)
        return 0;

    return viewer->GetReadConnection();
}

// ****************************************************************************
//  Method: ViewerProxy::GetWriteConnection
//
//  Purpose:
//    Get the connection that the viewer writes to.
//
//  Returns:    The connection the viewer writes to.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
// ****************************************************************************

Connection *
ViewerProxy::GetWriteConnection() const
{
    if (viewer == 0)
        return 0;

    return viewer->GetWriteConnection();
}

// ****************************************************************************
// Method: ViewerProxy::GetLocalHostName
//
// Purpose: 
//   Returns the name of the local machine.
//
// Returns:    The name of the local machine.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 11:32:45 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ViewerProxy::GetLocalHostName() const
{
    if(viewer == 0)
    {
        EXCEPTION1(VisItException, "Viewer not created.");
    }

    return viewer->GetLocalHostName();
}

// ****************************************************************************
// Method: ViewerProxy::GetLocalUserName
//
// Purpose: 
//   Returns the local user name.
//
// Returns:    The local user name.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 21 10:06:52 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

const std::string &
ViewerProxy::GetLocalUserName() const
{
    if(viewer == 0)
    {
        EXCEPTION1(VisItException, "Viewer not created.");
    }

    return viewer->GetLocalUserName();
}

// ****************************************************************************
//  Method: ViewerProxy::ProcessInput
//
//  Purpose:
//    Read data from the viewer and process it.
//
//  Programmer: Eric Brugger
//  Creation:   August 22, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Apr 24 10:35:56 PDT 2001
//    Added code to detect that the viewer died.
//
//    Brad Whitlock, Wed Mar 20 17:45:28 PST 2002
//    I abstracted the read code.
//
// ****************************************************************************

void
ViewerProxy::ProcessInput()
{
    //
    // Try and read from the viewer.
    //
    int amountRead = viewer->GetWriteConnection()->Fill();

    //
    // Try and process the input.
    //
    if (amountRead > 0)
        xfer->Process();
}

// ****************************************************************************
// Method: ViewerProxy::AddArgument
//
// Purpose: 
//   Adds an argument to the viewer's argument list.
//
// Arguments:
//   arg : The argument to be added.
//
// Note:
//   This only has an effect if called before the Create call.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 13:48:12 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Dec 20 11:53:23 PDT 2002
//   I made argv be a stringVector.
//
// ****************************************************************************

void
ViewerProxy::AddArgument(const std::string &arg)
{
    if(arg.size() == 0)
        return;

    argv.push_back(arg);
}

// ****************************************************************************
//  Method: ViewerProxy::Create
//
//  Purpose:
//    Create a viewer.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Aug 25 11:14:07 PDT 2000
//    I changed the code so it uses RemoteProcess. When "localhost" is
//    passed to RemoteProcess::Open, it opens a local process without ssh.
//
//    Brad Whitlock, Thu Aug 31 14:52:51 PST 2000
//    I removed the code that create the attribute subjects.
//
//    Brad Whitlock, Thu Sep 7 13:57:13 PST 2000
//    I added code to add arguments to the viewer's RemoteProcess class
//    before launching it.
//
//    Eric Brugger, Fri Sep 15 11:22:49 PDT 2000
//    I connected globalAtts and asliceAtts to the xfer object.
//
//    Brad Whitlock, Tue Sep 19 18:54:05 PST 2000
//    I connected the profileList.
//
//    Kathleen Bonnell, Wed Oct 11 08:40:50 PDT 2000
//    I connected onionpeelAtts to the xfer object.
//
//    Eric Brugger, Wed Oct 25 14:29:27 PDT 2000
//    I removed the argument "name" and modified the routine to launch
//    the viewer using "visit -viewer".
//
//    Brad Whitlock, Fri Nov 10 15:06:06 PST 2000
//    I connected materialAtts to xfer.
//
//    Brad Whitlock, Wed Dec 13 13:02:25 PST 2000
//    I connected the matselAtts to xfer.
//
//    Eric Brugger, Thu Dec 21 11:52:39 PST 2000
//    I changed the order in which the AttributeSubjects are connected to
//    the viewer to match the new order in the viewer.
//
//    Hank Childs, Wed Jan 10 12:10:40 PST 2001
//    Added new attributes for volume plots.
//
//    Brad Whitlock, Fri Feb 9 14:23:51 PST 2001
//    Added save image attributes.
//
//    Brad Whitlock, Fri Feb 16 13:36:51 PST 2001
//    Added ContourAttributes.
//
//    Kathleen Bonnell, Tue Mar  6 10:25:25 PST 2001 
//    Added SurfaceAttributes.
//
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I modified the routine to handle plots generically.
//
//    Brad Whitlock, Mon Apr 23 09:43:48 PDT 2001
//    Added MessageAttributes.
//
//    Brad Whitlock, Mon Apr 30 14:06:38 PST 2001
//    Added engineList and statusAtts.
//
//    Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001 
//    I connected eraseAtts to the xfer object.
//
//    Brad Whitlock, Mon Jun 11 14:25:45 PST 2001
//    Added colorTableAtts.
//
//    Brad Whitlock, Thu Jun 21 12:58:20 PDT 2001
//    Added silRestrictionAtts.
//
//    Hank Childs, Mon Jul 23 13:43:41 PDT 2001
//    Removed material selection from xfer.
//
//    Jeremy Meredith, Thu Jul 26 03:13:22 PDT 2001
//    Added support for real operator plugins.
//
//    Brad Whitlock, Wed Jul 25 16:58:40 PST 2001
//    Added the view attributes.
//
//    Jeremy Meredith, Wed Sep  5 14:07:52 PDT 2001
//    Added plugin manager attributes.
//
//    Brad Whitlock, Tue Sep 4 22:36:10 PST 2001
//    Added appearance attributes.
//
//    Brad Whitlock, Fri Sep 14 13:49:43 PST 2001
//    Added the light list.
//
//    Sean Ahern, Fri Sep 14 15:31:41 PDT 2001
//    Added expression list.
//
//    Jeremy Meredith, Fri Sep 28 13:44:53 PDT 2001
//    Added plugin manager attributes.  Also, don't attach plugins
//    here since they probably won't be loaded yet.
//
//    Eric Brugger, Mon Nov 19 13:31:48 PST 2001
//    Added AnimationAttributes.
//
//    Brad Whitlock, Mon Sep 17 10:57:40 PDT 2001
//    Added syncAtts.
//
//    Kathleen Bonnell, Wed Dec  5 13:42:07 PST 2001
//    Added pick attributes.
//
//    Brad Whitlock, Wed Feb 20 14:27:02 PST 2002
//    Added print attributes.
//
//    Hank Childs, Thu May 23 18:36:38 PDT 2002
//    Renamed saveImageAtts to saveWindowAtts.
//
//    Brad Whitlock, Mon Sep 16 14:29:46 PST 2002
//    I added WindowInformation and RenderingAttributes.
//
//    Brad Whitlock, Fri Sep 6 13:58:11 PST 2002
//    I added a query list.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Brad Whitlock, Thu Feb 27 11:34:24 PDT 2003
//    I added support for special opcodes.
//
//    Kathleen Bonnell, Tue Mar  4 09:57:44 PST 2003 
//    Added globalLineoutAtts. 
//
//    Brad Whitlock, Wed Mar 12 10:46:42 PDT 2003
//    I added iconifyOpcode.
//
//    Eric Brugger, Wed Aug 20 10:45:18 PDT 2003
//    I added viewCurveAttributes.
//
//    Jeremy Meredith, Thu Oct  9 15:51:06 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Brad Whitlock, Wed Oct 29 10:58:39 PDT 2003
//    Added annotationOptionsList.
//
// ****************************************************************************

void
ViewerProxy::Create()
{
    //
    // Create the viewer process.  The viewer is executed using
    // "visit -viewer".
    //
    viewer = new RemoteProcess(std::string("visit"));
    viewer->AddArgument(std::string("-viewer"));

    //
    // Add any extra arguments to the viewer before opening it.
    //
    for (int i = 0; i < argv.size(); ++i)
        viewer->AddArgument(argv[i]);

    //
    // Open the viewer.
    //
    viewer->Open("localhost",
                 HostProfile::MachineName, "", 
                 false, 0,
                 1, 1);

    //
    // Form the xfer object for the RPCs.
    //
    xfer      = new Xfer;
    viewerRPC = new ViewerRPC;
    xfer->Add(viewerRPC);

    //
    // Hook up the viewer's SocketConnections to the xfer object.
    //
    xfer->SetInputConnection(viewer->GetWriteConnection());
    xfer->SetOutputConnection(viewer->GetReadConnection());

    //
    // Attach the AttributeSubjects to the xfer object.
    //
    xfer->Add(syncAtts);
    xfer->Add(appearanceAtts);
    xfer->Add(pluginManagerAttributes);
    xfer->Add(globalAtts);
    xfer->Add(plotList);
    xfer->Add(hostProfiles);
    xfer->Add(messageAtts);
    xfer->Add(saveWindowAtts);
    xfer->Add(statusAtts);
    xfer->Add(engineList);
    xfer->Add(colorTableAtts);
    xfer->Add(exprList);
    xfer->Add(annotationAtts);
    xfer->Add(silRestrictionAtts);
    xfer->Add(viewCurveAttributes);
    xfer->Add(view2DAttributes);
    xfer->Add(view3DAttributes);
    xfer->Add(lightList);
    xfer->Add(animationAtts);
    xfer->Add(pickAtts);
    xfer->Add(printerAtts);
    xfer->Add(windowInfo);
    xfer->Add(renderAtts);
    xfer->Add(keyframeAtts);
    xfer->Add(queryList);
    xfer->Add(queryAtts);
    xfer->Add(materialAtts);
    xfer->Add(globalLineoutAtts);
    xfer->Add(annotationObjectList);

    xfer->ListObjects();

    // Create the special opcodes.
    animationStopOpcode = xfer->CreateNewSpecialOpcode();
    iconifyOpcode = xfer->CreateNewSpecialOpcode();
}

// ****************************************************************************
//  Method:  ViewerProxy::LoadPlugins
//
//  Purpose:
//    Unload and reload all the plugins
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 26, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 25 13:00:10 PST 2002
//    I added code to print the list of connected objects to the log file.
//
//    Jeremy Meredith, Wed Aug 21 12:51:28 PDT 2002
//    I renamed some plot/operator plugin manager methods for refactoring.
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow (since there is a corresponding
//    LoadPluginsOnDemand).
// ****************************************************************************

void
ViewerProxy::LoadPlugins()
{
    int i;
    PlotPluginManager     *pMgr = PlotPluginManager::Instance();
    OperatorPluginManager *oMgr = OperatorPluginManager::Instance();

    if (nPlots > 0 || nOperators > 0)
    {
        debug1 << "Reloading plugins at runtime not supported\n";
        return;
    }

    //
    // Go through the plugin atts and en/disable the ones specified
    // by the plugin attributes
    //
    for (i=0; i<pluginManagerAttributes->GetId().size(); i++)
    {
        if (! pluginManagerAttributes->GetEnabled()[i]) // not enabled
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (pMgr->PluginExists(id))
                    pMgr->DisablePlugin(id);
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (oMgr->PluginExists(id))
                    oMgr->DisablePlugin(id);
            }
        }
        else // is enabled -- it had better be available
        {
            std::string id = pluginManagerAttributes->GetId()[i];
            if (pluginManagerAttributes->GetType()[i] == "plot")
            {
                if (pMgr->PluginExists(id))
                    pMgr->EnablePlugin(id);
                else
                    EXCEPTION1(VisItException,
                               "A plot plugin enabled by the viewer "
                               "was not availabe in the GUI.");
            }
            else if (pluginManagerAttributes->GetType()[i] == "operator")
            {
                if (oMgr->PluginExists(id))
                    oMgr->EnablePlugin(id);
                else
                    EXCEPTION1(VisItException,
                               "A operator plugin enabled by the viewer "
                               "was not availabe in the GUI.");
            }
        }
    }

    //
    // Now load dynamic libraries
    //
    pMgr->LoadPluginsNow();
    oMgr->LoadPluginsNow();

    //
    // Initialize the plot attribute state objects.
    //
    nPlots = pMgr->GetNEnabledPlugins();
    plotAtts = new AttributeSubject*[nPlots];
    for (i = 0; i < nPlots; ++i)
    {
        CommonPlotPluginInfo *info =
            pMgr->GetCommonPluginInfo(pMgr->GetEnabledID(i));
        plotAtts[i] = info->AllocAttributes();
    }

    //
    // Initialize the operator attribute state objects.
    //
    nOperators = oMgr->GetNEnabledPlugins();
    operatorAtts = new AttributeSubject*[nOperators];
    for (i = 0; i < nOperators; ++i)
    {
        CommonOperatorPluginInfo *info = 
            oMgr->GetCommonPluginInfo(oMgr->GetEnabledID(i));
        operatorAtts[i] = info->AllocAttributes();
    }

    //
    // Attach the plot AttributeSubjects to the xfer object.
    //
    for (i = 0; i < nPlots; ++i)
    {
        xfer->Add(plotAtts[i]);
    }

    //
    // Attach the operator AttributeSubjects to the xfer object.
    //
    for (i = 0; i < nOperators; ++i)
    {
        xfer->Add(operatorAtts[i]);
    }

    //
    // Print the list of connected objects to the log file.
    //
    xfer->ListObjects();
}

// ****************************************************************************
//  Method: ViewerProxy::Close
//
//  Purpose:
//    Terminate the viewer.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Apr 3 12:50:01 PDT 2002
//    Called a new method of RemoteProcess to wait for the viewer to quit.
//
// ****************************************************************************

void
ViewerProxy::Close()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::CloseRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();

    //
    // Wait for the viewer to exit.
    //
    viewer->WaitForTermination();
}

// ****************************************************************************
//  Method: ViewerProxy::AddWindow
//
//  Purpose:
//    Add a window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::AddWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AddWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::CloneWindow
//
//  Purpose:
//    Clones the current window.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Oct 15 16:25:24 PST 2002
//
// ****************************************************************************

void
ViewerProxy::CloneWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::CloneWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DeleteWindow
//
//  Purpose:
//    Delete the active window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::DeleteWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeleteWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetWindowLayout
//
//  Purpose:
//    Set the window layout.
//
//  Arguments:
//    layout    The layout to use.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::SetWindowLayout(int layout)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetWindowLayoutRPC);
    viewerRPC->SetWindowLayout(layout);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetActiveWindow
//
//  Purpose:
//    Set the active window.
//
//  Arguments:
//    windowId  The identifier of the window to make active.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::SetActiveWindow(int windowId)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetActiveWindowRPC);
    viewerRPC->SetWindowId(windowId);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::IconifyAllWindows
//
// Purpose: 
//   Tells the viewer to iconify all of its windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:58:05 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 12 10:47:20 PDT 2003
//   I made the method also send a special opcode to immediately iconify
//   the windows.
//
// ****************************************************************************

void
ViewerProxy::IconifyAllWindows()
{
    //
    // Send a special opcode to stop the animation.
    //
    xfer->SendSpecialOpcode(iconifyOpcode);

    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::IconifyAllWindowsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::DeIconifyAllWindows
//
// Purpose: 
//   Tells the viewer to de-iconify all of its windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:58:37 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DeIconifyAllWindows()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeIconifyAllWindowsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ShowAllWindows
//
// Purpose: 
//   Tells the viewer to show all of its windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:32:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ShowAllWindows()
{
    // Set the rpc type and arguments.
    viewerRPC->SetRPCType(ViewerRPC::ShowAllWindowsRPC);

    // Issue the RPC.
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::HideAllWindows
//
// Purpose: 
//   Tells the viewer to hide all of its windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:32:33 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::HideAllWindows()
{
    // Set the rpc type and arguments.
    viewerRPC->SetRPCType(ViewerRPC::HideAllWindowsRPC);

    // Issue the RPC.
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ClearWindow
//
//  Purpose:
//    Clear the active window.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::ClearWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ClearWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ClearAllWindows
//
//  Purpose:
//    Clear all the windows.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************

void
ViewerProxy::ClearAllWindows()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ClearAllWindowsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ConnectToMetaDataServer
//
// Purpose: 
//   Tells the viewer's metadata server running on hostName to connect to the
//   program running on localHost that is listening on the specified port.
//
// Arguments:
//   hostName  : The host on which the mdserver is running.
//   args      : The arguments that tell the mdserver how to connect back.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 11:26:00 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 19 11:41:13 PDT 2002
//   I added the key argument.
//
//   Brad Whitlock, Mon May 5 14:10:09 PST 2003
//   I replaced several arguments with textual args.
//
// ****************************************************************************

void
ViewerProxy::ConnectToMetaDataServer(const std::string &hostName,
    const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ConnectToMetaDataServerRPC);
    viewerRPC->SetProgramHost(hostName);
    viewerRPC->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::OpenDatabase
//
//  Purpose:
//    Open a database.
//
//  Arguments:
//    database        : The name of the database to open.
//    timeState       : The timestate that we want to open.
//    addDefaultPlots : Whether we want to allow the viewer to add default
//                      plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Thu May 15 13:03:57 PST 2003
//    I made it possible to open a database at a later time state.
//
//    Brad Whitlock, Wed Oct 22 12:22:44 PDT 2003
//    I made it possible to tell the viewer that we don't want to add default
//    plots even if the database has them.
//   
// ****************************************************************************

void
ViewerProxy::OpenDatabase(const std::string &database, int timeState,
    bool addDefaultPlots)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::OpenDatabaseRPC);
    viewerRPC->SetDatabase(database);
    viewerRPC->SetIntArg1(timeState);
    viewerRPC->SetBoolFlag(addDefaultPlots);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ReOpenDatabase
//
//  Purpose:
//    Reopens a database.
//
//  Arguments:
//    database  The name of the database to reopen.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jul 29 15:16:37 PST 2002
//
// ****************************************************************************

void
ViewerProxy::ReOpenDatabase(const std::string &database, bool forceClose)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ReOpenDatabaseRPC);
    viewerRPC->SetDatabase(database);
    // Store the flag in the window layout.
    viewerRPC->SetWindowLayout(forceClose ? 1 : 0);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ReplaceDatabase
//
//  Purpose:
//    Replaces the open database with this database.
//
//  Arguments:
//    database  The name of the database to use.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Mar 6 16:07:47 PST 2002
//
//  Modifications:
//    Brad Whitlock, Wed Oct 15 15:37:44 PST 2003
//    I added an optional timeState argument so we can replace databases
//    at later time states.
//
// ****************************************************************************

void
ViewerProxy::ReplaceDatabase(const std::string &database, int timeState)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ReplaceDatabaseRPC);
    viewerRPC->SetDatabase(database);
    viewerRPC->SetIntArg1(timeState);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::OverlayDatabase
//
//  Purpose:
//    Overlayes a database.
//
//  Arguments:
//    database  The name of the database to overlay.
//
//  Programmer: Brad Whitlock
//  Creation:   Wed Mar 6 16:08:26 PST 2002
//
// ****************************************************************************

void
ViewerProxy::OverlayDatabase(const std::string &database)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::OverlayDatabaseRPC);
    viewerRPC->SetDatabase(database);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ClearCache
//
// Purpose: 
//   Tells the viewer to clear the cache for the compute engine on the
//   specified host.
//
// Arguments:
//   hostName : The host where the compute engine is running.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 14:16:15 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ClearCache(const std::string &hostName)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ClearCacheRPC);
    viewerRPC->SetProgramHost(hostName);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::OpenComputeEngine
//
//  Purpose:
//    Open a compute engine.
//
//  Arguments:
//    hostName    The name of the host to open the engine on.
//    engineName  The name of the engine to execute.
//    options     The options to start the engine with.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr 30 12:21:56 PDT 2001
//    I modified the interface.
//
// ****************************************************************************
void
ViewerProxy::OpenComputeEngine(const std::string &hostName, const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::OpenComputeEngineRPC);
    viewerRPC->SetProgramHost(hostName);
    viewerRPC->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::CloseComputeEngine
//
// Purpose: 
//   Tells the viewer to close a compute engine on the specified host.
//
// Arguments:
//   hostName : The host's engine that will be terminated.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:23:41 PDT 2001
//
// Modifications:
//   
// ****************************************************************************
void
ViewerProxy::CloseComputeEngine(const std::string &hostName)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::CloseComputeEngineRPC);
    viewerRPC->SetProgramHost(hostName);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::InterruptComputeEngine
//
// Purpose: 
//   Interrupts the engine on the specified host.
//
// Arguments:
//   hostName : The host of the engine that we want to interrupt.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 12:24:28 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:10:28 PDT 2001
//    Changed the interruption mechanism.
//   
// ****************************************************************************
void
ViewerProxy::InterruptComputeEngine(const std::string &hostName)
{
    xfer->SendInterruption();
}

// ****************************************************************************
//  Method: ViewerProxy::OpenMDServer
//
//  Purpose:
//    Opens an mdserver on the specified host using the specified arguments.
//
//  Arguments:
//    hostName : The host on which to launch the mdserver.
//    argv     : The arguments to use when launching the mdserver.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Jan 13 08:44:24 PDT 2003
//
// ****************************************************************************

void
ViewerProxy::OpenMDServer(const std::string &hostName, const stringVector &args)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::OpenMDServerRPC);
    viewerRPC->SetProgramHost(hostName);
    viewerRPC->SetProgramOptions(args);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationSetNFrames
//
//  Purpose:
//    Set the number of frames for the active animation.
//
//  Arguments:
//    frame     The number of frames.
//
//  Programmer: Eric Brugger
//  Creation:   August 31, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationSetNFrames(int nFrames)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationSetNFramesRPC);
    viewerRPC->SetNFrames(nFrames);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationPlay
//
//  Purpose:
//    Play the active animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationPlay()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationPlayRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationReversePlay
//
//  Purpose:
//    Play the active animation in reverse.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationReversePlay()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationReversePlayRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationStop
//
//  Purpose:
//    Stop the active animation.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Feb 27 11:33:16 PDT 2003
//    I made this function also send a special opcode to stop the animation.
//
// ****************************************************************************

void
ViewerProxy::AnimationStop()
{
    //
    // Send a special opcode to stop the animation.
    //
    xfer->SendSpecialOpcode(animationStopOpcode);

    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationStopRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationNextFrame
//
//  Purpose:
//    Advance the active animation to the next frame.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationNextFrame()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationNextFrameRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationPreviousFrame
//
//  Purpose:
//    Advance the active animation to the previous frame.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationPreviousFrame()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationPreviousFrameRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AnimationSetFrame
//
//  Purpose:
//    Set the frame for the active animation.
//
//  Arguments:
//    frame     The frame number.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::AnimationSetFrame(int frame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AnimationSetFrameRPC);
    viewerRPC->SetFrameNumber(frame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SaveWindow
//
//  Purpose:
//    Save the current window.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
// ****************************************************************************
void
ViewerProxy::SaveWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SaveWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::PrintWindow
//
// Purpose:
//   Print the current window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 13:59:38 PST 2002
//
// Modifications:
//
// ****************************************************************************
void
ViewerProxy::PrintWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::PrintWindowRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::DisableRedraw
//
// Purpose: 
//   Disables updates for the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:41:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DisableRedraw()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DisableRedrawRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::RedrawWindow
//
// Purpose: 
//   Redraws the VisWindow.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:42:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::RedrawWindow()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::RedrawRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::HideToolbars
//
// Purpose: 
//   Hides the toolbars for the active vis window or for all vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:22:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::HideToolbars(bool forAllWindows)
{
    //
    // Set the rpc type and arguments.
    //
    if(forAllWindows)
        viewerRPC->SetRPCType(ViewerRPC::HideToolbarsForAllWindowsRPC);
    else
        viewerRPC->SetRPCType(ViewerRPC::HideToolbarsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ShowToolbars
//
// Purpose: 
//   Shows the toolbars for the active vis window or for all vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 29 11:22:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ShowToolbars(bool forAllWindows)
{
    //
    // Set the rpc type and arguments.
    //
    if(forAllWindows)
        viewerRPC->SetRPCType(ViewerRPC::ShowToolbarsForAllWindowsRPC);
    else
        viewerRPC->SetRPCType(ViewerRPC::ShowToolbarsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AddPlot
//
//  Purpose:
//    Add a plot to the plot list.
//
//  Arguments:
//    type      The type of plot to add.
//    var       The variable to use for the plot.
//
//  Programmer: Eric Brugger
//  Creation:   August 4, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerProxy::AddPlot(int type, const std::string &var)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AddPlotRPC);
    viewerRPC->SetPlotType(type);
    viewerRPC->SetVariable(var);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetPlotFrameRange
//
//  Purpose:
//    Set the frame range for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame0    The start frame of the plot.
//    frame1    The end frame of the plot.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
void
ViewerProxy::SetPlotFrameRange(int plotId, int frame0, int frame1)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetPlotFrameRangeRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(frame0);
    viewerRPC->SetIntArg3(frame1);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DeletePlotKeyframe
//
//  Purpose:
//    Delete the keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   November 18, 2002
//
// ****************************************************************************
void
ViewerProxy::DeletePlotKeyframe(int plotId, int frame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeletePlotKeyframeRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(frame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::MovePlotKeyframe
//
//  Purpose:
//    Move the the position of a keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerProxy::MovePlotKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::MovePlotKeyframeRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(oldFrame);
    viewerRPC->SetIntArg3(newFrame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetPlotDatabaseState
//
//  Purpose:
//    Set the database state associated with the specified frame and plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The frame number.
//    state     The state to associate with the frame.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
// ****************************************************************************
void
ViewerProxy::SetPlotDatabaseState(int plotId, int frame, int state)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetPlotDatabaseStateRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(frame);
    viewerRPC->SetIntArg3(state);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DeletePlotDatabaseKeyframe
//
//  Purpose:
//    Delete the specified database keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   December 30, 2002
//
// ****************************************************************************
void
ViewerProxy::DeletePlotDatabaseKeyframe(int plotId, int frame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeletePlotDatabaseKeyframeRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(frame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::MovePlotDatabaseKeyframe
//
//  Purpose:
//    Move the the position of a database keyframe for the specified plot.
//
//  Arguments:
//    plotId    The id of the plot.
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerProxy::MovePlotDatabaseKeyframe(int plotId, int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::MovePlotDatabaseKeyframeRPC);
    viewerRPC->SetIntArg1(plotId);
    viewerRPC->SetIntArg2(oldFrame);
    viewerRPC->SetIntArg3(newFrame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DeleteActivePlots
//
//  Purpose:
//    Delete the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::DeleteActivePlots()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeleteActivePlotsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::HideActivePlots
//
//  Purpose:
//    Hide the active plots from the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::HideActivePlots()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::HideActivePlotsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DrawPlots
//
//  Purpose:
//    Draw any undrawn plot in the plot list.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::DrawPlots()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DrawPlotsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetActivePlots
//
//  Purpose:
//    Set the active plots.
//
//  Arguments:
//    activePlotIds     : The indices of the new active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Apr 11 11:34:15 PDT 2003
//    I added code to set a boolean flag.
//
// ****************************************************************************

void
ViewerProxy::SetActivePlots(const intVector &activePlotIds)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetActivePlotsRPC);
    viewerRPC->SetActivePlotIds(activePlotIds);
    viewerRPC->SetBoolFlag(false);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetActivePlots
//
//  Purpose:
//    Set the active plots.
//
//  Arguments:
//    activePlotIds     : The indices of the new active plots.
//    activeOperatorIds : The indices of the new active operators.
//    expandedPlots     : Whether the plots are expanded.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Apr 11 14:55:23 PST 2003
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetActivePlots(const intVector &activePlotIds,
    const intVector &activeOperatorIds, const intVector &expandedPlots)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetActivePlotsRPC);
    viewerRPC->SetActivePlotIds(activePlotIds);
    viewerRPC->SetActiveOperatorIds(activeOperatorIds);
    viewerRPC->SetExpandedPlotIds(expandedPlots);
    viewerRPC->SetBoolFlag(true);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ChangeActivePlotsVar
//
//  Purpose:
//    Change the plot variable for the active plots.
//
//  Arguments:
//    var       The new variable name.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::ChangeActivePlotsVar(const std::string &var)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ChangeActivePlotsVarRPC);
    viewerRPC->SetVariable(var);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::AddOperator
//
//  Purpose:
//    Add the specified operator to the active plots.
//
//  Arguments:
//    oper      The operator to add.
//    fromDefault  Flag indicating whether the operator should be initialized
//                 from its DefaultAtts or from its Client Atts.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Jul 26 03:13:36 PDT 2001
//    Removed all references to OperType.
//
//    Kathleen Bonnell,  
//    Added 'fromDefault' arg. Use it to set the bool flag in the rpc.
//
// ****************************************************************************
void
ViewerProxy::AddOperator(int oper, const bool fromDefault)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::AddOperatorRPC);
    viewerRPC->SetOperatorType(oper);
    viewerRPC->SetBoolFlag(fromDefault);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::PromoteOperator
//
// Purpose: 
//   Tells the viewer to promote an operator for the specified plot. This means
//   that the operator is moved to later in the pipeline.
//
// Arguments:
//   operatorId : The index of the operator to promote.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::PromoteOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::PromoteOperatorRPC);
    viewerRPC->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::DemoteOperator
//
// Purpose: 
//   Tells the viewer to demote an operator for the specified plot. This means
//   that the operator is moved closer to the start of the pipeline.
//
// Arguments:
//   operatorId : The index of the operator to demote.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DemoteOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::DemoteOperatorRPC);
    viewerRPC->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::RemoveOperator
//
// Purpose: 
//   Tells the viewer to remove an operator from the specified plot.
//
// Arguments:
//   operatorId : The index of the operator to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:31:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::RemoveOperator(int operatorId)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::RemoveOperatorRPC);
    viewerRPC->SetOperatorType(operatorId);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::RemoveLastOperator
//
//  Purpose:
//    Remove the last plot operator from the active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::RemoveLastOperator()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::RemoveLastOperatorRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::RemoveAllOperators
//
//  Purpose:
//    Remove all the plot operators from the active plots.
//
//  Programmer: Eric Brugger
//  Creation:   August 15, 2000
//
// ****************************************************************************
void
ViewerProxy::RemoveAllOperators()
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::RemoveAllOperatorsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetDefaultPlotOptions
//
//  Purpose:
//    Set the default options for the specified plot type.
//
//  Arguments:
//    type      The plot type to set the default options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerProxy::SetDefaultPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultPlotOptionsRPC);
    viewerRPC->SetPlotType(type);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetPlotOptions
//
//  Purpose:
//    Set the plot options for the active plots of the specified plot type.
//
//  Arguments:
//    type      The plot type to set the options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Eric Brugger, Thu Mar  8 13:00:36 PST 2001
//    I changed the type of type to an integer.
//
// ****************************************************************************
void
ViewerProxy::SetPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetPlotOptionsRPC);
    viewerRPC->SetPlotType(type);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ResetPlotOptions
//
//  Purpose:
//    Reset the plot options for the active plots of the specified plot type
//    to the default plot attributes.
//
//  Arguments:
//    type      The plot type for which to reset the attributes.
//
//  Programmer: Brad Whitlock, 
//  Creation:   Tue Aug 14 17:22:13 PST 2001
//
//  Modifications:
//
// ****************************************************************************
void
ViewerProxy::ResetPlotOptions(int type)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetPlotOptionsRPC);
    viewerRPC->SetPlotType(type);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetDefaultOperatorOptions
//
//  Purpose:
//    Set the default options for the specified operator type.
//
//  Arguments:
//    oper      The operator type to set the default options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************
void
ViewerProxy::SetDefaultOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultOperatorOptionsRPC);
    viewerRPC->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetOperatorOptions
//
//  Purpose:
//    Set the operator options for the operators of the active plots of
//    the specified operator type.
//
//  Arguments:
//    oper      The operator type to set the options for.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
// ****************************************************************************
void
ViewerProxy::SetOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetOperatorOptionsRPC);
    viewerRPC->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ResetOperatorOptions
//
//  Purpose:
//    Reset the operator options for the operators of the active plots of
//    the specified operator type to the default operator attributes.
//
//  Arguments:
//    oper      The operator type for which to set the options.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 14 17:20:53 PST 2001
//
// ****************************************************************************
void
ViewerProxy::ResetOperatorOptions(int oper)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetOperatorOptionsRPC);
    viewerRPC->SetOperatorType(oper);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetAnimationAttributes
//
//  Purpose:
//    Applies the animation attributes.
//
//  Programmer: Eric Brugger
//  Creation:   November 19, 2001 
//
// ****************************************************************************

void
ViewerProxy::SetAnimationAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetAnimationAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetAnnotationAttributes
//
//  Purpose:
//    Applies the annotation attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001 
//
//  Modifications:
//    Brad Whitlock, Thu Aug 30 09:53:42 PDT 2001
//    Renamed the method to SetAnnotationAttributes.
//
// ****************************************************************************

void
ViewerProxy::SetAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetDefaultAnnotationAttributes
//
// Purpose: 
//   Sets the default annotation attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:54:29 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetDefaultAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetAnnotationAttributes
//
// Purpose: 
//   Reset the annotation attributes to the default values.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:55:07 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetAnnotationAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetAnnotationAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::AddAnnotationObject
//
// Purpose: 
//   Tells the viewer to add a new annotation object of the specifed type.
//
// Arguments:
//   annotType : The type of annotation object to add. This argument corresponds
//               to the AnnotationType enum in AnnotationObject.h
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:53:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::AddAnnotationObject(int annotType)
{
    viewerRPC->SetRPCType(ViewerRPC::AddAnnotationObjectRPC);
    viewerRPC->SetIntArg1(annotType);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::HideActiveAnnotationObjects
//
// Purpose: 
//   Hides the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::HideActiveAnnotationObjects()
{
    viewerRPC->SetRPCType(ViewerRPC::HideActiveAnnotationObjectsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Deletes the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DeleteActiveAnnotationObjects()
{
    viewerRPC->SetRPCType(ViewerRPC::DeleteActiveAnnotationObjectsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Raises the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::RaiseActiveAnnotationObjects()
{
    viewerRPC->SetRPCType(ViewerRPC::RaiseActiveAnnotationObjectsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::LowersActiveAnnotationObjects
//
// Purpose: 
//   Lowers the active annotation objects.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::LowerActiveAnnotationObjects()
{
    viewerRPC->SetRPCType(ViewerRPC::LowerActiveAnnotationObjectsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetAnnotationObjectOptions
//
// Purpose: 
//   Tells the viewer to update the annotations using the options in the
//   annotation options list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 10:54:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetAnnotationObjectOptions()
{
    viewerRPC->SetRPCType(ViewerRPC::SetAnnotationObjectOptionsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetDefaultAnnotationObjectList
//
// Purpose: 
//   Tells the viewer to set the default annotation object list using the
//   client annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:12:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetDefaultAnnotationObjectList()
{
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultAnnotationObjectListRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetAnnotationObjectList
//
// Purpose: 
//   Tells the viewer to set the client annotation object list using the
//   default annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:12:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetAnnotationObjectList()
{
    viewerRPC->SetRPCType(ViewerRPC::ResetAnnotationObjectListRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetMaterialAttributes
//
//  Purpose:
//    Applies the material attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   October 24, 2002
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetDefaultMaterialAttributes
//
// Purpose: 
//   Sets the default material attributes.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetDefaultMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetMaterialAttributes
//
// Purpose: 
//   Reset the material attributes to the default values.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetMaterialAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetMaterialAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetKeyframeAttributes
//
//  Purpose:
//    Set the current keyframe attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  8, 2002
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetKeyframeAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetKeyframeAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetLightList
//
//  Purpose:
//    Applies the light list to the window.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 14:00:09 PST 2001 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetLightList()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetLightListRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetDefaultLightList
//
// Purpose: 
//   Sets the default light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 13:50:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetDefaultLightList()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultLightListRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetLightList
//
// Purpose: 
//   Reset the light list to the default values.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 13:50:47 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetLightList()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetLightListRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetPickLetter
//
// Purpose: 
//   Reset the pick attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   December 9, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetPickLetter()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetPickLetterRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}


// ****************************************************************************
// Method: ViewerProxy::ResetPickAttributes
//
// Purpose: 
//   Reset the pick attributes to default values.
//
// Programmer: Kathleen Bonnell 
// Creation:   November 26, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetPickAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ResetPickAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}


// ****************************************************************************
// Method: ViewerProxy::SetActiveContinuousColorTable
//
// Purpose: 
//   Sets the active continuous color table. This is the color table that
//   is used for all new plots that need a continuous color table.
//
// Arguments:
//   colorTableName : The name of the continuous colortable to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 16:55:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetActiveContinuousColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    if(colorTableAtts->GetColorTableIndex(colorTableName) != -1)
    {
        colorTableAtts->SetActiveContinuous(colorTableName);
        colorTableAtts->Notify();

        // Update the color table. This has the effect of making all plots
        // use the default color table update to use the new active color
        // table.
        UpdateColorTable(colorTableName);
    }
}

// ****************************************************************************
// Method: ViewerProxy::SetActiveDiscreteColorTable
//
// Purpose: 
//   Sets the active discrete color table. This is the color table that
//   is used for all new plots that need a discrete color table.
//
// Arguments:
//   colorTableName : The name of the discrete colortable to use.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 16:55:21 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetActiveDiscreteColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    if(colorTableAtts->GetColorTableIndex(colorTableName) != -1)
    {
        colorTableAtts->SetActiveDiscrete(colorTableName);
        colorTableAtts->Notify();
    }
}

// ****************************************************************************
// Method: ViewerProxy::DeleteColorTable
//
// Purpose: 
//   Deletes the specified color table from the list of color tables. This
//   can cause all plots that use the deleted color table to be updated after
//   the color table is deleted.
//
// Arguments:
//   colorTableName : The name of the color table to delete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 17:03:07 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DeleteColorTable(const std::string &colorTableName)
{
    // If it's a valid color table name, make it active.
    int index = colorTableAtts->GetColorTableIndex(colorTableName);
    if(index != -1)
    {
        // Remove the color table from the list and update.
        colorTableAtts->RemoveColorTable(index);
        colorTableAtts->Notify();

        // Update the color table. The specified color table will no
        // longer exist in the list of color tables so all plots that used
        // that color table will have their color tables changed to something
        // else.
        UpdateColorTable(colorTableName);
    }
}

// ****************************************************************************
// Method: ViewerProxy::UpdateColorTable
//
// Purpose: 
//   Forces the viewer to update all plots that use the specified color table.
//   If the color table does not exist, then all plots that used it are reset
//   so they use the current color table.
//
// Arguments:
//   colorTableName : The name of the color table that's being updated.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 17:20:23 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::UpdateColorTable(const std::string &colorTableName)
{
    //
    // Set the RPC type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::UpdateColorTableRPC);
    viewerRPC->SetColorTableName(colorTableName);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ExportColorTable
//
// Purpose: 
//   Forces the viewer to export the named color table to a small XML file.
//
// Arguments:
//   colorTableName : The name of the color table that's being exported.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 16:48:09 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ExportColorTable(const std::string &colorTableName)
{
    //
    // Set the RPC type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::ExportColorTableRPC);
    viewerRPC->SetColorTableName(colorTableName);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::InvertBackgroundColor
//
// Purpose: 
//   Tells the viewer to swap its background color with its foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:51:22 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::InvertBackgroundColor()
{
    viewerRPC->SetRPCType(ViewerRPC::InvertBackgroundRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::WriteConfigFile
//
//  Purpose:
//    Writes a config file containing the default values for the viewer's
//    state objects.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 11:52:35 PDT 2000
//
// ****************************************************************************

void
ViewerProxy::WriteConfigFile()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::WriteConfigFileRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ExportEntireState
//
// Purpose: 
//   Tells the viewer to dump its entire state to an XML file.
//
// Arguments:
//   filename : The name of the file used to write the state.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 11:58:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ExportEntireState(const std::string &filename)
{
    viewerRPC->SetRPCType(ViewerRPC::ExportEntireStateRPC);
    viewerRPC->SetVariable(filename);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ImportEntireState
//
// Purpose: 
//   Tells the viewer to set its entire state using the values stored in
//   the named file.
//
// Arguments:
//   filename   : The name of the file to read for the state.
//   inVisItDir : Whether the session file is in the .visit directory.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 11:58:23 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ImportEntireState(const std::string &filename, bool inVisItDir)
{
    viewerRPC->SetRPCType(ViewerRPC::ImportEntireStateRPC);
    viewerRPC->SetVariable(filename);
    viewerRPC->SetBoolFlag(inVisItDir);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetViewCurve
//
// Purpose: 
//   Tells the viewer to use the new curve view attributes.
//
// Programmer: Eric Brugger
// Creation:   August 20, 2003
//
// ****************************************************************************

void
ViewerProxy::SetViewCurve()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetViewCurveRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetView2D
//
// Purpose: 
//   Tells the viewer to use the new 2d view attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 16:48:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetView2D()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetView2DRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetView3D
//
// Purpose: 
//   Tells the viewer to use the new 3d view attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 26 16:48:38 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetView3D()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetView3DRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ClearViewKeyframes
//
//  Purpose: 
//    Tells the viewer to clear the view keyframes.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ClearViewKeyframes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ClearViewKeyframesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::DeleteViewKeyframe
//
//  Purpose: 
//    Tells the viewer to delete the specified view keyframe.
//
//  Arguments:
//    frame     The keyframe to delete.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerProxy::DeleteViewKeyframe(int frame)
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::DeleteViewKeyframeRPC);
    viewerRPC->SetFrame(frame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::MoveViewKeyframe
//
//  Purpose:
//    Move the position of a view keyframe.
//
//  Arguments:
//    oldFrame  The old location of the keyframe.
//    newFrame  The new location of the keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 28, 2003
//
// ****************************************************************************
void
ViewerProxy::MoveViewKeyframe(int oldFrame, int newFrame)
{
    //
    // Set the rpc type and arguments.
    //
    viewerRPC->SetRPCType(ViewerRPC::MoveViewKeyframeRPC);
    viewerRPC->SetIntArg1(oldFrame);
    viewerRPC->SetIntArg2(newFrame);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::SetViewKeyframe
//
//  Purpose: 
//    Tells the viewer to set a view keyframe.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetViewKeyframe()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetViewKeyframeRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ResetView
//
// Purpose: 
//   Tells the viewer to reset the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ResetView()
{
    viewerRPC->SetRPCType(ViewerRPC::ResetViewRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::RecenterView
//
// Purpose: 
//   Tells the viewer to recenter the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::RecenterView()
{
    viewerRPC->SetRPCType(ViewerRPC::RecenterViewRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetViewExtents
//
// Purpose: 
//   Tells the viewer to set the view extents.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 12:42:03 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetViewExtentsType(int t)
{
    viewerRPC->SetRPCType(ViewerRPC::SetViewExtentsTypeRPC);
    viewerRPC->SetWindowLayout(t);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleMaintainViewMode
//
// Purpose: 
//   Tells the viewer to toggle the maintain view mode.
//
// Programmer: Eric Brugger
// Creation:   April 18, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleMaintainViewMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleMaintainViewModeRPC);
    viewerRPC->Notify();
}


// ****************************************************************************
// Method: ViewerProxy::ToggleFullFrameMode
//
// Purpose: 
//   Tells the viewer to toggle the full frame mode.
//
// Programmer: Kathleen Bonnell 
// Creation:   May 13, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleFullFrameMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleFullFrameRPC);
    viewerRPC->Notify();
}


// ****************************************************************************
// Method: ViewerProxy::UndoView
//
// Purpose: 
//   Tells the viewer to undo the last view change.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::UndoView()
{
    viewerRPC->SetRPCType(ViewerRPC::UndoViewRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleLockViewMode
//
// Purpose: 
//   Tells the viewer to lock the view.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleLockViewMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleLockViewModeRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleLockTime
//
// Purpose: 
//   Toggles the viewer's lock time flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:47:51 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleLockTime()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleLockTimeRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleLockTools
//
// Purpose: 
//   Toggles the viewer's lock tools flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:48:22 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleLockTools()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleLockToolsRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleSpinMode
//
// Purpose: 
//   Tells the viewer to toggle the spin mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:40:01 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleSpinMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleSpinModeRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
//  Method: ViewerProxy::ToggleCameraViewMode
//
//  Purpose: 
//    Tells the viewer to toggle the camera view mode.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleCameraViewMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleCameraViewModeRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetWindowMode
//
// Purpose: 
//   Sets the window mode.
//
// Arguments:
//   mode : The new window mode [0,2]
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:01:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetWindowMode(int mode)
{
    viewerRPC->SetRPCType(ViewerRPC::SetWindowModeRPC);
    viewerRPC->SetWindowMode(mode);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ToggleBoundingBoxMode
//
// Purpose: 
//   Tells the viewer whether or not bbox mode should be used.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:02:12 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ToggleBoundingBoxMode()
{
    viewerRPC->SetRPCType(ViewerRPC::ToggleBoundingBoxModeRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::EnableTool
//
// Purpose: 
//   Tells the viewer to enable/disable a tool.
//
// Arguments:
//   tool    : The index of the tool.
//   enabled : Whether or not the tool is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 17:02:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::EnableTool(int tool, bool enabled)
{
    viewerRPC->SetRPCType(ViewerRPC::EnableToolRPC);
    viewerRPC->SetToolId(tool);
    viewerRPC->SetBoolFlag(enabled);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::CopyViewToWindow
//
// Purpose: 
//   Copies the view from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::CopyViewToWindow(int from, int to)
{
    viewerRPC->SetRPCType(ViewerRPC::CopyViewToWindowRPC);
    // store from window in windowLayout
    viewerRPC->SetWindowLayout(from);
    viewerRPC->SetWindowId(to);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::CopyLightingToWindow
//
// Purpose: 
//   Copies the view from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::CopyLightingToWindow(int from, int to)
{
    viewerRPC->SetRPCType(ViewerRPC::CopyLightingToWindowRPC);
    // store from window in windowLayout
    viewerRPC->SetWindowLayout(from);
    viewerRPC->SetWindowId(to);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::CopyAnnotationsToWindow
//
// Purpose: 
//   Copies the annotations from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 16:17:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::CopyAnnotationsToWindow(int from, int to)
{
    viewerRPC->SetRPCType(ViewerRPC::CopyAnnotationsToWindowRPC);
    // store from window in windowLayout
    viewerRPC->SetWindowLayout(from);
    viewerRPC->SetWindowId(to);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::CopyPlotsToWindow
//
// Purpose: 
//   Copies the plots from one window to another window.
//
// Arguments:
//   from : The source window.
//   to   : The destination window.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:26:43 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::CopyPlotsToWindow(int from, int to)
{
    viewerRPC->SetRPCType(ViewerRPC::CopyPlotsToWindowRPC);
    // store from window in windowLayout
    viewerRPC->SetWindowLayout(from);
    viewerRPC->SetWindowId(to);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetPlotSILRestriction
//
// Purpose: 
//   Tells the viewer to apply the SIL restriction to the selected plots in
//   the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:33:41 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Account for the SIL restriction now being a pointer.
//
// ****************************************************************************

void
ViewerProxy::SetPlotSILRestriction()
{
    SILRestrictionAttributes *newSRA =internalSILRestriction->MakeAttributes();
    if(newSRA)
    {
        // Copy the attribute representation of the SIL restriction into the
        // silRestrictionAtts object and send notify observers. Note that
        // SetUpdate is set to false so the proxy's Update method is NOT called
        *silRestrictionAtts = *newSRA;
        SetUpdate(false);
        silRestrictionAtts->Notify();

        // Now that the new SIL restriction attributes have been sent to the
        // viewer, send the RPC that tells the viewer to apply them.
        viewerRPC->SetRPCType(ViewerRPC::SetPlotSILRestrictionRPC);
        viewerRPC->Notify();

        // Delete the new SRA since we're done with it.
        delete newSRA;
    }
}

// ****************************************************************************
// Method: ViewerProxy::SetPlotSILRestriction
//
// Purpose: 
//   Tells the viewer to apply the specified new SIL restriction to the
//   selected plots in the plot list.
//
// Arguments:
//   newRestriction the new SIL restriction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 21 13:33:41 PST 2001
//
// Modifications:
//   
//   Hank Childs, Mon Dec  2 14:13:55 PST 2002
//   Account for the SIL restriction now being a pointer.
//
// ****************************************************************************

void
ViewerProxy::SetPlotSILRestriction(avtSILRestriction_p newRestriction)
{
    // Copy the new SIL restriction into the internal SIL restriction object.
    internalSILRestriction = newRestriction;

    SILRestrictionAttributes *newSRA =internalSILRestriction->MakeAttributes();
    if(newSRA)
    {
        // Copy the attribute representation of the SIL restriction into the
        // silRestrictionAtts object and send notify observers. Note that
        // SetUpdate is set to false so the proxy's Update method is NOT called
        *silRestrictionAtts = *newSRA;
        SetUpdate(false);
        silRestrictionAtts->Notify();

        // Now that the new SIL restriction attributes have been sent to the
        // viewer, send the RPC that tells the viewer to apply them.
        viewerRPC->SetRPCType(ViewerRPC::SetPlotSILRestrictionRPC);
        viewerRPC->Notify();

        // Delete the new SRA since we're done with it.
        delete newSRA;
    }
}

// ****************************************************************************
// Method: ViewerProxy::ProcessExpressions
//
// Purpose: 
//   Issues a ProcessExpressions RPC to the viewer. This RPC tells the viewer
//   to update its plots with new expressions.
//
// Programmer: Sean Ahern
// Creation:   Wed Sep 26 16:32:00 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ProcessExpressions()
{
    // Set the rpc type.
    viewerRPC->SetRPCType(ViewerRPC::ProcessExpressionsRPC);

    // Issue the RPC.
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetAppearanceAttributes
//
// Purpose: 
//   Issues a SetAppearance RPC to the viewer. This RPC tells the viewer to
//   update its gui colors/font/style...
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 22:38:51 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetAppearanceAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetAppearanceRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::GetPlotAttributes
//
// Purpose: 
//   Returns a pointer to the attribute subject for the specified plot type.
//
// Arguments:
//   type      The type of the plot to return the attributes for.
//
// Returns:    A pointer to the plot's attribute subject.
//
// Programmer: Eric Brugger
// Creation:   March 8, 2001
//
// ****************************************************************************

AttributeSubject *
ViewerProxy::GetPlotAttributes(int type) const
{
    //
    // Check the arguments.
    //
    if (type < 0 || type >= nPlots)
    {
        return 0;
    }

    return plotAtts[type];
}

// ****************************************************************************
// Method: ViewerProxy::GetOperatorAttributes
//
// Purpose: 
//   Returns a pointer to the attribute subject for the specified operator type
//
// Arguments:
//   type      The type of the operator to return the attributes for.
//
// Returns:    A pointer to the operator's attribute subject.
//
// Programmer: Jeremy Meredith
// Creation:   July 25, 2001
//
// Modifications:
//
//   Hank Childs, Wed Nov  7 14:55:42 PST 2001
//   Cut and paste error.
//
// ****************************************************************************

AttributeSubject *
ViewerProxy::GetOperatorAttributes(int type) const
{
    //
    // Check the arguments.
    //
    if (type < 0 || type >= nOperators)
    {
        return 0;
    }

    return operatorAtts[type];
}


// ****************************************************************************
// Method: ViewerProxy::ClearPickPoints
//
// Purpose: 
//   Tells the viewer to clear its pick points.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:52:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ClearPickPoints()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::ClearPickPointsRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::ClearReferenceLines
//
// Purpose: 
//   Tells the viewer to clear its reference lines.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 6 16:52:47 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::ClearReferenceLines()
{
    viewerRPC->SetRPCType(ViewerRPC::ClearRefLinesRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetRenderingAttributes
//
// Purpose: 
//   Tells the viewer to use the new rendering attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 13:19:33 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::SetRenderingAttributes()
{
    viewerRPC->SetRPCType(ViewerRPC::SetRenderingAttributesRPC);
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::SetWindowArea
//
// Purpose: 
//   Sets the window area. This is the area used for the vis windows.
//
// Arguments:
//   x : The x location of the window area.
//   y : The y location of the window area.
//   w : The width of the window area.
//   h : The height of the window area.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:17:50 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 20 14:31:04 PST 2002
//   Made it use the automatically generated viewerRPC object.
//
//   Brad Whitlock, Mon Feb 10 11:53:10 PDT 2003
//   I turned snprintf into SNPRINTF.
//
// ****************************************************************************

void
ViewerProxy::SetWindowArea(int x, int y, int w, int h)
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetWindowAreaRPC);
    char str[50];
    SNPRINTF(str, 50, "%dx%d+%d+%d", w, h, x, y);
    viewerRPC->SetWindowArea(str);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::DatabaseQuery
//
// Purpose: 
//   Tells the viewer to perform the named database query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   vars      : The variables that we're querying.
//   arg1      : Optional integer argument. 
//   arg2      : Optional integer argument.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:35:17 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added optioninal integer args. 
//
// ****************************************************************************

void
ViewerProxy::DatabaseQuery(const std::string &queryName,
    const stringVector &vars, const int arg1, const int arg2)
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::DatabaseQueryRPC);
    viewerRPC->SetQueryName(queryName);
    viewerRPC->SetQueryVariables(vars);
    viewerRPC->SetIntArg1(arg1);
    viewerRPC->SetIntArg2(arg2);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::PointQuery
//
// Purpose: 
//   Tells the viewer to perform the named point query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   pt        : The location of the point to query. It can be in screen 
//               or world coordinates.
//   vars      : The variables that we're querying.
//   arg1      : An optional int argument.
//   arg2      : An optional int argument.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:36:12 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Nov 26 14:17:55 PST 2003
//   Added optional int args.
//   
// ****************************************************************************

void
ViewerProxy::PointQuery(const std::string &queryName, const double pt[3],
    const stringVector &vars, const int arg1, const int arg2)
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::PointQueryRPC);
    viewerRPC->SetQueryName(queryName);
    viewerRPC->SetQueryPoint1(pt);
    viewerRPC->SetQueryVariables(vars);
    viewerRPC->SetIntArg1(arg1);
    viewerRPC->SetIntArg2(arg2);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::LineQuery
//
// Purpose: 
//   Tells the viewer to perform the named line query.
//
// Arguments:
//   queryName : The name of the query to perform.
//   pt1       : The start location of the line to query. It can be in screen 
//               or world coordinates.
//   pt2       : The end location of the line to query. It can be in screen 
//               or world coordinates.
//   vars      : The variables that we're querying.
//   samples   : The number of samples to be used in the query. 
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:37:43 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added samples arg.
//   
// ****************************************************************************

void
ViewerProxy::LineQuery(const std::string &queryName, const double pt1[3],
    const double pt2[3], const stringVector &vars, const int samples)
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::LineQueryRPC);
    viewerRPC->SetQueryName(queryName);
    viewerRPC->SetQueryPoint1(pt1);
    viewerRPC->SetQueryPoint2(pt2);
    viewerRPC->SetQueryVariables(vars);
    viewerRPC->SetIntArg1(samples);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

// ****************************************************************************
// Method: ViewerProxy::Pick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified screen location.
//
// Arguments:
//   x,y  : The pick point location in screen coordinates.
//   vars : The variables that we're querying.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:38:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::Pick(int x, int y, const stringVector &vars)
{
    double pt[3];
    pt[0] = (double)x;
    pt[1] = (double)y;
    pt[2] = 0.;
    PointQuery("Pick", pt, vars);
}


// ****************************************************************************
// Method: ViewerProxy::Pick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified world coordinate.
//
// Arguments:
//   xyz  : The pick point location in world coordinates.
//   vars   : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 23, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::Pick(double xyz[3], const stringVector &vars)
{
    PointQuery("WorldPick", xyz, vars);
}

// ****************************************************************************
// Method: ViewerProxy::NodePick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified screen location.
//
// Arguments:
//   x,y  : The pick point location in screen coordinates.
//   vars : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   June 25, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::NodePick(int x, int y, const stringVector &vars)
{
    double pt[3];
    pt[0] = (double)x;
    pt[1] = (double)y;
    pt[2] = 0.;
    PointQuery("NodePick", pt, vars);
}


// ****************************************************************************
// Method: ViewerProxy::NodePick
//
// Purpose: 
//   Tells the viewer to add a pick point at the specified world coordinate.
//
// Arguments:
//   xyz  : The pick point location in world coordinates.
//   vars   : The variables that we're querying.
//
// Programmer: Kathleen Bonnell 
// Creation:   July 23, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerProxy::NodePick(double xyz[3], const stringVector &vars)
{
    PointQuery("WorldNodePick", xyz, vars);
}

// ****************************************************************************
// Method: ViewerProxy::Lineout
//
// Purpose: 
//   Tells the viewer to add a reference line at the specified screen location.
//
// Arguments:
//   x0,y0   : The start location of the line in screen coordinates.
//   x1,y1   : The start location of the line in screen coordinates.
//   vars    : The variables that we're querying.
//   samples : The number of samples along the line. 
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 9 16:38:51 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 27 11:43:46 PDT 2002
//   I changed the routine so it takes lineout endpoints in world space.
//
//   Kathleen Bonnell, Wed Jul 23 17:04:10 PDT 2003
//   Added samples arg.
//   
// ****************************************************************************

void
ViewerProxy::Lineout(const double p0[3], const double p1[3],
    const stringVector &vars, const int samples)
{
    LineQuery("Lineout", p0, p1, vars, samples);
}



// ****************************************************************************
//  Method: ViewerProxy::SetGlobalLineotuAttributes
//
//  Purpose:
//    Applies the global lineout attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 4, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetGlobalLineoutAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetGlobalLineoutAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}


// ****************************************************************************
//  Method: ViewerProxy::SetDefaultPickAttributes
//
//  Purpose:
//    Applies the default pick attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 9, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetDefaultPickAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetDefaultPickAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}


// ****************************************************************************
//  Method: ViewerProxy::SetPickAttributes
//
//  Purpose:
//    Applies the pick attributes.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 30, 2003 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerProxy::SetPickAttributes()
{
    //
    // Set the rpc type.
    //
    viewerRPC->SetRPCType(ViewerRPC::SetPickAttributesRPC);

    //
    // Issue the RPC.
    //
    viewerRPC->Notify();
}

