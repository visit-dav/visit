/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
#include <stdio.h>
#include <snprintf.h>
#include <ViewerSubject.h>

#include <QDesktopWidget>
#include <QFile>
#include <QLocale>
#include <QTimer>
#include <QTranslator>

#include <AnimationAttributes.h>
#include <AnnotationAttributes.h>
#include <AnnotationObject.h>
#include <AnnotationObjectList.h>
#include <Appearance.h>
#include <AppearanceAttributes.h>
#include <AxisRestrictionAttributes.h>
#include <ClientMethod.h>
#include <ClientInformation.h>
#include <ClientInformationList.h>
#include <ColorControlPoint.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <DatabaseCorrelation.h>
#include <DatabaseCorrelationList.h>
#include <DBPluginInfoAttributes.h>
#include <EngineKey.h>
#include <EngineList.h>
#include <ExportDBAttributes.h>
#include <FileOpenOptions.h>
#include <FileFunctions.h>
#include <GlobalAttributes.h>
#include <GlobalLineoutAttributes.h>
#include <MachineProfile.h>
#include <HostProfileList.h>
#include <VisItInit.h>
#include <InitVTKRendering.h>
#include <InstallationFunctions.h>
#include <InteractorAttributes.h>
#include <InvalidVariableException.h>
#include <KeyframeAttributes.h>
#include <LostConnectionException.h>
#include <MaterialAttributes.h>
#include <MeshManagementAttributes.h>
#include <MessageAttributes.h>
#include <MovieAttributes.h>
#include <OperatorPluginInfo.h>
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
#include <SelectionList.h>
#include <SelectionProperties.h>
#include <SelectionSummary.h>
#include <SILRestrictionAttributes.h>
#include <SimulationCommand.h>
#include <SimulationUIValues.h>
#include <SingleAttributeConfigManager.h>
#include <SocketConnection.h>
#include <StatusAttributes.h>
#include <StringHelpers.h>
#include <SyncAttributes.h>
#include <QueryOverTimeAttributes.h>
#include <Utility.h>
#include <VariableMenuPopulator.h>
#include <ViewerRPC.h>
#include <Xfer.h>

#include <DatabaseActions.h>
#include <ViewerActionLogic.h>
#include <ViewerActionManager.h>
#include <ViewerConnectionProgress.h>
#include <ParsingExprList.h>
#include <ViewerClientConnection.h>
#include <ViewerCommandFromSimObserver.h>
#include <ViewerConfigManager.h>
#include <ViewerConnectionPrinterUI.h>
#include <ViewerEngineManagerInterface.h>
#include <ViewerFactoryMain.h>
#include <ViewerFactoryUI.h>
#include <ViewerFileServerInterface.h>
#include <ViewerInternalCommands.h>
#include <ViewerMessaging.h>
#include <ViewerMetaDataObserver.h>
#include <ViewerMethods.h>
#include <ViewerObserverToSignal.h>
#include <ViewerOperatorFactory.h>
#include <ViewerPasswordWindow.h>
#include <ViewerChangeUsernameUI.h>
#include <ViewerPlot.h>
#include <ViewerPlotFactory.h>
#include <ViewerPlotList.h>
#include <ViewerPopupMenu.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerServerManager.h>
#include <ViewerState.h>
#include <ViewerStateManager.h>
#include <ViewerStateBuffered.h>
#include <ViewerSILAttsObserver.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerWindowManagerAttributes.h>
#include <VisWinRendering.h>
#include <VisWinRenderingWithoutWindowWithInteractions.h> //remove after fix?
#include <WindowInformation.h>
#include <PlotPluginManager.h>
#include <OperatorPluginManager.h>
#include <avtImageFileWriter.h>
#include <ViewerClientInformation.h>
#include <ViewerClientInformationElement.h>

#include <QApplication>
#include <QSocketNotifier>
#include <QvisColorTableButton.h>
#include <QvisNoDefaultColorTableButton.h>

#include <DebugStream.h>
#include <ParentProcess.h>
#include <TimingsManager.h>
#include <WindowMetrics.h>

#include <avtCallback.h>
#include <avtColorTables.h>
#include <avtDatabaseMetaData.h>
#include <avtSimulationInformation.h>
#include <avtSimulationCommandSpecification.h>
#include <SharedDaemon.h>
#include <MDServerManager.h>

#ifdef HAVE_DDT
#include <DDTManager.h>
#endif

#if !defined(_WIN32)
#include <strings.h>
#include <unistd.h>
#else
#include <process.h>
static int nConfigArgs = 1;
#endif

#include <algorithm>

#include <visit-config.h>
#ifdef HAVE_OSMESA
#include <vtkVisItOSMesaRenderingFactory.h>
#endif

// ****************************************************************************
// Class: ViewerCommandDeferredCommandFromSimulation
//
// Purpose:
//   Handles deferred commands from a simulation.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:59:13 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerCommandDeferredCommandFromSimulation : public ViewerInternalCommand
{
public:
    ViewerCommandDeferredCommandFromSimulation(ViewerSubject *vs,
                                               EngineKey a,
                                               const std::string &b,
                                               const std::string &c) : 
        ViewerInternalCommand(), viewerSubject(vs), key(a), db(b), command(c)
    {
    }

    virtual ~ViewerCommandDeferredCommandFromSimulation()
    {
    }

    virtual void Execute()
    {
        viewerSubject->HandleCommandFromSimulation(key, db, command);
    }
private:
    ViewerSubject *viewerSubject;
    EngineKey      key;
    std::string    db;
    std::string    command;
};

// ****************************************************************************
// Class: ViewerCommandSync
//
// Purpose:
//   Handles sync with clients.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 11:59:41 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class ViewerCommandSync : public ViewerInternalCommand
{
public:
    ViewerCommandSync(ViewerObserverToSignal *obj, int t) : 
        ViewerInternalCommand(), syncObserver(obj), tag(t)
    {
    }

    virtual ~ViewerCommandSync()
    {
    }

    virtual void Execute()
    {
        syncObserver->SetUpdate(false);

        // Send the sync to all clients.
        GetViewerState()->GetSyncAttributes()->SetSyncTag(tag);
        GetViewerState()->GetSyncAttributes()->Notify();
    }
private:
    ViewerObserverToSignal *syncObserver;
    int tag;
};

// ****************************************************************************
// Method: OverrideCreateConnectionPrinter
//
// Purpose:
//   Overrides factory method for creating a connection printer object.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep  6 02:05:52 PDT 2014
//
// Modifications:
//
// ****************************************************************************

static ViewerConnectionPrinter *
OverrideCreateConnectionPrinter()
{
    return new ViewerConnectionPrinterUI();
}

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
//    Jeremy Meredith, Wed Apr 21 13:19:50 EDT 2010
//    Save a copy of the original system host profiles.
//
// ****************************************************************************

ViewerSubject::ViewerSubject() : ViewerBaseUI(), 
    launchEngineAtStartup(), openDatabaseOnStartup(), openScriptOnStartup(),
    interpretCommands(), xfer(), clients(),
    unknownArguments(), clientArguments()
{
    //
    // Initialize pointers to some Qt objects that don't get created
    // until later.
    //
    checkParent = 0;

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
    // Create and connect state objects.
    //
    GetViewerStateManager()->CreateState();
    viewerDelayedState = 0;
    viewerDelayedMethods = 0;

    //
    // Option for whether this Viewer will dynamically allow new clients
    // activated from commandline
    //
    shared_viewer_daemon = NULL;

    /// assign each client a unique id..
    /// new export functions are getting added and BroadcastToAllClients
    /// would really be inefficient for this process..
    clientIds = 0;

    //
    // Initialize timer variables.
    //
    animationTimeout = 1;
    lastAnimation = 0;
    timer = NULL;
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
//    Jeremy Meredith, Wed Apr 21 13:20:11 EDT 2010
//    Delete original system host profile list.
//
// ****************************************************************************

ViewerSubject::~ViewerSubject()
{
    delete viewerRPCObserver;
    delete postponedActionObserver;
    delete clientMethodObserver;
    delete clientInformationObserver;
    delete colorTableObserver;

    delete GetPlotFactory();
    delete GetOperatorFactory();
    delete syncObserver;

    delete GetViewerProperties();
    delete GetViewerState();
    delete GetViewerMethods();
    delete GetViewerStateManager();
    delete GetViewerMessaging();
    delete GetViewerFileServer();
    delete GetViewerEngineManager();

    delete inputConnection;
    delete viewerDelayedState;
    delete viewerDelayedMethods;
    delete shared_viewer_daemon;
}

void
ViewerSubject::AddNewViewerClientConnection(ViewerClientConnection* newClient)
{

    newClient->SetupSpecialOpcodeHandler(SpecialOpcodeCallback, (void *)this);

    // Connect up the new client so we can handle its signals.
    connect(newClient, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
            this,      SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
    connect(newClient, SIGNAL(DisconnectClient(ViewerClientConnection *)),
            this,      SLOT(DisconnectClient(ViewerClientConnection *)));

    /// assign unique id to the client..
    newClient->GetViewerClientAttributes().SetId((int)clientIds++);

    clients.push_back(newClient);

    //if(newClient->GetViewerClientAttributes().GetRenderingType() == ViewerClientAttributes::Image)
    //    BroadcastAdvanced(GetViewerState()->GetView3DAttributes());

    //if(newClient->GetViewerClientAttributes().GetRenderingType() == ViewerClientAttributes::Data)
    //    BroadcastAdvanced(0);

    // Discover the client's information.
    QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));
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
//   Brad Whitlock, Thu Apr 22 16:53:34 PST 2010
//   Use a signal to schedule HeavyInitialization.
//
//   Brad Whitlock, Tue May  1 10:06:12 PDT 2012
//   Call GetVisItResourcesDirectory to get the translations directory.
//
//   Brad Whitlock, Tue Oct 21 15:09:53 PDT 2014
//   Use ViewerFactoryMain for non-ui case.
//
// ****************************************************************************

void
ViewerSubject::Initialize()
{
    int timeid = visitTimer->StartTimer();

    // If we're not in -nowin mode then use the UI factory. The default is to use
    // the non-UI factory.
    if(GetViewerProperties()->GetNowin())
        SetViewerFactory(new ViewerFactoryMain());
    else
        SetViewerFactory(new ViewerFactoryUI());

    // Make VisIt translation aware.
    QTranslator *translator = new QTranslator(0);
    QString transPath(GetVisItResourcesDirectory(VISIT_RESOURCES_TRANSLATIONS).c_str());
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

    if (!GetViewerProperties()->GetNowin())
    {
        // Install an event processing callback. This is used to process events
        // while we're waiting for data from the engine.
        SetProcessEventsCallback(ProcessEventsCB, (void *)this);

        // Customize the colors and fonts.
        GetAppearance(qApp, GetViewerState()->GetAppearanceAttributes());
        CustomizeAppearance();
    }

    // Ensure that we always create a connection printer for VCL that uses 
    // socket notifiers.
    GetViewerFactory()->OverrideCreateConnectionPrinter(
        OverrideCreateConnectionPrinter);

    // Set up some callbacks that perform some extra ViewerSubject stuff.
    ViewerActionLogic::SetPostponeActionCallback(PostponeActionCallback, (void *)this);
    DatabaseActionBase::SetSimConnectCallback(ViewerSubject::SimConnectCallback, (void *)this);
    DatabaseActionBase::SetUpdateExpressionCallback(ViewerSubject::UpdateExpressionCallback, (void*)this);

    // Install a launch progress callback for the server manager.
    ViewerServerManager::SetLaunchProgressCallback(LaunchProgressCB, (void*)this);
    // Install a callback for the server manager for when it needs to launch 
    // programs via the engine.
    ViewerServerManager::SetOpenWithEngineCallback(OpenWithEngine, (void*)this);
    // Install a callback to schedule execution of internal commands.
    GetViewerMessaging()->SetCommandsNotifyCallback(CommandNotificationCallback, (void*)this);

    //
    // Set up the Xfer object.
    //
    ConnectXfer();

    //
    // Connect the socket notifiers, etc.
    //
    ConnectObjectsAndHandlers();

    //
    // Connect the the default state objectsto the config manager.
    //
    GetViewerStateManager()->ConnectDefaultState();

    //
    // If we are not deferring heavy initialization, do it now.
    //
    if(!deferHeavyInitialization)
    {
        //
        // Disable reading of commands from the client.
        //
        GetViewerMessaging()->BlockClientInput(true);

        //
        // Do heavy initialization.
        //
        HeavyInitialization();

        //
        // Enable reading of commands from the client. Note that we call
        // the EnableClientInput slot function using a timer because it
        // must be called from the event loop in case ProcessConfigFileSettings
        // added a slot function to the event loop. This prevents race
        // conditions with reading settings and processing commands from the
        // client.
        //
        QTimer::singleShot(350, this, SLOT(EnableClientInput()));
    }
    else
    {
        connect(this, SIGNAL(scheduleHeavyInitialization()),
                this, SLOT(HeavyInitialization()),
                Qt::QueuedConnection);
    }

    if(shared_viewer_daemon)
        QTimer::singleShot(350,shared_viewer_daemon,SLOT(init()));

    visitTimer->StopTimer(timeid, "Total time setting up");
}

// ****************************************************************************
// Method: ViewerSubject::EnableClientInput
//
// Purpose:
//   Enables client input.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 14:39:26 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::EnableClientInput()
{
    GetViewerMessaging()->BlockClientInput(false);
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
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.
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
        RemoteProcess::SetChangeUserNameCallback(&ViewerChangeUsernameUI::ChangeUsernameCallback);
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
        MachineProfile::SetDefaultUserName(parent->GetTheUserName());
    }
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
        GetViewerStateManager()->ConnectPluginDefaultState();
        bool local = GetViewerStateManager()->ProcessSettings();
        if(local)
            QTimer::singleShot(300, this, SLOT(DelayedProcessSettings()));

        //
        // Turn on stereo if it was enabled from the command line
        //
        if (GetViewerProperties()->GetDefaultStereoToOn())
            GetViewerState()->GetRenderingAttributes()->SetStereoRendering(true);

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
            GetViewerMessaging()->Warning(error);
        }

#ifdef HAVE_DDT
        DDTInitialize();
#endif

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

        //
        // Create a timer that is used for animations.
        //
        ViewerWindowManager::Instance()->SetAnimationCallback(AnimationCallback, (void *)this);
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(HandleAnimation()));

        // Hook up the viewer delayed state. If we're currently reading from the parent,
        // we don't want to hook it up since it adds observers to the subjects in
        // the global viewer state, which could currently be in a Notify() if 
        // processingFromParent is true.
        if(processingFromParent)
            QTimer::singleShot(100, this, SLOT(CreateViewerDelayedState()));
        else
        {
            // This is the more common case
            CreateViewerDelayedState();
        }

        // Discover the client's information.
        QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));

        // Open a database on startup.
        if(!WindowMetrics::EmbeddedWindowState())
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
            for(size_t i = 0; i < clients.size(); ++i)
            {
                if(clients[i] != client)
                    clients[i]->BroadcastToClient(subj);
            }
        }

        // Schedule the input to be processed by the main event loop.
        QTimer::singleShot(10, this, SLOT(ProcessFromParent()));
    }
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
        if(client->GetViewerClientAttributes().GetExternalClient()) {
            std::cout << "Disconnecting client: " << client->GetViewerClientAttributes().GetTitle() << std::endl;
        }
        client->deleteLater();

        // check to see if all other clients are remote, if they are then quit since
        // all admin clients have quit
        bool adminClient = false;
        for(size_t i = 0; i < clients.size(); ++i)
        {
            if(!clients[i]->GetViewerClientAttributes().GetExternalClient())
            {
                adminClient = true;
                break;
            }
        }

        // If we ever get down to no client connections, quit.
        if(clients.size() < 1 || !adminClient)
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
    CATCHALL
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
    while (!done)
    {
        done = true;
        for (size_t i=0; i<pluginAtts->GetId().size(); i++)
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
    for (size_t i=0; i<pluginAtts->GetId().size(); i++)
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
    for (int i=0; i<pmgr->GetNAllPlugins(); i++)
    {
        std::string id = pmgr->GetAllID(i);
        if (pluginAtts->GetIndexByID(id) < 0)
        {
            pluginAtts->AddPlugin(pmgr->GetPluginName(id),     "plot",
                                  pmgr->GetPluginVersion(id),  id,
                                  pmgr->PluginEnabled(id));
        }
    }
    for (int i=0; i<omgr->GetNAllPlugins(); i++)
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
    for (int i = 0; i < GetPlotFactory()->GetNPlotTypes(); ++i)
    {
        AttributeSubject *attr = GetPlotFactory()->GetClientAtts(i);
        if (attr != 0)
        {
            GetViewerState()->RegisterPlotAttributes(attr);

            xfer.Add(GetViewerState()->GetPlotAttributes(i));
            xfer.Add(GetViewerState()->GetPlotInformation(i));
        }
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
//   Brad Whitlock, Fri Feb  5 15:28:13 PST 2010
//   I added code to override an operator's operatorCategory in the plugin
//   manager attributes if it's not set to anything worth keeping.
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
    for (int i = 0; i < GetOperatorFactory()->GetNOperatorTypes(); ++i)
    {
        AttributeSubject *attr = GetOperatorFactory()->GetClientAtts(i);
        if (attr != 0)
        {
            xfer.Add(attr);
            GetViewerState()->RegisterOperatorAttributes(attr);
        }
    }

    // Set the operator's category name.
    for (int i = 0; i < GetOperatorPluginManager()->GetNAllPlugins(); i++)
    {
        std::string id(GetOperatorPluginManager()->GetAllID(i));
        // Get the operator's category and set it in the plugin manager 
        // attributes if its plugin category is empty.
        std::string category(GetOperatorPluginManager()->
                             GetOperatorCategoryName(id));

        if(GetViewerState()->GetPluginManagerAttributes()->
            PluginCategoryNameNotSet(id))
        {
            GetViewerState()->GetPluginManagerAttributes()->
                SetPluginCategoryName(id, category);
        }
    }

    // List the objects connected to xfer.
    xfer.ListObjects();

    visitTimer->StopTimer(total, "Loading operator plugins and instantiating objects.");
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
    if (!launchEngineAtStartup.empty())
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

            GetViewerEngineManager()->CreateEngine(
                EngineKey("localhost",""), // The name of the engine (host)
                args,               // The engine arguments
                true,               // Whether to skip the engine chooser
                GetViewerProperties()->GetNumEngineRestarts(), // Number of allowed restarts
                true);              // Whether we're reverse launching

            ViewerWindowManager::Instance()->ShowAllWindows();
        }
        else
        {
            GetViewerEngineManager()->CreateEngine(
                EngineKey(launchEngineAtStartup,""),                 // The name of the engine (host)
                GetViewerProperties()->GetEngineParallelArguments(), // The engine arguments
                true,                                                // Whether to skip the engine chooser
                GetViewerProperties()->GetNumEngineRestarts(),       // Number of allowed restarts
                false);                                              // Whether we're reverse launching
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
//   Jeremy Meredith, Fri Mar 26 13:12:48 EDT 2010
//   Allow for the -o command line option to take an optional ,<pluginID>
//   suffix, e.g. "-o foobar,LAMMPS_1.0".
//
//   Brad Whitlock, Thu Aug 28 10:47:15 PDT 2014
//   Use ViewerMethods to open the database. 
//
// ****************************************************************************

void
ViewerSubject::OpenDatabaseOnStartup()
{
    if(!openDatabaseOnStartup.empty())
    {
        ViewerWindowManager::Instance()->UpdateActions();
        ViewerWindowManager::Instance()->ShowAllWindows();

        // Open the database.
        stringVector split = StringHelpers::split(openDatabaseOnStartup,',');
        std::string db, format;
        if(split.size() >= 2)
        {
            db = split[0];
            format = split[1];
        }
        else if(split.size() == 1)
            db = split[0];
        else
            return;

        GetViewerMethods()->OpenDatabase(db, 0, true, format);
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
//   settings using the local settings DataNode.
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
    GetViewerStateManager()->ProcessLocalSettings();
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
    if (interruptionEnabled && !WindowMetrics::EmbeddedWindowState()) //if not embedded or pyside client
    {
        std::map<EngineKey,QSocketNotifier*>::iterator it;
        for(it = engineKeyToNotifier.begin(); it != engineKeyToNotifier.end(); ++it)
            it->second->setEnabled(false);

        qApp->processEvents(QEventLoop::AllEvents, 100);

        for(it = engineKeyToNotifier.begin(); it != engineKeyToNotifier.end(); ++it)
            it->second->setEnabled(true);
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
        int wmBorder[4] = {0, 0, 0, 0};
        int wmShift[2] = {0, 0};
        int wmScreen[4] = {0, 0, 0, 0};

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
//    Jeremy Meredith, Thu Feb 18 15:39:42 EST 2010
//    Host profiles are now handles outside the config manager.
//
//    Jeremy Meredith, Wed Apr 21 13:20:28 EDT 2010
//    Save a copy of the original host profiles loaded from the system dir.
//    Don't read any host profiles if they passed -noconfig.
//
//    Hank Childs, Fri Feb 11 14:19:18 PST 2011
//    Fix unmatched timer call.
//
//    Brad Whitlock, Thu Aug 28 17:06:48 PDT 2014
//    Use state manager.
//
// ****************************************************************************

void
ViewerSubject::ReadConfigFiles(int argc, char **argv)
{
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

    GetViewerStateManager()->ReadConfigFile(specifiedConfig);
    GetViewerStateManager()->ReadHostProfiles();
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
//
//    Jeremy Meredith, Fri Mar 26 10:39:17 EDT 2010
//    Though we do not need to use the command line to specify assumed and
//    fallback formats anymore, such usage still has some conveniences.
//    Added support to munge the preferred list when given those options.
//
//    Vern Staats, Thu May 12 12:54:40 PDT 2011
//    Add code to always force ssh tunneling of all data connections that
//    is conditional on VISIT_FORCE_SSH_TUNNELING.
//
//    Carson Brownlee, Sun May  6 16:25:28 PDT 2012
//    Add -manta argument.
//
//    Eric Brugger, Fri May 10 14:44:11 PDT 2013
//    I removed support for mangled mesa.
//
// ****************************************************************************

void
ViewerSubject::ProcessCommandLine(int argc, char **argv)
{
    std::string tmpGeometry, tmpViewerGeometry;
    bool geometryProvided = false, viewerGeometryProvided = false;

    // Read the config files.
    ReadConfigFiles(argc, argv);

    int shared_daemon_port = -1;
    std::string shared_daemon_password = "";

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
                GetViewerProperties()->SetDebugLevel(debugLevel);
                GetViewerProperties()->SetBufferDebug(bufferDebug);

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
#ifdef HAVE_OSMESA
            vtkVisItOSMesaRenderingFactory::ForceMesa();
#endif
            RemoteProcess::DisablePTY();
            SetNowinMode(true);
        }
        else if (strcmp(argv[i], "-pyuiembedded") == 0 ||
                 strcmp(argv[i], "-uifile") == 0 ||
                 strcmp(argv[i], "-pysideviewer") == 0 ||
                 strcmp(argv[i], "-pysideclient") == 0)
        {
            WindowMetrics::SetEmbeddedWindowState(true);
        }
        else if (strcmp(argv[i], "-manta") == 0)
        {
            avtCallback::SetMantaMode(true);
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
            GetViewerProperties()->SetEngineParallelArguments(SplitValues(argv[++i], ';'));
        }
        else if(strcmp(argv[i], "-nowindowmetrics") == 0)
        {
            debug1 << "Handling -nowindowmetrics" << endl;
            GetViewerProperties()->SetUseWindowMetrics(false);
        }
        else if(strcmp(argv[i], "-sshtunneling") == 0)
        {
            GetViewerProperties()->SetForceSSHTunneling(true);
        }
        else if (strcmp(argv[i], "-assume_format") == 0)
        {
            if ((i + 1 >= argc))
            {
                cerr << "The -assume_format option must be followed by a "
                        "string." << endl;
            }
            GetViewerProperties()->GetAssumedFormats().push_back(argv[i+1]);
            ++i;
        }
        else if (strcmp(argv[i], "-fallback_format") == 0)
        {
            if ((i + 1 >= argc))
            {
                cerr << "The -fallback_format option must be followed by a "
                        "string." << endl;
            }
            GetViewerProperties()->GetFallbackFormats().push_back(argv[i+1]);
            ++i;
        }
        else if (strcmp(argv[i], "-shared_port") == 0)
        {
            if ((i + 1 >= argc) || (!isdigit(*(argv[i+1]))))
            {
                cerr << "The -shared_port option must be followed by an "
                        "integer(valid port) number." << endl;
                continue;
            }
            shared_daemon_port = atoi(argv[i+1]);
            ++i;
        }
        else if (strcmp(argv[i], "-shared_password") == 0)
        {
            if ((i + 1 >= argc))
            {
                cerr << "The -shared_password option must be followed by a "
                        "string." << endl;
                continue;
            }
            shared_daemon_password = argv[i+1];
            ++i;
        }
        else // Unknown argument -- add it to the list
        {
            clientArguments.push_back(argv[i]);
            unknownArguments.push_back(argv[i]);
        }
#ifdef VISIT_FORCE_SSH_TUNNELING
        // 20110318 VRS patch to lock in ssh tunneling
        GetViewerProperties()->SetForceSSHTunneling(true);
#endif
    }

    if(shared_daemon_port != -1 || shared_daemon_password.length() > 0)
    {
        if(shared_daemon_port == -1 || shared_daemon_password == "")
        {
             std::cerr << "Viewer Sharing not enabled port or password not set correctly"
                 << std::endl;
        }
        else
        {
            shared_viewer_daemon = new SharedDaemon(this,shared_daemon_port,shared_daemon_password);
            ///register visWindow update function
            ///TODO: possibly come up with a better solution?
            ViewerWindow::SetRenderEventCallback(RenderEventCallback, this);
            VisWinRenderingWithoutWindowWithInteractions::SetInteractorCallback(CreateGlobalInteractor);
            /// force shared mode to have interactions when in nowin mode..
            avtCallback::SetNowinInteractionMode(true);

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
    GetViewerFileServer()->CloseServers();
    GetViewerEngineManager()->CloseEngines();

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
// Method: ViewerSubject::ConnectToMetaDataServer
//
// Purpose: 
//   Execute ViewerRPC::ConnectToMetaDataServerRPC
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 22 10:57:49 PDT 2014
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::ConnectToMetaDataServer()
{
    const char *mName = "ViewerSubject::ConnectToMetaDataServer: ";
    int timeid = visitTimer->StartTimer();

    //
    // Write the arguments to the debug logs
    //
    debug1 << mName << "start" << endl;
    debug1 << mName << "Telling mdserver on host "
           << GetViewerState()->GetViewerRPC()->GetProgramHost()
           << " to connect to another client." << endl;
    debug1 << "Arguments:" << endl;
    const stringVector &sv = GetViewerState()->GetViewerRPC()->GetProgramOptions();
    for(size_t i = 0; i < sv.size(); ++i)
         debug1 << "\t" << sv[i].c_str() << endl;

    //
    // Tell the viewer's fileserver to have its mdserver running on 
    // the specified host to connect to another process.
    //
    GetViewerFileServer()->ConnectServer(
        GetViewerState()->GetViewerRPC()->GetProgramHost(),
        GetViewerState()->GetViewerRPC()->GetProgramOptions()); 

    visitTimer->StopTimer(timeid, "Time spent telling mdserver to connect to client.");

    //
    // Check to see if there were errors in mdserver plugin initialization
    //
    std::string err = GetViewerFileServer()->
        GetPluginErrors(GetViewerState()->GetViewerRPC()->GetProgramHost());
    if (!err.empty())
    {
        GetViewerMessaging()->Warning(err);
    }
    debug1 << mName << "end" << endl;

    //
    // Do heavy initialization if we still need to do it.
    //
    emit scheduleHeavyInitialization();
}

/*
  Helper function for converting to serialized data
  */
void GetSerializedData(int windowIndex,
                       int width,
                       int height,
                       double resolution,
                       const ViewerClientInformation::OutputFormat& type,
                       std::vector<ViewerClientInformationElement>& elementList)
{

    ViewerWindow* vwin = ViewerWindowManager::Instance()->GetWindow(windowIndex);

    if( type == ViewerClientInformation::Image )
    {
        ViewerClientInformationElement element;

        size_t len = 0;
        const char* result = 0;

        avtImage_p image = vwin->ScreenCapture();

        /// convert to format..
        if(width == -1 || height == -1)
            result = avtImageFileWriter::WriteToByteArray(image->GetImage(),resolution,1,len);
        else
            result = avtImageFileWriter::WriteToByteArray(image->GetImage(),resolution,1,len,width, height);

        if(len > 0){
            QByteArray data(result,(int)len);

            element.SetData(QString(data.toBase64()).toStdString());
            element.SetFormat(ViewerClientInformation::Image);
            element.SetWindowId(vwin->GetWindowId()+1);
            elementList.push_back(element);
        }
        delete [] result;
    }

    if( type == ViewerClientInformation::Data )
    {
        ViewerPlotList *plotList = vwin->GetPlotList();

        //TODO: Fix this return value..
        if(plotList->GetNumRealizedPlots() == 0 || plotList->GetNumVisiblePlots() == 0) return;

        size_t len = 0;
        const char* result = 0;

        for(int j = 0; j < plotList->GetNumPlots(); ++j)
        {
            ViewerPlot* plot = plotList->GetPlot(j);

            avtDataObjectReader_p reader = plot->GetReader();

            if(reader->InputIsImage())
            {
                avtImage_p image = reader->GetImageOutput();
                if(width == -1 || height == -1)
                    result = avtImageFileWriter::WriteToByteArray(image->GetImage(),resolution,1,len);
                else
                    result = avtImageFileWriter::WriteToByteArray(image->GetImage(),resolution,1,len, width, height);
                if(len > 0){
                    QByteArray data(result,(int)len);
                    ViewerClientInformationElement element;
                    element.SetData(QString(data.toBase64()).toStdString());
                    element.SetFormat(ViewerClientInformation::Image);
                    element.SetWindowId(vwin->GetWindowId()+1);
                    elementList.push_back(element);
                }
                /// free the memory
                delete [] result;
            }
            else if(reader->InputIsDataset())
            {
                avtDataset_p dataset = reader->GetDatasetOutput();
                std::string res = dataset->GetDatasetAsString();

                if(res.size() > 0) {
                    QByteArray data(res.c_str(),(int)res.size());
                    ViewerClientInformationElement element;
                    element.SetData(QString(data.toBase64()).toStdString());
                    element.SetFormat(ViewerClientInformation::Data);
                    element.SetWindowId(vwin->GetWindowId()+1);
                    elementList.push_back(element);
                }
            }
        }
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::ExportWindow
//
// Purpose:
//   Experimental! Export screen capture over memory.
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::ExportWindow()
{
    JSONNode node;
    node.Parse(GetViewerState()->GetViewerRPC()->GetStringArg1());


    intVector windowIds = node["plotIds"].AsIntVector();
    std::string format = node["format"].GetString();


    ViewerClientInformation* qatts = GetViewerState()->GetViewerClientInformation();
    ViewerClientInformation::OutputFormat of;

    qatts->OutputFormat_FromString(format, of);

    if(of == ViewerClientInformation::None)
    {
        std::cerr << "None selected for Export Window" << std::endl;
        return;
    }

    int clientId = GetViewerState()->GetViewerRPC()->GetIntArg1();

    int resultId = -1;
    /// Broadcast directly to client..
    for(int i = 0; i < (int)clients.size(); ++i) {
        ViewerClientAttributes& client = clients[i]->GetViewerClientAttributes();
        if(client.GetId() == clientId) {
            resultId = i;
            break;
        }
    }

    if(resultId < 0) {
        /// client does not exist anymore?
        std::cerr << "Export request for client that does not exist.." << std::endl;
        return;
    }

    ViewerClientConnection* client = clients[resultId];
    ViewerClientAttributes& clatts = client->GetViewerClientAttributes();

    // resolution and window

    std::vector<ViewerClientInformationElement> elementList;

    qatts->ClearVars();

    for(int i = 0; i < (int)windowIds.size(); ++i)
    {
        ViewerWindow* vwin = ViewerWindowManager::Instance()->GetWindow(i);

        bool match = false;
        for(size_t j = 0; j < windowIds.size(); ++j)
        {
            if(windowIds[j] == vwin->GetWindowId()+1)
            {
                match = true;
                break;
            }
        }

        if(!match) continue;

        GetSerializedData(i, clatts.GetImageWidth(), clatts.GetImageHeight(), clatts.GetImageResolutionPcnt(), of, elementList);
    }

    for(size_t i = 0; i < elementList.size(); ++i)
        qatts->AddVars(elementList[i]);

    client->BroadcastToClient(qatts);

    qatts->ClearVars();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::ExportHostProfile
//
// Purpose:
//   Export Selected HostProfile to Directory.
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::ExportHostProfile()
{
    JSONNode node;
    node.Parse(GetViewerState()->GetViewerRPC()->GetStringArg1());

    std::string profileName = node["profileName"].GetString();
    std::string fileName = node["fileName"].GetString();
    bool saveInUserDir = node["saveInUserDir"].GetBool();

    std::string userdir = GetAndMakeUserVisItHostsDirectory();
    HostProfileList *hpl = GetViewerState()->GetHostProfileList();

    for (int i = 0; i < hpl->GetNumMachines(); ++i)
    {
        MachineProfile &pl = hpl->GetMachines(i);
        std::string host = pl.GetHostNickname();

        if(host != profileName) continue;

        std::string name = "";

        if(!saveInUserDir)
            name = fileName;
        else
            name = userdir + VISIT_SLASH_STRING + fileName;

        GetViewerMessaging()->Status(
            TR("Host profile %1 exported to %2").
               arg(host).
               arg(name));

        // Tell the user what happened.
        GetViewerMessaging()->Message(
            TR("VisIt exported host profile \"%1\" to the file: %2. ").
               arg(host).
               arg(name));

        SingleAttributeConfigManager mgr(&pl);
        mgr.Export(name);
        break;
    }
}

// ****************************************************************************
// Method: ViewerSubject::Export
//
// Purpose:
//   Handle new export functions
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

inline void replaceAll( string &s, const string &search, const string &replace ) {
    for( size_t pos = 0; ; pos += replace.length() ) {
        // Locate the substring to replace
        pos = s.find( search, pos );
        if( pos == string::npos ) break;
        // Replace by erasing and inserting
        s.erase( pos, search.length() );
        s.insert( pos, replace );
    }
}

void
ViewerSubject::Export()
{
    JSONNode node;
    std::string str = GetViewerState()->GetViewerRPC()->GetStringArg1();
    int clientId = GetViewerState()->GetViewerRPC()->GetIntArg1();

    replaceAll(str,"\\\\", "\\");
    replaceAll(str,"\\\"", "\"");
    node.Parse(str);

    std::string action = node["action"].GetString();

    if(action == "ExportWindows") {
        ExportWindow();
    }

    if(action == "ExportHostProfile") {
        ExportHostProfile();
    }

    if(action == "GetFileList") {
        std::string host = node["host"].GetString();
        std::string remotePath = node["path"].GetString();

        //std::cout << "host!" << host << " " << remotePath << std::endl;

        GetViewerFileServer()->NoFaultStartServer(host, stringVector());

        MDServerManager::ServerMap& map = MDServerManager::Instance()->GetServerMap();

//        for(MDServerManager::ServerMap::iterator itr = map.begin(); itr != map.end(); ++itr) {
//            std::string key = itr->first;
//            std::cout << "key: " << key << std::endl;
//        }

        if(map.find(host) != map.end()) {
            MDServerManager::ServerInfo* info = map[host];
            QString expanded = info->proxy->GetMDServerMethods()->ExpandPath(remotePath).c_str();

            if( expanded.size() > 2 && (expanded.endsWith("/.") || expanded.endsWith("\\.")) ) {
                expanded.truncate(expanded.length()-2);
            }

            //std::cout << "expanded: " << expanded.toStdString() << std::endl;

            info->proxy->GetMDServerMethods()->ChangeDirectory(expanded.toStdString());

            const MDServerMethods::FileList* list = info->proxy->GetMDServerMethods()->GetFileList("*", true);

            JSONNode node;
            node["files"] = JSONNode::JSONArray();
            node["dirs"] = JSONNode::JSONArray();

            for(int i = 0; i < list->files.size(); ++i) {
                node["files"].Append("&quot;" + list->files[i].name + "&quot;" );
            }
            for(int i = 0; i < list->dirs.size(); ++i) {
                node["dirs"].Append("&quot;" + list->dirs[i].name + "&quot;");
            }

            GetViewerState()->GetQueryAttributes()->SetDefaultName("FileList");
            stringVector sv;
            sv.push_back(node.ToString());
            GetViewerState()->GetQueryAttributes()->SetDefaultVars(sv);
            GetViewerState()->GetQueryAttributes()->Notify();
        }
    }

    if(action == "RegisterNewWindow") {
        int windowId = node["windowId"].GetInt();
        std::string typeMap = node["type"].ToString();
        //std::cout << typeMap << std::endl;
        int type = (int)ViewerClientAttributes::None;
        if(typeMap == "Image" || typeMap == "\"Image\"") {
            type = (int)ViewerClientAttributes::Image;
        } else if(typeMap == "Data"|| typeMap == "\"Data\""){
            type = (int)ViewerClientAttributes::Data;
        }

        if(clientId >= 0 && clientId < clients.size()) {
            ViewerClientConnection* conn = clients[clientId];

            intVector& activeWindows = conn->GetViewerClientAttributes().GetWindowIds();
            intVector& typeWindows = conn->GetViewerClientAttributes().GetRenderingTypes();
            int index = -1;
            for(size_t i = 0; i < activeWindows.size(); ++i) {
                if(activeWindows[i] == windowId) {
                    index = (int)i;
                    break;
                }
            }
            //std::cout << "registering new window for clientId " << activeWindows.size() << " "
            //          << clientId << " " << index << "  " << " " << windowId << " " << type << std::endl;
            if(index == -1) {
                activeWindows.push_back(windowId);
                typeWindows.push_back(type);
            }
        }

        /// activeWindows are 1-based but internal windows are 0-based
        /// really confusing :)
        BroadcastImage(windowId-1, false);
        BroadcastData(windowId-1, clientId);
    }

    if(action == "ColorTable") {

        std::string name = node["name"].GetString();
        intVector red = node["red"].AsIntVector();
        intVector green = node["green"].AsIntVector();;
        intVector blue = node["blue"].AsIntVector();;
        intVector alpha = node["alpha"].AsIntVector();
        intVector pcnt = node["pcnt"].AsIntVector();

        //std::cout << "creating colormap: " << node.ToString() << std::endl;

        ColorTableAttributes* ctAtts = GetViewerState()->GetColorTableAttributes();

        ColorControlPointList ccpl;


        for(size_t i = 0; i <  pcnt.size(); ++i) {
            int scaled = pcnt[i];

            ColorControlPoint p;

            float r = float(scaled-red[0])/float(red[1]-red[0]);
            float g = float(scaled-green[0])/float(green[1]-green[0]);
            float b = float(scaled-blue[0])/float(blue[1]-blue[0]);
            float a = float(scaled-alpha[0])/float(alpha[1]-alpha[0]);

            r = r > 1.0f ? 0.0f : std::min(std::max(0.0f,r),1.0f);
            g = g > 1.0f ? 0.0f : std::min(std::max(0.0f,g),1.0f);
            b = b > 1.0f ? 0.0f : std::min(std::max(0.0f,b),1.0f);
            a = std::min(std::max(0.0f,a),1.0f);

            //std::cout << r << " " << g << " " << b << " " << a << std::endl;
            unsigned char colors[4];
            colors[0] = (unsigned char) (r*255.0f);
            colors[1] = (unsigned char) (g*255.0f);
            colors[2] = (unsigned char) (b*255.0f);
            colors[3] = (unsigned char) (a*255.0f);
            p.SetColors(colors);

            float position = float(scaled)/255.0f;
            p.SetPosition(position);
            ccpl.AddControlPoints(p);

            //std::cout << (int)colors[0] << " " << (int)colors[1] << " " << (int)colors[2] << " " << (int)colors[3] << " " << position << std::endl;
        }

        //std::cout << "setting color table:" << name << std::endl;
        ctAtts->AddColorTable(name,ccpl);
        ctAtts->Notify();

    }
    if(action == "UpdateMouseActions") {
        int windowId = node["windowId"].GetInt()-1;
        std::string button = node["mouseButton"].GetString();

        double start_dx = node["start_dx"].GetDouble();
        double start_dy = node["start_dy"].GetDouble();
        double end_dx = node["end_dx"].GetDouble();
        double end_dy = node["end_dy"].GetDouble();
        bool ctrl = node["ctrl"].GetBool();
        bool shift = node["shift"].GetBool();

        //// start_dx, start_dy, end_dx, end_dy are percentages in x and y...
        ViewerWindow* win = ViewerWindowManager::Instance()->GetWindow(windowId);

        if(!win) {
            ///TODO: exception...
            std::cerr << "window " << windowId+1 << " does not exist" << std::endl;
            return;
        }

        //std::cout << "button action: " << button << std::endl;
        if(button == "Pan") {
            avtView3D v = win->GetView3D();
            v.imagePan[0] = start_dx;
            v.imagePan[1] = start_dy;
            win->SetView3D(v);
        }
        else {
            win->UpdateMouseActions(button,
                                    start_dx, start_dy,
                                    end_dx, end_dy,
                                    ctrl, shift);
        }
    }

    if(action == "ForceRedraw") {
        int windowId = node["windowId"].GetInt()-1;
        ViewerWindow* win = ViewerWindowManager::Instance()->GetWindow(windowId);

        if(!win) {
            std::cerr << "Invalid Window " << windowId+1 << " Selected";
            return;
        }

        win->ClearWindow(false);
        win->GetPlotList()->RealizePlots(false);
        BroadcastImage(windowId, false);
        BroadcastData(windowId, clientId);
    }
}

// ****************************************************************************
// Method: ViewerSubject::SimConnect
//
// Purpose:
//   This method is called when we perform database opening actions that require
//   some extra work to connect to simulations.
//
// Arguments:
//   ek     : The key of the sim to which we're connecting.
//   cbdata : Callback data.
//
// Returns:    
//
// Note:       In this case, the extra work we're doing when connecting to the
//             simulation is to hook up socket notifiers, etc that will help
//             the viewer respond to asynchronous simulation commands and state
//             updates.
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 17:41:53 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::SimConnectCallback(EngineKey &ek, void *cbdata)
{
    ViewerSubject *This = (ViewerSubject *)cbdata;
    This->SimConnect(ek);
}

void
ViewerSubject::SimConnect(EngineKey &ek)
{
    std::string host(ek.OriginalHostName()), db(ek.SimName());

    int sock = GetViewerEngineManager()->GetWriteSocket(ek);
    QSocketNotifier *sn = new QSocketNotifier(sock, QSocketNotifier::Read, this);

    simulationSocketToKey[sock] = ek;

    connect(sn, SIGNAL(activated(int)),
            this, SLOT(ReadFromSimulationAndProcess(int)));

    engineKeyToNotifier[ek] = sn;

    engineMetaDataObserver[ek] = new ViewerMetaDataObserver(
        GetViewerEngineManager()->GetSimulationMetaData(ek), host, db);
    connect(engineMetaDataObserver[ek],
            SIGNAL(metaDataUpdated(const std::string&,const std::string&, const avtDatabaseMetaData*)),
            this,
            SLOT(HandleMetaDataUpdated(const std::string&,const std::string&, const avtDatabaseMetaData*)));

    engineSILAttsObserver[ek] = new ViewerSILAttsObserver(
        GetViewerEngineManager()->GetSimulationSILAtts(ek), host, db);
    connect(engineSILAttsObserver[ek],
            SIGNAL(silAttsUpdated(const std::string&,const std::string&, const SILAttributes*)),
            this,
            SLOT(HandleSILAttsUpdated(const std::string&,const std::string&, const SILAttributes*)));

    engineCommandObserver[ek] = new ViewerCommandFromSimObserver(
        GetViewerEngineManager()->GetCommandFromSimulation(ek), ek, db);
    connect(engineCommandObserver[ek],
            SIGNAL(execute(const EngineKey&,const std::string&, const std::string &)),
            this,
            SLOT(DeferCommandFromSimulation(const EngineKey&,const std::string&, const std::string &)));
}

// ****************************************************************************
// Method: ViewerSubject::UpdateExpressionCallback
//
// Purpose:
//   This callback function is called when we open databases and need to update
//   the viewer's variable menus.
//
// Arguments:
//   md  : A pointer to the new file metadata.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Aug 27 17:43:03 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::UpdateExpressionCallback(const avtDatabaseMetaData *md, void *)
{
    ExpressionList *adder = ParsingExprList::Instance()->GetList();
    VariableMenuPopulator::GetOperatorCreatedExpressions(*adder, md, 
                                                         ViewerBase::GetOperatorPluginManager());
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
//   Kathleen Bonnell, Fri Jun 18 15:11:42 MST 2010
//   Use '.session' as extension on windows, too.
//
// ****************************************************************************

void
ViewerSubject::RemoveCrashRecoveryFile() const
{
    QString filename(GetUserVisItDirectory().c_str());
    filename += "crash_recovery";
    filename += ".session";
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
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
//
//    Brad Whitlock, Mon Nov  9 11:51:49 PST 2009
//    Don't process messages while the engine is launching.
//
// ****************************************************************************

void
ViewerSubject::ProcessFromParent()
{
    if(GetViewerProperties()->GetInExecute() ||
       GetViewerProperties()->GetInLaunch())
    {
        debug1 << "The viewer engine manager is busy processing a request "
                  "so we should not process input from the client. Let's "
                  "reschedule this method to run again later." << endl;
        QTimer::singleShot(200, this, SLOT(ProcessFromParent()));
    }
    else if(GetViewerMessaging()->ClientInputBlocked())
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
        CATCHALL
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
// Method: ViewerSubject::CommandNotificationCallback
//
// Purpose:
//   This callback is called when internal commands are added to the command
//   queue inside ViewerMessaging. We use it the callback to fire a timer event
//   that will process the commands.
//
// Arguments:
//   cbdata  : The callback data.
//   timeout : A timeout to use for the timer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 12:15:57 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::CommandNotificationCallback(void *cbdata, int timeout)
{
    ViewerSubject *This = (ViewerSubject *)cbdata;
    QTimer::singleShot(timeout, This, SLOT(ProcessInternalCommands()));
}

// ****************************************************************************
//  Method: ViewerSubject::ProcessInternalCommands
//
//  Purpose: 
//    This is a Qt slot function that is called when there are internal commands
//    that must be processed.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep  3 12:18:55 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::ProcessInternalCommands()
{
    // See if we're allowed to process commands right now.
    bool allowed = !(GetViewerProperties()->GetInExecute() ||
                     GetViewerProperties()->GetInLaunch()  ||
                     launchingComponent);

    if(allowed)
        GetViewerMessaging()->ProcessCommands();
    else
    {
        // We're not allowed to execute commands right now. Schedule for later.
        CommandNotificationCallback((void *)this, 400);
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
    ViewerConnectionProgress *progress = (ViewerConnectionProgress *)data[1];

    // Only show the dialog if windows have been shown.
    bool windowsShowing = !ViewerWindowManager::Instance()->GetWindowsHidden() &&
                          progress != NULL;

    if (stage == 0)
    {
        This->StartLaunchProgress();
        if (windowsShowing)
        {
            progress->Show();
            retval = !progress->GetCancelled();
        }
    }
    else if (stage == 1)
    {
        if (windowsShowing)
        {
            qApp->processEvents(QEventLoop::AllEvents, 50);
            retval = !progress->GetCancelled();
        }
    }
    else if (stage == 2)
    {
        This->EndLaunchProgress();
        if (windowsShowing) 
        {
            if(!progress->GetIgnoreHide())
                progress->Hide();
            retval = !progress->GetCancelled();
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
//
//   Brad Whitlock, Mon Nov  9 11:49:32 PST 2009
//   Don't send keep alives during an engine launch (this flag more broadly
//   applies to don't do it during the engine chooser or during launch).
//
// ****************************************************************************

void
ViewerSubject::SendKeepAlives()
{
    TRY
    {
        if(launchingComponent || 
           GetViewerProperties()->GetInExecute() ||
           GetViewerProperties()->GetInLaunch())
        {
            // We're launching a component so we don't want to send keep alive
            // signals right now but try again in 20 seconds.
            QTimer::singleShot(20 * 1000, this, SLOT(SendKeepAlives()));
        }
        else
        {
            GetViewerMessaging()->Status(TR("Sending keep alive signals..."));
            GetViewerFileServer()->SendKeepAlives();
            GetViewerEngineManager()->SendKeepAlives();
            ViewerServerManager::SendKeepAlivesToLaunchers();
            GetViewerMessaging()->ClearStatus();
        }
    }
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
// ****************************************************************************

void
ViewerSubject::HandleViewerRPC()
{
    TRY
    {
        HandleViewerRPCEx();
    }
    CATCHALL
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
//    Brad Whitlock, Fri Aug 29 00:10:15 PDT 2014
//    Massive rewrite.
//
// ****************************************************************************

void
ViewerSubject::HandleViewerRPCEx()
{
    // Tell the clients that state logging should be turned off. By state
    // logging, I mean all freely exchanged state objects except for the
    // logRPC, which should be logged when received unless it is a
    // SetStateLoggingRPC.
    //
    GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
    GetViewerState()->GetLogRPC()->SetBoolFlag(false);
    GetViewerState()->GetLogRPC()->Notify();
    GetViewerState()->GetLogRPC()->CopyAttributes(GetViewerState()->GetViewerRPC());
    GetViewerState()->GetLogRPC()->Notify();

    debug4 << "Handling "
           << ViewerRPC::ViewerRPCType_ToString(GetViewerState()->GetViewerRPC()->GetRPCType()).c_str()
           << " RPC." << endl;

    //
    // Handle the RPC. These are special cases. Use the action mechanism.
    //
    switch(GetViewerState()->GetViewerRPC()->GetRPCType())
    {
    case ViewerRPC::CloseRPC:
        Close();
        break;
    case ViewerRPC::ConnectToMetaDataServerRPC:
        // This comes in before the action manager is initialized so let's handle
        // it without actions. It's okay since this is not an RPC that we'd usually
        // handle from something that's not the viewer.
        ConnectToMetaDataServer();
        break;
    case ViewerRPC::OpenClientRPC:
        // This action only has relevance for the viewer.
        OpenClient();
        break;
    case ViewerRPC::ExportRPC:
        // This action only has relevance for the viewer.
        Export();
        break;
    case ViewerRPC::MaxRPC:
        // no-op
        break;
    default:
        // If an RPC is not handled in the above cases, handle it as an action.
        ViewerWindowManager::Instance()->GetActiveWindow()->GetActionManager()->
            HandleAction(*GetViewerState()->GetViewerRPC());
    }


    //if(GetViewerState()->GetViewerRPC()->GetRPCType() == ViewerRPC::DrawPlotsRPC)
    //    BroadcastAdvanced(0);

    // Tell the clients that it's okay to start logging again.
    GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
    GetViewerState()->GetLogRPC()->SetBoolFlag(true);
    GetViewerState()->GetLogRPC()->Notify();

    debug4 << "Done handling "
           << ViewerRPC::ViewerRPCType_ToString(GetViewerState()->GetViewerRPC()->GetRPCType()).c_str()
           << " RPC." << endl;
}

void
ViewerSubject::BroadcastData(int windowId, int clientId)
{
    /// check if any clients have enabled advanced broadcasting before even
    /// starting..

    ViewerClientConnectionVector activeClients;

    //for(size_t i = 0; i < clients.size(); ++i)
    size_t i = clientId;
    {
        ViewerClientAttributes& clatts = clients[i]->GetViewerClientAttributes();
        const intVector& activeWindows = clatts.GetWindowIds();
        const intVector& typeWindows = clatts.GetRenderingTypes();

        for(size_t j = 0; j < activeWindows.size(); ++j) {
            //std::cout << activeWindows[j] << " " << typeWindows[j] << std::endl;
            if(activeWindows[j] == windowId+1 &&
              (ViewerClientAttributes::RenderType)typeWindows[j] == ViewerClientAttributes::Data) {
                activeClients.push_back(clients[i]);
            }
        }
    }

    if(activeClients.size() == 0) return;

    typedef std::vector<ViewerClientInformationElement> ViewerClientInformationElementList;
    std::map<std::string, ViewerClientInformationElementList> geometryElementMap;

    ViewerWindowManager* manager = ViewerWindowManager::Instance();

    for(size_t x = 0; x < activeClients.size(); ++x)
    {
        const ViewerClientAttributes& clatts = activeClients[x]->GetViewerClientAttributes();

        /// otherwise check if it needs transmission in

        /// for now do screen capture, until all clients are stable
        ViewerWindow* vwin = manager->GetWindow(windowId);

        QString qdim = QString("%1x%2").arg(clatts.GetImageWidth())
                .arg(clatts.GetImageHeight());
        std::string dimensions = qdim.toStdString();

        /// if this particular version already exists then skip it..
        if(geometryElementMap.count(dimensions) > 0) continue;

        /// get dataset..
        ViewerPlotList *plotList = vwin->GetPlotList();

        if(plotList->GetNumRealizedPlots() == 0 || plotList->GetNumVisiblePlots() == 0)
            continue;

        GetSerializedData(windowId,
                          clatts.GetImageWidth(),
                          clatts.GetImageHeight(),
                          clatts.GetImageResolutionPcnt(),
                          ViewerClientInformation::Data,
                          geometryElementMap[dimensions]);
    }

    /// if no data to transmit then immediately return..
    if(geometryElementMap.size() == 0) return;

    ViewerClientInformation* qatts = GetViewerState()->GetViewerClientInformation();

    /// I am clearing memory through the ClearVars() operations..
    for(size_t i = 0; i < activeClients.size(); ++i)
    {
        ViewerClientAttributes& clatts = activeClients[i]->GetViewerClientAttributes();

        QString qdim = QString("%1x%2").arg(clatts.GetImageWidth())
                .arg(clatts.GetImageHeight());

        std::string dimensions = qdim.toStdString();

        if(geometryElementMap.count(dimensions) == 0) continue;

        qatts->ClearVars();

        ViewerClientInformationElementList& elementList = geometryElementMap[dimensions];
        for(size_t k = 0; k < elementList.size(); ++k) {
            qatts->AddVars(elementList[k]);
        }
        //std::cout << "broadcasting data" << std::endl;
        activeClients[i]->BroadcastToClient(qatts);
    }

    /// save memory..
    qatts->ClearVars();
}

void
ViewerSubject::BroadcastImage(int windowId, bool inMotion)
{
    /// for now do not update while render update is in motion..
    if(inMotion)
        return;

    /// check if any clients have enabled advanced broadcasting before even
    /// starting..

    ViewerClientConnectionVector activeClients;

    for(size_t i = 0; i < clients.size(); ++i)
    {
        ViewerClientAttributes& clatts = clients[i]->GetViewerClientAttributes();
        const intVector& activeWindows = clatts.GetWindowIds();
        const intVector& typeWindows = clatts.GetRenderingTypes();

        for(size_t j = 0; j < activeWindows.size(); ++j) {
            if(activeWindows[j] == windowId+1 &&
              (ViewerClientAttributes::RenderType)typeWindows[j] == ViewerClientAttributes::Image) {
                activeClients.push_back(clients[i]);
            }
        }
    }

    if(activeClients.size() == 0) return;

    typedef std::vector<ViewerClientInformationElement> ViewerClientInformationElementList;
    std::map<std::string, ViewerClientInformationElementList> geometryElementMap;

    for(size_t x = 0; x < activeClients.size(); ++x)
    {
        const ViewerClientAttributes& clatts = activeClients[x]->GetViewerClientAttributes();

        /// otherwise check if it needs transmission in
        QString qdim = QString("%1x%2").arg(clatts.GetImageWidth())
                .arg(clatts.GetImageHeight());

        std::string dimensions = qdim.toStdString();

        /// if this particular version already exists then skip it..
        if(geometryElementMap.count(dimensions) > 0) continue;

        //int timerId = visitTimer->StartTimer(true);
        GetSerializedData(windowId,
                          clatts.GetImageWidth(),
                          clatts.GetImageHeight(),
                          clatts.GetImageResolutionPcnt(),
                          ViewerClientInformation::Image,
                          geometryElementMap[dimensions]);
    }

    /// if no data to transmit then immediately return..
    if(geometryElementMap.size() == 0) return;

    ViewerClientInformation* qatts = GetViewerState()->GetViewerClientInformation();

    /// I am clearing memory through the ClearVars() operations..
    for(size_t i = 0; i < activeClients.size(); ++i)
    {
        ViewerClientAttributes& clatts = activeClients[i]->GetViewerClientAttributes();

        QString qdim = QString("%1x%2").arg(clatts.GetImageWidth())
                .arg(clatts.GetImageHeight());

        std::string dimensions = qdim.toStdString();

        if(geometryElementMap.count(dimensions) == 0) continue;

        qatts->ClearVars();

        ViewerClientInformationElementList& elementList = geometryElementMap[dimensions];
        for(size_t k = 0; k < elementList.size(); ++k) {
            qatts->AddVars(elementList[k]);
        }
        activeClients[i]->BroadcastToClient(qatts);
    }

    /// save memory..
    qatts->ClearVars();
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
ViewerSubject::PostponeActionCallback(int windowId, const ViewerRPC &args, void *cbdata)
{
    ViewerSubject *This = (ViewerSubject *)cbdata;

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
    GetViewerState()->GetPostponedAction()->SetWindow(windowId);
    GetViewerState()->GetPostponedAction()->SetRPC(args);

    // Add the postponed input to the xfer object so it can be executed later.
    This->AddInputToXfer(0, GetViewerState()->GetPostponedAction());

    debug4 << "Postponing execution of  "
           << ViewerRPC::ViewerRPCType_ToString(args.GetRPCType())
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
            GetViewerState()->GetLogRPC()->Notify();
            //BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            // Tell the logging client to log a change to set the window to the
            // window that originated the RPC.
            if(win != wM->GetActiveWindow())
            {
                GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetActiveWindowRPC);
                GetViewerState()->GetLogRPC()->SetWindowId(win->GetWindowId()+1);
                GetViewerState()->GetLogRPC()->Notify();
                //BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            }
            GetViewerState()->GetLogRPC()->CopyAttributes(&GetViewerState()->
                GetPostponedAction()->GetRPC());
            GetViewerState()->GetLogRPC()->Notify();
            //BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());

            // Handle the action.
            actionMgr->HandleAction(GetViewerState()->GetPostponedAction()->GetRPC());

            // Tell the logging client to log a change to set the window back to
            // The current active window.
            if(win != wM->GetActiveWindow())
            {
                GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetActiveWindowRPC);
                GetViewerState()->GetLogRPC()->SetWindowId(wM->GetActiveWindow()->GetWindowId()+1);
                GetViewerState()->GetLogRPC()->Notify();
                //BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
            }
            // Tell the clients that it's okay to start logging again.
            GetViewerState()->GetLogRPC()->SetRPCType(ViewerRPC::SetStateLoggingRPC);
            GetViewerState()->GetLogRPC()->SetBoolFlag(true);
            GetViewerState()->GetLogRPC()->Notify();
            //BroadcastToAllClients((void*)this, GetViewerState()->GetLogRPC());
        }
        else
        {
            debug1 << "Could not handle postponed action "
                   << tName << " because its window is gone." << endl;
        }
    }
    CATCHALL
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
        const EngineList *engines = GetViewerState()->GetEngineList();
        const stringVector &hosts = engines->GetEngineName();
        const stringVector &sims  = engines->GetSimulationName();
        for(size_t i = 0; i < hosts.size(); ++i)
            GetViewerEngineManager()->InterruptEngine(EngineKey(hosts[i], sims[i]));
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
        for(size_t i = 0; i < This->clients.size(); ++i)
            This->clients[i]->BroadcastToClient(subj);

        //This->BroadcastAdvanced(subj);
    }
}

// ****************************************************************************
// Method: ViewerSubject::HandleSync
//
// Purpose: 
//   This is a Qt slot function that is called when the syncAtts are modified
//   by the client. This function queues an internal command that will send
//   the syncAtts back to the client.
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
    TRY
    {
        GetViewerMessaging()->QueueCommand(
            new ViewerCommandSync(syncObserver,
                                  GetViewerState()->GetSyncAttributes()->GetSyncTag()));
    }
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
            for(int j = 0; j < (int)client.GetMethodNames().size(); ++j)
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
    CATCHALL
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
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
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY
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
//   Brad Whitlock, Tue Nov 29 20:34:06 PST 2011
//   Adapt to newer ViewerConnectionProgressDialog.
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

    ViewerConnectionProgress *progress = NULL;

    TRY
    {
        if(!GetViewerProperties()->GetNowin())
        {
            progress = GetViewerFactory()->CreateConnectionProgress();
            progress->SetHostName("localhost");
            progress->SetComponentName(clientName);
            progress->SetParallel(false);
            progress->SetTimeout(5);
        }

        //
        // Try launch the client (unless a different client will) and start
        // listening for a connection from the new client.
        //
        void *cbData[2];
        cbData[0] = (void *)this;
        cbData[1] = (void *)progress;
        stringVector args(clientArguments);
        for(size_t i = 0; i < programOptions.size(); ++i)
            args.push_back(programOptions[i]);
        newClient->LaunchClient(program, args, 0, 0, LaunchProgressCB, cbData);
        clients.push_back(newClient);

        // Connect up the new client so we can handle its signals.
        connect(newClient, SIGNAL(InputFromClient(ViewerClientConnection *, AttributeSubject *)),
                this,      SLOT(AddInputToXfer(ViewerClientConnection *, AttributeSubject *)));
        connect(newClient, SIGNAL(DisconnectClient(ViewerClientConnection *)),
                this,      SLOT(DisconnectClient(ViewerClientConnection *)));

        GetViewerMessaging()->Message(TR("Added a new client to the viewer."));

        // Discover the client's information.
        QTimer::singleShot(100, this, SLOT(DiscoverClientInformation()));
    }
    CATCH(VisItException)
    {
        delete newClient;

        GetViewerMessaging()->Error(
           TR("VisIt could not connect to the new client %1.").
           arg(program));
    }
    ENDTRY

    delete progress;
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
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
// ****************************************************************************

void
ViewerSubject::ReadFromSimulationAndProcess(int socket)
{
    if (simulationSocketToKey.count(socket) <= 0)
        return;

    EngineKey ek = simulationSocketToKey[socket];

    TRY
    {
       GetViewerEngineManager()->ReadDataAndProcess(ek);
    }
    CATCH(LostConnectionException)
    {
        ViewerWindowManager::Instance()->ResetNetworkIds(ek);
        GetViewerEngineManager()->CloseEngine(ek);
        delete engineMetaDataObserver[ek];
        delete engineSILAttsObserver[ek];
        delete engineCommandObserver[ek];
        delete engineKeyToNotifier[ek];
        engineMetaDataObserver.erase(ek);
        engineSILAttsObserver.erase(ek);
        engineCommandObserver.erase(ek);
        engineKeyToNotifier.erase(ek);
    }
    CATCHALL
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
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
//
//    Brad Whitlock, Mon Aug  6 12:12:22 PDT 2012
//    Print the metadata we're sending to the client.
//
// ****************************************************************************

void
ViewerSubject::HandleMetaDataUpdated(const string &host,
                                     const string &file,
                                     const avtDatabaseMetaData *md)
{
    const char *mName = "ViewerSubject::HandleMetaDataUpdated: ";

    TRY
    {
        // Handle MetaData updates
        ViewerFileServerInterface *fs = GetViewerFileServer();

        *GetViewerState()->GetDatabaseMetaData() = *md;
        if (DebugStream::Level4())
        {
            debug4 << mName << "Received metadata from simulation "
                   << host << ":" << file << endl;
            GetViewerState()->GetDatabaseMetaData()->Print(DebugStream::Stream4());
        }

        fs->SetSimulationMetaData(host, file, *GetViewerState()->GetDatabaseMetaData());
        // The file server will modify the metadata slightly; make sure
        // we picked up the new one.
        *GetViewerState()->GetDatabaseMetaData() = *fs->GetMetaData(host, file);
        ViewerWindowManager *wM=ViewerWindowManager::Instance();
        ViewerPlotList *plotList = wM->GetActiveWindow()->GetPlotList();
        plotList->UpdateExpressionList(false);

        if (DebugStream::Level4())
        {
            debug4 << mName << "Sending metadata to the client: " << endl;
            GetViewerState()->GetDatabaseMetaData()->Print(DebugStream::Stream4());
        }

        GetViewerState()->GetDatabaseMetaData()->SelectAll();
        GetViewerState()->GetDatabaseMetaData()->Notify();

#ifdef HAVE_DDT
        // If this is a DDTSim simulation, we may be using the animation controls to control
        // the sim. Update the animation window information.
        if (DDTManager::isDatabaseDDTSim(file) && DDTManager::isDDTSim(wM->GetActiveWindow()))
        {
            wM->UpdateWindowInformation(WINDOWINFO_ANIMATION, -1);
            wM->UpdateActions();

            const EngineKey &key = plotList->GetEngineKey();
            const avtDatabaseMetaData *md = GetViewerEngineManager()->GetSimulationMetaData(key);
            // If supported, send a command to the DDTSim simulation to (optionally)
            // generate python commands to set up suitable plots for the current
            // vispoint
            if (md && key.IsSimulation())
            {
                for (int i=0; i<md->GetSimInfo().GetNumGenericCommands(); ++i)
                    if (md->GetSimInfo().GetGenericCommands(i).GetName()=="plot")
                    {
                        GetViewerEngineManager()->SendSimulationCommand(
                                key, "plot", "");
                    }
            }
        }
#endif
    }
    CATCHALL
    {
        debug1 << "Could not accept the new metadata from " << host << ":" << file << endl;
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
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
//
//    Brad Whitlock, Wed Jul 11 11:00:29 PDT 2012
//    Call UpdateSILRestriction on the plot list. That's like a replace in 
//    that it will update the plot's SIL to match the new SIL but it does not
//    clear the actor or cause the plot to reexecute.
//
// ****************************************************************************

void
ViewerSubject::HandleSILAttsUpdated(const string &host,
                                    const string &file,
                                    const SILAttributes *sa)
{
    int nWindows = 0, *windowIndices = 0;

    TRY
    {
        // Handle SIL updates
        ViewerFileServerInterface *fs = GetViewerFileServer();

        *GetViewerState()->GetSILAttributes() = *sa;
        fs->SetSimulationSILAtts(host, file, *GetViewerState()->GetSILAttributes());
        GetViewerState()->GetSILAttributes()->SelectAll();
        GetViewerState()->GetSILAttributes()->Notify();

        // Replace the plots that use host:file in the plot list so they will get
        // updated SIL restrictions, etc.
        ViewerWindowManager *wMgr=ViewerWindowManager::Instance();
        windowIndices = wMgr->GetWindowIndices(&nWindows);
        EngineKey ek(host, file);
        for(int i = 0; i < nWindows; ++i)
        {
            ViewerWindow *win = wMgr->GetWindow(windowIndices[i]);
            ViewerPlotList *plist = win->GetPlotList();

            plist->ClearDefaultSILRestrictions(host, file);
            plist->UpdateSILRestriction(ek, file);
        }
    }
    CATCHALL
    {
        ; // nothing
    }
    ENDTRY

    delete [] windowIndices;
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
//   Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//   Replaced CATCHALL(...) with CATCHALL
//
//   Kathleen Biagas, Mon Aug  4 15:42:48 PDT 2014
//   Send category name when adding color table.
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
            QvisNoDefaultColorTableButton::clearAllColorTables();

            int nNames = colorAtts->GetNumColorTables();
            const stringVector &names = colorAtts->GetNames();
            for(int i = 0; i < nNames; ++i)
            {
                QvisColorTableButton::addColorTable(names[i].c_str(), 
                    colorAtts->GetColorTables(i).GetCategoryName().c_str());
                QvisNoDefaultColorTableButton::addColorTable(names[i].c_str(),
                    colorAtts->GetColorTables(i).GetCategoryName().c_str());
            }

            // Update all of the QvisColorTableButton widgets.
            QvisColorTableButton::updateColorTableButtons();
            // Update all of the QvisNoDefaultColorTableButton widgets.
            QvisNoDefaultColorTableButton::updateColorTableButtons();
        }
    }
    CATCHALL
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
//  key : The simulation that sent the command.
//  db  : The filename of the simulation that sent the command.
//  command : The command.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 24 09:40:58 PST 2008
//
// Modifications:
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

        // Defer execution of the command.
        ViewerCommandDeferredCommandFromSimulation *cmd = 
            new ViewerCommandDeferredCommandFromSimulation(this, key, db, command);
        GetViewerMessaging()->QueueCommand(cmd);
    }
    CATCHALL
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
//   Brad Whitlock, Sun Feb 27 21:12:17 PST 2011
//   I added the SetUI command.
//
// ****************************************************************************

void
ViewerSubject::HandleCommandFromSimulation(const EngineKey &key, 
    const std::string &db, const std::string &command)
{
    debug1 << "HandleCommandFromSimulation: key=" << key.ID()
           << ", db=" << db.c_str() << ", command=\"" << command << "\""
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
        GetViewerMessaging()->Message(command.substr(8,command.size()-8));
    }
    else if(command.substr(0,6) == "Error:")
    {
        GetViewerMessaging()->Error(command.substr(6,command.size()-6));
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
        GetViewerEngineManager()->SendSimulationCommand(key, cmd, args);
    }
    else if(command.substr(0,5) == "SetUI")
    {
        stringVector s = SplitValues(command, ':');

        // s[0] = SetUI
        // s[1] = "i" or "s"
        // s[2] = name
        // s[3] = value
        // s[4] = enabled

        // Send the new values to the client so they can be used to update
        // the custom sim window there.
        GetViewerState()->GetSimulationUIValues()->SetHost(key.OriginalHostName());
        GetViewerState()->GetSimulationUIValues()->SetSim(key.SimName());
        GetViewerState()->GetSimulationUIValues()->SetName(s[2]);
#if 0
        if(s[1] == "i")
        {
            int ival = atoi(s[3].c_str());
            GetState()->GetSimulationUIValues()->SetIvalue(ival);
        }
        else
#endif
            GetViewerState()->GetSimulationUIValues()->SetSvalue(s[3]);
        GetViewerState()->GetSimulationUIValues()->SetEnabled(s[4] == "1");
        GetViewerState()->GetSimulationUIValues()->Notify();
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
        for(int j = 0; j < (int)client.GetMethodNames().size() && !hasInterpreter; ++j)
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
                 
// ****************************************************************************
// Method: ViewerSubject::OpenWithEngine
//
// Purpose: 
//   This is a callback function that lets us launch a program through the
//   compute engine instead of through VCL.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 29 16:35:08 PST 2011
//
// Modifications:
//   
// ****************************************************************************

void
ViewerSubject::OpenWithEngine(const std::string &remoteHost, 
    const stringVector &args, void *data)
{
    const char *mName = "ViewerSubject::OpenWithEngine: ";

    debug1 << mName << "start" << endl;
    EngineKey ek(remoteHost, "");

    // We use the data argument to pass in a pointer to the connection
    // progress window.
    ViewerConnectionProgress *progress = (ViewerConnectionProgress *)data;

    if(progress != NULL)
        progress->SetIgnoreHide(true);

    // Launch an engine if one does not exist so we're able to use the
    // same dialog as the mdserver.
    if(!GetViewerEngineManager()->EngineExists(ek))
    {
        debug1 << mName << "Creating engine for " << ek.HostName() << endl;

        stringVector moreArgs;
        bool skipChooser = false;
        int numRestarts = 0;
        bool reverseLaunch = false;
        GetViewerEngineManager()->CreateEngineEx(ek, moreArgs, skipChooser, numRestarts, 
            reverseLaunch, progress);
    }

    debug1 << mName << "Telling engine on host " << ek.HostName() << "to run program:" << endl;
    for(size_t i = 0; i < args.size(); ++i)
        debug1 << "\t" << args[i] << endl;
    debug1 << endl;
    GetViewerEngineManager()->LaunchProcess(ek, args);

    if(progress != NULL)
        progress->SetIgnoreHide(false);

    debug1 << mName << "end" << endl;
}

// ****************************************************************************
// Method: ViewerSubject::AnimationCallback
//
// Purpose:
//   This callback function is registered with ViewerWindowManager to provide
//   animation timer services.
//
// Arguments:
//   op     : The operation 0==update, 1=stop.
//   cbdata : the callback data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Sep  2 15:40:04 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
ViewerSubject::AnimationCallback(int op, void *cbdata)
{
    ViewerSubject *This = (ViewerSubject *)cbdata;
    if(op == 0)
        This->UpdateAnimationTimer();
    else
        This->StopAnimationTimer();
}

// ****************************************************************************
//  Method: ViewerSubject::UpdateAnimationTimer
//
//  Purpose: 
//    This routine determines if the timer for performing animations should
//    be changed (either turned on or off) based on the current state of the
//    timer and the state of all the animations.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Brad Whitlock, Tue May 14 11:25:01 PDT 2002
//    Added code to allow the playback speed to be changed.
//
//    Brad Whitlock, Tue Jul 23 17:03:59 PST 2002
//    I fixed a bug that prevented animations from playing if the first
//    window does not exist.
//
//    Brad Whitlock, Wed Jan 22 16:48:41 PST 2003
//    I added code to turn off the animation timer if the windows are hidden
//    or iconified.
//
//    Brad Whitlock, Wed Mar 12 09:44:55 PDT 2003
//    I added a check to make sure that individual windows are checked for
//    visibility before they are considered for animation.
//
//    Brad Whitlock, Mon Jan 26 09:49:19 PDT 2004
//    I changed how we check the window for animation.
//
//    Brad Whitlock, Wed Dec 10 16:24:29 PST 2008
//    Use AnimationAttributes.
//
//    Brad Whitlock, Tue Sep  2 15:37:13 PDT 2014
//    Rewrite for ViewerSubject.
//
// ****************************************************************************

void
ViewerSubject::UpdateAnimationTimer()
{
    //
    // Determine if any animations are playing.
    //
    bool playing = false;
    ViewerWindowManager *windowMgr = ViewerWindowManager::Instance();
    if(windowMgr->GetWindowsHidden() || windowMgr->GetWindowsIconified())
         playing = false;
    else
    {
         // Look for an animation that is playing.
        std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
        for (size_t i = 0; i < windows.size(); i++)
        {
            if (windows[i]->IsVisible())
            {
                AnimationAttributes::AnimationMode mode =
                    windows[i]->GetPlotList()->GetAnimationAttributes().GetAnimationMode();

                if (mode == AnimationAttributes::PlayMode ||
                    mode == AnimationAttributes::ReversePlayMode)
                {
                    playing = true;
                    break;
                }
            }
        }
    }

    //
    // Turn on timer if one doesn't already exist and an animation is
    // playing or turn off the timer if it is on and no animations are
    // playing.
    //
    if (playing)
    {
        int timeout = GetViewerState()->GetAnimationAttributes()->GetTimeout();
        if (!timer->isActive())
        {
            timer->start(timeout);
        }
        else if(timeout != animationTimeout)
        {
            // Change the playback speed.
            timer->setInterval(timeout);
        }
        animationTimeout = timeout;
    }
    else if (timer->isActive())
        timer->stop();
}

// ****************************************************************************
// Method: ViewerSubject::StopAnimationTimer
//
// Purpose: 
//   Turns off the animation timer and makes all animations stop.
//
// Note:       This method is only called when a window is deleted using
//             the window decorations.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 24 17:50:04 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jan 26 09:48:30 PDT 2004
//   I changed how animations are stopped.
//
//   Brad Whitlock, Wed Dec 10 16:27:15 PST 2008
//   Use AnimationAttributes.
//
//   Brad Whitlock, Tue Sep  2 15:37:13 PDT 2014
//   Rewrite for ViewerSubject.
//
// ****************************************************************************

void
ViewerSubject::StopAnimationTimer()
{
    if(timer->isActive())
    {
        //
        // Turn off the timer so we don't try to animate anything while
        // we're waiting for the window to delete.
        //
        timer->stop();

        //
        // Turn off animation in all windows.
        //
        int numWindows = 0;
        std::vector<ViewerWindow *> windows = ViewerWindowManager::Instance()->GetWindows();
        for(size_t i = 0; i < windows.size(); ++i)
        {
            AnimationAttributes a(windows[i]->GetPlotList()->GetAnimationAttributes());
            a.SetAnimationMode(AnimationAttributes::StopMode);
            windows[i]->GetPlotList()->SetAnimationAttributes(a);
            ++numWindows;
        }

        //
        // If there is only one window, update the global atts since there
        // will be no pending delete to update them.
        //
        if(numWindows < 2)
            ViewerWindowManager::Instance()->UpdateWindowInformation(WINDOWINFO_ANIMATION);
    }
}

// ****************************************************************************
//  Method: ViewerWindowManager::HandleAnimation
//
//  Purpose: 
//    This routine gets called whenever the animation timer goes off.  It
//    advances the appropriate animation to the next frame.  The routine
//    uses a round robin approach to decide which animation to advance so
//    so that all the animations will get advanced synchronously.
//
//  Programmer: Eric Brugger
//  Creation:   October 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu May 9 12:33:53 PDT 2002
//    Added code to prevent going to the next frame when the engine is
//    already executing.
//
//    Brad Whitlock, Wed Jul 24 14:55:50 PST 2002
//    I fixed the scheduling algorithm for choosing the next window.
//
//    Brad Whitlock, Tue Sep 9 15:16:55 PST 2003
//    I added code to tell the ViewerSubject to process some of the input
//    that it received from the client. I had to do this because the animation's
//    NextFrame and PrevFrame methods often need to get a new plot from the
//    compute engine. It uses an RPC to do that and while it is in the RPC,
//    it checks for new input from the client and it looks for an interrupt
//    opcode in that input. If it finds an interrupt then it interrupts,
//    otherwise the input is left unprocessed in the input buffer. The RPC
//    also calls some code to process Qt window events. Unfortunately, that
//    function call does not process client input because the socket has been
//    read. This is okay because it would process client input, which could
//    potentially alter the plot, in the middle of executing a plot. To fix
//    the situation, I tell the ViewerSubject to process any client input
//    that it has after the plot has been executed. This lets us process
//    client input without the danger of being inside the engine proxy's
//    Execute RPC and it is pretty much a noop when we get to this function
//    with an animation that's been cached.
//
//    Brad Whitlock, Mon Jan 26 09:51:08 PDT 2004
//    I changed how we check for animation since there are now multiple
//    time sliders that could update.
//
//    Brad Whitlock, Wed Dec 10 16:25:10 PST 2008
//    Use AnimationAttributes.
//
//    Brad Whitlock, Fri Jan 9 15:05:09 PST 2009
//    Added code to make sure that exceptions do not get propagated into the
//    Qt event loop.
//
//    Mark C. Miller, Wed Jun 17 17:46:18 PDT 2009
//    Replaced CATCHALL(...) with CATCHALL
//
//    Brad Whitlock, Tue Sep  2 15:37:13 PDT 2014
//    Rewrite for ViewerSubject.
//
// ****************************************************************************

void
ViewerSubject::HandleAnimation()
{
    ViewerWindowManager *windowMgr = ViewerWindowManager::Instance();

    //
    // Return without doing anything if the engine is executing.
    //
    if(GetViewerProperties()->GetInExecute() || 
       windowMgr->GetWindowsHidden() ||
       windowMgr->GetWindowsIconified())
    {
        return;
    }

    //
    // Determine the next animation to update.
    //
    std::vector<ViewerWindow *> windows = windowMgr->GetWindows();
    int i, startFrame = lastAnimation + 1;
    if(startFrame == (int)windows.size())
        startFrame = 0;
    for(i = startFrame; i != lastAnimation; )
    {
        AnimationAttributes::AnimationMode mode =
            windows[i]->GetPlotList()->GetAnimationAttributes().GetAnimationMode();

        if (mode == AnimationAttributes::PlayMode ||
            mode == AnimationAttributes::ReversePlayMode)
        {
            lastAnimation = i;
            break;
        }

        // Go to the next window index wrapping around if needed.
        if(i == int(windows.size() - 1))
            i = 0;
        else
            ++i;
    }

    //
    // Advance the animation if animation is allowed for the new
    // animation. We check the flag first in case the window was deleted.
    //
    if(windows[lastAnimation] != NULL)
    {
        AnimationAttributes::AnimationMode mode =
            windows[lastAnimation]->GetPlotList()->GetAnimationAttributes().GetAnimationMode();

        // Prevent the timer from emitting any signals since the
        // code to handle animation may get back to the Qt event
        // loop which makes it possible to get back here reentrantly.
        timer->blockSignals(true);

        TRY
        {
            if (mode == AnimationAttributes::PlayMode)
            {
                // Change to the next frame in the animation, which will likely
                // cause us to have to read a plot from the compute engine.
                windows[lastAnimation]->GetPlotList()->ForwardStep();

                // Send new window information to the client if we're animating
                // the active window.
                windowMgr->UpdateWindowInformation(WINDOWINFO_ANIMATION, lastAnimation);

                // Process any client input that we had to ignore while reading
                // the plot from the compute engine.
                ProcessFromParent();
            }
            else if(mode == AnimationAttributes::ReversePlayMode)
            {
                // Change to the next frame in the animation, which will likely
                // cause us to have to read a plot from the compute engine.
                windows[lastAnimation]->GetPlotList()->BackwardStep();

                // Send new window information to the client if we're animating
                // the active window.
                windowMgr->UpdateWindowInformation(WINDOWINFO_ANIMATION, lastAnimation);

                // Process any client input that we had to ignore while reading
                // the plot from the compute engine.
                ProcessFromParent();
            }
        }
        CATCHALL
        {
            ; // nothing
        }
        ENDTRY 

        // Start the timer up again.
        timer->blockSignals(false);
    }
}

void ViewerSubject::RenderEventCallback(int windowId, bool inMotion, void* data) {
    ViewerSubject* vs = (ViewerSubject*)data;
    vs->BroadcastImage(windowId, inMotion);
}

#include <QVTKInteractor.h>
vtkRenderWindowInteractor* ViewerSubject::CreateGlobalInteractor() {
    QVTKInteractor* iren = QVTKInteractor::New();
    return iren;
}
