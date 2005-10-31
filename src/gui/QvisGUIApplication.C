#include <stdio.h>
#include <algorithm>
#include <map>

#include <visit-config.h> // To get the version number
#include <qcolor.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qprintdialog.h>
#include <qprinter.h>
#include <qprocess.h>
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

#if defined(Q_WS_MACX)
// On MacOS X, we manage the printer options window instead of letting
// Qt do it when we use the Mac or Aqua style.
#include <Carbon/Carbon.h>
#include <qstyle.h>
#endif

#include <qtimer.h>
#include <QvisGUIApplication.h>

#include <PlotPluginInfo.h>
#include <PlotPluginManager.h>
#include <OperatorPluginInfo.h>
#include <OperatorPluginManager.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ChangeDirectoryException.h>
#include <DatabaseCorrelationList.h>
#include <GetFileListException.h>
#include <SaveWindowAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <AppearanceAttributes.h>
#include <HostProfile.h>
#include <GlobalLineoutAttributes.h>
#include <InteractorAttributes.h>
#include <MovieAttributes.h>
#include <ObserverToCallback.h>
#include <PickAttributes.h>
#include <QueryAttributes.h>
#include <PlotList.h>
#include <Plot.h>
#include <PrinterAttributes.h>
#include <RenderingAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <WindowInformation.h>

#include <QvisApplication.h>
#include <FileServerList.h>
#include <QvisAnimationWindow.h>
#include <QvisAnnotationWindow.h>
#include <QvisAppearanceWindow.h>
#include <QvisColorTableWindow.h>
#include <QvisCommandWindow.h>
#include <QvisDatabaseCorrelationListWindow.h>
#include <QvisEngineWindow.h>
#include <QvisExportDBWindow.h>
#include <QvisExpressionsWindow.h>
#include <QvisFileInformationWindow.h>
#include <QvisFileSelectionWindow.h>
#include <QvisGlobalLineoutWindow.h>
#include <QvisHelpWindow.h>
#include <QvisHostProfileWindow.h>
#include <QvisInteractorWindow.h>
#include <QvisInterpreter.h>
#include <QvisKeyframeWindow.h>
#include <QvisLightingWindow.h>
#include <QvisMainWindow.h>
#include <QvisMaterialWindow.h>
#include <QvisMessageWindow.h>
#include <QvisMovieProgressDialog.h>
#include <QvisOutputWindow.h>
#include <QvisPickWindow.h>
#include <QvisPlotManagerWidget.h>
#include <QvisPluginWindow.h>
#include <QvisPreferencesWindow.h>
#include <QvisQueryWindow.h>
#include <QvisRenderingWindow.h>
#include <QvisSaveMovieWizard.h>
#include <QvisSaveWindow.h>
#include <QvisSessionFileDatabaseLoader.h>
#include <QvisSimulationWindow.h>
#include <QvisSubsetWindow.h>
#include <QvisQueryOverTimeWindow.h>
#include <QvisVariableButton.h>
#include <QvisViewWindow.h>
#include <QvisVisItUpdate.h>
#include <QvisWizard.h>

#include <SplashScreen.h>
#include <WindowMetrics.h>

#include <BadHostException.h>
#include <CancelledConnectException.h>
#include <CouldNotConnectException.h>
#include <IncompatibleVersionException.h>
#include <IncompatibleSecurityTokenException.h>
#include <LostConnectionException.h>
#include <TimingsManager.h>
#include <DebugStream.h>
#include <Utility.h>

#if defined(_WIN32)
#include <windows.h> // for LoadLibrary
#endif

#include <snprintf.h>

// Some defines
#define VISIT_GUI_CONFIG_FILE "guiconfig"
#define VIEWER_READY_TAG       100
#define SET_FILE_HIGHLIGHT_TAG 101
#define LOAD_ACTIVESOURCE_TAG  102
#define INTERPRETER_SYNC_TAG   103

#define WINDOW_FILE_SELECTION    0
#define WINDOW_FILE_INFORMATION  1
#define WINDOW_MESSAGE           2
#define WINDOW_OUTPUT            3
#define WINDOW_HOSTPROFILES      4
#define WINDOW_SAVE              5
#define WINDOW_ENGINE            6
#define WINDOW_ANIMATION         7
#define WINDOW_ANNOTATION        8 
#define WINDOW_COLORTABLE        9 
#define WINDOW_EXPRESSIONS      10  
#define WINDOW_SUBSET           11
#define WINDOW_PLUGINMANAGER    12    
#define WINDOW_VIEW             13
#define WINDOW_APPEARANCE       14 
#define WINDOW_KEYFRAME         15
#define WINDOW_LIGHTING         16
#define WINDOW_GLOBALLINEOUT    17    
#define WINDOW_MATERIALOPTIONS  18      
#define WINDOW_PICK             19
#define WINDOW_HELP             20
#define WINDOW_QUERY            21
#define WINDOW_PREFERENCES      22  
#define WINDOW_RENDERING        23
#define WINDOW_CORRELATION      24  
#define WINDOW_TIMEQUERY        25
#define WINDOW_INTERACTOR       26
#define WINDOW_SIMULATION       27
#define WINDOW_EXPORT_DB        28
#define WINDOW_COMMAND          29

const char *QvisGUIApplication::windowNames[] = {
"File selection",
"File information",
"Information",
"Output",
"Host profiles",
"Set save options",
"Compute engines",
"Animation",
"Annotation",
"Color tables",
"Expressions",
"Subset",
"Plugin Manager",
"View",
"Appearance",
"Keyframe Editor",
"Lighting",
"Lineout options",
"Material Reconstruction Options",
"Pick",
"Help",
"Query",
"Preferences",
"Rendering options",
"Database correlation list",
"QueryOverTime",
"Interactors",
"Simulations",
"Export Database",
"Commands"
};

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
//   Brad Whitlock, Fri Mar 12 13:47:37 PST 2004
//   I added keepAliveTimer.
//
//   Brad Whitlock, Fri Apr 9 14:09:03 PST 2004
//   I added allowFileSelectionChange.
//
//   Jeremy Meredith, Wed Aug 25 10:54:58 PDT 2004
//   Add observers for the new SIL and metadata attributes of the viewer proxy.
//
//   Brad Whitlock, Wed Feb 9 17:53:05 PST 2005
//   Added VisItUpdate and developmentVersion.
//
//   Brad Whitlock, Wed Apr 20 16:48:23 PST 2005
//   Added saveMovieWizard, movieAtts, interpreter, and observer for
//   client method.
//
//   Brad Whitlock, Thu Jul 14 11:00:40 PDT 2005
//   Removed developmentVersion.
//
//   Brad Whitlock, Thu Oct 27 14:17:01 PST 2005
//   Added sessionFileHelper.
//
//   Brad Whitlock, Fri Oct 28 13:50:21 PST 2005
//   Added movieArguments.
//
// ****************************************************************************

QvisGUIApplication::QvisGUIApplication(int &argc, char **argv) :
    ConfigManager(), GUIBase(), message(), plotWindows(),
    operatorWindows(), otherWindows(), foregroundColor(), backgroundColor(),
    applicationStyle(), loadFile(), sessionFile(), movieArguments()
{
    completeInit = visitTimer->StartTimer();
    int total = visitTimer->StartTimer();

    // Tell Qt that we want lots of colors.
    QApplication::setColorSpec(QApplication::ManyColor);

    // The viewer is initially not alive.
    viewerIsAlive = false;
    closeAllClients = true;
    viewerInitiatedQuit = false;
    reverseLaunch = false;

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
    keepAliveTimer = 0;
    allowFileSelectionChange = true;
    visitUpdate = 0;
    saveMovieWizard = 0;    
    interpreter = 0;
    movieProgress = 0;
    sessionFileHelper = 0;

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

    // Add some left-over arguments to the movie script.
    AddMovieArguments(argc, argv);

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
    // Create an observer for the meta data attributes.
    //
    metaDataObserver = new ObserverToCallback(viewer->GetDatabaseMetaData(),
         QvisGUIApplication::UpdateMetaDataAttributes, this);
    SILObserver = new ObserverToCallback(viewer->GetSILAtts(),
         QvisGUIApplication::UpdateMetaDataAttributes, this);

    //
    // Create an observer for the client method attributes.
    //
    clientMethodObserver = new ObserverToCallback(viewer->GetClientMethod(),
         QvisGUIApplication::ClientMethodCallback, this);

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
//   Brad Whitlock, Tue Apr 27 14:23:59 PST 2004
//   I prevented windows from being deleted on MacOS X since it was causing
//   a crash.
//
//   Brad Whitlock, Wed Apr 20 16:48:11 PST 2005
//   Added interpreter. I also made it possible to detach from the viewer
//   rather than always telling it to close.
//
// ****************************************************************************

QvisGUIApplication::~QvisGUIApplication()
{
#if !defined(_WIN32) && !defined(__APPLE__)
    // Delete the windows.
    int i;
    for(WindowBaseMap::iterator pos = otherWindows.begin();
        pos != otherWindows.end(); ++pos)
    {
        delete pos->second;
    }
    for(i = 0; i < plotWindows.size(); ++i)
    {
        if(plotWindows[i] != 0)
            delete plotWindows[i];
    }
    for(i = 0; i < operatorWindows.size(); ++i)
    {
        if(operatorWindows[i] != 0)
            delete operatorWindows[i];
    }
#endif

    // Delete the file server
    delete fileServer;
    fileServer = 0;

    // Close down the viewer and delete it.
    if(viewerIsAlive)
    {
        if(viewerInitiatedQuit)
        {
            debug1 << "Quitting because viewer told us to." << endl;
        }
        else
        {
            if(closeAllClients)
            {
                debug1 << "Telling viewer to close." << endl;
                viewer->Close();
            }
            else
            {
                debug1 << "Telling viewer to detach this GUI." << endl;
                viewer->Detach();
            }
        }
    }
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
//   Hank Childs, Sat Jan 24 10:45:05 PST 2004
//   Add a stage where we tell the mdserver to load its plugins.
//
//   Brad Whitlock, Thu May 6 14:50:20 PST 2004
//   Moved the stage where we tell the mdserver to load plugins and I added
//   more complete timing information.
//
// ****************************************************************************

void
QvisGUIApplication::HeavyInitialization()
{
    int timeid;
    bool moreInit = true;
    bool gotoNextStage = true;
    WindowMetrics *wm = 0;

    // Create some timers.
    if(heavyInitStage == 0) 
        stagedInit = visitTimer->StartTimer();
    timeid = visitTimer->StartTimer();
    
    debug4 << "QvisGUIApplication::HeavyInitialization: heavyInitStage="
           << heavyInitStage << endl;

    switch(heavyInitStage)
    {
    case 0:
        SplashScreenProgress("Calculating window metrics...", 5);
        // Calculate the window metrics
        
        wm = WindowMetrics::Instance();
        visitTimer->StopTimer(timeid, "stage 0 - Calculating window metrics");

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
        visitTimer->StopTimer(timeid, "stage 1");
        break;
    case 2:
        // Create the main window.
        CreateMainWindow();
        visitTimer->StopTimer(timeid, "stage 2 - Creating main window");
        break;
    case 3:
        SplashScreenProgress("Starting viewer...", 12);
        visitTimer->StopTimer(timeid, "stage 3");
        break;
    case 4:
        // Launch the viewer.
        LaunchViewer();
        visitTimer->StopTimer(timeid, "stage 4 - Launching viewer");
        break;
    case 5:
        // Create the socket notifier and hook it up to the viewer.
        fromViewer = new QSocketNotifier(
            viewer->GetWriteConnection()->GetDescriptor(),
            QSocketNotifier::Read);
        connect(fromViewer, SIGNAL(activated(int)),
                this, SLOT(ReadFromViewer(int)));

        SplashScreenProgress("Starting metadata server...", 32);
        visitTimer->StopTimer(timeid, "stage 5");
        break;
    case 6:
        // Initialize the file server. This connects the GUI to the mdserver
        // running on localhost.

        fileServer->SetConnectCallback(StartMDServer, (void *)viewer);
        fileServer->Initialize();
        visitTimer->StopTimer(timeid, "stage 6 - Launching mdserver");
        break;
    case 7:
        SplashScreenProgress("Launched the metadata server...", 52);
        visitTimer->StopTimer(timeid, "stage 7");
        break;
    case 8:
        // Set the current directory in the loadFile if its path is empty.
        if(loadFile.host == fileServer->GetHost() &&
           loadFile.path == "")
        {
            SplashScreenProgress("Getting the path...", 55);
            loadFile.path = fileServer->GetPath();
        }
        visitTimer->StopTimer(timeid, "stage 8");
        break;
    case 9:
        SplashScreenProgress("Creating crucial windows...", 60);
        visitTimer->StopTimer(timeid, "stage 9");
        break;
    case 10:
        // Create the GUI's crucial windows.
        SetupWindows();
        visitTimer->StopTimer(timeid, "stage 10 - Creating crucial windows");
        break;
    case 11:
        SplashScreenProgress("Loading plugin information...", 70);
        visitTimer->StopTimer(timeid, "stage 11");
        break;
    case 12:
        // Load plugin info
        PlotPluginManager::Initialize(PlotPluginManager::GUI);
        OperatorPluginManager::Initialize(OperatorPluginManager::GUI);
        visitTimer->StopTimer(timeid, "stage 12 - Loading plugin info");
        break;
    case 13:
        SplashScreenProgress("Processing config file...", 80);
        visitTimer->StopTimer(timeid, "stage 13");
        break;
    case 14:
        // Process the config file settings.
        ProcessConfigSettings(systemSettings, true);
        ProcessConfigSettings(localSettings, false);
        visitTimer->StopTimer(timeid, "stage 14 - Processing config file");
        break;
    case 15:
        // Tell the mdserver to load its plugins. It can't load plugins before
        // we process the config settings though because that sets up the 
        // file server, which sets and reads files for the active directory.
        // Loading plugins before that stalls the gui if the mdserver is
        // slow about loading plugins.
        fileServer->LoadPlugins();
        viewer->UpdateDBPluginInfo(fileServer->GetHost());
        visitTimer->StopTimer(timeid, "stage 15 - Telling mdserver to load plugins");
        break;
    case 16:
        // Let the GUI read from the viewer now.
        allowSocketRead = true;

        // Create a trigger that will cause the GUI to finish initialization
        // when the viewer is ready.
        Synchronize(VIEWER_READY_TAG);
        moreInit = false;
        visitTimer->StopTimer(timeid, "stage 16");
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
    else
        visitTimer->StopTimer(stagedInit, "HeavyInitialization");
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
//   Brad Whitlock, Thu May 6 14:51:29 PST 2004
//   I removed the timers because they're now 1 level up.
//
//   Brad Whitlock, Tue May 3 16:16:51 PST 2005
//   I passed qt_argc, qt_argv into ViewerProxy::Launch so we can support
//   reverse launching without the GUI really knowing about it.
//
// ****************************************************************************

void
QvisGUIApplication::LaunchViewer()
{
    TRY
    {
        // Add some more arguments and launch the viewer.
        AddViewerSpaceArguments();
        viewer->AddArgument("-defer");
        viewer->Create(&qt_argc, &qt_argv);
        viewerIsAlive = true;

        // Set the default user name in the host profiles.
        HostProfile::SetDefaultUserName(viewer->GetLocalUserName());
    }
    CATCH(IncompatibleVersionException)
    {
        cerr << "The version numbers of the GUI and the viewer do not match."
             << endl;
        // Re-throw the exception.
        RETHROW;
    }
    CATCH(IncompatibleSecurityTokenException)
    {
        cerr << "The viewer did not return the proper credentials."
             << endl;
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
//   Brad Whitlock, Fri Apr 9 12:24:44 PDT 2004
//   Added a tag to set the file panel's file highlight mode.
//
//   Brad Whitlock, Fri May 6 11:17:04 PDT 2005
//   Added a tag to make the interpreter execute some commands.
//
// ****************************************************************************

void
QvisGUIApplication::HandleSynchronize(int val)
{
    if(val == VIEWER_READY_TAG)
    {
        QTimer::singleShot(10, this, SLOT(FinalInitialization()));
    }
    else if(val == SET_FILE_HIGHLIGHT_TAG)
    {
        // Set the appropriate file highlight for the file panel now that
        // we're ready for user operation.
        mainWin->SetAllowFileSelectionChange(allowFileSelectionChange);
    }
    else if(val == LOAD_ACTIVESOURCE_TAG)
    {
        // Check the window information for the active source.
        loadFile = QualifiedFilename(viewer->GetWindowInformation()->
            GetActiveSource());
        LoadFile(loadFile, false);
    }
    else if(val == INTERPRETER_SYNC_TAG)
    {
        QTimer::singleShot(10, interpreter, SLOT(ProcessCommands()));
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
// Method: QvisGUIApplication::ClientMethodCallback
//
// Purpose: 
//   This method handles a clientmethod object.
//
// Arguments:
//   s    : the client method object.
//   data : callback data.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 18:03:04 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::ClientMethodCallback(Subject *s, void *data)
{
    QvisGUIApplication *app = (QvisGUIApplication *)data;
    app->HandleClientMethod();
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
//   Brad Whitlock, Fri Mar 12 14:15:35 PST 2004
//   Added code to create a timer that is used to send keep alive signals
//   to the mdservers.
//
//   Brad Whitlock, Wed May 5 08:47:22 PDT 2004
//   Removed code to setup printer so it is done later on demand. I also added
//   a new stage that creates non-plugin GUI windows that the config file
//   wants to be visible. Finally, I made the main window be shown later to
//   speed up launch time on MacOS X.
//
//   Brad Whitlock, Mon Aug 30 08:54:17 PDT 2004
//   Prevented an error message from being shown unintentionally.
//
//   Brad Whitlock, Fri Jan 7 16:34:32 PST 2005
//   Changed the code so dismissing the splashscreen is the last thing done
//   on MacOS X so we are more likely to get the GUI's menu and not the 
//   viewer's menu.
//
//   Brad Whitlock, Wed Feb 16 11:03:57 PDT 2005
//   Added code to show the release notes the first time a the use runs a
//   new version of VisIt.
//
//   Brad Whitlock, Thu Jul 14 11:01:26 PDT 2005
//   I made it use GetIsDevelopmentVersion.
//
// ****************************************************************************

void
QvisGUIApplication::FinalInitialization()
{
    bool moreInit = true;
    
    // Create time timers.
    if(initStage == 0)
        stagedInit = visitTimer->StartTimer();
    int timeid = visitTimer->StartTimer();

    debug4 << "QvisGUIApplication::FinalInitialization: initStage="
           << initStage << endl;

    switch(initStage)
    {
    case 0:
        allowSocketRead = false;

#ifndef Q_WS_MACX
        // Tell the viewer to show all of its windows.
        viewer->ShowAllWindows();

        // Show the main window
        mainWin->show();
#else
        // On MacOS X, just tell the viewer to show for now.
        viewer->ShowAllWindows();
#endif

        // Indicate that future messages should go to windows and not
        // to the console.
        writeToConsole = false;

        visitTimer->StopTimer(timeid, "stage 0 - Showing windows");
        break;
    case 1:
        //
        // Create the non-plugin GUI windows that should be visible according
        // to the config file.
        //
        SplashScreenProgress("Creating visible windows...", 90);
        CreateInitiallyVisibleWindows(localSettings);
        visitTimer->StopTimer(timeid, "stage 1 - Creating visible windows");
        break;
    case 2:
        SplashScreenProgress("Processing window configs...", 95);
        // Process the window config file settings.
        ProcessWindowConfigSettings(systemSettings);
        ProcessWindowConfigSettings(localSettings);
        visitTimer->StopTimer(timeid, "stage 2 - Processing window configs");
        break;
    case 3:
        // Show that we're ready.
        SplashScreenProgress("VisIt is ready.", 100);
        visitTimer->StopTimer(timeid, "stage 3");
        break;
    case 4:
#ifndef Q_WS_MACX
        // If we're not on MacOS X, hide the splashscreen now.
        if(splash)
            splash->hide();
#endif
        visitTimer->StopTimer(timeid, "stage 4 - Hiding splashscreen");
        break;
    case 5:
        // Load the initial data file.
        if(!loadFile.Empty() && !sessionFile.isEmpty())
        {
            Message("When a session file is specified on the command line, "
                    "files specified with the -o argument are ignored.");
        }
        else
            LoadFile(loadFile, true);
        visitTimer->StopTimer(timeid, "stage 5 - LoadFile");
        break;
    case 6:
        // Load the initial session file.
        RestoreSessionFile(sessionFile);
        visitTimer->StopTimer(timeid, "stage 6 - RestoreSessionFile");
        break;
    case 7:
        // Create a timer that will send keep alive signals to the mdservers
        // every 5 minutes.
        keepAliveTimer = new QTimer(this, "keepAliveTimer");
        connect(keepAliveTimer, SIGNAL(timeout()),
                this, SLOT(SendKeepAlives()));
        keepAliveTimer->start(5 * 60 * 1000);
        visitTimer->StopTimer(timeid, "stage 7 - Create keepalive");
        break;
    case 8:
#ifdef Q_WS_MACX
        // In the MacOS X version, show the main window last because it 
        // trims off about 1.5 seconds off of the launch.
        mainWin->show();
#endif
        allowSocketRead = true;
        visitTimer->StopTimer(timeid, "stage 8");
        break;
    case 9:
#ifdef Q_WS_MACX
        // On MacOS X, we hide the splashscreen last thing so we are very
        // near 100% likely to get the GUI's menu in the main Mac menu.
        if(splash)
            splash->hide();
        visitTimer->StopTimer(timeid, "stage 9 - Hiding splashscreen");
#else
        visitTimer->StopTimer(timeid, "stage 9 - no op");
#endif
        break;
    case 10:
        // Show the release notes if this is the first time that the
        // user has run this version of VisIt.
        if(GetIsDevelopmentVersion())
        {
            // Make sure that we don't allow updates in development versions.
            mainWin->updateNotAllowed();
        }
        else
        { 
            ConfigStateEnum code;
            ConfigStateIncrementRunCount(code);
            if(code == CONFIGSTATE_FIRSTTIME)
                QTimer::singleShot(1000, this, SLOT(displayReleaseNotesIfAvailable()));
        }

        // If we were reverse launched, do a synchronize to try and open
        // the active source later.
        if(reverseLaunch)
            Synchronize(LOAD_ACTIVESOURCE_TAG);

        visitTimer->StopTimer(timeid, "stage 10 - Incrementing run count");
        visitTimer->StopTimer(stagedInit, "FinalInitialization");
        visitTimer->StopTimer(completeInit, "VisIt to be ready");
        moreInit = false;
        ++initStage;
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
// Method: QvisGUIApplication::Quit
//
// Purpose: 
//   This is a Qt slot function that is called when the user wants to
//   quit VisIt.
//
// Note:       If there's more than 1 client attached to the viewer then ask
//             the user if he wants to totally quit or just detach.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 12:18:37 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::Quit()
{
    if(!viewerInitiatedQuit)
    {
        if(viewer->GetClientInformationList()->GetNumClientInformations() > 1)
        {
            // disconnect some slots so we don't keep getting the dialog.
            disconnect(mainApp, SIGNAL(aboutToQuit()), mainApp, SLOT(closeAllWindows()));
            disconnect(mainApp, SIGNAL(lastWindowClosed()), this, SLOT(Quit()));

            if(QMessageBox::information(mainWin,
               "VisIt", "There is more than 1 VisIt client connected to the "
               "viewer. Do you want to quit everything? \n\n"
               "Answering No will just detach the GUI and leave the viewer "
               "and its remaining clients running.",
               QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            {
                closeAllClients = true;
            }
            else
                closeAllClients = false;
        }
        else
            closeAllClients = true;
    }

    mainApp->quit();
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
//    Mark C. Miller, Tue Jan 18 12:44:34 PST 2005
//    Improved the error message for when VisIt ignores the '-geometry' flag
//
//    Brad Whitlock, Wed Feb 16 10:31:57 PDT 2005
//    Added code to parse -dv.
//
//    Brad Whitlock, Wed May 4 13:15:25 PST 2005
//    I made sure that host and port are preserved when we are reverse
//    launching the gui.
//
//    Brad Whitlock, Thu Jul 14 10:52:46 PDT 2005
//    I made it use SetIsDevelopmentVersion.
//
// ****************************************************************************

void
QvisGUIApplication::ProcessArguments(int &argc, char **argv)
{
    AppearanceAttributes *aa = viewer->GetAppearanceAttributes();

    //
    // If we're reverse launching then there are certain arguments that we
    // should not strip out here. They will be stripped out by ViewerProxy.
    //
    int i;
    for(i = 1; i < argc; ++i)
    {
        std::string current(argv[i]);
        if(current == "-reverse_launch")
        {
            reverseLaunch = true;
            break;
        }
    }

    for(i = 1; i < argc; ++i)
    {
        std::string current(argv[i]);

        // The host and port should only be stripped here if we're not
        // doing a reverse launch.
        bool stripHostAndPort = false;
        if(current == std::string("-host") ||
           current == std::string("-port"))
        {
            stripHostAndPort = !reverseLaunch;
        }

        // Remove any arguments that could be dangerous to the viewer.
        if(stripHostAndPort || 
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
            cerr << "The -geometry command-line flag is ignored by VisIt's "
                 << "GUI because geometry can be set from within the GUI."
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
        else if(current == "-dv")
        {
            SetIsDevelopmentVersion(true);
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
// Method: QvisGUIApplication::AddMovieArguments
//
// Purpose: 
//   Adds certain arguments to the list of arguments that will be passed along
//   to the movie-maker.
//
// Arguments:
//   argc : The number of arguments.
//   argv : The list of arguments.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 28 12:21:19 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::AddMovieArguments(int argc, char **argv)
{
    for(int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if(arg == "-geometry" ||
           arg == "-key" ||
           arg == "=sessionfile" ||
           arg == "-host" ||
           arg == "-port" ||
           arg == "-sessionfile" ||
           arg == "-o" ||
           arg == "-format" ||
           arg == "-output")
        {
            // skip the 2nd argument
            ++i;
        }
        else if(arg == "-reverse_launch")
        {
            // Skip it
        }
        else
            movieArguments.push_back(argv[i]);
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
//   Eric Brugger, Fri Feb 13 14:05:48 PST 2004
//   I modified the routine to allow the width of the area to be at most
//   the height in the case of a vertical gui and the width to be at most
//   twice the height in the case of a horizontal gui.
//
//   Eric Brugger, Thu Jun 30 08:21:41 PDT 2005
//   I removed the restrictions on the width so that the entire available
//   screen area would be passed to the viewer.
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
//   Brad Whitlock, Wed May 5 15:58:47 PST 2004
//   I moved a line from another method into this one.
//
//   Brad Whitlock, Tue Dec 14 09:07:18 PDT 2004
//   Connected some new signals from the plot manager widget to this object
//   so we can support plot and operator wizards without the plot manager
//   widget having to deal with the plugin manager.
//
//   Hank Childs, Thu Jan 13 13:24:37 PST 2005
//   Change slots so that we can determine if iconify windows is spontaneous.
//
//   Brad Whitlock, Wed Feb 9 17:54:44 PST 2005
//   Connected a new updateVisIt slot.
//
//   Brad Whitlock, Wed Mar 2 17:19:40 PST 2005
//   Disable VisIt update with older Qt versions.
//
//   Brad Whitlock, Mon Mar 21 15:23:57 PST 2005
//   Added save movie.
//
// ****************************************************************************

void
QvisGUIApplication::CreateMainWindow()
{
    int orientation = viewer->GetAppearanceAttributes()->GetOrientation();

    // Make it so the application terminates when the last
    // window is closed.
    connect(mainApp, SIGNAL(aboutToQuit()), mainApp, SLOT(closeAllWindows()));
    connect(mainApp, SIGNAL(lastWindowClosed()), this, SLOT(Quit()));
    connect(mainApp, SIGNAL(hideApplication()), this, SLOT(NonSpontaneousIconifyWindows()));
    connect(mainApp, SIGNAL(showApplication()), this, SLOT(DeIconifyWindows()));

    // Create the main window. Note that the static attributes of
    // QvisWindowBase, which all windows use, are being set here through
    // the mainWin pointer.
    std::string title("VisIt ");
    title += VERSION;
    mainWin = new QvisMainWindow(orientation, title.c_str());
    connect(mainWin, SIGNAL(quit()), this, SLOT(Quit()));
    connect(mainWin, SIGNAL(saveSettings()), this, SLOT(SaveSettings()));
    connect(mainWin, SIGNAL(iconifyWindows(bool)), this, SLOT(IconifyWindows(bool)));
    connect(mainWin, SIGNAL(deIconifyWindows()), this, SLOT(DeIconifyWindows()));
    connect(mainWin, SIGNAL(activateAboutWindow()), this, SLOT(AboutVisIt()));
    connect(mainWin, SIGNAL(saveWindow()), this, SLOT(SaveWindow()));
    connect(mainWin, SIGNAL(saveMovie()), this, SLOT(SaveMovie()));
    connect(mainWin, SIGNAL(printWindow()), this, SLOT(PrintWindow()));
    connect(mainWin, SIGNAL(activatePrintWindow()), this, SLOT(SetPrinterOptions()));
    connect(mainWin->GetPlotManager(), SIGNAL(activatePlotWindow(int)),
            this, SLOT(ActivatePlotWindow(int)));
    connect(mainWin->GetPlotManager(), SIGNAL(addPlot(int, const QString &)),
            this, SLOT(AddPlot(int, const QString &)));
    connect(mainWin->GetPlotManager(), SIGNAL(activateOperatorWindow(int)),
            this, SLOT(ActivateOperatorWindow(int)));
    connect(mainWin->GetPlotManager(), SIGNAL(addOperator(int)),
            this, SLOT(AddOperator(int)));
    connect(mainWin, SIGNAL(refreshFileList()), this, SLOT(RefreshFileList()));
    connect(mainWin, SIGNAL(reopenOnNextFrame()),
            this, SLOT(RefreshFileListAndNextFrame()));
    connect(mainWin, SIGNAL(restoreSession()), this, SLOT(RestoreSession()));
    connect(mainWin, SIGNAL(saveSession()), this, SLOT(SaveSession()));
#if QT_VERSION < 0x030100
    mainWin->updateNotAllowed();
#else
    connect(mainWin, SIGNAL(updateVisIt()), this, SLOT(updateVisIt()));
#endif
    mainWin->ConnectMessageAttr(&message);
    mainWin->ConnectGUIMessageAttributes();
    mainWin->ConnectGlobalAttributes(viewer->GetGlobalAttributes());
    mainWin->ConnectPlotList(viewer->GetPlotList());
    mainWin->ConnectWindowInformation(viewer->GetWindowInformation());
    mainWin->ConnectViewerStatusAttributes(viewer->GetStatusAttributes());

    // Move and resize the GUI so that we can get accurate size and
    // position information from it.
    MoveAndResizeMainWindow(orientation);
}

// ****************************************************************************
// Method: QvisGUIApplication::SetupWindows
//
// Purpose: 
//   Create the crucial windows and set up slot functions to be called when
//   we want to look at the other windows.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:53:20 PST 2004
//
// Modifications:
//   Brad Whitlock, Wed Aug 4 16:13:42 PST 2004
//   Forced the file selection window to be created.
//
//   Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//   Added InteractorWindow. 
//
//   Kathleen Bonnell, Fri Aug 20 15:51:50 PDT 2004 
//   Forced creation of ColorTableWindow, so that colortable names will be
//   available to other windows as needed.
//
//   Brad Whitlock, Thu Dec 9 09:50:33 PDT 2004
//   Added code to connect the application's method to show the expression 
//   window to the variable button so all variable buttons can open the
//   expression window.
//
//   Jeremy Meredith, Mon Apr  4 16:06:50 PDT 2005
//   Added the Simulations window.
//
//   Hank Childs, Tue May 24 17:11:00 PDT 2005
//   Added the Export DB window.
//
//   Brad Whitlock, Wed Apr 20 17:37:07 PST 2005
//   Added command window.
//
// ****************************************************************************

void
QvisGUIApplication::SetupWindows()
{
     // Create the crucial windows
     messageWin = (QvisMessageWindow *)GetWindowPointer(WINDOW_MESSAGE);

     // Create the file selection window because it contains the callbacks
     // that update the GUI when we interact with mdservers.
     GetWindowPointer(WINDOW_FILE_SELECTION);

     outputWin = (QvisOutputWindow *)GetWindowPointer(WINDOW_OUTPUT);
     outputWin->CreateEntireWindow();
     connect(mainWin, SIGNAL(activateOutputWindow()), outputWin, SLOT(show()));
     connect(outputWin, SIGNAL(unreadOutput(bool)),
             mainWin, SLOT(unreadOutput(bool)));

     pluginWin = (QvisPluginWindow *)GetWindowPointer(WINDOW_PLUGINMANAGER);
     connect(mainWin, SIGNAL(activatePluginWindow()),
             pluginWin, SLOT(show()));
     connect(pluginWin, SIGNAL(pluginSettingsChanged()),
             this, SLOT(LoadPlugins()));

     appearanceWin = (QvisAppearanceWindow *)GetWindowPointer(WINDOW_APPEARANCE);
     connect(mainWin, SIGNAL(activateAppearanceWindow()),
             appearanceWin, SLOT(show()));
     connect(appearanceWin, SIGNAL(changeAppearance(bool)),
             this, SLOT(CustomizeAppearance(bool)));

     pickWin = (QvisPickWindow *)GetWindowPointer(WINDOW_PICK);
     connect(mainWin, SIGNAL(activatePickWindow()),
             pickWin, SLOT(show()));

     preferencesWin = (QvisPreferencesWindow *)GetWindowPointer(WINDOW_PREFERENCES);
     connect(mainWin, SIGNAL(activatePreferencesWindow()),
             preferencesWin, SLOT(show()));
     connect(preferencesWin, SIGNAL(changeTimeFormat(const TimeFormat &)),
             mainWin, SLOT(SetTimeStateFormat(const TimeFormat &)));
     connect(preferencesWin, SIGNAL(showSelectedFiles(bool)),
             mainWin, SLOT(SetShowSelectedFiles(bool)));
     connect(preferencesWin, SIGNAL(allowFileSelectionChange(bool)),
             mainWin, SLOT(SetAllowFileSelectionChange(bool)));

     colorTableWin = (QvisColorTableWindow *)GetWindowPointer(WINDOW_COLORTABLE);
     connect(mainWin, SIGNAL(activateColorTableWindow()),
             colorTableWin, SLOT(show()));

     //
     // Connect the variable button to the method to create a new expression in
     // the expression window.
     //
     QvisVariableButton::ConnectExpressionCreation(this, SLOT(newExpression()));

     //
     // Non crucial windows can be created later on demand. Instead of
     // creating all of the windows, create slot functions to
     // create and show then on demand.
     //
     connect(mainWin, SIGNAL(activateFileWindow()),
             this, SLOT(showFileSelectionWindow()));
     connect(mainWin, SIGNAL(activateFileInformationWindow()),
             this, SLOT(showFileInformationWindow()));
     connect(mainWin, SIGNAL(activateHostWindow()),
             this, SLOT(showHostProfilesWindow()));
     connect(mainWin, SIGNAL(activateSaveWindow()),
             this, SLOT(showSaveWindow()));
     connect(mainWin, SIGNAL(activateEngineWindow()),
             this, SLOT(showEngineWindow()));
     connect(mainWin, SIGNAL(activateAnimationWindow()),
             this, SLOT(showAnimationWindow()));
     connect(mainWin, SIGNAL(activateAnnotationWindow()),
             this, SLOT(showAnnotationWindow()));
     connect(mainWin, SIGNAL(activateExpressionsWindow()),
             this, SLOT(showExpressionsWindow()));
     connect(mainWin, SIGNAL(activateSubsetWindow()),
             this, SLOT(showSubsetWindow()));
     connect(mainWin->GetPlotManager(), SIGNAL(activateSubsetWindow()),
             this, SLOT(showSubsetWindow()));
     connect(mainWin, SIGNAL(activateViewWindow()),
             this, SLOT(showViewWindow()));
     connect(mainWin, SIGNAL(activateKeyframeWindow()),
             this, SLOT(showKeyframeWindow()));
     connect(mainWin, SIGNAL(activateLightingWindow()),
             this, SLOT(showLightingWindow()));
     connect(mainWin, SIGNAL(activateGlobalLineoutWindow()),
             this, SLOT(showGlobalLineoutWindow()));
     connect(mainWin, SIGNAL(activateMaterialWindow()),
             this, SLOT(showMaterialWindow()));
     connect(mainWin, SIGNAL(activateCopyrightWindow()),
             this, SLOT(displayCopyright()));
     connect(mainWin, SIGNAL(activateHelpWindow()),
             this, SLOT(showHelpWindow()));
     connect(mainWin, SIGNAL(activateReleaseNotesWindow()),
             this, SLOT(displayReleaseNotes()));
     connect(mainWin, SIGNAL(activateQueryWindow()),
             this, SLOT(showQueryWindow()));
     connect(mainWin, SIGNAL(activateRenderingWindow()),
             this, SLOT(showRenderingWindow()));
     connect(mainWin, SIGNAL(activateCorrelationListWindow()),
             this, SLOT(showCorrelationListWindow()));
     connect(mainWin, SIGNAL(activateQueryOverTimeWindow()),
             this, SLOT(showQueryOverTimeWindow()));
     connect(mainWin, SIGNAL(activateInteractorWindow()),
             this, SLOT(showInteractorWindow()));
     connect(mainWin, SIGNAL(activateSimulationWindow()),
             this, SLOT(showSimulationWindow()));
     connect(mainWin, SIGNAL(activateExportDBWindow()),
             this, SLOT(showExportDBWindow()));
     connect(mainWin, SIGNAL(activateCommandWindow()),
             this, SLOT(showCommandWindow()));
}

// ****************************************************************************
// Method: QvisGUIApplication::WindowFactory
//
// Purpose: 
//   Creates a window and returns a pointer to it.
//
// Arguments:
//   i : The index of the type of window to create.
//
// Returns:    A pointer to a window.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:54:09 PST 2004
//
// Modifications:
//   Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//   Added InteractorWindow. 
//   
//   Jeremy Meredith, Mon Apr  4 16:07:10 PDT 2005
//   Added the Simulations window.
//
//   Brad Whitlock, Wed Apr 20 17:36:35 PST 2005
//   Added the commands window.
//
//   Jeremy Meredith, Thu Apr 28 17:49:31 PDT 2005
//   Changed the exact information sent to the Simulations window.
//
//   Hank Childs, Tue May 24 17:11:00 PDT 2005
//   Added the Export DB window.
//
// ****************************************************************************

QvisWindowBase *
QvisGUIApplication::WindowFactory(int i)
{
    QvisWindowBase *win = 0;

    switch(i)
    {
    case WINDOW_FILE_SELECTION:
        // Create a file selection window.
        { QvisFileSelectionWindow *fsWin = new QvisFileSelectionWindow(windowNames[i]);
          fsWin->ConnectSubjects(viewer->GetHostProfileList());
          win = fsWin;
        }
        break;
    case WINDOW_FILE_INFORMATION:
        // Create the file information window.
        win = new QvisFileInformationWindow(fileServer, windowNames[i],
            "FileInfo", mainWin->GetNotepad());
        break;
    case WINDOW_MESSAGE:
        // Create the message window
        win = new QvisMessageWindow(&message, windowNames[i]);
        break;
    case WINDOW_OUTPUT:
        // Create the output window
        win = new QvisOutputWindow(&message, windowNames[i], "Output",
            mainWin->GetNotepad());
        break;
    case WINDOW_HOSTPROFILES:
        // Create the host profile window
        win = new QvisHostProfileWindow(viewer->GetHostProfileList(),
            windowNames[i], "Profiles", mainWin->GetNotepad());
        break;
    case WINDOW_SAVE:
        // Create the save window.
        win = new QvisSaveWindow(viewer->GetSaveWindowAttributes(),
           windowNames[i], "Save options", mainWin->GetNotepad());
        break;
    case WINDOW_ENGINE:
        // Create the engine window.
        { QvisEngineWindow *ewin = new QvisEngineWindow(viewer->GetEngineList(),
            windowNames[i], "Engines", mainWin->GetNotepad());
          ewin->ConnectStatusAttributes(viewer->GetStatusAttributes());
          win = ewin;
        }
        break;
    case WINDOW_ANIMATION:
        // Create the animation window.
        win = new QvisAnimationWindow(viewer->GetAnimationAttributes(),
            windowNames[i], "Animation", mainWin->GetNotepad());
        break;
    case WINDOW_ANNOTATION:
        // Create the annotation window.
        { QvisAnnotationWindow *aWin = new QvisAnnotationWindow(windowNames[i],
             "Annotation", mainWin->GetNotepad());
          aWin->ConnectAnnotationAttributes(viewer->GetAnnotationAttributes());
          aWin->ConnectAnnotationObjectList(viewer->GetAnnotationObjectList());
          win = aWin;
        }
        break;
    case WINDOW_COLORTABLE:
        // Create the colortable window,
        win = new QvisColorTableWindow(viewer->GetColorTableAttributes(),
            windowNames[i], "Color tables", mainWin->GetNotepad());
        break;
    case WINDOW_EXPRESSIONS:
        // Create the expressions window,
        win = new QvisExpressionsWindow(viewer->GetExpressionList(),
            windowNames[i], "Expressions", mainWin->GetNotepad());
        break;
    case WINDOW_SUBSET:
        // Create the subset window.
        win = new QvisSubsetWindow(viewer->GetSILRestrictionAttributes(),
            windowNames[i], "Subset", mainWin->GetNotepad());
        break;
    case WINDOW_PLUGINMANAGER:
        // Create the plugin manager window.
        win = new QvisPluginWindow(viewer->GetPluginManagerAttributes(),
            windowNames[i], "Plugins", mainWin->GetNotepad());
        break;
    case WINDOW_VIEW:
        // Create the view window.
        { QvisViewWindow *viewWin = new QvisViewWindow(windowNames[i], "View",
              mainWin->GetNotepad());
           viewWin->ConnectCurveAttributes(viewer->GetViewCurveAttributes());
           viewWin->Connect2DAttributes(viewer->GetView2DAttributes());
           viewWin->Connect3DAttributes(viewer->GetView3DAttributes());
           viewWin->ConnectWindowInformation(viewer->GetWindowInformation());
           win = viewWin;
        }
        break;
    case WINDOW_APPEARANCE:
        // Create the appearance window.
        win = new QvisAppearanceWindow(viewer->GetAppearanceAttributes(),
             windowNames[i], "Appearance", mainWin->GetNotepad());
        break;
    case WINDOW_KEYFRAME:
        // Create the keyframe window.
        { QvisKeyframeWindow *kfWin = new QvisKeyframeWindow(
            viewer->GetKeyframeAttributes(), windowNames[i], 
            "Keyframer", mainWin->GetNotepad());
          kfWin->ConnectWindowInformation(viewer->GetWindowInformation());
          kfWin->ConnectPlotList(viewer->GetPlotList());
          win = kfWin;
        }
        break;
    case WINDOW_LIGHTING:
        // Create the lighting window.
        win = new QvisLightingWindow(viewer->GetLightList(),
            windowNames[i], "Lighting", mainWin->GetNotepad());
        break;
    case WINDOW_GLOBALLINEOUT:
        // Create the global lineout window.
        win = new QvisGlobalLineoutWindow(viewer->GetGlobalLineoutAttributes(),
            windowNames[i], "Lineout", mainWin->GetNotepad());
        break;
    case WINDOW_MATERIALOPTIONS:
        // Create the material options window.
        win = new QvisMaterialWindow(viewer->GetMaterialAttributes(),
            windowNames[i], "MIR Options", mainWin->GetNotepad());
        break;
    case WINDOW_PICK:
        // Create the pick window.
        { QvisPickWindow *pwin = new QvisPickWindow(viewer->GetPickAttributes(),
            windowNames[i], "Pick", mainWin->GetNotepad());
            pwin->CreateEntireWindow();
          win = pwin;
        }
        break;
    case WINDOW_HELP:
        // Create the help window
        win = new QvisHelpWindow("Help");
        break;
    case WINDOW_QUERY:
        // Create the query window.
        { QvisQueryWindow *queryWin = new QvisQueryWindow(windowNames[i],
            "Query", mainWin->GetNotepad());
          queryWin->ConnectQueryList(viewer->GetQueryList());
          queryWin->ConnectQueryAttributes(viewer->GetQueryAttributes());
          queryWin->ConnectPickAttributes(viewer->GetPickAttributes());
          queryWin->ConnectPlotList(viewer->GetPlotList());
          win = queryWin;
        }
        break;
    case WINDOW_PREFERENCES:
        // Create the preferences window.
        win = new QvisPreferencesWindow(viewer->GetGlobalAttributes(),
            windowNames[i], "Preferences", mainWin->GetNotepad());
        break;
    case WINDOW_RENDERING:
        // Create the rendering preferences window.
        { QvisRenderingWindow *renderingWin = new QvisRenderingWindow(
            windowNames[i], "Rendering", mainWin->GetNotepad());
          renderingWin->ConnectRenderingAttributes(viewer->GetRenderingAttributes());
          renderingWin->ConnectWindowInformation(viewer->GetWindowInformation());
          win = renderingWin;
        }
        break;
    case WINDOW_CORRELATION:
        // Create the database correlation list window.
        win = new QvisDatabaseCorrelationListWindow(
            viewer->GetDatabaseCorrelationList(), windowNames[i],
            "Correlations", mainWin->GetNotepad());
        break;
    case WINDOW_TIMEQUERY:
        // Create the time query window.
        win = new QvisQueryOverTimeWindow(viewer->GetQueryOverTimeAttributes(),
            windowNames[i], "QueryOverTime", mainWin->GetNotepad());
        break;
    case WINDOW_INTERACTOR:
        // Create the time query window.
        win = new QvisInteractorWindow(viewer->GetInteractorAttributes(),
            windowNames[i], "Interactor", mainWin->GetNotepad());
        break;
    case WINDOW_SIMULATION:
        // Create the simulation window.
        { QvisSimulationWindow *swin =
                     new QvisSimulationWindow(viewer->GetEngineList(),
                                              windowNames[i], "Simulations",
                                              mainWin->GetNotepad());
          swin->ConnectStatusAttributes(viewer->GetStatusAttributes());
          swin->SetNewMetaData(fileServer->GetOpenFile(),
                               fileServer->GetMetaData());
          win = swin;
        }
        break;
    case WINDOW_EXPORT_DB:
        // Create the export DB window.
        win = new QvisExportDBWindow(viewer->GetExportDBAttributes(),
           windowNames[i], "Export Database", mainWin->GetNotepad());
        break;
    case WINDOW_COMMAND:
        // Create the command window.
        win = new QvisCommandWindow(windowNames[i], "Command", mainWin->GetNotepad());
        connect(win, SIGNAL(runCommand(const QString &)),
                this, SLOT(Interpret(const QString &)));
        break;
    }

    return win;
}

// ****************************************************************************
// Method: QvisGUIApplication::CreateInitiallyVisibleWindows
//
// Purpose: 
//   Creates the windows that are initially visible based on the settings
//   in the config file.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:55:00 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::CreateInitiallyVisibleWindows(DataNode *node)
{
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

    for(int i = 0; i < sizeof(windowNames) / sizeof(const char *); ++i)
    {
        DataNode *wNode = guiNode->GetNode(windowNames[i]);
        if(wNode != 0)
        {
            bool windowVisible = false;
            DataNode *visibleNode = wNode->GetNode("visible");
            DataNode *postedNode = wNode->GetNode("posted");
            if(visibleNode != 0)
            {
                windowVisible = visibleNode->AsBool();
                if(postedNode != 0)
                    windowVisible |= postedNode->AsBool();
            }

            //
            // If the window exists, let the code to initialize the window
            // get executed since it will apply the new settings.
            //
            bool windowExists = (otherWindows.find(windowNames[i]) !=
                otherWindows.end());

            //
            // The window is visible so get its pointer, which indirectly
            // causes the window to be created.
            //
            if(windowVisible || windowExists)
            {
                debug1 << (windowExists ? "Initializing" : "Creating")
                       << " the \"" << windowNames[i]
                       << "\" window because it was in the config file."
                       << endl;

                // Make the window read its settings.
                if(node != localSettings && node != systemSettings)
                {
                    // The node is noehter the local settings nor the system
                    // settings so create the window but don't initialize it.
                    QvisWindowBase *win = GetWindowPointer(i);

                    // If the window was created, initialize it using the
                    // specified settings.
                    if(win != 0)
                    {
                        if(windowExists)
                        {
                            debug1 << "Hiding \"" << windowNames[i]
                                   << "\" window" << endl;
                            win->hide();
                        }

                        ReadWindowSettings(win, node);
                    }
                }
                else
                {
                    // Ask for the initialized window so it will be created 
                    // and get initialized with the local and system settings.
                    GetInitializedWindowPointer(i);
                }
            }
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::ReadWindowSettings
//
// Purpose: 
//   Makes a window read its config settings.
//
// Arguments:
//   win : The window that we want to read its config settings.
//   node : The node where the settings are stored.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:55:36 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::ReadWindowSettings(QvisWindowBase *win, DataNode *node)
{
    if(win != 0 && node != 0)
    {
        // Look for the VisIt tree.
        DataNode *visitRoot = node->GetNode("VisIt");
        if(visitRoot != 0)
        {
            // Get the gui node.
            DataNode *guiNode = visitRoot->GetNode("GUI");
            if(guiNode != 0)
            {
                // Get the version
                DataNode *version = visitRoot->GetNode("Version");
                const char *configVersion = 0;
                if(version != 0)
                    configVersion = version->AsString().c_str();
                win->ProcessOldVersions(guiNode, configVersion);
                win->SetFromNode(guiNode, borders);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::GetWindowPointer
//
// Purpose: 
//   Returns a pointer to the i'th window, even if it needs to be created
//   first.
//
// Arguments:
//   i : The index of the window whose pointer we want.
//
// Returns:    A pointer to the i'th window.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:56:29 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisWindowBase *
QvisGUIApplication::GetWindowPointer(int i)
{
    QvisWindowBase *win = 0;
    const char *wName = windowNames[i];
    WindowBaseMap::iterator pos = otherWindows.find(wName);
    if(pos == otherWindows.end())
    {
        win = WindowFactory(i);
        if(win != 0)
        {
            // Add the window to the list of windows.
            otherWindows[wName] = win;
        }
    }
    else
        win = pos->second;

    return win;
}

// ****************************************************************************
// Method: QvisGUIApplication::GetInitializedWindowPointer
//
// Purpose: 
//   Returns a pointer to the i'th window after it has been initialized using
//   the system and local settings.
//
// Arguments:
//   i : The index of the window that we want to create.
//
// Returns:    A pointer to the window that we want.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 15:19:21 PST 2004
//
// Modifications:
//   
// ****************************************************************************

QvisWindowBase *
QvisGUIApplication::GetInitializedWindowPointer(int i)
{
    QvisWindowBase *win = GetWindowPointer(i);

    if(win != 0)
    {
        if(win->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *pwin = (QvisPostableWindow *)win;
            if(pwin->posted())
                pwin->hide();
        }

        ReadWindowSettings(win, systemSettings);
        ReadWindowSettings(win, localSettings);
    }

    return win;
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
   
        // Add a NULL window to the list of plot windows. We'll create the
        // plugin window later when we need it.
        plotWindows.push_back(0);

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

        // Add a NULL window to the list of operator windows. We'll create the
        // plugin window later when we need it.
        operatorWindows.push_back(0);

        // Add an option to the main window's operator manager widget's
        // operator list.
        mainWin->GetPlotManager()->AddOperatorType(GUIInfo->GetMenuName(),
                                                   GUIInfo->XPMIconData());
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::EnsurePlotWindowIsCreated
//
// Purpose: 
//   This method creates the i'th plot plugin window if it has not been
//   created yet.
//
// Arguments:
//   i : The index of the plot plugin to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:57:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::EnsurePlotWindowIsCreated(int i)
{
    if(plotWindows[i] == 0)
    {
        // Get a pointer to the plot plugin manager.
        PlotPluginManager *plotPluginManager = PlotPluginManager::Instance();

        // Get a pointer to the GUI portion of the plot plugin information.
        GUIPlotPluginInfo *GUIInfo = plotPluginManager->GetGUIPluginInfo(
            plotPluginManager->GetEnabledID(i));
   
        // Create the plot plugin window.
        plotWindows[i] = GUIInfo->CreatePluginWindow(i,
            viewer->GetPlotAttributes(i), mainWin->GetNotepad());
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::EnsureOperatorWindowIsCreated
//
// Purpose: 
//   This method creates the i'th operator plugin window if it has not been
//   created yet.
//
// Arguments:
//   i : The index of the operator plugin to create.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 6 14:57:52 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::EnsureOperatorWindowIsCreated(int i)
{
    if(operatorWindows[i] == 0)
    {
        // Get a pointer to the operator plugin manager.
        OperatorPluginManager *operatorPluginManager = OperatorPluginManager::Instance();

        // Get a pointer to the GUI portion of the operator plugin information.
        GUIOperatorPluginInfo *GUIInfo = operatorPluginManager->GetGUIPluginInfo(
            operatorPluginManager->GetEnabledID(i));

        // Create the operator plugin window.
        operatorWindows[i] = GUIInfo->CreatePluginWindow(i,
            viewer->GetOperatorAttributes(i), mainWin->GetNotepad());
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
//    Brad Whitlock, Tue Feb 24 10:25:12 PDT 2004
//    I made it open a text file instead of a binary file.
//
//    Brad Whitlock, Fri Jan 30 14:46:54 PST 2004
//    I added code to save whether the selected files list should be shown.
//
//    Jeremy Meredith, Tue Mar 30 12:26:59 PST 2004
//    Added code to make sure we're not saving a simulation as a valid
//    database name.  The actual plots won't get saved anyway, due to 
//    ViewerPlotList::CreateNode.
//
//    Brad Whitlock, Fri Apr 9 14:00:49 PST 2004
//    Added code to save whether we're allowing the file panel's selection
//    to be updated.
//
//    Brad Whitlock, Wed May 5 16:11:34 PST 2004
//    I changed how we iterate through otherWindows.
//
//    Brad Whitlock, Thu Feb 17 16:02:55 PST 2005
//    I made it return a bool.
//
// ****************************************************************************

bool
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
    for(WindowBaseMap::iterator pos = otherWindows.begin();
        pos != otherWindows.end(); ++pos)
    {
        pos->second->CreateNode(guiNode);
    }

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

        // Make sure we're not saving a simulation as a valid database
        if (p.GetIsFromSimulation())
            continue;

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

    // Save whether the selected files list should be shown.
    guiNode->AddNode(
        new DataNode("showSelectedFiles", mainWin->GetShowSelectedFiles()));
    // Save whether the selected files list highlight should ever get changed.
    guiNode->AddNode(
        new DataNode("allowFileSelectionChange",
                     mainWin->GetAllowFileSelectionChange()));

    // Try to open the output file.
    if((fp = fopen(filename, "wt")) == 0)
        return false;

    // Write the output file to stdout for now.
    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    WriteObject(&root);

    // close the file
    fclose(fp);
    fp = 0;

    return true;
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
//   Brad Whitlock, Tue May 4 17:03:34 PST 2004
//   Since plugin windows don't necessarily exist all the time now, save
//   the plugin window or its configs.
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
        if(plotWindows[i] != 0)
            plotWindows[i]->CreateNode(parentNode);
    }

    // Iterate through each operator window and have it add its window
    // information to the config file data.
    for(i = 0; i < operatorWindows.size(); ++i)
    {
        if(operatorWindows[i] != 0)
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
        ++sessionCount;
        SaveSessionFile(fileName);
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SaveSessionFile
//
// Purpose: 
//   Saves a session with the specified file name.
//
// Arguments:
//   fileName : The filename to use when saving the session.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 14:57:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisGUIApplication::SaveSessionFile(const QString &fileName)
{
#if defined(_WIN32)
    QString sessionExtension(".vses");
#else
    QString sessionExtension(".session");
#endif

    // Force the file to have a .session extension.
    QString sessionName(fileName);
    if(sessionName.right(sessionExtension.length()) != sessionExtension)
        sessionName += sessionExtension;

    // Tell the viewer to save a session file.
    viewer->ExportEntireState(sessionName.latin1());

    // Write the gui part of the session with a ".gui" extension.
    QString retval(sessionName);
    sessionName += ".gui";
    WriteConfigFile(sessionName.latin1());

    return retval;
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
//    Brad Whitlock, Tue Feb 24 10:25:34 PDT 2004
//    I made it open the file in text mode.
//
// ****************************************************************************

DataNode *
QvisGUIApplication::ReadConfigFile(const char *filename)
{
    DataNode *node = 0;

    // Try and open the file for reading.
    if((fp = fopen(filename, "rt")) == 0)
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
//   Brad Whitlock, Thu Feb 26 13:30:49 PST 2004
//   Added code to tell the viewer to clear the caches on all engines.
//
//   Brad Whitlock, Thu May 6 15:17:34 PST 2004
//   I added a call to the new CreateInitiallyVisibleWindows method, which
//   creates the windows for which we have information stored in the 
//   session file.
//
//   Brad Whitlock, Mon Aug 2 13:30:50 PST 2004
//   I added a second call to read the session file so if the specified
//   session file can't be opened then we check in the user's .visit directory
//   before giving up.
//
//   Brad Whitlock, Wed Aug 4 15:51:26 PST 2004
//   I made LoadFile take an argument.
//
//   Brad Whitlock, Thu Oct 27 16:27:44 PST 2005
//   I added sessionFileHelper, an object that makes sure the databases for
//   the session all get opened but quits the session loading if we cancel
//   any mdserver launches.
//
// ****************************************************************************

void
QvisGUIApplication::RestoreSessionFile(const QString &s)
{
    // If the user chose a file, tell the viewer to import that session file.
    if(!s.isEmpty())
    {
        std::string filename(s.latin1());
          
        // Make the gui read in its part of the config.
        std::string guifilename(filename);
        guifilename += ".gui";
        DataNode *node = ReadConfigFile(guifilename.c_str());

        // If the file could not be opened then try and prepend the
        // VisIt directory to it.
        if(node == 0)
        {
            if(guifilename[0] != SLASH_CHAR)
            {
                filename = GetUserVisItDirectory() + filename;
                guifilename = GetUserVisItDirectory() + guifilename;
                debug1 << "The desired session file " << s.latin1()
                       << ".gui could not be opened. VisIt will try and open "
                       << guifilename.c_str() << endl;
                node = ReadConfigFile(guifilename.c_str());
            }
        }

        if(node)
        {
            ProcessConfigSettings(node, false);
            CreateInitiallyVisibleWindows(node);
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
                        //
                        // If we're going to be opening up files, have the
                        // viewer tell all engines to clear their caches.
                        //
                        viewer->ClearCacheForAllEngines();

                        //
                        // Have the session file helper open each of the files
                        // that we'll need.
                        //
                        if(sessionFileHelper == 0)
                        {
                            sessionFileHelper = new QvisSessionFileDatabaseLoader(
                                this, "sessionFileHelper");
                            connect(sessionFileHelper, SIGNAL(complete(const QString &)),
                                    this, SLOT(sessionFileHelper_LoadSession(const QString &)));
                            connect(sessionFileHelper, SIGNAL(loadFile(const QString &)),
                                    this, SLOT(sessionFileHelper_LoadFile(const QString &)));
                        }
                        sessionFileHelper->SetDatabases(plotDatabases->AsStringVector());
                        sessionFileHelper->Start(filename.c_str());
                    }
                }
            }

            delete node;
        }
        else
        {
            // Have the viewer read in its part of the config. Note that we
            // pass the inVisItDir flag as false because we don't want to have
            // the viewer prepend the .visit directory to the file since it's
            // already part of the filename.
            viewer->ImportEntireState(filename, false);
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::sessionFileHelper_LoadFile
//
// Purpose: 
//   Qt slot function used with sessionFileHelper to load a particular
//   database.
//
// Arguments:
//   db : The name of the database to load.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:28:45 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::sessionFileHelper_LoadFile(const QString &db)
{
    QualifiedFilename f(db.latin1());
    if(!fileServer->HaveOpenedFile(f))
        LoadFile(f, false);
}

// ****************************************************************************
// Method: QvisGUIApplication::sessionFileHelper_LoadSession
//
// Purpose: 
//   Qt slot function that tells the viewer to load the session file.
//
// Arguments:
//   filename : The name of the session file to load.
//
// Note:       This slot is only called by sessionFileHelper if all of the
//             required databases were loaded (no mdserver cancellations).
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 27 16:29:28 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::sessionFileHelper_LoadSession(const QString &filename)
{
    // Have the viewer read in its part of the config. Note that we
    // pass the inVisItDir flag as false because we don't want to have
    // the viewer prepend the .visit directory to the file since it's
    // already part of the filename.
    viewer->ImportEntireState(filename, false);
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
    {
        InitializeFileServer(guiNode);
    }
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
//   Brad Whitlock, Fri Jan 30 14:41:50 PST 2004
//   I added code to set whether the selected files should be shown.
//
//   Brad Whitlock, Fri Apr 9 14:03:51 PST 2004
//   I added code to set whether the file panel's highlight should ever
//   be changed.
//
//   Brad Whitlock, Wed May 5 15:50:59 PST 2004
//   I removed some code that processed the config settings for the non-plugin
//   windows.
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

    // Set the time format.
    TimeFormat fmt;
    fmt.SetFromNode(guiNode);
    mainWin->SetTimeStateFormat(fmt);
    preferencesWin->SetTimeStateFormat(fmt);

    // Get whether the selected files should be shown.
    DataNode *ssfNode = 0;
    if((ssfNode = guiNode->GetNode("showSelectedFiles")) != 0)
    {
        mainWin->SetShowSelectedFiles(ssfNode->AsBool());
        preferencesWin->SetShowSelectedFiles(ssfNode->AsBool());
    }

    // Get whether the selected files should be shown.
    DataNode *afscNode = 0;
    if((afscNode = guiNode->GetNode("allowFileSelectionChange")) != 0)
    {
        allowFileSelectionChange = afscNode->AsBool();
        mainWin->SetAllowFileSelectionChange(afscNode->AsBool());
        preferencesWin->SetAllowFileSelectionChange(afscNode->AsBool());
    }

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
//   Brad Whitlock, Wed May 5 16:57:09 PST 2004
//   I added code to create plugin windows that don't exist at the time we
//   are reading the config file.
//
// ****************************************************************************

void
QvisGUIApplication::ReadPluginWindowConfigs(DataNode *parentNode,
    const char *configVersion)
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

        std::string key(GUIInfo->GetMenuName());
        key += " plot attributes";

        if(plotWindows[i] != 0 &&
           plotWindows[i]->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *win = (QvisPostableWindow *)plotWindows[i];
            if(win->posted())
                win->hide();
        }

        if(parentNode->GetNode(key) != 0)
        {
            debug1 << "Found \"" << key.c_str() << "\" in the config so "
                   << "that window will be created." << endl;
            EnsurePlotWindowIsCreated(i);
            plotWindows[i]->ProcessOldVersions(parentNode, configVersion);
            plotWindows[i]->SetFromNode(parentNode, borders);
        }
    }

    // Get a pointer to the operator plugin manager.
    OperatorPluginManager *operatorPluginManager = OperatorPluginManager::Instance();

    // Create the window and populate the menu for each operator plugin.
    for(i = 0; i < operatorPluginManager->GetNEnabledPlugins(); ++i)
    {
        // Get a pointer to the GUI portion of the operator plugin information.
        GUIOperatorPluginInfo *GUIInfo = operatorPluginManager->GetGUIPluginInfo(
                                       operatorPluginManager->GetEnabledID(i));

        std::string key(GUIInfo->GetMenuName());
        key += " operator attributes";

        if(operatorWindows[i] != 0 &&
            operatorWindows[i]->inherits("QvisPostableWindow"))
        {
            QvisPostableWindow *win = (QvisPostableWindow *)operatorWindows[i];
            if(win->posted())
                win->hide();
        }

        if(parentNode->GetNode(key) != 0)
        {
            debug1 << "Found \"" << key.c_str() << "\" in the config so "
                   << "that window will be created." << endl;
            EnsureOperatorWindowIsCreated(i);
            operatorWindows[i]->ProcessOldVersions(parentNode, configVersion);
            operatorWindows[i]->SetFromNode(parentNode, borders);
        }
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
//   Brad Whitlock, Thu Oct 27 16:06:55 PST 2005
//   Added code to catch CancelledConnectException.
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
    CATCH(CancelledConnectException)
    {
        QString msgStr;
        msgStr.sprintf("The launch of a metadata server on "
                       "host \"%s\" was cancelled.", fileServer->GetHost().c_str());
        Error(msgStr);
    }
    ENDTRY
}

// ****************************************************************************
// Method: QvisGUIApplication::GetVirtualDatabaseDefinitions
//
// Purpose: 
//   Gets all of the virtual file definitions for files in the applied
//   file list.
//
// Arguments:
//   defs : The map into which the definitions will be stored.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 27 11:59:42 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::GetVirtualDatabaseDefinitions(
    StringStringVectorMap &defs)
{
    const QualifiedFilenameVector &files = fileServer->GetAppliedFileList();

    // Add the definitions for all virtual files to the map.
    defs.clear();
    for(int i = 0; i < files.size(); ++i)
    {
        if(files[i].IsVirtual())
        {
            defs[files[i].FullName()] = 
                fileServer->GetVirtualFileDefinition(files[i].FullName());
        }
    }
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
//   Brad Whitlock, Mon Feb 7 14:23:34 PST 2005
//   I changed this mode of refreshing the selected files list so that it
//   only adds new files to the selected files list. The previous behavior
//   reread all directories and only kept files that match the filter. Doing
//   so made it possible for files to disappear from the selected files list.
//   The new method merges the old selected files list with the new, updated
//   selected files list.
//
// ****************************************************************************

void
QvisGUIApplication::RefreshFileList()
{
    // Save the current host and path.
    std::string  oldHost(fileServer->GetHost());
    std::string  oldPath(fileServer->GetPath());

    // Save the definitions for the virtual databases.
    StringStringVectorMap oldVirtualDefinitions;
    GetVirtualDatabaseDefinitions(oldVirtualDefinitions);

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
    QualifiedFilenameVector refreshedFiles(fileServer->GetAppliedFileList());
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
            {
                // Only add the file if it's not already in the list.
                if(std::find(refreshedFiles.begin(), refreshedFiles.end(),
                   newFiles[i]) == refreshedFiles.end())
                {
                    refreshedFiles.push_back(newFiles[i]);
                }
            }
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

    // Get the definitions for the virtual databases now that we have reread
    // all of the directories.
    StringStringVectorMap newVirtualDefinitions;
    GetVirtualDatabaseDefinitions(newVirtualDefinitions);

    //
    // If the open file is in the list of new files and it is a virtual db,
    // then reopen it so we pick up new time states.
    //
    for(i = 0; i < refreshedFiles.size(); ++i)
    {
        if(refreshedFiles[i].IsVirtual())
        {
            std::string fileName(refreshedFiles[i].FullName());
            StringStringVectorMap::const_iterator oldDef = 
                oldVirtualDefinitions.find(fileName);
            StringStringVectorMap::const_iterator newDef =
                newVirtualDefinitions.find(fileName);

            // If the virtual file definitions are different then reopen the
            // database on the viewer so that plots are reexecuted.
            if(oldDef != newDef)
            {
                debug1 << "Telling the viewer to check " << fileName.c_str()
                       << " for new time states." << endl;
                viewer->CheckForNewStates(fileName);
            }
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
//   Brad Whitlock, Tue Jan 27 21:41:00 PST 2004
//   Changed the name of the viewer RPC that advances to the next state.
//
// ****************************************************************************

void
QvisGUIApplication::RefreshFileListAndNextFrame()
{
    RefreshFileList();
    viewer->TimeSliderNextState();
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
//   Brad Whitlock, Tue Apr 6 14:39:18 PST 2004
//   I added code to allow the file selection to change when we're loading
//   a file.
//
//   Brad Whitlock, Wed Aug 4 15:51:55 PST 2004
//   I removed some code to prevent this method from doing anything if a
//   session file was specified and moved it to a higher level so this
//   method always does something provided the input file is not empty.
//
//   Brad Whitlock, Thu Oct 27 16:08:29 PST 2005
//   Added code to catch CancelledConnectException.
//
// ****************************************************************************

void
QvisGUIApplication::LoadFile(QualifiedFilename &f, bool addDefaultPlots)
{
    if(!f.Empty())
    {
        int timeid = visitTimer->StartTimer();

        // Let the file panel highlight the new file.
        mainWin->SetAllowFileSelectionChange(true);

        // Temporarily save the old settings.
        std::string oldHost(fileServer->GetHost());
        std::string oldPath(fileServer->GetPath());
        std::string oldFilter(fileServer->GetFilter());

        TRY
        {
            // In case the path was relative, expand the path to a full path.
            f.path = fileServer->ExpandPath(f.path);

            // Switch to the right host/path and get the file list.
            fileServer->SetHost(f.host);
            fileServer->SetPath(f.path);
            fileServer->Notify();

            // Get the filtered file list and look to see if it contains the
            // file that we want to load.
            QualifiedFilenameVector files(fileServer->GetFilteredFileList());
            bool fileInList = false;
            int  timeState  = 0;
            QualifiedFilenameVector::const_iterator pos;
            for(pos = files.begin(); pos != files.end() && !fileInList; ++pos)
            {
                bool sameHost = f.host == pos->host;
                bool samePath = f.path == pos->path;

                if(sameHost && samePath)
                {
                    bool sameFile = f.filename == pos->filename;
                    if(pos->IsVirtual())
                    {
                        // Get the list of files in the virtual file.
                        stringVector def(fileServer->GetVirtualFileDefinition(*pos));

                        // See if the file that we want to open is in the virtual
                        // file definition.                       
                        for(int state = 0; state < def.size(); ++state)
                        {
                            if(f.filename == def[state])
                            {
                                fileInList = true;
                                timeState = state;
                                f.filename = pos->filename;
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
                files.push_back(f);

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
            SetOpenDataFile(f, timeState);

            // Tell the viewer to open the file too.
            viewer->OpenDatabase(f.FullName().c_str(), timeState,
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
                           f.FullName().c_str(),
                           fileServer->GetHost().c_str());
            Error(msgStr);
        }
        CATCH(CancelledConnectException)
        {
            // nothing
        }
        ENDTRY

        // Create a trigger that will cause the GUI to prevent the file
        // panel from updating once the viewer is done loading the file.
        Synchronize(SET_FILE_HIGHLIGHT_TAG);

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
// Method: QvisGUIApplication::SendKeepAlives
//
// Purpose: 
//   This is a Qt slot function that tells the file server to send keep alive
//   signals to the mdservers.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 12 14:12:30 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::SendKeepAlives()
{
    fileServer->SendKeepAlives();
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
        EnsurePlotWindowIsCreated(index);
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
        EnsureOperatorWindowIsCreated(index);
        operatorWindows[index]->show();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::NonSpontaneousIconifyWindows
//
// Purpose: 
//   This is a Qt slot function that iconifies all of the GUI windows.  It 
//   should only be called when the iconify request is not spontaneous.
//
// Programmer: Hank Childs
// Creation:   January 13, 2005
//
// ****************************************************************************

void
QvisGUIApplication::NonSpontaneousIconifyWindows(void)
{
    IconifyWindows(false);
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
//   Brad Whitlock, Tue May 4 16:58:20 PST 2004
//   I made it skip plot and operator windows that don't exist. I also changed
//   how we iterate through otherWindows.
//
//   Hank Childs, Thu Jan 13 13:24:37 PST 2005
//   Only minimize window if this is a non-spontaneous iconify request. ['5436]
//
// ****************************************************************************

void
QvisGUIApplication::IconifyWindows(bool isSpontaneous)
{
    // Iconify the main window.
    if (!isSpontaneous)
        mainWin->showMinimized();

#if !defined(_WIN32)
    int index;

    // Iconify all of the regular windows.
    for(WindowBaseMap::iterator pos = otherWindows.begin();
        pos != otherWindows.end(); ++pos)
    {
        pos->second->showMinimized();
    }

    // Iconify all of the plot windows.
    for(index = 0; index < plotWindows.size(); ++index)
    {
        if(plotWindows[index] != 0)
            plotWindows[index]->showMinimized();
    }

    // Iconify all of the operator windows.
    for(index = 0; index < operatorWindows.size(); ++index)
    {
        if(operatorWindows[index] != 0)
            operatorWindows[index]->showMinimized();
    }
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
//   Brad Whitlock, Tue May 4 16:59:32 PST 2004
//   I made it skip windows that don't exist. I also changed how we iterate
//   through otherWindows.
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
    for(WindowBaseMap::iterator pos = otherWindows.begin();
        pos != otherWindows.end(); ++pos)
    {
        pos->second->showNormal();
    }

    // Iconify all of the plot windows.
    for(index = 0; index < plotWindows.size(); ++index)
    {
        if(plotWindows[index] != 0)
            plotWindows[index]->showNormal();
    }

    // Iconify all of the operator windows.
    for(index = 0; index < operatorWindows.size(); ++index)
    {
        if(operatorWindows[index] != 0)
            operatorWindows[index]->showNormal();
    }
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
//   Brad Whitlock, Fri Jan 7 16:36:32 PST 2005
//   I added code to set the active window to the splashscreen so the focus
//   stays on the splashscreen when we run on MacOS X.
//
//   Brad Whitlock, Tue Mar 8 16:10:11 PST 2005
//   I ifdef'd out the setActiveWindow code so it only happens on MacOS X.
//
// ****************************************************************************

void
QvisGUIApplication::SplashScreenProgress(const char *msg, int prog)
{
    if(splash)
    {
#if defined(Q_WS_MACX)
        splash->setActiveWindow();
#endif
        splash->Progress(msg, prog);
    }
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
//   Brad Whitlock, Wed May 5 08:46:19 PDT 2004
//   Moved the printer initialization code to here since it's something
//   that is rarely done. We can do it when we want to open the printer
//   window.
//
//   Brad Whitlock, Thu Nov 4 18:00:10 PST 2004
//   I added MacOS X specific coding that lets us handle the setting up of
//   the printer options so we can actually print.
//
// ****************************************************************************
    
void
QvisGUIApplication::SetPrinterOptions()
{
#if defined(Q_WS_MACX)
    //
    // If we're on MacOS X and the Mac application style is being used, manage
    // the printer setup ourselves since the QPrinter object does not return
    // enough information when it uses the native MacOS X printer dialog. Here
    // we use the native MacOS X printer dialog but we get what we need out
    // of it.
    //
    if(qApp->style().inherits("QMacStyle"))
    {
        PMPageFormat pformat;
        PMPrintSettings psettings;
        PMPrintSession psession;
        int nObjectsToFree = 0;
        bool okayToPrint = true;
    
        TRY
        {
            if(PMCreateSession(&psession) != kPMNoError)
            {
            EXCEPTION0(VisItException);
            }
            nObjectsToFree = 1;
        
            if(PMCreatePrintSettings(&psettings) != kPMNoError)
            {
                EXCEPTION0(VisItException);
            }
            nObjectsToFree = 2;
            if(PMSessionDefaultPrintSettings(psession, psettings) != kPMNoError)
            {
                EXCEPTION0(VisItException);
            }
        
            if(PMCreatePageFormat(&pformat) != kPMNoError)
            {
                EXCEPTION0(VisItException);
            }
            nObjectsToFree = 3;
            if(PMSessionDefaultPageFormat(psession, pformat) != kPMNoError)
            {
                EXCEPTION0(VisItException);
            }
    
            //
            // Show the MacOS X printer window and allow the user to select
            // the printer to use when printing images in VisIt.
            //
            Boolean accepted = false;
            if(PMSessionPrintDialog(psession, psettings, pformat, &accepted) == kPMNoError &&
               accepted == true)
            {
                PrinterAttributes *p = viewer->GetPrinterAttributes();
        
                // Get the name of the printer to use for printing the image.
                CFArrayRef printerList = NULL;
                CFIndex currentIndex;
                PMPrinter currentPrinter;
                if(PMSessionCreatePrinterList(psession, &printerList, &currentIndex,
                   &currentPrinter) == kPMNoError)
                {
                    if(printerList != NULL)
                    {
                        const void *pData = CFArrayGetValueAtIndex(printerList,
                            currentIndex);
                        if(pData != NULL)
                        {
                            CFStringRef pName = (CFStringRef)pData;
                            char buf[1000]; buf[0] = '\0';
                            CFStringGetCString(pName, buf, 1000,
                                kCFStringEncodingMacRoman);
                            p->SetPrinterName(buf);
                        }
                        else
                        {
                            debug4 << "Could not find printer name" << endl;
                            CFRelease(printerList);
                            EXCEPTION0(VisItException);
                        }
                    
                        // Free the printerList
                        CFRelease(printerList);
                    }
                    else
                    {
                        debug4 << "Could not return the list of printer names"
                               << endl;
                        EXCEPTION0(VisItException);
                    }
                }

                // Get the options from the psettings object.
                PMOrientation orient;
                PMGetOrientation(pformat, &orient);
                p->SetPortrait(orient == kPMPortrait || orient == kPMReversePortrait);
        
                // Set the number of copies
                UInt32 ncopies = 1;
                PMGetCopies(psettings, &ncopies);
                p->SetNumCopies(int(ncopies));
     
                // Set some of the last properties
                p->SetOutputToFile(false);
                p->SetPrintColor(true);
                p->SetCreator(viewer->GetLocalUserName());

                // Tell the viewer what the properties are.
                if(printerObserver != 0)             
                    printerObserver->SetUpdate(false);
                p->Notify();
            }
            else
                debug4 << "User cancelled the printer options window." << endl;
        }
        CATCH(VisItException)
        {
            Error("VisIt encountered an error while setting up printer options.");
            okayToPrint = false;
        }
        ENDTRY
    
        //
        // Free the PM objects that we created.
        //
        switch(nObjectsToFree)
        {
        case 3:
            PMRelease(pformat);
            // Fall through
        case 2:
            PMRelease(psettings);
            // Fall through
        case 1:
            PMRelease(psession);       
        }
    
        //
        // Tell the viewer to print the image because the MacOS X printer
        // dialog has the word "Print" to click when you're done setting
        // options. This says to me that MacOS X applications expect to
        // print once the options are set.
        //
        if(okayToPrint)
            viewer->PrintWindow();
    }
    else
    {
#endif
        //
        // If we've never set up the printer options, set them up now using
        // Qt's printer object and printer dialog.
        //
        if(printer == 0)
        {
            int timeid = visitTimer->StartTimer();
            printer = new QPrinter;
            printerObserver = new ObserverToCallback(viewer->GetPrinterAttributes(),
                UpdatePrinterAttributes, (void *)printer);
            viewer->GetPrinterAttributes()->SetCreator(viewer->GetLocalUserName());
            PrinterAttributesToQPrinter(viewer->GetPrinterAttributes(), printer);
            visitTimer->StopTimer(timeid, "Setting up printer");
        }

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
#if defined(Q_WS_MACX)
    }
#endif
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


// ****************************************************************************
//  Method:  QvisGUIApplication::UpdateMetaDataAttributes
//
//  Purpose:
//    Callback for when metadata or SIL atts are changed.
//
//  Arguments:
//    subj       the new metadata attributes
//    data       the QvisGUIApplication instance
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
// ****************************************************************************

void
QvisGUIApplication::UpdateMetaDataAttributes(Subject *subj, void *data)
{
    //avtDatabaseMetaData *p = (avtDatabaseMetaData*)subj;
    QvisGUIApplication *guiapp = (QvisGUIApplication *)data;
    guiapp->HandleMetaDataUpdate();
}

// ****************************************************************************
//  Method:  QvisGUIApplication::HandleMetaDataUpdate
//
//  Purpose:
//    Handle manual updates for when metadata and SIL atts change for the
//    active plot.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Apr  4 16:07:27 PDT 2005
//    Added an update of the simulation window.
//
//    Jeremy Meredith, Thu Apr 28 17:49:31 PDT 2005
//    Changed the exact information sent to the Simulations window.
//
// ****************************************************************************

void
QvisGUIApplication::HandleMetaDataUpdate()
{
    // Poke the metadata into the file server
    fileServer->SetOpenFileMetaData(viewer->GetDatabaseMetaData());

    // Poke the SIL into the file server
    avtSIL *sil = new avtSIL(*viewer->GetSILAtts());
    fileServer->SetOpenFileSIL(sil);
    delete sil;

    //
    // Update what needs updating
    //
    // NOTE:  this is a bit too manual right now.  We may need to either
    // force updating of more places soon, or have more things observe
    // the updating metadata and be aware when it has changed.
    //

    // Plot Manager Widget
    mainWin->GetPlotManager()->Update(fileServer);

    // File Information Window
    string fileInfoWinName = windowNames[WINDOW_FILE_INFORMATION];
    if (otherWindows.count(fileInfoWinName))
    {
        QvisFileInformationWindow *fileInfoWin = (QvisFileInformationWindow*)
            otherWindows[fileInfoWinName];
        fileInfoWin->Update(fileServer);
    }

    // Simulation Window
    string simWinName = windowNames[WINDOW_SIMULATION];
    if (otherWindows.count(simWinName))
    {
        QvisSimulationWindow *simWin =
            (QvisSimulationWindow*)otherWindows[simWinName];
        simWin->SetNewMetaData(fileServer->GetOpenFile(),
                               fileServer->GetMetaData());
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::AddPlot
//
// Purpose: 
//   This is a Qt slot function that is called when the user tries to add a
//   new plot.
//
// Arguments:
//   plotType : The type of plot to be added.
//   varName  : The name of the variable to be plotted.
//
// Note:       This code used to be in the plot manager widget but was
//             relocated here and modified to support plot wizards.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:29:05 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::AddPlot(int plotType, const QString &varName)
{
    // Get a pointer to the GUI portion of the plot plugin information.
    PlotPluginManager *plotPluginManager = PlotPluginManager::Instance();
    GUIPlotPluginInfo *GUIInfo = plotPluginManager->GetGUIPluginInfo(
        plotPluginManager->GetEnabledID(plotType));

    // Try and create a wizard for the desired plot type.
    QString wName; wName.sprintf("plot_wizard_%d", plotType);
    QvisWizard *wiz = GUIInfo->CreatePluginWizard(
        viewer->GetPlotAttributes(plotType), mainWin, wName.latin1());

    if(wiz == 0)
    {
        // Set the cursor.
        SetWaitCursor();

        // Tell the viewer to add a plot.
        viewer->AddPlot(plotType, varName.latin1());

        // If we're in auto update mode, tell the viewer to draw the plot.
        if(AutoUpdate())
            viewer->DrawPlots();
    }
    else
    {
        // Execute the wizard.
        if(wiz->exec() == QDialog::Accepted)
        {
            // Set the cursor.
            SetWaitCursor();

            // Set the default plot options. This is a little bit of a hack
            // but my previous attempt to first create the plot and then
            // set its attributes resulted in the default plot attributes
            // coming back and clobbering the correct settings in the plot
            // attributes window. That method could work if setting the
            // plot options also caused them to be sent back to the client.
            wiz->SendAttributes();
            viewer->SetDefaultPlotOptions(plotType);

            // Tell the viewer to add a plot.
            viewer->AddPlot(plotType, varName.latin1());

            // If we're in auto update mode, tell the viewer to draw the plot.
            if(AutoUpdate())
                viewer->DrawPlots();
        }

        wiz->deleteLater();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::AddOperator
//
// Purpose: 
//   This is a Qt slot function that is called when the user tries to add
//   an operator.
//
// Arguments:
//   operatorType : The type of operator to add.
//
// Note:       This code used to be in the plot manager widget but I moved
//             it here so we can have operator wizards.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec 14 09:30:36 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::AddOperator(int operatorType)
{
    // Get a pointer to the GUI portion of the operator plugin information.
    OperatorPluginManager *operatorPluginManager = OperatorPluginManager::Instance();
    GUIOperatorPluginInfo *GUIInfo = operatorPluginManager->GetGUIPluginInfo(
        operatorPluginManager->GetEnabledID(operatorType));

    // Try and create a wizard for the desired operator type.
    QString wName; wName.sprintf("operator_wizard_%d", operatorType);
    QvisWizard *wiz = GUIInfo->CreatePluginWizard(
        viewer->GetOperatorAttributes(operatorType), mainWin, wName.latin1());

    if(wiz == 0)
    {
        // The operator has no wizard so just add the operator.
        viewer->AddOperator(operatorType);
    }
    else
    {
        // Execute the wizard.
        if(wiz->exec() == QDialog::Accepted)
        {
            // Send the operator's options. Note that this is done before
            // the operator is added because we may have drawn plots that
            // will be re-executed right away.
            wiz->SendAttributes();
            viewer->SetOperatorOptions(operatorType);

            // Tell the viewer to add an operator.
            viewer->AddOperator(operatorType, false);
        }

        wiz->deleteLater();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::newExpression
//
// Purpose: 
//   This is a Qt slot function that is called from variable buttons to
//   create a new expression.
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 9 09:58:59 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::newExpression()
{
    QvisExpressionsWindow *exprWin = (QvisExpressionsWindow *)
        GetInitializedWindowPointer(WINDOW_EXPRESSIONS);
    if(exprWin)
    {
        exprWin->show();
        exprWin->setActiveWindow();
        exprWin->raise();
        exprWin->newExpression();
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::updateVisIt
//
// Purpose: 
//   This method creates a QvisVisItUpdate object and tells it to look for a
//   new version of VisIt to install.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 9 17:56:51 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::updateVisIt()
{
    if(visitUpdate == 0)
    {
        visitUpdate = new QvisVisItUpdate(mainWin, "VisIt Update");
        connect(visitUpdate, SIGNAL(updateNotAllowed()),
                mainWin, SLOT(updateNotAllowed()));
        connect(visitUpdate, SIGNAL(installationComplete(const QString &)),
                this, SLOT(updateVisItCompleted(const QString &)));
    }

    visitUpdate->startUpdate();
}

// ****************************************************************************
// Method: QvisGUIApplication::updateVisItCompleted
//
// Purpose: 
//   This method restarts the current VisIt session in a new version of the
//   VisIt executable. It's called when a new version of VisIt has been
//   successfully installed.
//
// Arguments:
//   program : The full name of the new VisIt executable.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 11:43:06 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:49:41 PDT 2005
//   Fixed the code so it compiles on Windows.
//
//   Brad Whitlock, Fri May 6 12:14:14 PDT 2005
//   I made it use the Quit method.
//
// ****************************************************************************

void
QvisGUIApplication::updateVisItCompleted(const QString &program)
{
    QString msg("VisIt has been updated. Would you like VisIt to save \n"
                "its session, quit, and restart the session using the new \n"
                "version of VisIt?");
    if(QMessageBox::information(mainWin, "VisIt", msg,
       QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        debug1 << "User chose save session and start up again in the new VisIt."
               << endl; 

        QString visitDir(GetUserVisItDirectory().c_str());
        QString fileName(visitDir + "update_version");
#if defined(_WIN32)
        fileName += ".vses";
#else
        fileName += ".session";
#endif

        // Tell the viewer to save a session file.
        viewer->ExportEntireState(fileName.latin1());

        // Write the gui part of the session with a ".gui" extension.
        QString gfileName(fileName + ".gui");
        WriteConfigFile(gfileName.latin1());

#if defined(_WIN32)
        // Start the new version of VisIt.
        QProcess *newVisIt = new QProcess(program, this, "start new visit");
        newVisIt->addArgument("-sessionfile");
        newVisIt->addArgument(fileName);
        if(localOnly)
            newVisIt->addArgument("-localonly");
        if(!showSplash)
            newVisIt->addArgument("-nosplash");
        if(!readConfig)
            newVisIt->addArgument("-noconfig");
        newVisIt->start();

        // quit this version.
        Quit();
#else
        // Write a script to launch the new version of VisIt       
        FILE *f = fopen("exec_new_visit", "w");
        if(f == 0)
        {
            Error("VisIt could not automatically relaunch itself. "
                  "Please exit and restart VisIt.");
        }
        else
        {
            //
            // Write a C-shell script to launch the new version of VisIt.
            // We use a script because it affords us the opportunity to
            // remove some environment variables that mess up the launch
            // of the new version.
            //
            fprintf(f, "unsetenv VISITDIR\n");
            fprintf(f, "unsetenv VISITPROGRAM\n");
            fprintf(f, "unsetenv VISITVERSION\n");
            fprintf(f, "unsetenv VISITPLUGINDIR\n");
            fprintf(f, "unsetenv VISITPLUGININSTPUB\n");
            fprintf(f, "unsetenv VISITPLUGININSTPRI\n");
            fprintf(f, "unsetenv VISITPLUGININST\n");
            fprintf(f, "unsetenv VISITHOME\n");
            fprintf(f, "unsetenv VISITARCHHOME\n");
            fprintf(f, "unsetenv VISITHELPHOME\n");
            fprintf(f, "unsetenv LD_LIBRARY32_PATH\n");
            fprintf(f, "unsetenv LD_LIBRARYN32_PATH\n");
            fprintf(f, "unsetenv LD_LIBRARY64_PATH\n");
            fprintf(f, "unsetenv PYTHONHOME\n");
            QString commandLine(program);
            commandLine += " -sessionfile ";
            commandLine += fileName;
            if(localOnly)
                commandLine += " -localonly";
            if(!showSplash)
                commandLine += " -nosplash";
            if(!readConfig)
                commandLine += " -noconfig";
            fprintf(f, "%s\n", commandLine.latin1());
            fprintf(f, "sleep 2\n");
            fprintf(f, "rm -f exec_new_visit\n");
            fprintf(f, "exit 0\n");
            fclose(f);

            // Start the script to launch the new version of VisIt.
            QProcess *newVisIt = new QProcess(QString("csh"), this, "start new visit");
            newVisIt->addArgument("-f");
            newVisIt->addArgument("exec_new_visit");
            newVisIt->start();

            // quit this version.
            Quit();
        }
#endif
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SendInterface
//
// Purpose: 
//   This is a Qt slot function that sends the GUI's client interface to
//   the viewer.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 09:40:54 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::SendInterface()
{
    // The viewer uses this method to discover information about the GUI.
    ClientInformation *info = viewer->GetClientInformation();
    info->SetClientName("gui");
    info->ClearMethods();

    // Populate the method names and prototypes that the GUI supports
    // but don't advertise _QueryClientInformation.
    info->DeclareMethod("Quit", "");
    info->DeclareMethod("DeIconify", "");
    info->DeclareMethod("Hide",      "");
    info->DeclareMethod("Iconify",   "");
    info->DeclareMethod("Show",      "");
    info->DeclareMethod("MessageBoxYesNo",    "s");
    info->DeclareMethod("MessageBoxOkCancel", "s");
    info->DeclareMethod("MessageBoxOk",       "s");
    info->DeclareMethod("MovieProgress",      "sii");
    info->DeclareMethod("MovieProgressEnd",   "");
    info->SelectAll();
    info->Notify();

    debug5 << "GUI info: " << info->GetClientName().c_str()
           << endl;
    debug5 << "methods:" << endl;
    for(int j = 0; j < info->GetMethodNames().size(); ++j)
    {
        debug5 << "\t" << info->GetMethod(j).c_str() << "("
               << info->GetMethodPrototype(j).c_str() << ")" << endl;
    }
    debug5 << endl;
}

// ****************************************************************************
// Method: QvisGUIApplication::HandleClientMethod
//
// Purpose: 
//   Handles GUI client methods, which are commands that can be called by
//   other VisIt clients.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 18:07:21 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 09:21:55 PDT 2005
//   Moved creation of movie progress dialog to this method.
//
// ****************************************************************************

void
QvisGUIApplication::HandleClientMethod()
{
    ClientMethod *method = viewer->GetClientMethod();
    int index;

    if(method->GetMethodName() == "_QueryClientInformation")
    {
        debug5 << "GUI received _QueryClientInformation method. Tell the "
                  "viewer which client methods the GUI supports." << endl;
        // Xfer is disabled right now since we got here via Xfer::Process. We
        // can't send anything back to the viewer until we get to the main
        // event loop so schedule SendInterface.
        QTimer::singleShot(10, this, SLOT(SendInterface()));
    }
    else
    {
        int okay = viewer->MethodRequestHasRequiredInformation();
     
        if(okay == 0)
        {
            debug5 << "Client method " << method->GetMethodName().c_str()
                   << " is not supported by the GUI." << endl;
        }
        else if(okay == 1)
        {
            QString s;
            s.sprintf("Client method %s is supported by the GUI but not "
                      "enough information was passed in the method request.", 
                      method->GetMethodName().c_str());
            Warning(s);
        }
        else
        {
            // The method is supported and we have all of the information.
            if(method->GetMethodName() == "Quit")
            {
                viewerInitiatedQuit = true;
                Quit();
            }
            else if(method->GetMethodName() == "DeIconify")
                DeIconifyWindows();
            else if(method->GetMethodName() == "Hide")
            {
                mainWin->hide();

                // Iconify all of the regular windows.
                for(WindowBaseMap::iterator pos = otherWindows.begin();
                    pos != otherWindows.end(); ++pos)
                {
                    pos->second->hide();
                }

                // Iconify all of the plot windows.
                for(index = 0; index < plotWindows.size(); ++index)
                {
                    if(plotWindows[index] != 0)
                        plotWindows[index]->hide();
                }

                // Iconify all of the operator windows.
                for(index = 0; index < operatorWindows.size(); ++index)
                {
                    if(operatorWindows[index] != 0)
                        operatorWindows[index]->hide();
                }
            }
            else if(method->GetMethodName() == "Iconify")
                IconifyWindows(false);
            else if(method->GetMethodName() == "Show")
            {
                mainWin->show();

                // Iconify all of the regular windows.
                for(WindowBaseMap::iterator pos = otherWindows.begin();
                    pos != otherWindows.end(); ++pos)
                {
                    pos->second->show();
                }

                // Iconify all of the plot windows.
                for(index = 0; index < plotWindows.size(); ++index)
                {
                    if(plotWindows[index] != 0)
                        plotWindows[index]->show();
                }

                // Iconify all of the operator windows.
                for(index = 0; index < operatorWindows.size(); ++index)
                {
                    if(operatorWindows[index] != 0)
                        operatorWindows[index]->show();
                }
            }
            else if(method->GetMethodName() == "MessageBoxYesNo")
            {
                // Ask the user a question in a yes/no message box.
                if(QMessageBox::information(mainWin, "VisIt",
                   method->GetStringArgs()[0].c_str(),
                   QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                {
                    QTimer::singleShot(10, this, SLOT(SendMessageBoxResult0()));
                }
                else
                    QTimer::singleShot(10, this, SLOT(SendMessageBoxResult1()));
            }
            else if(method->GetMethodName() == "MessageBoxOkCancel")
            {
                // Ask the user a question in a yes/no message box.
                int result = 0;
                if(QMessageBox::information(mainWin, "VisIt",
                   method->GetStringArgs()[0].c_str(),
                   QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel)
                {
                    QTimer::singleShot(10, this, SLOT(SendMessageBoxResult0()));
                }
                else
                    QTimer::singleShot(10, this, SLOT(SendMessageBoxResult1()));
            }
            else if(method->GetMethodName() == "MessageBoxOk")
            {
                // Ask the user a question in a yes/no message box.
                QMessageBox::information(mainWin, "VisIt",
                    method->GetStringArgs()[0].c_str(),
                    QMessageBox::Ok);
                QTimer::singleShot(10, this, SLOT(SendMessageBoxResult0()));
            }
            else if(method->GetMethodName() == "MovieProgress")
            {
                // Set the movie progress dialog's properties.
                if(movieProgress == 0)
                {
                    movieProgress = new QvisMovieProgressDialog(mainWin,
                        "movieProgress");
                    movieProgress->setCaption("VisIt movie progress");
                    movieProgress->setLabelText("Making movie");
                    movieProgress->setProgress(0);
                    connect(movieProgress, SIGNAL(cancelled()),
                            this, SLOT(CancelMovie()));
                }

                if(movieProgress != 0)
                {
                    QString labelText(method->GetStringArgs()[0].c_str());
                    int current = method->GetIntArgs()[0];
                    int total   = method->GetIntArgs()[1];

                    if(!movieProgress->isVisible())
                        movieProgress->show();
                    if(labelText != movieProgress->labelText())
                        movieProgress->setLabelText(labelText);
                    if(total != movieProgress->totalSteps())
                        movieProgress->setTotalSteps(total);
                    if(current != movieProgress->progress())
                        movieProgress->setProgress(current);
                }
            }
            else if(method->GetMethodName() == "MovieProgressEnd")
            {
                if(movieProgress != 0)
                    movieProgress->hide();
            }
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::SendMessageBoxResult
//
// Purpose: 
//   This is a Qt slot function that is called to send messagebox results
//   as a client method.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 9 14:20:56 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::SendMessageBoxResult0()
{
    ClientMethod *method = viewer->GetClientMethod();
    method->SetMethodName("MessageBoxResult");
    method->ClearArgs();
    method->AddArgument(0);
    clientMethodObserver->SetUpdate(false);
    method->Notify();
}

void
QvisGUIApplication::SendMessageBoxResult1()
{
    ClientMethod *method = viewer->GetClientMethod();
    method->SetMethodName("MessageBoxResult");
    method->ClearArgs();
    method->AddArgument(1);
    clientMethodObserver->SetUpdate(false);
    method->Notify();
}

// ****************************************************************************
// Function: QuoteSpaces
//
// Purpose: 
//   Adds quotes around a string that contains spaces.
//
// Arguments:
//   s : The string to check.
//
// Returns:    The original string or a quoted string if the original had
//             spaces.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 12 13:34:39 PST 2005
//
// Modifications:
//   
// ****************************************************************************

std::string
QuoteSpaces(const std::string &s)
{
    bool nospaces = true;
    for(int i = 0; i < s.size() && nospaces; ++i)
        nospaces &= (s[i] != ' ');

    std::string retval(s);
    if(!nospaces)
        retval = std::string("\"") + retval + std::string("\"");

    return retval;
}

// ****************************************************************************
// Function: GetMovieCommandLine
//
// Purpose: 
//   This function creates a command line for "visit -movie" based on the 
//   information stored in the movieAtts.
//
// Arguments:
//   movieAtts : The movie attributes used to create the command line.
//   args      : The return vector for the command line args.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 2 10:50:21 PDT 2005
//
// Modifications:
//   Brad Whitlock, Thu Jul 14 10:55:37 PDT 2005
//   I made it use GetVisItLauncher.
//
//   Brad Whitlock, Fri Oct 28 13:52:31 PST 2005
//   I moved the code to add the visit command to the caller. I also added 
//   code to add the version number if we're not running a development version.
//
// ****************************************************************************

void
GetMovieCommandLine(const MovieAttributes *movieAtts, stringVector &args)
{
    if(!GetIsDevelopmentVersion())
    {
        args.push_back("-v");
        args.push_back(VERSION);
    }

    // iterate over the formats
    args.push_back("-format");
    const stringVector &fmt = movieAtts->GetFileFormats();
    int i;
    std::string F;
    for(i = 0; i < fmt.size(); ++i)
    {
        F += fmt[i];
        if(i < (fmt.size() - 1))
            F += ",";
    }
    args.push_back(F);

    // iterate over the geometries
    args.push_back("-geometry");
    const intVector &w = movieAtts->GetWidths();
    const intVector &h = movieAtts->GetHeights();
    std::string G;
    for(i = 0; i < w.size(); ++i)
    {
        char tmp[100];
        SNPRINTF(tmp, 100, "%dx%d", w[i], h[i]);
        G += tmp;
        if(i < (w.size() - 1))
            G += ",";
    }
    args.push_back(G);

    if(movieAtts->GetStereo())
        args.push_back("-stereo");

    args.push_back("-output");
    std::string dirFile(movieAtts->GetOutputDirectory());
    if(dirFile == ".")
        dirFile += SLASH_STRING;
    dirFile += movieAtts->GetOutputName();
    args.push_back(QuoteSpaces(dirFile));
}

// ****************************************************************************
// Function: UpdateCurrentWindowSizes
//
// Purpose: 
//   This function updates the movieAtts with the correct window width and
//   height.
//
// Arguments:
//   movieAtts     : The movie attributes to update.
//   currentWidth  : The current window width.
//   currentHeight : The current window height.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 22 12:11:07 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
UpdateCurrentWindowSizes(MovieAttributes *movieAtts, int currentWidth,
    int currentHeight)
{
    //
    // Iterate through the movieAtts' useCurrentSize vector and substitute
    // the active window's actual width and height.
    //
    intVector widths(movieAtts->GetWidths());
    intVector heights(movieAtts->GetHeights());
    const unsignedCharVector &useCurrentSize = movieAtts->GetUseCurrentSize();
    const doubleVector &scales = movieAtts->GetScales();
    for(int i = 0; i < widths.size(); ++i)
    {
        if(useCurrentSize[i] == 1)
        {
            const int MAX_WINDOW_SIZE = 4096;
            int w = int(scales[i] * double(currentWidth));
            widths[i] = (w > MAX_WINDOW_SIZE) ? MAX_WINDOW_SIZE : w;
            int h = int(scales[i] * double(currentHeight));
            heights[i] = (h > MAX_WINDOW_SIZE) ? MAX_WINDOW_SIZE : h;
        }
    }
    movieAtts->SetWidths(widths);
    movieAtts->SetHeights(heights);
}

// ****************************************************************************
// Function: MakeCodeSlashes
//
// Purpose: 
//   Turns '\' characters into "\\" in the output string.
//
// Arguments:
//   s : The string to convert.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 11 10:13:48 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

QString
MakeCodeSlashes(const QString &s)
{
#if defined(_WIN32)
    QString retval;
    char tmp[2] = {'\0', '\0'};

    for(int i = 0; i < s.length(); ++i)
    {
        tmp[0] = s[i];
        if(s[i] == '\\')
            retval += tmp;
        retval += tmp;
    }

    return retval;
#else
    return s;
#endif
}

// ****************************************************************************
// Method: QvisGUIApplication::SaveMovie
//
// Purpose: 
//   This is Qt slot function that opens the "Save movie" wizard and leads 
//   the user through setting various movie options.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 21 15:39:15 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Jul 11 09:24:51 PDT 2005
//   Moved creation of the movie progress dialog into the ClientMethod handler
//   so we don't have to put code in the viewer to close the dialog if the
//   CLI can't be launched.
//
// ****************************************************************************

void
QvisGUIApplication::SaveMovie()
{
    MovieAttributes *movieAtts = viewer->GetMovieAttributes();

    // Replace the widths and heights of formats using the current window
    // size with the current window size so the values shown will be right
    // if the user changes the format to use a specific width, height.
    WindowInformation *winInfo = viewer->GetWindowInformation();
    int cw = winInfo->GetWindowSize()[0];
    int ch = winInfo->GetWindowSize()[1];
    UpdateCurrentWindowSizes(movieAtts, cw, ch);

    if(saveMovieWizard == 0)
    {
        saveMovieWizard = new QvisSaveMovieWizard(movieAtts,
            mainWin, "Save movie wizard");
    }
    else
        saveMovieWizard->UpdateAttributes();

    // Execute the save movie wizard to gather the requirements for the movie.
    if(saveMovieWizard->Exec() == QDialog::Accepted)
    {
        // Make the wizard copy its local movieAtts into the viewer's movieAtts
        // and send them to the viewer.
        saveMovieWizard->SendAttributes();

        // Determine the movie's output name.
        QString dirFile(movieAtts->GetOutputDirectory().c_str());
        if(dirFile == ".")
            dirFile += SLASH_STRING;
        dirFile += movieAtts->GetOutputName().c_str();

        // Replace the widths and heights of formats using the current window
        // size with the current window size.
        UpdateCurrentWindowSizes(movieAtts, cw, ch);
        movieAtts->Notify();

        if(movieAtts->GetGenerationMethod() == MovieAttributes::NowCurrentInstance)
        {
            // Determine the location of the makemovie script.
#if defined(_WIN32)
            std::string makemovie(GetVisItArchitectureDirectory() + "\\makemovie.py");
#else
            std::string makemovie(GetVisItArchitectureDirectory() + "/bin/makemovie.py");
#endif

            // Turn "\" into "\\" so the interpreter is happy.
            QString makemovie2(MakeCodeSlashes(makemovie.c_str()));
            dirFile   = MakeCodeSlashes(dirFile);

            // Assemble a string of code to execute.
            QString code; code.sprintf("try:\n    Source('%s')\n", makemovie2.latin1());
            code += "    movie = MakeMovie()\n";
            code += "    movie.usesCurrentPlots = 1\n";
            code += "    movie.sendClientFeedback = 1\n";
            const stringVector &formats = movieAtts->GetFileFormats();
            const intVector &widths  = movieAtts->GetWidths();
            const intVector &heights = movieAtts->GetHeights();
            for(int i = 0; i < formats.size(); ++i)
            {
                QString order;
                order.sprintf("    movie.RequestFormat(\"%s\", %d, %d)\n",
                    formats[i].c_str(), widths[i], heights[i]);
                code += order;
            }
            code += "    movie.movieBase = \"" + dirFile + "\"\n";
            code += "    movie.screenCaptureImages = 0\n";
            code += "    movie.stereo = ";
            if(movieAtts->GetStereo())
                code += "1\n";
            else
                code += "0\n";
            code += "    movie.GenerateFrames()\n";
            code += "    if(movie.EncodeFrames()):\n";
            code += "        movie.Cleanup()\n";
            code += "except VisItInterrupt:\n";
            code += "    pass\n";
            code += "except:\n";
            code += "    ClientMethod(\"MovieProgressEnd\")\n";
            code += "    ClientMethod(\"MessageBoxOk\", \"VisIt could not "
                    "interpret the script to create your movie so no movie "
                    "was generated.\")\n";
            code += "    raise\n";
            code += "ClientMethod(\"MovieProgressEnd\")\n";
            Interpret(code);
        }
        else
        {
            // Save the current session.
            QString msg, sessionFile(SaveSessionFile(dirFile));
            bool errFlag = false;

            // Get the command line arguments.
            stringVector args;
            args.push_back(QuoteSpaces(GetVisItLauncher()));
            args.push_back("-movie");
            GetMovieCommandLine(movieAtts, args);
            args.push_back("-sessionfile");
            args.push_back(QuoteSpaces(std::string(sessionFile.latin1())));
            for(int m = 0; m < movieArguments.size(); ++m)
                args.push_back(movieArguments[m]);

            if (movieAtts->GetGenerationMethod() == MovieAttributes::NowNewInstance)
            {
                msg = "VisIt executed ";

                // Fire off "visit -movie" under the covers. Perhaps have some
                // stuff to send back progress to this process.
                QString program(args[0].c_str());
                QProcess *movieMaker = new QProcess(program, this, "movieMaker");
                for(int i = 1; i < args.size(); ++i)
                    movieMaker->addArgument(args[i].c_str());
                movieMaker->setCommunication(0);
                if(!movieMaker->start())
                {
                    errFlag = true;
                    Error(QString("VisIt could not run ") + program + QString("."));
                }
            }
            else
            {
                msg = "Execute";
            }

            if(!errFlag)
            {
                // Finish creating the message.
                msg += " the following command line to begin making your movie:\n\n";
                for(int i = 0; i < args.size(); ++i)
                {
                    msg += args[i].c_str();
                    msg += " ";
                }

                // Open a dialog that lists the "visit -movie" command that 
                // you have to run to make the movie.
                QMessageBox::information(mainWin, "VisIt", msg, QMessageBox::Ok);
            }
        }
    }
}

// ****************************************************************************
// Method: QvisGUIApplication::CancelMovie
//
// Purpose: 
//   This is a Qt slot function that is called by the Movie Progress Dialog
//   when we click its Cancel button.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 21 11:12:27 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::CancelMovie()
{
    ClientMethod *method = viewer->GetClientMethod();
    method->ClearArgs();
    method->SetMethodName("Interrupt");
    method->Notify();
}

// ****************************************************************************
// Method: QvisGUIApplication::Interpret
//
// Purpose: 
//   This is a Qt slot function that tells the interpreter to execute a string
//   containing code of some type.
//
// Arguments:
//   s : The code to execute.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 10:49:55 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::Interpret(const QString &s)
{
    if(interpreter == 0)
    {
        interpreter = new QvisInterpreter(this, "Interpreter");
        connect(interpreter, SIGNAL(Synchronize()),
                this, SLOT(InterpreterSync()));
    }

    interpreter->Interpret(s);
}

// ****************************************************************************
// Method: QvisGUIApplication::InterpreterSync
//
// Purpose: 
//   Starts a GUI synchronization that will cause the interpreter to execute
//   code when the synchronization returns.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 6 11:17:31 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisGUIApplication::InterpreterSync()
{
    Synchronize(INTERPRETER_SYNC_TAG);
}

//
// Qt slot functions to show windows that are created later on demand.
//

void QvisGUIApplication::showFileSelectionWindow()   { GetInitializedWindowPointer(WINDOW_FILE_SELECTION)->show(); }
void QvisGUIApplication::showFileInformationWindow() { GetInitializedWindowPointer(WINDOW_FILE_INFORMATION)->show(); }
void QvisGUIApplication::showHostProfilesWindow()    { GetInitializedWindowPointer(WINDOW_HOSTPROFILES)->show(); }
void QvisGUIApplication::showSaveWindow()            { GetInitializedWindowPointer(WINDOW_SAVE)->show(); }
void QvisGUIApplication::showEngineWindow()          { GetInitializedWindowPointer(WINDOW_ENGINE)->show(); }
void QvisGUIApplication::showAnimationWindow()       { GetInitializedWindowPointer(WINDOW_ANIMATION)->show(); }
void QvisGUIApplication::showAnnotationWindow()      { GetInitializedWindowPointer(WINDOW_ANNOTATION)->show(); }
void QvisGUIApplication::showCommandWindow()         { GetInitializedWindowPointer(WINDOW_COMMAND)->show(); }
void QvisGUIApplication::showExpressionsWindow()     { GetInitializedWindowPointer(WINDOW_EXPRESSIONS)->show(); }
void QvisGUIApplication::showSubsetWindow()          { GetInitializedWindowPointer(WINDOW_SUBSET)->show(); }
void QvisGUIApplication::showViewWindow()            { GetInitializedWindowPointer(WINDOW_VIEW)->show(); }
void QvisGUIApplication::showKeyframeWindow()        { GetInitializedWindowPointer(WINDOW_KEYFRAME)->show(); }
void QvisGUIApplication::showLightingWindow()        { GetInitializedWindowPointer(WINDOW_LIGHTING)->show(); }
void QvisGUIApplication::showGlobalLineoutWindow()   { GetInitializedWindowPointer(WINDOW_GLOBALLINEOUT)->show(); }
void QvisGUIApplication::showMaterialWindow()        { GetInitializedWindowPointer(WINDOW_MATERIALOPTIONS)->show(); }
void QvisGUIApplication::showHelpWindow()            { GetInitializedWindowPointer(WINDOW_HELP)->show(); }
void QvisGUIApplication::displayCopyright()          { ((QvisHelpWindow *)GetInitializedWindowPointer(WINDOW_HELP))->displayCopyright(); }
void QvisGUIApplication::displayReleaseNotes()       { ((QvisHelpWindow *)GetInitializedWindowPointer(WINDOW_HELP))->displayReleaseNotes(); }
void QvisGUIApplication::displayReleaseNotesIfAvailable()
                                                     { ((QvisHelpWindow *)GetInitializedWindowPointer(WINDOW_HELP))->displayReleaseNotesIfAvailable(); }
void QvisGUIApplication::showQueryWindow()           { GetInitializedWindowPointer(WINDOW_QUERY)->show(); }
void QvisGUIApplication::showRenderingWindow()       { GetInitializedWindowPointer(WINDOW_RENDERING)->show(); }
void QvisGUIApplication::showCorrelationListWindow() { GetInitializedWindowPointer(WINDOW_CORRELATION)->show(); }
void QvisGUIApplication::showQueryOverTimeWindow()   { GetInitializedWindowPointer(WINDOW_TIMEQUERY)->show(); }
void QvisGUIApplication::showInteractorWindow()      { GetInitializedWindowPointer(WINDOW_INTERACTOR)->show(); }
void QvisGUIApplication::showSimulationWindow()      { GetInitializedWindowPointer(WINDOW_SIMULATION)->show(); }
void QvisGUIApplication::showExportDBWindow()        { GetInitializedWindowPointer(WINDOW_EXPORT_DB)->show(); }
