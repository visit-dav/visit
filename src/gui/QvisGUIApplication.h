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

#ifndef QVIS_GUI_APPLICATION_H
#define QVIS_GUI_APPLICATION_H
#include <gui_exports.h>
#include <vector>
#include <string>
#include <qobject.h>
#include <visit-config.h>
#include <ViewerProxy.h>
#include <ConfigManager.h>
#include <GUIBase.h>
#include <QvisWindowBase.h>
#include <MessageAttributes.h>
#include <StatusSubject.h>
#include <QualifiedFilename.h>
#include <maptypes.h>

// Long list of forward declarations.
class DataNode;
class ObserverToCallback;
class QPrinter;
class QSocketNotifier;
class QTimer;
class QvisAppearanceWindow;
class QvisApplication;
class QvisColorTableWindow;
class QvisInterpreter;
class QvisMainWindow;
class QvisMacroWindow;
class QvisMessageWindow;
class QvisMovieProgressDialog;
class QvisOutputWindow;
class QvisPickWindow;
class QvisPluginWindow;
class QvisPreferencesWindow;
class QvisSaveMovieWizard;
class QvisSessionFileDatabaseLoader;
class QvisVisItUpdate;
class SplashScreen;

// ****************************************************************************
// Class: QvisGUIApplication
//
// Purpose:
//    This is the main application object for the VisIt graphical user
//    interface. It contains all windows, proxies and important state
//    information.
//
// Notes:      The viewer proxy pointer in GUIBase is allocated in
//             this class's constructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug 31 14:12:07 PST 2000
//
// Modifications:
//    Brad Whitlock, Fri Feb 9 17:36:37 PST 2001
//    I added a save window.
//
//    Brad Whitlock, Fri Feb 16 14:47:49 PST 2001
//    I added a contour plot window.
//
//    Kathleen Bonnell, Tue Mar  6 16:03:13 PST 2001 
//    I added a surface plot window.
//
//    Eric Brugger, Wed Mar  7 16:20:47 PST 2001
//    I changed the types of the plot attribute windows to the more
//    generic QvisPostableWindowObserver type.
//
//    Brad Whitlock, Fri Mar 23 11:00:47 PDT 2001
//    Added the vector plot window.
//
//    Brad Whitlock, Fri Mar 23 16:16:26 PST 2001
//    Modified things so plot and operator windows are treated as plugins.
//
//    Brad Whitlock, Thu Mar 29 14:48:53 PST 2001
//    Added the localOnly member.
//
//    Brad Whitlock, Thu Apr 19 10:38:05 PDT 2001
//    Added slots to handle window iconification.
//
//    Brad Whitlock, Tue Apr 24 16:15:13 PST 2001
//    Added a flag called viewerIsAlive.
//
//    Brad Whitlock, Tue May 1 16:05:51 PST 2001
//    Added the engine window.
//
//    Brad Whitlock, Mon Jun 11 13:59:27 PST 2001
//    Added the color table window.
//
//    Brad Whitlock, Sun Jun 17 20:58:38 PST 2001
//    Added the annotation window.
//
//    Brad Whitlock, Thu May 24 14:54:44 PST 2001
//    Added the subset window.
//
//    Jeremy Meredith, Fri Jul 20 11:26:44 PDT 2001
//    Added CalculateShift and shiftX/Y.
//
//    Jeremy Meredith, Fri Jul 20 13:56:30 PDT 2001
//    Added CalculateScreen and screenX/Y/W/H.
//
//    Jeremy Meredith, Mon Jul 23 16:45:34 PDT 2001
//    Added smallScreen.
//
//    Brad Whitlock, Thu Jul 26 15:42:17 PST 2001
//    Added the view window.
//
//    Sean Ahern, Fri Aug 31 23:59:54 PDT 2001
//    Added the splash screen.
//
//    Jeremy Meredith, Wed Sep  5 13:59:37 PDT 2001
//    Added plugin manager window.
//
//    Brad Whitlock, Tue Sep 4 22:48:48 PST 2001
//    Added infrastructure for gui color/style support.
//
//    Jeremy Meredith, Fri Sep 14 13:54:13 PDT 2001
//    Added preshiftX/Y.
//
//    Jeremy Meredith, Tue Sep 25 15:23:11 PDT 2001
//    Removed functions to calculate window, screen metrics.  These have
//    been encapsulated into WindowMetrics.  All platform specific code
//    has been removed from this class and its source file.
//
//    Jeremy Meredith, Fri Sep 28 13:57:44 PDT 2001
//    Added LoadPlugins method.
//
//    Brad Whitlock, Wed Oct 17 09:19:20 PDT 2001
//    Added the lighting window.
//
//    Eric Brugger, Mon Nov 19 14:58:56 PST 2001
//    Added the animation window.
//
//    Kathleen Bonnell, Wed Dec 12 12:04:58 PST 2001 
//    Added the pick window.
//
//    Jeremy Meredith, Fri Feb  1 16:15:28 PST 2002
//    Added members for QT's commandline arguments.
//
//    Brad Whitlock, Tue Feb 19 10:33:47 PDT 2002
//    Added copyright window and added mechanism for reading multiple
//    config files.
//
//    Brad Whitlock, Wed Feb 20 13:47:42 PST 2002
//    Added printer support.
//
//    Sean Ahern, Tue Apr 16 11:50:18 PDT 2002
//    Renamed MoveMainWindow to MoveAndResizeMainWindow to better suit its
//    purpose.
//
//    Jeremy Meredith, Wed May  8 15:23:03 PDT 2002
//    Added keyframe window.
//
//    Brad Whitlock, Thu May 9 17:02:26 PST 2002
//    Got rid of fileServer since it is now a static member of the base class.
//
//    Brad Whitlock, Thu Jul 11 16:49:40 PST 2002
//    Added the help window.
//
//    Brad Whitlock, Tue Aug 20 13:53:31 PST 2002
//    Added the file information window.
//
//    Brad Whitlock, Wed Sep 18 17:29:21 PST 2002
//    Added the rendering window.
//
//    Brad Whitlock, Fri Sep 6 16:04:40 PST 2002
//    Added the query window.
//
//    Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//    Added material options window.
//
//    Brad Whitlock, Thu Dec 26 16:47:18 PST 2002
//    I changed the interface for the StartMDServer method.
//
//    Kathleen Bonnell, Wed Feb 19 13:13:24 PST 2003
//    Added Global Lineout Window. 
//
//    Eric Brugger, Thu Mar 13 11:34:20 PST 2003
//    Added preferences window.
//
//    Brad Whitlock, Mon May 5 14:02:12 PST 2003
//    I changed the interface to StartMDServer.
//
//    Brad Whitlock, Mon Jun 23 11:42:54 PDT 2003
//    I added RefreshFileList.
//
//    Brad Whitlock, Mon Jul 14 11:50:34 PDT 2003
//    I added RestoreSession and SaveSession.
//
//    Brad Whitlock, Thu Jul 31 15:13:20 PST 2003
//    Added a method to try and read from the viewer when called from a timer.
//
//    Brad Whitlock, Thu Sep 4 10:25:48 PDT 2003
//    I made it use QvisApplication.
//
//    Brad Whitlock, Wed Oct 22 12:10:42 PDT 2003
//    Added addDefaultPlots to the LoadFile method.
//
//    Brad Whitlock, Mon Nov 10 14:51:46 PST 2003
//    I added sessionFile.
//
//    Brad Whitlock, Fri Jan 30 17:47:03 PST 2004
//    I added a database correlation list window.
//
//    Brad Whitlock, Fri Mar 12 13:39:25 PST 2004
//    I added keepAliveTimer and a new slot function.
//
//    Kathleen Bonnell, Wed Mar 31 10:13:43 PST 2004
//    Added QueryOverTime window. 
//
//    Brad Whitlock, Fri Apr 9 14:11:11 PST 2004
//    I added allowFileSelectionChange, which is an internal flag only used
//    at startup.
//
//    Brad Whitlock, Wed May 5 16:02:04 PST 2004
//    I added more support for not creating windows, etc before they are 
//    really needed. This should help launch time.
//
//    Brad Whitlock, Tue Jul 27 12:02:40 PDT 2004
//    Added GetVirtualDatabaseDefinitions.
//
//    Brad Whitlock, Wed Aug 4 15:50:05 PST 2004
//    Added a filename argument to the LoadFile method.
//
//    Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004
//    Added Interactor window. 
//
//    Kathleen Bonnell, Fri Aug 20 15:45:47 PDT 2004 
//    Made colorTableWindow a 'crucial' window, that gets created immediately, 
//    so that color table name are available to other windows as needed. 
//
//    Jeremy Meredith, Wed Aug 25 11:11:39 PDT 2004
//    Added methods to observe updating metadata coming from the viewer.
//    This is used by simulations who send the metadata from the engine.
//
//    Brad Whitlock, Thu Dec 9 10:12:24 PDT 2004
//    Added newExpression, AddPlot, AddOperator slots.
//
//    Hank Childs, Thu Jan 13 13:21:27 PST 2005
//    Added NonSpontaneousIconifyWindow and added an argument to 
//    IconifyWindow indicating if it is spontaneous.
//
//    Brad Whitlock, Wed Feb 9 17:51:44 PST 2005
//    Added an object that knows how to update VisIt.
//
//    Brad Whitlock, Mon Mar 21 15:19:37 PST 2005
//    Added support for saving movies.
//
//    Jeremy Meredith, Mon Apr  4 16:07:59 PDT 2005
//    Added a simulations window.
//
//    Hank Childs, Tue May 24 17:07:58 PDT 2005
//    Added a export DB window.
//
//    Brad Whitlock, Wed May 4 18:00:32 PST 2005
//    Added support for executing client methods.
//
//    Brad Whitlock, Thu Jul 14 11:00:23 PDT 2005
//    removed developmentVersion;
//
//    Brad Whitlock, Thu Oct 27 16:26:46 PST 2005
//    Added sessionFileHelper to help open the databases in a session file.
//
//    Brad Whitlock, Fri Oct 28 12:15:52 PDT 2005
//    Added movie arguments so the user can pass certain arguments to the
//    movie script on the command line when we launch a new VisIt.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes window
//
//    Brad Whitlock, Thu Feb 2 18:53:59 PST 2006
//    I split SaveMovie into SaveMovie and SaveMovieMain.
//
//    Brad Whitlock, Tue Mar 7 10:19:37 PDT 2006
//    Added UpdateSavedConfigFile.
//
//    Brad Whitlock, Tue Jul 25 12:31:39 PDT 2006
//    Added support for -geometry.
//
//    Jeremy Meredith, Mon Aug 28 17:28:42 EDT 2006
//    Added File Open window.
//
//    Brad Whitlock, Tue Nov 14 15:17:53 PST 2006
//    Added ability to restore sessions with different sources.
//
//    Brad Whitlock, Fri Jun 15 09:34:43 PDT 2007
//    Added macroWindow.
//
//    Dave Pugmire, Thu Jan 31 10:47:06 EST 2008
//    Added sessionDir and UpdateSessionDir.
//
// ****************************************************************************

class GUI_API QvisGUIApplication : public QObject, public ConfigManager, public GUIBase
{
    Q_OBJECT
public:
    QvisGUIApplication(int &argc, char **argv);
    ~QvisGUIApplication();
    int Exec();

private:
    void AddViewerArguments(int argc, char **argv);
    void AddViewerSpaceArguments();
    void AddMovieArguments(int argc, char **argv);
    void SplashScreenProgress(const char *, int);
    void CalculateViewerArea(int orientation, int &x, int &y,
                             int &width, int &height);
    void CreateMainWindow();
    void SetupWindows();
    QvisWindowBase *WindowFactory(int i);
    QvisWindowBase *GetWindowPointer(int i);
    QvisWindowBase *GetInitializedWindowPointer(int i);
    void CreateInitiallyVisibleWindows(DataNode *node);
    void ReadWindowSettings(QvisWindowBase *win, DataNode *node);

    void CreatePluginWindows();
    void EnsurePlotWindowIsCreated(int i);
    void EnsureOperatorWindowIsCreated(int i);

    void LaunchViewer();
    void InitializeFileServer(DataNode *);
    void LoadFile(QualifiedFilename &f, bool addDefaultPlots);
    void LoadSessionFile();
    void MoveAndResizeMainWindow(int orientation);
    void ProcessArguments(int &argc, char **argv);
    virtual DataNode *ReadConfigFile(const char *filename);
    void ProcessConfigSettings(DataNode *settings, bool systemConfig);
    void ProcessWindowConfigSettings(DataNode *settings);
    void SetOrientation(int orientation);
    virtual bool WriteConfigFile(const char *filename);
    void ReadSavedMainWindowGeometry(DataNode *node,
                                     bool &wh_set, int &w, int &h,
                                     bool &xy_set, int &x, int &y);

    void ReadPluginWindowConfigs(DataNode *parentNode, const char *configVersion);
    void WritePluginWindowConfigs(DataNode *parentNode);
    void Synchronize(int tag);
    void HandleSynchronize(int val);
    void HandleMetaDataUpdate();
    void HandleClientMethod();
    void GetVirtualDatabaseDefinitions(StringStringVectorMap &defs);

    QString SaveSessionFile(const QString &s);

    // Internal callbacks
    static void StartMDServer(const std::string &hostName,
                              const stringVector &args, void *data);
    static void UpdatePrinterAttributes(Subject *subj, void *data);
    static void SyncCallback(Subject *s, void *data);
    static void UpdateMetaDataAttributes(Subject *subj, void *data);
    static void ClientMethodCallback(Subject *subj, void *data);

    void RestoreSessionFile(const QString &, const stringVector &);
    int  GetNumMovieFrames();
    void UpdateSessionDir( const std::string &sessionFileName );

    void RestoreCrashRecoveryFile();
    void RemoveCrashRecoveryFile(bool) const;
    QString CrashRecoveryFile() const;
public slots:
    void newExpression();
    void saveCrashRecoveryFile();
    void Interpret(const QString &);
private slots:
    void Quit();
    void HeavyInitialization();
    void ReadFromViewer(int);
    void DelayedReadFromViewer();
    void SendKeepAlives();
    void SaveSettings();
    void ActivatePlotWindow(int index);
    void ActivateOperatorWindow(int index);
    void IconifyWindows(bool);
    void NonSpontaneousIconifyWindows();
    void DeIconifyWindows();
    void AboutVisIt();
    void CustomizeAppearance(bool notify);
    void LoadPlugins();
    void FinalInitialization();
    void SendInterface();
    void InterpreterSync();
    void SendMessageBoxResult0();
    void SendMessageBoxResult1();
    void CancelMovie();

    void SaveMovie();
    void SaveMovieMain();
    void SaveWindow();
    void SetPrinterOptions();
    void PrintWindow();
    void RefreshFileList();
    void RefreshFileListAndNextFrame();
    void RestoreSession();
    void RestoreSessionWithDifferentSources();
    void SaveSession();
    void sessionFileHelper_LoadFile(const QString &);
    void sessionFileHelper_LoadSession(const QString &);
    void sessionFileHelper_LoadSessionWithDifferentSources(const QString &,
             const stringVector &);
    void UpdateSavedConfigFile();
    void SaveCrashRecoveryFile();

    // Plot, operator related slots.
    void AddPlot(int, const QString &);
    void AddOperator(int);

    // Slots to show windows.
    void showFileSelectionWindow();
    void showFileOpenWindow();
    void showFileInformationWindow();
    void showHostProfilesWindow();
    void showSaveWindow();
    void showEngineWindow();
    void showAnimationWindow();
    void showAnnotationWindow();
    void showCommandWindow();
    void showExpressionsWindow();
    void showSubsetWindow();
    void showViewWindow();
    void showKeyframeWindow();
    void showLightingWindow();
    void showGlobalLineoutWindow();
    void showMacroWindow();
    void showMaterialWindow();
    void showHelpWindow();
    void displayCopyright();
    void displayReleaseNotes();
    void displayReleaseNotesIfAvailable();
    void showQueryWindow();
    void showRenderingWindow();
    void showCorrelationListWindow();
    void showQueryOverTimeWindow();
    void showInteractorWindow();
    void showSimulationWindow();
    void showExportDBWindow();
    void showMeshManagementWindow();

    void updateVisIt();
    void updateVisItCompleted(const QString &);
private:
    static const char           *windowNames[];
    int                          completeInit;
    int                          stagedInit;
    bool                         viewerIsAlive;
    bool                         closeAllClients;
    bool                         viewerInitiatedQuit;
    bool                         reverseLaunch;

    MessageAttributes            message;

    // The Application
    QvisApplication              *mainApp;

    // A socket notifier to tell us when the viewer proxy has input.
    QSocketNotifier              *fromViewer;
    bool                          allowSocketRead;

    // A timer to make sure that we send keep alives to the mdservers.
    QTimer                       *keepAliveTimer;

    QPrinter                     *printer;
    ObserverToCallback           *printerObserver;

    // Handle metadata updates
    ObserverToCallback           *metaDataObserver;
    ObserverToCallback           *SILObserver;

    SplashScreen                 *splash;
    bool                          showSplash;

    // Commandline arguments for QT
    int    qt_argc;
    char **qt_argv;

    // Crucial Windows
    QvisMainWindow               *mainWin;
    QvisMessageWindow            *messageWin;
    QvisOutputWindow             *outputWin;
    QvisPluginWindow             *pluginWin;
    QvisAppearanceWindow         *appearanceWin;
    QvisPickWindow               *pickWin;
    QvisPreferencesWindow        *preferencesWin;
    QvisColorTableWindow         *colorTableWin;
    QvisSaveMovieWizard          *saveMovieWizard;
    QvisMovieProgressDialog      *movieProgress;

    // Important objects
    QvisInterpreter              *interpreter;
    QvisVisItUpdate              *visitUpdate;
    QvisSessionFileDatabaseLoader *sessionFileHelper;

    // Contains pointers to all of the plot windows.
    WindowBaseVector             plotWindows;

    // Contains pointers to all of the operator windows.
    WindowBaseVector             operatorWindows;

    // Contains pointers to all of the other windows.
    WindowBaseMap                otherWindows;

    // Options that can be set via the command line
    bool                         localOnly;
    bool                         readConfig;

    // These hold the border sizes of the main window.
    int                          borders[4];
    // Other sizing parameters for windows
    int                          shiftX;
    int                          shiftY;
    int                          preshiftX;
    int                          preshiftY;
    int                          screenX;
    int                          screenY;
    int                          screenW;
    int                          screenH;
    bool                         useWindowMetrics;
    // Command line gui geometry.
    bool                         savedGUIGeometry;
    int                          savedGUISize[2];
    int                          savedGUILocation[2];

    // Some appearance attributes
    QString                      foregroundColor;
    QString                      backgroundColor;
    QString                      applicationStyle;
    QString                      applicationFont;

    // File to load on startup.
    QualifiedFilename            loadFile;
    bool                         allowFileSelectionChange;

    // Session variables
    QString                      sessionFile;
    int                          sessionCount;
    std::string                  sessionDir;

    // Movie variables
    stringVector                 movieArguments;

    // Synchronization attributes
    ObserverToCallback           *syncObserver;
    int                          initStage;
    int                          heavyInitStage;
    int                          windowInitStage;
    int                          windowTimeId;

    // Client method attributes
    ObserverToCallback           *clientMethodObserver;

    // Config file storage
    DataNode                     *systemSettings;
    DataNode                     *localSettings;
};

#endif
