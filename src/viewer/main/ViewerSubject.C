/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
//                               ViewerSubject.C                             //
// ************************************************************************* //

#include <ctype.h>
#include <stdlib.h>
#include <snprintf.h>
#include <ViewerSubject.h>

#include <QDesktopWidget>
#include <QFile>
#include <QLocale>
#include <QTimer>
#include <QTranslator>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObjectList.h>
#include <Appearance.h>
#include <AppearanceAttributes.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ColorTableAttributes.h>
#include <ConstructDDFAttributes.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DBPluginInfoAttributes.h>
#include <EngineKey.h>
#include <EngineList.h>
#include <ExportDBAttributes.h>
#include <FileOpenOptions.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <HostProfile.h>
#include <HostProfileList.h>
#include <VisItInit.h>
#include <InitVTK.h>
#include <InstallationFunctions.h>
#include <InteractorAttributes.h>
#include <InvalidVariableException.h>
#include <KeyframeAttributes.h>
#include <LostConnectionException.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <MovieAttributes.h>
#include <PickAttributes.h>
#include <PlotInfoAttributes.h>
#include <PlotList.h>
#include <PostponedAction.h>
#include <PluginManagerAttributes.h>
#include <PrinterAttributes.h>
#include <ProcessAttributes.h>
#include <QueryAttributes.h>
#include <QueryList.h>
#include <RemoteProcess.h>
#include <RenderingAttributes.h>
#include <SaveWindowAttributes.h>
#include <SILRestrictionAttributes.h>
#include <SimulationCommand.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <Utility.h>
#include <ViewerRPC.h>
#include <Xfer.h>

#include <ViewerActionBase.h>
#include <ViewerActionManager.h>
#include <ViewerConnectionProgressDialog.h>
#include <ParsingExprList.h>
#include <ViewerClientConnection.h>
#include <ViewerCommandFromSimObserver.h>
#include <ViewerConfigManager.h>
#include <ViewerEngineManager.h>
#include <ViewerRemoteProcessChooser.h>
#include <ViewerFileServer.h>
#include <ViewerMessageBuffer.h>
#include <ViewerMetaDataObserver.h>
#include <ViewerMethods.h>
#include <ViewerObserverToSignal.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPasswordWindow.h>
#include <ViewerPlot.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerServerManager.h>
#include <ViewerState.h>
#include <ViewerStateBuffered.h>
#include <ViewerSILAttsObserver.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>
#include <VisWinRendering.h>
#include <WindowInformation.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>

#include <QApplication>
#include <QSocketNotifier>
#include <QvisColorTableButton.h>
#include <DebugStream.h>
#include <TimingsManager.h>
#include <WindowMetrics.h>

#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDatabaseMetaData.h>
#include <avtSimulationInformation.h>

#if !defined(_WIN32)
#include <strings.h>
#include <unistd.h>
#else
#include <process.h>
static int nConfigArgs = 1;
#endif

#include <algorithm>

static std::string getToken(std::string buff, bool reset);
static int getVectorTokens(std::string buff, std::vector<std::string> &tokens, int nodeType);

struct DeferredCommandFromSimulation
{
    EngineKey   key;
    std::string db;
    std::string command;
};

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
//    Jeremy Meredith, Wed Aug 25 10:32:18 PDT 2004
//    Added metadata and SIL attributes (needed for simulations).
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added procAtts
//
//    Brad Whitlock, Fri Apr 15 10:42:31 PDT 2005
//    Added postponedAction.
//
//    Brad Whitlock, Mon May 2 14:02:43 PST 2005
//    Made parent be a pointer, added clients, viewerState, inputConnection,
//    clientMethod, clientInformation, clientInformationList.
//
//    Hank Childs, Mon Jul 18 16:00:32 PDT 2005
//    Initialize qt_argv.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006 
//    Add plotInfoAtts. 
//
//    Brad Whitlock, Mon Feb 12 17:36:41 PST 2007
//    Changed base class to ViewerBase.
//
//    Brad Whitlock, Thu Apr 10 09:49:27 PDT 2008
//    Added applicationLocale.
//
//    Brad Whitlock, Fri May  9 14:39:09 PDT 2008
//    Qt 4.
//
//    Brad Whitlock, Thu Aug 14 09:57:59 PDT 2008
//    Removed mainApp, Added call to CreateState.
//
//    Brad Whitlock, Thu Apr  9 15:58:47 PDT 2009
//    I initialized openDatabaseOnStartup, openScriptOnStartup.
//
//    Brad Whitlock, Tue Apr 14 11:47:41 PDT 2009
//    I moved many members into ViewerProperties.
//
// ****************************************************************************

ViewerSubject::ViewerSubject() : ViewerBase(0), 
    launchEngineAtStartup(), openDatabaseOnStartup(), openScriptOnStartup(),
    interpretCommands(), xfer(), clients(),
    engineParallelArguments(), unknownArguments(), clientArguments()
{
    //
    // Initialize pointers to some Qt objects that don't get created
    // until later.
    //
    checkParent = 0;
    checkRenderer = 0;

    //
    // We start out with no ParentProcess object and it's only temporary anyway.
    //
    parent = 0;
    inputConnection = 0;

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
    // Initialize pointers to some objects that don't get created until later.
    //
    keepAliveTimer = 0;
    viewerRPCObserver = 0;
    postponedActionObserver = 0;
    clientMethodObserver = 0;
    clientInformationObserver = 0;
    syncObserver = 0;
    colorTableObserver = 0;

    //
    // By default, read the config files.
    //
    configMgr = new ViewerConfigManager(this);
    systemSettings = 0;
    localSettings = 0;

    //
    // Initialize pointers to some objects that don't get created until later.
    //
    plotFactory = 0;
    operatorFactory = 0;
    messageBuffer = new ViewerMessageBuffer;
    messagePipe[0] = -1; messagePipe[1] = -1;

    viewerSubject = this;   // FIX_ME Hack, this should be removed.

    //
    // Create and connect state objects.
    //
    CreateState();
    viewerDelayedState = 0;
    viewerDelayedMethods = 0;
}

// ****************************************************************************
//  Method: ViewerSubject destructor
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Aug 25 10:32:18 PDT 2004
//    Added metadata and SIL attributes (needed for simulations).
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added procAtts
//
//    Brad Whitlock, Fri Apr 15 11:11:17 PDT 2005
//    Added postponedActionObserver.
//
//    Brad Whitlock, Mon May 2 14:03:29 PST 2005
//    Added viewerState, inputConnection, clientMethod, clientInformation
//    clientInformationList, and movieAtts.
//
//    Hank Childs, Mon Jul 18 16:00:32 PDT 2005
//    Free qt_argv.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006 
//    Add plotInfoAtts. 
//
//    Mark C. Miller, Wed Jan 10 11:50:51 PST 2007
//    Fixed mismatched delete for configFileName
//
//    Brad Whitlock, Mon Feb 12 10:33:39 PDT 2007
//    Delete the ViewerState and ViewerMethods objects.
//
//    Brad Whitlock, Wed Aug 20 15:45:57 PDT 2008
//    Delete viewerDelayedState.
//
//    Brad Whitlock, Tue Apr 14 11:21:46 PDT 2009
//    Delete viewer properties.
//
// ****************************************************************************

ViewerSubject::~ViewerSubject()
{
    delete messageBuffer;
    delete viewerRPCObserver;
    delete postponedActionObserver;
    delete clientMethodObserver;
    delete clientInformationObserver;
    delete colorTableObserver;

    delete plotFactory;
    delete operatorFactory;
    delete configMgr;
    delete syncObserver;

    delete GetViewerProperties();
    delete GetViewerState();
    delete GetViewerMethods();
    delete inputConnection;
    delete viewerDelayedState;
    delete viewerDelayedMethods;
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
//    Brad Whitlock, Thu Aug 14 10:02:40 PDT 2008
//    Changed the method so it only connects to the parent process. Connecting
//    to a parent process will now be optional based on whether this method
//    is called.
//
//    Brad Whitlock, Tue Apr 14 11:57:47 PDT 2009
//    Set LaunchedByClient in the ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::Connect(int *argc, char ***argv)
{
    if(parent == 0)
    {
        int timeid = visitTimer->StartTimer();
        parent = new ParentProcess;
        if(parent->Connect(1, 1, argc, argv, true))
            GetViewerProperties()->SetLaunchedByClient(true);
        visitTimer->StopTimer(timeid, "Connecting to client");
    }
}

// ****************************************************************************
// Method: ViewerSubject::Initialize
//
// Purpose: 
//   First stage of initialization for various objects in the viewer subject.
//   Other stages are started by calling this method.
//
// Arguments:
//
// Returns:    
//
// Note:       Must be called after ProcessArguments(), Connect() and after a
//             QApplication has been created.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 14 10:04:24 PDT 2008
//
// Modifications:
//   Brad Whitlock, Wed Nov 26 11:39:27 PDT 2008
//   Get the current appearance attributes
//
//   Brad Whitlock, Tue Apr 14 11:57:11 PDT 2009
//   Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::Initialize()
{
    int timeid = visitTimer->StartTimer();

    // Make VisIt translation aware.
    QTranslator *translator = new QTranslator(0);
#if defined(_WIN32)
    QString transPath(GetVisItArchitectureDirectory().c_str());
    transPath += "\\translations\\";
#else
    QString transPath(GetVisItArchitectureDirectory().c_str());
    transPath += "/bin/translations/";
#endif
    QString applicationLocale(GetViewerProperties()->GetApplicationLocale().c_str());
    if(applicationLocale == "default")
        applicationLocale = QLocale::system().name();
    QString transFile(QString("visit_") + applicationLocale);
    debug1 << "Trying to load translator file: " << (transPath + transFile).toStdString() << endl;
    if(translator->load(transFile, transPath))
    {
        qApp->installTranslator(translator);
        debug1 << "Loaded translation " << (transPath + transFile).toStdString() << endl;
    }
    else
    {
        debug1 << "Could not load translation." << endl;
        delete translator;
    }

    // Customize the colors and fonts.
    if (!GetViewerProperties()->GetNowin())
    {
        GetAppearance(qApp, GetViewerState()->GetAppearanceAttributes());
        CustomizeAppearance();
    }

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
    {
        //
        // Disable reading of commands from the client.
        //
        BlockSocketSignals(true);

        //
        // Do heavy initialization.
        //
        HeavyInitialization();

        //
        // Enable reading of commands from the client. Note that we call
        // the EnableSocketSignals slot function using a timer because it
        // must be called from the event loop in case ProcessConfigFileSettings
        // added a slot function to the event loop. This prevents race
        // conditions with reading settings and processing commands from the
        // client.
        //
        QTimer::singleShot(350, this, SLOT(EnableSocketSignals()));
    }

    visitTimer->StopTimer(timeid, "Total time setting up");
}

// ****************************************************************************
// Method: ViewerSubject::CreateState
//
// Purpose: 
//   Creates the viewer's state objects and adds them to the viewerState
//   object, which lets us more easily create copies of the viewer's state.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 16:19:53 PST 2005
//
// Modifications:
//   
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Hank Childs, Mon Feb 13 21:54:12 PST 2006
//   Added construct ddf attributes.
//
//   Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006 
//   Add plotInfoAtts. 
//
//   Brad Whitlock, Mon Feb 12 11:05:27 PDT 2007
//   Rewrote to use ViewerState.
//
//   Jeremy Meredith, Wed Jan 23 16:30:13 EST 2008
//   Added file open options.
//
//   Jeremy Meredith, Mon Feb  4 13:31:02 EST 2008
//   Added remaining axis array view support.
//
// ****************************************************************************

void
ViewerSubject::CreateState()
{
    ViewerState *s = GetViewerState();

    // The ViewerState object automatically creates its own state objects in
    // the right order. However, certain objects in the viewer use their own
    // copies and it is those objects that we need to use in the ViewerState
    // object, etc. Let's override the values of some of the objects in the
    // ViewerState object with those of the other viewer objects.
    //
    // Since the important viewer objects now inherit from ViewerBase, they should
    // use the object from ViewerState *ViewerBase::GetState() when possible 
    // instead of maintaining their own objects. If we eventually switch to that
    // paradigm then we can delete this code!

    s->SetDBPluginInfoAttributes(ViewerFileServer::Instance()->GetDBPluginInfoAtts(), false);
    s->SetFileOpenOptions(ViewerFileServer::Instance()->GetFileOpenOptions(), false);
    s->SetExportDBAttributes(ViewerEngineManager::Instance()->GetExportDBAtts(),  false);
    s->SetConstructDDFAttributes(ViewerEngineManager::Instance()->GetConstructDDFAtts(),  false);
    s->SetGlobalAttributes(ViewerWindowManager::GetClientAtts(), false);
    s->SetDatabaseCorrelationList(ViewerFileServer::Instance()->GetDatabaseCorrelationList(), false);
    s->SetPlotList(ViewerPlotList::GetClientAtts(), false);
    s->SetHostProfileList(ViewerEngineManager::GetClientAtts(), false);
    s->SetSaveWindowAttributes(ViewerWindowManager::GetSaveWindowClientAtts(), false);
    s->SetEngineList(ViewerEngineManager::GetEngineList(), false);
    s->SetColorTableAttributes(avtColorTables::Instance()->GetColorTables(), false);
    s->SetExpressionList(ParsingExprList::Instance()->GetList(), false);
    s->SetAnnotationAttributes(ViewerWindowManager::Instance()->GetAnnotationClientAtts(), false);
    s->SetSILRestrictionAttributes(ViewerPlotList::GetClientSILRestrictionAtts(), false);
    s->SetViewAxisArrayAttributes(ViewerWindowManager::Instance()->GetViewAxisArrayClientAtts(), false);
    s->SetViewCurveAttributes(ViewerWindowManager::Instance()->GetViewCurveClientAtts(), false);
    s->SetView2DAttributes(ViewerWindowManager::Instance()->GetView2DClientAtts(), false);
    s->SetView3DAttributes(ViewerWindowManager::Instance()->GetView3DClientAtts(), false);
    s->SetLightList(ViewerWindowManager::Instance()->GetLightListClientAtts(), false);
    s->SetAnimationAttributes(ViewerWindowManager::Instance()->GetAnimationClientAtts(), false);
    s->SetPickAttributes(ViewerQueryManager::Instance()->GetPickClientAtts(), false);
    s->SetPrinterAttributes(ViewerWindowManager::Instance()->GetPrinterClientAtts(), false);
    s->SetWindowInformation(ViewerWindowManager::Instance()->GetWindowInformation(), false);
    s->SetRenderingAttributes(ViewerWindowManager::Instance()->GetRenderingAttributes(), false);
    s->SetKeyframeAttributes(ViewerWindowManager::Instance()->GetKeyframeClientAtts(), false);
    s->SetQueryList(ViewerQueryManager::Instance()->GetQueryTypes(), false);
    s->SetQueryAttributes(ViewerQueryManager::Instance()->GetQueryClientAtts(), false);
    s->SetMaterialAttributes(ViewerEngineManager::GetMaterialClientAtts(), false);
    s->SetGlobalLineoutAttributes(ViewerQueryManager::Instance()->GetGlobalLineoutClientAtts(), false);
    s->SetAnnotationObjectList(ViewerWindowManager::GetAnnotationObjectList(), false);
    s->SetQueryOverTimeAttributes(ViewerQueryManager::Instance()->GetQueryOverTimeClientAtts(), false);
    s->SetInteractorAttributes(ViewerWindowManager::Instance()->GetInteractorClientAtts(), false);
    s->SetMeshManagementAttributes(ViewerEngineManager::GetMeshManagementClientAtts(), false);
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
//   Brad Whitlock, Mon May 2 11:41:32 PDT 2005
//   I made it use the ViewerState object to connect the xfer object. I also
//   made parent be a pointer.
//
//   Brad Whitlock, Thu Aug 14 13:06:42 PDT 2008
//   Only set up xfer's input/output connections if there is a parent.
//
// ****************************************************************************

void
ViewerSubject::ConnectXfer()
{
    //
    // Set up xfer's connections so it can send/receive the RPCs.
    //
    if(parent != 0)
    {
        xfer.SetInputConnection(parent->GetWriteConnection());
        xfer.SetOutputConnection(parent->GetReadConnection());
    }

    //
    // Set up all of the objects that have been added to viewerState so far.
    //
    for(int i = 0; i < GetViewerState()->GetNumStateObjects(); ++i)
        xfer.Add(GetViewerState()->GetStateObject(i));

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
//   Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//   Added an engine key used to index (and restart) engines.
//
//   Brad Whitlock, Fri Apr 15 11:13:18 PDT 2005
//   Added a new observer to handle postponed actions.
//
//   Brad Whitlock, Thu May 5 19:22:00 PST 2005
//   Added new observers for client information.
//
//   Brad Whitlock, Mon Feb 12 11:06:42 PDT 2007
//   Made it use ViewerState and renamed a class for translating 
//   Subject/Observer into Qt signals. Added color table observer.
//
//   Jeremy Meredith, Wed Apr 30 12:23:04 EDT 2008
//   Added the apparent-host setting to parent process.  Since we
//   try to use 127.0.0.1 whenever possible, this is the way to
//   get the externally visible hostname.
//
//   Brad Whitlock, Fri May  9 14:51:37 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerSubject::ConnectObjectsAndHandlers()
{
    //
    // Create a QSocketNotifier that tells us to call ReadFromParentAndProcess.
    //
    if(parent != 0 && parent->GetWriteConnection() != 0)
    {
        if(parent->GetWriteConnection()->GetDescriptor() != -1)
        {
            checkParent = new QSocketNotifier(
                parent->GetWriteConnection()->GetDescriptor(),
                QSocketNotifier::Read, this);
            connect(checkParent, SIGNAL(activated(int)),
                    this, SLOT(ReadFromParentAndProcess(int)));
        }
    }

    //
    // Create an observer for the viewerRPC object. The RPC's are actually
    // handled by the ViewerSubject by a slot function.
    //
    viewerRPCObserver = new ViewerObserverToSignal(GetViewerState()->GetViewerRPC());
    connect(viewerRPCObserver, SIGNAL(execute()),
            this, SLOT(HandleViewerRPC()));

    //
    // Create an observer for the postponedAction object. The actions are
    // actually handled by the ViewerSubject by a slot function.
    //
    postponedActionObserver = new ViewerObserverToSignal(GetViewerState()->GetPostponedAction());
    connect(postponedActionObserver, SIGNAL(execute()),
            this, SLOT(HandlePostponedAction()));

    //
    // Create an observer for the syncAtts object. Each time the object
    // updates, send the attributes back to the client.
    //
    syncObserver = new ViewerObserverToSignal(GetViewerState()->GetSyncAttributes());
    connect(syncObserver, SIGNAL(execute()),
            this, SLOT(HandleSync()));

    //
    // Create an observer for the clientMethod object. Each time the object
    // updates, send it back to the client.
    //
    clientMethodObserver = new ViewerObserverToSignal(GetViewerState()->GetClientMethod());
    connect(clientMethodObserver, SIGNAL(execute()),
            this, SLOT(HandleClientMethod()));

    //
    // Create an observer for the clientInformation object. Each time the
    // object updates, add it to the clientInformationList and send it back
    // to the client.
    //
    clientInformationObserver = new ViewerObserverToSignal(GetViewerState()->GetClientInformation());
    connect(clientInformationObserver, SIGNAL(execute()),
            this, SLOT(HandleClientInformation()));

    //
    // Create an observer for color table attributes so we can update the color
    // table buttons that we use in the viewer.
    //
    colorTableObserver = new ViewerObserverToSignal(GetViewerState()->GetColorTableAttributes());
    connect(colorTableObserver, SIGNAL(execute()),
            this, SLOT(HandleColorTable()));

    //
    // Create a timer that activates every 5 minutes to send a keep alive
    // signal to all of the remote processes. This will keep their connections
    // alive.
    //
    keepAliveTimer = new QTimer(this);
    connect(keepAliveTimer, SIGNAL(timeout()),
            this, SLOT(SendKeepAlives()));
    keepAliveTimer->start(5 * 60 * 1000);

    //
    // Register a callback function to be called when launching a remote
    // process requires authentication.
    //
#if !defined(_WIN32)
    if (!GetViewerProperties()->GetNowin())
    {
        RemoteProcess::SetAuthenticationCallback(&ViewerPasswordWindow::authenticate);
    }
#endif

    if(parent != 0)
    {
        //
        // Get the localhost name from the parent and give it to the
        // ViewerServerManager and EngineKey so it can use it when needed.
        //
        ViewerServerManager::SetLocalHost(parent->GetApparentHostName());
        EngineKey::SetLocalHost(parent->GetApparentHostName());

        //
        // Set the default user name.
        //
        HostProfile::SetDefaultUserName(parent->GetTheUserName());
    }
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
//    Kathleen Bonnell, Wed Aug 18 09:25:33 PDT 2004 
//    Added ViewerWindowManger's InteractorAtts to config manager.
//
//    Brad Whitlock, Wed Jun 22 10:23:00 PDT 2005
//    Added movieAtts.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added MeshManagementAttributes
//
//    Brad Whitlock, Fri Feb 23 11:33:26 PDT 2007
//    Made it use ViewerState when only client attributes are needed.
//
//    Jeremy Meredith, Wed Jan 23 16:30:24 EST 2008
//    Added file open options.
//
// ****************************************************************************

void
ViewerSubject::ConnectConfigManager()
{
    //
    // Connect objects that can be written to the config file. Note that
    // we don't hook up some fancy mechanism for viewer state because
    // viewer state contains all of the client attributes and some objects
    // have client attributes and default attributes. We want to hook up
    // default attributes if they are available.
    //
    configMgr->Add(GetViewerState()->GetGlobalAttributes());
    configMgr->Add(GetViewerState()->GetHostProfileList());
    configMgr->Add(GetViewerState()->GetSaveWindowAttributes());
    configMgr->Add(GetViewerState()->GetColorTableAttributes());
    configMgr->Add(GetViewerState()->GetExpressionList());
    configMgr->Add(GetViewerState()->GetAnimationAttributes());
    configMgr->Add(ViewerWindowManager::GetAnnotationDefaultAtts());
    configMgr->Add(GetViewerState()->GetViewCurveAttributes());
    configMgr->Add(GetViewerState()->GetView2DAttributes());
    configMgr->Add(GetViewerState()->GetView3DAttributes());
    configMgr->Add(ViewerWindowManager::GetLightListDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetWindowAtts());
    configMgr->Add(GetViewerState()->GetWindowInformation());
    configMgr->Add(GetViewerState()->GetPrinterAttributes());
    configMgr->Add(GetViewerState()->GetRenderingAttributes());
    configMgr->Add(ViewerEngineManager::GetMaterialDefaultAtts());
    configMgr->Add(ViewerEngineManager::GetMeshManagementDefaultAtts());
    configMgr->Add(ViewerWindowManager::GetDefaultAnnotationObjectList());
    configMgr->Add(ViewerQueryManager::Instance()->GetPickDefaultAtts());
    configMgr->Add(ViewerQueryManager::Instance()->GetQueryOverTimeDefaultAtts());
    configMgr->Add(ViewerWindowManager::Instance()->GetInteractorDefaultAtts());
    configMgr->Add(GetViewerState()->GetMovieAttributes());
    configMgr->Add(GetViewerState()->GetFileOpenOptions());
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
//   Brad Whitlock, Tue Feb 13 13:55:42 PST 2007
//   Made it use ViewerState.
//
// ****************************************************************************

void
ViewerSubject::InformClientOfPlugins() const
{
    GetViewerState()->GetPluginManagerAttributes()->Notify();
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
//   Brad Whitlock, Mon Aug 2 15:39:05 PST 2004
//   I added code to disable reading from the client while we're executing
//   in here and until we are done processing any events generated by the
//   code executed in this method.
//
//   Jeremy Meredith, Tue Feb  8 08:57:07 PST 2005
//   Added detection of plot and operator plugin errors found during
//   plugin initialization.
//
//   Brad Whitlock, Mon May 2 14:23:17 PST 2005
//   Added code to turn the parent object and the checkParent objects into
//   a ViewerClientConnection object now that the viewer is initialized.
//
//   Brad Whitlock, Wed Aug 20 15:46:47 PDT 2008
//   Added code to set up ViewerMethods so it writes into a buffered viewer
//   state that gets copied into the central xfer's input buffer.
//
//   Brad Whitlock, Thu Apr  9 15:59:56 PDT 2009
//   I added code to open a file on startup.
//
//   Brad Whitlock, Tue Apr 14 12:00:09 PDT 2009
//   Use ViewerProperties.
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
        if (GetViewerProperties()->GetDefaultStereoToOn())
            ViewerWindowManager::GetRenderingAttributes()->SetStereoRendering(true);

        //
        // Add the initial windows.
        //
        AddInitialWindows();

        //
        // Launch an engine if certain command line flags were given.
        //
        LaunchEngineOnStartup();

        //
        // Get plot and operator plugin errors found during initialization
        // and report them through the client (gui/cli).  Database plugin
        // errors are found when starting a new mdserver or opening a file.
        //
        string ep, eo;
        ep = GetPlotPluginManager()->GetPluginInitializationErrors();
        eo = GetOperatorPluginManager()->GetPluginInitializationErrors();
        string error = ep + eo;
        if (!error.empty())
        {
            Warning(error.c_str());
        }

        //
        // Now that everything's been fully initialized, donate the
        // ParentProcess object to the clients vector so we can keep track of the
        // connection to the main client as we will for new clients that will
        // be created later. Note that we silence xfer by turning off its
        // output and we set an update callback function so that instead of
        // just serializing the subject to a buffer, we broadcast it to each
        // client using our BroadcastToAllClients callback function.
        //
        ViewerClientConnection *client = new ViewerClientConnection(parent,
            checkParent, GetViewerState(), this, "connection0");
        client->SetupSpecialOpcodeHandler(SpecialOpcodeCallback, (void *)this);
        parent = 0;
        inputConnection = new BufferConnection;
        xfer.SetInputConnection(inputConnection);
        xfer.SetOutputConnection(0);
        xfer.SetUpdateCallback(BroadcastToAllClients, (void *)this);
        if(checkParent != 0)
        {
            disconnect(checkParent, SIGNAL(activated(int)),
                       this, SLOT(ReadFromParentAndProcess(int)));
        }
        connect(client, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
                this,   SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
        connect(client, SIGNAL(DisconnectClient(ViewerClientConnection *)),
                this,   SLOT(DisconnectClient(ViewerClientConnection *)));
        clients.push_back(client);

        // Hook up the viewer delayed state. If we're currently reading from the parent,
        // we don't want to hook it up since it adds observers to the subjects in
        // the global viewer state, which could currently be in a Notify() if 
        // processingFromParent is true.
        if(processingFromParent)
            QTimer::singleShot(100, this, SLOT(CreateViewerDelayedState()));
        else
            CreateViewerDelayedState();

        // Discover the client's information.
        QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));

        // Open a database on startup.
        QTimer::singleShot(100, this, SLOT(OpenDatabaseOnStartup()));

        // Open a script on startup.
        QTimer::singleShot(100, this, SLOT(OpenScriptOnStartup()));

        heavyInitializationDone = true;
        visitTimer->StopTimer(timeid, "Heavy initialization.");
    }
}

// ****************************************************************************
// Method: ViewerSubject::CreateViewerDelayedState
//
// Purpose: 
//   This method sets up the delayed viewer state, which lets us execute
//   viewer methods later in the central xfer queue. Methods executed in this
//   fashion won't be executed during engine RPC's or at other bad times.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 11:48:31 PDT 2008
//
// Modifications:
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//   
// ****************************************************************************

void
ViewerSubject::CreateViewerDelayedState()
{
    TRY
    {
        // Create an internal ViewerState that we'll use for the ViewerMethods 
        // object. We use a buffered version so all of the input from the viewer
        // methods and state will be buffered into the central input for the
        // xfer object. This should cause all commands to be buffered until they
        // can be safely executed. For example, this prevents us from executing
        // commands via ViewerMethods while the engine is executing.
        viewerDelayedState = new ViewerStateBuffered(GetViewerState());
        connect(viewerDelayedState, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
                this,                SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
        // Override the base class's ViewerMethods object, if it exists, with one that
        // uses the buffered state.
        viewerDelayedMethods = new ViewerMethods(viewerDelayedState->GetState());
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::GetViewerDelayedState
//
// Purpose: 
//   Returns the buffered viewer state if it has been created.
//
// Arguments:
//
// Returns:    The buffered viewer state otherwise the regular unbuffered
//             viewer state.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 20 16:06:25 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

ViewerState *
ViewerSubject::GetViewerDelayedState()
{
    return (viewerDelayedState != 0) ? viewerDelayedState->GetState() : GetViewerState();
}

ViewerMethods *
ViewerSubject::GetViewerDelayedMethods()
{
    return viewerDelayedMethods;
}

// ****************************************************************************
// Method: ViewerSubject::AddInputToXfer
//
// Purpose: 
//   All ViewerClientConnection objects emit a signal that is connected to
//   this method, which adds the client's input to the single xfer that we
//   actually use to schedule execution of RPC's.
//
// Arguments:
//   subj : The AttributeSubject that needs to be processed.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 2 13:51:34 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 11:59:15 PDT 2007
//   Made it use ViewerState.
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::AddInputToXfer(ViewerClientConnection *client,
    AttributeSubject *subj)
{
    TRY
    {
        // Write the state object into the buffered input so we can process it
        // later.
        Connection *input = xfer.GetBufferedInputConnection();
        input->WriteInt(subj->GetGuido());
        int sz = subj->CalculateMessageSize(*input);
        input->WriteInt(sz);
        subj->Write(*input);

        // In the meantime, to prevent problems with state inconsistency between
        // clients, send the state object to all but the client that sent the
        // state object if the state object is one that we can freely send. Note
        // that we don't send ViewerRPC, postponedAction, syncAtts, messageAtts,
        // statusAtts, metaData, silAtts.
        if(subj->GetGuido() >= GetViewerState()->FreelyExchangedState())
        {
            for(int i = 0; i < clients.size(); ++i)
            {
                if(clients[i] != client)
                    clients[i]->BroadcastToClient(subj);
            }
        }

        // Schedule the input to be processed by the main event loop.
        QTimer::singleShot(10, this, SLOT(ProcessFromParent()));
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::DisconnectClient
//
// Purpose: 
//   This is a Qt slot function that is called when a client connection is lost.
//
// Arguments:
//   client : The client connection that we lost.
//
// Programmer: Brad Whitlock
// Creation:   Mon May 2 16:39:12 PST 2005
//
// Modifications:
//   Brad Whitlock, Fri Jan 9 14:03:58 PST 2009
//   I added code to catch VisItException since we should not allow it to
//   escape from a Qt slot.
//   
// ****************************************************************************

void
ViewerSubject::DisconnectClient(ViewerClientConnection *client)
{
    TRY
    {
        ViewerClientConnectionVector::iterator pos = std::find(clients.begin(),
            clients.end(), client);
        if(pos != clients.end())
            clients.erase(pos);

        disconnect(client, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
                   this, SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
        disconnect(client, SIGNAL(DisconnectClient(ViewerClientConnection *)),
                   this, SLOT(DisconnectClient(ViewerClientConnection *)));

        debug1 << "VisIt's viewer lost a connection to one of its clients ("
               << client->Name().toStdString() << ")." << endl;
        client->deleteLater();

        // If we ever get down to no client connections, quit.
        if(clients.size() < 1)
        {
            Close();
        }
        else
        {
            // We have at least one client so we should discover the client's
            // information.
            QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
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
//    Brad Whitlock, Mon Feb 12 12:03:10 PDT 2007
//    I made it use ViewerState.
//
//    Brad Whitlock, Tue Jun 24 14:46:18 PDT 2008
//    I changed how the plugin managers are accessed.
//
// ****************************************************************************

void
ViewerSubject::InitializePluginManagers()
{
    //
    // Load the plugin info.
    //
    int timeid = visitTimer->StartTimer();
    GetPlotPluginManager()->Initialize(PlotPluginManager::Viewer);
    GetOperatorPluginManager()->Initialize(OperatorPluginManager::Viewer);
    visitTimer->StopTimer(timeid, "Loading plugin info.");

    PlotPluginManager     *pmgr = GetPlotPluginManager();
    OperatorPluginManager *omgr = GetOperatorPluginManager();
    PluginManagerAttributes *pluginAtts = GetViewerState()->GetPluginManagerAttributes();

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
//   Brad Whitlock, Mon May 2 11:47:10 PDT 2005
//   I added code to add the plot attributes to the viewerState.
//
//   Brad Whitlock, Mon Feb 12 12:04:49 PDT 2007
//   Changed how plots are registered with ViewerState.
//
//   Brad Whitlock, Thu Jan  8 10:18:16 PST 2009
//   I made sure that the plot info attrbutes also get registered with xfer.
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
        GetPlotPluginManager()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the plot plugins. "
             << "The error message is: \"" << e.Message().c_str() << "\"" << endl;
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
        {
            GetViewerState()->RegisterPlotAttributes(attr);

            xfer.Add(GetViewerState()->GetPlotAttributes(i));
            xfer.Add(GetViewerState()->GetPlotInformation(i));
        }

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
//   Brad Whitlock, Mon May 2 11:47:10 PDT 2005
//   I added code to add the plot attributes to the viewerState.
//
//   Brad Whitlock, Mon Feb 12 12:04:49 PDT 2007
//   Changed how operators are registered with ViewerState.
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
        GetOperatorPluginManager()->LoadPluginsNow();
    }
    CATCH2(VisItException, e)
    {
        // Just print out an error message to the console because we cannot
        // abort without hanging the viewer's client.
        cerr << "VisIt could not read all of the operator plugins. "
             << "The error message is: \"" << e.Message().c_str() << "\"" << endl;
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
        {
            xfer.Add(attr);
            GetViewerState()->RegisterOperatorAttributes(attr);
        }

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
//   Kathleen Bonnell, Wed Aug 18 09:25:33 PDT 2004 
//   Added InteractorAtts.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes
//
//   Jeremy Meredith, Wed Jan 23 16:31:06 EST 2008
//   We might start an mdserver before reading the config files, so
//   make sure we send default file opening options from the config file
//   to all existing mdservers.
//
//   Brad Whitlock, Tue Apr 14 12:02:10 PDT 2009
//   Use ViewerProperties.
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
    if(!GetViewerProperties()->GetNoConfig())
        avtColorTables::Instance()->ImportColorTables();

    // Send the user's config settings to the client.
    configMgr->Notify();

    delete systemSettings; systemSettings = 0;

    // Let other viewer objects set their properties from the local settings.
    if(localSettings)
        QTimer::singleShot(300, this, SLOT(DelayedProcessSettings()));

    // Add the appearanceAtts *after* the config settings have been read. This
    // prevents overwriting the attributes and sending them to the client.
    configMgr->Add(GetViewerState()->GetAppearanceAttributes());

    // Add the pluginAtts *after* the config settings have been read.
    // First, tell the client which plugins we've really loaded.
    configMgr->Add(GetViewerState()->GetPluginManagerAttributes());

    // Copy the default annotation attributes into the client annotation
    // attributes.
    ViewerWindowManager::SetClientAnnotationAttsFromDefault();

    // Copy the default material atts to the client material atts
    ViewerEngineManager::SetClientMaterialAttsFromDefault();

    // Copy the default pick atts to the client pick atts
    ViewerQueryManager::Instance()->SetClientPickAttsFromDefault();

    // Copy the default time query atts to the client time query atts
    ViewerQueryManager::Instance()->SetClientQueryOverTimeAttsFromDefault();

    // Copy the default time query atts to the client time query atts
    ViewerWindowManager::Instance()->SetClientInteractorAttsFromDefault();

    // Send the queries to the client.
    ViewerQueryManager::Instance()->GetQueryTypes()->Notify();

    // Copy the default mesh management atts to the client material atts
    ViewerEngineManager::SetClientMeshManagementAttsFromDefault();

    // If we started an mdserver before the default file open options were
    // obtained from the config file, we need to re-notify mdservers.
    SetDefaultFileOpenOptions();

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
//   Brad Whitlock, Fri Jan 9 14:22:56 PST 2009
//   I removed DisconnectWindow.
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
//    Brad Whitlock, Thu Apr  9 14:39:12 PDT 2009
//    I added support for reverse launching.
//
//    Brad Whitlock, Tue Apr 14 12:03:32 PDT 2009
//    Use ViewerProperties.
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
        if(launchEngineAtStartup.substr(0,6) == "-host=")
        {
            stringVector tokens = SplitValues(launchEngineAtStartup, ',');
            stringVector args;
            args.push_back("visit");
            for(size_t i = 0; i < tokens.size(); ++i)
            {
                stringVector comps = SplitValues(tokens[i], '=');
                if(comps.size() == 1)
                    args.push_back(comps[0]);
                else if(comps.size() == 2)
                {
                    args.push_back(comps[0]);
                    args.push_back(comps[1]);
                }
            }

            ViewerEngineManager::Instance()->CreateEngine(
                EngineKey("localhost",""), // The name of the engine (host)
                args,               // The engine arguments
                true,               // Whether to skip the engine chooser
                GetViewerProperties()->GetNumEngineRestarts(), // Number of allowed restarts
                true);              // Whether we're reverse launching

            ViewerWindowManager::Instance()->ShowAllWindows();
        }
        else
        {
            ViewerEngineManager::Instance()->CreateEngine(
                EngineKey(launchEngineAtStartup,""), // The name of the engine (host)
                engineParallelArguments,             // The engine arguments
                true,                                // Whether to skip the engine chooser
                GetViewerProperties()->GetNumEngineRestarts(), // Number of allowed restarts
                false);                              // Whether we're reverse launching
        }
    }
}

// ****************************************************************************
// Method: ViewerSubject::OpenDatabaseOnStartup
//
// Purpose: 
//   This method opens a file on startup if there is a file to open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 16:05:43 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::OpenDatabaseOnStartup()
{
    if(openDatabaseOnStartup != "")
    {
        OpenDatabaseHelper(openDatabaseOnStartup, // DB name
           0,    // timeState, 
           true, // addDefaultPlots
           true, // updateWindowInfo,
           "");  // forcedFileType

        ViewerWindowManager::Instance()->UpdateActions();
        ViewerWindowManager::Instance()->ShowAllWindows();
    }
}

// ****************************************************************************
// Method: ViewerSubject::OpenScriptOnStartup
//
// Purpose: 
//   This method opens a script on startup if there is a script to open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr  9 16:05:43 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::OpenScriptOnStartup()
{
    if(openScriptOnStartup != "")
    {
        std::string cmd("Source('");
        cmd += openScriptOnStartup;
        cmd += "')";
        InterpretCommands(cmd);
        ViewerWindowManager::Instance()->ShowAllWindows();
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
//   Brad Whitlock, Tue Mar 7 12:16:39 PDT 2006
//   I changed this method so that it no longer calls SetFromNode since that
//   method now contains some code to validate a session file before allowing
//   it to be imported. Since settings are incomplete session files at best,
//   the validation code was indicating the session file had errors and VisIt
//   did not allow the objects to initialize themselves. We call them here
//   directly so they can initialize themselves without the session file
//   validation code.
//
//   Brad Whitlock, Fri Nov 10 10:56:00 PDT 2006
//   Added arguments to some SetFromNode methods.
//
//   Cyrus Harrison, Fri Mar 16 09:21:24 PDT 2007
//   Added call to SetFromNode from the ViewerEngineManager
//
//   Brad Whitlock, Wed Feb 13 14:06:56 PST 2008
//   Added configVersion to the calls to SetFromNode.
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::DelayedProcessSettings()
{
    const char *mName = "ViewerSubject::DelayedProcessSettings: ";
    TRY
    {
        if(localSettings != 0)
        {
            // Get the VisIt node.
            DataNode *visitRoot = localSettings->GetNode("VisIt");
            if(visitRoot == 0)
            {
                debug1 << mName << "Can't read VisIt node." << endl;
                return;
            }

            // Get the viewer node.
            DataNode *viewerNode = visitRoot->GetNode("VIEWER");
            if(viewerNode == 0)
            {
                debug1 << mName << "Can't read VisIt node." << endl;
                return;
            }

            // Get the ViewerSubject node
            DataNode *searchNode = viewerNode->GetNode("ViewerSubject");
            if(searchNode == 0)
            {
                debug1 << mName << "Can't read ViewerSubject node." << endl;
                return;
            }

            // Get the version
            std::string configVersion(VERSION);
            DataNode *version = visitRoot->GetNode("Version");
            if(version != 0)
                configVersion = version->AsString();

            // Let the important objects read their settings.
            std::map<std::string, std::string> empty;
            ViewerFileServer::Instance()->SetFromNode(searchNode, empty, configVersion);
            ViewerWindowManager::Instance()->SetFromNode(searchNode, empty, configVersion);
            ViewerQueryManager::Instance()->SetFromNode(searchNode, configVersion);
            ViewerEngineManager::Instance()->SetFromNode(searchNode, configVersion);

            delete localSettings;  localSettings = 0;
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
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
//   Brad Whitlock, Fri May  9 14:47:32 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Aug 14 09:56:41 PDT 2008
//   Use qApp.
//
//   Brad Whitlock, Tue Apr 28 19:14:23 PST 2009
//   I disabled the simulation socket notifiers since we don't want them to
//   be enabled while processing events since this method is a callback for
//   when we're reading from the simulation. If the socket notifiers are
//   allowed to operate then it disrupts the synchronous send/recv's that
//   we're using to communicate with the simulation. This caused the simulation
//   connection to disconnect on Windows.
//
// ****************************************************************************

void
ViewerSubject::ProcessEvents()
{
    std::map<EngineKey,QSocketNotifier*>::iterator it;
    for(it = engineKeyToNotifier.begin(); it != engineKeyToNotifier.end(); ++it)
        it->second->setEnabled(false);

    if (interruptionEnabled)
    {
        qApp->processEvents(QEventLoop::AllEvents, 100);
    }

    for(it = engineKeyToNotifier.begin(); it != engineKeyToNotifier.end(); ++it)
        it->second->setEnabled(true);
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
//    Brad Whitlock, Wed Jan 11 17:38:13 PST 2006
//    I passed a flag to the window metrics.
//
//    Brad Whitlock, Fri Aug 22 14:33:24 PST 2008
//    I made it honor the useWindowMetrics setting in the event that
//    none of the geometry or other flags were set.
//
//    Brad Whitlock, Tue Apr 14 12:04:03 PDT 2009
//    Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::InitializeWorkArea()
{
    char           tmp[50];
    int            x, y, w, h;

    if (GetViewerProperties()->GetNowin())
    {
        if (GetViewerProperties()->GetWindowBorders().size() == 0)
        {
            GetViewerProperties()->SetWindowBorders("0,0,0,0");
        }
        if (GetViewerProperties()->GetWindowShift().size() == 0)
        {
            GetViewerProperties()->SetWindowShift("0,0");
        }
        if (GetViewerProperties()->GetWindowPreShift().size() == 0)
        {
            GetViewerProperties()->SetWindowPreShift("0,0");
        }
        if (GetViewerProperties()->GetWindowGeometry().size() == 0)
        {
            if (GetViewerProperties()->GetWindowSmall())
                GetViewerProperties()->SetWindowGeometry("512x512");
            else
                GetViewerProperties()->SetWindowGeometry("1024x1024");
        }
    }
    else
    {
        //
        // If any of the options are missing then use the WindowMetrics
        // class to fill in the blanks.
        //
        int wmBorder[4], wmShift[2], wmScreen[4];
        if(GetViewerProperties()->GetWindowBorders().size() == 0 ||
           GetViewerProperties()->GetWindowShift().size() == 0 ||
           GetViewerProperties()->GetWindowPreShift().size() == 0 || 
           GetViewerProperties()->GetWindowGeometry().size() == 0)
        {
            if(GetViewerProperties()->GetUseWindowMetrics())
            {
                WindowMetrics *wm = WindowMetrics::Instance();
                wm->MeasureScreen(true);

                wmBorder[0] = wm->GetBorderT();
                wmBorder[1] = wm->GetBorderB();
                wmBorder[2] = wm->GetBorderL();
                wmBorder[3] = wm->GetBorderR();

                wmShift[0] = wm->GetShiftX();
                wmShift[1] = wm->GetShiftY();

                wmScreen[0] = wm->GetScreenW();
                wmScreen[1] = wm->GetScreenH();
                wmScreen[2] = wm->GetScreenX();
                wmScreen[3] = wm->GetScreenY();
                delete wm;
            }
            else
            {
                // May want platform specific coding here.
                wmBorder[0] = 22;
                wmBorder[1] = 0;
                wmBorder[2] = 0;
                wmBorder[3] = 0;

                wmShift[0] = 0;
                wmShift[1] = 22;

                QRect geom = qApp->desktop()->screenGeometry();
                wmScreen[0] = geom.width();
                wmScreen[1] = geom.height();
                wmScreen[2] = geom.x();
                wmScreen[3] = geom.y();
            }
        }

        //
        // Use WindowMetrics to determine the borders.
        //
        if(GetViewerProperties()->GetWindowBorders().size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d,%d,%d",
                     wmBorder[0], wmBorder[1], wmBorder[2], wmBorder[3]);
            GetViewerProperties()->SetWindowBorders(tmp);
        }

        //
        // Use WindowMetrics to determine the shift.
        //
        if(GetViewerProperties()->GetWindowShift().size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d", wmShift[0], wmShift[1]);
            GetViewerProperties()->SetWindowShift(tmp);
        }

        //
        // Use WindowMetrics to determine the preshift.
        //
        if(GetViewerProperties()->GetWindowPreShift().size() == 0)
        {
            SNPRINTF(tmp, 50, "%d,%d", wmShift[0], wmShift[1]);
            GetViewerProperties()->SetWindowPreShift(tmp);
        }

        //
        // Use WindowMetrics to determine the geometry.
        //
        if(GetViewerProperties()->GetWindowGeometry().size() == 0)
        {
            int h1 = int(wmScreen[0] * 0.8);
            int h2 = int(wmScreen[1] * 0.8);
            h = (h1 < h2) ? h1 : h2;
            w = h;
            x = wmScreen[2] + wmScreen[0] - w;
            y = wmScreen[3];
            if(GetViewerProperties()->GetWindowSmall())
            {
                w /= 2; h /= 2; x += w;
            }

            SNPRINTF(tmp, 50, "%dx%d+%d+%d", w, h, x, y);
            GetViewerProperties()->SetWindowGeometry(tmp);
        }
        else if(GetViewerProperties()->GetWindowSmall())
        {
            if(sscanf(GetViewerProperties()->GetWindowGeometry().c_str(),
                      "%dx%d+%d+%d", &w, &h, &x, &y) == 4)
            {
                w /= 2;
                h /= 2;
                SNPRINTF(tmp, 50, "%dx%d+%d+%d", w, h, x, y);
                GetViewerProperties()->SetWindowGeometry(tmp);
            }
        }
    }

    //
    // Set the options in the viewer window manager.
    //
    ViewerWindowManager *windowManager=ViewerWindowManager::Instance();
    windowManager->SetBorders(GetViewerProperties()->GetWindowBorders().c_str());
    windowManager->SetShift(GetViewerProperties()->GetWindowShift().c_str());
    windowManager->SetPreshift(GetViewerProperties()->GetWindowPreShift().c_str());
    windowManager->SetGeometry(GetViewerProperties()->GetWindowGeometry().c_str());
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
//   Brad Whitlock, Mon Mar 19 16:30:42 PST 2007
//   Added font changing.
//
//   Brad Whitlock, Fri May  9 14:48:31 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Aug 14 09:56:54 PDT 2008
//   Use qApp.
//
//   Brad Whitlock, Wed Nov 26 11:35:10 PDT 2008
//   Use a new function from winutil since it does a better job.
//
// ****************************************************************************

void
ViewerSubject::CustomizeAppearance()
{
    if (!GetViewerProperties()->GetNowin())
    {
        SetAppearance(qApp, GetViewerState()->GetAppearanceAttributes());
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
//    Brad Whitlock, Wed Feb 16 09:35:48 PDT 2005
//    Updated since I moved Get*ConfigFile to Utility.h instead of having
//    them be ConfigManager methods.
//
//    Brad Whitlock, Mon Feb 12 12:23:11 PDT 2007
//    I made it use ViewerState.
//
//    Hank Childs, Mon Feb 26 11:33:37 PST 2007
//    Issue a warning when a config file was not found.
//
//    Kathleen Bonnell, Tue Jul 24 15:44:37 PDT 2007 
//    Added WIN32 specific-code to handle an arg that may have spaces in it. 
//
//    Brad Whitlock, Thu Aug 14 14:44:22 PDT 2008
//    Move creation of the config manager to the constructor.
//
//    Brad Whitlock, Tue Apr 14 12:14:12 PDT 2009
//    Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::ReadConfigFiles(int argc, char **argv)
{
    int timeid = visitTimer->StartTimer();

    //
    // Look for config file, related flags.
    //
    bool specifiedConfig = false;
    for (int i = 1 ; i < argc ; i++)
    {
        if (strcmp(argv[i], "-noconfig") == 0)
        {
            GetViewerProperties()->SetNoConfig(true);
            GetViewerProperties()->SetConfigurationFileName("");
        }
        else if (strcmp(argv[i], "-config") == 0 &&
                 (i+1) < argc &&
                 !GetViewerProperties()->GetNoConfig())
        {
            specifiedConfig = true;
#ifndef WIN32
            GetViewerProperties()->SetConfigurationFileName(argv[i+1]);
#else
            string tmp = argv[i+1];
            int argcnt = 1;
            if (argv[i+1][0] == '\"' || argv[i+1][0] == '\'')
            {
                for (int j = i+2; j < argc; j++, argcnt++)
                {
                    if (tmp[tmp.length()-1] == '\"' || 
                        tmp[tmp.length()-1] == '\'')
                        break;
                    tmp += " ";
                    tmp += argv[j];
                }
            }
            GetViewerProperties()->SetConfigurationFileName(tmp);
            nConfigArgs = argcnt;
#endif
        }       
    }

    //
    // Read the config file and setup the appearance attributes. Note that
    // we call the routine to process the config file settings here because
    // it only has to update the appearance attributes.
    //
    timeid = visitTimer->StartTimer();
    char *configFile = GetSystemConfigFile();
    if (GetViewerProperties()->GetNoConfig())
        systemSettings = NULL;
    else
        systemSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    std::string configFileName(GetViewerProperties()->GetConfigurationFileName());
    const char *cfn = (configFileName != "") ? configFileName.c_str() : NULL;
    configFile = GetDefaultConfigFile(cfn);
    if (specifiedConfig && strcmp(configFile, cfn) != 0)
    {
        cerr << "\n\nYou specified a config file with the \"-config\" option,"
                " but the config file could not be located.  Note that this "
                "may be because you must fully qualify the directory of the "
                "config file.\n\n\n";
    }
    if (GetViewerProperties()->GetNoConfig())
        localSettings = NULL;
    else
        localSettings = configMgr->ReadConfigFile(configFile);
    delete [] configFile;
    configMgr->Add(GetViewerState()->GetAppearanceAttributes());
    configMgr->Add(GetViewerState()->GetPluginManagerAttributes());
    configMgr->ProcessConfigSettings(systemSettings);
    configMgr->ProcessConfigSettings(localSettings);
    configMgr->ClearSubjects();
    visitTimer->StopTimer(timeid, "Reading config files.");
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
//    Brad Whitlock, Wed May 4 11:30:29 PDT 2005
//    Added clientArguments, which get passed to any clients that the viewer
//    reverse launches.
//
//    Hank Childs, Tue Dec  6 11:52:38 PST 2005
//    Make sure to tell AVT that we are doing software rendering with -nowin.
//
//    Brad Whitlock, Wed Jan 11 17:40:16 PST 2006
//    I added support for -nowindowmetrics.
//
//    Mark C. Miller, Wed Aug  9 18:59:30 PDT 2006
//    Pass "-stereo" to engine. Allow "-timings" in addition to "-timing"
//    Changed interface to SetStereoEnabled to void args
//
//    Brad Whitlock, Wed Nov 22 09:27:10 PDT 2006
//    Added -viewer_geometry.
//
//    Jeremy Meredith, Tue Jul 17 16:39:40 EDT 2007
//    Added -fullscreen argument.
//
//    Kathleen Bonnell, Tue Jul 24 15:51:03 PDT 2007 
//    On Windows, increment using nConfigArgs when removing -config and its
//    arg.
//
//    Brad Whitlock, Thu Apr 10 09:48:44 PDT 2008
//    Added support for -locale argument.
//
//    Brad Whitlock, Wed Aug 13 10:31:38 PDT 2008
//    Moved the code to read the config files to here.
//
//    Jeremy Meredith, Wed Dec  3 16:00:10 EST 2008
//    Allow the -sshtunneling argument to be passed to the viewer as a
//    convenience.  This will even override whatever the setting is in
//    a selected host profile.
//
//    Brad Whitlock, Thu Apr  9 14:45:03 PDT 2009
//    I added -connectengine support and -o and -s  support.
//
//    Mark C. Miller, Tue Apr 21 14:24:18 PDT 2009
//    Added logic to manage buffering of debug logs; an extra 'b' after level.
// ****************************************************************************

void
ViewerSubject::ProcessCommandLine(int argc, char **argv)
{
    std::string tmpGeometry, tmpViewerGeometry;
    bool geometryProvided = false, viewerGeometryProvided = false;

    // Read the config files.
    ReadConfigFiles(argc, argv);

    //
    // Process the command line for the viewer.
    //
    for (int i = 1 ; i < argc ; i++)
    {
        if (strcmp(argv[i], "-borders") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Borders string missing for -borders option" << endl;
                continue;
            }
            GetViewerProperties()->SetWindowBorders(argv[i+1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-shift") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Shift string missing for -shift option" << endl;
                continue;
            }
            GetViewerProperties()->SetWindowShift(argv[i+1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-preshift") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Preshift string missing for -preshift option" << endl;
                continue;
            }
            GetViewerProperties()->SetWindowPreShift(argv[i+1]);
            i += 1;
        }
        else if (strcmp(argv[i], "-geometry") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Geometry string missing for -geometry option" << endl;
                continue;
            }
            tmpGeometry = argv[i+1];
            geometryProvided = true;
            i += 1;
        }
        else if (strcmp(argv[i], "-viewer_geometry") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "Geometry string missing for -viewer_geometry option" << endl;
                continue;
            }
            tmpViewerGeometry = argv[i+1];
            viewerGeometryProvided = true;
            i += 1;
        }
        else if (strcmp(argv[i], "-small") == 0)
        {
            GetViewerProperties()->SetWindowSmall(true);
        }
        else if (strcmp(argv[i], "-debug") == 0)
        {
            int debugLevel = 1; 
            bool bufferDebug = false;
            if (i+1 < argc && isdigit(*(argv[i+1])))
                debugLevel = atoi(argv[i+1]);
            else
                cerr << "Warning: debug level not specified, assuming 1" << endl;

            if (i+1 < argc && *(argv[i+1]+1) == 'b')
               bufferDebug = true;

            if (debugLevel > 0 && debugLevel < 6)
            {
                ViewerServerManager::SetDebugLevel(debugLevel, bufferDebug);

                clientArguments.push_back(argv[i]);
                clientArguments.push_back(argv[i+1]);
            }
            i++;
        }
        else if (strcmp(argv[i], "-host")     == 0 ||
                 strcmp(argv[i], "-port")     == 0 ||
                 strcmp(argv[i], "-nread")    == 0 ||
                 strcmp(argv[i], "-nwrite")   == 0 ||
                 strcmp(argv[i], "-nborders") == 0)
        {
            // this argument and the following option are dangerous to pass on
            i++;
        }
        else if (strcmp(argv[i], "-wpipe") == 0 ||
                 strcmp(argv[i], "-rpipe") == 0)
        {
            // This argument and its following options are dangerous to pass on
            i += 2;
        }
        else if (strcmp(argv[i], "-background") == 0 ||
                strcmp(argv[i], "-bg") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "The -background option must be followed by a color."
                     << endl;
                continue;
            }

            clientArguments.push_back(argv[i]);
            clientArguments.push_back(argv[i+1]);

            // Store the background color in the viewer's appearance
            // attributes so the gui will be colored properly on startup.
            GetViewerState()->GetAppearanceAttributes()->SetBackground(std::string(argv[i+1]));
            ++i;
        }
        else if(strcmp(argv[i], "-config") == 0)
        {
            // Make sure the -config flag and the filename that follows it is
            // not passed along to other components.
#ifndef WIN32
            ++i;
#else
            i+=nConfigArgs; 
#endif
        }
        else if (strcmp(argv[i], "-foreground") == 0 ||
                strcmp(argv[i], "-fg") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "The -foreground option must be followed by a color."
                     << endl;
                continue;
            }

            clientArguments.push_back(argv[i]);
            clientArguments.push_back(argv[i+1]);

            // Store the foreground color in the viewer's appearance
            // attributes so the gui will be colored properly on startup.
            GetViewerState()->GetAppearanceAttributes()->SetForeground(std::string(argv[i+1]));
            ++i;
        }
        else if (strcmp(argv[i], "-style") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "The -style option must be followed by a style name."
                     << endl;
                continue;
            }
            if (
#ifdef QT_WS_MACX
                strcmp(argv[i + 1], "macintosh") == 0 ||
#endif
#ifdef QT_WS_WIN
                strcmp(argv[i + 1], "windowsxp") == 0 ||
                strcmp(argv[i + 1], "windowsvista") == 0 ||
#endif
                strcmp(argv[i + 1], "windows") == 0 ||
                strcmp(argv[i + 1], "motif") == 0 ||
                strcmp(argv[i + 1], "cde") == 0 ||
                strcmp(argv[i + 1], "plastique") == 0 ||
                strcmp(argv[i + 1], "cleanlooks") == 0
               )
            {
                clientArguments.push_back(argv[i]);
                clientArguments.push_back(argv[i+1]);

                GetViewerState()->GetAppearanceAttributes()->SetStyle(argv[i+1]);
            }
            ++i;
        }
        else if (strcmp(argv[i], "-font") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "The -font option must be followed by a "
                        "font description." << endl;
                continue;
            }

            clientArguments.push_back(argv[i]);
            clientArguments.push_back(argv[i+1]);

            GetViewerState()->GetAppearanceAttributes()->SetFontName(argv[i + 1]);
            ++i;
        }
        else if (strcmp(argv[i], "-locale") == 0)
        {
            if (i + 1 >= argc)
            {
                cerr << "The -locale option must be followed by a "
                        "locale name." << endl;
                continue;
            }

            clientArguments.push_back(argv[i]);
            clientArguments.push_back(argv[i+1]);

            GetViewerProperties()->SetApplicationLocale(argv[i+1]);
            ++i;
        }
        else if (strcmp(argv[i], "-timing") == 0 ||
                 strcmp(argv[i], "-timings") == 0)
        {
            //
            // Enable timing and pass the option to child processes.
            //
            visitTimer->Enable();

            clientArguments.push_back(argv[i]);
            unknownArguments.push_back(argv[i]);
        }
        else if (strcmp(argv[i], "-noint") == 0)
        {
            interruptionEnabled = false;
        }
        else if (strcmp(argv[i], "-noconfig") == 0)
        {
            // do nothing; processed by an earlier parsing of the command line
        }
        else if (strcmp(argv[i], "-defer") == 0)
        {
            deferHeavyInitialization = true;
        }
        else if (strcmp(argv[i], "-nowin") == 0)
        {
            InitVTK::ForceMesa();
            RemoteProcess::DisablePTY();
            SetNowinMode(true);
        }
        else if (strcmp(argv[i], "-fullscreen") == 0)
        {
            GetViewerProperties()->SetWindowFullScreen(true);
        }
        else if (strcmp(argv[i], "-nopty") == 0)
        {
            RemoteProcess::DisablePTY();
        }
        else if (strcmp(argv[i], "-stereo") == 0)
        {
            VisWinRendering::SetStereoEnabled();
            GetViewerProperties()->SetDefaultStereoToOn(true);
            unknownArguments.push_back(argv[i]);
        }
        else if (strcmp(argv[i], "-launchengine") == 0)
        {
            if(i + 1 >= argc)
            {
                cerr << "The -launchengine option must be followed by a "
                        "host name." << endl;
                continue;
            }
            launchEngineAtStartup = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-connectengine") == 0)
        {
            if(i + 1 >= argc)
            {
                cerr << "The -connectengine option must be followed by an "
                        "argument containing the engine connection parameters. "
                        "That argument is of the form: -host=val,-key=val,-port=val" << endl;
                continue;
            }
            launchEngineAtStartup = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-o") == 0)
        {
            if(i + 1 >= argc)
            {
                cerr << "The -o option must be followed by a filename." << endl;
                continue;
            }
            openDatabaseOnStartup = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-s") == 0)
        {
            if(i + 1 >= argc)
            {
                cerr << "The -s option must be followed by a script filename." << endl;
                continue;
            }
            openScriptOnStartup = argv[i+1];
            ++i;
        }
        else if (strcmp(argv[i], "-key") == 0)
        {
            if(i + 1 >= argc)
            {
                cerr << "The -key option must be followed by a key." << endl;
                continue;
            }
            // Don't do anything with the key. Just skip over it.
            ++i;
        }
        else if (strcmp(argv[i], "-numrestarts") == 0)
        {
            if ((i + 1 >= argc) || (!isdigit(*(argv[i+1]))))
            {
                cerr << "The -numrestarts option must be followed by an "
                        "integer number." << endl;
                continue;
            }
            GetViewerProperties()->SetNumEngineRestarts(atoi(argv[++i]));
        }
        else if (strcmp(argv[i], "-engineargs") == 0)
        {
            if ((i + 1 >= argc))
            {
                cerr << "The -engineargs option must be followed by a "
                        "string." << endl;
                continue;
            }
            engineParallelArguments = SplitValues(argv[++i], ';');
        }
        else if(strcmp(argv[i], "-nowindowmetrics") == 0)
        {
            debug1 << "Handling -nowindowmetrics" << endl;
            GetViewerProperties()->SetUseWindowMetrics(false);
        }
        else if(strcmp(argv[i], "-sshtunneling") == 0)
        {
            ViewerServerManager::ForceSSHTunnelingForAllConnections();
        }
        else // Unknown argument -- add it to the list
        {
            clientArguments.push_back(argv[i]);
            unknownArguments.push_back(argv[i]);
        }
    }

    // Set the geometry based on the argument that was provided with
    // -viewer_geometry taking precedence.
    if(viewerGeometryProvided)
        GetViewerProperties()->SetWindowGeometry(tmpViewerGeometry);
    else if(geometryProvided)
        GetViewerProperties()->SetWindowGeometry(tmpGeometry);

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
//   Brad Whitlock, Tue Aug 3 15:35:07 PST 2004
//   Made the engine manager save its settings so they are available in
//   session files so visit -movie can use them.
//
//   Brad Whitlock, Thu Nov 9 16:13:05 PST 2006
//   I added code to create a SourceMap node in the saved data so we
//   can reference it from other parts of the session to make it easier to
//   change databases.
//
//   Brad Whitlock, Tue Mar 27 11:18:36 PDT 2007
//   Made GetDatabasesForWindows get all databases not just MT ones.
//
// ****************************************************************************

void
ViewerSubject::CreateNode(DataNode *parentNode, bool detailed)
{
    if(parentNode == 0)
        return;

    DataNode *vsNode = new DataNode("ViewerSubject");
    parentNode->AddNode(vsNode);

    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    stringVector databases;
    intVector    wIds;
    // Get the ids of the windows that currently exist.
    int i, nWin, *windowIndices;
    windowIndices = wM->GetWindowIndices(&nWin);
    for(i = 0; i < nWin; ++i)
        wIds.push_back(windowIndices[i]);
    delete [] windowIndices;

    // Get all of the databases that are open in the specified windows
    // (even the ST databases).
    wM->GetDatabasesForWindows(wIds, databases, true);

    // Create a map of source ids to source names and also store
    // that information into the session.
    char keyName[100];
    std::map<std::string, std::string> dbToSource;
    DataNode *sourceMapNode = new DataNode("SourceMap");
    for(i = 0; i < databases.size(); ++i)
    {
        SNPRINTF(keyName, 100, "SOURCE%02d", i);
        std::string key(keyName);
        dbToSource[databases[i]] = key;
        sourceMapNode->AddNode(new DataNode(key, databases[i]));
    }
    vsNode->AddNode(sourceMapNode);

    ViewerFileServer::Instance()->CreateNode(vsNode, dbToSource, detailed);
    wM->CreateNode(vsNode, dbToSource, detailed);
    if(detailed)
        ViewerQueryManager::Instance()->CreateNode(vsNode);
    if(detailed)
        ViewerEngineManager::Instance()->CreateNode(vsNode);
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
// Notes:
//   This method is only called when VisIt is reading session files.
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
//   Brad Whitlock, Wed Jan 11 14:44:13 PST 2006
//   I added some error checking to the session file processing.
//
//   Brad Whitlock, Thu Nov 9 17:14:12 PST 2006
//   I added support for SourceMap, which other objects use to get the
//   names of the databases that are used in the visualization.
//
//   Cyrus Harrison, Fri Mar 16 09:02:17 PDT 2007
//   Added support for ViewerEngineManager to restore its settings
//   from a node. 
//
//   Brad Whitlock, Wed Feb 13 14:08:18 PST 2008
//   Added configVersion argument.
//
// ****************************************************************************

bool
ViewerSubject::SetFromNode(DataNode *parentNode, 
    const std::map<std::string,std::string> &sourceToDB, 
    const std::string &configVersion)
{
    bool fatalError = true;

    if(parentNode == 0)
        return fatalError;

    DataNode *vsNode = parentNode->GetNode("ViewerSubject");
    if(vsNode == 0)
        return fatalError;

    // See if there are any obvious errors in the session file.
    fatalError = ViewerWindowManager::Instance()->SessionContainsErrors(vsNode);
    if(!fatalError)
    {
        ViewerFileServer::Instance()->SetFromNode(vsNode, sourceToDB, configVersion);
        ViewerWindowManager::Instance()->SetFromNode(vsNode, sourceToDB, configVersion);
        ViewerQueryManager::Instance()->SetFromNode(vsNode, configVersion);
        ViewerEngineManager::Instance()->SetFromNode(vsNode, configVersion);
    }

    return fatalError;
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
//    Brad Whitlock, Mon May 9 08:32:20 PDT 2005
//    I made it tell all clients to quit.
//
//    Brad Whitlock, Thu Jan 25 18:06:08 PST 2007
//    Disconnect socket notifiers so simulations can't send any more commands.
//
//    Brad Whitlock, Thu Aug 14 09:57:29 PDT 2008
//    Use qApp.
//
// ****************************************************************************

void
ViewerSubject::Close()
{
    //
    // Don't accept any more input from simulations.
    //
    std::map<EngineKey,QSocketNotifier*>::iterator it;
    for(it = engineKeyToNotifier.begin(); it != engineKeyToNotifier.end(); ++it)
    {
        disconnect(it->second, SIGNAL(activated(int)),
                   this, SLOT(ReadFromSimulationAndProcess(int)));
    }

    //
    // Perform the rpc.
    //
    debug1 << "Starting to close the viewer." << endl;
    ViewerWindowManager::Instance()->HideAllWindows();
    ViewerFileServer::Instance()->CloseServers();
    ViewerEngineManager::Instance()->CloseEngines();

    //
    // Tell all of the clients to quit.
    //
    GetViewerState()->GetClientMethod()->SetMethodName("Quit");
    GetViewerState()->GetClientMethod()->ClearArgs();
    BroadcastToAllClients((void *)this, GetViewerState()->GetClientMethod());

    //
    // Break out of the application loop.
    //
    qApp->exit(0);
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
    int from = GetViewerState()->GetViewerRPC()->GetWindowLayout();
    int to = GetViewerState()->GetViewerRPC()->GetWindowId();
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
    int from = GetViewerState()->GetViewerRPC()->GetWindowLayout();
    int to = GetViewerState()->GetViewerRPC()->GetWindowId();
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
    int from = GetViewerState()->GetViewerRPC()->GetWindowLayout();
    int to = GetViewerState()->GetViewerRPC()->GetWindowId();
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
    int from = GetViewerState()->GetViewerRPC()->GetWindowLayout();
    int to = GetViewerState()->GetViewerRPC()->GetWindowId();
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
//   Brad Whitlock, Thu Jul 29 16:43:31 PST 2004
//   I added a call to HeavyInitialization so the viewer gets initialized
//   when called with the -defer argument by clients that should not use it
//   (cli, java).
//
// ****************************************************************************
void
ViewerSubject::ShowAllWindows()
{
    HeavyInitialization();

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
//  Method:  ViewerSubject::SendSimulationCommand
//
//  Purpose:
//    Sends a control command to a simulation.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 21, 2005
//
// ****************************************************************************
void
ViewerSubject::SendSimulationCommand()
{
    //
    // Get the rpc arguments.
    //
    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();
    const std::string &command  = GetViewerState()->GetViewerRPC()->GetStringArg1();
    const std::string &argument = GetViewerState()->GetViewerRPC()->GetStringArg2();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->
        SendSimulationCommand(EngineKey(hostName, simName), command, argument);
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
//    Brad Whitlock, Mon Apr 19 10:04:47 PDT 2004
//    I added the updateWindowInfo argument so we don't always have to update
//    the window information since that can cause extra updates in the gui.
//    I also added code to make the plot list check its active source vs the
//    active time slider so it can reset the active time slider if it no
//    longer makes sense to have one.
//
//    Jeremy Meredith, Wed Aug 25 10:34:53 PDT 2004
//    Made simulations connect the write socket from the engine to a new
//    socket notifier, which signals a method to read and process data from
//    the engine.  Hook up a new metadata and SIL atts observer to the 
//    metadata and SIL atts from the corresponding engine proxy, and have
//    those observers call callbacks when they get new information.
//
//    Brad Whitlock, Thu Feb 3 10:34:17 PDT 2005
//    Added a little more code to validate the timeState so if it's out of
//    range, we update the time slider to a valid value and we tell the
//    compute engine a valid time state at which to open the database. I also
//    made the routine return the time state in case it needs to be used
//    by the caller.
//
//    Jeremy Meredith, Tue Feb  8 08:58:49 PST 2005
//    Added a query for errors detected during plugin initialization.
//
//    Jeremy Meredith, Wed May 11 09:04:52 PDT 2005
//    Added security key to simulation connection.
//
//    Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//    Added ability to force using a specific plugin when opening a file.
//
//    Hank Childs, Thu Jan 11 15:33:07 PST 2007
//    Return an invalid time state when the file open fails.
//
//    Brad Whitlock, Thu Jan 25 18:48:15 PST 2007
//    Hooked up code to handle requests from the simulation.
//
//    Brad Whitlock, Thu Jan 24 12:00:29 PDT 2008
//    Added argument to ViewerPlotList::AddPlot().
//
//    Brad Whitlock, Fri Feb 15 14:54:34 PST 2008
//    Delete the adn from the default plot.
//
//    Hank Childs, Tue Feb 19 10:28:15 PST 2008
//    Fix bug introduced by Klocwork fix.
//
//    Brad Whitlock, Fri May  9 14:52:00 PDT 2008
//    Qt 4.
//
//    Cyrus Harrison,  Mon Aug  4 16:21:04 PDT 2008
//    Moved set of active host database until after we have obtained valid
//    meta data. 
//
//    Brad Whitlock, Tue Apr 14 13:38:10 PDT 2009
//    Use ViewerProperties.
//
// ****************************************************************************

int
ViewerSubject::OpenDatabaseHelper(const std::string &entireDBName,
    int timeState, bool addDefaultPlots, bool updateWindowInfo,
    const std::string &forcedFileType)
{
    int  i;
    const char *mName = "ViewerSubject::OpenDatabaseHelper: ";
    debug1 << mName << "Opening database " << entireDBName.c_str()
           << ", timeState=" << timeState
           << ", addDefaultPlots=" << (addDefaultPlots?"true":"false")
           << ", updateWindowInfo=" << (updateWindowInfo?"true":"false")
           << ", forcedFileType=\"" << forcedFileType.c_str() << "\"" << endl;

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
    

    //
    // Get the number of time states and set that information into the
    // active animation. The mdserver will clamp the time state that it
    // uses to open the database if timeState is out of range at this point.
    //
    const avtDatabaseMetaData *md = fs->GetMetaDataForState(host, db,
                                                            timeState,
                                                            forcedFileType);
    if (md != NULL)
    {
        // set the active host database name now that we have valid metadata.
        plotList->SetHostDatabaseName(hdb.c_str());
        
        //
        // If the database has more than one time state then we should
        // add it to the list of database correlations so we have a trivial
        // correlation for this database.
        //
        bool nStatesDecreased = false;
        if(md->GetNumStates() > 1)
        {
            //
            // Get the name of the database so we can use that for the name
            // of a new trivial database correlation.
            //
            const std::string &correlationName = plotList->GetHostDatabaseName();

            debug3 << mName << "Correlation for " << hdb.c_str() << " is "
                   << correlationName.c_str() << endl;

            //
            // In the case where we're reopening a database that now has
            // fewer time states, clamp the timeState value to be in the new
            // range of time states so we set the time slider to a valid
            // value and we use a valid time state when telling the compute
            // engine to open the database.
            //
            if(timeState > md->GetNumStates() - 1)
            {
                debug3 << mName << "There are " << md->GetNumStates()
                       << " time states in the database but timeState was "
                       << "set to "<< timeState <<". Clamping timeState to ";
                timeState = md->GetNumStates() - 1;
                debug3 << timeState << "." << endl;
                nStatesDecreased = true;
            }

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
        else if(timeState > 0)
        {
            debug3 << mName << "There is only 1 time state in the database "
                   << "but timeState was set to "<< timeState <<". Clamping "
                   << "timeState to 0.";
            timeState = 0;
            nStatesDecreased = true;
        }

        //
        // Make sure that it is appropriate to have the time slider that
        // is currently used in the plot list.
        //
        if(!wM->GetActiveWindow()->GetTimeLock())
            plotList->ValidateTimeSlider();

        // Alter the time slider for the database that we opened.
        if(nStatesDecreased)
            wM->AlterTimeSlider(hdb);

        //
        // Update the global atts since that has the list of sources.
        //
        wM->UpdateGlobalAtts();

        //
        // Since we updated the source and we made have also updated the time
        // slider and time slider states when the new database was opened, send
        // back the source, time sliders, and animation information.
        //
        if(updateWindowInfo || nStatesDecreased)
        {
            wM->UpdateWindowInformation(WINDOWINFO_SOURCE |
                WINDOWINFO_TIMESLIDERS | WINDOWINFO_ANIMATION);
        }

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
            ViewerEngineManager *vem = ViewerEngineManager::Instance();
            success = vem->ConnectSim(ek, noArgs,
                                      md->GetSimInfo().GetHost(),
                                      md->GetSimInfo().GetPort(),
                                      md->GetSimInfo().GetSecurityKey());

            if (success)
            {
                int sock = vem->GetWriteSocket(ek);
                QSocketNotifier *sn = new QSocketNotifier(sock,
                    QSocketNotifier::Read, this);

                simulationSocketToKey[sock] = ek;

                connect(sn, SIGNAL(activated(int)),
                        this, SLOT(ReadFromSimulationAndProcess(int)));

                engineKeyToNotifier[ek] = sn;

                engineMetaDataObserver[ek] = new ViewerMetaDataObserver(
                                   vem->GetSimulationMetaData(ek), host, db);
                connect(engineMetaDataObserver[ek],
                        SIGNAL(metaDataUpdated(const std::string&,const std::string&,
                                              const avtDatabaseMetaData*)),
                        this,
                        SLOT(HandleMetaDataUpdated(const std::string&,const std::string&,
                                                 const avtDatabaseMetaData*)));

                engineSILAttsObserver[ek] = new ViewerSILAttsObserver(
                                   vem->GetSimulationSILAtts(ek), host, db);
                connect(engineSILAttsObserver[ek],
                        SIGNAL(silAttsUpdated(const std::string&,const std::string&,
                                              const SILAttributes*)),
                        this,
                        SLOT(HandleSILAttsUpdated(const std::string&,const std::string&,
                                                  const SILAttributes*)));

                engineCommandObserver[ek] = new ViewerCommandFromSimObserver(
                                   vem->GetCommandFromSimulation(ek), ek, db);
                connect(engineCommandObserver[ek],
                        SIGNAL(execute(const EngineKey&,const std::string&,
                                       const std::string &)),
                        this,
                        SLOT(DeferCommandFromSimulation(const EngineKey&,const std::string&,
                                                        const std::string &)));

            }
        }
        else
        {
            if (GetViewerProperties()->GetNowin())
            {
                success = ViewerEngineManager::Instance()->
                    CreateEngine(ek,
                                 engineParallelArguments,
                                 false,
                                 GetViewerProperties()->GetNumEngineRestarts());
            }
            else
            {
                success = ViewerEngineManager::Instance()->
                    CreateEngine(ek,
                                 noArgs,
                                 false,
                                 GetViewerProperties()->GetNumEngineRestarts());
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
            bool defaultPlotsAdded = false;

            for(i=0; i<md->GetNumDefaultPlots(); i++)
            {
                const avtDefaultPlotMetaData *dp = md->GetDefaultPlot(i);
                DataNode *adn = CreateAttributesDataNode(dp);

                //
                // Use the plot plugin manager to get the plot type index from
                // the plugin id.
                //
                int type = GetPlotPluginManager()->GetEnabledIndex(dp->pluginID);

                if(type != -1)
                {
                    debug4 << "Adding default plot: type=" << type
                           << " var=" << dp->plotVar.c_str() << endl;
                    plotList->AddPlot(type, dp->plotVar, false, false, true, adn);
                    defaultPlotsAdded = true;
                }

                if (adn != NULL)
                {
                    delete adn;
                    adn = NULL;
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
        }
        else
        {
            debug4 << "Default plots were not added because the plot list "
                      "already contains plots from "
                   << host.c_str() << ":" << db.c_str() << endl;
        }
    }
    else
    {
        // We had a problem opening the file ... indicate that with the
        // return value (which is timeState).
        timeState = -1;
    }

    //
    // Check to see if there were errors in the mdserver
    //
    string err = ViewerFileServer::Instance()->GetPluginErrors(host);
    if (!err.empty())
    {
        Warning(err.c_str());
    }

    return timeState;
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
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper.
//
//   Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//   Added ability to force using a specific plugin when opening a file.
//
//   Hank Childs, Thu Jan 11 15:33:07 PST 2007
//   Add return value to indicate errors.
//
// ****************************************************************************

bool
ViewerSubject::OpenDatabase()
{
    int ts = OpenDatabaseHelper(GetViewerState()->GetViewerRPC()->GetDatabase(), GetViewerState()->GetViewerRPC()->GetIntArg1(),
                       GetViewerState()->GetViewerRPC()->GetBoolFlag(), true,
                       GetViewerState()->GetViewerRPC()->GetStringArg1());
    return (ts >= 0 ? true : false);
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
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper.
//   
// ****************************************************************************

void
ViewerSubject::ActivateDatabase()
{
    const std::string &database = GetViewerState()->GetViewerRPC()->GetDatabase();

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
        OpenDatabaseHelper(database, 0, true, true);
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
//   Brad Whitlock, Tue Jul 27 10:10:47 PDT 2004
//   I made it do something.
//
// ****************************************************************************

void
ViewerSubject::CheckForNewStates()
{
    //
    // Add new states to the specified database and update the metadata,
    // correlations, and plot list caches.
    //
    debug1 << "CheckForNewStates: " << GetViewerState()->GetViewerRPC()->GetDatabase().c_str() << endl;

    ViewerWindowManager::Instance()->CheckForNewStates(GetViewerState()->GetViewerRPC()->GetDatabase());
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
//   Jeremy Meredith, Thu Apr 15 17:01:11 PDT 2004
//   Added the reOpenState to the ReplaceDatabase call so that the right
//   MD/SIL would get used in all circumstances.
//
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper.
//
//   Brad Whitlock, Mon May 3 13:00:21 PST 2004
//   I made it pass an engine key to replacedatabase.
//
//   Jeremy Meredith, Wed Aug 25 10:39:25 PDT 2004
//   Made it use the generic integer argument for forceClose so as to not be
//   misleading.
//
//    Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//    Changed the interface to ReplaceDatabase to support option for only 
//    replacing active plots.
//
// ****************************************************************************

void
ViewerSubject::ReOpenDatabase()
{
    //
    // Get the rpc arguments.
    //
    std::string hostDatabase(GetViewerState()->GetViewerRPC()->GetDatabase());
    bool forceClose = (GetViewerState()->GetViewerRPC()->GetIntArg1() == 1);

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
    // will remove the correlation for the database.  Do not clear the
    // metadata if it is a simulation because we lose all of our current
    // information by doing so (since the mdserver has almost no information).
    // If it is a simulation, we will get updated metadata indirectly when
    // we open the database again, regardless of if we clear the cached one.
    //
    if (!isSim)
        fileServer->ClearFile(hostDatabase);

    //
    // Tell the compute engine to clear any cached information about the
    // database so it forces the networks to re-execute.
    //
    EngineKey key(host, "");
    if (isSim)
        key = EngineKey(host, db);

    //
    // Clear the cache for the database.
    //
    ViewerEngineManager::Instance()->ClearCache(key, db.c_str());

    //
    // Open the database. Since reopening a file can result in a different
    // number of time states (potentially fewer), use the time state returned
    // by OpenDatabaseHelper for the replace operation.
    //
    reOpenState = OpenDatabaseHelper(hostDatabase, reOpenState, false, true);

    //
    // Now perform the database replacement in all windows that use the
    // specified database.
    //
    ViewerWindowManager::Instance()->ReplaceDatabase(key, db, reOpenState,
                                                     false, true, false);
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
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper so it won't update the
//   window information since we're already doing that here. I also added
//   a call to validate the plot lists's time slider.
//
//   Brad Whitlock, Mon May 3 13:21:19 PST 2004
//   I made it use the plot list's engine key in the call to ReplaceDatabase.
//
//   Brad Whitlock, Thu Feb 3 11:07:53 PDT 2005
//   I made the time state used for file replacement be the value that is now
//   returned from OpenDatabaseHelper.
//
//   Brad Whitlock, Wed Mar 16 16:32:14 PST 2005
//   I added code to make sure that the active window is still compatible
//   with other time-locked windows after the database was replaced.
//
//    Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//    Changed the interface to ReplaceDatabase to support option for only 
//    replacing active plots.
//
// ****************************************************************************

void
ViewerSubject::ReplaceDatabase()
{
    debug4 << "ReplaceDatabase: db=" 
           << GetViewerState()->GetViewerRPC()->GetDatabase().c_str()
           << ", time=" << GetViewerState()->GetViewerRPC()->GetIntArg1() 
           << ", onlyReplaceActive=" 
           << GetViewerState()->GetViewerRPC()->GetIntArg2() << endl;

    //
    // If the replace is merely changing the timestate, then turn on
    // view limit merging.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerWindow *win = wM->GetActiveWindow();
    if(win == 0)
        return;

    if (GetViewerState()->GetViewerRPC()->GetDatabase() == win->GetPlotList()->GetHostDatabaseName())
        win->SetMergeViewLimits(true);

    //
    // First open the database.
    //
    int timeState = GetViewerState()->GetViewerRPC()->GetIntArg1();
    
    timeState = OpenDatabaseHelper(GetViewerState()->GetViewerRPC()->GetDatabase(), timeState,
                                   false, false);

    bool onlyReplaceActive = (bool) GetViewerState()->GetViewerRPC()->GetIntArg2();
    //
    // Now perform the database replacement.
    //
    ViewerPlotList *plotList = win->GetPlotList();
    plotList->ReplaceDatabase(plotList->GetEngineKey(),
                              plotList->GetDatabaseName(),
                              timeState,
                              true,
                              false,
                              onlyReplaceActive);

    //
    // If the current window is time-locked then we have to make sure that
    // its new database is compatible with being time-locked with other
    // windows.
    //
    if(win->GetTimeLock())
    {
        intVector windowIds;
        wM->GetTimeLockedWindowIndices(windowIds);
        if(windowIds.size() > 1)
        {
            debug2 << "We have more than 1 time locked window. We have to "
                      "make sure that we have a suitable multi-window "
                      "database correlation."
                   << endl;
            // Create or alter the most suitable correlation to be used for
            // time-locked windows. If we have to create a multi-window
            // database correlation then we'll have set
            DatabaseCorrelation *C = wM->CreateMultiWindowCorrelation(windowIds);
            if(C != 0)
            {
                std::string hdb(win->GetPlotList()->GetHostDatabaseName());
                debug2 << "ReplaceDatabase: The active window is time-locked "
                          "and uses the multi-window database correlation: "
                       << C->GetName().c_str() << ". We have to make sure that "
                       << "we display time state " << timeState << " for the "
                       << "database: " << hdb.c_str() << endl;

                // We have to find the time state in C where we find
                // the new database's new state. Then we have to make
                // the other time locked windows go to that time state.
                int cts = C->GetInverseCorrelatedTimeState(hdb, timeState);
                if(cts != -1)
                {
                    debug2 << "Correlation state "<< cts
                           << " will allow us to show time state "
                           << timeState << " for database " << hdb.c_str()
                           << ". We also have to update the other locked "
                              "windows.\n";
                    wM->SetFrameIndex(cts);
                }
            } // C != 0
        }
    }

    //
    // Make sure the time slider is set to something appropriate.
    //
    plotList->ValidateTimeSlider();

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
//   Brad Whitlock, Mon Apr 19 10:00:13 PDT 2004
//   I added another argument to OpenDatabaseHelper.
//
//   Brad Whitlock, Mon May 3 13:58:36 PST 2004
//   I removed an argument from OverlayDatabase.
//
//   Brad Whitlock, Thu Jul 24 09:21:56 PDT 2008
//   Made it possible to overlay at a particular time state.
//
// ****************************************************************************

void
ViewerSubject::OverlayDatabase()
{
    int state = GetViewerState()->GetViewerRPC()->GetIntArg1();
    debug4 << "OverlayDatabase: db=" << GetViewerState()->GetViewerRPC()->GetDatabase().c_str()
           << ", time=" << state << endl;

    //
    // First open the database.
    //
    OpenDatabaseHelper(GetViewerState()->GetViewerRPC()->GetDatabase(), 
                       state, false, true);

    //
    // Now perform the database replacement.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();
    plotList->OverlayDatabase(plotList->GetEngineKey(),
                              plotList->GetDatabaseName(), state);

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
    ViewerWindowManager::Instance()->CloseDatabase(GetViewerState()->GetViewerRPC()->GetDatabase());
}

// ****************************************************************************
// Method: ViewerSubject::HandleRequestMetaData
//
// Purpose: 
//   Gets metadata for the specified database and time state and returns it
//   to the client. There should be no side effects such as setting the 
//   active database.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 9 16:30:48 PST 2007
//
// Modifications:
//   
//   Mark C. Miller, Wed Apr 22 13:48:13 PDT 2009
//   Changed interface to DebugStream to obtain current debug level.
// ****************************************************************************

void
ViewerSubject::HandleRequestMetaData()
{
    const char *mName = "ViewerSubject::HandleRequestMetaData: ";

    //
    // Expand the new database name and then set it into the plot list.
    //
    std::string hdb(GetViewerState()->GetViewerRPC()->GetDatabase()), host, db;
    ViewerFileServer *fs = ViewerFileServer::Instance();
    fs->ExpandDatabaseName(hdb, host, db);

    //
    // Get the number of time states and set that information into the
    // active animation. The mdserver will clamp the time state that it
    // uses to open the database if timeState is out of range at this point.
    //
    const avtDatabaseMetaData *md = NULL;
    int ts = GetViewerState()->GetViewerRPC()->GetStateNumber();
    if(ts == -1)
    {
        debug4 << mName << "Calling fs->GetMetaData(" << host
               << ", " << db << ", true)" << endl;
        md = fs->GetMetaData(host, db, true);
    }
    else
    {
        // The time state is not "ANY" so we will ask for a particular
        // database state. Some file formats can be time-varying or
        // time-invariant and we don't know which it will be so we
        // need to clear the metadata for the database so we will read
        // it again at the right time state.
        fs->ClearFile(hdb);

        debug4 << mName << "Calling fs->GetMetaDataForState(" << host
               << ", " << db << ", " << ts << ", \"\")" << endl;
        md = fs->GetMetaDataForState(host, db, ts, true, "");
    }

    if(md != 0)
    {
        // Copy the metadata so we can send it to the client.
        *GetViewerState()->GetDatabaseMetaData() = *md;

        // Print the metadata to the debug logs.
        debug5 << mName << "Metadata contains: " << endl;
        if(DebugStream::Level5())
            md->Print(DebugStream::Stream5(), 1);
    }
    else
    {
        // Empty out the metadata.
        *GetViewerState()->GetDatabaseMetaData() = avtDatabaseMetaData();

        debug5 << mName << "No metadata was found." << endl;
    }
    GetViewerState()->GetDatabaseMetaData()->SelectAll();
    GetViewerState()->GetDatabaseMetaData()->Notify();

    //
    // Check to see if there were errors in the mdserver
    //
    string err = ViewerFileServer::Instance()->GetPluginErrors(host);
    if (!err.empty())
    {
        Warning(err.c_str());
    }
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
//   Brad Whitlock, Mon Apr 19 08:44:46 PDT 2004
//   I moved the code to update the window information from
//   ViewerWindowManager::CreateDatabaseCorrelation to here so things in the
//   gui don't update as frequently when opening a database.
//
//   Brad Whitlock, Wed Apr 30 09:18:24 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::CreateDatabaseCorrelation()
{
    const std::string &name = GetViewerState()->GetViewerRPC()->GetDatabase();

    //
    // Make sure that the correlation does not have the same name as
    // an existing source.
    //
    if(ViewerFileServer::Instance()->IsDatabase(name))
    {
        QString err = tr("You cannot define a database correlation that "
                         "has the same name as a source. No database "
                         "correlation will be created for %1.").
                      arg(name.c_str());
        Error(err);
    }
    else
    {
        ViewerWindowManager::Instance()->CreateDatabaseCorrelation(
            name, GetViewerState()->GetViewerRPC()->GetProgramOptions(),
            GetViewerState()->GetViewerRPC()->GetIntArg1(), 0, GetViewerState()->GetViewerRPC()->GetIntArg2());
        ViewerWindowManager::Instance()->UpdateWindowInformation(
            WINDOWINFO_TIMESLIDERS | WINDOWINFO_ANIMATION);
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
        GetViewerState()->GetViewerRPC()->GetDatabase(), GetViewerState()->GetViewerRPC()->GetProgramOptions(),
        GetViewerState()->GetViewerRPC()->GetIntArg1(), GetViewerState()->GetViewerRPC()->GetIntArg2());
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
    const std::string &name = GetViewerState()->GetViewerRPC()->GetDatabase();
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
//    Brad Whitlock, Wed Apr 13 14:58:21 PST 2005
//    I made it issue a warning message if a compute engine is already
//    running on the desired host.
//
//    Brad Whitlock, Wed Apr 30 09:19:27 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Tue Apr 14 13:40:52 PDT 2009
//    Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::OpenComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const string       &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const stringVector &options  = GetViewerState()->GetViewerRPC()->GetProgramOptions();

    //
    // Perform the rpc.
    //
    bool givenOptions = (options.size() > 0);
    bool givenCLArgs  = (engineParallelArguments.size() > 0);

    EngineKey key(hostName, "");
    if(ViewerEngineManager::Instance()->EngineExists(key))
    {
        QString msg = tr("VisIt did not open a new compute engine on host %1 "
                         "because a compute engine is already running there.").
                      arg(hostName.c_str());
        Warning(msg);
    }
    else if (givenOptions)
    {
        ViewerEngineManager::Instance()->CreateEngine(
            key,
            options,
            true,
            GetViewerProperties()->GetNumEngineRestarts());
    }
    else if (GetViewerProperties()->GetNowin() && givenCLArgs)
    {
        ViewerEngineManager::Instance()->CreateEngine(
            key,
            engineParallelArguments,
            true,
            GetViewerProperties()->GetNumEngineRestarts());
    }
    else
    {
        ViewerEngineManager::Instance()->CreateEngine(
            key,
            options,
            false,
            GetViewerProperties()->GetNumEngineRestarts());
    }
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
//   Jeremy Meredith, Tue Mar 30 10:52:06 PST 2004
//   Added an engine key used to index (and restart) engines.
//   This was needed for simulation support.
// 
//   Brad Whitlock, Mon May 3 14:18:03 PST 2004
//   I added code to reset the network ids for the plots that use the engine
//   that we're closing.
//  
// ****************************************************************************

void
ViewerSubject::CloseComputeEngine()
{
    //
    // Get the rpc arguments.
    //
    const string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

    //
    // We're closing the engine so reset all of the network ids for plots that
    // are on the specified engine. This ensures that pick, etc works when
    // we use a new compute engine.
    //
    EngineKey key(hostName, simName);
    ViewerWindowManager::Instance()->ResetNetworkIds(key);

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->CloseEngine(key);
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
    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const stringVector &options = GetViewerState()->GetViewerRPC()->GetProgramOptions();
    ViewerFileServer::Instance()->NoFaultStartServer(hostName, options);
}

// ****************************************************************************
// Method: ViewerSubject::UpdateDBPluginInfo
//
// Purpose: 
//    Updates the DB plugin info.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// ****************************************************************************

void
ViewerSubject::UpdateDBPluginInfo()
{
    //
    // Get the rpc arguments.
    //
    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();

    //
    // Perform the RPC.
    //
    ViewerFileServer::Instance()->UpdateDBPluginInfo(hostName);
}

// ****************************************************************************
// Method: ViewerSubject::ConstructDDF
//
// Purpose: 
//     Construct a derived data function.
//
// Programmer: Hank Childs
// Creation:   February 13, 2006
//
// Modifications:
//   Brad Whitlock, Wed Apr 30 09:20:14 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::ConstructDDF()
{
    //
    // Perform the RPC.
    //
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIDs;
    plist->GetActivePlotIDs(plotIDs);
    if (plotIDs.size() <= 0)
    {
        Error(tr("To construct a derived data function, you must have an active"
                 " plot.  No DDF was created."));
        return;
    }
    if (plotIDs.size() > 1)
        Message(tr("Only one DDF can be created at a time.  VisIt is using the "
                   "first active plot."));

    ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
    const EngineKey   &engineKey = plot->GetEngineKey();
    int networkId = plot->GetNetworkID();
    TRY
    {
        if (ViewerEngineManager::Instance()->ConstructDDF(engineKey, 
                                                          networkId))
        {
            Message(tr("Created DDF"));
        }
        else
        {
            Error(tr("Unable to create DDF"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::ExportDatabase
//
// Purpose: 
//     Exports a database.
//
// Programmer: Hank Childs
// Creation:   May 25, 2005
//
// Modifications:
//   Jeremy Meredith, Tue Mar 27 16:55:05 EDT 2007
//   Added error text to message when we have something more useful to say.
//
//   Brad Whitlock, Wed Apr 30 09:21:22 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::ExportDatabase()
{
    //
    // Perform the RPC.
    //
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIDs;
    plist->GetActivePlotIDs(plotIDs);
    if (plotIDs.size() <= 0)
    {
        Error(tr("To export a database, you must have an active plot.  No database was saved."));
        return;
    }
    if (plotIDs.size() > 1)
        Message(tr("Only one database can be exported at a time.  VisIt is exporting the first active plot."));

    ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
    const EngineKey   &engineKey = plot->GetEngineKey();
    int networkId = plot->GetNetworkID();
    TRY
    {
        if (ViewerEngineManager::Instance()->ExportDatabase(engineKey, 
                                                            networkId))
        {
            Message(tr("Exported database"));
        }
        else
        {
            Error(tr("Unable to export database"));
        }
    }
    CATCH2(VisItException, e)
    {
        QString msg = tr("Unable to export database: %1").arg(e.Message().c_str());
        Error(msg);
    }
    ENDTRY
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
    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

    //
    // Perform the RPC.
    //
    ViewerEngineManager::Instance()->ClearCache(EngineKey(hostName, simName));
}

// ****************************************************************************
// Method: ViewerSubject::ClearCacheForAllEngines
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
    int       type = GetViewerState()->GetViewerRPC()->GetPlotType();

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
    int plot = GetViewerState()->GetViewerRPC()->GetPlotType();

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
//  Method: ViewerSubject::AddInitializedOperator
//
//  Purpose:
//    Execute the AddInitializedOperator RPC.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue May 8 16:54:36 PST 2007
//
//  Modifications:
//    Kathleen Bonnell, Fri Sep 14 16:28:38 PDT 2007
//    Lineout needs a different path.
//
// ****************************************************************************

void
ViewerSubject::AddInitializedOperator()
{
    //
    // Get the rpc arguments.
    //
    int type = GetViewerState()->GetViewerRPC()->GetOperatorType();

    OperatorPluginManager *opMgr = GetOperatorPluginManager();
    bool lineout = (opMgr->GetPluginName(opMgr->GetEnabledID(type))
                    == "Lineout");

    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    if (!lineout)
    {
        bool applyToAll = wM->GetClientAtts()->GetApplyOperator();
        wM->GetActiveWindow()->GetPlotList()->AddOperator(type, applyToAll, false);
    }
    else
    {
        wM->GetActiveWindow()->Lineout(false);
    }
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
    int       type = GetViewerState()->GetViewerRPC()->GetOperatorType();

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
    int oper = GetViewerState()->GetViewerRPC()->GetOperatorType();

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
// Method: ViewerSubject::SetViewAxisArray
//
// Purpose: 
//   Tells the viewer window manager to apply the axis array view
//   attributes to the active window.
//
// Programmer: Jeremy Meredith
// Creation:   February  4, 2008
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetViewAxisArray()
{
    //
    // Perform the rpc.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetViewAxisArrayFromClient();
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
    int frame = GetViewerState()->GetViewerRPC()->GetFrame();
 
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
    int oldFrame = GetViewerState()->GetViewerRPC()->GetIntArg1();
    int newFrame = GetViewerState()->GetViewerRPC()->GetIntArg2();
 
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
    const char *ctName = GetViewerState()->GetViewerRPC()->GetColorTableName().c_str();

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
//   Brad Whitlock, Wed Apr 30 09:25:22 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::ExportColorTable()
{
    //
    // Perform the rpc.
    //
    const std::string &ctName = GetViewerState()->GetViewerRPC()->GetColorTableName();
    std::string msg;
    if(avtColorTables::Instance()->ExportColorTable(ctName, msg))
    {
        // If we successfully exported the color table, msg is set to the
        // name of the color table file that was created. We want to send
        // a status message and a message.
        QString msg2 = tr("Color table %1 exported to %2").
                       arg(ctName.c_str()).
                       arg(msg.c_str());
        Status(msg2);

        // Tell the user what happened.
        msg2 = tr("VisIt exported color table \"%1\" to the file: %2. "
           "You can share that file with colleagues who want to use your "
           "color table. Simply put the file in their .visit directory, run "
           "VisIt and the color table will appear in their list of color "
           "tables when VisIt starts up.").
           arg(ctName.c_str()).
           arg(msg.c_str());
        Message(msg2);
    }
    else
        Error(msg.c_str(), false);
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
//    Brad Whitlock, Wed Feb 16 11:51:49 PDT 2005
//    Made it call utility function GetDefaultConfigFile.
//
//    Brad Whitlock, Thu Feb 17 16:11:21 PST 2005
//    I made it issue an error message if the settings can't be saved.
//
//    Brad Whitlock, Wed Apr 30 09:26:08 PDT 2008
//    Support for internationalization.
//
//    Brad Whitlock, Tue Apr 14 13:42:31 PDT 2009
//    Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::WriteConfigFile()
{
    std::string configFileName(GetViewerProperties()->GetConfigurationFileName());
    const char *cfn = (configFileName != "") ? configFileName.c_str() : 0;
    char *defaultConfigFile = GetDefaultConfigFile(cfn);

    //
    // Tell the ViewerWindowManager to get the current location, size of the
    // viewer windows so that information can be saved.
    //
    ViewerWindowManager::Instance()->UpdateWindowAtts();

    //
    // Tell the configuration manager to write the file.
    //
    if(!configMgr->WriteConfigFile(defaultConfigFile))
    {
        QString err = tr("VisIt could not save your settings to: %1.").
                      arg(defaultConfigFile);
        Error(err);
    }

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
    configMgr->ExportEntireState(GetViewerState()->GetViewerRPC()->GetVariable());
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
//   Brad Whitlock, Fri Nov 10 09:38:25 PDT 2006
//   Added arguments to the call to configMgr->ImportEntireState.
//
// ****************************************************************************

void
ViewerSubject::ImportEntireState()
{
    stringVector empty;
    configMgr->ImportEntireState(GetViewerState()->GetViewerRPC()->GetVariable(),
                                 GetViewerState()->GetViewerRPC()->GetBoolFlag(),
                                 empty, false);
    configMgr->NotifyIfSelected();
}

// ****************************************************************************
// Method: ViewerSubject::ImportEntireStateWithDifferentSources
//
// Purpose: 
//   Restores a session file with a different list of sources.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 10 09:37:54 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ImportEntireStateWithDifferentSources()
{
     configMgr->ImportEntireState(GetViewerState()->GetViewerRPC()->GetVariable(),
                                  GetViewerState()->GetViewerRPC()->GetBoolFlag(),
                                  GetViewerState()->GetViewerRPC()->GetProgramOptions(), true);
     configMgr->NotifyIfSelected();
}

// ****************************************************************************
// Method: ViewerSubject::RemoveCrashRecoveryFile
//
// Purpose: 
//   Removes the viewer's crash recovery file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 12:06:32 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri May  9 14:54:53 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
ViewerSubject::RemoveCrashRecoveryFile() const
{
    QString filename(GetUserVisItDirectory().c_str());
    filename += "crash_recovery";
#if defined(_WIN32)
    filename += ".vses";
#else
    filename += ".session";
#endif
    // Remove the viewer's crash recovery file if it exists.
    QFile cr(filename);
    if(cr.exists())
    {
        debug1 << "Removing crash recovery file: "
               << filename.toStdString() << endl;
        cr.remove();
    }
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
//   Brad Whitlock, Tue Mar 20 13:47:49 PST 2007
//   Added name argument.
//
// ****************************************************************************

void
ViewerSubject::AddAnnotationObject()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->AddAnnotationObject(GetViewerState()->GetViewerRPC()->GetIntArg1(),
                            GetViewerState()->GetViewerRPC()->GetStringArg1());
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
//  Method: ViewerSubject::SetMeshManagementAttributes
//
//  Purpose: Execute the SetMeshManagementAttributes RPC.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005 
//
// ****************************************************************************

void
ViewerSubject::SetMeshManagementAttributes()
{
    // Do nothing; there is only a global copy, and nothing
    // is regenerated automatically just yet
}

// ****************************************************************************
//  Method: ViewerSubject::SetDefaultMeshManagementAttributes
//
//  Purpose: Sets the default material atts from the client material atts.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005 
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultMeshManagementAttributes()
{
    ViewerEngineManager *eM=ViewerEngineManager::Instance();
    eM->SetDefaultMeshManagementAttsFromClient();
}

// ****************************************************************************
//  Method: ViewerSubject::ResetMeshManagementAttributes
//
//  Purpose: Sets the default material attributes into the material attributes
//
//  Programmer: Mark C. Miller 
//  Creation:   November 5, 2005 
//   
// ****************************************************************************

void
ViewerSubject::ResetMeshManagementAttributes()
{
    ViewerEngineManager *eM=ViewerEngineManager::Instance();
    eM->SetClientMeshManagementAttsFromDefault();
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
//   Brad Whitlock, Tue Apr 14 13:43:06 PDT 2009
//   Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::SetWindowArea()
{
    const char *area = GetViewerState()->GetViewerRPC()->GetWindowArea().c_str();
    
    //
    // Recalculate the layouts and reposition the windows.
    //
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    if(GetViewerProperties()->GetWindowSmall())
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
//   Jeremy Meredith, Tue Feb  8 08:58:49 PST 2005
//   Added a query for errors detected during plugin initialization.
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
           << GetViewerState()->GetViewerRPC()->GetProgramHost().c_str()
           << " to connect to another client." << endl;
    debug4 << "Arguments:" << endl;
    const stringVector &sv = GetViewerState()->GetViewerRPC()->GetProgramOptions();
    for(int i = 0; i < sv.size(); ++i)
         debug4 << "\t" << sv[i].c_str() << endl;

    //
    // Tell the viewer's fileserver to have its mdserver running on 
    // the specified host to connect to another process.
    //
    ViewerFileServer::Instance()->ConnectServer(
        GetViewerState()->GetViewerRPC()->GetProgramHost(),
        GetViewerState()->GetViewerRPC()->GetProgramOptions()); 

    visitTimer->StopTimer(timeid, "Time spent telling mdserver to connect to client.");

    //
    // Check to see if there were errors in mdserver plugin initialization
    //
    string err = ViewerFileServer::Instance()->
                                GetPluginErrors(GetViewerState()->GetViewerRPC()->GetProgramHost());
    if (!err.empty())
    {
        Warning(err.c_str());
    }

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
//  Method: ViewerSubject::ToggleMaintainDataMode
//
//  Purpose: 
//    A Qt slot function that toggles the maintain view mode for the specified
//    window.
//
//  Arguments:
//    windowIndex  The index of the window whose maintain data mode will be
//                 toggled.
//
//  Programmer: Eric Brugger
//  Creation:   March 29, 2004
//
// ****************************************************************************

void
ViewerSubject::ToggleMaintainDataMode(int windowIndex)
{
    ViewerWindowManager::Instance()->ToggleMaintainDataMode(windowIndex);
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
     avtExtentType viewType = (avtExtentType)GetViewerState()->GetViewerRPC()->GetWindowLayout();
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
//   Kathleen Bonnell, Wed Dec 15 17:12:47 PST 2004 
//   Added another bool arg to qm->DatabaseQuery. 
//   
//   Hank Childs, Tue Jul 11 14:34:06 PDT 2006
//   Add double arguments.
//
//   Brad Whitlock, Wed Apr 30 09:27:08 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::DatabaseQuery()
{
    // Send the client a status message.
    QString msg = tr("Performing %1 query...").
                  arg(GetViewerState()->GetViewerRPC()->GetQueryName().c_str());
    Status(msg);

    ViewerWindow *vw = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerQueryManager *qm = ViewerQueryManager::Instance();
    qm->DatabaseQuery(vw, GetViewerState()->GetViewerRPC()->GetQueryName(), GetViewerState()->GetViewerRPC()->GetQueryVariables(),
                      GetViewerState()->GetViewerRPC()->GetBoolFlag(), 
                      GetViewerState()->GetViewerRPC()->GetIntArg1(), GetViewerState()->GetViewerRPC()->GetIntArg2(),
                      (bool)GetViewerState()->GetViewerRPC()->GetIntArg3(), GetViewerState()->GetViewerRPC()->GetDoubleArg1(),
                      GetViewerState()->GetViewerRPC()->GetDoubleArg2());

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
//   Kathleen Bonnell, Wed Dec 15 17:12:47 PST 2004 
//   Added another bool arg to qm->DatabaseQuery. 
//
//   Brad Whitlock, Wed Apr 30 09:27:46 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::PointQuery()
{
    // Send the client a status message.
    QString msg = tr("Performing %1 query...").
                  arg(GetViewerState()->GetViewerRPC()->GetQueryName().c_str());
    Status(msg);

    ViewerQueryManager *qm = ViewerQueryManager::Instance();
    qm->PointQuery(GetViewerState()->GetViewerRPC()->GetQueryName(), 
                   GetViewerState()->GetViewerRPC()->GetQueryPoint1(),
                   GetViewerState()->GetViewerRPC()->GetQueryVariables(),
                   GetViewerState()->GetViewerRPC()->GetIntArg1(), 
                   GetViewerState()->GetViewerRPC()->GetIntArg2(),
                   GetViewerState()->GetViewerRPC()->GetBoolFlag(),
                   (bool)GetViewerState()->GetViewerRPC()->GetIntArg3()); 

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
//   Kathleen Bonnell, Fri Jul  9 16:24:56 PDT 2004 
//   Changed Call from "LineQuery" to "StartLineQuery", added call to
//   MessageRendererThread. 
//   
//   Kathleen Bonnell, Tue May 15 10:43:49 PDT 2007 
//   Added bool arg to StartLineQuery. 
//
//   Brad Whitlock, Wed Apr 30 09:27:08 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
ViewerSubject::LineQuery()
{
    // Send the client a status message.
    QString msg = tr("Performing %1 query...").
                  arg(GetViewerState()->GetViewerRPC()->GetQueryName().c_str());
    Status(msg);

    ViewerQueryManager::Instance()->StartLineQuery( 
        GetViewerState()->GetViewerRPC()->GetQueryName().c_str(),
        GetViewerState()->GetViewerRPC()->GetQueryPoint1(), 
        GetViewerState()->GetViewerRPC()->GetQueryPoint2(),
        GetViewerState()->GetViewerRPC()->GetQueryVariables(), 
        GetViewerState()->GetViewerRPC()->GetIntArg1(),
        GetViewerState()->GetViewerRPC()->GetBoolFlag());
    MessageRendererThread("finishLineQuery;");
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
//    Brad Whitlock, Mon Aug 23 17:08:27 PST 2004
//    I changed this routine so it is called when blockSocketSignals is true
//    but in that case, it reschedules itself to run later. This fixes a
//    problem on Windows where the socket notifier did not keep notifying
//    that the socket had input even though we did not read it.
//
//    Mark C. Miller, Tue Dec 14 14:09:45 PST 2004
//    Re-ordered else clauses so that if(blockSocketSignals) comes before
//    if(processingFromParent).
//
//    Brad Whitlock, Tue Dec 21 15:04:30 PST 2004
//    I added code to reschedule the method to run again later in the event
//    that it gets here "recursively". What was happening was that the
//    OpenComputeEngine from the CLI sometimes was processed independently
//    of the Sync that was followed and while processing the OpenComputeEngine
//    RPC, we got here again from the connection progress dialog. We were not
//    rescheduling the method to run again and were losing the sync. When the
//    sync was not being sent back to the CLI, it hung. VisIt00005692.
//
// ****************************************************************************

void
ViewerSubject::ProcessFromParent()
{
    if(ViewerEngineManager::Instance()->InExecute())
    {
        debug1 << "The viewer engine manager is busy processing a request "
                  "so we should not process input from the client. Let's "
                  "reschedule this method to run again later." << endl;
        QTimer::singleShot(200, this, SLOT(ProcessFromParent()));
    }
    else if(blockSocketSignals)
    {
        debug1 << "The viewer is set to ignore input from the client at this "
                  "time. Let's reschedule this method to run again later."
               << endl;
        QTimer::singleShot(200, this, SLOT(ProcessFromParent()));
    }
    else if(processingFromParent)
    {
        debug1 << "The viewer tried to recursively enter "
                  "ViewerSubject::ProcessFromParent! Let's return from this "
                  "level and reschedule this method to run again later."
               << endl;
        QTimer::singleShot(200, this, SLOT(ProcessFromParent()));
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
        CATCHALL(...)
        {
            processingFromParent = false;
            // Consume the exception.
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
//    Brad Whitlock, Mon Aug 23 17:10:02 PST 2004
//    I added conditional compilation for the code that causes the method to
//    return early without reading the input from the client. Now the code to
//    ignore the client is in the ProcessFromParent method.
//
//    Brad Whitlock, Thu Aug 14 09:57:44 PDT 2008
//    Use qApp.
//
// ****************************************************************************

void
ViewerSubject::ReadFromParentAndProcess(int)
{
    TRY
    {
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
        qApp->quit();
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
// Method: ViewerSubject::EnabledSocketSignals
//
// Purpose: 
//   Enables the viewer to read from the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 2 15:44:49 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::EnableSocketSignals()
{
    blockSocketSignals = false;
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
//    Kathleen Bonnell, Fri Jul  9 13:40:42 PDT 2004 
//    Added handlers for finishLineout, and finishLineQuery.
//
//    Kathleen Bonnell, Thu Aug  5 17:30:06 PDT 2004 
//    Added handlers for 'Sync'. (at Brad's suggestion).
//
//    Mark C. Miller, Thu Nov 11 16:31:43 PST 2004
//    Moved code to test if VEM is InRender to inside the block that 
//    processes the SR mode change message
//
//    Mark C. Miller, Sat Nov 13 09:35:51 PST 2004
//    Removed code to test if VEM is InRender and defer SR mode change
//
//    Brad Whitlock, Tue Mar 20 11:59:36 PDT 2007
//    Added updateAOL to update the annotation object list.
//
//    Brad Whitlock, Thu Jan 24 09:45:18 PST 2008
//    Added simcmd to handle deferred simulation commands.
//
//    Brad Whitlock, Fri Jan 9 14:47:07 PST 2009
//    Added exception handling code so exceptions cannot get back into the
//    Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::ProcessRendererMessage()
{
    TRY
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
            CATCH_RETURN(1);
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
                CATCH_RETURN(1);
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
            else if (strncmp(msg, "finishLineout", 13) == 0)
            {
                ViewerQueryManager::Instance()->FinishLineout(); 
            }
            else if (strncmp(msg, "finishLineQuery", 15) == 0)
            {
                ViewerQueryManager::Instance()->FinishLineQuery(); 
                ClearStatus();
            }
            else if (strncmp(msg, "Sync", 4) == 0)
            {
                int tag = 0; 
                int offset = 5; // strlen("Sync ");
                sscanf (&msg[offset], "%d",  &tag);
                syncObserver->SetUpdate(false);

                // Send the sync to all clients.
                GetViewerState()->GetSyncAttributes()->SetSyncTag(tag);
                GetViewerState()->GetSyncAttributes()->Notify();
            }
            else if (strncmp(msg, "updateAOL", 9) == 0)
            {
                ViewerWindowManager::Instance()->UpdateAnnotationObjectList();
            }
            else if (strncmp(msg, "simcmd", 6) == 0)
            {
                DeferredCommandFromSimulation *simCmd = 0;
                int offset = 7;  // = strlen("simcmd ");
                sscanf (&msg[offset], "%p", &simCmd);
                if(simCmd != 0)
                {
                    HandleCommandFromSimulation(simCmd->key, simCmd->db, simCmd->command);
                    delete simCmd;
                }
            }
        }
    }
    CATCHALL(...)
    {
    }
    ENDTRY
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
    if(checkParent != 0)
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
    if(checkParent != 0)
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
//   Brad Whitlock, Tue Dec 21 15:02:28 PST 2004
//   I removed the hasPendingEvents call altogether because it was preventing
//   the connection progress dialog's buttons from updating on their timer.
//   It should be safe to do this because the dialog's timer is constantly
//   generating new events to process.
//
//   Brad Whitlock, Fri May  9 14:50:23 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Thu Oct  9 13:23:27 PDT 2008
//   Don't perform dialog operations if the dialog does not exist.
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
    bool windowsShowing = !ViewerWindowManager::Instance()->GetWindowsHidden() &&
                          dialog != 0;

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
            qApp->processEvents(QEventLoop::AllEvents, 50);
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
//   Mark C. Miller, Tue Jun 15 19:49:22 PDT 2004
//   Added code to test if engine is also in render
//
//   Mark C. Miller, Mon Dec 13 15:59:26 PST 2004
//   Subsumed meaning of InRender in InExecute
//
//   Brad Whitlock, Fri Jan 9 14:50:31 PST 2009
//   Added exception handling code to prevent exceptions from being thrown
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::SendKeepAlives()
{
    TRY
    {
        if(launchingComponent || ViewerEngineManager::Instance()->InExecute())
        {
            // We're launching a component so we don't want to send keep alive
            // signals right now but try again in 20 seconds.
            QTimer::singleShot(20 * 1000, this, SLOT(SendKeepAlives()));
        }
        else
        {
            Status(tr("Sending keep alive signals..."));
            ViewerFileServer::Instance()->SendKeepAlives();
            ViewerEngineManager::Instance()->SendKeepAlives();
            ViewerServerManager::SendKeepAlivesToLaunchers();
            ClearStatus();
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::HandleViewerRPC
//
// Purpose: 
//    This is a Qt slot function that handles RPC's for the
//    ViewerSubject class.
//
// Note:       No exceptions are allowed to be propagated back to the Qt 
//             event loop.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 9 14:56:49 PST 2009
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::HandleViewerRPC()
{
    TRY
    {
        HandleViewerRPCEx();
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
//  Method: ViewerSubject::HandleViewerRPCEx
//
//  Purpose: 
//    Handles RPC's for the ViewerSubject class.
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
//    Eric Brugger, Mon Mar 29 14:25:03 PST 2004
//    I added ToggleMaintainDataModeRPC.
//
//    Kathleen Bonnell, Wed Mar 31 11:08:05 PST 2004 
//    Added methods related to QueryOverTimeAttributes.
//
//    Kathleen Bonnell, Thu Aug  5 08:34:15 PDT 2004 
//    Added ResetLineoutColorRPC.
//
//    Kathleen Bonnell, Wed Aug 18 09:25:33 PDT 2004 
//    Added methods related to InteractorAttributes.
//
//    Mark C. Miller, Tue Mar  8 18:06:19 PST 2005
//    Added GetProcessAttributes 
//
//    Jeremy Meredith, Mon Apr  4 17:33:55 PDT 2005
//    Added SendSimulationCommand.
//
//    Brad Whitlock, Mon Apr 25 18:03:42 PST 2005
//    I made the actionHandled flag be set to true in the case of a CloseRPC
//    so we don't attempt to update any of the actions, which can result in
//    unwanted mdserver launches to update the plot and operator toolbars.
//
//    Hank Childs, Thu May 26 17:51:49 PDT 2005
//    Added export database RPC.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added SetTryHarderCyclesTimesRPC
//
//    Kathleen Bonnell, Wed Jul 27 15:47:34 PDT 2005
//    Added SuppressQueryOutputRPC.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes rpcs 
//
//    Brad Whitlock, Thu Nov 17 17:09:13 PST 2005
//    Added methods rpcs to move and resize windows.
//
//    Brad Whitlock, Thu Jan 5 16:10:44 PST 2006
//    Added code to broadcast the RPC to be executed to all clients so one
//    client can log the actions taken by another.
//
//    Kathleen Bonnell, Tue Jun 20 16:02:38 PDT 2006 
//    Add UpdatePlotInfoAtts. 
//
//    Brad Whitlock, Fri Nov 10 09:39:18 PDT 2006
//    Added ImportEntireStateWithDifferentSourcesRPC.
//
//    Hank Childs, Fri Jan 12 10:02:32 PST 2007
//    If there was a failure from an RPC, then don't call update actions.
//
//    Brad Whitlock, Mon Feb 12 16:35:16 PST 2007
//    Made it use ViewerState.
//
//    Brad Whitlock, Fri Mar 9 16:26:48 PST 2007
//    Added RequestMetaData.
//
//    Brad Whitlock, Tue May 8 16:57:00 PST 2007
//    Added AddInitializedOperator.
//
//    Cyrus Harrison, Tue Sep 18 11:14:37 PDT 2007
//    Added SetQueryFloatFormat()
//
//    Kathleen Bonnell, Tue Oct  9 17:04:58 PDT 2007 
//    Add SetCreateMeshQualityExpressions, SetCreateTimeDerivativeExpressions.
//
//    Cyrus Harrison, Wed Nov 28 12:04:31 PST 2007
//    Add SetCreateVectorMagnitudeExpressions
//
//    Jeremy Meredith, Wed Jan 23 16:32:35 EST 2008
//    Added SetDefaultFileOpenOptionsRPC.
//
//    Jeremy Meredith, Mon Feb  4 13:31:02 EST 2008
//    Added remaining axis array view support.
//
//    Cyrus Harrison, Thu Feb 21 16:12:44 PST 2008
//    Add SetSuppressMessages
//
//    Brad Whitlock, Wed Jan 14 13:58:02 PST 2009
//    I removed UpdatePlotInfoAtts.
//
//    Hank Childs, Wed Jan 28 14:51:03 PST 2009
//    Added support for named selections.
//
// ****************************************************************************

void
ViewerSubject::HandleViewerRPCEx()
{
    //
    // Get a pointer to the active window's action manager.
    //
    bool actionHandled = false;
    ViewerActionManager *actionMgr = 0;

    // Tell the clients that state logging should be turned off. By state
    // logging, I mean all freely exchanged state objects except for the
    // logRPC, which should be logged when received unless it is a
    // SetStateLoggingRPC.
    //
    GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
    GetViewerState()->GetLogRPC()->SetBoolFlag(false);
    BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
    GetViewerState()->GetLogRPC()->CopyAttributes(GetViewerState()->GetViewerRPC());
    BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());

    debug4 << "Handling "
           << ViewerRPC::ViewerRPCType_ToString(GetViewerState()->GetViewerRPC()->GetRPCType()).c_str()
           << " RPC." << endl;

    //
    // Handle the RPC. Note that these should be replaced with actions.
    //
    bool everythingOK = true;
    switch(GetViewerState()->GetViewerRPC()->GetRPCType())
    {
    case ViewerRPC::CloseRPC:
        actionHandled = true;
        Close();
        break;
    case ViewerRPC::OpenDatabaseRPC:
        everythingOK = everythingOK && OpenDatabase();
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
    case ViewerRPC::AddInitializedOperatorRPC:
        AddInitializedOperator();
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
    case ViewerRPC::SetViewAxisArrayRPC:
        SetViewAxisArray();
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
    case ViewerRPC::ToggleMaintainDataModeRPC:
        ToggleMaintainDataMode();
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
    case ViewerRPC::ImportEntireStateWithDifferentSourcesRPC:
        ImportEntireStateWithDifferentSources();
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
    case ViewerRPC::ResetLineoutColorRPC:
        ResetLineoutColor();
        break;
    case ViewerRPC::SetInteractorAttributesRPC:
        SetInteractorAttributes();
        break;
    case ViewerRPC::SetDefaultInteractorAttributesRPC:
        SetDefaultInteractorAttributes();
        break;
    case ViewerRPC::ResetInteractorAttributesRPC:
        ResetInteractorAttributes();
        break;
    case ViewerRPC::GetProcInfoRPC:
        GetProcessAttributes();
        break;
    case ViewerRPC::SendSimulationCommandRPC:
        SendSimulationCommand();
        break;
    case ViewerRPC::UpdateDBPluginInfoRPC:
        UpdateDBPluginInfo();
        break;
    case ViewerRPC::ExportDBRPC:
        ExportDatabase();
        break;
    case ViewerRPC::SetTryHarderCyclesTimesRPC:
        SetTryHarderCyclesTimes();
        break;
    case ViewerRPC::OpenClientRPC:
        OpenClient();
        break;
    case ViewerRPC::SuppressQueryOutputRPC:
        SuppressQueryOutput();
        break;
   case ViewerRPC::SetQueryFloatFormatRPC:
        SetQueryFloatFormat();
        break;
    case ViewerRPC::SetMeshManagementAttributesRPC:
        SetMeshManagementAttributes();
        break;
    case ViewerRPC::SetDefaultMeshManagementAttributesRPC:
        SetDefaultMeshManagementAttributes();
        break;
    case ViewerRPC::ResetMeshManagementAttributesRPC:
        ResetMeshManagementAttributes();
        break;
    case ViewerRPC::MoveWindowRPC:
        MoveWindow();
        break;
    case ViewerRPC::MoveAndResizeWindowRPC:
        MoveAndResizeWindow();
        break;
    case ViewerRPC::ResizeWindowRPC:
        ResizeWindow();
        break;
    case ViewerRPC::ConstructDDFRPC:
        ConstructDDF();
        break;
    case ViewerRPC::RequestMetaDataRPC:
        HandleRequestMetaData();
        break;
    case ViewerRPC::SetTreatAllDBsAsTimeVaryingRPC:
        SetTreatAllDBsAsTimeVarying();
        break;
    case ViewerRPC::SetCreateMeshQualityExpressionsRPC:
        SetCreateMeshQualityExpressions();
        break;
    case ViewerRPC::SetCreateTimeDerivativeExpressionsRPC:
        SetCreateTimeDerivativeExpressions();
        break;
    case ViewerRPC::SetCreateVectorMagnitudeExpressionsRPC:
        SetCreateVectorMagnitudeExpressions();
        break;
    case ViewerRPC::SetDefaultFileOpenOptionsRPC:
        SetDefaultFileOpenOptions();
        break;
    case ViewerRPC::SetSuppressMessagesRPC:
        SetSuppressMessages();
        break;
    case ViewerRPC::ApplyNamedSelectionRPC:
        ApplyNamedSelection();
        break;
    case ViewerRPC::CreateNamedSelectionRPC:
        CreateNamedSelection();
        break;
    case ViewerRPC::DeleteNamedSelectionRPC:
        DeleteNamedSelection();
        break;
    case ViewerRPC::LoadNamedSelectionRPC:
        LoadNamedSelection();
        break;
    case ViewerRPC::SaveNamedSelectionRPC:
        SaveNamedSelection();
        break;
    case ViewerRPC::MaxRPC:
        break;
    default:
        // If an RPC is not handled in the above cases, handle it as
        // an action.
        actionMgr = ViewerWindowManager::Instance()->
            GetActiveWindow()->GetActionManager();
        actionMgr->HandleAction(*GetViewerState()->GetViewerRPC());
        actionHandled = true;
    }

    //
    // We need to do this until all items in the switch statement are
    // removed and converted to actions.
    //

    if (everythingOK && !actionHandled)
        ViewerWindowManager::Instance()->UpdateActions();

    // Tell the clients that it's okay to start logging again.
    GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
    GetViewerState()->GetLogRPC()->SetBoolFlag(true);
    BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());

    debug4 << "Done handling "
           << ViewerRPC::ViewerRPCType_ToString(GetViewerState()->GetViewerRPC()->GetRPCType()).c_str()
           << " RPC." << endl;
}

// ****************************************************************************
// Method: ViewerSubject::PostponeAction
//
// Purpose: 
//   Postpones an action by copying its serialized contents into the
//   xfer object's input connection. This allows us to safely schedule its
//   execution.
//
// Arguments:
//   action : The action to postpone.
//
// Note:       All interactive user actions (popup menu, toolbar) are queued
//             up using this method so we can record which window they came
//             from and schedule them to be executed later when we resume
//             processing input from the client. This makes it safe to allow
//             the controls to be available all the time.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 14 16:27:31 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon May 2 13:51:01 PST 2005
//   I made it use AddInputToXfer.
//
//   Brad Whitlock, Tue Feb 13 14:03:48 PST 2007
//   Made it use ViewerState.
//
// ****************************************************************************

void
ViewerSubject::PostponeAction(ViewerActionBase *action)
{
    //
    // Okay so this is a little weird. We store the action's RPC information
    // into a postponedAction object which we then write into the
    // xfer object's buffered input connection so it is as though
    // the object came directly from the client socket. We write into the
    // buffered input connection because it is guaranteed to contain only
    // whole messages. Also, we write into a postponedAction object because
    // we have to be able to record which window originated the action
    // so we get it right later when we execute it. If we just wrote the
    // action's RPC to the buffer actions would only work for the active window.
    //

    // Store the action information into the postponedAction object.
    GetViewerState()->GetPostponedAction()->SetWindow(
        action->GetWindow()->GetWindowId());
    GetViewerState()->GetPostponedAction()->SetRPC(action->GetArgs());

    // Add the postponed input to the xfer object so it can be executed later.
    AddInputToXfer(0, GetViewerState()->GetPostponedAction());

    debug4 << "Postponing execution of  "
           << action->GetName().c_str()
           << " action." << endl;
}

// ****************************************************************************
// Method: ViewerSubject::HandlePostponedAction
//
// Purpose: 
//   Handles postponed actions that have been queued up in the client input.
//
// Note:       This method executes actions that have been postponed. Note
//             that only ViewerRPC's that are handled in the viewer as
//             actions can be executed through this method because it takes
//             an interactive widget, etc to get an action queued in the
//             first place.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 15 10:52:10 PDT 2005
//
// Modifications:
//   Brad Whitlock, Tue Jan 10 11:37:48 PDT 2006
//   Added code to send RPC's to the clients to ensure that they can be
//   logged. Maybe this should only be done if there's a client that has a
//   MacroRecord client method
//
//   Brad Whitlock, Mon Feb 12 17:04:45 PST 2007
//   Made it use ViewerState.
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::HandlePostponedAction()
{
    TRY
    {
        ViewerWindowManager *wM = ViewerWindowManager::Instance();
        int index = GetViewerState()->GetPostponedAction()->GetWindow();
        ViewerRPC::ViewerRPCType t = GetViewerState()->GetPostponedAction()->
            GetRPC().GetRPCType();
        const char *tName = ViewerRPC::ViewerRPCType_ToString(t).c_str();
        ViewerWindow *win = wM->GetWindow(index);
        if(win != 0)
        {
            ViewerActionManager *actionMgr = win->GetActionManager();

            debug1 << "Handling postponed action "
                   << tName << " for window " << (win->GetWindowId()+1) << "."
                   << endl;

            // Tell the clients that state logging should be turned off. By state
            // logging, I mean all freely exchanged state objects except for the
            // logRPC, which should be logged when received unless it is a
            // SetStateLoggingRPC.
            //
            GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
            GetViewerState()->GetLogRPC()->SetBoolFlag(false);
            BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            // Tell the logging client to log a change to set the window to the
            // window that originated the RPC.
            if(win != wM->GetActiveWindow())
            {
                GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetActiveWindowRPC);
                GetViewerState()->GetLogRPC()->SetWindowId(win->GetWindowId()+1);
                BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            }
            GetViewerState()->GetLogRPC()->CopyAttributes(&GetViewerState()->
                GetPostponedAction()->GetRPC());
            BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());

            // Handle the action.
            actionMgr->HandleAction(GetViewerState()->GetPostponedAction()->GetRPC());

            // Tell the logging client to log a change to set the window back to
            // The current active window.
            if(win != wM->GetActiveWindow())
            {
                GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetActiveWindowRPC);
                GetViewerState()->GetLogRPC()->SetWindowId(wM->GetActiveWindow()->GetWindowId()+1);
                BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            }
            // Tell the clients that it's okay to start logging again.
            GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
            GetViewerState()->GetLogRPC()->SetBoolFlag(true);
            BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
        }
        else
        {
            debug1 << "Could not handle postponed action "
                   << tName << " because its window is gone." << endl;
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
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
//   Brad Whitlock, Thu May 5 17:12:10 PST 2005
//   I made interruption be handled as a special opcode.
//
// ****************************************************************************

void
ViewerSubject::ProcessSpecialOpcodes(int opcode)
{
    ViewerWindowManager *wMgr = ViewerWindowManager::Instance();
    const int interruptOpcode = -1;

    if(opcode == interruptOpcode)
    {
        // Clear the queued input for the master xfer object.
        debug1 << "Interrupt: flushing master xfer input." << endl;
        xfer.GetInputConnection()->Flush();

        // Tell all of the engines to interrupt. If we could ever run
        // more than 1 engine at the same time then we would have to
        // know which engine is executing right now.
        debug1 << "Interrupt: telling engines to interrupt." << endl;
        ViewerEngineManager *eM = ViewerEngineManager::Instance();
        EngineList *engines = eM->GetEngineList();
        const stringVector &hosts = engines->GetEngines();
        const stringVector &sims  = engines->GetSimulationName();
        for(int i = 0; i < hosts.size(); ++i)
            eM->InterruptEngine(EngineKey(hosts[i], sims[i]));
    }
    else if(opcode == animationStopOpcode)
        wMgr->Stop();
    else if(opcode == iconifyOpcode)
        wMgr->IconifyAllWindows();

    // Update actions.
    wMgr->UpdateActions();
}

// ****************************************************************************
// Method: ViewerSubject::BroadcastToAllClients
//
// Purpose: 
//   This is a special callback for the master xfer object to be called when
//   it needs to update. We use this method to tell all of the clients to
//   update themselves.
//
// Arguments:
//   data1 : The callback data. We store the "this" pointer in when we
//           register the callback function.
//   data2 : A pointer to the subject being sent to the client.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 3 15:14:41 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::BroadcastToAllClients(void *data1, Subject *data2)
{
    ViewerSubject *This = (ViewerSubject *)data1;
    if(This != 0)
    {
        AttributeSubject *subj = (AttributeSubject *)data2;
        for(int i = 0; i < This->clients.size(); ++i)
            This->clients[i]->BroadcastToClient(subj);
    }
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
//   Kathleen Bonnell, Thu Aug  5 17:30:06 PDT 2004
//   Defer sending the syncAtts back by creating a MessageRendererThread,
//   at Brad's suggestion to help alleviate synchronization problems found
//   with the CLI (through regression tests).
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::HandleSync()
{
    TRY
    {
        char msg[100];
        SNPRINTF(msg, 100, "Sync %d;", GetViewerState()->GetSyncAttributes()->GetSyncTag());
        MessageRendererThread(msg);
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::HandleClientMethod
//
// Purpose: 
//   The client method object is not normally sent automatically to the
//   clients so we do it here.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 16:48:36 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 16:37:46 PST 2007
//   Made it use ViewerState.
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::HandleClientMethod()
{
    TRY
    {
        if(GetViewerState()->GetClientMethod()->GetMethodName() == "_QueryClientInformation")
        {
            debug1 << "One of the clients is coded such that it sends the"
                      " _QueryClientInformation method back to the viewer. "
                      "We're preventing that situation because it causes an "
                      "infinite loop." << endl;
        }
        else
        {
            debug1 << "Broadcasting client method: "
                   << GetViewerState()->GetClientMethod()->GetMethodName().c_str() << " to all "
                   << clients.size() << " client(s)." << endl;
    
            BroadcastToAllClients((void *)this, GetViewerState()->GetClientMethod());
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::HandleClientInformation
//
// Purpose: 
//   Adds the client information to the list and sends it back to the client.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 19:23:49 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 16:36:28 PST 2007
//   Made it use ViewerState.
//
//   Brad Whitlock, Tue Mar 13 11:42:29 PDT 2007
//   Updated due to code generation changes.
//
//   Brad Whitlock, Mon Jun 11 11:50:45 PDT 2007
//   Added code to send pending interpret commands if we have an interpreter
//   and commands to process.
//
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::HandleClientInformation()
{
    TRY
    {
        debug5 << "Received client information. Sending new client information "
                  "list to all clients." << endl;
        GetViewerState()->GetClientInformationList()->AddClients(
            *GetViewerState()->GetClientInformation());

        // Print the client information list to the debug logs.
        for(int i = 0; i < GetViewerState()->GetClientInformationList()->GetNumClients(); ++i)
        {
            const ClientInformation &client = GetViewerState()->GetClientInformationList()->operator[](i);
            debug3 << "client["<< i << "] = " << client.GetClientName().c_str()
                   << endl;
            debug3 << "methods:" << endl;
            for(int j = 0; j < client.GetMethodNames().size(); ++j)
            {
                debug3 << "\t" << client.GetMethod(j).c_str() << "("
                       << client.GetMethodPrototype(j).c_str() << ")" << endl;
            }
            debug3 << endl;
        }

        BroadcastToAllClients((void *)this, GetViewerState()->GetClientInformationList());

        // If we just received client method information from the cli then try and 
        // interpret any commands that we've stored up.
        if(HasInterpreter() && interpretCommands.size() > 0)
        {
            // Interpret the stored up commands.
            debug4 << "Interpreting stored up commands." << endl;
            InterpretCommands("\n");
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::DiscoverClientInformation
//
// Purpose: 
//   Invokes a special client method on all clients so we know which methods,
//   etc that they have.
//
// Programmer: Brad Whitlock
// Creation:   Thu May 5 19:26:45 PST 2005
//
// Modifications:
//   Brad Whitlock, Mon Feb 12 16:39:49 PST 2007
//   Made it use ViewerState.
//
//   Brad Whitlock, Tue Mar 13 11:42:29 PDT 2007
//   Updated due to code generation changes.
//
//   Brad Whitlock, Fri Jan 9 14:03:58 PST 2009
//   I added code to catch VisItException since we should not allow it to
//   escape from a Qt slot.
//
// ****************************************************************************

void
ViewerSubject::DiscoverClientInformation()
{
    debug1 << "DiscoverClientInformation: clear the client information list "
              "and send _QueryClientInformation to all clients. " << endl;
    TRY
    {
        // Clear out what we know about the clients.
        GetViewerState()->GetClientInformationList()->ClearClients();

        // Ask the current set of clients to tell us about themselves.
        GetViewerState()->GetClientMethod()->SetMethodName("_QueryClientInformation");
        GetViewerState()->GetClientMethod()->ClearArgs();
        BroadcastToAllClients((void *)this, GetViewerState()->GetClientMethod());
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
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
//    Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//    Added exception handling to make sure that exceptions do not escape
//    back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::ConnectWindow(ViewerWindow *win)
{
    TRY
    {
        win->GetActionManager()->EnableActions(ViewerWindowManager::Instance()->GetWindowAtts());
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
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


// ****************************************************************************
// Method: ViewerSubject::ResetLineoutColor
//
// Purpose: 
//   Resets color used by lineout to default values. 
//
// Programmer: Kathleen Bonnell 
// Creation:   August 5, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetLineoutColor()
{
    ViewerQueryManager::Instance()->ResetLineoutColor(); 
}


// ****************************************************************************
//  Method: ViewerSubject::SetInteractorAttributes
//
//  Purpose:
//    Execute the SetInteractorAttributes RPC.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 16, 2004 
//
//  Modifications:
//
// ****************************************************************************

void
ViewerSubject::SetInteractorAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetInteractorAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::SetDefaultInteractorAttributes
//
// Purpose: 
//   Sets the default interactor atts from the client interactor atts.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetDefaultInteractorAttributes()
{
    ViewerWindowManager::SetDefaultInteractorAttsFromClient();
}

// ****************************************************************************
// Method: ViewerSubject::ResetInteractorAttributes
//
// Purpose: 
//   Sets the default interactor attributes into the interactor attributes
//   for the active window.
//
// Programmer: Kathleen Bonnell 
// Creation:   August 16, 2004
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResetInteractorAttributes()
{
    ViewerWindowManager *wM=ViewerWindowManager::Instance();
    wM->SetInteractorAttsFromDefault();
}

// ****************************************************************************
// Method: ViewerSubject::GetProcessAttributes
//
// Purpose: Gets unix process information
//
// Programmer: Mark C. Miller
// Creation:   Tuesday, January 18, 2004 
// 
// Modifications:
//   Brad Whitlock, Tue May 10 16:36:54 PST 2005
//   Made it work on Win32.
//
//   Brad Whitlock, Mon Feb 12 12:11:16 PDT 2007
//   Made it use ViewerState.
//
// ****************************************************************************

void
ViewerSubject::GetProcessAttributes()
{
    ProcessAttributes tmpAtts;

    string componentName = VisItInit::ComponentIDToName(GetViewerState()->GetViewerRPC()->GetIntArg1());
    if (componentName == "engine")
    {
        const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
        const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

        ViewerEngineManager *vem = ViewerEngineManager::Instance();
        vem->GetProcInfo(EngineKey(hostName, simName), tmpAtts);
    }
    else if (componentName == "viewer")
    {
#if defined(_WIN32)
        int pid = _getpid();
        int ppid = -1;
#else
        int pid = getpid();
        int ppid = getppid();
#endif
        char myHost[256];
        gethostname(myHost, sizeof(myHost));

        std::vector<double> tmpPids;
        tmpPids.push_back(pid);

        std::vector<double> tmpPpids;
        tmpPpids.push_back(ppid);

        std::vector<string> tmpHosts;
        tmpHosts.push_back(myHost);

        tmpAtts.SetPids(tmpPids);
        tmpAtts.SetPpids(tmpPpids);
        tmpAtts.SetHosts(tmpHosts);
        tmpAtts.SetIsParallel(false); // would be better to check for threads
    }
    else
    {
        Warning(tr("Currently, GetProcessAttributes() works only for "
                "\"engine\" or \"viewer\""));
        return;
    }

    *GetViewerState()->GetProcessAttributes() = tmpAtts;
    GetViewerState()->GetProcessAttributes()->SelectAll();
    GetViewerState()->GetProcessAttributes()->Notify();
}

// ****************************************************************************
// Method: ViewerSubject::OpenClient
//
// Purpose: 
//   Handles the ViewerRPC that tells the viewer to reverse launch a new
//   viewer client.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 11:36:45 PDT 2005
//
// Modifications:
//   Brad Whitlock, Wed Apr 30 09:27:08 PDT 2008
//   Support for internationalization.
//
// ****************************************************************************

void
ViewerSubject::OpenClient()
{
    const std::string &clientName = GetViewerState()->GetViewerRPC()->GetDatabase();
    const std::string &program = GetViewerState()->GetViewerRPC()->GetProgramHost();
    stringVector programOptions(GetViewerState()->GetViewerRPC()->GetProgramOptions());

    debug1 << "ViewerSubject::OpenClient" << endl;

    //
    // Create a new viewer client connection.
    //
    ViewerClientConnection *newClient = new 
        ViewerClientConnection(GetViewerState(), this, clientName.c_str());
    newClient->SetupSpecialOpcodeHandler(SpecialOpcodeCallback, (void *)this);
    ViewerConnectionProgressDialog *dialog = 0;

    TRY
    {
        if(!GetViewerProperties()->GetNowin())
        {
            dialog = new ViewerConnectionProgressDialog(clientName.c_str(),
                "localhost", false, 5);

            // Register the dialog with the password window so we can set
            // the dialog's timeout to zero if we have to prompt for a
            // password.
            ViewerPasswordWindow::SetConnectionProgressDialog(dialog);
        }

        //
        // Try launch the client (unless a different client will) and start
        // listening for a connection from the new client.
        //
        void *cbData[2];
        cbData[0] = (void *)this;
        cbData[1] = (void *)dialog;
        stringVector args(clientArguments);
        for(int i = 0; i < programOptions.size(); ++i)
            args.push_back(programOptions[i]);
        newClient->LaunchClient(program, args, 0, 0, LaunchProgressCB, cbData);
        clients.push_back(newClient);

        // Connect up the new client so we can handle its signals.
        connect(newClient, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
                this,      SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
        connect(newClient, SIGNAL(DisconnectClient(ViewerClientConnection *)),
                this,      SLOT(DisconnectClient(ViewerClientConnection *)));

        Message(tr("Added a new client to the viewer."));

        // Discover the client's information.
        QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));
    }
    CATCH(VisItException)
    {
        delete newClient;

        QString msg = tr("VisIt could not connect to the new client %1.").
                      arg(program.c_str());
        Error(msg);
    }
    ENDTRY

    delete dialog;
}

// ****************************************************************************
//  Method:  ViewerSubject::ReadFromSimulationAndProcess
//
//  Purpose:
//    Callback from QSocketNotifier connected to the write socket
//    from a simulation-engine.  Reads what the simulation is sending
//    and processes it.
//
//  Arguments:
//    socket     the write socket for the notification
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Brad Whitlock, Thu Jan 25 14:20:24 PST 2007
//    Added engineCommandObserver.
//
//    Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//    Added exception handling to make sure that exceptions do not escape
//    back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::ReadFromSimulationAndProcess(int socket)
{
    if (simulationSocketToKey.count(socket) <= 0)
        return;

    ViewerEngineManager *vem = ViewerEngineManager::Instance();

    EngineKey ek = simulationSocketToKey[socket];

    TRY
    {
       vem->ReadDataAndProcess(ek);
    }
    CATCH(LostConnectionException)
    {
        ViewerWindowManager::Instance()->ResetNetworkIds(ek);
        ViewerEngineManager::Instance()->CloseEngine(ek);
        delete engineMetaDataObserver[ek];
        delete engineSILAttsObserver[ek];
        delete engineCommandObserver[ek];
        delete engineKeyToNotifier[ek];
        engineMetaDataObserver.erase(ek);
        engineSILAttsObserver.erase(ek);
        engineCommandObserver.erase(ek);
        engineKeyToNotifier.erase(ek);
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::HandleMetaDataUpdated
//
//  Purpose:
//    This is the callback for when the metadata is updated by a simulation.
//    The only task it really needs to do is poke the new one into the
//    ViewerFileServer and send the new one on to other observers (i.e.
//    the gui).
//
//  Arguments:
//    host,file     the attributes for the simulation
//    md            the new, updated metadata
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon Apr  4 17:34:17 PDT 2005
//    Made sure we notify clients about the right metadata.
//
//    Brad Whitlock, Mon Feb 12 17:00:19 PST 2007
//    Made it use ViewerState.
//
//    Hank Childs, Fri Oct 26 08:40:01 PDT 2007
//    Update the expression list.  Expressions from the sim won't work without
//    this.
//
//    Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//    Added exception handling to make sure that exceptions do not escape
//    back into the Qt event loop.
//   
// ****************************************************************************

void
ViewerSubject::HandleMetaDataUpdated(const string &host,
                                     const string &file,
                                     const avtDatabaseMetaData *md)
{
    TRY
    {
        // Handle MetaData updates
        ViewerFileServer *fs = ViewerFileServer::Instance();

        *GetViewerState()->GetDatabaseMetaData() = *md;
        fs->SetSimulationMetaData(host, file, *GetViewerState()->GetDatabaseMetaData());
        // The file server will modify the metadata slightly; make sure
        // we picked up the new one.
        *GetViewerState()->GetDatabaseMetaData() = *fs->GetMetaData(host, file);
        ViewerWindowManager *wM=ViewerWindowManager::Instance();
        ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();
        plotList->UpdateExpressionList(false);
        GetViewerState()->GetDatabaseMetaData()->SelectAll();
        GetViewerState()->GetDatabaseMetaData()->Notify();
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::HandleSILAttsUpdated
//
//  Purpose:
//    This is the callback for when the SIL atts are updated by a simulation.
//    The only task it really needs to do is poke the new one into the
//    ViewerFileServer and send the new one on to other observers (i.e.
//    the gui).
//
//  Arguments:
//    host,file     the attributes for the simulation
//    sa            the new, updated SILAttributes
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 25, 2004
//
//  Modifications:
//    Brad Whitlock, Mon Feb 12 17:01:12 PST 2007
//    I made it use ViewerState.
//
//    Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//    Added exception handling to make sure that exceptions do not escape
//    back into the Qt event loop.
//
// ****************************************************************************

void
ViewerSubject::HandleSILAttsUpdated(const string &host,
                                    const string &file,
                                    const SILAttributes *sa)
{
    TRY
    {
        // Handle SIL updates
        ViewerFileServer *fs = ViewerFileServer::Instance();

        *GetViewerState()->GetSILAttributes() = *sa;
        fs->SetSimulationSILAtts(host, file, *GetViewerState()->GetSILAttributes());
        GetViewerState()->GetSILAttributes()->SelectAll();
        GetViewerState()->GetSILAttributes()->Notify();
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::HandleColorTable
//
// Purpose: 
//   This is a Qt slot function that is called when the color table attributes
//   update. The purpose is to update all color table buttons so they have the
//   right list of color table names.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 20 11:56:19 PDT 2007
//
// Modifications:
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//   
// ****************************************************************************

void
ViewerSubject::HandleColorTable()
{
    TRY
    {
        ColorTableAttributes *colorAtts = GetViewerState()->GetColorTableAttributes();

        // For when we have pixmaps later, here are the rules...
        // 1. if just #0 is ever selected, we're adding/deleting colortables.
        // 2. If just #1 is selected , we're modifying a color table.
        // 3. If just #2 is selected, we're changing the default colormap. This is
        //    only of concern if I decide to show what the default is in a widget.

        // If the names or the color table attributes are changing, then we
        // have to update the widget.
        if(colorAtts->IsSelected(0) || colorAtts->IsSelected(1))
        {
            // Clear all of the color tables.
            QvisColorTableButton::clearAllColorTables();

            int nNames = colorAtts->GetNames().size();
            const stringVector &names = colorAtts->GetNames();
            for(int i = 0; i < nNames; ++i)
            {
                QvisColorTableButton::addColorTable(names[i].c_str());
            }

            // Update all of the QvisColorTableButton widgets.
            QvisColorTableButton::updateColorTableButtons();
        }
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::DeferCommandFromSimulation
//
// Purpose: 
//   This method is called when we get a command from a simulation. We save it
//   for later when we can call it from the top of the event loop. This is done
//   for safety since it should prevent us from trying to service more engine
//   RPCs while we may already be waiting on a blocking RPC.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 24 09:40:58 PST 2008
//
// Modifications:
//   Brad Whitlock, Fri Jan 9 14:25:04 PST 2009
//   Added exception handling to make sure that exceptions do not escape
//   back into the Qt event loop.
//   
// ****************************************************************************

void
ViewerSubject::DeferCommandFromSimulation(const EngineKey &key, 
    const std::string &db, const std::string &command)
{
    TRY
    {
        debug1 << "DeferCommandFromSimulation: key=" << key.ID().c_str()
               << ", db=" << db.c_str() << ", command=\"" << command.c_str() << "\""
               << endl;

        // Save the arguments for later.
        DeferredCommandFromSimulation *simCmd = new DeferredCommandFromSimulation;
        simCmd->key = key;
        simCmd->db = db;
        simCmd->command = command;

        // Send a message to process the simulation command from the top level
        // of the event loop.
        char msg[200];
        SNPRINTF(msg, 200, "simcmd %p;", (void*)simCmd);
        MessageRendererThread(msg);
    }
    CATCHALL(...)
    {
        ; // nothing
    }
    ENDTRY
}

// ****************************************************************************
// Method: ViewerSubject::HandleCommandFromSimulation
//
// Purpose: 
//   This method gets called when we need to handle commands that come from
//   the simulation.
//
// Arguments:
//   key     : The engine key.
//   db      : The database that was open.
//   command : The command to execute.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 25 14:50:19 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Jun 11 11:55:03 PDT 2007
//   Made it use the InterpretCommands method.
//
//   Brad Whitlock, Thu Feb 26 13:57:58 PST 2009
//   I added Message and Error commands from the simulation.
//
//   Brad Whitlock, Fri Mar 27 11:27:40 PDT 2009
//   I added INTERNALSYNC so we can sync with the simulation.
//
//   Cyrus Harrison, Tue Apr 14 13:35:54 PDT 2009
//   Changed the interface to ReplaceDatabase to support option for only 
//   replacing active plots.
//
// ****************************************************************************

void
ViewerSubject::HandleCommandFromSimulation(const EngineKey &key, 
    const std::string &db, const std::string &command)
{
    debug1 << "HandleCommandFromSimulation: key=" << key.ID().c_str()
           << ", db=" << db.c_str() << ", command=\"" << command.c_str() << "\""
           << endl;

    if(command == "UpdatePlots")
    {
        // The simulation told us that it wants us to update all of the plots 
        // that use it as a source.
        ViewerWindowManager::Instance()->ReplaceDatabase(key, db, 0, false, true,
                                                         false);
    }
    else if(command.substr(0,8) == "Message:")
    {
        Message(command.substr(8,command.size()-8).c_str());
    }
    else if(command.substr(0,6) == "Error:")
    {
        Error(command.substr(6,command.size()-6).c_str());
    }
    else if(command.substr(0,10) == "Interpret:")
    {
        InterpretCommands(command.substr(10, command.size()-10));
    }
    else if(command.substr(0,12) == "INTERNALSYNC")
    {
        // Send the command back to the engine so it knows we're done syncing.
        std::string cmd("INTERNALSYNC");
        std::string args(command.substr(13, command.size()-1));
        ViewerEngineManager::Instance()->SendSimulationCommand(key, cmd, args);
    }
}

// ****************************************************************************
// Method: ViewerSubject::HasInterpreter
//
// Purpose: 
//   Scans the client information list and looks for a client that has an
//   Interpret method.
//
// Returns:    True if there is a client with an interpreter; False otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 11:54:16 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

bool
ViewerSubject::HasInterpreter() const
{
    // Let's make sure that there's an Interpret client first.
    bool hasInterpreter = false;
    for(int i = 0; 
        i < GetViewerState()->GetClientInformationList()->GetNumClients() &&
        !hasInterpreter;
        ++i)
    {
        const ClientInformation &client = GetViewerState()->GetClientInformationList()->GetClients(i);
        for(int j = 0; j < client.GetMethodNames().size() && !hasInterpreter; ++j)
            hasInterpreter = client.GetMethod(j) == "Interpret" &&
                             client.GetMethodPrototype(j) == "s";
    }

    return hasInterpreter;
}

// ****************************************************************************
// Method: ViewerSubject::InterpretCommands
//
// Purpose: 
//   Allows the viewer to interpret CLI commands, launching a CLI if one does
//   not exist.
//
// Arguments:
//   commands : The command string to execute. More than one line can be
//              included by using end of line characters.
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 11:51:32 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::InterpretCommands(const std::string &commands)
{
    const char *mName = "ViewerSubject::InterpretCommands: ";

    // Make sure there's an end of line.
    std::string cmd(commands);
    if(cmd.size() > 0 && cmd[cmd.size()-1] != '\n')
        cmd += "\n";

    // Print what we're trying to execute.
    debug4 << mName << "commands = \"" << cmd.c_str() << "\"" << endl;

    // Let's make sure that there's an Interpret client first.
    if(!HasInterpreter())
    {
        // We don't have an interpreter and there are no sim commands, meaning
        // that we have not yet tried to launch an interpreting client while
        // processing commands. That means we need to launch a client.
        if(interpretCommands.size() == 0)
        {
            // Add to the sim commands. NOTE that this needs to happen before
            // the client is launched or we can get into InterpretCommands 
            // again if another command is send for interpretation while the
            // client is getting launched.
            interpretCommands += cmd;

            debug4 << mName << "The simulation wants to interpret some "
                "commands but there's no interpreter. Start one." << endl;
            stringVector clientArgs;
            clientArgs.push_back("-cli");
            clientArgs.push_back("-newconsole");
            GetViewerMethods()->OpenClient("CLI", "visit", clientArgs);
        }
        else
        {
            // Add to the sim commands.
            interpretCommands += cmd;
        }
    }
    else
    {
        interpretCommands += cmd;

        debug4 << mName << "Telling client to interpret!" << endl;

        // The simulation told us to interpret some python code.
        stringVector args;
        args.push_back(interpretCommands);
        GetViewerState()->GetClientMethod()->SetMethodName("Interpret");
        GetViewerState()->GetClientMethod()->ClearArgs();
        GetViewerState()->GetClientMethod()->SetStringArgs(args);
        BroadcastToAllClients((void *)this, GetViewerState()->GetClientMethod());

        // Clear out the command buffer.
        interpretCommands.clear();
    }
}

// ****************************************************************************
//  Method:  ViewerSubject::SetTryHarderCyclesTimes
//
//  Purpose: Handle a SetTryHarderCyclesTimes RPC
//
//  Programmer:  Mark C. Miller 
//  Creation:    May 25, 2005 
//
//  Modifications:
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Moved storage for this state info to VWM which is managing
//    GlobalAttributes.
// ****************************************************************************

void
ViewerSubject::SetTryHarderCyclesTimes()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetTryHarderCyclesTimes(GetViewerState()->GetViewerRPC()->GetIntArg1());
}


// ****************************************************************************
//  Method:  ViewerSubject::SetTreatAllDBsAsTimeVarying
//
//  Purpose: Handle a SetTreatAllDBsAsTimeVarying RPC
//
//  Programmer:  Mark C. Miller 
//  Creation:    June 11, 2007 
//
//  Modifications:
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Moved storage for this state info to VWM which is managing
//    GlobalAttributes.
// ****************************************************************************

void
ViewerSubject::SetTreatAllDBsAsTimeVarying()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetTreatAllDBsAsTimeVarying(GetViewerState()->GetViewerRPC()->GetIntArg1());
}


// ****************************************************************************
// Method: ViewerSubject::SuppressQueryOutput
//
// Purpose: 
//   Turns on/off printing of query output. 
//
// Programmer: Kathleen Bonnell 
// Creation:   July 27, 2005 
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SuppressQueryOutput()
{
    ViewerQueryManager::Instance()->
        SuppressQueryOutput(GetViewerState()->GetViewerRPC()->GetBoolFlag());
}

// ****************************************************************************
// Method: ViewerSubject::SetQueryFloatFormat
//
// Purpose: 
//   Sets the floating point format string used for queries.
//
// Programmer: Cyrus Harrison
// Creation:   September 18, 2007
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::SetQueryFloatFormat()
{
    ViewerQueryManager::Instance()->
        SetQueryFloatFormat(GetViewerState()->GetViewerRPC()->GetStringArg1());
}


// ****************************************************************************
// Method: ViewerSubject::MoveWindow
//
// Purpose: 
//   Moves a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:10:57 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::MoveWindow()
{
    ViewerWindowManager::Instance()->MoveWindow(
        GetViewerState()->GetViewerRPC()->GetWindowId()-1,
        GetViewerState()->GetViewerRPC()->GetIntArg1(),
        GetViewerState()->GetViewerRPC()->GetIntArg2());
}

// ****************************************************************************
// Method: ViewerSubject::MoveAndResizeWindow
//
// Purpose: 
//   Moves and resizes a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:11:08 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::MoveAndResizeWindow()
{
    ViewerWindowManager::Instance()->MoveAndResizeWindow(
        GetViewerState()->GetViewerRPC()->GetWindowId()-1,
        GetViewerState()->GetViewerRPC()->GetIntArg1(),
        GetViewerState()->GetViewerRPC()->GetIntArg2(),
        GetViewerState()->GetViewerRPC()->GetIntArg3(),
        GetViewerState()->GetViewerRPC()->GetWindowLayout());
}

// ****************************************************************************
// Method: ViewerSubject::ResizeWindow
//
// Purpose: 
//   Resizes a window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 17 17:11:26 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ResizeWindow()
{
    ViewerWindowManager::Instance()->ResizeWindow(
        GetViewerState()->GetViewerRPC()->GetWindowId()-1,
        GetViewerState()->GetViewerRPC()->GetIntArg1(),
        GetViewerState()->GetViewerRPC()->GetIntArg2());
}

// ****************************************************************************
//  Method:  ViewerSubject::SetCreateMeshQualityExpressions
//
//  Purpose: Handle a SetCreateMeshQualityExpressions RPC
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 9, 2007 
//
//  Modifications:
// ****************************************************************************

void
ViewerSubject::SetCreateMeshQualityExpressions()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetCreateMeshQualityExpressions(
        GetViewerState()->GetViewerRPC()->GetIntArg1());
}


// ****************************************************************************
//  Method:  ViewerSubject::SetCreateTimeDerivativeExpressions
//
//  Purpose: Handle a SetCreateTimeDerivativeExpressions RPC
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    October 9, 2007 
//
//  Modifications:
// ****************************************************************************

void
ViewerSubject::SetCreateTimeDerivativeExpressions()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetCreateTimeDerivativeExpressions(
        GetViewerState()->GetViewerRPC()->GetIntArg1());
}

// ****************************************************************************
//  Method:  ViewerSubject::SetCreateVectorMagnitudeExpressions
//
//  Purpose: Handle a SetCreateVectorMagnitudeExpression RPC
//
//  Programmer:  Cyrus Harrison
//  Creation:    November 28, 2007 
//
// ****************************************************************************

void
ViewerSubject::SetCreateVectorMagnitudeExpressions()
{
    ViewerWindowManager *wM = ViewerWindowManager::Instance();
    wM->SetCreateVectorMagnitudeExpressions(
        GetViewerState()->GetViewerRPC()->GetIntArg1());
}

// ****************************************************************************
//  Method:  ViewerSubject::ApplyNamedSelection()
//
//  Purpose: Handle a ApplyNamedSelection RPC
//
//  Programmer:  Hank Childs
//  Creation:    January 28, 2009
//
// ****************************************************************************

void
ViewerSubject::ApplyNamedSelection()
{
    int   i;

    std::string selName = GetViewerState()->GetViewerRPC()->GetStringArg1();

    //
    // Perform the RPC.
    //
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIDs;
    plist->GetActivePlotIDs(plotIDs, false);
    if (plotIDs.size() <= 0)
    {
        Error(tr("To apply a named selection, you must have an active"
                 " plot.  No named selection was applied."));
        return;
    }
    std::vector<std::string> plotNames;
    for (i = 0 ; i < plotIDs.size() ; i++)
    {
        plotNames.push_back(plist->GetPlot(plotIDs[i])->GetPlotName());
    }
    ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
    int networkId = plot->GetNetworkID();
    const EngineKey   &engineKey = plot->GetEngineKey();
    for (int i = 1 ; i < plotIDs.size() ; i++)
    {
        ViewerPlot *plot = plist->GetPlot(plotIDs[i]);
        const EngineKey   &engineKey2 = plot->GetEngineKey();
        if (engineKey2 != engineKey)
        {
            Error(tr("All plots involving a named selection must come from"
                 " the same engine.  No named selection was applied."));
        }
    }
        
    TRY
    {
        if (ViewerEngineManager::Instance()->ApplyNamedSelection(engineKey, 
                                                      plotNames, selName))
        {
            // Only force a redraw if we have active, non-hidden, realized
            // plots.
            vector<int> plotIDs2;
            plist->GetActivePlotIDs(plotIDs2, true);
            if (plotIDs2.size() > 0)
            {
                GetViewerMethods()->ClearWindow(false);
                GetViewerMethods()->DrawPlots(false);
            }
            Message(tr("Applied named selection"));
        }
        else
        {
            Error(tr("Unable to apply named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::CreateNamedSelection()
//
//  Purpose: Handle a CreateNamedSelection RPC
//
//  Programmer:  Hank Childs
//  Creation:    January 28, 2009
//
// ****************************************************************************

void
ViewerSubject::CreateNamedSelection()
{
    std::string selName = GetViewerState()->GetViewerRPC()->GetStringArg1();

    //
    // Perform the RPC.
    //
    ViewerWindow *win = ViewerWindowManager::Instance()->GetActiveWindow();
    ViewerPlotList *plist = win->GetPlotList();
    intVector plotIDs;
    plist->GetActivePlotIDs(plotIDs);
    if (plotIDs.size() <= 0)
    {
        Error(tr("To create a named selection, you must have an active"
                 " plot.  No named selection was created."));
        return;
    }
    if (plotIDs.size() > 1)
    {
        Error(tr("You can only have one active plot when creating a named"
                 " selection.  No named selection was created."));
        return;
    }
    ViewerPlot *plot = plist->GetPlot(plotIDs[0]);
    int networkId = plot->GetNetworkID();
    const EngineKey   &engineKey = plot->GetEngineKey();
        
    TRY
    {
        if (ViewerEngineManager::Instance()->CreateNamedSelection(engineKey, 
                                                      networkId, selName))
        {
            Message(tr("Created named selection"));
        }
        else
        {
            Error(tr("Unable to create named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::DeleteNamedSelection()
//
//  Purpose: Handle a DeleteNamedSelection RPC
//
//  Programmer:  Hank Childs
//  Creation:    January 28, 2009
//
// ****************************************************************************

void
ViewerSubject::DeleteNamedSelection()
{
    //
    // Get the rpc arguments.
    //
    std::string selName = GetViewerState()->GetViewerRPC()->GetStringArg1();

    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

    EngineKey engineKey(hostName, simName);

    //
    // Perform the RPC.
    //
    TRY
    {
        if (ViewerEngineManager::Instance()->DeleteNamedSelection(engineKey, selName))
        {
            Message(tr("Deleted named selection"));
        }
        else
        {
            Error(tr("Unable to delete named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::LoadNamedSelection()
//
//  Purpose: Handle a LoadNamedSelection RPC
//
//  Programmer:  Hank Childs
//  Creation:    January 28, 2009
//
// ****************************************************************************

void
ViewerSubject::LoadNamedSelection()
{
    //
    // Get the rpc arguments.
    //
    std::string selName = GetViewerState()->GetViewerRPC()->GetStringArg1();

    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

    EngineKey engineKey(hostName, simName);

    //
    // Perform the RPC.
    //
    TRY
    {
        if (ViewerEngineManager::Instance()->LoadNamedSelection(engineKey, selName))
        {
            Message(tr("Loaded named selection"));
        }
        else
        {
            Error(tr("Unable to load named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::SaveNamedSelection()
//
//  Purpose: Handle a SaveNamedSelection RPC
//
//  Programmer:  Hank Childs
//  Creation:    January 28, 2009
//
// ****************************************************************************

void
ViewerSubject::SaveNamedSelection()
{
    //
    // Get the rpc arguments.
    //
    std::string selName = GetViewerState()->GetViewerRPC()->GetStringArg1();

    const std::string &hostName = GetViewerState()->GetViewerRPC()->GetProgramHost();
    const std::string &simName  = GetViewerState()->GetViewerRPC()->GetProgramSim();

    EngineKey engineKey(hostName, simName);

    //
    // Perform the RPC.
    //
    TRY
    {
        if (ViewerEngineManager::Instance()->SaveNamedSelection(engineKey, selName))
        {
            Message(tr("Saved named selection"));
        }
        else
        {
            Error(tr("Unable to save named selection"));
        }
    }
    CATCH2(VisItException, e)
    {
        char message[1024];
        SNPRINTF(message, 1024, "(%s): %s\n", e.GetExceptionType().c_str(),
                                             e.Message().c_str());
        Error(message);
    }
    ENDTRY
}


// ****************************************************************************
//  Method:  ViewerSubject::SetSuppressMessages()
//
//  Purpose: Handle a SetSuppressMessages RPC
//
//  Programmer:  Cyrus Harrison
//  Creation:    February  21, 2008
//
// ****************************************************************************

void
ViewerSubject::SetSuppressMessages()
{
    bool value = (bool) GetViewerState()->GetViewerRPC()->GetIntArg1();
    if(value)
        EnableMessageSuppression();
    else
        DisableMessageSuppression();
}


// ****************************************************************************
//  Method:  ViewerSubject::SetDefaultFileOpenOptions
//
//  Purpose:
//    Makes the current state file open options the default for all
//    future opening actions by broadcasting them to the existing
//    metadata servers and engines, and having the file server and
//    engine manager keep track of them and send them to new engines/
//    mdservers.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************

void
ViewerSubject::SetDefaultFileOpenOptions()
{
    ViewerFileServer *fs = ViewerFileServer::Instance();
    fs->BroadcastUpdatedFileOpenOptions();
    ViewerEngineManager *em = ViewerEngineManager::Instance();
    em->UpdateDefaultFileOpenOptions(fs->GetFileOpenOptions());
}

// ****************************************************************************
// Method: ViewerSubject::GetNowinMode
//
// Purpose: 
//   Get the nowin mode.
//
// Returns:    The current nowin mode.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:16:39 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr 14 11:28:20 PDT 2009
//   Use ViewerProperties.
//   
// ****************************************************************************

bool
ViewerSubject::GetNowinMode() const
{
    return GetViewerProperties()->GetNowin();
}

// ****************************************************************************
// Method: ViewerSubject::SetNowinMode
//
// Purpose: 
//   Set the nowin mode.
//
// Arguments:
//   value : The new nowin value.
//
// Note:       This method only has an effect before the Initialize method
//             has been called.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 15:16:58 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Apr 14 11:28:20 PDT 2009
//   Use ViewerProperties.
//
// ****************************************************************************

void
ViewerSubject::SetNowinMode(bool value)
{
    GetViewerProperties()->SetNowin(value);

    avtCallback::SetSoftwareRendering(value);
    avtCallback::SetNowinMode(value);
}
