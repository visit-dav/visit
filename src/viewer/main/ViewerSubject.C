// ************************************************************************* //
//                               ViewerSubject.C                             //
// ************************************************************************* //

#include <ctype.h>
#include <stdlib.h>
#include <snprintf.h>
#include <ViewerSubject.h>

#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#if QT_VERSION >= 230
#include <qsgistyle.h>
#endif
#include <qtimer.h>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AppearanceAttributes.h>
#include <ColorTableAttributes.h>
#include <EngineList.h>
#include <ExpressionList.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfile.h>
#include <HostProfileList.h>
#include <InitVTK.h>
#include <InvalidVariableException.h>
#include <KeyframeAttributes.h>
#include <LostConnectionException.h>
#include <MaterialAttributes.h>
#include <MessageAttributes.h>
#include <PickAttributes.h>
#include <PlotList.h>
#include <PluginManagerAttributes.h>
#include <PrinterAttributes.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <RemoteProcess.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <SyncAttributes.h>

#include <ViewAttributes.h>
#include <ViewerActionManager.h>
#include <ViewerAnimation.h>
#include <ViewerConnectionProgressDialog.h>
#include <ViewerExpressionList.h>
#include <ViewerConfigManager.h>
#include <ViewerEngineManager.h>
#include <ViewerEngineChooser.h>
#include <ViewerFileServer.h>
#include <ViewerMessageBuffer.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPasswordWindow.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerQueryManager.h>
#include <ViewerRPCObserver.h>
#include <ViewerServerManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>
#include <VisWinRendering.h>
#include <WindowInformation.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>

#include <qapplication.h>
#include <qsocketnotifier.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <WindowMetrics.h>

#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDatabaseMetaData.h>

#if !defined(_WIN32)
#include <strings.h>
#include <unistd.h>
#else
#if defined(GetMessage)
#undef GetMessage
#endif
#endif

// Global variables.  This is a hack, they should be removed.
ViewerSubject  *viewerSubject=0;

using std::string;

// ****************************************************************************
//  Method: ViewerSubject constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 08:52:24 PDT 2000
//    Added onionpeel operator to operatorFactory.
//
//    Brad Whitlock, Fri Oct 27 14:05:57 PST 2000
//    I added code to make the viewer work with Qt.
//
//    Brad Whitlock, Fri Nov 10 15:20:13 PST 2000
//    I added the material plot.
//
//    Jeremy Meredith, Tue Dec 12 13:59:45 PST 2000
//    Added the material selection operator.
//
//    Eric Brugger, Thu Dec 21 11:41:11 PST 2000
//    I modified the routine to work with the new plot factory.
//
//    Brad Whitlock, Mon Apr 30 16:05:14 PST 2001
//    I added messaging state objects.
//
//    Brad Whitlock, Thu Jun 14 15:59:48 PST 2001
//    I added a color table object.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001
//    I added annotationAttributes. 
//
//    Brad Whitlock, Fri May 25 16:38:31 PST 2001
//    Added initialization of an error handling variable.
//
//    Jeremy Meredith, Fri Jul 20 11:24:40 PDT 2001
//    Added initialization of shift.
//
//    Jeremy Meredith, Thu Jul 26 03:15:04 PDT 2001
//    Added support for real operator plugins.
//
//    Brad Whitlock, Thu Aug 30 09:01:56 PDT 2001
//    I removed the annotation attributes.
//
//    Jeremy Meredith, Wed Sep  5 14:05:56 PDT 2001
//    Added plugin manager attributes.
//
//    Jeremy Meredith, Fri Sep 14 13:30:55 PDT 2001
//    Added initialization of preshift.
//
//    Jeremy Meredith, Fri Sep 28 13:45:32 PDT 2001
//    Added reading of plugin manager attributes, disabling
//    of plugins, delayed loading of plugins.
//
//    Brad Whitlock, Fri Oct 19 12:29:24 PDT 2001
//    Added a timer object.
//
//    Eric Brugger, Mon Oct 29 09:47:30 PST 2001
//    Removed the timer object and work process.
//
//    Eric Brugger, Mon Nov 19 15:42:07 PST 2001
//    I added animation attributes.
//
//    Brad Whitlock, Mon Sep 17 11:04:41 PDT 2001
//    Added sync attributes.
//
//    Brad Whitlock, Fri Jan 4 17:29:54 PST 2002
//    Added a flag for interruption checking.
//
//    Jeremy Meredith, Tue Apr 16 11:25:30 PDT 2002
//    Added ability to process a few command line args before anything happens.
//    Added flag to disable reading of config files.
//
//    Brad Whitlock, Mon Mar 25 12:46:12 PDT 2002
//    Removed a flag that checks for zero length reads. Made pipe related
//    coding compile only when the viewer is multithreaded.
//
//    Jeremy Meredith, Tue Apr 23 15:51:47 PDT 2002
//    Added initialization of nowin flag.
//
//    Jeremy Meredith, Wed May  8 15:39:43 PDT 2002
//    Added keyframe attributes.
//
//    Brad Whitlock, Mon Aug 19 16:01:15 PST 2002
//    I removed the animationAtts member.
//
//    Jeremy Meredith, Wed Aug 21 12:51:28 PDT 2002
//    I renamed some plot/operator plugin manager methods for refactoring.
//
//    Eric Brugger, Mon Nov 25 09:37:08 PST 2002
//    I moved the keyframe attribute subject to the window manager.
//
//    Brad Whitlock, Mon Dec 2 13:37:00 PST 2002
//    I removed the color tables.
//
//    Jeremy Meredith, Thu Dec 19 12:05:29 PST 2002
//    Added support for launching engines from the command line.
//    Added ability to temporarily block socket signals.
//
//    Brad Whitlock, Thu Jan 9 11:51:01 PDT 2003
//    I made some members be strings to avoid memory problems.
//
//    Kathleen Bonnell, Wed Feb  5 08:30:42 PST 2003 
//    Pass '!nowin' as an argument in the creation of mainApp, so that gui
//    isn't created in nowin mode. 
//
//    Jeremy Meredith, Fri Feb 28 12:21:01 PST 2003
//    Renamed LoadPlugins to LoadPluginsNow (since there is a corresponding
//    LoadPluginsOnDemand).
//
//    Brad Whitlock, Fri May 16 14:53:17 PST 2003
//    I added support for named config files.
//
// ****************************************************************************

ViewerSubject::ViewerSubject(int *argc, char ***argv) : borders(), shift(),
    preshift(), geometry()
{
    //
    // Enabled interruption checking by default.
    //
    interruptionEnabled = true;

    //
    // Will be set by the -launchengine flag.
    //
    launchEngineAtStartup = "";

    //
    // can be overridden by the -numrestarts flag.
    //
    numEngineRestarts = VIEWER_NUM_ENGINE_RESTARTS;

    //
    // Set by BlockSocketSignals method.
    //
    blockSocketSignals = false;

    //
    // Set some flags related to viewer windows.
    //
    nowin = false;
    smallWindow = false;

    //
    // By default, read the config files.
    //
    noconfig = false;
    configFileName = 0;

    //
    // Process a few command line arguments that need to happen first
    //
    ProcessCommandLine1(argc, argv);

    // Create the messaging attributes.
    messageAtts = new MessageAttributes;
    statusAtts = new StatusAttributes;
    // Create the appearance attributes.
    appearanceAtts = new AppearanceAttributes;
    // Create the sync attributes.
    syncAtts = new SyncAttributes;
    // Create the plugin attributes
    pluginAtts = new PluginManagerAttributes;

    // Read the config file and setup the appearance attributes. Note that
    // we call the routine to process the config file settings here because
    // it only has to update the appearance attributes.
    configMgr = new ViewerConfigManager;
    char *configFile = configMgr->GetSystemConfigFile();
    if (noconfig)
        systemSettings = NULL;
    else
        systemSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    configFile = configMgr->GetDefaultConfigFile(configFileName);
    if (noconfig)
        localSettings = NULL;
    else
        localSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    configMgr->Add(appearanceAtts);
    configMgr->Add(pluginAtts);
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);
    configMgr->Notify();
    configMgr->ClearSubjects();

    PlotPluginManager     *pmgr = PlotPluginManager::Instance();
    OperatorPluginManager *omgr = OperatorPluginManager::Instance();

    //
    // Go through the saved plugin atts and remove the ones
    // not available to the viewer
    //
    bool done = false;
    int i;
    while (!done)
    {
        done = true;
        for (i=0; i<pluginAtts->GetId().size(); i++)
        {
            std::string id = pluginAtts->GetId()[i];
            if ((pluginAtts->GetType()[i] == "plot"     &&
                 !pmgr->PluginExists(id)                  ) ||
                (pluginAtts->GetType()[i] == "operator" &&
                 !omgr->PluginExists(id)                  ))
            {
                done = false;
                pluginAtts->RemovePlugin(id);
                break;
            }
        }
    }

    //
    // Go through the saved plugin atts and disable the ones specified
    // to be disabled in the plugin managers
    //
    for (i=0; i<pluginAtts->GetId().size(); i++)
    {
        std::string id = pluginAtts->GetId()[i];
        if (pluginAtts->GetEnabled()[i] == false)
        {
            if (pluginAtts->GetType()[i] == "plot")
            {
                if (pmgr->PluginExists(id))
                    pmgr->DisablePlugin(id);
            }
            else if (pluginAtts->GetType()[i] == "operator")
            {
                if (omgr->PluginExists(id))
                    omgr->DisablePlugin(id);
            }
        }
    }

    //
    // Now add those to the atts that are in the manager but not yet listed
    //
    for (i=0; i<pmgr->GetNAllPlugins(); i++)
    {
        std::string id = pmgr->GetAllID(i);
        if (pluginAtts->GetIndexByID(id) < 0)
            pluginAtts->AddPlugin(pmgr->GetPluginName(id),     "plot",
                                  pmgr->GetPluginVersion(id),  id);
    }
    for (i=0; i<omgr->GetNAllPlugins(); i++)
    {
        std::string id = omgr->GetAllID(i);
        if (pluginAtts->GetIndexByID(id) < 0)
            pluginAtts->AddPlugin(omgr->GetPluginName(id),     "operator",
                                  omgr->GetPluginVersion(id),  id);
    }

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        PlotPluginManager::Instance()->LoadPluginsNow();
        OperatorPluginManager::Instance()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the plugins. "
             << "The error message is: \"" << e.GetMessage().c_str() << "\"" << endl;
    }
    ENDTRY

    //
    // Process the command line arguments first since some may be removed
    // by QApplication::QApplication.
    //
    ProcessCommandLine2(argc, argv);

    //
    // Create the QApplication context. This sets the qApp pointer.
    //
    char **argv2 = new char *[*argc + 3];
    int argc2 = *argc + 2;
    for(i = 0; i < *argc; ++i)
        argv2[i] = (*argv)[i];
    argv2[*argc] = "-font";
    argv2[*argc+1] = (char*)appearanceAtts->GetFontDescription().c_str();
    argv2[*argc+2] = NULL;
    mainApp = new QApplication(argc2, argv2, !nowin);
    CustomizeAppearance();
    delete [] argv2;

#ifdef VIEWER_MT
    //
    // Try to create a pipe to communicate with the rendering thread.
    //
    if (pipe(messagePipe) < 0)
    {
        cerr << "Can not create the pipe for communicating with the master\n";
        cerr << "thread.\n";
        messagePipe[0] = -1;
        messagePipe[1] = -1;
    }
    else
    {
        // Create a QSocketNotifier that will tell us when to call
        // ProcessRendererMessage.
        checkRenderer = new QSocketNotifier(messagePipe[0],
            QSocketNotifier::Read);
        connect(checkRenderer, SIGNAL(activated(int)),
                this, SLOT(ProcessRendererMessage()));
    }
#endif
    messageBuffer = new ViewerMessageBuffer;

    //
    // Create the Plot factory.
    //
    plotFactory = new ViewerPlotFactory();

    //
    // Create the Operator factory.
    //
    operatorFactory = new ViewerOperatorFactory();

    viewerSubject = this;   // FIX_ME Hack, this should be removed.
}

// ****************************************************************************
//  Method: ViewerSubject destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Oct 5 19:41:09 PST 2000
//    I removed the SocketConnections.
//
//    Brad Whitlock, Mon Oct 30 11:16:36 PDT 2000
//    I modified the code to delete borders, geometry and added code to
//    close the message pipes to the rendering thread.
//
//    Brad Whitlock, Tue Nov 21 12:42:09 PDT 2000
//    Added code to close down the mdservers.
//
//    Brad Whitlock, Mon Apr 30 16:05:44 PST 2001
//    Added messaging subjects.
//
//    Brad Whitlock, Thu Jun 14 16:00:58 PST 2001
//    Added the color table object.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    I added annotationAtts. 
//
//    Jeremy Meredith, Fri Jul 20 11:24:56 PDT 2001
//    Added 'shift'.
//
//    Brad Whitlock, Thu Aug 30 10:29:40 PDT 2001
//    Removed the annotation attributes.
//
//    Jeremy Meredith, Wed Sep  5 14:05:52 PDT 2001
//    Added plugin manager attributes.
//
//    Brad Whitlock, Wed Sep 5 00:30:58 PDT 2001
//    Added the appearance attributes.
//
//    Jeremy Meredith, Fri Sep 14 13:31:05 PDT 2001
//    Added 'preshift'.
//
//    Eric Brugger, Mon Nov 19 15:42:07 PST 2001
//    I added animation attributes.
//
//    Brad Whitlock, Mon Sep 17 11:05:47 PDT 2001
//    Added sync attributes.
//
//    Brad Whitlock, Thu Feb 7 16:04:21 PST 2002
//    Added code to delete the default SIL restrictions.
//
//    Brad Whitlock, Thu Mar 7 11:42:17 PDT 2002
//    Removed the code to delete the default SIL restrictions.
//
//    Brad Whitlock, Tue Apr 16 12:54:41 PDT 2002
//    Closed the pipes only if the viewer is multithreaded.
//
//    Jeremy Meredith, Wed May  8 15:39:52 PDT 2002
//    Added keyframe attributes.
//
//    Brad Whitlock, Mon Aug 19 16:01:34 PST 2002
//    I removed the animationAtts member.
//
//    Eric Brugger, Mon Nov 25 09:37:08 PST 2002
//    I moved the keyframe attribute subject to the window manager.
//
//    Brad Whitlock, Mon Dec 2 13:37:14 PST 2002
//    I removed the colorTables member.
//
//    Brad Whitlock, Fri Dec 27 14:56:13 PST 2002
//    I moved the code to close the mdservers to another method.
//
//    Brad Whitlock, Thu Jan 9 11:51:32 PDT 2003
//    I removed the code to delete the borders, shift, preshift, and
//    geometry arrays since they are now strings.
//
//    Brad Whitlock, Fri May 16 14:53:44 PST 2003
//    I added code to delete configFileName.
//
// ****************************************************************************

ViewerSubject::~ViewerSubject()
{
    delete messageBuffer;
    delete viewerRPCObserver;
    delete plotFactory;
    delete operatorFactory;
    delete configMgr;
    delete messageAtts;
    delete statusAtts;
    delete pluginAtts;
    delete appearanceAtts;
    delete syncAtts;
    delete syncObserver;
    delete configFileName;

#ifdef VIEWER_MT
    if(messagePipe[0] != -1)
        close(messagePipe[0]);

    if(messagePipe[1] != -1)
        close(messagePipe[1]);
#endif
}

// ****************************************************************************
//  Method: ViewerSubject::Connect
//
//  Purpose:
//    Connect to the parent process.
//
//  Arguments:
//    argc      The argc from the command line which contains information
//              about which ports to use.
//    argv      The argv from the command line which contains information
//              about which ports to use.
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Sep 19 19:01:45 PST 2000
//    I connected the host profile list to xfer.
//
//    Brad Whitlock, Thu Sep 28 11:05:51 PDT 2000
//    I added an rpc to write a config file. I also added code to create
//    a ViewerConfigurationManager object to read defaults from the config
//    file.
//
//    Kathleen Bonnell, Wed Oct 11 08:52:24 PDT 2000
//    I connected ViewerOnionPeelOperator to xfer and configMgr.
//
//    Brad Whitlock, Thu Oct 5 19:41:31 PST 2000
//    I changed how this class hooks up Xfer to the parent process.
//
//    Brad Whitlock, Fri Oct 27 14:13:16 PST 2000
//    I removed the VisApplication class and altered the code to work with Qt.
//
//    Brad Whitlock, Fri Nov 10 15:21:25 PST 2000
//    I added the material plot.
//
//    Eric Brugger, Thu Dec 21 11:41:11 PST 2000
//    I modified the routine to treat plots generically using the plot
//    factory.
//
//    Brad Whitlock, Fri Feb 9 15:38:04 PST 2001
//    Connected the ViewerWindowManager's new SaveImageAttributes state object.
//
//    Eric Brugger, Thu Mar  8 16:35:03 PST 2001
//    Changed the order of connecting some of the attribute subjects to
//    match the viewer proxy.
//
//    Brad Whitlock, Mon Apr 23 13:13:16 PST 2001
//    Added the messageAtts subject to xfer.
//
//    Brad Whitlock, Mon Apr 30 14:24:49 PST 2001
//    Added statusAtts and engineList subjects to xfer.
//
//    Brad Whitlock, Thu Jun 14 16:01:26 PST 2001
//    Added the viewer color table object.
//
//    Kathleen Bonnell, Mon Jun 18 14:56:09 PDT 2001 
//    I added annotationAtts. 
//
//    Jeremy Meredith, Tue Jul  3 11:03:37 PDT 2001
//    I renamed ReadFromParent to ReadFromParentAndProcess. 
//
//    Brad Whitlock, Thu Jun 21 13:46:12 PST 2001
//    Connected the plot list's new SIL restriction attributes to xfer.
//
//    Jeremy Meredith, Thu Jul 26 03:15:22 PDT 2001
//    Added support for real operator plugins.
//
//    Brad Whitlock, Fri Jul 20 14:46:10 PST 2001
//    Connected the view state objects.
//
//    Brad Whitlock, Thu Aug 30 09:04:46 PDT 2001
//    Modified how the annotation attributes are connected.
//
//    Jeremy Meredith, Wed Sep  5 14:05:47 PDT 2001
//    Added plugin manager attributes.
//
//    Brad Whitlock, Wed Sep 5 00:32:34 PDT 2001
//    Added appearance attributes.
//
//    Brad Whitlock, Mon Sep 24 14:07:17 PST 2001
//    Added code to set the localHost name with the ViewerEngineManager.
//
//    Jeremy Meredith, Fri Sep 28 13:46:46 PDT 2001
//    Attach plugin manager attributes to xfer and the config manager.
//
//    Brad Whitlock, Fri Sep 14 14:14:47 PST 2001
//    Connected the light list.
//
//    Brad Whitlock, Fri Nov 2 11:32:05 PDT 2001
//    Connected the ViewerWindowManager's windowAtts to the config manager
//    so viewer window locations and sizes are saved to the config file.
//
//    Eric Brugger, Mon Nov 19 15:42:07 PST 2001
//    I added animation attributes.
//
//    Brad Whitlock, Mon Sep 17 11:06:00 PDT 2001
//    Added syncAtts.
//
//    Kathleen Bonnell, Wed Nov 28 09:48:18 PST 2001
//    I added pick attributes.
//
//    Brad Whitlock, Tue Feb 19 13:15:29 PST 2002
//    Made the state objects get initialized from both the system wide
//    settings and the local settings.
//
//    Brad Whitlock, Wed Feb 20 14:33:56 PST 2002
//    I added the printer attributes.
//
//    Brad Whitlock, Thu Apr 11 15:48:26 PST 2002
//    Added code to set the default user name.
//
//    Brad Whitlock, Mon Mar 25 17:00:08 PST 2002
//    I modified the communication interface.
//
//    Jeremy Meredith, Wed May  8 15:39:57 PDT 2002
//    Added keyframe attributes.
//
//    Hank Childs, Thu May 23 18:41:19 PDT 2002
//    Renamed SaveImageAttributes to SaveWindowAttributes.
//
//    Brad Whitlock, Mon Aug 19 16:01:57 PST 2002
//    I removed the animationAtts member.
//
//    Brad Whitlock, Mon Sep 16 15:29:35 PST 2002
//    I connected the WindowInformation and the RenderingAttributes.
//
//    Brad Whitlock, Fri Sep 6 14:14:22 PST 2002
//    I connected the query list.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options.
//
//    Kathleen Bonnell, Fri Nov 15 09:07:36 PST 2002  
//    Connect  PickAttributes from ViewerQueryManager instead of 
//    ViewerWindowManager. 
//
//    Eric Brugger, Mon Nov 25 09:37:08 PST 2002
//    I moved the keyframe attribute subject to the window manager.
//
//    Brad Whitlock, Thu Feb 27 11:23:02 PDT 2003
//    I added code to set up special opcodes.
//
//    Brad Whitlock, Wed Mar 12 10:33:20 PDT 2003
//    I added iconifyOpcode.
//
// ****************************************************************************

void
ViewerSubject::Connect(int *argc, char ***argv)
{
    //
    // Connect to the parent.
    //
    parent.Connect(argc, argv, true);

    //
    // Get the localhost name from the parent and give it to the
    // ViewerEngineManager so it can use it when needed.
    //
    ViewerEngineManager::Instance()->SetLocalHost(parent.GetHostName());

    //
    // Create an xfer object for communicating the RPCs.
    //
    xfer.Add(&viewerRPC);
    xfer.SetInputConnection(parent.GetWriteConnection());
    xfer.SetOutputConnection(parent.GetReadConnection());

    //
    // Create an observer for the viewerRPC object. The RPC's are actually
    // handled by the ViewerSubject by a slot function.
    //
    viewerRPCObserver = new ViewerRPCObserver(&viewerRPC);
    connect(viewerRPCObserver, SIGNAL(executeRPC()),
            this, SLOT(HandleViewerRPC()));

    //
    // Create an observer for the syncAtts object. Each time the object
    // updates, send the attributes back to the client.
    //
    syncObserver = new ViewerRPCObserver(syncAtts);
    connect(syncObserver, SIGNAL(executeRPC()),
            this, SLOT(HandleSync()));

    //
    // Create a QSocketNotifier that tells us to call ReadFromParentAndProcess.
    //
    if(parent.GetWriteConnection())
    {
        if(parent.GetWriteConnection()->GetDescriptor() != -1)
        {
            checkParent = new QSocketNotifier(
                parent.GetWriteConnection()->GetDescriptor(),
                QSocketNotifier::Read);
            connect(checkParent, SIGNAL(activated(int)),
                    this, SLOT(ReadFromParentAndProcess(int)));
        }
    }

    // Connect the client attribute subjects.
    xfer.Add(syncAtts);
    xfer.Add(appearanceAtts);
    xfer.Add(pluginAtts);
    xfer.Add(ViewerWindowManager::GetClientAtts());
    xfer.Add(ViewerPlotList::GetClientAtts());
    xfer.Add(ViewerEngineManager::GetClientAtts());
    xfer.Add(messageAtts);
    xfer.Add(ViewerWindowManager::GetSaveWindowClientAtts());
    xfer.Add(statusAtts);
    xfer.Add(ViewerEngineManager::GetEngineList());
    xfer.Add(avtColorTables::Instance()->GetColorTables());
    xfer.Add(ViewerExpressionList::Instance()->GetList());
    xfer.Add(ViewerWindowManager::Instance()->GetAnnotationClientAtts());
    xfer.Add(ViewerPlotList::GetClientSILRestrictionAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetView2DClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetView3DClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetLightListClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetAnimationClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetPickAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetPrinterClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetWindowInformation());
    xfer.Add(ViewerWindowManager::Instance()->GetRenderingAttributes());
    xfer.Add(ViewerWindowManager::Instance()->GetKeyframeClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetQueryTypes());
    xfer.Add(ViewerQueryManager::Instance()->GetQueryClientAtts());
    xfer.Add(ViewerEngineManager::GetMaterialClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetGlobalLineoutClientAtts());

    int i;
    for (i = 0; i < plotFactory->GetNPlotTypes(); ++i)
    {
        AttributeSubject *attr = plotFactory->GetClientAtts(i);

        if (attr != 0)
            xfer.Add(attr);
    }

    for (i = 0; i < operatorFactory->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *attr = operatorFactory->GetClientAtts(i);

        if (attr != 0)
            xfer.Add(attr);
    }

    // List the objects connected to xfer.
    xfer.ListObjects();

    // Set up special opcodes and their handler.
    animationStopOpcode = xfer.CreateNewSpecialOpcode();
    iconifyOpcode = xfer.CreateNewSpecialOpcode();
    xfer.SetupSpecialOpcodeHandler(SpecialOpcodeCallback, (void *)this);

    //
    // Create the configuration manager and connect objects that can be
    // written to the config file.
    //
    configMgr->Add(ViewerWindowManager::GetClientAtts());
    configMgr->Add(ViewerEngineManager::GetClientAtts());
    for (i = 0; i < plotFactory->GetNPlotTypes(); ++i)
    {
        AttributeSubject *attr = plotFactory->GetDefaultAtts(i);

        if (attr != 0)
            configMgr->Add(attr);
    }
    for (i = 0; i < operatorFactory->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *attr = operatorFactory->GetDefaultAtts(i);

        if (attr != 0)
            configMgr->Add(attr);
    }
    configMgr->Add(ViewerWindowManager::GetSaveWindowClientAtts());
    configMgr->Add(avtColorTables::Instance()->GetColorTables());
    configMgr->Add(ViewerExpressionList::Instance()->GetList());
    configMgr->Add(ViewerWindowManager::GetAnimationClientAtts());
    configMgr->Add(ViewerWindowManager::GetAnnotationDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetView2DClientAtts());
    configMgr->Add(ViewerWindowManager::GetView3DClientAtts());
    configMgr->Add(ViewerWindowManager::GetLightListDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetWindowAtts());
    configMgr->Add(ViewerWindowManager::GetWindowInformation());
    configMgr->Add(ViewerWindowManager::GetPrinterClientAtts());
    configMgr->Add(ViewerWindowManager::GetRenderingAttributes());
    configMgr->Add(ViewerEngineManager::GetMaterialDefaultAtts());

    // Set the default user name.
    HostProfile::SetDefaultUserName(parent.GetTheUserName());

    // Make the hooked up state objects set their properties from
    // both the system and local settings.
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);
    configMgr->Notify();
    delete systemSettings; systemSettings = 0;
    delete localSettings;  localSettings = 0;

    // Add the appearanceAtts *after* the config settings have been read. This
    // prevents overwriting the attributes and sending them to the client.
    configMgr->Add(appearanceAtts);

    // Add the pluginAtts *after* the config settings have been read.
    // First, tell the client which plugins we've really loaded.
    pluginAtts->Notify();
    configMgr->Add(pluginAtts);

    // Copy the default annotation attributes into the client annotation
    // attributes.
    ViewerWindowManager::SetClientAnnotationAttsFromDefault();

    // Copy the default material atts to the client material atts
    ViewerEngineManager::SetClientMaterialAttsFromDefault();

    // Send the queries to the client.
    ViewerQueryManager::Instance()->GetQueryTypes()->Notify();

    // Set the query manager's operator factory pointer.
    ViewerQueryManager::Instance()->SetOperatorFactory(operatorFactory);
}

// ****************************************************************************
//  Method: ViewerSubject::Execute
//
//  Purpose:
//    Execute the viewer subject.
//
//  Programmer: Eric Brugger
//  Creation:   August 16, 2000
//
//  Modifications:
//    Brad Whitlock, Wed Nov 1 14:56:23 PST 2000
//    I made it use Qt and return a return code.
//
//    Brad Whitlock, Mon Nov 27 14:24:02 PST 2000
//    I added code to connect some of ViewerWindowManager's signals to
//    ViewerSubject's slots.
//
//    Brad Whitlock, Fri May 25 16:45:22 PST 2001
//    Added code to print a message if a LostConnectionException is encountered.
//
//    Jeremy Meredith, Fri Jul 20 11:25:10 PDT 2001
//    Added call to SetShift.
//
//    Jeremy Meredith, Fri Sep 14 13:30:17 PDT 2001
//    Added call to SetPreshift.
//
//    Brad Whitlock, Mon Oct 22 18:52:46 PST 2001
//    Changed the exception handling keywords to macros.
//
//    Jeremy Meredith, Thu Dec 19 12:13:34 PST 2002
//    Added support for launching engines from the command line.
//
//    Brad Whitlock, Fri Dec 27 12:41:32 PDT 2002
//    I passed an empty string vector to the CreateEngine method because
//    the engine manager now passes the ViewerServerManager's unknown
//    arguments to the engines that get launched.
//   
//    Kathleen Bonnell, Fri Feb  7 09:09:47 PST 2003  
//    Added registration of the authentication callback. (moved from viewer.C)
//    
// ****************************************************************************

int
ViewerSubject::Execute()
{
    //
    // Create the window.
    //
    ViewerWindowManager *windowManager=ViewerWindowManager::Instance();
    if(windowManager != NULL)
    {
        // Connect
        connect(windowManager, SIGNAL(createWindow(ViewerWindow *)),
                this, SLOT(ConnectWindow(ViewerWindow *)));
        connect(windowManager, SIGNAL(deleteWindow(ViewerWindow *)),
                this, SLOT(DisconnectWindow(ViewerWindow *)));

        // Initialize the area that will be used to place the windows.
        InitializeWorkArea();

        // Make the window manager add its initial number of windows.
        windowManager->AddInitialWindows();
    }

    //
    // Register a callback function to be called when launching a remote
    // process requires authentication.
    //
#if !defined(_WIN32)
    if (!ViewerWindow::GetNoWinMode())
    {
        RemoteProcess::SetAuthenticationCallback(&ViewerPasswordWindow::authenticate);
    }
#endif

    //
    // Launch an engine if needed, never popping up the chooser window
    //
    if (launchEngineAtStartup != "")
    {
        stringVector noArgs;
        ViewerEngineManager::Instance()->CreateEngine(launchEngineAtStartup.c_str(),
                                                      noArgs, true, numEngineRestarts);
    }

    //
    // Enter the event processing loop.
    //
    int retval;
    TRY
    {
        retval = mainApp->exec();
    }
    CATCH(LostConnectionException)
    {
        cerr << "The component that launched VisIt's viewer has terminated "
                "abnormally." << endl;
        retval = -1;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: ViewerSubject::ProcessEventsCB
//
// Purpose: 
//   This is a static callback function that calls ProcessEvents.
//
// Arguments:
//   cbData : A pointer to a ViewerSubject.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 7 16:27:22 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ProcessEventsCB(void *cbData)
{
    if(cbData)
    {
         ViewerSubject *This = (ViewerSubject *)cbData;
         This->ProcessEvents();
    }
}

// ****************************************************************************
// Method: ViewerSubject::ProcessEvents
//
// Purpose: 
//   This method is called from other loops where we do not have control.
//   For the most part this is called from the inner loop of certain RPCs
//   where the work lasts long enough where we want to still redraw, etc.
//   
// Programmer: Brad Whitlock
// Creation:   Tue May 7 16:25:01 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu May 9 11:41:33 PDT 2002
//   Only process events if we allow interruption. This makes it so we can
//   prevent synchronous clients from going too fast for the viewer.
//
// ****************************************************************************

void
ViewerSubject::ProcessEvents()
{
    if(interruptionEnabled)
    {
         mainApp->processEvents(10);
    }
}

// ****************************************************************************
// Method: ViewerSubject::InitializeWorkArea
//
// Purpose: 
//   Determines how much of the screen will be allocated to viewer windows.
//
// Programmer: Brad Whitlock
// Creation:   Mon Feb 4 16:37:15 PST 2002
//
// Modifications:
//    Jeremy Meredith, Tue Apr 23 15:51:57 PDT 2002
//    Added test to set some good numbers if running in -nowin mode.
//
//    Brad Whitlock, Thu Jan 9 11:52:21 PDT 2003
//    I made borders, shift, preshift, and geometry be strings.
//
// ****************************************************************************

void
ViewerSubject::InitializeWorkArea()
{
    char           tmp[50];
    int            x, y, w, h;
    WindowMetrics *wm = 0;

    if (nowin)
    {
        if (borders.size() == 0)
        {
            borders  = "0,0,0,0";
        }
        if (shift.size() == 0)
        {
            shift    = "0,0";
        }
        if (preshift.size() == 0)
        {
            preshift = "0,0";
        }
        if (geometry.size() == 0)
        {
            if (smallWindow)
                geometry = "1024x1024";
            else
                geometry = "512x512";
        }
    }
    else
    {
        //
        // If any of the options are missing then use the WindowMetrics
        // class to fill in the blanks.
        //
        if(borders.size() == 0 || shift.size() == 0 ||
           preshift.size() == 0 || geometry.size() == 0)
        {
            wm = WindowMetrics::Instance();
        }

        //
        // Use WindowMetrics to determine the borders.
        //
        if(borders.size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d,%d,%d",
                     wm->GetBorderT(), wm->GetBorderB(),
                     wm->GetBorderL(), wm->GetBorderR());
            borders = tmp;
        }

        //
        // Use WindowMetrics to determine the shift.
        //
        if(shift.size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d", wm->GetShiftX(), wm->GetShiftY());
            shift = tmp;
        }

        //
        // Use WindowMetrics to determine the preshift.
        //
        if(preshift.size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d", wm->GetShiftX(), wm->GetShiftY());
            preshift = tmp;
        }

        //
        // Use WindowMetrics to determine the geometry.
        //
        if(geometry.size() == 0)
        {
            int h1 = int(wm->GetScreenW() * 0.8);
            int h2 = int(wm->GetScreenH() * 0.8);
            h = (h1 < h2) ? h1 : h2;
            w = h;
            x = wm->GetScreenX() + wm->GetScreenW() - w;
            y = wm->GetScreenY();
            if(smallWindow)
            {
                w /= 2; h /= 2; x += w;
            }

            SNPRINTF(tmp, 50, "%dx%d+%d+%d", w, h, x, y);
            geometry = tmp;
        }
        else if(smallWindow)
        {
            if(sscanf(geometry.c_str(), "%dx%d+%d+%d", &w, &h, &x, &y) == 4)
            {
                w /= 2;
                h /= 2;
                SNPRINTF(tmp, 50, "%dx%d+%d+%d", w, h, x, y);
                geometry = tmp;
            }
        }

        //
        // Delete the WindowMetrics object.
        //
        delete wm;
    }

    //
    // Set the options in the viewer window manager.
    //
    ViewerWindowManager *windowManager=ViewerWindowManager::Instance();
    windowManager->SetBorders(borders.c_str());
    windowManager->SetShift(shift.c_str());
    windowManager->SetPreshift(preshift.c_str());
    windowManager->SetGeometry(geometry.c_str());
}

// ****************************************************************************
// Method: ViewerSubject::CustomizeAppearance
//
// Purpose: 
//   Customizes the viewer's appearance based on command line arguments and
//   settings in the configuration file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 5 10:22:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CustomizeAppearance()
{
    //
    // Set the style and inform the widgets.
    //
    if(appearanceAtts->GetStyle() == "cde")
        mainApp->setStyle(new QCDEStyle);
    else if(appearanceAtts->GetStyle() == "windows")
        mainApp->setStyle(new QWindowsStyle);
    else if(appearanceAtts->GetStyle() == "platinum")
        mainApp->setStyle(new QPlatinumStyle);
#if QT_VERSION >= 230
    else if(appearanceAtts->GetStyle() == "sgi")
        mainApp->setStyle(new QSGIStyle);
#endif
    else
        mainApp->setStyle(new QMotifStyle);

    //
    // Set the colors and inform the widgets.
    //
    QColor bg(appearanceAtts->GetBackground().c_str());
    QColor fg(appearanceAtts->GetForeground().c_str());
    QColor btn(bg);

    // Put the converted RGB format color into the appearance attributes.
    char tmp[20];
    SNPRINTF(tmp, 20, "#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
    appearanceAtts->SetBackground(tmp);
    SNPRINTF(tmp, 20, "#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
    appearanceAtts->SetForeground(tmp);

    int h,s,v;
    fg.hsv(&h,&s,&v);
    QColor base = Qt::white;
    bool bright_mode = false;
    if(v >= 255 - 50)
    {
        base = btn.dark(150);
        bright_mode = TRUE;
    }

    QColorGroup cg(fg, btn, btn.light(),
                   btn.dark(), btn.dark(150), fg, Qt::white, base, bg);
    if (bright_mode)
    {
        cg.setColor(QColorGroup::HighlightedText, base );
        cg.setColor(QColorGroup::Highlight, Qt::white );
    }
    else
    {
        cg.setColor(QColorGroup::HighlightedText, Qt::white );
        cg.setColor(QColorGroup::Highlight, Qt::darkBlue );
    }
    QColor disabled((fg.red()+btn.red())/2,
                    (fg.green()+btn.green())/2,
                    (fg.blue()+btn.blue())/2);
    QColorGroup dcg(disabled, btn, btn.light( 125 ), btn.dark(), btn.dark(150),
                    disabled, Qt::white, Qt::white, bg );
    if(bright_mode)
    {
        dcg.setColor(QColorGroup::HighlightedText, base);
        dcg.setColor(QColorGroup::Highlight, Qt::white);
    }
    else
    {
        dcg.setColor(QColorGroup::HighlightedText, Qt::white);
        dcg.setColor(QColorGroup::Highlight, Qt::darkBlue);
    }
    QPalette pal(cg, dcg, cg);
    mainApp->setPalette(pal, true);
}

// ****************************************************************************
//  Method: ViewerSubject::GetPlotFactory
//
//  Purpose:
//    Return a pointer to the ViewerPlotFactory.
//
//  Returns:    A pointer to the ViewerPlotFactory.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerPlotFactory *
ViewerSubject::GetPlotFactory() const
{
    return plotFactory;
}

// ****************************************************************************
//  Method: ViewerSubject::GetOperatorFactory
//
//  Purpose:
//    Return a pointer to the ViewerOperatorFactory.
//
//  Returns:    A pointer to the ViewerOperatorFactory.
//
//  Programmer: Eric Brugger
//  Creation:   September 18, 2000
//
//  Modifications:
//
// ****************************************************************************

ViewerOperatorFactory *
ViewerSubject::GetOperatorFactory() const
{
    return operatorFactory;
}

// ****************************************************************************
//  Method: ViewerSubject::ProcessCommandLine1
//
//  Purpose:
//    Process the viewer command line arguments.
//
//  Arguments:
//    argc      The number of command line arguments.
//    argv      The command line arguments.
//
//  Programmer: Jeremy Meredith
//  Creation:   April 17, 2002
//
//  Modifications:
//    Brad Whitlock, Fri May 16 14:54:28 PST 2003
//    I added support for the -config flag to read a named config file.
//
// ****************************************************************************

void
ViewerSubject::ProcessCommandLine1(int *argc, char ***argv)
{
    int    argc2 = *argc;
    char **argv2 = *argv;

    //
    // Preprocess the command line for the viewer.
    //
    for (int i = 1 ; i < argc2 ; i++)
    {
        if (strcmp(argv2[i], "-noconfig") == 0)
        {
            noconfig = true;
            if(configFileName != 0)
            {
                delete [] configFileName;
                configFileName = 0;
            }
        }
        else if (strcmp(argv2[i], "-config") == 0 && (i+1) < argc2 && !noconfig)
        {
            if(configFileName != 0)
            {
                delete [] configFileName;
                configFileName = 0;
            }
            int len = strlen(argv2[i+1]);
            configFileName = new char[len + 1];
            strcpy(configFileName, argv2[i+1]);
        }       
    }
}

// ****************************************************************************
//  Method: ViewerSubject::ProcessCommandLine2
//
//  Purpose:
//    Process the viewer command line arguments.
//
//  Arguments:
//    argc      The number of command line arguments.
//    argv      The command line arguments.
//
//  Programmer: Eric Brugger
//  Creation:   September 13, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Nov 27 17:18:46 PST 2000
//    Added code to handle the -debug flag.
//
//    Jeremy Meredith, Fri Apr 20 10:32:51 PDT 2001
//    Added code to collect "other" options to pass them to remote processes.
//
//    Jeremy Meredith, Fri Jul 20 11:25:28 PDT 2001
//    Added parsing of 'shift'.
//
//    Brad Whitlock, Wed Sep 5 09:44:40 PDT 2001
//    Added code to process certain color, style arguments.
//
//    Jeremy Meredith, Fri Sep 14 13:31:19 PDT 2001
//    Added parsing of 'preshift'.
//
//    Eric Brugger, Mon Nov 5 14:33:21
//    Added code to parse 'timing'.
//
//    Brad Whitlock, Fri Jan 4 17:30:50 PST 2002
//    Added code to parse 'noint'.
//
//    Hank Childs, Mon Jan 28 16:14:47 PST 2002
//    Added support for nowin mode.
//
//    Brad Whitlock, Mon Feb 4 16:41:48 PST 2002
//    Added code to parse 'small'.
//
//    Jeremy Meredith, Tue Apr 23 15:53:21 PDT 2002
//    Added setting of internal flag to keep track of nowin mode.
//
//    Hank Childs, Tue Apr 23 19:36:57 PDT 2002
//    Made a call to avt callback indicating nowin mode.
//
//    Brad Whitlock, Fri May 3 16:40:50 PST 2002
//    Made it use ViewerServerManager.
//
//    Jeremy Meredith, Sat Aug 17 11:06:20 PDT 2002
//    Added nowin support for ViewerEngineChooser.
//
//    Jeremy Meredith, Tue Nov 19 17:14:28 PST 2002
//    Added stereo flag.
//
//    Jeremy Meredith, Thu Dec 19 12:08:46 PST 2002
//    Added support for launching engines from the command line.
//
//    Brad Whitlock, Fri Dec 27 12:19:38 PDT 2002
//    I added code to strip off the -key argument and I made borders, shift,
//    preshift, and geometry be real strings so the viewer does not crash
//    on exit in some situations.
//
// ****************************************************************************

void
ViewerSubject::ProcessCommandLine2(int *argc, char ***argv)
{
    int    argc2 = *argc;
    char **argv2 = *argv;

    //
    // Process the command line for the viewer.
    //
    for (int i = 1 ; i < argc2 ; i++)
    {
        if (strcmp(argv2[i], "-borders") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "Borders string missing for -borders option" << endl;
                continue;
            }
            borders = argv2[i+1];
            i += 1;
        }
        else if (strcmp(argv2[i], "-shift") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "Shift string missing for -shift option" << endl;
                continue;
            }
            shift = argv2[i+1];
            i += 1;
        }
        else if (strcmp(argv2[i], "-preshift") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "Preshift string missing for -preshift option" << endl;
                continue;
            }
            preshift = argv2[i+1];
            i += 1;
        }
        else if (strcmp(argv2[i], "-geometry") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "Geometry string missing for -geometry option" << endl;
                continue;
            }
            geometry = argv2[i+1];
            i += 1;
        }
        else if (strcmp(argv2[i], "-small") == 0)
        {
            smallWindow = true;
        }
        else if (strcmp(argv2[i], "-debug") == 0)
        {
            int debugLevel = 1; 
            if (i+1 < argc2 && isdigit(*(argv2[i+1])))
               debugLevel = atoi(argv2[++i]);
            else
               cerr << "Warning: debug level not specified, assuming 1" << endl;
            if(debugLevel > 0 && debugLevel < 6)
            {
                ViewerEngineManager::SetDebugLevel(debugLevel);
                ViewerFileServer::SetDebugLevel(debugLevel);
            }
        }
        else if (strcmp(argv2[i], "-host")     == 0 ||
                 strcmp(argv2[i], "-port")     == 0 ||
                 strcmp(argv2[i], "-nread")    == 0 ||
                 strcmp(argv2[i], "-nwrite")   == 0 ||
                 strcmp(argv2[i], "-nborders") == 0)
        {
            // this argument and the following option are dangerous to pass on
            i++;
        }
        else if(strcmp(argv2[i], "-background") == 0 ||
                strcmp(argv2[i], "-bg") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -background option must be followed by a color."
                     << endl;
                continue;
            }
            // Store the background color in the viewer's appearance
            // attributes so the gui will be colored properly on startup.
            appearanceAtts->SetBackground(std::string(argv2[i+1]));
            ++i;
        }
        else if(strcmp(argv2[i], "-foreground") == 0 ||
                strcmp(argv2[i], "-fg") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -foreground option must be followed by a color."
                     << endl;
                continue;
            }
            // Store the foreground color in the viewer's appearance
            // attributes so the gui will be colored properly on startup.
            appearanceAtts->SetForeground(std::string(argv2[i+1]));
            ++i;
        }
        else if(strcmp(argv2[i], "-style") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -style option must be followed by a style name."
                     << endl;
                continue;
            }
            if(strcmp(argv2[i + 1], "motif") == 0 ||
               strcmp(argv2[i + 1], "cde") == 0 ||
               strcmp(argv2[i + 1], "windows") == 0 ||
               strcmp(argv2[i + 1], "platinum") == 0
#if QT_VERSION >= 230
               || strcmp(argv2[i + 1], "sgi") == 0
#endif
                     )
            {
                appearanceAtts->SetStyle(argv2[i+1]);
            }
            ++i;
        }
        else if(strcmp(argv2[i], "-font") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -font option must be followed by a "
                        "font description." << endl;
                continue;
            }
            appearanceAtts->SetFontDescription(argv2[i + 1]);
            ++i;
        }
        else if(strcmp(argv2[i], "-timing") == 0)
        {
            //
            // Enable timing and pass the option to child processes.
            //
            visitTimer->Enable();

            unknownArguments.push_back(argv2[i]);
        }
        else if(strcmp(argv2[i], "-noint") == 0)
        {
            interruptionEnabled = false;
        }
        else if (strcmp(argv2[i], "-noconfig") == 0)
        {
            // do nothing; processed by an earlier parsing of the command line
        }
        else if (strcmp(argv2[i], "-nowin") == 0)
        {
            InitVTK::ForceMesa();
            ViewerWindow::SetNoWinMode(true);
            ViewerEngineChooser::SetNoWinMode(true);
            avtCallback::SetNowinMode(true);
            nowin = true;
        }
        else if (strcmp(argv2[i], "-stereo") == 0)
        {
            VisWinRendering::SetStereoEnabled(true);
        }
        else if (strcmp(argv2[i], "-launchengine") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -launchengine option must be followed by a "
                        "host name." << endl;
                continue;
            }
            launchEngineAtStartup = argv2[i+1];
            ++i;
        }
        else if (strcmp(argv2[i], "-key") == 0)
        {
            if(i + 1 >= argc2)
            {
                cerr << "The -key option must be followed by a key." << endl;
                continue;
            }
            // Don't do anything with the key. Just skip over it.
            ++i;
        }
        else if (strcmp(argv2[i], "-numrestarts") == 0)
        {
            if ((i + 1 >= argc2) || (!isdigit(*(argv2[i+1]))))
            {
                cerr << "The -numrestarts option must be followed by an "
                        "integer number." << endl;
                continue;
            }
            numEngineRestarts = atoi(argv2[++i]);
        }
        else // Unknown argument -- add it to the list
        {
            unknownArguments.push_back(argv2[i]);
        }
    }

    ViewerServerManager::SetArguments(unknownArguments);
}

// ****************************************************************************
//  Method: ViewerSubject::MessageRendererThread
//
//  Purpose:
//    Send a message to the rendering thread.
//
//  Arguments:
//    message   The message to send to the rendering thread.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Apr 16 12:42:56 PDT 2002
//    Added a single-thread implementation that does not use pipes.
//
// ****************************************************************************

void
ViewerSubject::MessageRendererThread(const char *message)
{
#ifdef VIEWER_MT
    int msglen = strlen(message);
    if (write(this->messagePipe[1], message, msglen) != msglen)
    {
        cerr << "Error sending a message to the master thread.\n";
    }
#else
    messageBuffer->AddString(message);
    QTimer::singleShot(1, this, SLOT(ProcessRendererMessage()));
#endif
}

// ****************************************************************************
// Method: ViewerSubject::Error
//
// Purpose: 
//   Sends an error message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//
// ****************************************************************************

void
ViewerSubject::Error(const char *message)
{
    if((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Error);
    messageAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Warning
//
// Purpose: 
//   Sends a warning message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//   
// ****************************************************************************

void
ViewerSubject::Warning(const char *message)
{
    if((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Warning);
    messageAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Message
//
// Purpose: 
//   Sends a message to the GUI.
//
// Arguments:
//   message : The message that gets sent to the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 23 13:41:01 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue May 20 14:24:51 PST 2003
//   Updated MessageAttributes.
//   
// ****************************************************************************

void
ViewerSubject::Message(const char *message)
{
    if((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Message);
    messageAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Status
//
// Purpose: 
//   Sends a status message to the GUI.
//
// Arguments:
//   message : The status message that is sent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:38:09 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerSubject::Status(const char *message)
{
    statusAtts->SetSender("viewer");
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(1);
    statusAtts->SetMessage(message);
    statusAtts->SetDuration(StatusAttributes::DEFAULT_DURATION);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Status
//
// Purpose: 
//   Sends a status message to the GUI.
//
// Arguments:
//   message      : The status message that is sent.
//   milliseconds : The duration of time that the status message is displayed.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 21 13:24:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::Status(const char *message, int milliseconds)
{
    statusAtts->SetSender("viewer");
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(1);
    statusAtts->SetMessage(message);
    statusAtts->SetDuration(milliseconds);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Status
//
// Purpose: 
//   Sends a status message for a component other than the viewer.
//
// Arguments:
//   sender  : The name of the component sending the message.
//   message : The message to be sent.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 1 12:06:22 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//   
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerSubject::Status(const char *sender, const char *message)
{
    statusAtts->SetSender(sender);
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(1);
    statusAtts->SetMessage(message);
    statusAtts->SetDuration(StatusAttributes::DEFAULT_DURATION);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::Status
//
// Purpose: 
//   Sends a status message for a component other than the viewer. This
//   message includes percent done, etc.
//
// Arguments:
//   sender       : The component that sent the status update. This is a
//                  host name for an engine that sent the message.
//   percent      : The percent through the current stage.
//   curStage     : The number of the current stage.
//   curStageName : The name of the current stage.
//   maxStage     : The maximum number of stages.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:39:30 PST 2001
//
// Modifications:
//    Jeremy Meredith, Fri Jun 29 15:09:08 PDT 2001
//    Added the MessageType field.
//   
//    Brad Whitlock, Fri Sep 21 13:26:46 PST 2001
//    Added the duration field.
//
// ****************************************************************************

void
ViewerSubject::Status(const char *sender, int percent, int curStage,
    const char *curStageName, int maxStage)
{
    statusAtts->SetSender(sender);
    statusAtts->SetClearStatus(false);
    statusAtts->SetMessageType(2);
    statusAtts->SetPercent(percent);
    statusAtts->SetCurrentStage(curStage);
    statusAtts->SetCurrentStageName(curStageName);
    statusAtts->SetMaxStage(maxStage);
    statusAtts->SetDuration(StatusAttributes::DEFAULT_DURATION);
    statusAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::ClearStatus
//
// Purpose: 
//   Sends an empty message to the status bar to clear it.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 14:33:37 PST 2001
//
// Modifications:
//    Jeremy Meredith, Tue Jul  3 15:21:37 PDT 2001
//    Also set all percentages/stages to zeroes.
//
// ****************************************************************************

void
ViewerSubject::ClearStatus(const char *sender)
{
    statusAtts->SetSender((sender == 0) ? "viewer" : sender);
    statusAtts->SetClearStatus(true);
    statusAtts->SetPercent(0);
    statusAtts->SetCurrentStage(0);
    statusAtts->SetCurrentStageName("");
    statusAtts->SetMaxStage(0);
    statusAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerSubject::Close
//
//  Purpose:
//    Execute the Close RPC.
//
//  Programmer: Eric Brugger
//  Creation:   August 11, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 19 11:41:15 PDT 2001
//    Added code to clear the work procedure.
//
//    Eric Brugger, Mon Oct 29 09:47:30 PST 2001
//    Removed the timer object and work process.
//
//    Jeremy Meredith, Fri Dec 20 11:11:23 PST 2002
//    Added code to hide the windows before trying to close the engines.
//    This makes it feel more responsive.
//
//    Brad Whitlock, Fri Dec 27 14:55:22 PST 2002
//    I made it close down the meta-data servers too.
//
// ****************************************************************************

void
ViewerSubject::Close()
{
    //
    // Perform the rpc.
    //
    debug1 << "Starting to close the viewer." << endl;
    ViewerWindowManager::Instance()->HideAllWindows();
    ViewerFileServer::Instance()->CloseServers();
    ViewerEngineManager::Instance()->CloseEngines();

    //
    // Break out of the application loop.
    //
    mainApp->exit(0);
}

// ****************************************************************************
// Method: ViewerSubject::CopyAnnotationsToWindow
//
// Purpose: 
//   Copies the annotation attributes from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyAnnotationsToWindow()
{
    int from = viewerRPC.GetWindowLayout();
    int to = viewerRPC.GetWindowId();
    CopyAnnotationsToWindow(from, to);
}

// ****************************************************************************
// Method: ViewerSubject::CopyLightingToWindow
//
// Purpose: 
//   Copies the lighting attributes from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyLightingToWindow()
{
    int from = viewerRPC.GetWindowLayout();
    int to = viewerRPC.GetWindowId();
    CopyLightingToWindow(from, to);
}

// ****************************************************************************
// Method: ViewerSubject::CopyViewToWindow
//
// Purpose: 
//   Copies the view attributes from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyViewToWindow()
{
    int from = viewerRPC.GetWindowLayout();
    int to = viewerRPC.GetWindowId();
    CopyViewToWindow(from, to);
}

// ****************************************************************************
// Method: ViewerSubject::CopyPlotsToWindow
//
// Purpose: 
//   Copies the plots from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:30:02 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyPlotsToWindow()
{
    int from = viewerRPC.GetWindowLayout();
    int to = viewerRPC.GetWindowId();
    CopyPlotsToWindow(from, to);
}

// ****************************************************************************
//  Method: ViewerSubject::ClearAllWindows
//
//  Purpose:
//    Execute the ClearAllWindows RPC.
//
//  Programmer: Eric Brugger
//  Creation:   October 4, 2000
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSubject::ClearAllWindows()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->ClearAllWindows();
}

// ****************************************************************************
//  Method: ViewerSubject::ClearPickPoints
//
//  Purpose: 
//    This is a Qt slot function that clears the pick points from 
//    the specified window.
//
//  Arguments:
//    windowIndex  The index of the window. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 27, 2001 
//
// ****************************************************************************

void
ViewerSubject::ClearPickPoints(int windowIndex)
{
    //
    // Reset the window's view.
    //
    ViewerWindowManager::Instance()->ClearPickPoints(windowIndex);
}

// ****************************************************************************
//  Method: ViewerSubject::ClearRefLines
//
//  Purpose: 
//    This is a Qt slot function that clears the ref lines from 
//    the specified window.
//
//  Arguments:
//    windowIndex  The index of the window. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   April 25, 2002 
//
// ****************************************************************************

void
ViewerSubject::ClearRefLines(int windowIndex)
{
    //
    // Reset the window's view.
    //
    ViewerWindowManager::Instance()->ClearRefLines(windowIndex);
}

// ****************************************************************************
// Method: ViewerSubject::IconifyAllWindows
//
// Purpose: 
//   Iconifies all viewer windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:08:05 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::IconifyAllWindows()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->IconifyAllWindows();
}

// ****************************************************************************
// Method: ViewerSubject::DeIconifyAllWindows
//
// Purpose: 
//   DeIconifies all viewer windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 11:08:38 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DeIconifyAllWindows()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->DeIconifyAllWindows();
}

// ****************************************************************************
// Method: ViewerSubject::ShowAllWindows
//
// Purpose: 
//   Shows all viewer windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:33:19 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
ViewerSubject::ShowAllWindows()
{
    // Perform the rpc.
    ViewerWindowManager::Instance()->ShowAllWindows();
}

// ****************************************************************************
// Method: ViewerSubject::HideAllWindows
//
// Purpose: 
//   Hides all viewer windows.
//
// Programmer: Sean Ahern
// Creation:   Tue Apr 16 12:33:39 PDT 2002
//
// Modifications:
//   
// ****************************************************************************
void
ViewerSubject::HideAllWindows()
{
    // Perform the rpc.
    ViewerWindowManager::Instance()->HideAllWindows();
}
// ****************************************************************************
//  Method: ViewerSubject::SaveWindow
//
//  Purpose:
//    Saves the window.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2001
//
// ****************************************************************************

void
ViewerSubject::SaveWindow()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->SaveWindow();
}

// ****************************************************************************
// Method: ViewerSubject::PrintWindow
//
// Purpose:
//   Prints the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 14:33:11 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::PrintWindow()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->PrintWindow();
}

// ****************************************************************************
// Method: ViewerSubject::DisableRedraw
//
// Purpose: 
//   Disables redraw for the active window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:56:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DisableRedraw()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->DisableRedraw();
}

// ****************************************************************************
// Method: ViewerSubject::RedrawWindow
//
// Purpose: 
//   Redraws the active window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 19 14:57:15 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::RedrawWindow()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->RedrawWindow();
}

// ****************************************************************************
//  Method: ViewerSubject::OpenDatabaseHelper
//
//  Purpose:
//    Opens a database.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Jeremy Meredith, Fri Apr 20 10:33:42 PDT 2001
//    Added code to pass "other" options to the engine when starting.
//
//    Brad Whitlock, Wed Nov 14 17:08:07 PST 2001
//    Added code to set the number of time steps in the animation.
//
//    Brad Whitlock, Wed Sep 11 16:29:27 PST 2002
//    I changed the code so an engine is only launched when the file can
//    be opened.
//
//    Brad Whitlock, Tue Dec 10 15:34:17 PST 2002
//    I added code to tell the engine to open the database.
//
//    Brad Whitlock, Mon Dec 30 14:59:24 PST 2002
//    I changed how nFrames and nStates are set.
//
//    Brad Whitlock, Fri Jan 17 11:35:29 PDT 2003
//    I added code to reset nFrames if there are no plots in the plot list.
//
//    Brad Whitlock, Tue Feb 11 11:56:34 PDT 2003
//    I made it use STL strings.
//
//    Brad Whitlock, Tue Mar 25 14:23:16 PST 2003
//    I made it capable of defining a virtual database.
//
//    Brad Whitlock, Fri Apr 4 11:10:08 PDT 2003
//    I changed how the number of frames in an animation is updated.
//
//    Brad Whitlock, Thu May 15 13:34:19 PST 2003
//    I added the timeState argument and renamed the method.
//
// ****************************************************************************

void
ViewerSubject::OpenDatabaseHelper(const std::string &entireDBName,
    int timeState, bool updateNFrames)
{
    debug1 << "Opening database " << entireDBName.c_str()
           << ", timeState=" << timeState << endl;

    //
    // Associate the database with the currently active animation (window).
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveAnimation()->GetPlotList();

    //
    // Set the new host/database name into the plot list. This splits it.
    //
    plotList->SetHostDatabaseName(entireDBName);
    std::string host(plotList->GetHostName());
    std::string db(plotList->GetDatabaseName());

    //
    // Expand the database name to its full path just in case.
    //
    ViewerFileServer *fs = ViewerFileServer::Instance();
    std::string expandedDB(fs->ExpandedFileName(host, db));
    plotList->SetDatabaseName(expandedDB.c_str());
    db = plotList->GetDatabaseName();

    //
    // Get the number of time states and set that information into the
    // active animation.
    //
    const avtDatabaseMetaData *md = fs->GetMetaData(host, db, timeState);
    if(md != NULL)
    {
        if(updateNFrames)
        {
            // Update the number of animation frames.
            ViewerAnimation *animation = wM->GetActiveAnimation();
            animation->UpdateNFrames();

            // Move to the specified time state.
            if(timeState > 0)
                animation->SetFrameIndex(timeState);

            wM->UpdateGlobalAtts();
        }

        //
        // Create a compute engine to use with the database.
        //
        stringVector noArgs;
        ViewerEngineManager::Instance()->CreateEngine(host.c_str(), noArgs,
                                                      false, numEngineRestarts);

        //
        // Tell the new engine to open the specified database.
        //
        ViewerEngineManager *eMgr = ViewerEngineManager::Instance();
        if(md->GetIsVirtualDatabase() && md->GetNumStates() > 1)
        {
            eMgr->DefineVirtualDatabase(host.c_str(), db.c_str(),
                                        md->GetTimeStepPath().c_str(),
                                        md->GetTimeStepNames(),
                                        timeState);
        }
        else
            eMgr->OpenDatabase(host.c_str(), db.c_str(), timeState);
    }
}

// ****************************************************************************
// Method: ViewerSubject::OpenDatabase
//
// Purpose: 
//   Opens a database.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 15 13:33:17 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::OpenDatabase()
{
    OpenDatabaseHelper(viewerRPC.GetDatabase(), viewerRPC.GetIntArg1(),
                       true);
}

// ****************************************************************************
// Method: ViewerSubject::ReOpendatabase
//
// Purpose: 
//   Reopens a database and updates plots that use it.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 29 15:20:14 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 4 16:25:43 PST 2002
//   I changed it so view recentering does not happen unless the it is
//   enabled in the window.
//
//   Brad Whitlock, Tue Feb 11 12:00:03 PDT 2003
//   I made it use STL strings.
//
//   Brad Whitlock, Thu Apr 3 10:32:43 PDT 2003
//   I made some changes that allow for better animation behavior.
//
//   Eric Brugger, Fri Apr 18 12:46:00 PDT 2003
//   I replaced auto center with maintain view.
//
//   Brad Whitlock, Fri May 16 11:42:49 PDT 2003
//   I made it use a helper method to open the database.
//
// ****************************************************************************

void
ViewerSubject::ReOpenDatabase()
{
    //
    // Get the rpc arguments.
    //
    std::string hostDatabase(viewerRPC.GetDatabase());
    bool forceClose = (viewerRPC.GetWindowLayout() == 1);

    //
    // Clear default SIL restrictions
    //
    std::string host, db;
    ViewerPlotList::SplitHostDatabase(hostDatabase, host, db);
    ViewerPlotList::ClearDefaultSILRestrictions(host, db);

    //
    // Clear out any previous information about the file.
    //
    ViewerFileServer *fileServer = ViewerFileServer::Instance();
    fileServer->ClearFile(hostDatabase);
    if(forceClose)
        fileServer->CloseFile(host);

    //
    // First open the database.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerAnimation *animation = wM->GetActiveAnimation();
    ViewerPlotList *plotList = animation->GetPlotList();
    OpenDatabaseHelper(hostDatabase, animation->GetFrameIndex(), false);

    //
    // Tell the compute engine to clear any cached information about the
    // database so it forces the networks to re-execute.
    //
    ViewerEngineManager::Instance()->ClearCache(host.c_str(), db.c_str());
 
    //
    // Now perform the database replacement.
    //
    plotList->ReplaceDatabase(host, db, true);
    wM->UpdateGlobalAtts();

    //
    // Recenter the active window's view and redraw.
    //
    if(wM->GetActiveWindow() && !wM->GetActiveWindow()->GetMaintainViewMode())
        wM->RecenterView();
}

// ****************************************************************************
// Method: ViewerSubject::ReplaceDatabase
//
// Purpose: 
//   Replaces the database used in the plots in the current animation.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 6 16:27:52 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Dec 4 16:25:43 PST 2002
//   I changed it so view recentering does not happen unless the it is
//   enabled in the window.
//
//   Brad Whitlock, Tue Feb 11 12:01:37 PDT 2003
//   I made it use STL strings.
//
//   Brad Whitlock, Thu Apr 3 10:32:43 PDT 2003
//   I made some changes that allow for better animation behavior.
//
//   Eric Brugger, Fri Apr 18 12:46:00 PDT 2003
//   I replaced auto center with maintain view.
//
//   Brad Whitlock, Thu May 15 13:30:27 PST 2003
//   I made it use OpenDatabaseHelper.
//
// ****************************************************************************

void
ViewerSubject::ReplaceDatabase()
{
    //
    // First open the database.
    //
    OpenDatabaseHelper(viewerRPC.GetDatabase(), 0, true);

    //
    // Now perform the database replacement.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveAnimation()->GetPlotList();
    plotList->ReplaceDatabase(plotList->GetHostName(),
                              plotList->GetDatabaseName());
    wM->UpdateGlobalAtts();

    //
    // Recenter the active window's view and redraw.
    //
    if(wM->GetActiveWindow() && !wM->GetActiveWindow()->GetMaintainViewMode())
        wM->RecenterView();
}

// ****************************************************************************
// Method: ViewerSubject::OverlayDatabase
//
// Purpose: 
//   Creates new plots using the new database and adds them to the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 6 16:27:52 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 11 12:02:33 PDT 2003
//   I made it use STL strings.
//
//   Brad Whitlock, Thu May 15 13:30:57 PST 2003
//   I made it use OpenDatabaseHelper.
//
// ****************************************************************************

void
ViewerSubject::OverlayDatabase()
{
    //
    // First open the database.
    //
    OpenDatabaseHelper(viewerRPC.GetDatabase(), 0, true);

    //
    // Now perform the database replacement.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveAnimation()->GetPlotList();
    plotList->OverlayDatabase(plotList->GetHostName(),
                              plotList->GetDatabaseName());

    //
    // Recenter the active window's view and redraw.
    //
    wM->RecenterView();
}

// ****************************************************************************
//  Method: ViewerSubject::OpenComputeEngine
//
//  Purpose:
//    Execute the OpenComputeEngine RPC.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Apr 30 12:31:21 PDT 2001
//    Added the implementation.
//
//    Jeremy Meredith, Thu Dec 19 12:13:58 PST 2002
//    Added code to skip the engine profile chooser if options were specified.
//
// ****************************************************************************

void
ViewerSubject::OpenComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const char *hostName   = viewerRPC.GetProgramHost().c_str();
    const stringVector &options = viewerRPC.GetProgramOptions();

    //
    // Perform the rpc.
    //
    ViewerEngineManager::Instance()->CreateEngine(hostName, options,
                                                  options.size() > 0,
                                                  numEngineRestarts);
}

// ****************************************************************************
// Method: ViewerSubject::CloseComputeEngine
//
// Purpose: 
//   Execute the CloseComputeEngine RPC.
//
// Programmer: Brad Whitlock
// Creation:   Mon Apr 30 13:08:14 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CloseComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const char *hostName = viewerRPC.GetProgramHost().c_str();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->CloseEngine(hostName);
}

// ****************************************************************************
// Method: ViewerSubject::OpenMDServer
//
// Purpose: 
//   Execute the OpenMDServer RPC.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jan 13 08:58:17 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::OpenMDServer()
{
     //
    // Get the rpc arguments.
    //
    const std::string &hostName = viewerRPC.GetProgramHost();
    const stringVector &options = viewerRPC.GetProgramOptions();
    ViewerFileServer::Instance()->NoFaultStartServer(hostName, options);
}

// ****************************************************************************
// Method: ViewerSubject::ClearCache
//
// Purpose: 
//   Execute the ClearCache RPC.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 14:20:23 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ClearCache()
{
    //
    // Get the rpc arguments.
    //
    const char *hostName = viewerRPC.GetProgramHost().c_str();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->ClearCache(hostName);
}

// ****************************************************************************
//  Method: ViewerSubject::SetDefaultPlotOptions
//
//  Purpose:
//    Execute the SetDefaultPlotOptions RPC.
//
//  Programmer: Eric Brugger
//  Creation:   August 17, 2000
//
//  Modifications:
//
// ****************************************************************************

void 
ViewerSubject::SetDefaultPlotOptions()
{
    //
    // Get the rpc arguments.
    //
    int       type = viewerRPC.GetPlotType();

    //
    // Perform the rpc.
    //
    plotFactory->SetDefaultAttsFromClient(type);
}

// ****************************************************************************
//  Method: ViewerSubject::ResetPlotOptions
//
//  Purpose:
//    Execute the ResetPlotOptions RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 14 17:33:08 PST 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 18 16:58:46 PST 2002
//    I added code to set the attributes back into the selected plots.
//
// ****************************************************************************

void
ViewerSubject::ResetPlotOptions()
{
    //
    // Get the rpc arguments.
    //
    int plot = viewerRPC.GetPlotType();

    //
    // Update the client so it has the default attributes.
    //
    plotFactory->SetClientAttsFromDefault(plot);

    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->GetActiveAnimation()->GetPlotList()->SetPlotAtts(plot);
}

// ****************************************************************************
//  Method: ViewerSubject::SetDefaultOperatorOptions
//
//  Purpose:
//    Execute the SetDefaultOperatorOptions RPC.
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSubject::SetDefaultOperatorOptions()
{
    //
    // Get the rpc arguments.
    //
    int       type = viewerRPC.GetOperatorType();

    //
    // Perform the rpc.
    //
    operatorFactory->SetDefaultAttsFromClient(type);
}

// ****************************************************************************
//  Method: ViewerSubject::ResetOperatorOptions
//
//  Purpose:
//    Execute the ResetOperatorOptions RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 14 17:33:08 PST 2001
//
//  Modifications:
//    Brad Whitlock, Thu Jul 18 16:55:02 PST 2002
//    I added code to set the default attributes back into the selected
//    plots that have the designated operator.
//
// ****************************************************************************

void
ViewerSubject::ResetOperatorOptions()
{
    //
    // Get the rpc arguments.
    //
    int oper = viewerRPC.GetOperatorType();

    //
    // Update the client so it has the default attributes.
    //
    operatorFactory->SetClientAttsFromDefault(oper);

    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    bool apply = wM->GetClientAtts()->GetApplyOperator();
    wM->GetActiveAnimation()->GetPlotList()->SetPlotOperatorAtts(oper, apply);
}

// ****************************************************************************
// Method: ViewerSubject::SetView2D
//
// Purpose: 
//   Tells the viewer window manager to apply the 2d view attributes to the
//   active window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 20 14:52:29 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetView2D()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetView2DFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::SetView3D
//
// Purpose: 
//   Tells the viewer window manager to apply the 3d view attributes to the
//   active window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 20 14:52:29 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetView3D()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetView3DFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ClearViewKeyframes
//
// Purpose: 
//   Tells the viewer window manager to clear the view keyframes for the
//   active window.
//
// Programmer: Eric Brugger
// Creation:   January 3, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ClearViewKeyframes()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->ClearViewKeyframes();
}

// ****************************************************************************
// Method: ViewerSubject::DeleteViewKeyframe
//
// Purpose: 
//   Tells the viewer window manager to delete a view keyframe from the
//   active window.
//
// Programmer: Eric Brugger
// Creation:   January 3, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DeleteViewKeyframe()
{
    //
    // Get the rpc arguments.
    //
    int frame = viewerRPC.GetFrame();
 
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->DeleteViewKeyframe(frame);
}

// ****************************************************************************
// Method: ViewerSubject::MoveViewKeyframe
//
// Purpose: 
//   Tells the viewer window manager to move a view keyframe in the
//   active window.
//
// Programmer: Eric Brugger
// Creation:   January 28, 2003
//
// ****************************************************************************

void
ViewerSubject::MoveViewKeyframe()
{
    //
    // Get the rpc arguments.
    //
    int oldFrame = viewerRPC.GetIntArg1();
    int newFrame = viewerRPC.GetIntArg2();
 
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->MoveViewKeyframe(oldFrame, newFrame);
}

// ****************************************************************************
// Method: ViewerSubject::SetViewKeyframe
//
// Purpose: 
//   Tells the viewer window manager to set a view keyframe for the
//   active window.
//
// Programmer: Eric Brugger
// Creation:   January 3, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetViewKeyframe()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetViewKeyframe();
}

// ****************************************************************************
// Method: ViewerSubject::UpdateColorTable
//
// Purpose: 
//   Updates all plots that use the color table specified in the RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 13 17:38:55 PST 2001
//
// Notes:      It is worth noting that this RPC may be called with the name
//             of a color table that does not exist. When that happens, it is
//             usually assumed to be the case that the default color table
//             as been deleted and has been set to a new value. This routine
//             is then called to update all plots which use the default color
//             table.
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::UpdateColorTable()
{
    //
    // Get the rpc arguments.
    //
    const char *ctName = viewerRPC.GetColorTableName().c_str();

    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->UpdateColorTable(ctName);
}

// ****************************************************************************
//  Method: ViewerSubject::WriteConfigFile
//
//  Purpose:
//    Execute the WriteConfigFile RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 11:07:44 PDT 2000
//
//  Modifications:
//    Brad Whitlock, Fri Nov 2 11:26:32 PDT 2001
//    Added code to tell the ViewerWindowManager to gather the size, location
//    of its windows so it can be saved.
//
//    Brad Whitlock, Fri May 16 15:47:35 PST 2003
//    I made it capable of saving to the config file that was specified
//    with the -config option when VisIt started. If no config file was ever
//    specified, we save to the default config file name.
//
// ****************************************************************************

void
ViewerSubject::WriteConfigFile()
{
    char *defaultConfigFile = configMgr->GetDefaultConfigFile(configFileName);

    //
    // Tell the ViewerWindowManager to get the current location, size of the
    // viewer windows so that information can be saved.
    //
    ViewerWindowManager::Instance()->UpdateWindowAtts();

    //
    // Tell the configuration manager to write the file.
    //
    configMgr->WriteConfigFile(defaultConfigFile);

    //
    // Delete the memory for the config file name.
    //
    delete [] defaultConfigFile;
}

// ****************************************************************************
//  Method: ViewerSubject::SetAnimationAttributes
//
//  Purpose:
//    Execute the SetAnimationonAttributes RPC.
//
//  Programmer: Eric Brugger
//  Creation:   November 19, 2001 
//
// ****************************************************************************

void
ViewerSubject::SetAnimationAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetAnimationAttsFromClient();
}

// ****************************************************************************
//  Method: ViewerSubject::SetAnnotationAttributes
//
//  Purpose:
//    Execute the SetAnnotationAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 18, 2001 
//
//  Modifications:
//    Brad Whitlock, Thu Aug 30 09:19:25 PDT 2001
//    Renamed the method to SetAnnotationAttributes and modified the code
//    to account for the fact that the annotation attributes are no longer
//    in this class.
//
// ****************************************************************************

void
ViewerSubject::SetAnnotationAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetAnnotationAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::SetDefaultAnnotationAttributes
//
// Purpose: 
//   Sets the default annotation atts from the client annotation atts.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:21:17 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultAnnotationAttributes()
{
    ViewerWindowManager::SetDefaultAnnotationAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetAnnotationAttributes
//
// Purpose: 
//   Sets the default annotation attributes into the annotation attributes
//   for the active window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 30 09:37:02 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetAnnotationAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetAnnotationAttsFromDefault();
}

// ****************************************************************************
//  Method: ViewerSubject::SetKeyframeAttributes
//
//  Purpose:
//    Execute the SetKeyframeAttributes RPC.
//
//  Programmer: Eric Brugger
//  Creation:   November 25, 2002
//
//  Modifications:
//
// ****************************************************************************
 
void
ViewerSubject::SetKeyframeAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetKeyframeAttsFromClient();
}

// ****************************************************************************
//  Method: ViewerSubject::SetMaterialAttributes
//
//  Purpose:
//    Execute the SetMaterialAttributes RPC.
//
//  Programmer: Jeremy Meredith 
//  Creation:   October 24, 2002 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSubject::SetMaterialAttributes()
{
    // Do nothing; there is only a global copy, and nothing
    // is regenerated automatically just yet
}

// ****************************************************************************
// Method: ViewerSubject::SetDefaultMaterialAttributes
//
// Purpose: 
//   Sets the default material atts from the client material atts.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultMaterialAttributes()
{
    ViewerEngineManager *eM=ViewerEngineManager::Instance();
    eM->SetDefaultMaterialAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetMaterialAttributes
//
// Purpose: 
//   Sets the default material attributes into the material attributes
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetMaterialAttributes()
{
    ViewerEngineManager *eM=ViewerEngineManager::Instance();
    eM->SetClientMaterialAttsFromDefault();
}

// ****************************************************************************
// Method: ViewerSubject::SetAppearanceAttributes
//
// Purpose: 
//   Makes the viewer customize its appearance using the new appearance
//   settings.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 5 10:27:57 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetAppearanceAttributes()
{
    CustomizeAppearance();
}

// ****************************************************************************
//  Method: ViewerSubject::SetLightList
//
//  Purpose:
//    Execute the SetLightList RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Sep 14 14:10:52 PST 2001
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSubject::SetLightList()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetLightListFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::SetDefaultLightList
//
// Purpose: 
//   Sets the default light list from the client light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 14:10:19 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultLightList()
{
    ViewerWindowManager::SetDefaultLightListFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetLightList
//
// Purpose: 
//   Sets the default lightlist into the light list for the active window.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 14 14:08:56 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetLightList()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetLightListFromDefault();
}

// ****************************************************************************
// Method: ViewerSubject::SetRenderingAttributes
//
// Purpose: 
//   Sets the rendering attributes for the active window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 19 13:34:09 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetRenderingAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetRenderingAttributes();
}

// ****************************************************************************
// Method: ViewerSubject::SetWindowArea
//
// Purpose: 
//   Tells the viewer where to put its vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 16:07:04 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Feb 5 09:24:03 PDT 2002
//   Made it respect the -small flag.
//
// ****************************************************************************

void
ViewerSubject::SetWindowArea()
{
    const char *area = viewerRPC.GetWindowArea().c_str();
    
    //
    // Recalculate the layouts and reposition the windows.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    if(smallWindow)
    {
        int x, y, w, h;
        if(sscanf(area, "%dx%d+%d+%d", &w, &h, &x, &y) == 4)
        {
            char tmp[30];
            w /= 2;
            h /= 2;
            SNPRINTF(tmp, 30, "%dx%d+%d+%d", w, h, x, y);
            wM->SetGeometry(tmp);
        }
    }
    else
        wM->SetGeometry(area);

    wM->SetWindowLayout(wM->GetWindowLayout());
}

// ****************************************************************************
//  Method: ViewerSubject::ConnectToMetaDataServer
//
//  Purpose: 
//    Tells an mdserver to connect to another program. If the desired mdserver
//    does not exist, it is launched and told to connect to the program.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Nov 21 11:52:23 PDT 2000
//
//  Modifications:
//   Brad Whitlock, Thu Dec 26 16:24:29 PST 2002
//   I added support for security checking.
//
//   Brad Whitlock, Mon May 5 14:31:23 PST 2003
//   I changed how the arguments are passed.
//
// ****************************************************************************

void
ViewerSubject::ConnectToMetaDataServer()
{
    //
    // Tell the viewer's fileserver to have its mdserver running on 
    // the specified host to connect to another process.
    //
    ViewerFileServer::Instance()->ConnectServer(
        viewerRPC.GetProgramHost(),
        viewerRPC.GetProgramOptions()); 
}

// ****************************************************************************
//  Function: ProcessExpressions
//
//  Purpose:
//    RPC function for applying the expressions to the existing plots.
//
//  Programmer: Sean Ahern
//  Creation:   Wed Sep 26 16:35:26 PDT 2001
//
//  Modifications:
//      Commented out the annoying error message.
//
// ****************************************************************************
void
ViewerSubject::ProcessExpressions()
{
    //fprintf(stderr, "ViewerSubject::ProcessExpressions(): NOT YET IMPLEMENTED\n");
}

// ****************************************************************************
//  Method: ViewerSubject::ClearWindow
//
//  Purpose:
//    This is a Qt slot function that executes the ClearWindow RPC.
//
//  Programmer: Eric Brugger
//  Creation:   August 22, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Nov 7 12:22:37 PDT 2000
//    Modified to use ViewerWindowManager::ClearWindow.
//
// ****************************************************************************

void
ViewerSubject::ClearWindow(int windowIndex)
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager::Instance()->ClearWindow(windowIndex);
}

// ****************************************************************************
//  Method: ViewerSubject::ToggleCameraViewMode
//
//  Purpose: 
//    This is a Qt slot function that toggles whether or not camera view mode
//    is enabled for the specified window.
//
//  Arguments:
//    windowIndex  The index of the window whose camera view mode will be
//                 toggled.
//
//  Programmer: Eric Brugger
//  Creation:   January 3, 2003
//
//  Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ToggleCameraViewMode(int windowIndex)
{
    //
    // Toggle the window's camera view mode.
    //
    ViewerWindowManager::Instance()->ToggleCameraViewMode(windowIndex);
}

// ****************************************************************************
// Method: ViewerSubject::ToggleLockTime
//
// Purpose: 
//   Locks the time for the specified window.
//
// Arguments:
//   windowIndex : The index of the window for which to set the locktime flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:52:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ToggleLockTime(int windowIndex)
{
     ViewerWindowManager::Instance()->ToggleLockTime(windowIndex);
}

// ****************************************************************************
// Method: ViewerSubject::ToggleLockTools.
//
// Purpose: 
//   Locks the tools for the specified window.
//
// Arguments:
//   windowIndex : The index of the window for which to set the locktools flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 11:52:00 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ToggleLockTools(int windowIndex)
{
     ViewerWindowManager::Instance()->ToggleLockTools(windowIndex);
}

// ****************************************************************************
//  Method: ViewerSubject::ToggleMaintainViewMode
//
//  Purpose: 
//    A Qt slot function that toggles the maintain view mode for the specified
//    window.
//
//  Arguments:
//    windowIndex  The index of the window whose maintain view mode will be
//                 toggled.
//
//  Programmer: Eric Brugger
//  Creation:   April 18, 2003
//
// ****************************************************************************

void
ViewerSubject::ToggleMaintainViewMode(int windowIndex)
{
    ViewerWindowManager::Instance()->ToggleMaintainViewMode(windowIndex);
}

// ****************************************************************************
//  Method: ViewerSubject::SetViewExtentsType
//
//  Purpose:
//    Sets the type of extents to use for setting the view.
//
//  Programmer:  Eric Brugger
//  Creation:    February 23, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Sep 16 12:51:51 PDT 2002
//    I removed the arguments and made the view extents type come from
//    the viewer rpc.
//
// ****************************************************************************

void
ViewerSubject::SetViewExtentsType()
{
     avtExtentType viewType = (avtExtentType)viewerRPC.GetWindowLayout();
     ViewerWindowManager::Instance()->SetViewExtentsType(viewType);
}

// ****************************************************************************
// Method: ViewerSubject::CopyAnnotationsToWindow
//
// Purpose: 
//   Qt slot function that copies the annotation attributes from one window
//   to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyAnnotationsToWindow(int from, int to)
{
    ViewerWindowManager::Instance()->CopyAnnotationsToWindow(from-1, to-1);
}

// ****************************************************************************
// Method: ViewerSubject::CopyLightingToWindow
//
// Purpose: 
//   Qt slot function that copies the lighting attributes from one window
//   to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyLightingToWindow(int from, int to)
{
    ViewerWindowManager::Instance()->CopyLightingToWindow(from-1, to-1);
}

// ****************************************************************************
// Method: ViewerSubject::CopyViewToWindow
//
// Purpose: 
//   Qt slot function that copies the view attributes from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 27 17:01:32 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyViewToWindow(int from, int to)
{
    ViewerWindowManager::Instance()->CopyViewToWindow(from-1, to-1);
}

// ****************************************************************************
// Method: ViewerSubject::CopyPlotsToWindow
//
// Purpose: 
//   Qt slot function that copies the plots from one window to another.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:30:56 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CopyPlotsToWindow(int from, int to)
{
    ViewerWindowManager::Instance()->CopyAnimationToWindow(from-1, to-1);
}

// ****************************************************************************
// Method: ViewerSubject::DatabaseQuery
//
// Purpose: 
//   Performs a database query.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 14:23:13 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 30 14:38:33 PDT 2002
//   Added call to ViewerQueryManager's DatabaseQuery method. 
//   
// ****************************************************************************

void
ViewerSubject::DatabaseQuery()
{
    // Send the client a status message.
    QString msg;
    msg.sprintf("Performing %s query...", viewerRPC.GetQueryName().c_str());
    Status(msg.latin1());

    ViewerWindow *vw = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerQueryManager *qm = ViewerQueryManager::Instance();
    qm->DatabaseQuery(vw, viewerRPC.GetQueryName(), viewerRPC.GetQueryVariables());

    // Clear the status
    ClearStatus();
}

// ****************************************************************************
// Method: ViewerSubject::PointQuery
//
// Purpose: 
//   Performs a point query.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 14:23:13 PST 2002
//
// Modifications:
//   Kathleen Bonnell, Wed May 14 17:09:14 PDT 2003 
//   Removed debug code, and special case handling.  Made ViewerQueryManager
//   handle the query instead of ViewerWindowManager. 
//
// ****************************************************************************

void
ViewerSubject::PointQuery()
{
    // Send the client a status message.
    QString msg;
    msg.sprintf("Performing %s query...", viewerRPC.GetQueryName().c_str());
    Status(msg.latin1());

    ViewerQueryManager *qm = ViewerQueryManager::Instance();
    qm->PointQuery(viewerRPC.GetQueryName(), viewerRPC.GetQueryPoint1(),
                   viewerRPC.GetQueryVariables());

    // Clear the status
    ClearStatus();
}

// ****************************************************************************
// Method: ViewerSubject::LineQuery
//
// Purpose: 
//   Performs a line query.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 6 14:23:13 PST 2002
//
// Modifications:
//   ViewerQueryManager now handles the line queries.
//   
// ****************************************************************************

void
ViewerSubject::LineQuery()
{
    // Send the client a status message.
    QString msg;
    msg.sprintf("Performing %s query...", viewerRPC.GetQueryName().c_str());
    Status(msg.latin1());

    ViewerQueryManager::Instance()->LineQuery( viewerRPC.GetQueryName().c_str(),
              viewerRPC.GetQueryPoint1(), viewerRPC.GetQueryPoint2(),
              viewerRPC.GetQueryVariables());

    // Clear the status
    ClearStatus();
}

// ****************************************************************************
//  Method: ViewerSubject::ReadFromParentAndCheckForInterruption
//
//  Purpose:
//    Just like ReadFromParentAndProcess, but it only buffers the read data
//    and looks for interruption messages.
//
//  Arguments:
//    fd        The file descriptor to use for reading.
//
//  Programmer: Jeremy Meredith
//  Creation:   July  3, 2001
//
//  Modifications:
//    Brad Whitlock, Fri Oct 19 12:26:09 PDT 2001
//    Added code to process some input from the client if there is any and
//    we're playing an animation.
//
//    Eric Brugger, Mon Oct 29 09:47:30 PST 2001
//    I removed the code to process the client input and moved it to the
//    routine that is called by the timer.
//
//    Brad Whitlock, Fri Jan 4 17:28:51 PST 2002
//    I changed the code so the check for interruption can be disabled.
//
//    Brad Whitlock, Tue May 14 12:42:24 PDT 2002
//    I made interruption stop any animations that may be playing.
//
//    Brad Whitlock, Wed Feb 5 10:57:03 PDT 2003
//    I changed the call that stops the animations.
//
// ****************************************************************************

bool
ViewerSubject::ReadFromParentAndCheckForInterruption()
{
    bool retval = false;

    if(interruptionEnabled)
    {
        // See if the connection needs to be read.
        if(xfer.GetInputConnection()->NeedsRead())
        {
            xfer.GetInputConnection()->Fill();
        }

        //
        // Process the input if there is any.
        //
        retval = xfer.ReadPendingMessages();

        //
        // If we interrupted then stop animations.
        //
        if(retval)
            ViewerWindowManager::Instance()->Stop();
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerSubject::ProcessFromParent
//
//  Purpose:
//    This routine processes any input from the parent process.
//
//  Programmer: Eric Brugger
//  Creation:   October 29, 2001
//
// ****************************************************************************

void
ViewerSubject::ProcessFromParent()
{
    xfer.Process();
}

// ****************************************************************************
//  Function: ReadFromParentAndProcess
//
//  Purpose:
//    This is a Qt slot function that gets called when the socket
//    connected to the parent has data to read.
//
//  Arguments:
//    fd        The file descriptor to use for reading.
//
//  Programmer: Eric Brugger
//  Creation:   August 16, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Oct 27 14:19:18 PST 2000
//    I changed this function into a Qt slot function and moved the guts
//    of the function from ProcessInput to here.
//
//    Brad Whitlock, Fri May 25 16:40:51 PST 2001
//    I added code to throw an exception if we detect that the parent is dead.
//
//    Jeremy Meredith, Tue Jul  3 10:59:36 PDT 2001
//    Renamed to ReadFromParentAndProcess.
//
//    Brad Whitlock, Fri Mar 22 14:22:27 PST 2002
//    Made the connection read its input.
//
//    Jeremy Meredith, Thu Dec 19 12:16:08 PST 2002
//    Added check to make sure socket signals were not blocked before
//    proceeding.  Note -- placing it after the read caused them to be
//    ignored later in some cases; we just want to postpone them.
//
//    Brad Whitlock, Mon Apr 7 17:26:02 PST 2003
//    Added code to catch lost connection exceptions so we don't crash
//    on Linux.
//
// ****************************************************************************

void
ViewerSubject::ReadFromParentAndProcess(int)
{
    TRY
    {
        if (blockSocketSignals)
            return;

        int amountRead = xfer.GetInputConnection()->Fill();

        //
        // Try and process the input.
        //
        if (amountRead > 0)
            xfer.Process();
    }
    CATCH(LostConnectionException)
    {
        cerr << "The component that launched VisIt's viewer has terminated "
                "abnormally." << endl;
        mainApp->quit();
    }
    ENDTRY
}

// ****************************************************************************
//  Method:  ViewerSubject::BlockSocketSignals
//
//  Purpose:
//    Blocks any signals from the sockets temporarily
//
//  Arguments:
//    b          true to block signals, false to unblock them
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 19, 2002
//
// ****************************************************************************

void
ViewerSubject::BlockSocketSignals(bool b)
{
    blockSocketSignals = b;
}

// ****************************************************************************
//  Method: ViewerSubject::ProcessRendererMessage
//
//  Purpose: 
//    This is a Qt slot function that is called when there is input
//    to be read by the rendering thread.
//
//  Arguments:
//    fd        The file descriptor to use for reading.
//
//  Programmer: Eric Brugger
//  Creation:   August 16, 2000
//
//  Modifications:
//    Eric Brugger, Wed Feb 21 08:25:56 PST 2001
//    Replace the use of VisWindow with ViewerWindow.
//
//    Brad Whitlock, Fri Jun 15 13:28:15 PST 2001
//    Added a handler for redrawWindow.
//
//    Brad Whitlock, Wed Aug 22 11:22:57 PDT 2001
//    Added a handler for deleteWindow.
//
//    Brad Whitlock, Tue Apr 16 12:44:09 PDT 2002
//    Made portions compile conditionally and removed the fd argument.
//
//    Brad Whitlock, Fri Jan 31 14:50:42 PST 2003
//    I added code to update actions when a window is deleted because a
//    delete can happen using the window decorations which is outside our
//    normal control paths.
//
// ****************************************************************************

void
ViewerSubject::ProcessRendererMessage()
{
    char msg[512];

#ifdef VIEWER_MT
    //
    // Read from the message pipe.
    //
    int n = read(messagePipe[0], msg, 512);
    if (n < 0 && n >= 512)
    {
        cerr << "Error getting the message sent to the master.\n";
        return;
    }
    msg[n] = '\0';
    messageBuffer->AddString(msg);
#endif

    //
    // Add the string to the message buffer and then process messages
    // from it until there aren't any left.
    //
    while (messageBuffer->ReadMessage(msg) > 0)
    {
        //
        // Parse and process the message.
        //
        if (strncmp(msg, "updateWindow", 12) == 0)
        {
            ViewerWindow *window=0;

            int  offset = 15;  // = strlen("updateWindow 0x");
            sscanf (&msg[offset], "%p", &window);

            window->EnableUpdates();
        }
        else if (strncmp(msg, "redrawWindow", 12) == 0)
        {
            ViewerWindow *window=0;

            int  offset = 15;  // = strlen("redrawWindow 0x");
            sscanf (&msg[offset], "%p", &window);

            window->RedrawWindow();
        }
        else if (strncmp(msg, "deleteWindow", 12) == 0)
        {
            ViewerWindow *window=0;

            int  offset = 15;  // = strlen("deleteWindow 0x");
            sscanf (&msg[offset], "%p", &window);

            // Tell the viewer window manager to delete the window.
            ViewerWindowManager::Instance()->DeleteWindow(window);
            ViewerWindowManager::Instance()->UpdateActions();
        }
    }
}

// ****************************************************************************
// Method: ViewerSubject::StartLaunchProgress
//
// Purpose: 
//   This method is called when the launch progress callback is called the
//   first time. In this case, we disable command processing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 15:50:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::StartLaunchProgress()
{
    checkParent->setEnabled(false);
}

// ****************************************************************************
// Method: ViewerSubject::EndLaunchProgress
//
// Purpose: 
//   This method is called when the launch progress callback is called the
//   first time. In this case, we enable command processing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 15:50:30 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::EndLaunchProgress()
{
    checkParent->setEnabled(true);
}

// ****************************************************************************
// Method: ViewerSubject::LaunchProgressCB
//
// Purpose: 
//   This is the callback function for launch progress. It is called several
//   times as a process is being launched. Its job is to act as a surrogate
//   event loop and show the progress dialog.
//
// Arguments:
//   data : An array that contains a pointer to the ViewerSubject and the
//          progress dialog.
//   stage : The reason why the callback is being called. 0=start, 1=middle,
//           2=end.
//
// Returns:    true if no cancel has taken place. false if the user has
//             cancelled the launch.
//
// Note:       This routine is only called when HAVE_THREADS is defined.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 27 15:51:58 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed May 7 10:26:19 PDT 2003
//   I changed the code a little so the dialog does not necessarily have to
//   hide itself when the process is launched. This lets the window be active
//   for iterated process launches.
//
// ****************************************************************************

bool
ViewerSubject::LaunchProgressCB(void *d, int stage)
{
    bool retval = true;
    void **data = (void **)d;
    ViewerSubject *This = (ViewerSubject *)data[0];
    ViewerConnectionProgressDialog *dialog = 
        (ViewerConnectionProgressDialog *)data[1];
    // Only show the dialog if windows have been shown.
    bool windowsShowing = !ViewerWindowManager::Instance()->GetWindowsHidden();

    if(stage == 0)
    {
        This->StartLaunchProgress();
        if(windowsShowing)
        {
            dialog->show();
            retval = !dialog->getCancelled();
        }
    }
    else if(stage == 1)
    {
        if(windowsShowing)
        {
#if QT_VERSION >= 300
            if(qApp->hasPendingEvents())
#endif
                qApp->processOneEvent();
            retval = !dialog->getCancelled();
        }
    }
    else if(stage == 2)
    {
        This->EndLaunchProgress();
        if(windowsShowing) 
        {
            if(!dialog->getIgnoreHide())
                dialog->hide();
            retval = !dialog->getCancelled();
        }
    }

    return retval;
}

// ****************************************************************************
//  Method: ViewerSubject::HandleViewerRPC
//
//  Purpose: 
//    This is a Qt slot function that handles RPC's for the
//    ViewerSubject class.
//
//  Arguments:
//    rpc       A pointer to the ViewerRPC that should be executed.
//
//  Programmer: Brad Whitlock
//  Creation:   Fri Oct 27 14:58:06 PST 2000
//
//  Modifications:
//    Eric Brugger, Tue Jan 28 14:20:30 PST 2003
//    I added MovePlotKeyframe, MovePlotDatabaseKeyframe and MoveViewKeyframe.
//
//    Brad Whitlock, Wed Jan 29 13:53:54 PST 2003
//    I added support for actions.
//
//    Kathleen Bonnell, Wed Feb 19 11:56:32 PST 2003
//    Added SetGlobalLineoutAttributes.
//
//    Brad Whitlock, Mon Mar 17 09:36:03 PDT 2003
//    I removed some RPC's related to plots and operators since they are now
//    handled by actions.
//
//   Eric Brugger, Fri Apr 18 12:46:00 PDT 2003
//   I replaced auto center view with maintain view.
//
// ****************************************************************************

void
ViewerSubject::HandleViewerRPC()
{
    //
    // Get a pointer to the active window's action manager.
    //
    bool actionHandled = false;
    ViewerActionManager *actionMgr = ViewerWindowManager::Instance()->
        GetActiveWindow()->GetActionManager();

    //
    // Handle the RPC. Note that these should be replaced with actions.
    //
    switch(viewerRPC.GetRPCType())
    {
    case ViewerRPC::CloseRPC:
        Close();
        break;
    case ViewerRPC::ClearWindowRPC:
        ClearWindow();
        break;
    case ViewerRPC::ClearAllWindowsRPC:
        ClearAllWindows();
        break;
    case ViewerRPC::OpenDatabaseRPC:
        OpenDatabase();
        break;
    case ViewerRPC::ReOpenDatabaseRPC:
        ReOpenDatabase();
        break;
    case ViewerRPC::ReplaceDatabaseRPC:
        ReplaceDatabase();
        break;
    case ViewerRPC::OverlayDatabaseRPC:
        OverlayDatabase();
        break;
    case ViewerRPC::OpenComputeEngineRPC:
        OpenComputeEngine();
        break;
    case ViewerRPC::CloseComputeEngineRPC:
        CloseComputeEngine();
        break;
    case ViewerRPC::SaveWindowRPC:
        SaveWindow();
        break;
    case ViewerRPC::SetDefaultPlotOptionsRPC:
        SetDefaultPlotOptions();
        break;
    case ViewerRPC::SetDefaultOperatorOptionsRPC:
        SetDefaultOperatorOptions();
        break;
    case ViewerRPC::WriteConfigFileRPC:
        WriteConfigFile();
        break;
    case ViewerRPC::ConnectToMetaDataServerRPC:
        ConnectToMetaDataServer();
        break;
    case ViewerRPC::IconifyAllWindowsRPC:
        IconifyAllWindows();
        break;
    case ViewerRPC::DeIconifyAllWindowsRPC:
        DeIconifyAllWindows();
        break;
    case ViewerRPC::ShowAllWindowsRPC:
        ShowAllWindows();
        break;
    case ViewerRPC::HideAllWindowsRPC:
        HideAllWindows();
        break;
    case ViewerRPC::UpdateColorTableRPC:
        UpdateColorTable();
        break;
    case ViewerRPC::SetAnnotationAttributesRPC:
        SetAnnotationAttributes();
        break;
    case ViewerRPC::SetDefaultAnnotationAttributesRPC:
        SetDefaultAnnotationAttributes();
        break;
    case ViewerRPC::ResetAnnotationAttributesRPC:
        ResetAnnotationAttributes();
        break;
    case ViewerRPC::SetKeyframeAttributesRPC:
        SetKeyframeAttributes();
        break;
    case ViewerRPC::SetView2DRPC:
        SetView2D();
        break;
    case ViewerRPC::SetView3DRPC:
        SetView3D();
        break;
    case ViewerRPC::ClearViewKeyframesRPC:
        ClearViewKeyframes();
        break;
    case ViewerRPC::DeleteViewKeyframeRPC:
        DeleteViewKeyframe();
        break;
    case ViewerRPC::MoveViewKeyframeRPC:
        MoveViewKeyframe();
        break;
    case ViewerRPC::SetViewKeyframeRPC:
        SetViewKeyframe();
        break;
    case ViewerRPC::ResetPlotOptionsRPC:
        ResetPlotOptions();
        break;
    case ViewerRPC::ResetOperatorOptionsRPC:
        ResetOperatorOptions();
        break;
    case ViewerRPC::SetAppearanceRPC:
        SetAppearanceAttributes();
        break;
    case ViewerRPC::ProcessExpressionsRPC:
        ProcessExpressions();
        break;
    case ViewerRPC::SetLightListRPC:
        SetLightList();
        break;
    case ViewerRPC::SetDefaultLightListRPC:
        SetDefaultLightList();
        break;
    case ViewerRPC::ResetLightListRPC:
        ResetLightList();
        break;
    case ViewerRPC::SetAnimationAttributesRPC:
        SetAnimationAttributes();
        break;
    case ViewerRPC::DisableRedrawRPC:
        DisableRedraw();
        break;
    case ViewerRPC::RedrawRPC:
        RedrawWindow();
        break;
    case ViewerRPC::SetWindowAreaRPC:
        SetWindowArea();
        break;
    case ViewerRPC::PrintWindowRPC:
        PrintWindow();
        break;
    case ViewerRPC::ToggleMaintainViewModeRPC:
        ToggleMaintainViewMode();
        break;
    case ViewerRPC::ClearPickPointsRPC:
        ClearPickPoints();
        break;
    case ViewerRPC::ToggleCameraViewModeRPC:
        ToggleCameraViewMode();
        break;
    case ViewerRPC::CopyViewToWindowRPC:
        CopyViewToWindow();
        break;
    case ViewerRPC::CopyLightingToWindowRPC:
        CopyLightingToWindow();
        break;
    case ViewerRPC::CopyAnnotationsToWindowRPC:
        CopyAnnotationsToWindow();
        break;
    case ViewerRPC::CopyPlotsToWindowRPC:
        CopyPlotsToWindow();
        break;
    case ViewerRPC::ClearCacheRPC:
        ClearCache();
        break;
    case ViewerRPC::SetViewExtentsTypeRPC:
        SetViewExtentsType();
        break;
    case ViewerRPC::ClearRefLinesRPC:
        ClearRefLines();
        break;
    case ViewerRPC::SetRenderingAttributesRPC:
        SetRenderingAttributes();
        break;
    case ViewerRPC::DatabaseQueryRPC:
        DatabaseQuery();
        break;
    case ViewerRPC::PointQueryRPC:
        PointQuery();
        break;
    case ViewerRPC::LineQueryRPC:
        LineQuery();
        break;
    case ViewerRPC::SetMaterialAttributesRPC:
        SetMaterialAttributes();
        break;
    case ViewerRPC::SetDefaultMaterialAttributesRPC:
        SetDefaultMaterialAttributes();
        break;
    case ViewerRPC::ResetMaterialAttributesRPC:
        ResetMaterialAttributes();
        break;
    case ViewerRPC::ToggleLockTimeRPC:
        ToggleLockTime();
        break;
    case ViewerRPC::ToggleLockToolsRPC:
        ToggleLockTools();
        break;
    case ViewerRPC::OpenMDServerRPC:
        OpenMDServer();
        break;
    case ViewerRPC::SetGlobalLineoutAttributesRPC:
        SetGlobalLineoutAttributes();
        break;
    case ViewerRPC::MaxRPC:
        break;
    default:
        // If an RPC is not handled in the above cases, handle it as
        // an action.
        actionMgr->HandleAction(viewerRPC);
        actionHandled = true;
    }

    //
    // We need to do this until all items in the switch statement are
    // removed and converted to actions.
    //
    if(!actionHandled)
        ViewerWindowManager::Instance()->UpdateActions();
}

// ****************************************************************************
// Method: ViewerSubject::SpecialOpcodeCallback
//
// Purpose: 
//   This is a static callback function that ViewerSubject's xfer object
//   calls when it encounters a special opcode.
//
// Arguments:
//   opcode : The special opcode.
//   data   : user-defined data.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 11:24:44 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void // static
ViewerSubject::SpecialOpcodeCallback(int opcode, void *data)
{
    ViewerSubject *This = (ViewerSubject *)data;
    if(This)
        This->ProcessSpecialOpcodes(opcode);
}

// ****************************************************************************
// Method: ViewerSubject::ProcessSpecialOpcodes
//
// Purpose: 
//   This method handles special opcodes for the viewer.
//
// Arguments:
//   opcode : The opcode that we want to process.
//
// Note: Special opcodes are instructions from the client that we should
//       process right away instead of waiting for when it is convenient.
//       These are mostly intended for when the engine is busy and we
//       check for interruption using ReadFromParentAndCheckForInterruption
//       while it works. The ReadFromParentAndCheckForInterruption receives
//       input from the client but does not process it unless it is an
//       interruption or a special opcode. All other operations are buffered
//       so they can be processed later.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 27 11:25:53 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Mar 12 10:35:08 PDT 2003
//   I added code to process the new iconifyOpcode.
//
// ****************************************************************************

void
ViewerSubject::ProcessSpecialOpcodes(int opcode)
{
    ViewerWindowManager *wMgr = ViewerWindowManager::Instance();

    if(opcode == animationStopOpcode)
        wMgr->Stop();
    else if(opcode == iconifyOpcode)
        wMgr->IconifyAllWindows();

    // Update actions.
    wMgr->UpdateActions();
}

// ****************************************************************************
// Method: ViewerSubject::HandleSync
//
// Purpose: 
//   This is a Qt slot function that is called when the syncAtts are modified
//   by the client. This function sends the syncAtts back to the client.
//
// Note:       This slot function sends the syncAtts back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 17 11:17:08 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::HandleSync()
{
    syncAtts->SetSyncTag(syncAtts->GetSyncTag());
    syncObserver->SetUpdate(false);
    syncAtts->Notify();
}

// ****************************************************************************
//  Method: ViewerSubject::ConnectWindow
//
//  Purpose: 
//    This is a Qt slot function that is called when a window is created. Its
//    job is to connect the new window's signals to ViewerSubject's slots.
//
//  Arguments:
//    win       A pointer to the newly created window.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 14:21:48 PST 2000
//
//  Modifications:
//    Brad Whitlock, Wed Jan 29 14:12:55 PST 2003
//    I removed the old coding.
//
// ****************************************************************************

void
ViewerSubject::ConnectWindow(ViewerWindow *win)
{
    win->GetActionManager()->EnableActions(ViewerWindowManager::Instance()->GetWindowAtts());
}

// ****************************************************************************
//  Method: ViewerSubject::DisonnectWindow
//
//  Purpose: 
//    This is a Qt slot function that is called when a window is deleted. Its
//    job is to disconnect the deleted window's signals from ViewerSubject's
//    slots.
//
//  Arguments:
//    win       A pointer to the window that's being deleted.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 14:17:58 PST 2000
//
//  Modifications:
//    Brad Whitlock, Wed Feb 5 11:06:05 PDT 2003
//    I removed the code to disconnect signals.
//
// ****************************************************************************

void
ViewerSubject::DisconnectWindow(ViewerWindow *win)
{
}


// ****************************************************************************
//  Method: ViewerSubject::SetGlobalLineoutAttributes
//
//  Purpose:
//    Execute the SetGlobalLineoutAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   February 19, 2003 
//
// ****************************************************************************

void
ViewerSubject::SetGlobalLineoutAttributes()
{
    ViewerQueryManager *qM=ViewerQueryManager::Instance();
    qM->SetGlobalLineoutAttsFromClient();
}
