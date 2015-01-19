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
//                               ViewerSubject.h                             //
// ************************************************************************* //

#ifndef VIEWER_SUBJECT_H
#define VIEWER_SUBJECT_H

#define VIEWER_NUM_ENGINE_RESTARTS        2

#include <viewer_exports.h>
#include <QObject>

#include <ViewerBaseUI.h>
#include <ViewerMasterXfer.h>
#include <VisWindowTypes.h>
#include <avtTypes.h>
#include <EngineKey.h>
#include <vectortypes.h>
#include <string>
#include <map>

class QApplication;
class QSocketNotifier;
class QTimer;

class avtDatabaseMetaData;
class BufferConnection;
class ParentProcess;
class SILAttributes;

class ViewerClientConnection;
class ViewerCommandFromSimObserver;
class ViewerMetaDataObserver;
class ViewerMethods;
class ViewerObserverToSignal;
class ViewerRPC;
class ViewerSILAttsObserver;
class ViewerStateBuffered;
class SharedDaemon;
class vtkRenderWindowInteractor;

// ****************************************************************************
//  Class: ViewerSubject
//
//  Purpose:
//      ViewerSubject is the subject class for the Viewer proxy class.  It
//      implements all the methods in the Viewer class within the viewer
//      process.
//
//  Programmer: Eric Brugger
//  Creation:   August 9, 2000
//
//  Modifications:
//    Brad Whitlock, Fri Aug 29 15:56:15 PDT 2014
//    Lots of changes to remove important guts from this class so they can
//    be used in a more stand-alone way while this class remains the heart
//    of the viewer application when UI's are involved. I removed old 
//    modification comments.
//
// ****************************************************************************

class VIEWER_API ViewerSubject : public ViewerBaseUI
{
    Q_OBJECT
public:
    friend class ViewerCommandDeferredCommandFromSimulation;

    ViewerSubject();
    ~ViewerSubject();

    void ProcessCommandLine(int argc, char **argv);
    void Connect(int *argc, char ***argv);
    void Initialize();
    void RemoveCrashRecoveryFile() const;

    void SetNowinMode(bool);
    bool GetNowinMode() const;

    ViewerState   *GetViewerDelayedState();
    ViewerMethods *GetViewerDelayedMethods();
public:
    // Methods that should not be called outside of the "viewer".
    // ************************************************************************
    static bool LaunchProgressCB(void *data, int stage);
    static void SpecialOpcodeCallback(int opcode, void *data);

    void StartLaunchProgress();
    void EndLaunchProgress();
    void BlockSocketSignals(bool);

    // Callback function for opening processes via engine.
    static void OpenWithEngine(const std::string &remoteHost, 
                               const stringVector &args, void *data);

    void AddNewViewerClientConnection(ViewerClientConnection* newClient);

signals:
    void scheduleHeavyInitialization();
public slots:
    void ProcessFromParent();

private:
    void ConnectXfer();
    void ConnectObjectsAndHandlers();

    void InitializePluginManagers();
    void LoadPlotPlugins();
    void LoadOperatorPlugins();
    void InformClientOfPlugins() const;
    void AddInitialWindows();
    void LaunchEngineOnStartup();

    void ProcessEvents();
    void ReadConfigFiles(int argc, char **argv);
    void CustomizeAppearance();
    void InitializeWorkArea();

    bool HasInterpreter() const;
    void InterpretCommands(const std::string &commands);

    void UpdateAnimationTimer();
    void StopAnimationTimer();

private:
    // RPC handler methods.
    void HandleViewerRPCEx();
    void Close();
    void ConnectToMetaDataServer();
    void OpenClient();

private slots:
    void HeavyInitialization();

    void AddInputToXfer(ViewerClientConnection *, AttributeSubject *subj);
    void ProcessSpecialOpcodes(int opcode);
    void DisconnectClient(ViewerClientConnection *client);
    void DiscoverClientInformation();
    void CreateViewerDelayedState();

    void HandleViewerRPC();
    void HandlePostponedAction();
    void HandleSync();
    void HandleClientMethod();
    void HandleClientInformation();
    void HandleMetaDataUpdated(const std::string &host, const std::string &db,
                               const avtDatabaseMetaData *md);
    void HandleSILAttsUpdated(const std::string &host, const std::string &db,
                              const SILAttributes *md);
    void DeferCommandFromSimulation(const EngineKey &key, const std::string &db,
                                    const std::string &command);
    void HandleCommandFromSimulation(const EngineKey &key, const std::string &db,
                                     const std::string &command);
    void HandleColorTable();
    void ProcessInternalCommands();
    void ReadFromParentAndProcess(int);
    void DelayedProcessSettings();
    void SendKeepAlives();
    void EnableClientInput();

    void ReadFromSimulationAndProcess(int);

    void OpenDatabaseOnStartup();
    void OpenScriptOnStartup();

    void HandleAnimation();
private:
    typedef std::vector<ViewerClientConnection *> ViewerClientConnectionVector;
    static void BroadcastToAllClients(void *, Subject *);
    static void SimConnectCallback(EngineKey &ek, void *cbdata);
           void SimConnect(EngineKey &ek);
    static void UpdateExpressionCallback(const avtDatabaseMetaData *md, void *);
    static void AnimationCallback(int, void *);
    static void CommandNotificationCallback(void *cbdata, int timeout);
    static void ProcessEventsCB(void *cbData);
    static void PostponeActionCallback(int windowId, const ViewerRPC &args, void *cbdata);
private:
    QSocketNotifier       *checkParent;
    QTimer                *keepAliveTimer;
    bool                   launchingComponent;
    bool                   deferHeavyInitialization;
    bool                   heavyInitializationDone;
    bool                   interruptionEnabled;
    std::string            launchEngineAtStartup;
    std::string            openDatabaseOnStartup;
    std::string            openScriptOnStartup;
    bool                   blockSocketSignals;
    bool                   processingFromParent;
    int                    animationStopOpcode;
    int                    iconifyOpcode;
    std::string            interpretCommands;

    ViewerMasterXfer       xfer;
    ParentProcess         *parent;
    ViewerClientConnectionVector clients;
    BufferConnection      *inputConnection;
    ViewerStateBuffered   *viewerDelayedState;
    ViewerMethods         *viewerDelayedMethods;

    ViewerObserverToSignal *viewerRPCObserver;
    ViewerObserverToSignal *postponedActionObserver;
    ViewerObserverToSignal *syncObserver;
    ViewerObserverToSignal *clientMethodObserver;
    ViewerObserverToSignal *clientInformationObserver;
    ViewerObserverToSignal *colorTableObserver;

    std::map<int,EngineKey>                     simulationSocketToKey;
    std::map<EngineKey,QSocketNotifier*>        engineKeyToNotifier;
    std::map<EngineKey,ViewerMetaDataObserver*> engineMetaDataObserver;
    std::map<EngineKey,ViewerSILAttsObserver*>  engineSILAttsObserver;
    std::map<EngineKey,ViewerCommandFromSimObserver*>  engineCommandObserver;

    std::vector<std::string> unknownArguments;
    std::vector<std::string> clientArguments;

    QTimer            *timer;
    int               animationTimeout;
    int               lastAnimation;
private:
    /// Experimental Web Client Stuff
    //void BroadcastAdvanced(int windowId, bool inMotion);
    void BroadcastImage(int windowId, bool inMotion);
    void BroadcastData(int windowId, int clientId);
    void Export();
    void ExportWindow();
    void ExportHostProfile();
    static void RenderEventCallback(int windowId, bool inMotion, void* data);
    static vtkRenderWindowInteractor* CreateGlobalInteractor();

    SharedDaemon             *shared_viewer_daemon;
    size_t                   clientIds;
};

#endif
