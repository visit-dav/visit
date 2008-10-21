#include <VisItViewer.h>

#include <qdir.h>
#include <avtCallback.h>

#include <ViewerFileServer.h>
#include <ViewerSubject.h>
#include <ViewerState.h>
#include <ViewerMethods.h>

#include <DebugStream.h>
#include <VisItInit.h>
#include <InitVTK.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>

#include <StringHelpers.h>

#include <visit-config.h>

#include <QtVisWindow.h>
#include <vtkQtRenderWindow.h>

// Error handling callback functions.
static void ViewerErrorCallback(void *, const char *);
static void ViewerWarningCallback(void *, const char *);
// Log GLX and display information.
static void LogGlxAndXdpyInfo();

// ****************************************************************************
// Method: VisItViewer::Initialize
//
// Purpose: 
//   Initializes the viewer library, etc.
//
// Arguments:
//   argc : The number of command line args.
//   argv : The command line args.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:31:28 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Initialize(int *argc, char ***argv)
{
    VisItInit::SetComponentName("viewer");
    VisItInit::Initialize(*argc, *argv, 0, 1, false);
    LogGlxAndXdpyInfo();
}

// ****************************************************************************
// Method: VisItViewer::Finalize
//
// Purpose: 
//   Finalizes the viewer library.
//
// Note:       No viewer calls should be made after calling this function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:32:17 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Finalize()
{
    // Unload plugins.
    delete ViewerBase::GetPlotPluginManager();
    delete ViewerBase::GetOperatorPluginManager();

    VisItInit::Finalize();
}

// ****************************************************************************
// Method: VisItViewer::VisItViewer
//
// Purpose: 
//   Constructor for the VisItViewer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:32:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

VisItViewer::VisItViewer() : visitHome()
{
    visitHomeMethod = FromEnvironment;
    viewer = new ViewerSubject;

    //
    // Initialize the error logging.
    //
    VisItInit::ComponentRegisterErrorFunction(ViewerErrorCallback, (void*)viewer);
    InitVTK::Initialize();
    avtCallback::RegisterWarningCallback(ViewerWarningCallback, 
                                         (void*)viewer);
}

// ****************************************************************************
// Method: VisItViewer::~VisItViewer
//
// Purpose: 
//   Destructor for the VisItViewer class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:33:31 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

VisItViewer::~VisItViewer()
{
    delete viewer;
}

// ****************************************************************************
// Method: VisItViewer::SetVISITHOMEMethod
//
// Purpose: 
//   Set the method used for determining VISITHOME.
//
// Arguments:
//   m : The method to use for determining VISITHOME.
//
// Returns:    
//
// Note:       Must be called before ProcessCommandLine.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:23:29 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::SetVISITHOMEMethod(VISITHOME_METHOD m)
{
    visitHomeMethod = m;
    if(visitHomeMethod != UserSpecified)
        visitHome = "";
}

// ****************************************************************************
// Method: VisItViewer::SetVISITHOME
//
// Purpose: 
//   Set the path to VISITHOME. Implicitly selects UserDefined method for
//   for determining VISITHOME.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:24:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************
void
VisItViewer::SetVISITHOME(const std::string &path)
{
    visitHomeMethod = UserSpecified;
    visitHome = path;
}

// ****************************************************************************
// Method: VisItViewer::GetVisItHome
//
// Purpose: 
//   Return VisItHome based on the method that we've selected.
//
// Arguments:
//
// Returns:    The value that we'll use for VISITHOME
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 11:34:23 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
VisItViewer::GetVisItHome() const
{
    std::string home;
    if(visitHomeMethod == FromEnvironment)
    {
        if(getenv("VISITHOME") != NULL)
            home = getenv("VISITHOME");
    }
    else if(visitHomeMethod == FromArgv0)
    {
        QString appDir(QDir::current().path());
#if defined(__WIN32)
        int index = appDir.findRev("\\");
#else
        int index = appDir.findRev("/");
#endif
        if(index != -1)
            home = appDir.left(index).latin1();
        else
            home = appDir.latin1();
    }
    else // UserSpecified
        home = visitHome;

    return home;
}

// ****************************************************************************
// Method: VisItViewer::ProcessCommandLine
//
// Purpose: 
//   Sets various viewer options by examining the command line.
//
// Arguments:
//   argc   : The number of command line args.
//   argv   : The command line args.
//   addDir : Whether the application's startup dir should be added.
//
// Returns:    
//
// Note:       Calling this method is optional but it is encouraged so you
//             can pass options to the viewer. In addition, if you are not
//             setting VISITHOME from the environment then you really need to
//             call this function to ensure that the appropriate -dir options
//             are passed when launching other local VisIt components.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:33:49 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Oct 16 19:29:12 EDT 2008
//    Added a flag for whether or not ProcessCommandLine should force 
//    the identical version.  It's necessary for new viewer-apps but
//    might confuse VisIt-proper's smart versioning.
//   
// ****************************************************************************

void
VisItViewer::ProcessCommandLine(int argc, char **argv, bool addForceVersion)
{
    if(visitHomeMethod == FromArgv0 || visitHomeMethod == UserSpecified)
    {
        char dirName[1024];
        strcpy(dirName, GetVisItHome().c_str());

        // Call ProcessCommandLine with 4 extra arguments so the right
        // mdserver and engine get called if we're launching VisIt components
        // from an embedded application where we can't guarantee that various
        // VisIt environment variables will be set.
        int argc2 = argc + 4;
        char **argv2 = new char*[argc2 + 1];
        for(int i = 0; i < argc; ++i)
            argv2[i] = argv[i];
        argv2[argc    ] = "-dir";
        argv2[argc + 1] = dirName;
        argv2[argc + 2] = "-forceversion";
        argv2[argc + 3] = VERSION;
        argv2[argc + 4] = NULL;
        if (!addForceVersion)
        {
            argv2[argc + 2] = NULL;
            argc2 -= 2;
        }
        viewer->ProcessCommandLine(argc2, argv2);
        delete [] argv2;
    }
    else
    {
        // Call ProcessCommandLine with 2 extra arguments so the right
        // mdserver and engine get called if we're launching VisIt components
        // from an embedded application where we can't guarantee that various
        // VisIt environment variables will be set.
        int argc2 = argc + 2;
        char **argv2 = new char*[argc2 + 1];
        for(int i = 0; i < argc; ++i)
            argv2[i] = argv[i];
        argv2[argc    ] = "-forceversion";
        argv2[argc + 1] = VERSION;
        argv2[argc + 2] = NULL;
        if (!addForceVersion)
        {
            argv2[argc] = NULL;
            argc2 -= 2;
        }
        viewer->ProcessCommandLine(argc2, argv2);
        delete [] argv2;
    }
}

// ****************************************************************************
// Method: VisItViewer::Connect
//
// Purpose: 
//   Connects the viewer to the VisIt client that launched it.
//
// Arguments:
//   argc : The number of command line args.
//   argv : The command line args.
//
// Returns:    
//
// Note:       Calling this method is optional unless you want your viewer
//             application to be driven by a client application.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:34:48 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Connect(int *argc, char ***argv)
{
    viewer->Connect(argc, argv);
}

// ****************************************************************************
// Method: VisItViewer::SetWindowCreationCallback
//
// Purpose: 
//   This method installs a window creation function so that you have more
//   control over how your vtkQtRenderWindow objects are created.
//
// Arguments:
//   wcc : The window creation callback function.
//   wccdata : Data to be passed to the window creation callback function.
//
// Returns:    
//
// Note:       This method must be called before Setup() to have any effect
//             on the first vis window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:37:03 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::SetWindowCreationCallback(vtkQtRenderWindow* (*wcc)(void *),
                                       void *wccdata)
{
    // Turn off using the window metrics because we're going to embed our
    // windows in another application's widgets.
    viewer->SetUseWindowMetrics(false);
    // Set the window creation callback.
    QtVisWindow::SetWindowCreationCallback(wcc, wccdata);
//    ViewerWindowManager::Instance()->DisableWindowDeletion();
}

// ****************************************************************************
// Method: VisItViewer::Setup
//
// Purpose: 
//   Sets up most of the viewer objects.
//
// Arguments:
//
// Returns:    
//
// Note:       This method should be called after Connect, ProcessCommandLine
//             but before any
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:36:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Setup()
{
    // If the plugin dir has not been set then let's set it in the plugin
    // managers based on the current directory.
    if(getenv("VISITPLUGINDIR") == NULL)
    {
        std::string pluginDir(GetVisItHome() + "/plugins");

        viewer->GetOperatorPluginManager()->SetPluginDir(pluginDir.c_str());
        viewer->GetPlotPluginManager()->SetPluginDir(pluginDir.c_str());
    }

    viewer->Initialize();
}

// ****************************************************************************
// Method: VisItViewer::RemoveCrashRecoveryFile
//
// Purpose: 
//   Removes the crash recovery file.
//
// Note:       This method should be called on exit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:38:34 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::RemoveCrashRecoveryFile() const
{
    viewer->RemoveCrashRecoveryFile();
}

// ****************************************************************************
// Method: VisItViewer::GetNowinMode
//
// Purpose: 
//   Returns whether the viewer has been told to run -nowin.
//
// Returns:    True if the viewer is running -nowin, false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:38:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

bool
VisItViewer::GetNowinMode() const
{
    return viewer->GetNowinMode();
}

// ****************************************************************************
// Method: VisItViewer::Methods
//
// Purpose: 
//   Returns the methods object that lets you control the viewer.
//
// Arguments:
//
// Returns:    The methods object for controlling the viewer.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:39:33 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerMethods *
VisItViewer::Methods() const
{
    return viewer->GetViewerMethods();
}

ViewerMethods *
VisItViewer::DelayedMethods() const
{
    return viewer->GetViewerDelayedMethods();
}

// ****************************************************************************
// Method: VisItViewer::State
//
// Purpose: 
//   Returns the viewer state so you can access various state objects.
//
// Arguments:
//
// Returns:    The viewer state.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:03 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
VisItViewer::State() const
{
    return viewer->GetViewerState();
}

ViewerState *
VisItViewer::DelayedState() const
{
    return viewer->GetViewerDelayedState();
}


// ****************************************************************************
// Method: VisItViewer::GetMetaData
//
// Purpose: 
//   Returns a file's metadata.
//
// Arguments:
//   hostDB : The host:db filename.
//   ts       : The time state [optional].
//
// Returns:    The metadata for the file.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 16:37:55 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

const avtDatabaseMetaData *
VisItViewer::GetMetaData(const std::string &hostDB, int ts)
{
    std::string host, db;
    ViewerFileServer::SplitHostDatabase(hostDB, host, db);
    const avtDatabaseMetaData *md = 0;
    if(ts == -1)
        md = ViewerFileServer::Instance()->GetMetaData(host, db);
    else
        md = ViewerFileServer::Instance()->GetMetaDataForState(host, db, ts);
    return md;
}

// ****************************************************************************
// Method: VisItViewer::Error
//
// Purpose: 
//   Lets you issue an error message to VisIt clients.
//
// Arguments:
//   msg : The message to issue.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Error(const QString &msg)
{
    viewer->Error(msg);
}

// ****************************************************************************
// Method: VisItViewer::Warning
//
// Purpose: 
//   Lets you issue a warning message to VisIt clients.
//
// Arguments:
//   msg : The message to issue.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 16:40:26 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
VisItViewer::Warning(const QString &msg)
{
    viewer->Warning(msg);
}

// ****************************************************************************
// Method: VisItViewer::GetNumPlotPlugins
//
// Purpose: 
//   Return the number of plot plugins.
//
// Arguments:
//
// Returns:    The number of plot plugins.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:42:39 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
VisItViewer::GetNumPlotPlugins() const
{
    return viewer->GetPlotPluginManager()->GetNEnabledPlugins();
}

// ****************************************************************************
// Method: VisItViewer::GetPlotName
//
// Purpose: 
//   Returns the name of the i'th plot plugin.
//
// Arguments:
//   index : The index of the plot plugin whose name we want.
//
// Returns:    The name of the index'th plot plugin.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:42:59 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
VisItViewer::GetPlotName(int index) const
{
    std::string name; 
    if(index >= 0 && index < GetNumPlotPlugins())
    {
        std::string id = viewer->GetPlotPluginManager()->GetEnabledID(index);
        name = std::string(viewer->GetPlotPluginManager()->
                               GetViewerPluginInfo(id)->GetName());
    }
    return name;
}

// ****************************************************************************
// Method: VisItViewer::GetPlotIndex
//
// Purpose: 
//   Returns the index of the plot having the specified name.
//
// Arguments:
//   plotName : The name of the plot for which we want an index.
//
// Returns:    The index of the plot or -1 on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:43:41 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
VisItViewer::GetPlotIndex(const std::string &plotName) const
{
    for(int i = 0; i < GetNumPlotPlugins(); ++i)
    {
        if(GetPlotName(i) == plotName)
            return i;
    }
    return -1;
}

// ****************************************************************************
// Method: VisItViewer::GetNumOperatorPlugins
//
// Purpose: 
//   Get the number of operator plugins.
//
// Arguments:
// 
// Returns:    The number of operator plugins.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:44:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
VisItViewer::GetNumOperatorPlugins() const
{
    return viewer->GetOperatorPluginManager()->GetNEnabledPlugins();
}

// ****************************************************************************
// Method: VisItViewer::GetOperatorName
//
// Purpose: 
//   Get the name of the i'th operator.
//
// Arguments:
//   index : The index of the operator whose name we want.
//
// Returns:    The name of the operator.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:45:04 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
VisItViewer::GetOperatorName(int index) const
{
    std::string name; 
    if(index >= 0 && index < GetNumOperatorPlugins())
    {
        std::string id = viewer->GetOperatorPluginManager()->GetEnabledID(index);
        name = std::string(viewer->GetOperatorPluginManager()->
                               GetViewerPluginInfo(id)->GetName());
    }
    return name;
}

// ****************************************************************************
// Method: VisItViewer::GetOperatorIndex
//
// Purpose: 
//   Gets the index of the operator, given its name.
//
// Arguments:
//   operatorName : The operator name whose index we want.
//
// Returns:    The index of the operator or -1 on failure.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 19 15:45:52 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

int
VisItViewer::GetOperatorIndex(const std::string &operatorName) const
{
    for(int i = 0; i < GetNumOperatorPlugins(); ++i)
    {
        if(GetOperatorName(i) == operatorName)
            return i;
    }
    return -1;
}

// ****************************************************************************
// Method: VisItViewer::GetVisItCommand
//
// Purpose: 
//   Returns the VisIt command used by VISITHOME.
//
// Arguments:
//
// Returns:    The VisIt command used by VISITHOME.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 21 15:20:06 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

std::string
VisItViewer::GetVisItCommand() const
{
#if defined(__WIN32)
    return GetVisItHome() + "\\visit.exe";
#else
    return GetVisItHome() + "/bin/visit";
#endif
}

// ****************************************************************************
//  Function: ViewerErrorCallback
//
//  Purpose:
//      A callback routine that can issue error messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   August 8, 2003
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ViewerErrorCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->Error(msg);
}


// ****************************************************************************
//  Function: ViewerWarningCallback
//
//  Purpose:
//      A callback routine that can issue warning messages.
//
//  Arguments:
//      args    Arguments to the callback.
//      msg     The message to issue.
//
//  Programmer: Hank Childs
//  Creation:   February 15, 2005
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:17:49 PST 2007
//    Passed in the ViewerSubject pointer.
//
// ****************************************************************************

static void
ViewerWarningCallback(void *ptr, const char *msg)
{
    ((ViewerSubject *)ptr)->Warning(msg);
}

// ****************************************************************************
//  Function: Log output from a piped system command to debug logs
//
//  Programmer: Mark C. Miller
//  Created:    April 9, 2008
// ****************************************************************************

static void
LogCommand(const char *cmd, const char *truncate_at_pattern)
{
#if !defined(_WIN32)
    char buf[256];
    buf[sizeof(buf)-1] = '\0';
    FILE *pfile = popen(cmd,"r");

    if (pfile)
    {
        debug5 << endl;
        debug5 << "Begin output from \"" << cmd << "\"..." << endl;
        debug5 << "-------------------------------------------------------------" << endl;
        while (fgets(buf, sizeof(buf)-1, pfile) != 0)
        {
            if (truncate_at_pattern && (StringHelpers::FindRE(buf, truncate_at_pattern) != StringHelpers::FindNone))
            {
                debug5 << "############### TRUNCATED #################" << endl;
                break;
            }
            debug5 << buf;
        }
        debug5 << "End output from \"" << cmd << "\"..." << endl;
        debug5 << "-------------------------------------------------------------" << endl;
        debug5 << endl;
        pclose(pfile);
    }
#endif
}

// ****************************************************************************
//  Function: Log output from xdpyinfo and glxinfo commands w/truncation 
//
//  Programmer: Mark C. Miller
//  Created:    April 9, 2008
//
//  Modifications:
//    Mark C. Miller, Thu Apr 10 08:16:51 PDT 2008
//    Truncated glxinfo output
//
//    Sean Ahern, Thu Apr 24 18:17:33 EDT 2008
//    Avoided this if we're on the Mac.
//
//    Kathleen Bonnell, Wed Apr 30 10:59:18 PDT 2008 
//    Windows compiler doesn't like 'and', use '&&' instead. 
//
// ****************************************************************************

static void
LogGlxAndXdpyInfo()
{
#if !defined(_WIN32) && !defined(Q_WS_MACX)
    if (debug5_real)
    {
        LogCommand("xdpyinfo", "number of visuals"); // truncate at list of visuals
        LogCommand("glxinfo -v -t", "^Vis  Vis");    // truncate at table of visuals
    }
#endif
}

