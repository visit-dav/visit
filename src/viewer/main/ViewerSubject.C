// ************************************************************************* //
//                               ViewerSubject.C                             //
// ************************************************************************* //

#include <ctype.h>
#include <stdlib.h>
#include <snprintf.h>
#include <ViewerSubject.h>

#if QT_VERSION < 300
// If we're using less than Qt 3.0, include the style headers.
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#if QT_VERSION >= 230
#include <qsgistyle.h>
#endif
#endif

#include <qtimer.h>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <ColorTableAttributes.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <EngineKey.h>
#include <EngineList.h>
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
#include <QueryOverTimeAttributes.h>

#include <ViewerActionManager.h>
#include <ViewerConnectionProgressDialog.h>
#include <ParsingExprList.h>
#include <ViewerConfigManager.h>
#include <ViewerEngineManager.h>
#include <ViewerRemoteProcessChooser.h>
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

#include <algorithm>

static std::string getToken(std::string buff, bool reset);
static int getVectorTokens(std::string buff, std::vector<std::string> &tokens, int nodeType);


// Global variables.  This is a hack, they should be removed.
ViewerSubject  *viewerSubject=0;

using std::string;

// Static method to split a string using a given delimiter
static vector<string> SplitValues(const string &buff, char delim);


// ****************************************************************************
//  Method: ViewerSubject constructor
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Brad Whitlock, Tue Jun 17 14:39:12 PST 2003
//    I made sure that all objects and pointers are initialized and that
//    no heavy duty initialization takes place. I also removed old
//    modification comments.
//
//    Jeremy Meredith, Fri Sep 26 12:50:11 PDT 2003
//    Added defaultStereoToOn.
//
//    Brad Whitlock, Thu Dec 18 12:33:17 PDT 2003
//    I initialized processingFromParent.
//
//    Brad Whitlock, Fri Mar 12 12:12:59 PDT 2004
//    Added keepAliveTimer.
//
// ****************************************************************************

ViewerSubject::ViewerSubject() : parent(), xfer(), viewerRPC(),
    borders(), shift(), preshift(), geometry()
{
    //
    // Initialize pointers to some Qt objects that don't get created
    // until later.
    //
    mainApp = 0;
    checkParent = 0;
    checkRenderer = 0;

    //
    // By default we don't want to defer heavy initialization work.
    //
    deferHeavyInitialization = false;
    heavyInitializationDone = false;

    //
    // Enabled interruption checking by default.
    //
    interruptionEnabled = true;

    //
    // Set a flag indicating that we're not presently launching a component.
    //
    launchingComponent = false;

    //
    // Will be set by the -launchengine flag.
    //
    launchEngineAtStartup = "";

    //
    // Set by BlockSocketSignals method.
    //
    blockSocketSignals = false;

    //
    // Set the processingFromParent flag to false to indicate that we are
    // not currently processing input from the parent and it should be
    // safe to process input from the client.
    //
    processingFromParent = false;

    //
    // Initialize some special opcodes for xfer.
    //
    animationStopOpcode = 0;
    iconifyOpcode = 0;

    //
    // can be overridden by the -numrestarts flag.
    //
    numEngineRestarts = VIEWER_NUM_ENGINE_RESTARTS;

    //
    // Initialize pointers to some objects that don't get created until later.
    //
    keepAliveTimer = 0;
    viewerRPCObserver = 0;
    syncObserver = 0;
    messageAtts = 0;
    statusAtts = 0;
    appearanceAtts = 0;
    syncAtts = 0;

    //
    // Set some flags related to viewer windows.
    //
    nowin = false;
    smallWindow = false;
    defaultStereoToOn = false;

    //
    // By default, read the config files.
    //
    noconfig = false;
    configFileName = 0;
    configMgr = 0;
    systemSettings = 0;
    localSettings = 0;

    //
    // Initialize pointers to some objects that don't get created until later.
    //
    plotFactory = 0;
    operatorFactory = 0;
    messageBuffer = 0;
    messagePipe[0] = -1; messagePipe[1] = -1;
    pluginAtts = 0;

    viewerSubject = this;   // FIX_ME Hack, this should be removed.
}

// ****************************************************************************
//  Method: ViewerSubject destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
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
//    Brad Whitlock, Tue Jun 17 14:27:26 PST 2003
//    Completely reorganized.
//
//    Brad Whitlock, Tue Jul 29 11:18:07 PDT 2003
//    I changed the interface to ParentProcess::Connect.
//
// ****************************************************************************

void
ViewerSubject::Connect(int *argc, char ***argv)
{
    //
    // Connect to the parent.
    //
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();
    parent.Connect(1, 1, argc, argv, true);
    visitTimer->StopTimer(timeid, "Connecting to client");

    //
    // Create objects.
    //

    // Create the messaging attributes.
    messageAtts = new MessageAttributes;
    statusAtts = new StatusAttributes;
    // Create the appearance attributes.
    appearanceAtts = new AppearanceAttributes;
    // Create the sync attributes.
    syncAtts = new SyncAttributes;
    // Create the plugin attributes
    pluginAtts = new PluginManagerAttributes;
    messageBuffer = new ViewerMessageBuffer;

    //
    // Read the config files.
    //
    ReadConfigFiles(*argc, *argv);

    //
    // Process the command line arguments first since some may be removed
    // by QApplication::QApplication.
    //
    ProcessCommandLine(argc, argv);

    //
    // Create the QApplication context. This sets the qApp pointer.
    //
    char **argv2 = new char *[*argc + 3];
    int argc2 = *argc + 2;
    for(int i = 0; i < *argc; ++i)
        argv2[i] = (*argv)[i];
    argv2[*argc] = "-font";
    argv2[*argc+1] = (char*)appearanceAtts->GetFontDescription().c_str();
    argv2[*argc+2] = NULL;
    mainApp = new QApplication(argc2, argv2, !nowin);
    CustomizeAppearance();
    delete [] argv2;

    //
    // Set up the Xfer object.
    //
    ConnectXfer();

    //
    // Connect the socket notifiers, etc.
    //
    ConnectObjectsAndHandlers();

    //
    // Connect the objects to the config manager.
    //
    ConnectConfigManager();

    //
    // If we are not deferring heavy initialization, do it now.
    //
    if(!deferHeavyInitialization)
        HeavyInitialization();

    visitTimer->StopTimer(total, "Total time connecting and setting up");
}

// ****************************************************************************
//  Method: ViewerSubject::ReadConfigFiles.
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
ViewerSubject::ReadConfigFiles(int argc, char **argv)
{
    int timeid = visitTimer->StartTimer();

    //
    // Look for config file, related flags.
    //
    for (int i = 1 ; i < argc ; i++)
    {
        if (strcmp(argv[i], "-noconfig") == 0)
        {
            noconfig = true;
            if(configFileName != 0)
            {
                delete [] configFileName;
                configFileName = 0;
            }
        }
        else if (strcmp(argv[i], "-config") == 0 && (i+1) < argc && !noconfig)
        {
            if(configFileName != 0)
            {
                delete [] configFileName;
                configFileName = 0;
            }
            int len = strlen(argv[i+1]);
            configFileName = new char[len + 1];
            strcpy(configFileName, argv[i+1]);
        }       
    }

    //
    // Read the config file and setup the appearance attributes. Note that
    // we call the routine to process the config file settings here because
    // it only has to update the appearance attributes.
    //
    configMgr = new ViewerConfigManager(this);
    timeid = visitTimer->StartTimer();
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
    configMgr->ClearSubjects();
    visitTimer->StopTimer(timeid, "Reading config files.");
}

// ****************************************************************************
// Method: ViewerSubject::ConnectXfer
//
// Purpose: 
//   Connects various objects to the Xfer object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 14:56:01 PST 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 11:11:00 PDT 2003
//   I added curve view client attributes.
//
//   Brad Whitlock, Wed Oct 29 11:03:56 PDT 2003
//   Added the viewer window manager's annotation object list to xfer.
//
//   Brad Whitlock, Fri Jan 23 09:47:24 PDT 2004
//   I added the file server's database correlation list to xfer.
//
//   Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//   Added ViewerQueryManger's QueryOverTimeAtts to xfer.
//
// ****************************************************************************

void
ViewerSubject::ConnectXfer()
{
    //
    // Create an xfer object for communicating the RPCs.
    //
    xfer.Add(&viewerRPC);
    xfer.SetInputConnection(parent.GetWriteConnection());
    xfer.SetOutputConnection(parent.GetReadConnection());

    //
    // Connect the client attribute subjects.
    //
    xfer.Add(syncAtts);
    xfer.Add(appearanceAtts);
    xfer.Add(pluginAtts);
    xfer.Add(ViewerWindowManager::GetClientAtts());
    xfer.Add(ViewerFileServer::Instance()->GetDatabaseCorrelationList());
    xfer.Add(ViewerPlotList::GetClientAtts());
    xfer.Add(ViewerEngineManager::GetClientAtts());
    xfer.Add(messageAtts);
    xfer.Add(ViewerWindowManager::GetSaveWindowClientAtts());
    xfer.Add(statusAtts);
    xfer.Add(ViewerEngineManager::GetEngineList());
    xfer.Add(avtColorTables::Instance()->GetColorTables());
    xfer.Add(ParsingExprList::Instance()->GetList());
    xfer.Add(ViewerWindowManager::Instance()->GetAnnotationClientAtts());
    xfer.Add(ViewerPlotList::GetClientSILRestrictionAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetViewCurveClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetView2DClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetView3DClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetLightListClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetAnimationClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetPickClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetPrinterClientAtts());
    xfer.Add(ViewerWindowManager::Instance()->GetWindowInformation());
    xfer.Add(ViewerWindowManager::Instance()->GetRenderingAttributes());
    xfer.Add(ViewerWindowManager::Instance()->GetKeyframeClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetQueryTypes());
    xfer.Add(ViewerQueryManager::Instance()->GetQueryClientAtts());
    xfer.Add(ViewerEngineManager::GetMaterialClientAtts());
    xfer.Add(ViewerQueryManager::Instance()->GetGlobalLineoutClientAtts());
    xfer.Add(ViewerWindowManager::GetAnnotationObjectList());
    xfer.Add(ViewerQueryManager::Instance()->GetQueryOverTimeClientAtts());

    //
    // Set up special opcodes and their handler.
    //
    animationStopOpcode = xfer.CreateNewSpecialOpcode();
    iconifyOpcode = xfer.CreateNewSpecialOpcode();
    xfer.SetupSpecialOpcodeHandler(SpecialOpcodeCallback, (void *)this);
}

// ****************************************************************************
// Method: ViewerSubject::ConnectObjectsAndHandlers
//
// Purpose: 
//   Creates certain objects that are observers and sets up their slots and
//   callback functions.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 14:56:26 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//   
// ****************************************************************************

void
ViewerSubject::ConnectObjectsAndHandlers()
{
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
    // Create a timer that activates every 5 minutes to send a keep alive
    // signal to all of the remote processes. This will keep their connections
    // alive.
    //
    keepAliveTimer = new QTimer(this, "keepAliveTimer");
    connect(keepAliveTimer, SIGNAL(timeout()),
            this, SLOT(SendKeepAlives()));
    keepAliveTimer->start(5 * 60 * 1000);

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
    // Get the localhost name from the parent and give it to the
    // ViewerEngineManager and EngineKey so it can use it when needed.
    //
    ViewerServerManager::SetLocalHost(parent.GetHostName());
    EngineKey::SetLocalHost(parent.GetHostName());

    //
    // Set the default user name.
    //
    HostProfile::SetDefaultUserName(parent.GetTheUserName());
}

// ****************************************************************************
// Method: ViewerSubject::ConnectConfigManager
//
// Purpose: 
//   Connects objects to the config manager.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 14:57:28 PST 2003
//
// Modifications:
//    Eric Brugger, Wed Aug 20 11:11:00 PDT 2003
//    I added curve view client attributes.
//
//    Brad Whitlock, Fri Nov 7 13:57:47 PST 2003
//    I added the default annotation object list to the config manager..
//
//    Kathleen Bonnell, Wed Dec 17 14:44:26 PST 2003 
//    Added the default pick attributes to the config manager.
//
//    Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//    Added ViewerQueryManger's QueryOverTimeAtts to config manager.
//
// ****************************************************************************

void
ViewerSubject::ConnectConfigManager()
{
    //
    // Create the configuration manager and connect objects that can be
    // written to the config file.
    //
    configMgr->Add(ViewerWindowManager::GetClientAtts());
    configMgr->Add(ViewerEngineManager::GetClientAtts());
    configMgr->Add(ViewerWindowManager::GetSaveWindowClientAtts());
    configMgr->Add(avtColorTables::Instance()->GetColorTables());
    configMgr->Add(ParsingExprList::Instance()->GetList());
    configMgr->Add(ViewerWindowManager::GetAnimationClientAtts());
    configMgr->Add(ViewerWindowManager::GetAnnotationDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetViewCurveClientAtts());
    configMgr->Add(ViewerWindowManager::GetView2DClientAtts());
    configMgr->Add(ViewerWindowManager::GetView3DClientAtts());
    configMgr->Add(ViewerWindowManager::GetLightListDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetWindowAtts());
    configMgr->Add(ViewerWindowManager::GetWindowInformation());
    configMgr->Add(ViewerWindowManager::GetPrinterClientAtts());
    configMgr->Add(ViewerWindowManager::GetRenderingAttributes());
    configMgr->Add(ViewerEngineManager::GetMaterialDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetDefaultAnnotationObjectList());
    configMgr->Add(ViewerQueryManager::Instance()->GetPickDefaultAtts());
    configMgr->Add(ViewerQueryManager::Instance()->GetQueryOverTimeDefaultAtts());
}

// ****************************************************************************
// Method: ViewerSubject::InformClientOfPlugins
//
// Purpose: 
//   Inform the client of the plugins that are loaded. This needs to be done
//   prior to the config settings being read for the plugin objects because
//   once that happens, they are added to the config manager, which means that
//   they get transmitted to the client. Sending the pluginAtts first ensures
//   that the client will load the plugins before getting plugin object data.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 25 12:02:41 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::InformClientOfPlugins() const
{
    pluginAtts->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::HeavyInitialization
//
// Purpose: 
//   Does the expensive initialization like loading plugins, processing the
//   config file, setting up windows, and possibly launching an engine.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:09:28 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Mon Sep 15 13:09:19 PDT 2003
//   Tell ViewerQueryManager to initialize the query list after plugins load.
//   
//   Jeremy Meredith, Fri Sep 26 12:50:29 PDT 2003
//   Modify default rendering attributes to use stereo if it was specified
//   on the command line.
//
// ****************************************************************************

void
ViewerSubject::HeavyInitialization()
{
    if(!heavyInitializationDone)
    {
        int timeid = visitTimer->StartTimer();
        //
        // Do everything related to loading plugins.
        //
        InitializePluginManagers();
        LoadPlotPlugins();
        LoadOperatorPlugins();
        InformClientOfPlugins();

        ViewerQueryManager::Instance()->InitializeQueryList();
        //
        // Process the config file settings.
        //
        ProcessConfigFileSettings();

        //
        // Turn on stereo if it was enabled from the command line
        //
        if (defaultStereoToOn)
            ViewerWindowManager::GetRenderingAttributes()->SetStereoRendering(true);

        //
        // Add the initial windows.
        //
        AddInitialWindows();

        //
        // Launch an engine if certain command line flags were given.
        //
        LaunchEngineOnStartup();

        heavyInitializationDone = true;
        visitTimer->StopTimer(timeid, "Heavy initialization.");
    }
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
//    Brad Whitlock, Fri Jul 25 12:24:37 PDT 2003
//    Moved most of the code elsewhere.
//
// ****************************************************************************

int
ViewerSubject::Execute()
{
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
// Method: ViewerSubject::InitializePluginManagers
//
// Purpose: 
//   Reads the common plugin info for plot and operator plugins and populates
//   the pluginAtts.
//
// Note:       Moved from other methods.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:15:11 PST 2003
//
// Modifications:
//    Jeremy Meredith, Wed Nov  5 13:25:50 PST 2003
//    Added ability to have plugins disabled by default.
//
// ****************************************************************************

void
ViewerSubject::InitializePluginManagers()
{
    //
    // Load the plugin info.
    //
    int timeid = visitTimer->StartTimer();
    PlotPluginManager::Initialize(PlotPluginManager::Viewer);
    OperatorPluginManager::Initialize(OperatorPluginManager::Viewer);
    visitTimer->StopTimer(timeid, "Loading plugin info.");

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
    // to be disabled in the plugin managers and enable the ones
    // specified to be enabled.
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
        else
        {
            if (pluginAtts->GetType()[i] == "plot")
            {
                if (pmgr->PluginExists(id))
                    pmgr->EnablePlugin(id);
            }
            else if (pluginAtts->GetType()[i] == "operator")
            {
                if (omgr->PluginExists(id))
                    omgr->EnablePlugin(id);
            }
        }
    }

    //
    // Now add those to the atts that are in the manager but not yet listed
    // List them as enabled or disabled by their default state
    //
    for (i=0; i<pmgr->GetNAllPlugins(); i++)
    {
        std::string id = pmgr->GetAllID(i);
        if (pluginAtts->GetIndexByID(id) < 0)
        {
            pluginAtts->AddPlugin(pmgr->GetPluginName(id),     "plot",
                                  pmgr->GetPluginVersion(id),  id,
                                  pmgr->PluginEnabled(id));
        }
    }
    for (i=0; i<omgr->GetNAllPlugins(); i++)
    {
        std::string id = omgr->GetAllID(i);
        if (pluginAtts->GetIndexByID(id) < 0)
        {
            pluginAtts->AddPlugin(omgr->GetPluginName(id),     "operator",
                                  omgr->GetPluginVersion(id),  id,
                                  omgr->PluginEnabled(id));
        }
    }
}

// ****************************************************************************
// Method: ViewerSubject::LoadPlotPlugins
//
// Purpose: 
//   Loads the plot plugins and creates the plot factory object.
//
// Note:       Moved from other methods to here.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:16:13 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Sep 9 16:30:34 PST 2003
//   I fixed a bug where I accidentally hooked up the client plot attributes
//   to the config manager instead of hooking up the default plot attributes.
//   This prevented the plot attributes from being correctly sent to the
//   client.
//   
// ****************************************************************************

void
ViewerSubject::LoadPlotPlugins()
{
    int total  = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        PlotPluginManager::Instance()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the plot plugins. "
             << "The error message is: \"" << e.GetMessage().c_str() << "\"" << endl;
    }
    ENDTRY
    visitTimer->StopTimer(timeid, "Loading plot plugins.");

    //
    // Create the Plot factory.
    //
    plotFactory = new ViewerPlotFactory();
    for (int i = 0; i < plotFactory->GetNPlotTypes(); ++i)
    {
        AttributeSubject *attr = plotFactory->GetClientAtts(i);
        AttributeSubject *defaultAttr = plotFactory->GetDefaultAtts(i);

        if (attr != 0)
            xfer.Add(attr);

        if (defaultAttr != 0)
            configMgr->Add(defaultAttr);
    }

    visitTimer->StopTimer(total, "Loading plot plugins and instantiating objects.");
}

// ****************************************************************************
// Method: ViewerSubject::LoadOperatorPlugins
//
// Purpose: 
//   Loads the operator plugins and creates the operator factory object.
//
// Note:       Moved from other methods to here.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:16:57 PST 2003
//
// Modifications:
//   Brad Whitlock, Tue Sep 9 16:30:34 PST 2003
//   I fixed a bug where I accidentally hooked up the client operator
//   attributes to the config manager instead of hooking up the default
//   operator attributes. This prevented the operator attributes from being
//   correctly sent to the client
//
// ****************************************************************************

void
ViewerSubject::LoadOperatorPlugins()
{
    int total = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    //
    // It's safe to load the plugins now
    //
    TRY
    {
        OperatorPluginManager::Instance()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the operator plugins. "
             << "The error message is: \"" << e.GetMessage().c_str() << "\"" << endl;
    }
    ENDTRY
    visitTimer->StopTimer(timeid, "Loading operator plugins.");

    //
    // Create the Operator factory.
    //
    operatorFactory = new ViewerOperatorFactory();
    for (int i = 0; i < operatorFactory->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *attr = operatorFactory->GetClientAtts(i);
        AttributeSubject *defaultAttr = operatorFactory->GetDefaultAtts(i);

        if (attr != 0)
            xfer.Add(attr);

        if(defaultAttr)
            configMgr->Add(defaultAttr);
    }

    // Set the query manager's operator factory pointer.
    ViewerQueryManager::Instance()->SetOperatorFactory(operatorFactory);

    // List the objects connected to xfer.
    xfer.ListObjects();

    visitTimer->StopTimer(total, "Loading operator plugins and instantiating objects.");
}

// ****************************************************************************
// Method: ViewerSubject::ProcessConfigFileSettings
//
// Purpose: 
//   Processes the config file settings that were read in.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:19:27 PST 2003
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 17 14:44:26 PST 2003
//   Added PickAtts.
//
//   Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//   Added QueryOverTimeAtts.
//
// ****************************************************************************

void
ViewerSubject::ProcessConfigFileSettings()
{
    //
    // Make the hooked up state objects set their properties from
    // both the system and local settings.
    //
    int timeid = visitTimer->StartTimer();
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);

    // Import external color tables.
    if(!noconfig)
        avtColorTables::Instance()->ImportColorTables();

    // Send the user's config settings to the client.
    configMgr->Notify();

    delete systemSettings; systemSettings = 0;

    // Let other viewer objects set their properties from the local settings.
    if(localSettings)
        QTimer::singleShot(300, this, SLOT(DelayedProcessSettings()));

    // Add the appearanceAtts *after* the config settings have been read. This
    // prevents overwriting the attributes and sending them to the client.
    configMgr->Add(appearanceAtts);

    // Add the pluginAtts *after* the config settings have been read.
    // First, tell the client which plugins we've really loaded.
    configMgr->Add(pluginAtts);

    // Copy the default annotation attributes into the client annotation
    // attributes.
    ViewerWindowManager::SetClientAnnotationAttsFromDefault();

    // Copy the default material atts to the client material atts
    ViewerEngineManager::SetClientMaterialAttsFromDefault();

    // Copy the default pick atts to the client pick atts
    ViewerQueryManager::Instance()->SetClientPickAttsFromDefault();

    // Copy the default time query atts to the client time query atts
    ViewerQueryManager::Instance()->SetClientQueryOverTimeAttsFromDefault();

    // Send the queries to the client.
    ViewerQueryManager::Instance()->GetQueryTypes()->Notify();

    visitTimer->StopTimer(timeid, "Processing config file data.");
}

// ****************************************************************************
// Method: ViewerSubject::AddInitialWindows
//
// Purpose: 
//   Adds the appropriate number of vis windows.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:20:20 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::AddInitialWindows()
{
    //
    // Create the window.
    //
    ViewerWindowManager *windowManager=ViewerWindowManager::Instance();
    if (windowManager != NULL)
    {
        int timeid = visitTimer->StartTimer();
        // Connect
        connect(windowManager, SIGNAL(createWindow(ViewerWindow *)),
                this, SLOT(ConnectWindow(ViewerWindow *)));
        connect(windowManager, SIGNAL(deleteWindow(ViewerWindow *)),
                this, SLOT(DisconnectWindow(ViewerWindow *)));

        // Initialize the area that will be used to place the windows.
        InitializeWorkArea();

        // Make the window manager add an initial window.
        windowManager->AddWindow();

        visitTimer->StopTimer(timeid, "Adding windows.");
    }
}

// ****************************************************************************
// Method: ViewerSubject::LaunchEngineOnStartup
//
// Purpose: 
//   Launches an engine as part of the heavy initialization step.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 15:21:00 PST 2003
//
// Modifications:
//    Jeremy Meredith, Tue Mar 23 14:34:53 PST 2004
//    Use the engineParallelArguments for this launch.
//   
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to start engines.  We know we will not be
//    connecting to a running simulation at startup (right now), so we
//    can safely call CreateEngine instead of ConnectSim.
//
// ****************************************************************************

void
ViewerSubject::LaunchEngineOnStartup()
{
    //
    // Launch an engine if needed, never popping up the chooser window
    //
    if (launchEngineAtStartup != "")
    {
        stringVector noArgs;
        ViewerEngineManager::Instance()->
                CreateEngine(EngineKey(launchEngineAtStartup,""),
                             engineParallelArguments, true, numEngineRestarts);
    }
}

// ****************************************************************************
// Method: ViewerSubject::DelayedProcessSettings
//
// Purpose: 
//   Lets the various viewer objects from ViewerSubject on down process
//   settings using the localSettings DataNode.
//
// Note:       This is a Qt slot function that is called once the program
//             enters the event loop.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 2 12:24:07 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DelayedProcessSettings()
{
    if(localSettings != 0)
    {
        // Get the VisIt node.
        DataNode *visitRoot = localSettings->GetNode("VisIt");
        if(visitRoot == 0)
            return;
 
        // Get the viewer node.
        DataNode *viewerNode = visitRoot->GetNode("VIEWER");
        if(viewerNode == 0)
            return;

        // Try and set up everything from within the event loop.
        SetFromNode(viewerNode);

        delete localSettings;  localSettings = 0;
    }
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
    if (cbData)
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
//   Brad Whitlock, Tue Sep 9 15:38:10 PST 2003
//   I increased the amount of time that we can use to process events.
//
// ****************************************************************************

void
ViewerSubject::ProcessEvents()
{
    if (interruptionEnabled)
    {
         mainApp->processEvents(100);
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
//   Brad Whitlock, Fri Aug 15 13:18:41 PST 2003
//   Added support for more styles in Qt 3.0 and beyond.
//
// ****************************************************************************

void
ViewerSubject::CustomizeAppearance()
{
    //
    // Set the style and inform the widgets.
    //
#if QT_VERSION < 300
    if (appearanceAtts->GetStyle() == "cde")
        mainApp->setStyle(new QCDEStyle);
    else if (appearanceAtts->GetStyle() == "windows")
        mainApp->setStyle(new QWindowsStyle);
    else if (appearanceAtts->GetStyle() == "platinum")
        mainApp->setStyle(new QPlatinumStyle);
#if QT_VERSION >= 230
    else if (appearanceAtts->GetStyle() == "sgi")
        mainApp->setStyle(new QSGIStyle);
#endif
    else
        mainApp->setStyle(new QMotifStyle);
#else
    // Set the style via the style name.
    mainApp->setStyle(appearanceAtts->GetStyle().c_str());
#endif

    //
    // Set the colors and inform the widgets.
    //
    if(appearanceAtts->GetStyle() != "aqua" &&
       appearanceAtts->GetStyle() != "macintosh")
    {
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
        if (v >= 255 - 50)
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
        if (bright_mode)
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
//  Method: ViewerSubject::ProcessCommandLine
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
//    Jeremy Meredith, Thu Jun 26 10:52:32 PDT 2003
//    Renamed ViewerEngineChooser to ViewerRemoteProcessChooser.
//
//    Jeremy Meredith, Thu Jul  3 15:00:10 PDT 2003
//    Added -nopty to disable PTYs.  Added this functionality implicitly
//    to -nowin mode as well, as we use PTYs to capture text for windowing
//    but with -nowin we cannot open windows.
//
//    Brad Whitlock, Mon Jun 16 12:33:53 PDT 2003
//    I added code to make sure -rpipe and -wpipe are not passed on. I also
//    added code to defer heavy initialization until later.
//
//    Jeremy Meredith, Fri Sep 26 12:50:57 PDT 2003
//    Added defaultStereoToOn.
//
//    Brad Whitlock, Fri Aug 15 13:20:16 PST 2003
//    Added support for MacOS X styles.
//
//    Jeremy Meredith, Tue Mar 23 14:31:29 PST 2004
//    Added parsing of the new "-engineargs" flag.  This is to eliminate
//    blind passing-on of arguments like "-np" etc., because we will not
//    be able to remove those later if the user needs to re-launch the engine.
//    The viewer expects the engine parallel arguments to be passed in
//    as a colon-delimited string, with an optional leading/trailing colon
//    to prevent parsing errors.  The visit script fills this expectation.
//
//    Jeremy Meredith, Fri Mar 26 09:21:48 PST 2004
//    Changed colons in engineargs to semicolons.  Some PSUB options now
//    take colons, so that would mess it up.  Commas are already used in
//    the PSUB constraints (e.g. white,batch), so those wouldn't work either.
//
// ****************************************************************************

void
ViewerSubject::ProcessCommandLine(int *argc, char ***argv)
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
            if (debugLevel > 0 && debugLevel < 6)
            {
                ViewerServerManager::SetDebugLevel(debugLevel);
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
        else if (strcmp(argv2[i], "-wpipe") == 0 ||
                 strcmp(argv2[i], "-rpipe") == 0)
        {
            // This argument and its following options are dangerous to pass on
            i += 2;
        }
        else if (strcmp(argv2[i], "-background") == 0 ||
                strcmp(argv2[i], "-bg") == 0)
        {
            if (i + 1 >= argc2)
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
        else if(strcmp(argv2[i], "-config") == 0)
        {
            // Make sure the -config flag and the filename that follows it is
            // not passed along to other components.
            ++i;
        }
        else if (strcmp(argv2[i], "-foreground") == 0 ||
                strcmp(argv2[i], "-fg") == 0)
        {
            if (i + 1 >= argc2)
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
        else if (strcmp(argv2[i], "-style") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "The -style option must be followed by a style name."
                     << endl;
                continue;
            }
            if (strcmp(argv2[i + 1], "motif") == 0 ||
               strcmp(argv2[i + 1], "cde") == 0 ||
               strcmp(argv2[i + 1], "windows") == 0 ||
               strcmp(argv2[i + 1], "platinum") == 0
#if QT_VERSION >= 230
               || strcmp(argv2[i + 1], "sgi") == 0
#endif
#if QT_VERSION >= 300
#ifdef QT_WS_MACX
               || strcmp(argv2[i + 1], "aqua") == 0
               || strcmp(argv2[i + 1], "macintosh") == 0
#endif
#endif
                     )
            {
                appearanceAtts->SetStyle(argv2[i+1]);
            }
            ++i;
        }
        else if (strcmp(argv2[i], "-font") == 0)
        {
            if (i + 1 >= argc2)
            {
                cerr << "The -font option must be followed by a "
                        "font description." << endl;
                continue;
            }
            appearanceAtts->SetFontDescription(argv2[i + 1]);
            ++i;
        }
        else if (strcmp(argv2[i], "-timing") == 0)
        {
            //
            // Enable timing and pass the option to child processes.
            //
            visitTimer->Enable();

            unknownArguments.push_back(argv2[i]);
        }
        else if (strcmp(argv2[i], "-noint") == 0)
        {
            interruptionEnabled = false;
        }
        else if (strcmp(argv2[i], "-noconfig") == 0)
        {
            // do nothing; processed by an earlier parsing of the command line
        }
        else if (strcmp(argv2[i], "-defer") == 0)
        {
            deferHeavyInitialization = true;
        }
        else if (strcmp(argv2[i], "-nowin") == 0)
        {
            InitVTK::ForceMesa();
            ViewerWindow::SetNoWinMode(true);
            ViewerRemoteProcessChooser::SetNoWinMode(true);
            avtCallback::SetNowinMode(true);
            RemoteProcess::DisablePTY();
            nowin = true;
        }
        else if (strcmp(argv2[i], "-nopty") == 0)
        {
            RemoteProcess::DisablePTY();
        }
        else if (strcmp(argv2[i], "-stereo") == 0)
        {
            VisWinRendering::SetStereoEnabled(true);
            defaultStereoToOn = true;
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
        else if (strcmp(argv2[i], "-engineargs") == 0)
        {
            if ((i + 1 >= argc2))
            {
                cerr << "The -engineargs option must be followed by a "
                        "string." << endl;
                continue;
            }
            engineParallelArguments = SplitValues(argv2[++i], ';');
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
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerSubject::Error(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Error);
    messageAtts->Notify();

    debug1 << "Error - " << message << endl;
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
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerSubject::Warning(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Warning);
    messageAtts->Notify();

    debug1 << "Warning - " << message << endl;
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
//   Brad Whitlock, Fri Mar 19 16:12:00 PST 2004
//   Added code to print errors to debug1.
//
// ****************************************************************************

void
ViewerSubject::Message(const char *message)
{
    if ((message == 0) || (strlen(message) < 1))
        return;

    // Send the message to the observers of the viewer's messageAtts.
    messageAtts->SetText(std::string(message));
    messageAtts->SetSeverity(MessageAttributes::Message);
    messageAtts->Notify();

    debug1 << "Message - " << message << endl;
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
// Method: ViewerSubject::CreateNode
//
// Purpose: 
//   Saves the viewer's state to a DataNode object.
//
// Arguments:
//   parentNode : The node to which the state is added.
//   detailed   : Tells whether lots of details should be added to the nodes.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:32:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Jul 18 10:47:20 PDT 2003
//   Added detailed argument. Made query manager add its data.
//
//   Brad Whitlock, Thu Mar 18 08:47:17 PDT 2004
//   Made the file server save its settings.
//
// ****************************************************************************

void
ViewerSubject::CreateNode(DataNode *parentNode, bool detailed)
{
    if(parentNode == 0)
        return;

    DataNode *vsNode = new DataNode("ViewerSubject");
    parentNode->AddNode(vsNode);

    ViewerFileServer::Instance()->CreateNode(vsNode, detailed);
    ViewerWindowManager::Instance()->CreateNode(vsNode, detailed);
    if(detailed)
        ViewerQueryManager::Instance()->CreateNode(vsNode);
}

// ****************************************************************************
// Method: ViewerSubject::SetFromNode
//
// Purpose: 
//   Sets the viewer's state from a DataNode object.
//
// Arguments:
//   parentNode : The DataNode object to use to set the state.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 30 12:36:00 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 22 10:12:59 PDT 2003
//   Added code to let the query manager initialize itself.
//
//   Brad Whitlock, Thu Mar 18 08:48:39 PDT 2004
//   Added code to initialize the file server.
//
// ****************************************************************************

void
ViewerSubject::SetFromNode(DataNode *parentNode)
{
    if(parentNode == 0)
        return;

    DataNode *searchNode = parentNode->GetNode("ViewerSubject");
    if(searchNode == 0)
        return;

    ViewerFileServer::Instance()->SetFromNode(searchNode);
    ViewerWindowManager::Instance()->SetFromNode(searchNode);
    ViewerQueryManager::Instance()->SetFromNode(searchNode);
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
// Function: getToken
//
// Purpose: 
//   Return the first token readed from a buffer string. 
//   If there are no more tokens, it returns an empty string.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 11 12:07:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string getToken(std::string buff, bool reset = false)
{
    static std::string::size_type pos1 = 0;
    std::string::size_type pos2;
    std::string token;
    
    if (reset)
      pos1 = 0;
    
    if (pos1 == std::string::npos)
        return token;

    pos1 = buff.find_first_not_of(' ', pos1);  
    if (pos1 == std::string::npos)
        return token;

    pos2 = buff.find_first_of(' ', pos1);
    token = buff.substr(pos1, pos2-pos1);
    pos1 = pos2;

    return token;
}


// ****************************************************************************
// Function: getVectorTokens
//
// Purpose: 
//   Return a vector of tokens readed from a buffer string. 
//   The first token tell us the number of tokens that must be readed.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 11 12:07:06 PST 2003
//
// Modifications:
//   
// ****************************************************************************


int getVectorTokens(std::string buff, std::vector<std::string> &tokens, int nodeType)
{
    int length, ival;
    std::string token, numTokens;
    long lval;
    float fval;
    double dval;

    tokens.clear();
    
    numTokens = getToken(buff);  
    if (sscanf(numTokens.c_str(),"%d",&length) != 1)
        return 0;

    for(int j=0; j<length; j++)
    {
        token = getToken(buff);
        if (token.size() != 0)
        {
              switch(nodeType)
            {
            case CHAR_ARRAY_NODE:
            case CHAR_VECTOR_NODE:
            case UNSIGNED_CHAR_ARRAY_NODE:
            case UNSIGNED_CHAR_VECTOR_NODE:
            case INT_ARRAY_NODE:
            case INT_VECTOR_NODE:
                if(sscanf(token.c_str(),"%d",&ival) == 1)
                    tokens.push_back(token);
                break;

            case LONG_ARRAY_NODE:
            case LONG_VECTOR_NODE:
                if(sscanf(token.c_str(),"%ld",&lval) == 1)
                    tokens.push_back(token);
                break;        
            
            case FLOAT_ARRAY_NODE:
            case FLOAT_VECTOR_NODE:
                if(sscanf(token.c_str(),"%f",&fval) == 1)
                    tokens.push_back(token);
                break;
            
            case DOUBLE_ARRAY_NODE:
            case DOUBLE_VECTOR_NODE:
                if(sscanf(token.c_str(),"%lf",&dval) == 1)
                    tokens.push_back(token);
                break;
              
            case STRING_ARRAY_NODE:
            case STRING_VECTOR_NODE:
            case BOOL_ARRAY_NODE:
            case BOOL_VECTOR_NODE:
                if (token.size() > 0)
                    tokens.push_back(token);
                break;
          }
        }
    }

    if (tokens.size() != length)
        tokens.clear();

    return tokens.size();
}


// ****************************************************************************
// Method: ViewerSubject::CreateAttributesDataNode
//
// Purpose: 
//   Create a DataNode with the attributes of one plot.
//
// Programmer: Walter Herrera
// Creation:   Tue Sep 9 10:27:46 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DataNode *
ViewerSubject::CreateAttributesDataNode(const avtDefaultPlotMetaData *dp) const
{
    DataNode *node = 0, *fatherNode, *newNode;
    std::string nodeTypeToken, fatherName, attrName, attrValue;
    std::vector<std::string> tokens;
    int nodeType = 0, length, ival;
    char cval;
    unsigned char ucval;
    long lval;
    float fval;
    double dval;

    for(int i=0; i < dp->plotAttributes.size(); i++) 
    {
        nodeTypeToken = getToken(dp->plotAttributes[i], true);  
        if(sscanf(nodeTypeToken.c_str(), "%d", &nodeType) != 1)
            continue;

        fatherName = getToken(dp->plotAttributes[i]);  
        attrName = getToken(dp->plotAttributes[i]);  

        fatherNode = 0;
        if (node != 0)
            fatherNode = node->GetNode(fatherName);

        switch(nodeType)
        {
        case INTERNAL_NODE:
            if(fatherName == "NULL")
            {
                if(node != 0)
                    delete node;

                node = new DataNode(attrName);
            }
            else if(fatherNode != 0)
            {
                newNode = new DataNode(attrName);
                fatherNode->AddNode(newNode);
            }
            break;

        case CHAR_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    cval = (char)ival;
                    newNode = new DataNode(attrName,cval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case UNSIGNED_CHAR_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    ucval = (unsigned char)ival;
                    newNode = new DataNode(attrName,ucval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case INT_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%d",&ival) == 1)
                {                
                    newNode = new DataNode(attrName,ival);
                    fatherNode->AddNode(newNode);
                }
            }
            break;        

        case LONG_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%ld",&lval) == 1)
                {                
                    newNode = new DataNode(attrName,lval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;        

        case FLOAT_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%f",&fval) == 1)
                {                
                    newNode = new DataNode(attrName,fval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case DOUBLE_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (sscanf(attrValue.c_str(),"%lf",&dval) == 1)
                {                
                    newNode = new DataNode(attrName,dval);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case STRING_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  
                if (attrValue.size() != 0)
                {
                    newNode = new DataNode(attrName,attrValue);
                    fatherNode->AddNode(newNode);
                }
            }
            break;

        case BOOL_NODE:
            if(fatherNode != 0)
            {
                attrValue = getToken(dp->plotAttributes[i]);  

                if (attrValue.size() != 0)
                {
                    newNode = new DataNode(attrName,(attrValue == "true"));
                    fatherNode->AddNode(newNode);
                } 
            }
            break;        

        case CHAR_ARRAY_NODE:
        case CHAR_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                char *arrayItems = new char[length];
                charVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&ival);
                    arrayItems[j] = (char)ival;
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == CHAR_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case UNSIGNED_CHAR_ARRAY_NODE:
        case UNSIGNED_CHAR_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                unsigned char *arrayItems = new unsigned char[length];
                unsignedCharVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&ival);
                    arrayItems[j] = (unsigned char)ival;
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == UNSIGNED_CHAR_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case INT_ARRAY_NODE:
        case INT_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                int *arrayItems = new int[length];
                intVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%d",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == INT_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case LONG_ARRAY_NODE:
        case LONG_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                long *arrayItems = new long[length];
                longVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%ld",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == LONG_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;        

        case FLOAT_ARRAY_NODE:
        case FLOAT_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                float *arrayItems = new float[length];
                floatVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%f",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == FLOAT_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case DOUBLE_ARRAY_NODE:
        case DOUBLE_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                double *arrayItems = new double[length];
                doubleVector vectorItems(length);

                for(int j=0; j<length; j++)
                {
                    sscanf(tokens[j].c_str(),"%lf",&arrayItems[j]);
                    vectorItems.push_back(arrayItems[j]);
                }
                if (nodeType == DOUBLE_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,vectorItems);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case STRING_ARRAY_NODE:
        case STRING_VECTOR_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                std::string *arrayItems = new std::string[length];

                for(int j=0; j<length; j++)
                {
                    arrayItems[j] = tokens[j];
                }
                if (nodeType == STRING_ARRAY_NODE)
                    newNode = new DataNode(attrName,arrayItems,length);
                else
                    newNode = new DataNode(attrName,tokens);

                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;

        case BOOL_ARRAY_NODE:
            length = getVectorTokens(dp->plotAttributes[i], tokens, nodeType);

            if(fatherNode != 0 && length > 0)
            {
                bool *arrayItems = new bool[length];

                for(int j=0; j<length; j++)
                {
                    arrayItems[j] = (tokens[j] == "true");
                }

                newNode = new DataNode(attrName,arrayItems,length);
                fatherNode->AddNode(newNode);
                delete [] arrayItems;
            }
            break;
        }
    }
    
    return node;
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
//    Hank Childs, Thu Aug 14 09:10:00 PDT 2003
//    Added code to manage expressions from databases.
//
//    Walter Herrera, Thu Sep 04 16:13:43 PST 2003
//    I made it capable of creating default plots
//
//    Brad Whitlock, Fri Oct 3 10:40:49 PDT 2003
//    I prevented the addition of default plots if the plot list already
//    contains plots from the new database.
//
//    Brad Whitlock, Wed Oct 22 12:27:30 PDT 2003
//    I made the method actually use the addDefaultPlots argument.
//
//    Brad Whitlock, Fri Oct 24 17:07:52 PST 2003
//    I moved the code to update the expression list into the plot list.
//
//    Hank Childs, Fri Mar  5 11:39:22 PST 2004
//    Send the file format type to the engine.
//
//    Jeremy Meredith, Mon Mar 22 17:12:22 PST 2004
//    I made use of the "success" result flag from CreateEngine.
//
//    Brad Whitlock, Tue Mar 23 17:41:57 PST 2004
//    I added support for database correlations. I also prevented the default
//    plot from being realized if the engine was not launched.
//
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//    Added support for connecting to running simulations.
//
//    Jeremy Meredith, Fri Apr  2 14:14:54 PST 2004
//    Made it re-use command line arguments if we had some, as long as 
//    we were in nowin mode.
//
// ****************************************************************************

void
ViewerSubject::OpenDatabaseHelper(const std::string &entireDBName,
    int timeState, bool addDefaultPlots)
{
    int  i;

    debug1 << "Opening database " << entireDBName.c_str()
           << ", timeState=" << timeState << endl;

    //
    // Associate the database with the currently active animation (window).
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();

    //
    // Expand the new database name and then set it into the plot list.
    //
    std::string hdb(entireDBName), host, db;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    fs->ExpandDatabaseName(hdb, host, db);
    plotList->SetHostDatabaseName(hdb.c_str());

    //
    // Get the number of time states and set that information into the
    // active animation.
    //
    const avtDatabaseMetaData *md = fs->GetMetaDataForState(host, db, timeState);
    if (md != NULL)
    {
        //
        // If the database has more than one time state then we should
        // add it to the list of database correlations so we have a trivial
        // correlation for this database.
        //
        if(md->GetNumStates() > 1)
        {
            //
            // Get the name of the database so we can use that for the name
            // of a new trivial database correlation.
            //
            const std::string &correlationName = plotList->GetHostDatabaseName();

            debug3 << "Correlation for " << hdb.c_str() << " is "
                   << correlationName.c_str() << endl;

            //
            // Tell the window manager to create the correlation. We could
            // use the file server but this way also creates time sliders
            // for the new correlation in each window and makes the active
            // window's active time slider be the new correlation.
            //
            stringVector dbs; dbs.push_back(correlationName);
            int timeSliderState = (timeState >= 0) ? timeState : 0;
            wM->CreateDatabaseCorrelation(correlationName, dbs, 0,
                timeSliderState, md->GetNumStates());
        }

        //
        // Update the global atts since that has the list of sources.
        //
        wM->UpdateGlobalAtts();

        //
        // Since we updated the source and we made have also updated the time
        // slider and time slider states when the new database was opened, send
        // back the source, time sliders, and animation information.
        //
        wM->UpdateWindowInformation(WINDOWINFO_SOURCE | WINDOWINFO_TIMESLIDERS |
                                    WINDOWINFO_ANIMATION);

        //
        // Update the expression list.
        //
        plotList->UpdateExpressionList(false);

        //
        // Determine the name of the simulation
        //
        std::string sim = "";
        if (md->GetIsSimulation())
            sim = db;

        //
        // Create an engine key, used to index and start engines
        //
        EngineKey ek(host, sim);

        //
        // Tell the plot list the new engine key
        //
        plotList->SetEngineKey(ek);

        //
        // Create a compute engine to use with the database.
        //
        stringVector noArgs;
        bool success;
        if (md->GetIsSimulation())
        {
            success = ViewerEngineManager::Instance()->
                                                 ConnectSim(ek, noArgs,
                                                            md->GetSimHost(),
                                                            md->GetSimPort());
        }
        else
        {
            if (nowin)
            {
                success = ViewerEngineManager::Instance()->
                                     CreateEngine(ek, engineParallelArguments,
                                                  false, numEngineRestarts);
            }
            else
            {
                success = ViewerEngineManager::Instance()->
                                          CreateEngine(ek, noArgs, false,
                                                       numEngineRestarts);
            }
        }

        if (success)
        {
            //
            // Tell the new engine to open the specified database.
            // Don't bother if you couldn't even start an engine.
            //
            ViewerEngineManager *eMgr = ViewerEngineManager::Instance();
            if(md->GetIsVirtualDatabase() && md->GetNumStates() > 1)
            {
                eMgr->DefineVirtualDatabase(ek, md->GetFileFormat().c_str(),
                                            db.c_str(),
                                            md->GetTimeStepPath().c_str(),
                                            md->GetTimeStepNames(),
                                            timeState);
            }
            else
            {
                eMgr->OpenDatabase(ek, md->GetFileFormat().c_str(),
                                   db.c_str(), timeState);
            }
        }
        
        //
        // Create default plots if there are no plots from the database
        // already in the plot list.
        //
        if(addDefaultPlots && !plotList->FileInUse(host, db))
        {
            DataNode *adn = NULL;
            bool defaultPlotsAdded = false;

            for(i=0; i<md->GetNumDefaultPlots(); i++)
            {
                const avtDefaultPlotMetaData *dp = md->GetDefaultPlot(i);
                adn = CreateAttributesDataNode(dp);

                //
                // Use the plot plugin manager to get the plot type index from
                // the plugin id.
                //
                int type = PlotPluginManager::Instance()->GetEnabledIndex(dp->pluginID);

                if(type != -1)
                {
                    debug4 << "Adding default plot: type=" << type
                           << " var=" << dp->plotVar.c_str() << endl;
                    plotList->AddPlot(type, dp->plotVar, false, false, adn);
                    defaultPlotsAdded = true;
                }
            }

            //
            // Only realize the plots if we added some default plots *and*
            // the engine was successfully launched above.
            //
            if (defaultPlotsAdded && success)
            {
                plotList->RealizePlots();
            } 

            if (adn != NULL)
                delete adn;
        }
        else
        {
            debug4 << "Default plots were not added because the plot list "
                      "already contains plots from "
                   << host.c_str() << ":" << db.c_str() << endl;
        }
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
//   Brad Whitlock, Wed Oct 22 12:28:57 PDT 2003
//   I made it possible for default plots to not be added.
//
// ****************************************************************************

void
ViewerSubject::OpenDatabase()
{
    OpenDatabaseHelper(viewerRPC.GetDatabase(), viewerRPC.GetIntArg1(),
                       viewerRPC.GetBoolFlag());
}

// ****************************************************************************
// Method: ViewerSubject::ActivateDatabase
//
// Purpose: 
//   Sets the specified database as the new active source. This has the effect
//   of changing the active time slider to the time slider that best matches
//   the new source and the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 23:46:49 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ActivateDatabase()
{
    const std::string &database = viewerRPC.GetDatabase();

    //
    // Expand the database name to its full path just in case.
    //
    std::string expandedDB(database), host, db;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    fs->ExpandDatabaseName(expandedDB, host, db);

    //
    // If the database has been opened before then we can make it the active
    // plot list's active database. Then we can set the time slider if we
    // need to.
    //
    if(fs->IsDatabase(expandedDB))
    {
        const avtDatabaseMetaData *md = fs->GetMetaData(host, db);
        EngineKey newEngineKey;
        if (md && md->GetIsSimulation())
            newEngineKey = EngineKey(host, db);
        else
            newEngineKey = EngineKey(host, "");


        ViewerWindowManager::Instance()->GetActiveWindow()->
            GetPlotList()->ActivateSource(expandedDB, newEngineKey);
    }
    else
    {
        // We have not seen the database before so open it.
        OpenDatabaseHelper(database, 0, true);
    }
}

// ****************************************************************************
// Method: ViewerSubject::CheckForNewStates
//
// Purpose: 
//   Adds new time states for a database if there are any new states to add.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:56:42 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CheckForNewStates()
{
    //
    // Add new states to the specified database and update the metadata,
    // correlations, and plot list caches.
    //
    debug1 << "CheckForNewStates: " << viewerRPC.GetDatabase().c_str();
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
//   Kathleen Bonnell, Wed Jul 23 16:46:30 PDT 2003
//   Removed view recentering.
//
//   Brad Whitlock, Wed Oct 15 14:24:26 PST 2003
//   I made it call ViewerWindowManager's new ReplaceDatabase method so that
//   when the file is reopened, we replace the old version of the database
//   in all windows. This is primarily to let all windows know the new
//   size of the database if it is virtual and more time states have been
//   added.
//
//   Brad Whitlock, Mon Nov 3 10:03:51 PDT 2003
//   Made some interface changes to ViewerWindowManager::ReplaceDatabase.
//
//   Brad Whitlock, Fri Mar 19 16:20:40 PST 2004
//   I added code to expand the database name and use time sliders and 
//   database correlations to figure out where to open the database.
//
//   Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//   Added an engine key used to index (and restart) engines.
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
    // Expand the filename.
    //
    std::string host, db;
    ViewerFileServer *fileServer = ViewerFileServer::Instance();
    fileServer->ExpandDatabaseName(hostDatabase, host, db);
    debug1 << "Reopening " << hostDatabase.c_str() << endl;

    //
    // Clear default SIL restrictions
    //
    ViewerPlotList::ClearDefaultSILRestrictions(host, db);

    //
    // Clear out any previous information about the file on the mdserver.
    //
    if (forceClose)
        fileServer->CloseFile(host);

    //
    // Try to determine the time state at which the file should be
    // reopened. If the plot list has an active time slider, see if
    // the time slider's correlation includes the database that we're
    // reopening. If so, then we can use the active time slider's
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();
    DatabaseCorrelationList *cL = fileServer->GetDatabaseCorrelationList();

    int reOpenState = 0;
    if(plotList->HasActiveTimeSlider())
    {
        const std::string &activeTimeSlider = plotList->GetActiveTimeSlider();
        debug3 << "Reopening " << hostDatabase.c_str()
               << " with an active time slider: " << activeTimeSlider.c_str()
               << endl;
        DatabaseCorrelation *correlation = cL->FindCorrelation(activeTimeSlider);
        if(correlation != 0)
        {
            int state = 0, nStates = 0;
            plotList->GetTimeSliderStates(activeTimeSlider, state, nStates);
            reOpenState = correlation->GetCorrelatedTimeState(hostDatabase, state);
            debug3 << "The active time slider was a correlation involving "
                   << hostDatabase.c_str()
                   << " so we're using the correlated state to reopen the file."
                   << " state = " << reOpenState << endl;
        }
    }

    if(reOpenState < 0)
    {
        // There either was no active time slider, no correlation for the
        // active time slider or there was a correlation for the active time
        // slider but it had nothing to do with the database that we want to
        // reopen. We should try and use the active time slider for the
        // database we're trying to open if there is such a time slider.
        int ns;
        plotList->GetTimeSliderStates(hostDatabase, reOpenState, ns);
        debug3 << "Could not use correlation or active time slider to "
               << "get the reopen state for " << hostDatabase.c_str()
               << ". Using state " << reOpenState << endl;
    }

    //
    // Get the flag to determine if this is a simulation before we
    // clear the metadata from the file server.
    //
    bool isSim = false;
    const avtDatabaseMetaData *md = fileServer->GetMetaData(host, db);
    if (md && md->GetIsSimulation())
        isSim = true;

    //
    // Clear out any local information that we've cached about the file. We
    // have to do this after checking for the correlation because this call
    // will remove the correlation for the database.
    //
    fileServer->ClearFile(hostDatabase);

    //
    // Tell the compute engine to clear any cached information about the
    // database so it forces the networks to re-execute.
    //
    if (isSim)
        ViewerEngineManager::Instance()->ClearCache(EngineKey(host,db),
                                                    db.c_str());
    else
        ViewerEngineManager::Instance()->ClearCache(EngineKey(host,""),
                                                    db.c_str());

    //
    // Open the database.
    //
    OpenDatabaseHelper(hostDatabase, reOpenState, false);
 
    //
    // Now perform the database replacement in all windows that use the
    // specified database.
    //
    ViewerWindowManager::Instance()->ReplaceDatabase(host, db, 0, false, true);
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
//   Brad Whitlock, Wed Oct 15 15:40:44 PST 2003
//   I made it possible to replace a database at a later time state.
//
//   Brad Whitlock, Mon Nov 3 09:50:21 PDT 2003
//   I changed a flag to false in the call to OpenDatabaseHelper so the
//   animation's number of frames would not be updated because this caused
//   extra work. I passed the time state to the plot list's ReplaceDatabase
//   method instead.
//
//   Eric Brugger, Mon Dec  8 08:09:54 PST 2003
//   I added a call to turn on view limit merging if the new database
//   was the same as the old one.  I also made the test controlling
//   the call to recenter view more restrictive, also requiring the
//   window to be in 3d mode.
//
//   Brad Whitlock, Tue Jan 27 16:52:40 PST 2004
//   Changed for multiple time sliders.
//
// ****************************************************************************

void
ViewerSubject::ReplaceDatabase()
{
    debug4 << "ReplaceDatabase: db=" << viewerRPC.GetDatabase().c_str()
           << ", time=" << viewerRPC.GetIntArg1() << endl;

    //
    // If the replace is merely changing the timestate, then turn on
    // view limit merging.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerWindow *win = wM->GetActiveWindow();
    if(win == 0)
        return;

    if (viewerRPC.GetDatabase() == win->GetPlotList()->GetHostDatabaseName())
        win->SetMergeViewLimits(true);

    //
    // First open the database.
    //
    OpenDatabaseHelper(viewerRPC.GetDatabase(), viewerRPC.GetIntArg1(),
                       false);

    //
    // Now perform the database replacement.
    //
    ViewerPlotList *plotList = win->GetPlotList();
    plotList->ReplaceDatabase(plotList->GetHostName(),
                              plotList->GetDatabaseName(),
                              viewerRPC.GetIntArg1(),
                              true,
                              false);

    //
    // We have to send back the source and the time sliders since we
    // could have replaced at a later time state.
    //
    wM->UpdateWindowInformation(WINDOWINFO_SOURCE | WINDOWINFO_TIMESLIDERS);

    //
    // Recenter the active window's view and redraw.
    //
    if(!win->GetMaintainViewMode() && (win->GetWindowMode() == WINMODE_3D))
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
//   Brad Whitlock, Tue Jan 27 16:56:46 PST 2004
//   Changed for multiple time sliders.
//
//   Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//   Added an engine key used to index (and restart) engines.
//
// ****************************************************************************

void
ViewerSubject::OverlayDatabase()
{
    //
    // First open the database.
    //
    OpenDatabaseHelper(viewerRPC.GetDatabase(), 0, false);

    //
    // Now perform the database replacement.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();
    plotList->OverlayDatabase(plotList->GetEngineKey(),
                              plotList->GetHostName(),
                              plotList->GetDatabaseName());

    //
    // Recenter the active window's view and redraw.
    //
    wM->RecenterView();
}

// ****************************************************************************
// Method: ViewerSubject::CloseDatabase
//
// Purpose: 
//   Tell the viewer window manager to try and close the specified database.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 12:04:52 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CloseDatabase()
{
    ViewerWindowManager::Instance()->CloseDatabase(viewerRPC.GetDatabase());
}

// ****************************************************************************
// Method: ViewerSubject::CreateDatabaseCorrelation
//
// Purpose: 
//   Creates a new database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:52:00 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::CreateDatabaseCorrelation()
{
    const std::string &name = viewerRPC.GetDatabase();

    //
    // Make sure that the correlation does not have the same name as
    // an existing source.
    //
    if(ViewerFileServer::Instance()->IsDatabase(name))
    {
        std::string err("You cannot define a database correlation that "
                        "has the same name as a source. No database "
                        "correlation will be created for ");
        err += name;
        err += ".";
        Error(err.c_str());
    }
    else
    {
        ViewerWindowManager::Instance()->CreateDatabaseCorrelation(
            name, viewerRPC.GetProgramOptions(),
            viewerRPC.GetIntArg1(), 0, viewerRPC.GetIntArg2());
    }
}

// ****************************************************************************
// Method: ViewerSubject::AlterDatabaseCorrelation
//
// Purpose: 
//   Alters a database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:51:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::AlterDatabaseCorrelation()
{
    // Alter the database correlation and update all of the windows that
    // used it.
    ViewerWindowManager::Instance()->AlterDatabaseCorrelation(
        viewerRPC.GetDatabase(), viewerRPC.GetProgramOptions(),
        viewerRPC.GetIntArg1(), viewerRPC.GetIntArg2());
}

// ****************************************************************************
// Method: ViewerSubject::DeleteDatabaseCorrelation
//
// Purpose: 
//   Deletes a database correlation.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 23:50:47 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DeleteDatabaseCorrelation()
{
    // Delete the database correlation and update the windows that used it.
    const std::string &name = viewerRPC.GetDatabase();
    ViewerWindowManager::Instance()->DeleteDatabaseCorrelation(name);
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
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//
//    Jeremy Meredith, Fri Apr  2 14:14:54 PST 2004
//    Made it re-use command line arguments if we had some, as long as 
//    we were in nowin mode and no explicit arguments were given.
//
// ****************************************************************************

void
ViewerSubject::OpenComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const string       &hostName = viewerRPC.GetProgramHost();
    const stringVector &options  = viewerRPC.GetProgramOptions();

    //
    // Perform the rpc.
    //
    bool givenOptions = (options.size() > 0);
    bool givenCLArgs  = (engineParallelArguments.size() > 0);

    if (givenOptions)
        ViewerEngineManager::Instance()->CreateEngine(EngineKey(hostName,""),
                                                      options, true,
                                                      numEngineRestarts);
    else if (nowin && givenCLArgs)
        ViewerEngineManager::Instance()->CreateEngine(EngineKey(hostName,""),
                                                      engineParallelArguments,
                                                      true, numEngineRestarts);
    else
        ViewerEngineManager::Instance()->CreateEngine(EngineKey(hostName,""),
                                                      options, false,
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
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//    This was needed for simulation support.
//   
// ****************************************************************************

void
ViewerSubject::CloseComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const string &hostName = viewerRPC.GetProgramHost();
    const string &simName  = viewerRPC.GetProgramSim();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->CloseEngine(EngineKey(hostName, simName));
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
//    Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//    Added an engine key used to index (and restart) engines.
//    This was needed for simulation support.
//   
// ****************************************************************************

void
ViewerSubject::ClearCache()
{
    //
    // Get the rpc arguments.
    //
    const std::string &hostName = viewerRPC.GetProgramHost();
    const std::string &simName  = viewerRPC.GetProgramSim();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->ClearCache(EngineKey(hostName, simName));
}

// ****************************************************************************
// Method: ViewerSubject::ClearCacheForAllEngines.
//
// Purpose: 
//   Execute the ClearCache RPC on all engines.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 26 13:33:59 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ClearCacheForAllEngines()
{
    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->ClearCacheForAllEngines();
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
//    Brad Whitlock, Tue Jan 27 16:57:39 PST 2004
//    Changed for multiple time sliders.
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
    wM->GetActiveWindow()->GetPlotList()->SetPlotAtts(plot);
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
//    Brad Whitlock, Tue Jan 27 16:57:57 PST 2004
//    Changed for multiple time sliders.
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
    wM->GetActiveWindow()->GetPlotList()->SetPlotOperatorAtts(oper, apply);
}

// ****************************************************************************
// Method: ViewerSubject::SetViewCurve
//
// Purpose: 
//   Tells the viewer window manager to apply the curve view attributes to the
//   active window.
//
// Programmer: Eric Brugger
// Creation:   August 20, 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetViewCurve()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetViewCurveFromClient();
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
// Method: ViewerSubject::ExportColorTable
//
// Purpose: 
//   Exports the specified color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 17:03:13 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 12:11:51 PDT 2003
//   I changed the code to send back a status message and an error message
//   if the color table could not be exported.
//
// ****************************************************************************

void
ViewerSubject::ExportColorTable()
{
    //
    // Perform the rpc.
    //
    const std::string &ctName = viewerRPC.GetColorTableName();
    std::string msg;
    if(avtColorTables::Instance()->ExportColorTable(ctName, msg))
    {
        // If we successfully exported the color table, msg is set to the
        // name of the color table file that was created. We want to send
        // a status message and a message.
        std::string msg2;
        msg2 = std::string("Color table ") + ctName +
               std::string(" exported to ") + msg;
        Status(msg2.c_str());

        // Tell the user what happened.
        msg2 = std::string("VisIt exported color table \"") + ctName +
           std::string( "\" to the file: ") + msg +
           ". You can share that file with colleagues who want to use your "
           "color table. Simply put the file in their .visit directory, run "
           "VisIt and the color table will appear in their list of color "
           "tables when VisIt starts up.";
        Message(msg2.c_str());
    }
    else
        Error(msg.c_str());
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
// Method: ViewerSubject::ExportEntireState
//
// Purpose: 
//   Exports the viewer's entire state to an XML file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 12:38:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ExportEntireState()
{
    configMgr->ExportEntireState(viewerRPC.GetVariable());
}

// ****************************************************************************
// Method: ViewerSubject::ImportEntireState
//
// Purpose: 
//   Imports the viewer's entire state from an XML file.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 9 12:38:35 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed Jul 30 14:48:56 PST 2003
//   Added another argument to ImportEntireState.
//
//   Brad Whitlock, Mon Aug 25 14:28:00 PST 2003
//   Added the NotifyIfSelected method call.
//
// ****************************************************************************

void
ViewerSubject::ImportEntireState()
{
     configMgr->ImportEntireState(viewerRPC.GetVariable(),
                                  viewerRPC.GetBoolFlag());
     configMgr->NotifyIfSelected();
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
// Method: ViewerSubject::AddAnnotationObject
//
// Purpose: 
//   Handles the AddAnnotationObject RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::AddAnnotationObject()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->AddAnnotationObject(viewerRPC.GetIntArg1());
}

// ****************************************************************************
// Method: ViewerSubject::HideActiveAnnotationObjects
//
// Purpose: 
//   Handles the HideActiveAnnotationObjects RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::HideActiveAnnotationObjects()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->HideActiveAnnotationObjects();
}

// ****************************************************************************
// Method: ViewerSubject::DeleteActiveAnnotationObjects
//
// Purpose: 
//   Handles the DeleteActiveAnnotationObjects RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::DeleteActiveAnnotationObjects()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->DeleteActiveAnnotationObjects();
}

// ****************************************************************************
// Method: ViewerSubject::RaiseActiveAnnotationObjects
//
// Purpose: 
//   Handles the RaiseActiveAnnotationObjects RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::RaiseActiveAnnotationObjects()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->RaiseActiveAnnotationObjects();
}

// ****************************************************************************
// Method: ViewerSubject::LowerActiveAnnotationObjects
//
// Purpose: 
//   Handles the LowerActiveAnnotationObjects RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::LowerActiveAnnotationObjects()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->LowerActiveAnnotationObjects();
}

// ****************************************************************************
// Method: ViewerSubject::SetAnnotationObjectOptions
//
// Purpose: 
//   Handles the SetAnnotationObjectOptions RPC.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 29 11:10:40 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetAnnotationObjectOptions()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetAnnotationObjectOptions();
}

// ****************************************************************************
// Method: ViewerSubject::SetDefaultAnnotationObjectList
//
// Purpose: 
//   Copies the client annotation object list into the default annotation
//   object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:22:31 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultAnnotationObjectList()
{
    ViewerWindowManager::SetDefaultAnnotationObjectListFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetAnnotationObjectList
//
// Purpose: 
//   Handles the ResetAnnotationObjectList RPC by copying deleting the
//   annotation objects in the active window and adding new annotation objects
//   based on the default annotation object list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 7 14:21:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetAnnotationObjectList()
{
    ViewerWindowManager *wMgr = ViewerWindowManager::Instance();
    ViewerWindow *win = wMgr->GetActiveWindow();

    if(win != 0)
    {
        win->DeleteAllAnnotationObjects();
        win->CreateAnnotationObjectsFromList(*wMgr->GetDefaultAnnotationObjectList());
        wMgr->UpdateAnnotationObjectList();
    }
}

// ****************************************************************************
// Method: ViewerSubject::ResetPickAttributes
//
// Purpose: 
//   Resets pick attributes to default values. 
//
// Programmer: Kathleen Bonnell 
// Creation:   November 26, 2003 
//
// Modifications:
//   Kathleen Bonnell, Wed Dec 17 14:44:26 PST 2003
//   Changed call from ResetPickAtts to SetPickAttsFromDefault.
//   
// ****************************************************************************

void
ViewerSubject::ResetPickAttributes()
{
    ViewerQueryManager::Instance()->SetPickAttsFromDefault(); 
}


// ****************************************************************************
// Method: ViewerSubject::ResetPickLetter
//
// Purpose: 
//   Resets pick letter to default values. 
//
// Programmer: Kathleen Bonnell 
// Creation:   December 9, 2003 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetPickLetter()
{
    ViewerQueryManager::Instance()->ResetPickLetter(); 
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
        if (sscanf(area, "%dx%d+%d+%d", &w, &h, &x, &y) == 4)
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
//   Brad Whitlock, Mon Oct 6 11:52:06 PDT 2003
//   Added code to write the arguments to the debug log.
//
// ****************************************************************************

void
ViewerSubject::ConnectToMetaDataServer()
{
    int timeid = visitTimer->StartTimer();

    //
    // Write the arguments to the debug logs
    //
    debug4 << "Telling mdserver on host "
           << viewerRPC.GetProgramHost().c_str()
           << " to connect to another client." << endl;
    debug4 << "Arguments:" << endl;
    const stringVector &sv = viewerRPC.GetProgramOptions();
    for(int i = 0; i < sv.size(); ++i)
         debug4 << "\t" << sv[i].c_str() << endl;

    //
    // Tell the viewer's fileserver to have its mdserver running on 
    // the specified host to connect to another process.
    //
    ViewerFileServer::Instance()->ConnectServer(
        viewerRPC.GetProgramHost(),
        viewerRPC.GetProgramOptions()); 

    visitTimer->StopTimer(timeid, "Time spent telling mdserver to connect to client.");

    //
    // Do heavy initialization if we still need to do it.
    //
    HeavyInitialization();
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
//   Brad Whitlock, Tue Jan 27 17:30:53 PST 2004
//   I made it use renamed the copy method that it uses.
//
// ****************************************************************************

void
ViewerSubject::CopyPlotsToWindow(int from, int to)
{
    ViewerWindowManager::Instance()->CopyPlotListToWindow(from-1, to-1);
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
//   Kathleen Bonnell, Wed Jul 23 16:10:41 PDT 2003
//   Added int args to qm->DatabaseQuery. 
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
    qm->DatabaseQuery(vw, viewerRPC.GetQueryName(), viewerRPC.GetQueryVariables(),
                      viewerRPC.GetBoolFlag(), 
                      viewerRPC.GetIntArg1(), viewerRPC.GetIntArg2());

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
//   Kathleen Bonnell, Wed Nov 26 14:33:23 PST 2003
//   Use optional int args from RPC. 
//
//   Kathleen Bonnell, Thu Apr  1 19:13:59 PST 2004 
//   Use optional bool flag from RPC. 
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
                   viewerRPC.GetQueryVariables(),
                   viewerRPC.GetIntArg1(), viewerRPC.GetIntArg2(),
                   viewerRPC.GetBoolFlag()); 

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
//   Kathleen Bonnell, Fri Dec 20 11:36:11 PST 2002 
//   ViewerQueryManager now handles the line queries.
//
//   Kathleen Bonnell, Wed Jul 23 16:10:41 PDT 2003
//   Added IntArg1 to qm->LineQuery.
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
              viewerRPC.GetQueryVariables(), viewerRPC.GetIntArg1());

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

    if (interruptionEnabled)
    {
        // See if the connection needs to be read.
        if (xfer.GetInputConnection()->NeedsRead())
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
        if (retval)
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
//  Modifications:
//    Brad Whitlock, Thu Dec 18 12:26:10 PDT 2003
//    I changed this method so it schedules itself to be called again from
//    the event loop if the engine is executing. I also added code to prevent
//    it from doing anything if it is called via indirect recursion since that
//    is bad for this function.
//
// ****************************************************************************

void
ViewerSubject::ProcessFromParent()
{
    if(ViewerEngineManager::Instance()->InExecute())
    {
        debug1 << "The viewer engine manager is busy processing a request "
                  "so we should not process input from the client. Let's "
                  "reschedule this method to run again later" << endl;
        QTimer::singleShot(200, this, SLOT(ProcessFromParent()));
    }
    else if(processingFromParent)
    {
        debug1 << "The viewer tried to recursively enter "
                  "ViewerSubject::ProcessFromParent!" << endl;
    }
    else
    {
        TRY
        {
            // Process the input from the client.
            processingFromParent = true;
            xfer.Process();
            processingFromParent = false;
        }
        CATCH(VisItException)
        {
            processingFromParent = false;
            RETHROW;
        }
        ENDTRY
    }
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
//    Jeremy Meredith, Mon Aug 18 13:08:40 PDT 2003
//    Changed the return to a CATCH_RETURN now that it's inside a TRY block.
//
//    Brad Whitlock, Thu Dec 18 13:12:15 PST 2003
//    I changed the method so it calls ProcessFromParent instead of calling
//    Xfer::Process directly so we can get some protection from this method
//    getting called in the middle of an engine execute.
//
// ****************************************************************************

void
ViewerSubject::ReadFromParentAndProcess(int)
{
    TRY
    {
        if (blockSocketSignals)
        {
            CATCH_RETURN(1);
        }

        int amountRead = xfer.GetInputConnection()->Fill();

        //
        // Try and process the input.
        //
        if (amountRead > 0)
            ProcessFromParent();
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
//    Brad Whitlock, Fri Jul 18 12:20:06 PDT 2003
//    I added code to handle updateFrame, setInteractiveMode, and
//    activateTool. I also made the routine reschedule itself with the event
//    loop and return early if the engine is executing or if we're launching
//    a component.
//
//    Brad Whitlock, Tue Jan 27 17:01:50 PST 2004
//    Added support for multiple time sliders.
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
        //
        // If the engine manager is executing, return early but tell the
        // event loop to try to process the message again later.
        //
        else if(ViewerEngineManager::Instance()->InExecute() ||
                launchingComponent)
        {
            // Add the message back into the buffer.
            messageBuffer->AddString(msg);
            QTimer::singleShot(400, this, SLOT(ProcessRendererMessage()));
            return;
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
        else if (strncmp(msg, "activateTool", 12) == 0)
        {
            ViewerWindow *window = 0;
            int toolId = 0;
            int offset = 15;  // = strlen("activateTool 0x");
            sscanf (&msg[offset], "%p %d", &window, &toolId);

            // Tell the viewer window manager to delete the window.
            window->SetToolEnabled(toolId, true);
            ViewerWindowManager::Instance()->UpdateActions();
        }
        else if (strncmp(msg, "setInteractionMode", 18) == 0)
        {
            ViewerWindow *window = 0;
            int windowMode = 0;
            int offset = 21;  // = strlen("setInteractionMode 0x");
            sscanf (&msg[offset], "%p %d", &window, &windowMode);

            // Tell the window to set its interaction mode.
            window->SetInteractionMode(INTERACTION_MODE(windowMode));
            ViewerWindowManager::Instance()->UpdateActions();
        }
        else if (strncmp(msg, "updateFrame", 11) == 0)
        {
            ViewerWindow *window = 0;

            int offset = 14;  // = strlen("updateFrame 0x");
            sscanf (&msg[offset], "%p", &window);

            // Tell the window's animation to update.
            window->GetPlotList()->UpdateFrame();
            ViewerWindowManager::Instance()->UpdateActions();
        }
        else if (strncmp(msg, "setScalableRenderingMode", 24) == 0)
        {
            ViewerWindow *window = 0;
            int iMode = 0;
            int offset = 27;  // = strlen("setScalableRenderingMode 0x");
            sscanf (&msg[offset], "%p %d", &window, &iMode);
            bool newMode = (iMode==1?true:false);

            // Tell the window to change scalable rendering modes, if necessary 
            if (window->GetScalableRendering() != newMode)
                window->ChangeScalableRenderingMode(newMode);
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
//   Brad Whitlock, Wed Jul 23 13:51:04 PST 2003
//   Set the launchingComponent flag.
//
// ****************************************************************************

void
ViewerSubject::StartLaunchProgress()
{
    launchingComponent = true;
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
//   Brad Whitlock, Wed Jul 23 13:51:04 PST 2003
//   Set the launchingComponent flag.
//   
// ****************************************************************************

void
ViewerSubject::EndLaunchProgress()
{
    launchingComponent = false;
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
//   Brad Whitlock, Thu Oct 16 17:53:46 PST 2003
//   Prevented the hasPendingEvents method call from being made on MacOS X
//   since it was causing the Viewer to block waiting for events.
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

    if (stage == 0)
    {
        This->StartLaunchProgress();
        if (windowsShowing)
        {
            dialog->show();
            retval = !dialog->getCancelled();
        }
    }
    else if (stage == 1)
    {
        if (windowsShowing)
        {
#if QT_VERSION >= 300 && !defined(Q_WS_MACX)
            if (qApp->hasPendingEvents())
#endif
                qApp->processOneEvent();
            retval = !dialog->getCancelled();
        }
    }
    else if (stage == 2)
    {
        This->EndLaunchProgress();
        if (windowsShowing) 
        {
            if(!dialog->getIgnoreHide())
                dialog->hide();
            retval = !dialog->getCancelled();
        }
    }

    return retval;
}

// ****************************************************************************
// Method: ViewerSubject::SendKeepAlives
//
// Purpose: 
//   This is a Qt slot function that sends keep alive signals to all of the
//   remote components.
//
// Note:       If we're launching a remote component or if the engine is
//             executing then we don't send the keep alives. Instead, we
//             reschedule the routine to run again in 20 seconds.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 12:17:02 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SendKeepAlives()
{
    if(launchingComponent || ViewerEngineManager::Instance()->InExecute())
    {
        // We're launching a component so we don't want to send keep alive
        // signals right now but try again in 20 seconds.
        QTimer::singleShot(20 * 1000, this, SLOT(SendKeepAlives()));
    }
    else
    {
        Status("Sending keep alive signals...");
        ViewerFileServer::Instance()->SendKeepAlives();
        ViewerEngineManager::Instance()->SendKeepAlives();
        ViewerServerManager::SendKeepAlivesToLaunchers();
        ClearStatus();
    }
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
//    Eric Brugger, Fri Apr 18 12:46:00 PDT 2003
//    I replaced auto center view with maintain view.
//
//    Brad Whitlock, Mon Jun 23 16:22:51 PST 2003
//    I removed ClearWindow, ClearAllWindows, ClearRefLines, ClearPickPoints
//    since they are now actions.
//
//    Kathleen Bonnell, Tue Jul  1 09:21:57 PDT 2003 
//    Added SetPickAttributes. 
//
//    Brad Whitlock, Tue Jul 1 17:00:30 PST 2003
//    Added ExportColorTable.
//
//    Brad Whitlock, Wed Jul 9 12:35:44 PDT 2003
//    Added ExportEntireState and ImportEntireState.
//
//    Eric Brugger, Wed Aug 20 11:11:00 PDT 2003
//    I added SetViewCurve.
//
//    Kathleen Bonnell, Wed Nov 26 14:33:23 PST 2003 
//    Added ResetPickAttributesRPC.
//
//    Brad Whitlock, Wed Oct 29 11:06:48 PDT 2003
//    I added several new RPCs for handling annotations.
//
//    Kathleen Bonnell, Wed Dec 17 14:45:22 PST 2003 
//    Added ResetPickLetterRPC, SetDefaultPickAttributesRPC.
//
//    Brad Whitlock, Thu Jan 29 23:49:03 PST 2004
//    Added ActivateSource, CheckForNewStates, CreateDatabaseCorrelation,
//    AlterDatabaseCorrelation, DeleteDatabaseCorrelation, and CloseDatabase.
//
//    Brad Whitlock, Thu Feb 26 13:32:43 PST 2004
//    Added ClearCacheForAllEngines.
//
//    Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//    Added methods related to QueryOverTimeAttributes.
//
// ****************************************************************************

void
ViewerSubject::HandleViewerRPC()
{
    //
    // Get a pointer to the active window's action manager.
    //
    bool actionHandled = false;
    ViewerActionManager *actionMgr = 0;

    debug5 << "Handling "
           << ViewerRPC::ViewerRPCType_ToString(viewerRPC.GetRPCType())
           << " RPC." << endl;

    //
    // Handle the RPC. Note that these should be replaced with actions.
    //
    switch(viewerRPC.GetRPCType())
    {
    case ViewerRPC::CloseRPC:
        Close();
        break;
    case ViewerRPC::OpenDatabaseRPC:
        OpenDatabase();
        break;
    case ViewerRPC::CloseDatabaseRPC:
        CloseDatabase();
        break;
    case ViewerRPC::ActivateDatabaseRPC:
        ActivateDatabase();
        break;
    case ViewerRPC::CheckForNewStatesRPC:
        CheckForNewStates();
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
    case ViewerRPC::CreateDatabaseCorrelationRPC:
        CreateDatabaseCorrelation();
        break;
    case ViewerRPC::AlterDatabaseCorrelationRPC:
        AlterDatabaseCorrelation();
        break;
    case ViewerRPC::DeleteDatabaseCorrelationRPC:
        DeleteDatabaseCorrelation();
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
    case ViewerRPC::SetViewCurveRPC:
        SetViewCurve();
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
    case ViewerRPC::ClearCacheForAllEnginesRPC:
        ClearCacheForAllEngines();
        break;
    case ViewerRPC::SetViewExtentsTypeRPC:
        SetViewExtentsType();
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
    case ViewerRPC::SetPickAttributesRPC:
        SetPickAttributes();
        break;
    case ViewerRPC::ExportColorTableRPC:
        ExportColorTable();
        break;
    case ViewerRPC::ExportEntireStateRPC:
        ExportEntireState();
        break;
    case ViewerRPC::ImportEntireStateRPC:
        ImportEntireState();
        break;
    case ViewerRPC::ResetPickAttributesRPC:
        ResetPickAttributes();
        break;
    case ViewerRPC::AddAnnotationObjectRPC:
        AddAnnotationObject();
        break;
    case ViewerRPC::HideActiveAnnotationObjectsRPC:
        HideActiveAnnotationObjects();
        break;
    case ViewerRPC::DeleteActiveAnnotationObjectsRPC:
        DeleteActiveAnnotationObjects();
        break;
    case ViewerRPC::RaiseActiveAnnotationObjectsRPC:
        RaiseActiveAnnotationObjects();
        break;
    case ViewerRPC::LowerActiveAnnotationObjectsRPC:
        LowerActiveAnnotationObjects();
        break;
    case ViewerRPC::SetAnnotationObjectOptionsRPC:
        SetAnnotationObjectOptions();
        break;
    case ViewerRPC::SetDefaultAnnotationObjectListRPC:
        SetDefaultAnnotationObjectList();
        break;
    case ViewerRPC::ResetAnnotationObjectListRPC:
        ResetAnnotationObjectList();
        break;
    case ViewerRPC::ResetPickLetterRPC:
        ResetPickLetter();
        break;
    case ViewerRPC::SetDefaultPickAttributesRPC:
        SetDefaultPickAttributes();
        break;
    case ViewerRPC::SetQueryOverTimeAttributesRPC:
        SetQueryOverTimeAttributes();
        break;
    case ViewerRPC::SetDefaultQueryOverTimeAttributesRPC:
        SetDefaultQueryOverTimeAttributes();
        break;
    case ViewerRPC::ResetQueryOverTimeAttributesRPC:
        ResetQueryOverTimeAttributes();
        break;
    case ViewerRPC::MaxRPC:
        break;
    default:
        // If an RPC is not handled in the above cases, handle it as
        // an action.
        actionMgr = ViewerWindowManager::Instance()->
            GetActiveWindow()->GetActionManager();
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


// ****************************************************************************
//  Method: ViewerSubject::SetPickAttributes
//
//  Purpose:
//    Execute the SetPickAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   June 30, 2003 
//
// ****************************************************************************

void
ViewerSubject::SetPickAttributes()
{
    ViewerQueryManager::Instance()->SetPickAttsFromClient();
}

// ****************************************************************************
//  Method: ViewerSubject::SetDefaultPickAttributes
//
//  Purpose:
//    Execute the SetDefaultPickAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   December 9, 2003 
//
// ****************************************************************************

void
ViewerSubject::SetDefaultPickAttributes()
{
    ViewerQueryManager::Instance()->SetDefaultPickAttsFromClient();
}

// ****************************************************************************
//  Function:  SplitValues
//
//  Purpose:
//    Separate a string into a vector of strings using a single char delimiter.
//
//  Arguments:
//    buff       the string to split
//    delim      the single-character delimiter
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 23, 2004
//
// ****************************************************************************
static vector<string>
SplitValues(const string &buff, char delim)
{
    vector<string> output;
    
    string tmp="";
    for (int i=0; i<buff.length(); i++)
    {
        if (buff[i] == delim)
        {
            if (!tmp.empty())
                output.push_back(tmp);
            tmp = "";
        }
        else
        {
            tmp += buff[i];
        }
    }
    if (!tmp.empty())
        output.push_back(tmp);

    return output;
}


// ****************************************************************************
//  Method: ViewerSubject::SetQueryOverTimeAttributes
//
//  Purpose:
//    Execute the SetQueryOverTimeAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerSubject::SetQueryOverTimeAttributes()
{
    ViewerQueryManager::Instance()->SetQueryOverTimeAttsFromClient();
}


// ****************************************************************************
//  Method: ViewerSubject::SetDefaultQueryOverTimeAttributes
//
//  Purpose:
//    Execute the SetDefaultQueryOverTimeAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   March 24, 2004 
//
// ****************************************************************************

void
ViewerSubject::SetDefaultQueryOverTimeAttributes()
{
    ViewerQueryManager::Instance()->SetDefaultQueryOverTimeAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetQueryOverTimeAttributes
//
// Purpose: 
//   Resets time query attributes to default values. 
//
// Programmer: Kathleen Bonnell 
// Creation:   March 24, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetQueryOverTimeAttributes()
{
    ViewerQueryManager::Instance()->SetQueryOverTimeAttsFromDefault(); 
}
