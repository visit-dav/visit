#include <stdio.h>
#include <algorithm>
#include <map>

#include <visit-config.h> // To get the version number
#include <qcolor.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qsocketnotifier.h>
#include <qstatusbar.h>

#if QT_VERSION < 300
// If we're not using Qt 3.0, then include style headers.
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>
#if QT_VERSION >= 230
#include <qsgistyle.h>
#endif
#endif

#include <qtimer.h>
#include <QvisGUIApplication.h>

#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>

#include <ChangeDirectoryException.h>
#include <GetFileListException.h>
#include <SaveWindowAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <HostProfile.h>
#include <GlobalLineoutAttributes.h>
#include <ObserverToCallback.h>
#include <PickAttributes.h>
#include <QueryAttributes.h>
#include <PlotList.h>
#include <Plot.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SyncAttributes.h>
#include <WindowInformation.h>

#include <QvisApplication.h>
#include <FileServerList.h>
#include <QvisAnimationWindow.h>
#include <QvisAnnotationWindow.h>
#include <QvisAppearanceWindow.h>
#include <QvisColorTableWindow.h>
#include <QvisCommandLineWindow.h>
#include <QvisEngineWindow.h>
#include <QvisExpressionsWindow.h>
#include <QvisFileInformationWindow.h>
#include <QvisFileSelectionWindow.h>
#include <QvisGlobalLineoutWindow.h>
#include <QvisHelpWindow.h>
#include <QvisHostProfileWindow.h>
#include <QvisKeyframeWindow.h>
#include <QvisLightingWindow.h>
#include <QvisMainWindow.h>
#include <QvisMaterialWindow.h>
#include <QvisMessageWindow.h>
#include <QvisOutputWindow.h>
#include <QvisPickWindow.h>
#include <QvisPlotManagerWidget.h>
#include <QvisPluginWindow.h>
#include <QvisPreferencesWindow.h>
#include <QvisQueryWindow.h>
#include <QvisRenderingWindow.h>
#include <QvisSaveWindow.h>
#include <QvisSubsetWindow.h>
#include <QvisViewWindow.h>

#include <SplashScreen.h>
#include <WindowMetrics.h>

#include <BadHostException.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <LostConnectionException.h>
#include <TimingsManager.h>
#include <DebugStream.h>

#if defined(_WIN32)
#include <windows.h> // for LoadLibrary
#endif

#include <snprintf.h>

// Some defines
#define VISIT_GUI_CONFIG_FILE "guiconfig"
#define VIEWER_READY_TAG      100

// Some internal prototypes.
static void QPrinterToPrinterAttributes(QPrinter *, PrinterAttributes *);
static void PrinterAttributesToQPrinter(PrinterAttributes *, QPrinter *);

#if defined(_WIN32)
// ****************************************************************************
// Function: LongFileName
//
// Purpose: 
//   Converts a Windows short filename into a long filename.
//
// Arguments:
//   shortName : The short Windows name of the file.
//
// Returns:    The long windows name of the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 11 18:39:36 PST 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
LongFileName(const char *shortName)
{
    std::string retval(shortName);

    // Get the long filename for the file. We have to access
    // the GetLongPathName through the LoadLibrary call because
    // it is not supported in NT 4 and we need it to not try
    // to use the function if it does not exist.
    HINSTANCE lib = LoadLibrary("kernel32.dll");
    if(lib)
    {
        // Try and get a pointer to the GetLongPathName function.
        void *func = (void *)GetProcAddress((HMODULE)lib, "GetLongPathNameA");
        // Use the GetLongPathName function.
        if(func)
        {
            typedef DWORD (pathFuncType)(LPCTSTR, LPTSTR, DWORD);
            pathFuncType *lfn = (pathFuncType *)func;
            char *buf = new char[1000];
            if(lfn(shortName, buf, 1000) != 0)
                retval = std::string(buf);
            delete [] buf;
        }

        // Free the library.
        FreeLibrary(lib);
    }

    return retval;
}
#endif

// ****************************************************************************
// Method: QvisGUIApplication::QvisGUIApplication
//
// Purpose: 
//   Constructor for the QvisGUIApplication class.
//
// Arguments:
//   argc : The number of command line arguments.
//   argv : The command line arguments.
//
// Note:       
//   This method creates the FileServer, the Viewer, and all of the
//   windows for the GUI. Both the FileServer and the Viewer launch
//   remote processes so they could take a little while.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:14:42 PST 2000
//
// Modifications:
//   Eric Brugger, Wed Oct 25 14:44:58 PDT 2000
//   Modified the routine to match a change to the ViewerProxy Create
//   method.
//
//   Brad Whitlock, Wed Nov 22 15:26:18 PST 2000
//   Changed how the MDServer gets launched.
//
//   Brad Whitlock, Mon Nov 27 16:00:44 PST 2000
//   Changed the code so command line arguments not consumed by the GUI are 
//   passed on to the viewer.
//
//   Brad Whitlock, Thu Mar 29 14:44:34 PST 2001
//   Initialized the localOnly flag.
//
//   Brad Whitlock, Wed Apr 25 12:18:24 PDT 2001
//   Added code to handle IncompatibleVersionExceptions.
//
//   Jeremy Meredith, Fri Jul 20 11:07:43 PDT 2001
//   Added code to handle window managers correctly other than SGI's 4DWM.
//   It now also works correctly under enlightenment, KDE1/2's kwm, twm,
//   fvwm, fvwm2.  Also moved the "VisIt is Ready" message to the end of
//   this constructor.
//
//   Jeremy Meredith, Fri Jul 20 13:42:33 PDT 2001
//   Added code to calculate screen width,height,top,left to avoid toolbars
//   (panels) in desktop environments.
//
//   Jeremy Meredith, Mon Jul 23 16:45:13 PDT 2001
//   Added initialization of smallScreen.
//
//   Sean Ahern, Fri Aug 31 09:55:29 PDT 2001
//   Added the splash screen.
//
//   Brad Whitlock, Tue Sep 4 09:12:54 PDT 2001
//   Removed a hack.
//
//   Jeremy Meredith, Tue Sep  4 14:03:45 PDT 2001
//   Moved setting of default values back up above where they are
//   changed by ProcessArguments.
//
//   Brad Whitlock, Thu Sep 6 11:52:10 PDT 2001
//   Modified ordering to allow default appearance values from a config file.
//
//   Jeremy Meredith, Thu Sep 13 16:33:51 PDT 2001
//   Changed default border values to match the only WM I have access to
//   which does not work with our border size detection algorithm.
//
//   Jeremy Meredith, Fri Sep 14 13:54:39 PDT 2001
//   Added calculation of a preshift.  This assumes the main window
//   starts positioned at 100,100 before it is realized.
//
//   Jeremy Meredith, Tue Sep 25 15:24:00 PDT 2001
//   Made use of WindowMetrics to calculate borders, desktop, and shifts.
//
//   Jeremy Meredith, Tue Sep 25 18:02:51 PDT 2001
//   Fixed the progress updates so they are monotonically increasing again.
//
//   Brad Whitlock, Wed Sep 26 13:39:05 PST 2001
//   Initialized the configSettings pointer to NULL so we don't crash the
//   GUI if the config file cannot be read.
//
//   Brad Whitlock, Tue Jan 29 11:51:10 PDT 2002
//   Changed some method calls.
//
//   Jeremy Meredith, Fri Feb  1 16:14:26 PST 2002
//   Made it save off the arguments passed to QT since it doesn't use them
//   till later in QT's initialization.
//
//   Brad Whitlock, Wed Feb 6 16:03:02 PST 2002
//   Added code to set the wait cursor until VisIt is done initializing.
//
//   Brad Whitlock, Tue Feb 19 12:54:48 PDT 2002
//   Added code that allows initialization using system wide settings.
//
//   Brad Whitlock, Wed Feb 20 13:43:00 PST 2002
//   Added support for printing.
//
//   Brad Whitlock, Mon Mar 11 12:09:54 PDT 2002
//   Upgraded to Qt 3.0.
//
//   Brad Whitlock, Thu Apr 11 15:24:28 PST 2002
//   Added code to set the default host profile name.
//
//   Brad Whitlock, Thu May 9 14:38:30 PST 2002
//   Added initialization of loadFile member and made fileServer a pointer.
//
//   Brad Whitlock, Thu Jun 27 14:11:23 PST 2002
//   I added code to fill in the path of the loadFile variable if it does
//   not contain a path.
//
//   Jeremy Meredith, Fri Jul  5 18:08:25 PDT 2002
//   I made it look for a "guiconfig" file in the ".visit" directory 
//   instead of a ".visitguiconfig" file.
//
//   Brad Whitlock, Wed Sep 25 14:52:56 PST 2002
//   I split up the config file processing and made it honor the -noconfig flag.
//
//   Brad Whitlock, Tue Apr 1 08:07:29 PDT 2003
//   I made the -foreground, -background, and -style arguments supercede the
//   values stored in the appearance attributes in the config file.
//
//   Brad Whitlock, Thu Sep 4 10:26:54 PDT 2003
//   I made it use QvisApplication.
//
//   Brad Whitlock, Mon Nov 10 14:53:42 PST 2003
//   I initialized sessionFile.
//
// ****************************************************************************

QvisGUIApplication::QvisGUIApplication(int &argc, char **argv) :
    ConfigManager(), GUIBase(), message(), plotWindows(),
    operatorWindows(), otherWindows(), foregroundColor(), backgroundColor(),
    applicationStyle(), loadFile(), sessionFile()
{
    completeInit = visitTimer->StartTimer();
    int total = visitTimer->StartTimer();

    // Tell Qt that we want lots of colors.
    QApplication::setColorSpec(QApplication::ManyColor);

    // The viewer is initially not alive.
    viewerIsAlive = false;

    // Default border values.
    borders[0] = 26; // Top
    borders[1] = 4;  // Bottom
    borders[2] = 4;  // Left
    borders[3] = 4;  // Right
    // Default shift values.
    shiftX = 0;
    shiftY = 0;
    // Default preshift values.
    preshiftX = 0;
    preshiftY = 0;
    // Default values.
    localOnly = false;
    readConfig = true;
    sessionCount = 0;
    initStage = 0;
    heavyInitStage = 0;
    windowInitStage = 0;
    systemSettings = 0;
    localSettings = 0;
    printer = 0;
    printerObserver = 0;
    showSplash = true;
    fromViewer = 0;
    allowSocketRead = false;

    // Create the viewer, statusSubject, and fileServer for GUIBase.
    viewer = new ViewerProxy;
    statusSubject = new StatusSubject;
    fileServer = new FileServerList;

    // Process any GUI arguments that should not be passed on to other programs.
    // This also has the effect of setting color/style attributes. This must
    // be done before the application is created since Qt likes to strip off
    // certain arguments that the viewer also needs to be the right color, etc.
    ProcessArguments(argc, argv);

    int timeid = visitTimer->StartTimer();
    // Read the system config file.
    char *configFile = GetSystemConfigFile(VISIT_GUI_CONFIG_FILE);
    systemSettings = readConfig ? ReadConfigFile(configFile) : 0;
    delete [] configFile;
    // Read the local config file.
    configFile = GetDefaultConfigFile(VISIT_GUI_CONFIG_FILE);
    localSettings = readConfig ? ReadConfigFile(configFile) : 0;
    delete [] configFile;
    visitTimer->StopTimer(timeid, "Reading config files");

    // If any appearance command line arguments were given, store the values
    // into the appearance attributes so we can override the values from
    // the config files.
    AppearanceAttributes *aa = viewer->GetAppearanceAttributes();
    if(foregroundColor.length() > 0)
        aa->SetForeground(foregroundColor.latin1());
    if(backgroundColor.length() > 0)
        aa->SetBackground(backgroundColor.latin1());
    if(applicationStyle.length() > 0)
        aa->SetStyle(applicationStyle.latin1());

    // Add left-over arguments to the viewer.
    AddViewerArguments(argc, argv);

    //
    // Create the application and customize its appearance. Note that we
    // are setting the initial font through the command line because Qt
    // does not want to allow the font to change after qApp is created.
    //
    // note: JSM 2/1/02 -- should these all be strdup'd ?
    qt_argv = new char *[argc + 3];
    qt_argc = argc + 2;
    for(int i = 0; i < argc; ++i)
        qt_argv[i] = argv[i];
    qt_argv[argc] = "-font";
    qt_argv[argc+1] = (char*)viewer->GetAppearanceAttributes()->GetFontDescription().c_str();
    qt_argv[argc+2] = NULL;
    mainApp = new QvisApplication(qt_argc, qt_argv);
    SetWaitCursor();
    viewer->GetAppearanceAttributes()->SelectAll();
    CustomizeAppearance(false);

    //
    // Create the splashscreen.
    //
    if(showSplash)
    {
        timeid = visitTimer->StartTimer();

        // Create the splash screen.
        splash = new SplashScreen(false, "splashScreen");
        splash->show();
        visitTimer->StopTimer(timeid, "Creating splashscreen");
    }
    else
        splash = 0;

    //
    // Create an observer for the sync attributes.
    //
    syncObserver = new ObserverToCallback(viewer->GetSyncAttributes(),
         QvisGUIApplication::SyncCallback, this);

    //
    // Start the heavy duty initialization from within the event loop.
    //
    QTimer::singleShot(10, this, SLOT(HeavyInitialization()));

    visitTimer->StopTimer(total, "QvisGUIApplication constuctor");
}

// ****************************************************************************
// Method: QvisGUIApplication::~QvisGUIApplication
//
// Purpose: 
//   Destructor for the QvisGUIApplication class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:16:43 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Nov 27 16:46:25 PST 2000
//   Added code to delete the application.
//
//   Brad Whitlock, Fri Feb 1 14:31:33 PST 2002
//   Added code to delete the socket notifier.
//
//   Brad Whitlock, Wed Feb 20 12:03:56 PDT 2002
//   Added code to delete the printer object.
//
//   Brad Whitlock, Thu May 9 16:37:50 PST 2002
//   Added code to delete the file server.
//
//   Brad Whitlock, Wed Sep 25 10:06:28 PDT 2002
//   Since windows now have parents on the Windows platform, don't delete
//   them or it will cause a crash.
//
// ****************************************************************************

QvisGUIApplication::~QvisGUIApplication()
{
#if !defined(_WIN32)
    // Delete the windows.
    int i;
    for(i = 0; i < otherWindows.size(); ++i)
    {
        delete otherWindows[i];
    }
    for(i = 0; i < plotWindows.size(); ++i)
    {
        delete plotWindows[i];
    }
    for(i = 0; i < operatorWindows.size(); ++i)
    {
        delete operatorWindows[i];
    }
#endif

    // Delete the file server
    delete fileServer;
    fileServer = 0;

    // Close down the viewer and delete it.
    if(viewerIsAlive)
        viewer->Close();
    delete viewer;
    viewer = 0;

    // Delete the status subject that is used for the status bar.
    delete statusSubject;
    statusSubject = 0;

    // Delete the socket notifiers.
    delete fromViewer;

    // Delete the application
    delete mainApp;

    // Delete the args for QT
    delete [] qt_argv;

    // Delete the printer object.
    delete printer;
    delete printerObserver;

    delete syncObserver;
    delete systemSettings;
    delete localSettings;
}

// ****************************************************************************
// Method: QvisGUIApplication::HeavyInitialization
//
// Purpose: 
//   This method performs the heavy initialization for which we want progress
//   to be displayed in the splashscreen. This includes launching the viewer
//   and the mdserver.
//
// Note:       This method is called repeatedly until it is done.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 15:13:16 PST 2003
//
// Modifications:
//   
//   Hank Childs, Sat Jan 24 10:45:05 PST 2004
//   Add a stage where we tell the mdserver to load its plugins.
//
// ****************************************************************************

void
QvisGUIApplication::HeavyInitialization()
{
    int timeid;
    bool moreInit = true;
    bool gotoNextStage = true;
    WindowMetrics *wm = 0;

    debug4 << "QvisGUIApplication::HeavyInitialization: heavyInitStage="
           << heavyInitStage << endl;

    switch(heavyInitStage)
    {
    case 0:
        SplashScreenProgress("Calculating window metrics...", 5);
        // Calculate the window metrics
        timeid = visitTimer->StartTimer();
        wm = WindowMetrics::Instance();
        visitTimer->StopTimer(timeid, "Calculating window metrics");

        // Ise the window metrics to set some internal fields.
        screenX    = wm->GetScreenX();
        screenY    = wm->GetScreenY();
        screenW    = wm->GetScreenW();
        screenH    = wm->GetScreenH();
        borders[0] = wm->GetBorderT();
        borders[1] = wm->GetBorderB();
        borders[2] = wm->GetBorderL();
        borders[3] = wm->GetBorderR();
        shiftX     = wm->GetShiftX();
        shiftY     = wm->GetShiftY();
        preshiftX  = wm->GetPreshiftX();
        preshiftY  = wm->GetPreshiftY();
        break;
    case 1:
        SplashScreenProgress("Creating main window...", 10);
        break;
    case 2:
        // Create the main window.
        CreateMainWindow();
        break;
    case 3:
        SplashScreenProgress("Starting viewer...", 12);
        break;
    case 4:
        // Launch the viewer.
        LaunchViewer();
        break;
    case 5:
        // Create the socket notifier and hook it up to the viewer.
        fromViewer = new QSocketNotifier(
            viewer->GetWriteConnection()->GetDescriptor(),
            QSocketNotifier::Read);
        connect(fromViewer, SIGNAL(activated(int)),
                this, SLOT(ReadFromViewer(int)));

        SplashScreenProgress("Starting metadata server...", 32);
        break;
    case 6:
        // Initialize the file server. This connects the GUI to the mdserver
        // running on localhost.
        timeid = visitTimer->StartTimer();
        fileServer->SetConnectCallback(StartMDServer, (void *)viewer);
        fileServer->Initialize();
        visitTimer->StopTimer(timeid, "Launching mdserver");
        break;
    case 7:
        SplashScreenProgress("Launched the metadata server...", 52);
        break;
    case 8:
        // Set the current directory in the loadFile if its path is empty.
        if(loadFile.host == fileServer->GetHost() &&
           loadFile.path == "")
        {
            SplashScreenProgress("Getting the path...", 55);
            loadFile.path = fileServer->GetPath();
        }
        break;
    case 9:
        // Tell the mdserver to load its plugins.
        fileServer->LoadPlugins();
        break;
    case 10:
        // Create the GUI's windows.
        gotoNextStage = CreateWindows(56, 80);
        break;
    case 11:
        SplashScreenProgress("Loading plugin information...", 81);
        break;
    case 12:
        // Load plugin info
        timeid = visitTimer->StartTimer();
        PlotPluginManager::Initialize(PlotPluginManager::GUI);
        OperatorPluginManager::Initialize(OperatorPluginManager::GUI);
        visitTimer->StopTimer(timeid, "Loading plugin info");
        break;
    case 13:
        SplashScreenProgress("Processing config file...", 90);
        break;
    case 14:
        // Process the config file settings.
        timeid = visitTimer->StartTimer();
        ProcessConfigSettings(systemSettings, true);
        ProcessConfigSettings(localSettings, false);
        visitTimer->StopTimer(timeid, "Processing config file");
    case 15:
        // Let the GUI read from the viewer now.
        allowSocketRead = true;

        // Create a trigger that will cause the GUI to finish initialization
        // when the viewer is ready.
        Synchronize(VIEWER_READY_TAG);
        moreInit = false;
        break;
    default:
        moreInit = false;
    }

    if(moreInit)
    {
        if(gotoNextStage)
           ++heavyInitStage;
        QTimer::singleShot(10, this, SLOT(HeavyInitialization()));
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::LaunchViewer
//
// Purpose: 
//   This method launches the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 15:12:41 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::LaunchViewer()
{
    // Start up the viewer
    int timeid = visitTimer->StartTimer();
    const char * viewerMsg = "Starting viewer...";

    TRY
    {
        // Add some more arguments and launch the viewer.
        AddViewerSpaceArguments();
        viewer->AddArgument("-defer");
        viewer->Create();
        viewerIsAlive = true;
        visitTimer->StopTimer(timeid, viewerMsg);

        // Set the default user name in the host profiles.
        HostProfile::SetDefaultUserName(viewer->GetLocalUserName());
    }
    CATCH(IncompatibleVersionException)
    {
        cerr << "The version numbers of the GUI and the viewer do not match."
             << endl;
        visitTimer->StopTimer(timeid, viewerMsg);
        // Re-throw the exception.
        RETHROW;
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        cerr << "The viewer did not return the proper credentials."
             << endl;
        visitTimer->StopTimer(timeid, viewerMsg);
        // Re-throw the exception.
        RETHROW;
    }
    ENDTRY
}

// ****************************************************************************
// Method: QvisGUIApplication::Synchronize
//
// Purpose: 
//   Sends a synchronization tag to the viewer. When the viewer returns the 
//   tag, it means that it's ready for more input and we can use that to
//   do things in the GUI.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 15:53:25 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::Synchronize(int tag)
{
    // Send a tag to the viewer.
    viewer->GetSyncAttributes()->SetSyncTag(tag);
    syncObserver->SetUpdate(false);
    viewer->GetSyncAttributes()->Notify();
}

// ****************************************************************************
// Method: QvisGUIApplication::HandleSynchronize
//
// Purpose: 
//   Handles a trigger that was echoed back from the viewer.
//
// Arguments:
//   val : The trigger to handle.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 16:12:18 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::HandleSynchronize(int val)
{
    if(val == VIEWER_READY_TAG)
    {
        QTimer::singleShot(10, this, SLOT(FinalInitialization()));
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SyncCallback
//
// Purpose: 
//   This method handles a syncattributes object an converts it into a trigger.
//
// Arguments:
//   s    : The SyncAttributes.
//   data : callback data.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 16:12:59 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::SyncCallback(Subject *s, void *data)
{
    QvisGUIApplication *app = (QvisGUIApplication *)data;
    SyncAttributes *sync = (SyncAttributes *)s;
    app->HandleSynchronize(sync->GetSyncTag());
}

// ****************************************************************************
// Method: QvisGUIApplication::FinalInitialization
//
// Purpose: 
//   This is a Qt slot function that contains initialization code for the GUI
//   that is executed once the GUI is in its event loop and waiting to hear
//   that the viewer is ready.
//
// Note:       uses initStage variable.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 18 15:56:39 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Oct 22 12:13:04 PDT 2003
//   I passed a flag to LoadFile that lets it add default plots when the file
//   is opened.
//
//   Brad Whitlock, Mon Nov 10 14:56:51 PST 2003
//   I added a stage to load a session file.
//
//   Kathleen Bonnell, Mon Dec  1 08:40:21 PST 2003 
//   Ensure that qprinter is initialized with the viewer's PrinterAttributes. 
//
// ****************************************************************************

void
QvisGUIApplication::FinalInitialization()
{
    int timeid;
    bool moreInit = true;

    debug4 << "QvisGUIApplication::FinalInitialization: initStage="
           << initStage << endl;

    switch(initStage)
    {
    case 0:
        timeid = visitTimer->StartTimer();
        // Tell the viewer to show all of its windows.
        viewer->ShowAllWindows();

        // Show the main window
        mainWin->show();

        // Indicate that future messages should go to windows and not
        // to the console.
        writeToConsole = false;

        visitTimer->StopTimer(timeid, "Showing windows");
        break;
    case 1:
        timeid = visitTimer->StartTimer();
        // Process the window config file settings.
        ProcessWindowConfigSettings(systemSettings);
        ProcessWindowConfigSettings(localSettings);
        delete systemSettings; systemSettings = 0;
        delete localSettings;  localSettings = 0;

        visitTimer->StopTimer(timeid, "Processing window configs");
        break;
    case 2:
        // Set up printing
        timeid = visitTimer->StartTimer();
        printer = new QPrinter;
        printerObserver = new ObserverToCallback(viewer->GetPrinterAttributes(),
            UpdatePrinterAttributes, (void *)printer);
        viewer->GetPrinterAttributes()->SetCreator(viewer->GetLocalUserName());
        PrinterAttributesToQPrinter(viewer->GetPrinterAttributes(), printer);
        visitTimer->StopTimer(timeid, "Setting up printer");

        // Show that we're ready.
        SplashScreenProgress("VisIt is ready.", 100);
        break;
    case 3:
        if(splash)
            splash->hide();
        break;
    case 4:
        // Load the initial data file.
        LoadFile(true);
        break;
    case 5:
        // Load the initial session file.
        RestoreSessionFile(sessionFile);
        break;
    case 6:
        moreInit = false;
        ++initStage;
        visitTimer->StopTimer(completeInit, "VisIt to be ready");
        break;
    default:
        moreInit = false;
    }

    //
    // If some of the case have not been covered, make the event loop
    // schedule this function again later.
    //
    if(moreInit)
    {
        ++initStage;
        QTimer::singleShot(10, this, SLOT(FinalInitialization()));
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::Exec
//
// Purpose: 
//   The application's main event loop
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:17:08 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 24 16:14:24 PST 2001
//   Added top-level exception handling code.
//
//   Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//   Changed the exception handling keywords to macros.
//
// ****************************************************************************

int
QvisGUIApplication::Exec()
{
    int retval;

    TRY
    {
        // Execute the application's main loop.
        retval = mainApp->exec();
    }
    CATCHALL(...)
    {
        // return an error code.
        retval = -1;
    }
    ENDTRY

    return retval;
}

// ****************************************************************************
// Method: QvisGUIApplication::ProcessArguments
//
// Purpose: 
//   Sets internal flags from values stored in the command line arguments and
//   removes the argument from the list.
//
// Arguments:
//   argc : A reference to the number of arguments.
//   argv : The list of command line arguments.
//
// Note:       This method should remove any arguments that the GUI does not
//             handle.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:05:04 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 14:46:20 PST 2001
//   Added initialization of the localOnly flag.
//
//    Jeremy Meredith, Mon Jul 23 16:43:15 PDT 2001
//    Added parsing of "-small".
//
//    Brad Whitlock, Wed Sep 5 09:07:59 PDT 2001
//    Added code to process -background, -foreground, -style, -font args.
//
//    Brad Whitlock, Thu Oct 25 14:31:09 PST 2001
//    Added code to print a warning message when the -geometry command
//    line flag is specified.
//
//    Brad Whitlock, Mon Feb 4 16:50:18 PST 2002
//    Removed processing of -small.
//
//    Brad Whitlock, Thu May 9 14:33:15 PST 2002
//    Added -o argument that loads a file.
//
//    Brad Whitlock, Tue Apr 1 08:14:21 PDT 2003
//    I changed how foreground, background, and style arguments are handled.
//
//    Brad Whitlock, Thu Apr 24 14:24:07 PST 2003
//    I replaced an explicit call to GetLongPathName, which is not supported
//    in Windows NT 4.0, with code to get a pointer to that function in
//    the kernel32 dll and execute the function through the pointer. This way,
//    we don't assume the function exists and we can safely run on NT4 with
//    the same VisIt binary, though it may be at the cost of double clicking
//    on registered VisIt file types.
//
//    Brad Whitlock, Thu Jun 19 12:11:51 PDT 2003
//    I added -nosplash.
//
//    Brad Whitlock, Fri Aug 15 13:14:04 PST 2003
//    I added support for MacOS X styles.
//
//    Brad Whitlock, Mon Nov 10 15:01:32 PST 2003
//    I added support for the -sessionfile argument. I moved the filename
//    expansion code to LongFileName.
//
// ****************************************************************************

void
QvisGUIApplication::ProcessArguments(int &argc, char **argv)
{
    AppearanceAttributes *aa = viewer->GetAppearanceAttributes();

    for(int i = 1; i < argc; ++i)
    {
        std::string current(argv[i]);

        // Remove any arguments that could be dangerous to the viewer.
        if(current == std::string("-host") ||
           current == std::string("-port") ||
           current == std::string("-nread") ||
           current == std::string("-nwrite") ||
           current == std::string("-borders") ||
           current == std::string("-geometry") ||
           current == std::string("-o") ||
           current == std::string("-sessionfile"))
        {
            // Process the -o argument.
            if(current == std::string("-o"))
            {
                if(i + 1 < argc)
                {
#if defined(_WIN32)
                    std::string tmpFileName(LongFileName(argv[i+1]));
                    if(tmpFileName.substr(1,2) == ":\\")
                        tmpFileName = std::string("localhost:") + tmpFileName;
                    loadFile = QualifiedFilename(tmpFileName);
#else
                    loadFile = QualifiedFilename(argv[i + 1]);
#endif
                }
                else
                {
                    cerr << "The -o option must be followed by a "
                            "filename." << endl;
                }
            }
            else if(current == std::string("-sessionfile"))
            {
                if(i + 1 < argc)
                {
                    // Set the name of the session file that we're going to
                    // load once the GUI's done initializing.
#if defined(_WIN32)
                    sessionFile = QString(LongFileName(argv[i+1]).c_str());
#else
                    sessionFile = QString(argv[i+1]);
#endif
                }
                else
                {
                    cerr << "The -sessionfile option must be followed by a "
                            "session filename." << endl;
                }                
            }

            for(int j = i; j + 2 < argc; ++j)
               argv[j] = argv[j + 2];
            argc -= 2;
            --i;
        }

        // Check to see if other gui options were provided.
        if(current == std::string("-localonly"))
        {
            localOnly = true;
        }
        else if(current == std::string("-noconfig"))
        {
            readConfig = false;
        }
        else if(current == std::string("-nosplash"))
        {
            showSplash = false;
        }
        else if(current == std::string("-geometry"))
        {
            // Print a warning message to the console
            cerr << "The -geometry flag is not used in VisIt because the "
                 << "application geometry can be set from within the tool."
                 << endl;
        }
        else if(current == std::string("-background") ||
                current == std::string("-bg"))
        {
            if(i + 1 >= argc)
            {
                cerr << "The -background option must be followed by a color."
                     << endl;
                continue;
            }
            // Store the background color in the viewer proxy's appearance
            // attributes so the gui will be colored properly on startup.
            backgroundColor = argv[i+1];
            ++i;
        }
        else if(current == std::string("-foreground") ||
                current == std::string("-fg"))
        {
            if(i + 1 >= argc)
            {
                cerr << "The -foreground option must be followed by a color."
                     << endl;
                continue;
            }
            // Store the foreground color in the viewer proxy's appearance
            // attributes so the gui will be colored properly on startup.
            foregroundColor = argv[i+1];
            ++i;
        }
        else if(current == std::string("-style"))
        {
            if(i + 1 >= argc)
            {
                cerr << "The -style option must be followed by a style name."
                     << endl;
                continue;
            }
            std::string style(argv[i + 1]);
            if(style == "motif" || style == "cde" || style == "windows" ||
               style == "platinum"
#if QT_VERSION >= 230
               || style == "sgi"
#endif
#if QT_VERSION >= 300
#ifdef Q_WS_MACX
               || style == "aqua" || style == "macintosh"
#endif
#endif
               )
            {
                applicationStyle = argv[i + 1];
            }
            ++i;
        }
        else if(current == std::string("-font"))
        {
            if(i + 1 >= argc)
            {
                cerr << "The -font option must be followed by a "
                        "font description." << endl;
                continue;
            }
            aa->SetFontDescription(argv[i + 1]);
            ++i;
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::CustomizeAppearance
//
// Purpose: 
//   This is a Qt slot function that sets the application appearance settings
//   based on the values in the appearance attributes.
//
// Note:       This assumes that the viewer proxy and the main QApplication
//             have been created.
//
// Arguments:
//   notify : A flag indicating whether or not the other components should be
//            told about the style change.
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep 4 23:51:10 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 12:53:42 PDT 2002
//   Modified the code so it includes the main window orientation.
//
//    Jeremy Meredith, Fri Feb  1 15:10:13 PST 2002
//    Check the notify flag before setting the orientation.
//
//    Brad Whitlock, Thu Jun 19 11:51:25 PDT 2003
//    I removed the code to customize the splashscreen.
//
//    Brad Whitlock, Fri Aug 15 13:14:59 PST 2003
//    I added support for MacOS X styles.
//
// ****************************************************************************

void
QvisGUIApplication::CustomizeAppearance(bool notify)
{
    AppearanceAttributes *aa = viewer->GetAppearanceAttributes();
    bool backgroundSelected = aa->IsSelected(0);
    bool foregroundSelected = aa->IsSelected(1);
//    bool fontSelected = aa->IsSelected(2);
    bool styleSelected = aa->IsSelected(3);
    bool orientationSelected = aa->IsSelected(4);

    //
    // Set the style
    //
    if(styleSelected)
    {
#if QT_VERSION < 300
        if(aa->GetStyle() == "cde")
            mainApp->setStyle(new QCDEStyle);
        else if(aa->GetStyle() == "windows")
            mainApp->setStyle(new QWindowsStyle);
        else if(aa->GetStyle() == "platinum")
            mainApp->setStyle(new QPlatinumStyle);
#if QT_VERSION >= 230
        else if(aa->GetStyle() == "sgi")
            mainApp->setStyle(new QSGIStyle);
#endif
        else
            mainApp->setStyle(new QMotifStyle);
#else
        // Set the style via the style name.
        mainApp->setStyle(aa->GetStyle().c_str());
#endif
    }

    //
    // Set the application colors
    //
    bool needToSetColors = backgroundSelected || foregroundSelected ||
                           styleSelected;
    bool colorStyle = aa->GetStyle() != "aqua" && aa->GetStyle() != "macintosh";
    if(needToSetColors && colorStyle)
    {
        QColor bg(aa->GetBackground().c_str());
        QColor fg(aa->GetForeground().c_str());
        QColor btn(bg);

        // Put the converted RGB format color into the appearance attributes.
        char tmp[20];
        sprintf(tmp, "#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
        aa->SetBackground(tmp);
        sprintf(tmp, "#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
        aa->SetForeground(tmp);

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

    //
    // If the notify flag is set then tell the viewer and the splashscreen
    // about the appearance change.
    //
    if(notify)
    {
        //
        // Set the window orientation if is was selected and the main window
        // has been created.
        //
        if(orientationSelected)
        {
            SetOrientation(aa->GetOrientation());
        }

        // Tell the viewer about the new appearance.
        aa->Notify();
        viewer->SetAppearanceAttributes();
    }
    else
    {
        // This is important to do after the first time through this method
        // so we do not change styles accidentally.
        aa->UnSelectAll();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SetOrientation
//
// Purpose: 
//   Set the main window's orientation.
//
// Arguments:
//   vertical : Whether or not the main window is vertical.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 13:02:36 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::SetOrientation(int orientation)
{
    if(mainWin == 0)
        return;
    //
    // Tell the main window to set its orientation.
    //
    mainWin->SetOrientation(orientation);
    MoveAndResizeMainWindow(orientation);

    //
    // Tell the viewer to move its vis windows.
    //
    int x, y, w, h;
    CalculateViewerArea(orientation, x, y, w, h);
    viewer->SetWindowArea(x, y, w, h);
}

// ****************************************************************************
// Method: QvisGUIApplication::MoveAndResizeMainWindow
//
// Purpose: 
//   Move and resize the main window.
//
// Arguments:
//   vertical : Whether or not the main window is vertical.
//
// Note:
//   I'm using a hardcoded value of 400. This is not that great of a
//   thing to be doing, but it is the only way, so far, that I've found
//   to make the main window shrink to the right size when we change
//   orientations.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 13:02:36 PST 2002
//
// Modifications:
//   Sean Ahern, Tue Apr 16 11:49:23 PDT 2002
//   Renamed to better fit its purpose.
// 
//   Brad Whitlock, Thu Apr 18 13:10:20 PST 2002
//   Don't subtract the preshift on MS Windows.
//
// ****************************************************************************

void
QvisGUIApplication::MoveAndResizeMainWindow(int orientation)
{
    int x, y, w, h;

    //
    // Resize it appropriately.
    //
#if defined(_WIN32)
    x = screenX+borders[2]-shiftX;
    y = screenY+borders[0]-shiftY;
#else
    x = screenX+borders[2]-shiftX-preshiftX;
    y = screenY+borders[0]-shiftY-preshiftY;
#endif
    if (orientation < 2)
    {
        w = 400;
        h = screenH - borders[0] - borders[1];
    }
    else
    {
        w = screenW - borders[2] - borders[3];
        h = 400;
    }

    mainWin->setMaximumWidth(w);
    mainWin->setMinimumWidth(w);
    mainWin->setMaximumHeight(h);
    mainWin->setMinimumHeight(h);
    mainWin->resize(w, h);
    mainWin->move(x, y);
}

// ****************************************************************************
// Method: QvisGUIApplication::AddViewerArguments
//
// Purpose: 
//   Adds any arguments in the list to the arguments passed to the viewer.
//
// Arguments:
//   argc : The number of arguments.
//   argv : The list of arguments.
//
// Note:       Some arguments that are only used by the GUI are not stripped
//             at this point since they are needed to initialize Qt. Currently,
//             this is restricted to the -geometry argument.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 27 16:07:05 PST 2000
//
// Modifications:
//   Jeremy Meredith, Mon Jul 23 16:43:26 PDT 2001
//   Added stripping of "-small".
//
//   Brad Whitlock, Mon Feb 4 17:09:35 PST 2002
//   Removed stripping of small.
//
//   Brad Whitlock, Tue Apr 1 08:48:03 PDT 2003
//   I added code to add the right values for foreground, background and style.
//
// ****************************************************************************

void
QvisGUIApplication::AddViewerArguments(int argc, char **argv)
{
    AppearanceAttributes *aa = viewer->GetAppearanceAttributes();

    for(int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if(arg == "-geometry")
        {
            // skip the WxH+X+Y string too.
            ++i;
        }
        else if(arg == "-foreground" || arg == "-fg")
        {
            viewer->AddArgument(argv[i]);
            viewer->AddArgument(aa->GetForeground());
            ++i;
        }
        else if(arg == "-background" || arg == "-bg")
        {
            viewer->AddArgument(argv[i]);
            viewer->AddArgument(aa->GetBackground());
            ++i;
        }
        else if(arg == "-style")
        {
            viewer->AddArgument(argv[i]);
            viewer->AddArgument(aa->GetStyle());
            ++i;
        }
        else
            viewer->AddArgument(argv[i]);
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::CalculateViewerArea
//
// Purpose: 
//   Figures out where the viewer's windows should go.
//
// Arguments:
//   orientation      : The orientation of the main window.
//   x,y,width,height : Return variables for the geometry.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 15:48:00 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Feb 4 16:50:41 PST 2002
//   Removed coding for small screen.
//
// ****************************************************************************

void
QvisGUIApplication::CalculateViewerArea(int orientation, int &x, int &y,
                                        int &width, int &height)
{
    if (orientation < 2)
    {
        // vertical gui
        int mw = mainWin->width();
        x = screenX + mw + borders[2] + borders[3];
        y = screenY;
        width = screenW - mw - borders[2] - borders[3];
        height = screenH;
    }
    else
    {
        // horizontal gui
        int mh = mainWin->height();
        x = 0;
        y = mh + borders[0] + borders[1];
        width = screenW;
        height = screenH - mh - borders[0] - borders[1];
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::AddViewerSpaceArguments
//
// Purpose: 
//   This method adds command line arguments to the viewer before
//   the viewer is launched.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 16:33:09 PST 2000
//
// Modifications:
//    Jeremy Meredith, Fri Jul 20 11:16:46 PDT 2001
//    Moved CalculateBorders out of here.  Added "shift" arguments as well.
//   
//    Jeremy Meredith, Fri Jul 20 13:58:07 PDT 2001
//    Changed the geometry flags to use the calculated screen size.
//
//    Jeremy Meredith, Mon Jul 23 16:44:14 PDT 2001
//    Allowed smallScreen to shrink the screen passed to the viewer.
//
//    Jeremy Meredith, Fri Sep 14 13:55:24 PDT 2001
//    Added preshift arguments.
//
// ****************************************************************************

void
QvisGUIApplication::AddViewerSpaceArguments()
{
    char temp[100];
    int x, y, width, height;
    int orientation = viewer->GetAppearanceAttributes()->GetOrientation();

    // Figure out where the viewer's windows should go.
    CalculateViewerArea(orientation, x, y, width, height);

    // Tell the viewer where it can put its window.
    viewer->AddArgument("-geometry");
    sprintf(temp, "%dx%d+%d+%d", width, height, x, y);
    viewer->AddArgument(temp);

    // Tell the viewer the size of the window borders.
    viewer->AddArgument("-borders");
    sprintf(temp, "%d,%d,%d,%d", borders[0], borders[1],
            borders[2], borders[3]);
    viewer->AddArgument(temp);

    // Tell the viewer the amount of the window shift.
    viewer->AddArgument("-shift");
    sprintf(temp, "%d,%d", shiftX,shiftY);
    viewer->AddArgument(temp);

    // Tell the viewer the amount of the window preshift.
    viewer->AddArgument("-preshift");
    sprintf(temp, "%d,%d", preshiftX,preshiftY);
    viewer->AddArgument(temp);
}

// ****************************************************************************
// Method: QvisGUIApplication::CreateMainWindow
//
// Purpose: 
//   Creates the main window.
//
// Arguments:
//   orientation : Whether the window is vertical or horizontal.
//   progress    : The amount of progress to show.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 17 17:44:42 PST 2003
//
// Modifications:
//   Brad Whitlock, Wed Jul 30 16:51:44 PST 2003
//   Hooked up new reopenOnNextFrame signal for the main window.
//
//   Brad Whitlock, Thu Sep 4 10:28:07 PDT 2003
//   I hooked up hideApplication and showApplication signals.
//
// ****************************************************************************

void
QvisGUIApplication::CreateMainWindow()
{
    int timeid = visitTimer->StartTimer();
    int orientation = viewer->GetAppearanceAttributes()->GetOrientation();

    // Make it so the application terminates when the last
    // window is closed.
    connect(mainApp, SIGNAL(aboutToQuit()), mainApp, SLOT(closeAllWindows()));
    connect(mainApp, SIGNAL(lastWindowClosed()), mainApp, SLOT(quit()));
    connect(mainApp, SIGNAL(hideApplication()), this, SLOT(IconifyWindows()));
    connect(mainApp, SIGNAL(showApplication()), this, SLOT(DeIconifyWindows()));

    // Create the main window. Note that the static attributes of
    // QvisWindowBase, which all windows use, are being set here through
    // the mainWin pointer.
    std::string title("VisIt ");
    title += VERSION;
    mainWin = new QvisMainWindow(orientation, title.c_str());
    connect(mainWin, SIGNAL(saveSettings()), this, SLOT(SaveSettings()));
    connect(mainWin, SIGNAL(iconifyWindows()), this, SLOT(IconifyWindows()));
    connect(mainWin, SIGNAL(deIconifyWindows()), this, SLOT(DeIconifyWindows()));
    connect(mainWin, SIGNAL(activateAboutWindow()), this, SLOT(AboutVisIt()));
    connect(mainWin, SIGNAL(saveWindow()), this, SLOT(SaveWindow()));
    connect(mainWin, SIGNAL(printWindow()), this, SLOT(PrintWindow()));
    connect(mainWin, SIGNAL(activatePrintWindow()), this, SLOT(SetPrinterOptions()));
    connect(mainWin->GetPlotManager(), SIGNAL(activatePlotWindow(int)),
            this, SLOT(ActivatePlotWindow(int)));
    connect(mainWin->GetPlotManager(), SIGNAL(activateOperatorWindow(int)),
            this, SLOT(ActivateOperatorWindow(int)));
    connect(mainWin, SIGNAL(refreshFileList()), this, SLOT(RefreshFileList()));
    connect(mainWin, SIGNAL(reopenOnNextFrame()),
            this, SLOT(RefreshFileListAndNextFrame()));
    connect(mainWin, SIGNAL(restoreSession()), this, SLOT(RestoreSession()));
    connect(mainWin, SIGNAL(saveSession()), this, SLOT(SaveSession()));
    mainWin->ConnectMessageAttr(&message);
    mainWin->ConnectGUIMessageAttributes();
    mainWin->ConnectGlobalAttributes(viewer->GetGlobalAttributes());
    mainWin->ConnectPlotList(viewer->GetPlotList());
    mainWin->ConnectWindowInformation(viewer->GetWindowInformation());

    // Move and resize the GUI so that we can get accurate size and
    // position information from it.
    MoveAndResizeMainWindow(orientation);

    visitTimer->StopTimer(timeid, "Creating main window");
}

// *****************************************************************************
// Method: QvisGUIApplication::CreateWindows
//
// Purpose: 
//   Creates the various GUI windows and connects them to observe
//   state objects in the viewer and the file server.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:17:31 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 15:46:05 PST 2003
//   I removed old mod comments and rewrote the routine so it can be called
//   from within the Qt event loop.
//
//   Eric Brugger, Wed Aug 20 14:01:46 PDT 2003
//   Added curve view attributes.
//
//   Brad Whitlock, Mon Oct 13 17:22:45 PST 2003
//   I hooked up a new signal/slot between the main window and the preferenes
//   window.
//
//   Brad Whitlock, Fri Oct 31 14:24:24 PST 2003
//   I changed how the annotation window is initialized.
//
// ****************************************************************************

bool
QvisGUIApplication::CreateWindows(int startPercent, int endPercent)
{
    bool  done = false;
    const int nWindows = 25;
    float perWindow = float(endPercent - startPercent) / float(nWindows-1);
#define PERCENT int(startPercent + (perWindow * windowInitStage))

    debug4 << "QvisGUIApplication::CreateWindows: windowInitStage="
           << windowInitStage << endl;

    switch(windowInitStage)
    {
    case 0:
        windowTimeId = visitTimer->StartTimer();
        // Create the command line interface window.
        SplashScreenProgress("Creating CLI window...", PERCENT);
        cliWin = new QvisCommandLineWindow(
           "Command Line Interface", "CLI", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateCommandLineWindow()), cliWin, SLOT(show()));
        otherWindows.push_back(cliWin);
        break;
    case 1:
        // Create the file selection window.
        SplashScreenProgress("Creating File Selection window...", PERCENT);
        fileWin = new QvisFileSelectionWindow("File selection");
        fileWin->ConnectSubjects(viewer->GetHostProfileList());
        connect(mainWin, SIGNAL(activateFileWindow()), fileWin, SLOT(show()));
        otherWindows.push_back(fileWin);
        break;
    case 2:
        // Create the file information window.
        SplashScreenProgress("Creating File Information window...", PERCENT);
        fileInformationWin = new QvisFileInformationWindow(fileServer, "File information",
            "FileInfo", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateFileInformationWindow()), fileInformationWin, SLOT(show()));
        otherWindows.push_back(fileInformationWin);
        break;
    case 3:
        // Create the message window
        SplashScreenProgress("Creating Message window...", PERCENT);
        messageWin = new QvisMessageWindow(&message, "Information");
        otherWindows.push_back(messageWin);
        break;
    case 4:
        // Create the output window
        SplashScreenProgress("Creating Output window...", PERCENT);
        outputWin = new QvisOutputWindow(&message, "Output", "Output",
            mainWin->GetNotepad());
        outputWin->CreateEntireWindow();
        connect(mainWin, SIGNAL(activateOutputWindow()), outputWin, SLOT(show()));
        connect(outputWin, SIGNAL(unreadOutput(bool)),
                mainWin, SLOT(unreadOutput(bool)));
        otherWindows.push_back(outputWin);
        break;
    case 5:
        // Create the host profile window
        SplashScreenProgress("Creating Host Profile window...", PERCENT);
        hostWin = new QvisHostProfileWindow(viewer->GetHostProfileList(),
            "Host profiles", "Profiles", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateHostWindow()), hostWin, SLOT(show()));
        otherWindows.push_back(hostWin);
        break;
    case 6:
        // Create the save window.
        SplashScreenProgress("Creating Save window...", PERCENT);
        saveWin = new QvisSaveWindow(viewer->GetSaveWindowAttributes(),
           "Set save options", "Save options", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateSaveWindow()),
                saveWin, SLOT(show()));
        otherWindows.push_back(saveWin);
        break;
    case 7:
        // Create the engine window.
        SplashScreenProgress("Creating Engine window...", PERCENT);
        engineWin = new QvisEngineWindow(viewer->GetEngineList(),
            "Compute engines", "Engines", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateEngineWindow()), engineWin, SLOT(show()));
        otherWindows.push_back(engineWin);
        break;
    case 8:
        // Create the animation window.
        SplashScreenProgress("Creating Animation window...", PERCENT);
        animationWin = new QvisAnimationWindow(viewer->GetAnimationAttributes(),
            "Animation", "Animation", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateAnimationWindow()),
                animationWin, SLOT(show()));
        otherWindows.push_back(animationWin);
        break;
    case 9:
        // Create the annotation window.
        SplashScreenProgress("Creating Annotation window...", PERCENT);
        annotationWin = new QvisAnnotationWindow("Annotation", "Annotation",
            mainWin->GetNotepad());
        annotationWin->ConnectAnnotationAttributes(viewer->GetAnnotationAttributes());
        annotationWin->ConnectAnnotationObjectList(
            viewer->GetAnnotationObjectList());
        connect(mainWin, SIGNAL(activateAnnotationWindow()),
                annotationWin, SLOT(show()));
        otherWindows.push_back(annotationWin);
        break;
    case 10:
        // Create the colortable window,
        SplashScreenProgress("Creating Colortable window...", PERCENT);
        colorTableWin = new QvisColorTableWindow(viewer->GetColorTableAttributes(),
            "Color tables", "Color tables", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateColorTableWindow()),
                colorTableWin, SLOT(show()));
        otherWindows.push_back(colorTableWin);
        break;
    case 11:
        // Create the expressions window,
        SplashScreenProgress("Creating Expressions window...", PERCENT);
        exprWin = new QvisExpressionsWindow(viewer->GetExpressionList(),
            "Expressions", "Expressions", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateExpressionsWindow()),
                exprWin, SLOT(show()));
        otherWindows.push_back(exprWin);
        break;
    case 12:
        // Create the subset window.
        SplashScreenProgress("Creating Subset window...", PERCENT);
        subsetWin = new QvisSubsetWindow(viewer->GetSILRestrictionAttributes(),
            "Subset", "Subset", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateSubsetWindow()), subsetWin, SLOT(show()));
        connect(mainWin->GetPlotManager(), SIGNAL(activateSubsetWindow()),
                subsetWin, SLOT(show()));
        otherWindows.push_back(subsetWin);
        break;
    case 13:
        // Create the plugin manager window.
        pluginWin = new QvisPluginWindow(viewer->GetPluginManagerAttributes(),
            "Plugin Manager", "Plugins", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activatePluginWindow()),
                pluginWin, SLOT(show()));
        connect(pluginWin, SIGNAL(pluginSettingsChanged()),
                this, SLOT(LoadPlugins()));
        otherWindows.push_back(pluginWin);
        break;
    case 14:
        // Create the view window.
        SplashScreenProgress("Creating View window...", PERCENT);
        viewWin = new QvisViewWindow("View", "View", mainWin->GetNotepad());
        viewWin->ConnectCurveAttributes(viewer->GetViewCurveAttributes());
        viewWin->Connect2DAttributes(viewer->GetView2DAttributes());
        viewWin->Connect3DAttributes(viewer->GetView3DAttributes());
        viewWin->ConnectWindowInformation(viewer->GetWindowInformation());
        connect(mainWin, SIGNAL(activateViewWindow()),
                viewWin, SLOT(show()));
        otherWindows.push_back(viewWin);
        break;
    case 15:
        // Create the appearance window.
        SplashScreenProgress("Creating Appearance window...", PERCENT);
        appearanceWin = new QvisAppearanceWindow(viewer->GetAppearanceAttributes(),
            "Appearance", "Appearance", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateAppearanceWindow()),
                appearanceWin, SLOT(show()));
        connect(appearanceWin, SIGNAL(changeAppearance(bool)),
                this, SLOT(CustomizeAppearance(bool)));
        otherWindows.push_back(appearanceWin);
        break;
    case 16:
        // Create the keyframe window.
        SplashScreenProgress("Creating Keyframe window...", PERCENT);
        keyframeWin = new QvisKeyframeWindow(viewer->GetKeyframeAttributes(),
            "Keyframe Editor", "Keyframer", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateKeyframeWindow()),
                keyframeWin, SLOT(show()));
        otherWindows.push_back(keyframeWin);
        /*
          DISABLED TEMPORARILY - 5/8/02 JSM
        keyframeWin->ConnectAttributes(viewer->GetAnnotationAttributes(), "Annotation");
        keyframeWin->ConnectAttributes(viewer->GetAppearanceAttributes(), "Appearance");
        */
        keyframeWin->ConnectGlobalAttributes(viewer->GetGlobalAttributes());
        keyframeWin->ConnectPlotList(viewer->GetPlotList());
        break;
    case 17:
        // Create the lighting window.
        SplashScreenProgress("Creating Lighting window...", PERCENT);
        lightingWin = new QvisLightingWindow(viewer->GetLightList(),
            "Lighting", "Lighting", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateLightingWindow()),
                lightingWin, SLOT(show()));
        otherWindows.push_back(lightingWin);
        break;
    case 18:
        // Create the global lineout window.
        SplashScreenProgress("Creating GlobalLineout window...", PERCENT);
        globalLineoutWin = new QvisGlobalLineoutWindow(viewer->GetGlobalLineoutAttributes(),
            "Lineout", "Lineout", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateGlobalLineoutWindow()),
                globalLineoutWin, SLOT(show()));
        otherWindows.push_back(globalLineoutWin);
        break;
    case 19:
        // Create the material options window.
        SplashScreenProgress("Creating materials window...", PERCENT);
        materialWin = new QvisMaterialWindow(viewer->GetMaterialAttributes(),
            "Material Reconstruction Options", "MIR Options", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateMaterialWindow()),
                materialWin, SLOT(show()));
        otherWindows.push_back(materialWin);
        break;
    case 20:
        // Create the pick window.
        SplashScreenProgress("Creating Pick window...", PERCENT);
        pickWin = new QvisPickWindow(viewer->GetPickAttributes(),
            "Pick", "Pick", mainWin->GetNotepad());
        pickWin->CreateEntireWindow();
        connect(mainWin, SIGNAL(activatePickWindow()),
                pickWin, SLOT(show()));
        otherWindows.push_back(pickWin);
        break;
    case 21:
        // Create the help window
        SplashScreenProgress("Creating Help window...", PERCENT);
        helpWin = new QvisHelpWindow("Help");
        connect(mainWin, SIGNAL(activateCopyrightWindow()),
                helpWin, SLOT(displayCopyright()));
        connect(mainWin, SIGNAL(activateHelpWindow()),
                helpWin, SLOT(show()));
        connect(mainWin, SIGNAL(activateReleaseNotesWindow()),
                helpWin, SLOT(displayReleaseNotes()));
        otherWindows.push_back(helpWin);
        break;
    case 22:
        // Create the query window.
        SplashScreenProgress("Creating Query window...", PERCENT);
        queryWin = new QvisQueryWindow("Query", "Query", mainWin->GetNotepad());
        queryWin->ConnectQueryList(viewer->GetQueryList());
        queryWin->ConnectQueryAttributes(viewer->GetQueryAttributes());
        queryWin->ConnectPickAttributes(viewer->GetPickAttributes());
        queryWin->ConnectPlotList(viewer->GetPlotList());
        connect(mainWin, SIGNAL(activateQueryWindow()),
                queryWin, SLOT(show()));
        otherWindows.push_back(queryWin);
        break;
    case 23:
        // Create the preferences window.
        SplashScreenProgress("Creating Preferences window...", PERCENT);
        preferencesWin = new QvisPreferencesWindow(viewer->GetGlobalAttributes(),
            "Preferences", "Preferences", mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activatePreferencesWindow()),
                preferencesWin, SLOT(show()));
        connect(preferencesWin, SIGNAL(changeTimeFormat(const TimeFormat &)),
                mainWin, SLOT(SetTimeStateFormat(const TimeFormat &)));
        otherWindows.push_back(preferencesWin);
        break;
    case 24:
        // Create the rendering preferences window.
        SplashScreenProgress("Creating Rendering window...", PERCENT);
        renderingWin = new QvisRenderingWindow("Rendering options", "Rendering",
            mainWin->GetNotepad());
        connect(mainWin, SIGNAL(activateRenderingWindow()),
                renderingWin, SLOT(show()));
        renderingWin->ConnectRenderingAttributes(viewer->GetRenderingAttributes());
        renderingWin->ConnectWindowInformation(viewer->GetWindowInformation());
        otherWindows.push_back(renderingWin);

        // Hook up the viewer's status attributes to the main window and the
        // engine window.
        engineWin->ConnectStatusAttributes(viewer->GetStatusAttributes());
        mainWin->ConnectViewerStatusAttributes(viewer->GetStatusAttributes());

        // Move this code to the new last case when one is added.
        done = true;
        visitTimer->StopTimer(windowTimeId, "Creating windows");
        break;
    }

#undef PERCENT

    // Move to the next stage in window creation.
    ++windowInitStage;

    return done;
}

// ****************************************************************************
//  Method:  QvisGUIApplication::LoadPlugins
//
//  Purpose:
//    Load the plugins and create the needed windows.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 27, 2001
//
//  Modifications:
//    Brad Whitlock, Wed Feb 6 15:02:34 PST 2002
//    Added code to restore the regular cursor.
//
//    Brad Whitlock, Wed Mar 13 14:35:04 PST 2002
//    Upgraded to QT 3.0
//
//    Brad Whitlock, Wed Jun 18 16:25:10 PST 2003
//    Added timing information.
//
// ****************************************************************************

void
QvisGUIApplication::LoadPlugins()
{
    debug4 << "QvisGUIApplication::LoadPlugins()" << endl;

    int timeid = visitTimer->StartTimer();
    SplashScreenProgress("Loading plugins...", 92);
    viewer->LoadPlugins();
    visitTimer->StopTimer(timeid, "Loading plugins");

    timeid = visitTimer->StartTimer();
    SplashScreenProgress("Creating plugin windows...", 98);
    CreatePluginWindows();

    // Now that plugins are loaded, restore the normal cursor.
    RestoreCursor();
    // Enable the plot and operator menus.
    mainWin->GetPlotManager()->EnablePluginMenus();
    visitTimer->StopTimer(timeid, "Creating plugin windows");
}

// ****************************************************************************
// Method: QvisGUIApplication::CreatePluginWindows
//
// Purpose: 
//   Loops over the plugin list and creates the plugin windows.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 16:33:36 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Mon May  7 15:58:13 PDT 2001
//   Added Erase operator.
//   
//   Hank Childs, Fri Jul 20 09:19:07 PDT 2001
//   Removed material select operator.
//
//   Jeremy Meredith, Thu Jul 26 03:07:27 PDT 2001
//   Made it use real operator plugins.
//
//   Jeremy Meredith, Thu Jul 26 09:53:57 PDT 2001
//   Renamed plugin info to include the word "plot".
//
//   Hank Childs, Tue Sep 11 14:11:05 PDT 2001
//   Ensure that progress bar in monotonically increasing.
//
//   Jeremy Meredith, Tue Sep 25 18:02:51 PDT 2001
//   Fixed the progress updates so they are monotonically increasing again.
//
//   Jeremy Meredith, Fri Sep 28 13:58:31 PDT 2001
//   Made plugin managers key off of ID instead of index.
//
//   Brad Whitlock, Mon Apr 22 17:05:31 PST 2002
//   Removed message to the splash screen.
//
//   Jeremy Meredith, Wed May  8 15:22:05 PDT 2002
//   Added keyframe window connection.
//
//   Brad Whitlock, Thu Mar 13 09:49:53 PDT 2003
//   I added support for icons in the plot and operator menus.
//
// ****************************************************************************

void
QvisGUIApplication::CreatePluginWindows()
{
    int i;

    // Get a pointer to the plot plugin manager.
    PlotPluginManager *plotPluginManager = PlotPluginManager::Instance();

    // Create the window and populate the menu for each plot plugin.
    for(i = 0; i < plotPluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the plot plugin information.
        GUIPlotPluginInfo *GUIInfo = plotPluginManager->GetGUIPluginInfo(
                                                 plotPluginManager->GetEnabledID(i));
   
        // Create the plot plugin window.
        QvisPostableWindowObserver *win = GUIInfo->CreatePluginWindow(i,
            viewer->GetPlotAttributes(i), mainWin->GetNotepad());

        // Add the window to the list of plot windows.
        plotWindows.push_back(win);

        // Add the attributes to the keyframe window
        /*
          DISABLED TEMPORARILY - 5/8/02 JSM
        keyframeWin->ConnectPlotAttributes(viewer->GetPlotAttributes(i),i);
        */

        // Add an option to the main window's plot manager widget's plot list.
        mainWin->GetPlotManager()->AddPlotType(GUIInfo->GetMenuName(),
                                               GUIInfo->GetVariableTypes(),
                                               GUIInfo->XPMIconData());
    }

    // Get a pointer to the operator plugin manager.
    OperatorPluginManager *operatorPluginManager = OperatorPluginManager::Instance();

    // Create the window and populate the menu for each operator plugin.
    for(i = 0; i < operatorPluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the operator plugin information.
        GUIOperatorPluginInfo *GUIInfo = operatorPluginManager->GetGUIPluginInfo(
                                       operatorPluginManager->GetEnabledID(i));

        // Create the operator plugin window.
        QvisPostableWindowObserver *win = GUIInfo->CreatePluginWindow(i,
            viewer->GetOperatorAttributes(i), mainWin->GetNotepad());

        // Add the window to the list of operator windows.
        operatorWindows.push_back(win);

        // Add an option to the main window's operator manager widget's
        // operator list.
        mainWin->GetPlotManager()->AddOperatorType(GUIInfo->GetMenuName(),
                                                   GUIInfo->XPMIconData());
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::WriteConfigFile
//
// Purpose: 
//   Writes the GUI's part of the config file.
//
// Arguments:
//   filename : The config file to write.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:16:33 PST 2000
//
// Modifications:
//    Brad Whitlock, Fri Mar 23 16:20:49 PST 2001
//    Modified the code so it treats plots and operators like plugins.
//
//    Brad Whitlock, Tue May 1 16:09:11 PST 2001
//    Added the engine window.
//
//    Brad Whitlock, Mon Jun 11 14:04:47 PST 2001
//    Added the color table window.
//
//    Brad Whitlock, Sun Jun 17 21:01:24 PST 2001
//    Added the annotation window.
//
//    Brad Whitlock, Thu May 24 14:58:27 PST 2001
//    Added the subset window.
//
//    Brad Whitlock, Thu Jul 26 17:21:49 PST 2001
//    Added the view window.
//
//    Jeremy Meredith, Wed Sep  5 13:58:30 PDT 2001
//    Added plugin manager window.
//
//    Brad Whitlock, Wed Sep 5 08:45:41 PDT 2001
//    Rewrote how windows are saved.
//
//    Brad Whitlock, Tue May 20 14:55:54 PST 2003
//    Modified CreateNode interface for state objects.
//
//    Brad Whitlock, Thu Oct 9 17:12:14 PST 2003
//    Added code to save the databases that are being visualized so we can
//    attempt to load those files before restoring a session.
//
//    Brad Whitlock, Mon Oct 13 17:25:47 PST 2003
//    Added code to save the timestate format.
//
//    Brad Whitlock, Thu Dec 18 14:41:57 PST 2003
//    Changed CreateNode method calls.
//
// ****************************************************************************

void
QvisGUIApplication::WriteConfigFile(const char *filename)
{
    // Create the root node called "VisIt" and create a "Version"
    // node under it.
    DataNode root("VisIt");
    root.AddNode(new DataNode("Version", std::string(VERSION)));
    
    // Create a "GUI" node and add it under "VisIt".
    DataNode *guiNode = new DataNode("GUI");
    root.AddNode(guiNode);

    // Add some attributes under the "GUI" node.
    fileServer->CreateNode(guiNode, true, true);

    // Save the appearance attributes.
    viewer->GetAppearanceAttributes()->CreateNode(guiNode, true, false);

    // Make the windows save their attributes.
    for(int i = 0; i < otherWindows.size(); ++i)
        otherWindows[i]->CreateNode(guiNode);

    // Make the plugin windows add their information to the guiNode that is
    // saved into the config file.
    WritePluginWindowConfigs(guiNode);

    //
    // Save the list of files that are being used in plots. This is not
    // perfect for sessions where there may be many windows that contain
    // databases from several remote hosts because we're only considering
    // files from the active window.
    //
    stringVector plotDatabases;
    const PlotList *pl = viewer->GetPlotList();
    for(int j = 0; j < pl->GetNumPlots(); ++j)
    {
        const Plot &p = pl->GetPlot(j);
        // Make sure we only add it if it's not already there.
        if(std::find(plotDatabases.begin(),
                     plotDatabases.end(),
                     p.GetDatabaseName()) == plotDatabases.end())
        {
            plotDatabases.push_back(p.GetDatabaseName());
        }
    }
    if(plotDatabases.size() > 0)
        guiNode->AddNode(new DataNode("plotDatabases", plotDatabases));

    // Save the timestate format.
    TimeFormat fmt(mainWin->GetTimeStateFormat());
    fmt.CreateNode(guiNode, true, false);

    // Try to open the output file.
    if((fp = fopen(filename, "wb")) == 0)
        return;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(&root);

    // close the file
    fclose(fp);
    fp = 0;
}

// ****************************************************************************
// Method: QvisGUIApplication::WritePluginWindowConfigs
//
// Purpose: 
//   Loops through the lists of plugin windows and makes each of them add their
//   window information to the config file info.
//
// Arguments:
//   parentNode : The data node to which the plugins will add their window
//                information.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 16:10:37 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::WritePluginWindowConfigs(DataNode *parentNode)
{
    int i;

    // Iterate through each plot window and have it add its window information
    // to the config file data.
    for(i = 0; i < plotWindows.size(); ++i)
    {
        plotWindows[i]->CreateNode(parentNode);
    }

    // Iterate through each operator window and have it add its window
    // information to the config file data.
    for(i = 0; i < operatorWindows.size(); ++i)
    {
        operatorWindows[i]->CreateNode(parentNode);
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SaveSession
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to save out all of its
//   state to an XML file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 14 11:52:52 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Aug 12 11:23:52 PDT 2003
//   Added code to force the session file to have a .session extension.
//
//   Brad Whitlock, Mon Nov 10 15:11:20 PST 2003
//   I made sessions use the .vses extension when we're on Windows.
//
// ****************************************************************************

void
QvisGUIApplication::SaveSession()
{
#if defined(_WIN32)
    QString sessionExtension(".vses");
#else
    QString sessionExtension(".session");
#endif

    // Create the name of a VisIt session file to use.
    QString defaultFile;
    defaultFile.sprintf("%svisit%04d", GetUserVisItDirectory().c_str(),
                        sessionCount);
    defaultFile += sessionExtension;

    // Get the name of the file that the user saved.
    QString sFilter(QString("VisIt session (*") + sessionExtension + ")");
    QString fileName = QFileDialog::getSaveFileName(defaultFile, sFilter);

    // If the user chose to save a file, tell the viewer to write its state
    // to that file.
    if(!fileName.isNull())
    {
        // Force the file to have a .session extension.
        if(fileName.right(sessionExtension.length()) != sessionExtension)
            fileName += sessionExtension;

        // Tell the viewer to save a session file.
        ++sessionCount;
        viewer->ExportEntireState(fileName.latin1());

        // Write the gui part of the session with a ".gui" extension.
        fileName += ".gui";
        WriteConfigFile(fileName.latin1());
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::ReadConfigFile
//
// Purpose: 
//   Reads the config file and uses the GUI's portion to set window
//   information.
//
// Arguments:
//   filename : The name of the config file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 15:17:16 PST 2000
//
// Modifications:
//    Brad Whitlock, Fri Mar 23 16:19:47 PST 2001
//    Modified so plot and operator windows are treated as plugins.
//
//    Brad Whitlock, Tue May 1 16:09:54 PST 2001
//    Added engine window.
//
//    Brad Whitlock, Mon Jun 11 14:05:15 PST 2001
//    Added the colortable window.
//
//    Brad Whitlock, Sun Jun 17 21:01:54 PST 2001
//    Added annotation window.
//
//    Brad Whitlock, Thu May 24 14:58:58 PST 2001
//    Added subset window.
//
//    Brad Whitlock, Thu Jul 26 17:22:04 PST 2001
//    Added the view window.
//
//    Jeremy Meredith, Wed Sep  5 13:58:30 PDT 2001
//    Added plugin manager window.
//
//    Brad Whitlock, Wed Sep 5 08:40:11 PDT 2001
//    Moved most of the code to ProcessConfigSettings method.
//
//    Brad Whitlock, Tue Feb 19 12:47:42 PDT 2002
//    Modified the code so it returns the DataNode from the config file.
//
// ****************************************************************************

DataNode *
QvisGUIApplication::ReadConfigFile(const char *filename)
{
    DataNode *node = 0;

    // Try and open the file for reading.
    if((fp = fopen(filename, "rb")) == 0)
        return node;

    // Read the XML tag and ignore it.
    FinishTag();

    // Create a root node and use it to read the VisIt tree.
    node = new DataNode("ConfigSettings");
    ReadObject(node);
    fclose(fp); fp = 0;

    // Look for the VisIt tree.
    DataNode *visitRoot = node->GetNode("VisIt");
    if(visitRoot == 0)
        return node;

    // Get the gui node.
    DataNode *guiNode = visitRoot->GetNode("GUI");
    if(guiNode == 0)
        return node;

    // Force the appearance attributes to be set from the datanodes.
    viewer->GetAppearanceAttributes()->SetFromNode(guiNode);

    return node;
}

// ****************************************************************************
// Method: QvisGUIApplication::RestoreSession
//
// Purpose: 
//   This is a Qt slot function that allows the user to choose the name of
//   a VisIt session to restore and then tells the viewer to restore the
//   session using the file.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 14 11:54:10 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Oct 9 15:50:30 PST 2003
//   I changed how we restore sessions so that we first try to open
//   all of the files that are in the session.
//
//   Brad Whitlock, Wed Oct 22 12:12:11 PDT 2003
//   I prevented default plots from being added as we load files.
//
//   Brad Whitlock, Mon Nov 10 15:07:21 PST 2003
//   I moved the code to restore the session to RestoreSessionFile.
//
// ****************************************************************************

void
QvisGUIApplication::RestoreSession()
{
    // Get the name of the session to load.
    QString s(QFileDialog::getOpenFileName(GetUserVisItDirectory().c_str(),
#if defined(_WIN32)
              "VisIt session (*.vses)"));
#else
              "VisIt session (*.session)"));
#endif

    RestoreSessionFile(s);
}

// ****************************************************************************
// Method: QvisGUIApplication::RestoreSessionFile
//
// Purpose: 
//   Restores the specified session.
//
// Arguments:
//   s : The name of the session file to restore.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 10 15:07:37 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::RestoreSessionFile(const QString &s)
{
    // If the user chose a file, tell the viewer to import that session file.
    if(!s.isEmpty())
    {
        // Make the gui read in its part of the config.
        std::string guifilename(s.latin1());
        guifilename += ".gui";
        DataNode *node = ReadConfigFile(guifilename.c_str());
        if(node)
        {
            ProcessConfigSettings(node, false);
            ProcessWindowConfigSettings(node); 

            // Look for the VisIt tree.
            DataNode *visitRoot = node->GetNode("VisIt");
            if(visitRoot != 0)
            {
                // Get the gui node.
                DataNode *guiNode = visitRoot->GetNode("GUI");
                if(guiNode != 0)
                {
                    //
                    // Customize the GUI's appearance based on the session.
                    //
                    viewer->GetAppearanceAttributes()->SetFromNode(guiNode);
                    CustomizeAppearance(true);

                    //
                    // Look for the list of files that we need to open up
                    // and if we find it, open up each one of them.
                    //
                    DataNode *plotDatabases = guiNode->GetNode("plotDatabases");
                    if(plotDatabases)
                    {
                        const stringVector &db = plotDatabases->AsStringVector();
                        for(int i = 0; i < db.size(); ++i)
                        {
                            loadFile = QualifiedFilename(db[i]);
                            if(!fileServer->HaveOpenedFile(loadFile))
                                LoadFile(false);
                        }
                    }
                }
            }

            delete node;
        }

        // Have the viewer read in its part of the config. Note that we
        // pass the inVisItDir flag as false because we don't want to have
        // the viewer prepend the .visit directory to the file since it's
        // already part of the filename.
        std::string filename(s.latin1());
        viewer->ImportEntireState(filename, false);
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::ProcessConfigSettings
//
// Purpose: 
//   This method processes the configuration settings that were read by the
//   ReadConfigFile method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 5 08:39:57 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Feb 19 12:59:20 PDT 2002
//   Passed in a DataNode object from which to set the settings.
//
//   Brad Whitlock, Wed Sep 25 14:51:49 PST 2002
//   Moved a bunch of the code into ProcessWindowConfigSettings.
//
//   Brad Whitlock, Fri Mar 21 10:27:53 PDT 2003
//   I removed some code to get the version.
//
// ****************************************************************************

void
QvisGUIApplication::ProcessConfigSettings(DataNode *node, bool systemConfig)
{
    // If the node is not created, return.
    if(node == 0)
        return;

    // Look for the VisIt tree.
    DataNode *visitRoot = node->GetNode("VisIt");
    if(visitRoot == 0)
        return;

    // Get the gui node.
    DataNode *guiNode = visitRoot->GetNode("GUI");
    if(guiNode == 0)
        return;

    // Initialize the file server.
    if(!systemConfig)
        InitializeFileServer(guiNode);
}

// ****************************************************************************
// Method: QvisGUIApplication::ProcessWindowConfigSettings
//
// Purpose: 
//   This method processes the configuration settings that were read by the
//   ReadConfigFile method.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 25 14:51:13 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Mar 21 10:28:11 PDT 2003
//   I added code to process old versions.
//
//   Brad Whitlock, Fri Oct 10 12:11:45 PDT 2003
//   I added code to hide windows that are already posted so they don't cause
//   problems when we try and post them again or hide them when processing
//   the settings.
//
//   Brad Whitlock, Tue Oct 14 10:18:53 PDT 2003
//   I added code to set the timestate display mode for the main window and
//   the preferences window.
//
// ****************************************************************************

void
QvisGUIApplication::ProcessWindowConfigSettings(DataNode *node)
{
    // If the node is not created, return.
    if(node == 0)
        return;

    // Look for the VisIt tree.
    DataNode *visitRoot = node->GetNode("VisIt");
    if(visitRoot == 0)
        return;

    // Get the gui node.
    DataNode *guiNode = visitRoot->GetNode("GUI");
    if(guiNode == 0)
        return;

    // Get the version
    DataNode *version = visitRoot->GetNode("Version");
    const char *configVersion = 0;
    if(version != 0)
        configVersion = version->AsString().c_str();

    // Read the windows' information. The border sizes will need to be added
    // to the window's x, y coordinates so pass those to the SetFromNode
    // routines.
    for(int i = 0; i < otherWindows.size(); ++i)
    {
        if(otherWindows[i]->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *win = (QvisPostableWindow *)otherWindows[i];
            if(win->posted())
                win->hide();
        }
        otherWindows[i]->ProcessOldVersions(guiNode, configVersion);
        otherWindows[i]->SetFromNode(guiNode, borders);
    }

    // Set the time format.
    TimeFormat fmt;
    fmt.SetFromNode(guiNode);
    mainWin->SetTimeStateFormat(fmt);
    preferencesWin->SetTimeStateFormat(fmt);

    // Read the config file stuff for the plugin windows.
    ReadPluginWindowConfigs(guiNode, configVersion);
}

// ****************************************************************************
// Method: QvisGUIApplication::ReadPluginWindowConfigs
//
// Purpose: 
//   Loops through all of the plugin windows and makes them read their window
//   position, size information.
//
// Arguments:
//   parentNode : The data node that contains the information for the windows.
//   configVersion : The version of the config file.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 16:13:23 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Mar 21 10:22:49 PDT 2003
//   I added code to handle old versions.
//
//   Brad Whitlock, Fri Oct 10 12:11:45 PDT 2003
//   I added code to hide windows that are already posted so they don't cause
//   problems when we try and post them again or hide them when processing
//   the settings.
//
// ****************************************************************************

void
QvisGUIApplication::ReadPluginWindowConfigs(DataNode *parentNode,
    const char *configVersion)
{
    int i;

    // Iterate through each plot window and have it add its window information
    // to the config file data.
    for(i = 0; i < plotWindows.size(); ++i)
    {
        if(plotWindows[i]->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *win = (QvisPostableWindow *)plotWindows[i];
            if(win->posted())
                win->hide();
        }
        plotWindows[i]->ProcessOldVersions(parentNode, configVersion);
        plotWindows[i]->SetFromNode(parentNode, borders);
    }

    // Iterate through each operator window and have it add its window
    // information to the config file data.
    for(i = 0; i < operatorWindows.size(); ++i)
    {
        if(operatorWindows[i]->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *win = (QvisPostableWindow *)operatorWindows[i];
            if(win->posted())
                win->hide();
        }
        operatorWindows[i]->ProcessOldVersions(parentNode, configVersion);
        operatorWindows[i]->SetFromNode(parentNode, borders);
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::StartMDServer
//
// Purpose: 
//   Starts an mdserver through the viewer.
//
// Arguments:
//   hostName  : The host on which the mdserver is running.
//   args      : A vector containing the program arguments.
//   data      : A pointer that can be cast to "this".
//
// Returns:    
//
// Note:       This is a callback function that the fileServer object
//             uses to launch mdservers.
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 14:09:46 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 16:38:11 PST 2002
//   I added support for a security key.
//
//   Brad Whitlock, Mon May 5 14:03:13 PST 2003
//   I replaced most of the arguments with the args string vector.
//
// ****************************************************************************

void
QvisGUIApplication::StartMDServer(const std::string &hostName, 
    const stringVector &args, void *data)
{
    ViewerProxy *theViewer = (ViewerProxy *)data;

    // Have the viewer tells its mdserver running on hostName to connect
    // to the gui.
    theViewer->ConnectToMetaDataServer(hostName, args);
}

// ****************************************************************************
// Method: QvisGUIApplication::InitializeFileServer
//
// Purpose: 
//   Initializes the file server from entries in the config file.
//
// Arguments:
//   guiNode : A pointer to the GUI's data node.
//
// Note:       
//   This was broken out of the ReadConfigFile method due to the
//   complexity of things that can happen.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 4 16:41:18 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 14:43:03 PST 2001
//   Added code to prevent the file server from launching the initial mdserver
//   on a remote machine if the localOnly flag is set to true.
//
//   Brad Whitlock, Mon Oct 22 18:25:42 PST 2001
//   Changed the exception handling keywords to macros.
//
//   Brad Whitlock, Thu Oct 25 17:14:14 PST 2001
//   Added code to handle BadHostException.
//
//   Brad Whitlock, Mon Apr 15 11:57:30 PDT 2002
//   Added code to temporarily disable the splashscreen's window raising.
//
//   Sean Ahern, Tue Apr 16 15:30:13 PDT 2002
//   I removed the splash screen's ability to raise itself, so I removed
//   the raise code.
//
//   Brad Whitlock, Thu May 9 14:44:15 PST 2002
//   Changed fileServer so it is a pointer.
//
//   Brad Whitlock, Thu Jun 27 11:25:27 PDT 2002
//   I fixed the case where the default directory is bad so that it still
//   updates the file lists instead of leaving it in a bad state.
//
//   Brad Whitlock, Mon Apr 28 15:26:24 PST 2003
//   I added code to catch CouldNotConnectException in case we can't
//   connect to the mdserver. I also added code to prevent the path from
//   being changed if we're going to load a file.
//
// ****************************************************************************

void
QvisGUIApplication::InitializeFileServer(DataNode *guiNode)
{
    // Temporarily save the old settings.
    std::string oldHost(fileServer->GetHost());
    std::string oldPath(fileServer->GetPath());
    std::string oldFilter(fileServer->GetFilter());

    TRY
    {
        // Only unselect everything if there are entries for the file
        // server in the config file. If there aren't any entries, we
        // don't want to clear any pending selections.
        if(guiNode->GetNode("FileServerList") != 0)
        {
            // Read the new settings from the guiNode.
            fileServer->UnSelectAll();
            fileServer->SetFromNode(guiNode);

            // If the settings changed, notify the observers.
            if(fileServer->NumAttributesSelected() > 0)
            {
                // If the hosts are different and we are preventing the
                // mdserver from being launched remotely, tell the user about
                // with a warning message.
                if(localOnly && (oldHost != fileServer->GetHost()))
                {
                    QString msg;
                    msg.sprintf("Preventing the metadata server from being "
                                "launched on %s.", fileServer->GetHost().c_str());
                    Warning(msg);
                    fileServer->SetHost(oldHost);
                    fileServer->SetPath(oldPath);
                    fileServer->SetFilter(oldFilter);
                }
                else if(!loadFile.Empty())
                {
                    // We're going to load a file a little later so don't
                    // change the host or the path.
                    fileServer->SetHost(oldHost);
                    fileServer->SetPath(oldPath);
                }

                // Do the actual change in directory and notify observers
                // interested in the path and filter.
                fileServer->Notify();

                // Now that the path and filter have been applied, use them
                // to get the filtered file list and tell the observers that
                // are interested in that. It shouldn't hurt to do two notifies
                // since only the applied file list will be selected in this one.
                fileServer->SetAppliedFileList(fileServer->GetFilteredFileList());
                fileServer->Notify();
            }
        }
    }
    CATCH2(BadHostException, bhe)
    {
        QString msg;
        msg.sprintf("Hostname \"%s\" is not a recognized host.",
                    bhe.GetHostName().c_str());
        Error(msg);

        TRY
        {
            // Set the file server host, etc. back to the previous values.
            fileServer->SetHost(oldHost);
            fileServer->SetPath(oldPath);
            fileServer->SetFilter(oldFilter);
            fileServer->Notify();

            // Now that the path and filter have been applied, use them
            // to get the filtered file list and tell the observers that
            // are interested in that. It shouldn't hurt to do two notifies
            // since only the applied file list will be selected in this one.
            fileServer->SetAppliedFileList(fileServer->GetFilteredFileList());
            fileServer->Notify();
        }
        CATCH(CouldNotConnectException)
        {
            msg.sprintf("VisIt could not set the host back to \"%s\" because "
                        "no metadata server could be launched on that host.",
                        oldHost.c_str());
            Error(msg);
        }
        CATCH(VisItException)
        {
            msg.sprintf("VisIt could not set the host back to \"%s\".",
                        oldHost.c_str());
            Error(msg);
        }
        ENDTRY
    }
    CATCH2(ChangeDirectoryException, cde)
    {
        // Create a message and tell the user.
        QString msgStr;
        msgStr.sprintf("The metadata server running on %s "
            "could not change the current directory to %s.",
            fileServer->GetHost().c_str(), cde.GetDirectory().c_str());
        Error(msgStr);

        // Now set the path to the user's home directory and get the file
        // list and notify.
        fileServer->SetPath(fileServer->GetHomePath());
        fileServer->Notify();

        // Now that the path has been applied, use it to get the filtered
        // file list and tell the other observers.
        fileServer->SetAppliedFileList(fileServer->GetFilteredFileList());
        fileServer->Notify();
    }
    CATCH(GetFileListException)
    {
        // Create a message and tell the user.
        QString msgStr;
        msgStr.sprintf("The metadata server running on %s could not "
             "get the file list for the current directory.",
             fileServer->GetHost().c_str());
        Error(msgStr);
    }
    CATCH(CouldNotConnectException)
    {
        QString msgStr;
        msgStr.sprintf("VisIt could not launch a metadata server on "
                       "host \"%s\".", fileServer->GetHost().c_str());
        Error(msgStr);
    }
    ENDTRY
}

// ****************************************************************************
// Method: QvisGUIApplication::RefreshFileList
//
// Purpose: 
//   This is a Qt slot function that rereads the files in the current directory
//   and adds them to the new list of applied files, which are the files that
//   appear in the selected files list in the file panel.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 23 10:43:58 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Dec 2 16:47:35 PST 2003
//   Added code to compare virtual file definitions before reopening the
//   open database.
//
// ****************************************************************************

void
QvisGUIApplication::RefreshFileList()
{
    // Save the current host and path and virtual file definition.
    std::string  oldHost(fileServer->GetHost());
    std::string  oldPath(fileServer->GetPath());
    stringVector dbDef(fileServer->GetVirtualFileDefinition(fileServer->GetOpenFile()));

    //
    // Create a list of hosts,paths for which we must get a new list of files.
    //
    std::map<QualifiedFilename, bool> paths;
    const QualifiedFilenameVector &appliedFiles = fileServer->GetAppliedFileList();
    int i;
    for(i = 0; i < appliedFiles.size(); ++i)
    {
        QualifiedFilename temp(appliedFiles[i]);
        temp.filename = "a";
        paths[temp] = true;
    }

    //
    // Reread all of the directories that are in the applied file list.
    //
    QualifiedFilenameVector refreshedFiles;
    std::map<QualifiedFilename, bool>::const_iterator pos;
    for(pos = paths.begin(); pos != paths.end(); ++pos)
    {
        TRY
        {
            // Reread the current directory.
            fileServer->SetHost(pos->first.host);
            fileServer->SetPath("");
            fileServer->SetPath(pos->first.path);
            fileServer->SilentNotify();

            // Filter the new list of files add them to the refreshed list.
            QualifiedFilenameVector newFiles(fileServer->GetFilteredFileList());
            for(i = 0; i < newFiles.size(); ++i)
                refreshedFiles.push_back(newFiles[i]);
        }
        CATCH(VisItException)
        {
            ; // do nothing
        }
        ENDTRY
    }

    // Sort the file list before storing it.
    std::sort(refreshedFiles.begin(), refreshedFiles.end());

    //
    // Restore the previous host and path and set the new applied file list.
    //
    fileServer->SetHost(oldHost);
    fileServer->SetPath(oldPath);
    fileServer->SetAppliedFileList(refreshedFiles);
    fileServer->Notify();

    //
    // If the open file is in the list of new files and it is a virtual db,
    // then reopen it so we pick up new time states.
    //
    for(i = 0; i < refreshedFiles.size(); ++i)
    {
        if(refreshedFiles[i].IsVirtual() &&
           refreshedFiles[i] == fileServer->GetOpenFile())
        {
            // Get the new virtual file definiton.
            stringVector newDef(
                fileServer->GetVirtualFileDefinition(fileServer->GetOpenFile()));

            // If the virtual file definitions are different then reopen the
            // database on the viewer so that plots are reexecuted.
            if(dbDef != newDef)
            {
                viewer->ReOpenDatabase(refreshedFiles[i].FullName().c_str(),
                                       false);
            }
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::RefreshFileListAndNextFrame
//
// Purpose: 
//   This is a Qt slot function that rereads the files in the current directory
//   and adds them to the new list of applied files, which are the files that
//   appear in the selected files list in the file panel. After doing all that,
//   it tells the viewer to do a nextframe.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 30 16:57:16 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::RefreshFileListAndNextFrame()
{
    RefreshFileList();
    viewer->AnimationNextFrame();
}

// ****************************************************************************
// Method: QvisGUIApplication::LoadFile
//
// Purpose: 
//   Loads the initial data file if there is one.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 10 15:47:05 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Jun 27 13:14:36 PST 2002
//   I made it set the path back to the original path if we cannot change to
//   the supplied path.
//
//   Brad Whitlock, Mon Apr 28 13:35:56 PST 2003
//   I made it open a virtual file if the file to load belongs to a virtual
//   file. This makes it more convenient to double click files on Windows. I
//   also added code to handle CouldNotConnectException. Finally, I added code
//   to expand the file's path in case it was relative.
//
//   Brad Whitlock, Thu May 15 12:47:34 PDT 2003
//   I made it open a virtual file directly at the selected time state. We
//   no longer have to waste time opening it at the first time state and
//   then advancing to the selected time state.
//
//   Brad Whitlock, Wed Jun 18 10:30:04 PDT 2003
//   Added timing information.
//
//   Brad Whitlock, Fri Oct 10 10:26:26 PDT 2003
//   Changed so that files are added to the applied file list instead of
//   overwriting the applied file list. This way, the method can be called
//   multiple times and the names of each file that we've opened will be
//   in the list.
//
//   Brad Whitlock, Wed Oct 22 12:11:18 PDT 2003
//   Added a flag that determines whether default plots can be added.
//
//   Brad Whitlock, Mon Nov 3 10:56:54 PDT 2003
//   I added code that tells the viewer to open a database because I moved
//   that code out of SetOpenDataFile.
//
//   Brad Whitlock, Mon Nov 10 14:58:32 PST 2003
//   I added code to prevent the file from being loaded if a session file
//   name was given on the command line.
//
// ****************************************************************************

void
QvisGUIApplication::LoadFile(bool addDefaultPlots)
{
    if(!sessionFile.isEmpty())
    {
        Message("When a session file is specified on the command line, "
                "files specified with the -o argument are ignored.");
    }
    else if(!loadFile.Empty())
    {
        int timeid = visitTimer->StartTimer();

        // Temporarily save the old settings.
        std::string oldHost(fileServer->GetHost());
        std::string oldPath(fileServer->GetPath());
        std::string oldFilter(fileServer->GetFilter());

        TRY
        {
            // In case the path was relative, expand the path to a full path.
            loadFile.path = fileServer->ExpandPath(loadFile.path);

            // Switch to the right host/path and get the file list.
            fileServer->SetHost(loadFile.host);
            fileServer->SetPath(loadFile.path);
            fileServer->Notify();

            // Get the filtered file list and look to see if it contains the
            // file that we want to load.
            QualifiedFilenameVector files(fileServer->GetFilteredFileList());
            bool fileInList = false;
            int  timeState  = 0;
            QualifiedFilenameVector::const_iterator pos;
            for(pos = files.begin(); pos != files.end() && !fileInList; ++pos)
            {
                bool sameHost = loadFile.host == pos->host;
                bool samePath = loadFile.path == pos->path;

                if(sameHost && samePath)
                {
                    bool sameFile = loadFile.filename == pos->filename;
                    if(pos->IsVirtual())
                    {
                        // Get the list of files in the virtual file.
                        stringVector def(fileServer->GetVirtualFileDefinition(*pos));

                        // See if the file that we want to open is in the virtual
                        // file definition.                       
                        for(int state = 0; state < def.size(); ++state)
                        {
                            if(loadFile.filename == def[state])
                            {
                                fileInList = true;
                                timeState = state;
                                loadFile.filename = pos->filename;
                                break;
                            }
                        }
                    }
                    else if(sameFile)
                        fileInList = true;
                }
            }

            // Make sure that we put our file in the applied files list if it
            // is not already in the list.
            if(!fileInList)
                files.push_back(loadFile);

            //
            // Combine the files with the applied files so we can call this
            // method repeatedly and have all of the files that we've opened 
            // this way be in the applied file list.
            //
            fileServer->SetAppliedFileList(
                CombineQualifiedFilenameVectors(files,
                    fileServer->GetAppliedFileList()));
            fileServer->Notify();

            // Tell the viewer to show all of its windows since launching
            // an engine could take a while and we want the viewer window
            // to still pop up at roughly the same time as the gui.
            viewer->ShowAllWindows();

            // Try and open the data file for plotting.
            SetOpenDataFile(loadFile, timeState);

            // Tell the viewer to open the file too.
            viewer->OpenDatabase(loadFile.FullName().c_str(), timeState,
                                 addDefaultPlots);
        }
        CATCH2(BadHostException, bhe)
        {
            QString msg;
            msg.sprintf("Hostname \"%s\" is not a recognized host.",
                         bhe.GetHostName().c_str());
            Error(msg);

            // Set the file server host, etc. back to the previous values.
            fileServer->SetHost(oldHost);
            fileServer->SetPath(oldPath);
            fileServer->SetFilter(oldFilter);
            fileServer->Notify();

            // Now that the path and filter have been applied, use them
            // to get the filtered file list and tell the observers that
            // are interested in that. It shouldn't hurt to do two notifies
            // since only the applied file list will be selected in this one.
            fileServer->SetAppliedFileList(fileServer->GetFilteredFileList());
            fileServer->Notify();
        }
        CATCH2(ChangeDirectoryException, cde)
        {
            // Create a message and tell the user.
            QString msgStr;
            msgStr.sprintf("The metadata server running on %s "
                "could not change the current directory to %s.",
                fileServer->GetHost().c_str(), cde.GetDirectory().c_str());
            Error(msgStr);

            // Now set the path to the user's home directory and get the file
            // list and notify.
            fileServer->SetPath(oldPath);
            fileServer->Notify();

            // Now that the path has been applied, use it to get the filtered
            // file list and tell the other observers.
            fileServer->SetAppliedFileList(fileServer->GetFilteredFileList());
            fileServer->Notify();
        }
        CATCH(GetFileListException)
        {
            // Create a message and tell the user.
            QString msgStr;
            msgStr.sprintf("The metadata server running on %s could not "
                "get the file list for the current directory.",
                fileServer->GetHost().c_str());
            Error(msgStr);
        }
        CATCH(CouldNotConnectException)
        {
            QString msgStr;
            msgStr.sprintf("VisIt could not open %s because it could not "
                           "launch a metadata server on host \"%s\".",
                           loadFile.FullName().c_str(),
                           fileServer->GetHost().c_str());
            Error(msgStr);
        }
        ENDTRY

        visitTimer->StopTimer(timeid, "Loading initial file.");
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::ReadFromViewer
//
// Purpose: 
//   This is a Qt slot function that tells the viewer that it has
//   input that it can process.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:18:20 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Apr 24 10:44:59 PDT 2001
//   Added code to print a message when the viewer dies.
//
//   Brad Whitlock, Fri Jul 25 13:08:34 PST 2003
//   Added a check that allows us to ignore a socket read.
//
//   Brad Whitlock, Mon Jul 28 14:41:10 PST 2003
//   Made the e-mail address be visit-help@llnl.gov.
//
//   Brad Whitlock, Thu Jul 31 15:15:38 PST 2003
//   Fixed a bug that prevented full initialization of the GUI on Windows.
//
// ****************************************************************************

void
QvisGUIApplication::ReadFromViewer(int)
{
    if(allowSocketRead)
    {
        TRY
        {
            // Tell the viewer proxy that it has input to process.
            viewer->ProcessInput();
        }
        CATCH(LostConnectionException)
        {
            cerr << "VisIt's viewer exited abnormally! Aborting the Graphical "
                 << "User Interface. VisIt's developers may be reached at "
                 << "\"visit-help@llnl.gov\"."
                 << endl;
            viewerIsAlive = false;

            // Re-throw the exception.
            RETHROW;
        }
        ENDTRY
    }
    else
    {
        debug1 << "Reading from the viewer's socket is currently not allowed!"
               << endl;
#if defined(_WIN32)
        // If we ignore the socket read on Windows, we don't tend to keep
        // getting the message so call this function again though the event
        // loop using a timer.
        QTimer::singleShot(10, this, SLOT(DelayedReadFromViewer()));
#endif
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::DelayedReadFromViewer
//
// Purpose: 
//   Tries to read from the viewer.
//
// Note:       This method is called from the event loop in response to having
//             been scheduled earlier by a premature return from ReadFromViewer.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 31 15:16:30 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::DelayedReadFromViewer()
{
     ReadFromViewer(0);
}

// ****************************************************************************
// Method: QvisGUIApplication::SaveSettings
//
// Purpose: 
//   This is a Qt slot function that writes the VisIt config file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 3 14:32:52 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 17:20:06 PST 2001
//   Made the GUI write a different config file than the viewer.
//
//   Brad Whitlock, Thu Sep 5 16:31:33 PST 2002
//   I added status messages for saving settings.
//
// ****************************************************************************

void
QvisGUIApplication::SaveSettings()
{
    // Tell the user that we're saving settings.
    Status("Saving settings");

    // Write out the GUI's portion of the config file.
    char *configFile = GetDefaultConfigFile(VISIT_GUI_CONFIG_FILE);
    WriteConfigFile(configFile);
    delete [] configFile;

    // Tell the viewer to write out its portion of the config file.
    viewer->WriteConfigFile();

    // Clear the status bar.
    ClearStatus();
}

// ****************************************************************************
// Method: QvisGUIApplication::ActivatePlotWindow
//
// Purpose: 
//   This is a Qt slot function that is called when a plot window must be
//   activated.
//
// Arguments:
//   index : The index of the plot window that needs to be activated.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 16:49:25 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::ActivatePlotWindow(int index)
{
    if(index >= 0 && index < plotWindows.size())
    {
        plotWindows[index]->show();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::ActivateOperatorWindow
//
// Purpose: 
//   This is a Qt slot function that is called when an operator window must be
//   activated.
//
// Arguments:
//   index : The index of the operator window that needs to be activated.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 23 16:49:25 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::ActivateOperatorWindow(int index)
{
    if(index >= 0 && index < operatorWindows.size())
    {
        operatorWindows[index]->show();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::IconifyWindows
//
// Purpose: 
//   This is a Qt slot function that iconifies all of the GUI windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:38:44 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 12:05:00 PDT 2001
//   Rewrote how windows are iconified for normal windows.
//
//   Brad Whitlock, Wed Sep 25 10:08:12 PDT 2002
//   Since windows are now children of the main window on the Windows
//   platform, do not iconify them because the main window handles it.
//
// ****************************************************************************

void
QvisGUIApplication::IconifyWindows()
{
    // Iconify the main window.
    mainWin->showMinimized();

#if !defined(_WIN32)
    int index;

    // Iconify all of the regular windows.
    for(index = 0; index < otherWindows.size(); ++index)
        otherWindows[index]->showMinimized();

    // Iconify all of the plot windows.
    for(index = 0; index < plotWindows.size(); ++index)
        plotWindows[index]->showMinimized();

    // Iconify all of the operator windows.
    for(index = 0; index < operatorWindows.size(); ++index)
        operatorWindows[index]->showMinimized();
#endif

    // Iconify the viewer windows
    viewer->IconifyAllWindows();
}

// ****************************************************************************
// Method: QvisGUIApplication::DeIconifyWindows
//
// Purpose: 
//   This is a Qt slot function that de-iconifies all of the windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:39:37 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Sep 6 12:05:57 PDT 2001
//   Rewrote how regular windows are deiconified.
//
//   Brad Whitlock, Wed Sep 25 10:08:12 PDT 2002
//   Since windows are now children of the main window on the Windows
//   platform, do not deiconify them because the main window handles it.
//
// ****************************************************************************

void
QvisGUIApplication::DeIconifyWindows()
{
    // Deiconify all of the viewer windows.
    viewer->DeIconifyAllWindows();

    // Deiconify the main window.
    mainWin->showNormal();

#if !defined(_WIN32)
    int index;

    // deIconify all of the regular windows.
    for(index = 0; index < otherWindows.size(); ++index)
        otherWindows[index]->showNormal();

    // Iconify all of the plot windows.
    for(index = 0; index < plotWindows.size(); ++index)
        plotWindows[index]->showNormal();

    // Iconify all of the operator windows.
    for(index = 0; index < operatorWindows.size(); ++index)
        operatorWindows[index]->showNormal();
#endif
}

// ****************************************************************************
// Method: QvisGUIApplication::AboutVisIt
//
// Purpose: 
//   This a Qt slot function that tells the splash screen to show itself.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 11:41:23 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 11:54:27 PDT 2003
//   I added code to create the splashscreen if it has not been created.
//
// ****************************************************************************

void
QvisGUIApplication::AboutVisIt()
{
    if(splash == 0)
        splash = new SplashScreen(true, "splash");

    splash->About();
}

// ****************************************************************************
// Method: QvisGUIApplication::SplashScreenProgress
//
// Purpose: 
//   This method sends progress messages to the splash screen.
//
// Arguments:
//   msg  : The message to send to the splashscreen.
//   prog : The percentage of completion.
//
// Programmer: Sean Ahern
// Creation:   Thu Sep 13 11:41:23 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Jun 19 11:59:54 PDT 2003
//   I changed it so the splashscreen does not have to exist. I also added
//   code that processes one event so the text that we send to the splash-
//   screen gets drawn since this method is usually called outside the
//   event loop.
//
// ****************************************************************************

void
QvisGUIApplication::SplashScreenProgress(const char *msg, int prog)
{
    if(splash)
        splash->Progress(msg, prog);
}

// ****************************************************************************
// Method: QvisGUIApplication::SaveWindow
//
// Purpose: 
//   This is a Qt slot that tells the viewer to print the active window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 12:40:11 PDT 2002
//
// Modifications:
//   
//   Hank Childs, Wed Oct 15 08:50:14 PDT 2003
//   Turn off TIFFs where the VTK library doesn't work.
//
//   Kathleen Bonnell, Fri Oct 24 16:34:35 PDT 2003 
//   Remove TIFF fix, VTK now uses TIFF library.
//
// ****************************************************************************

void
QvisGUIApplication::SaveWindow()
{
    viewer->SaveWindow();
}

// ****************************************************************************
// Method: QvisGUIApplication::SetPrinterOptions
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to set its print options.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 12:40:41 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue May 14 15:11:55 PST 2002
//   Made it work with Qt 3.0.2.
//
// ****************************************************************************

void
QvisGUIApplication::SetPrinterOptions()
{
    if(printer->setup(mainWin))
    {
        //
        // Send all of the Qt printer options to the viewer
        //
        PrinterAttributes *p = viewer->GetPrinterAttributes();
        QPrinterToPrinterAttributes(printer, p);
        p->SetCreator(viewer->GetLocalUserName());
        printerObserver->SetUpdate(false);
        p->Notify();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::PrintWindow
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to print the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 12:41:13 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::PrintWindow()
{
    viewer->PrintWindow();
}

// ****************************************************************************
// Function: QPrinterToPrinterAttributes
//
// Purpose:
//   Copies the contents of a QPrinter object into a PrinterAttributes object.
//
// Arguments:      
//   printer : The source QPrinter object.
//   p       : The destination PrinterAttributes object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 13:49:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

static void
QPrinterToPrinterAttributes(QPrinter *printer, PrinterAttributes *p)
{
    if(!printer->printerName().isNull())
        p->SetPrinterName(printer->printerName().latin1());
    if(!printer->outputFileName().isNull())
        p->SetOutputToFileName(printer->outputFileName().latin1());
    if(!printer->printProgram().isNull())
        p->SetPrintProgram(printer->printProgram().latin1());
    if(!printer->docName().isNull())
        p->SetDocumentName(printer->docName().latin1());
    p->SetOutputToFile(printer->outputToFile());
    p->SetNumCopies(printer->numCopies());
    p->SetPortrait(printer->orientation() == QPrinter::Portrait);
    p->SetPrintColor(printer->colorMode() == QPrinter::Color);
    p->SetPageSize(printer->pageSize());
}

// ****************************************************************************
// Function: PrinterAttributesToQPrinter
//
// Purpose:
//   Copies the contents of a PrinterAttributes object into a QPrinter object.
//
// Arguments:      
//   p       : The source PrinterAttributes object.
//   printer : The destination QPrinter object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 13:49:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************

static void
PrinterAttributesToQPrinter(PrinterAttributes *p, QPrinter *printer)
{
    printer->setPrinterName(p->GetPrinterName().c_str());
    printer->setPrintProgram(p->GetPrintProgram().c_str());
    printer->setCreator(p->GetCreator().c_str());
    printer->setDocName(p->GetDocumentName().c_str());
    printer->setNumCopies(p->GetNumCopies());
    printer->setOrientation(p->GetPortrait() ? QPrinter::Portrait :
        QPrinter::Landscape);
    printer->setFromTo(1, 1);
    printer->setColorMode(p->GetPrintColor() ? QPrinter::Color :
        QPrinter::GrayScale);
    printer->setOutputFileName(p->GetOutputToFileName().c_str());
    printer->setOutputToFile(p->GetOutputToFile());
    printer->setPageSize((QPrinter::PageSize)p->GetPageSize());
}

// ****************************************************************************
// Function: QvisGUIApplication::UpdatePrinterAttributes
//
// Purpose:
//   This is a callback function that is called when the viewer's print
//   attributes change. This function copies the viewer's print attributes
//   into the application's QPrinter object.
//
// Arguments:      
//   subj : A pointer to the PrinterAttributes object that caused the call.
//   data : A pointer to the QPrinter that we're gonna update.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 20 13:51:06 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::UpdatePrinterAttributes(Subject *subj, void *data)
{
    PrinterAttributes *p = (PrinterAttributes *)subj;
    QPrinter *printer = (QPrinter *)data;
    PrinterAttributesToQPrinter(p, printer);
}
