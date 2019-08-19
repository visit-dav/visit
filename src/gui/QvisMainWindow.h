/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

#ifndef QVIS_MAIN_WINDOW_H
#define QVIS_MAIN_WINDOW_H
#include <gui_exports.h>
#include <QvisWindowBase.h>
#include <SimpleObserver.h>

// Forward declarations.
class QBoxLayout;
class QPixmap;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSplitter;
class QTimer;
class QVBoxLayout;
class QvisFilePanel;
class QvisNotepadArea;
class QvisSourceManagerWidget;
class QvisPlotManagerWidget;
class QvisTimeSliderControlWidget;
class QvisPostableMainWindow;
class GlobalAttributes;
class MessageAttributes;
class PlotList;
class StatusAttributes;
struct StatusSubject;
class TimeFormat;
class WindowInformation;

// ****************************************************************************
// Class: QvisMainWindow
//
// Purpose:
//   This is class that describes the VisIt main window.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 09:40:53 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Aug 30 13:48:49 PST 2000
//   Made it inherit from QvisWindowBase.
//
//   Brad Whitlock, Thu Apr 19 09:55:11 PDT 2001
//   Added signals to handle window iconification.
//
//   Brad Whitlock, Mon Apr 30 15:04:38 PST 2001
//   Added stuff to handle status messages from the viewer.
//
//   Brad Whitlock, Thu Jul 26 15:48:59 PST 2001
//   Added a signal to activate the view window.
//
//   Jeremy Meredith, Wed Sep  5 14:04:36 PDT 2001
//   Added plugin manager window.
//
//   Brad Whitlock, Thu Sep 6 11:54:18 PDT 2001
//   Added signals for the appearance window and the about box.
//
//   Kathleen Bonnell, Wed Dec 12 12:06:20 PST 2001 
//   Added pick window.
//
//   Brad Whitlock, Thu May 9 16:50:27 PST 2002
//   Changed a few things about the file server.
//
//   Brad Whitlock, Thu Jul 11 16:52:19 PST 2002
//   Added the help window.
//
//   Brad Whitlock, Tue Aug 20 13:45:32 PST 2002
//   Added the file information window.
//
//   Brad Whitlock, Mon Sep 16 15:45:50 PST 2002
//   I added a lot of features to the Windows menu. Then I removed the
//   maintain view and data toggle buttons. Finally, I added a menu item
//   for the Rendering preferences window.
//
//   Brad Whitlock, Fri Sep 6 16:11:16 PST 2002
//   Added the query window.
//
//   Brad Whitlock, Tue Oct 15 16:11:54 PST 2002
//   Added the clone window option.
//
//   Jeremy Meredith, Thu Oct 24 16:15:11 PDT 2002
//   Added material options menu item.
//
//   Brad Whitlock, Mon Nov 11 12:18:08 PDT 2002
//   I added window time and tool locking.
//
//   Kathleen Bonnell, Wed Feb 19 13:13:24 PST 2003  
//   I added activateGlobalLineoutWindow. 
//
//   Eric Brugger, Thu Mar 13 11:58:50 PST 2003
//   I added the preferences window.
//
//   Eric Brugger, Fri Apr 18 09:13:07 PDT 2003
//   I added maintain view limits.
//
//   Brad Whitlock, Wed May 21 07:42:22 PDT 2003
//   I added fullFrame.
//
//   Brad Whitlock, Mon Jun 23 10:14:54 PDT 2003
//   I added refreshFileList.
//
//   Brad Whitlock, Mon Jul 14 11:46:45 PDT 2003
//   I added restoreSession, saveSession signals.
//
//   Brad Whitlock, Wed Jul 30 16:49:21 PST 2003
//   Added reopenOnNextFrame signal.
//
//   Brad Whitlock, Mon Oct 13 17:08:13 PST 2003
//   Added methods to set the timestate display mode.
//
//   Brad Whitlock, Fri Jan 30 14:15:02 PST 2004
//   Added methods to set a flag indicating whether the selected files
//   should be shown.
//
//   Eric Brugger, Mon Mar 29 13:06:32 PST 2004
//   I added maintain data limits.
//
//   Kathleen Bonnell, Wed Mar 31 10:13:43 PST 2004
//   Added method to activate queryOverTime window. 
//
//   Brad Whitlock, Mon Apr 5 15:20:29 PST 2004
//   Added support for reopen and close coming up under an advanced menu.
//
//   Brad Whitlock, Tue Apr 6 14:13:14 PST 2004
//   Added a method to set whether the file panel is allowed to update the
//   file selection.
//
//   Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//   Added method to activate interactors window. 
//
//   Hank Childs, Thu Jan 13 13:18:27 PST 2005
//   Added a boolean to iconifyWindows to indicate if the request was 
//   spontaneous.
//
//   Brad Whitlock, Wed Feb 9 17:48:30 PST 2005
//   Added a help option to update VisIt.
//
//   Hank Childs, Tue May 24 16:59:38 PDT 2005
//   Added export database window.
//
//   Brad Whitlock, Wed Apr 20 17:37:56 PST 2005
//   Added command window and a quit signal.
//
//   Eric Brugger, Thu Jun 30 09:13:18 PDT 2005
//   Added a 2x3 window layout and removed the 4x4 window layout.
//
//   Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//   Added mesh management attributes window
//
//   Brad Whitlock, Tue Jul 25 09:42:39 PDT 2006
//   Added support for a splitter.
//
//   Jeremy Meredith, Mon Aug 28 17:28:42 EDT 2006
//   Added File Open window.
//
//   Brad Whitlock, Tue Nov 14 15:33:47 PST 2006
//   Added restoreSessionWithSources.
//
//   Brad Whitlock, Wed Nov 22 10:53:21 PDT 2006
//   Added override of show method.
//
//   Brad Whitlock, Fri Jun 15 09:32:04 PDT 2007
//   Added activateMacroWindow.
//
//   Brad Whitlock, Wed Jan 23 10:48:53 PST 2008
//   Added unlockEverything.
//
//   Brad Whitlock, Thu Jan 31 10:36:22 PST 2008
//   Added saveCrashRecoveryFile.
//
//   Cyrus Harrison, Mon Jun 30 14:14:59 PDT 2008
//   Initial Qt4 Port.
//
//   Sean Ahern, Wed Dec 31 11:30:16 EST 2008
//   Moved the help menu creation to a separate method.
//
//   Jeremy Meredith, Wed Feb  3 15:35:08 EST 2010
//   Removed maintain data; moved maintain view from Global settings
//   (Main window) to per-window Window Information (View window).
//
//   Jeremy Meredith, Fri Feb 19 20:59:34 EST 2010
//   Hide the Select File menu item when we're not in selected files mode.
//   Big redesign, adding icons and functionality and shuffling arrangement.
//
//   Cyrus Harrison, Fri Mar 12 10:50:26 PST 2010
//   More shuffling to provide better layouts. 
//
//   Hank Childs, Sun Aug  1 12:02:45 PDT 2010
//   Add signal for setting up CMFEs.
//
//   Brad Whitlock, Fri Aug  6 16:59:07 PDT 2010
//   Added signal to open Selections window.
//
//   Brad Whitlock, Mon Sep 20 16:38:48 PDT 2010
//   I removed navigate mode.
//
//   Gunther H. Weber, Thu Aug 18 18:57:20 PDT 2011
//   Added signal for initial Mac configuation.
//
//   Jonathan Byrd (Allinea Software), Sun 18 Dec, 2011
//   Added an Action and a Qt slot to attempt a connection to DDT
//
//   Cyrus Harrison, Tue Aug 13 15:46:52 PDT 2019
//   Removed updateVisIt related logic.
//
// ****************************************************************************

class GUI_API QvisMainWindow : public QvisWindowBase, public SimpleObserver
{
    Q_OBJECT

    using QvisWindowBase::SetFromNode;

public:
    QvisMainWindow(int orientation, const char *captionString = 0);
    ~QvisMainWindow();
    QvisNotepadArea *GetNotepad();
    QvisPlotManagerWidget *GetPlotManager();
    virtual void Update(Subject *ThechangedSubject);
    virtual void SubjectRemoved(Subject *TheRemovedSubject);
    void ConnectGlobalAttributes(GlobalAttributes *);
    void ConnectViewerStatusAttributes(StatusAttributes *);
    void ConnectGUIMessageAttributes();
    void ConnectPlotList(PlotList *);
    void ConnectWindowInformation(WindowInformation *);

    void SetOrientation(int orientation);
    const TimeFormat &GetTimeStateFormat() const;
    bool GetShowSelectedFiles() const;
    bool GetAllowFileSelectionChange() const;
    void OkayToSaveRecoveryFile();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, bool, const int *, const int *, const int *);
signals:
    void quit();
    void iconifyWindows(bool = false);
    void deIconifyWindows();

    // These signals are emitted when opening windows from the
    // VisIt menu bar.
    void activateEngineWindow();
    void activateHostWindow();
    void activateFileWindow();
    void activateFileOpenWindow();
    void activateFileInformationWindow();
    void activateSimulationWindow();
    void activateSaveWindow();
    void activateExportDBWindow();
    void activateAnimationWindow();
    void activateAnnotationWindow();
    void activateColorTableWindow();
    void activateCLI();
    void activateCommandWindow();
    void activateCorrelationListWindow();
    void activateExpressionsWindow();
    void activateCommandLineWindow();
    void activateKeyframeWindow();
    void activateLightingWindow();
    void activateMaterialWindow();
    void activateMacroWindow();
    void activatePickWindow();
    void activateQueryWindow();
    void activateSubsetWindow();
    void activateViewWindow();
    void activateCommandLogWindow();
    void activateAppearanceWindow();
    void activatePluginWindow();
    void activatePreferencesWindow();
    void activateRenderingWindow();
    void activateReleaseNotesWindow();
    void activateAboutWindow();
    void activateCopyrightWindow();
    void activateHelpWindow();
    void activateOutputWindow();
    void activateGlobalLineoutWindow();
    void activateQueryOverTimeWindow();
    void activateInteractorWindow();
    void activateMeshManagementWindow();
    void activateSelectionsWindow();
    void activateSetupHostProfilesAndConfig();
    void activateSeedMeWindow();

    void saveSettings();
    void saveWindow();
    void saveCinema();
    void saveMovie();
    void setupCMFE();
    void printWindow();

    void refreshFileList();
    void saveSession();
    void saveSessionAs();
    void restoreSession();
    void restoreSessionWithSources();
    void reopenOnNextFrame();

    void saveCrashRecoveryFile();
    void setSessionDir();
public slots:
    virtual void show();

    void unreadOutput(bool);
    void SetTimeStateFormat(const TimeFormat &fmt);
    void SetShowSelectedFiles(bool);
    void SetAllowFileSelectionChange(bool);
protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);
private slots:
    void reopenFile(QAction *);
    void closeFile(QAction *);

    void windowAdd();
    void windowClone();
    void windowDelete();
    void windowClearAll();
    void windowLayout1x1();
    void windowLayout1x2();
    void windowLayout2x2();
    void windowLayout2x3();
    void windowLayout2x4();
    void windowLayout3x3();

    void copyView(QAction *);
    void copyLighting(QAction *);
    void copyAnnotations(QAction *);
    void copyPlots(QAction *);
    void copyAll(QAction *);
    void clearPlots();
    void clearReferenceLines();
    void clearPickPoints();

    void emitActivateOutputWindow();

    void autoUpdateToggled(bool);

    void winset(int);
    void winset2(QAction *);

    void toggleSpinMode();
    void toggleFullFrameMode();

    void lockTime();
    void lockTools();
    void lockView();
    void unlockEverything();

    void connectToDDT();
private:
    void CreateMainContents(QSplitter *parent);
    void CreateMainContents(QvisPostableMainWindow *win);
    QWidget *CreateGlobalArea(QWidget *par);
    void UpdateFileMenuPopup(QMenu *, QAction *);
    void UpdateGlobalArea(bool doAll);
    void UpdateWindowList(bool doList);
    void UpdateWindowMenu(bool updateWindowNums);
    void UpdateCrashRecoveryTimer();
    void AddHelpMenu(void);
    void SetDefaultSplitterSizes(int);

private:
    QSplitter                   *splitter;
    QBoxLayout                  *topLayout;
    QvisFilePanel               *filePanel;
    QvisSourceManagerWidget     *sourceManager;
    QvisTimeSliderControlWidget *tsControl;
    QvisPlotManagerWidget       *plotManager;
    QvisNotepadArea             *notepad;

    bool                      unreadOutputFlag;
    QPushButton               *outputButton;
    QPixmap                   *outputRed;
    QPixmap                   *outputBlue;

    QPixmap                   *openIcon;

    QComboBox                 *activeWindowComboBox;
    QCheckBox                 *autoUpdateCheckBox;

    QMenu                     *filePopup;
    QAction                   *selFileAct;
    QAction                   *openFileAct;
    QMenu                     *fileAdvancedPopup;
    QAction                   *fileAdvancedPopupAct;
    QMenu                     *reopenPopup;
    QAction                   *reopenPopupAct;
    QMenu                     *closePopup;
    QAction                   *closePopupAct;
    QAction                   *refreshFileListAct;
    bool                       advancedMenuShowing;

    QMenu                    *winPopup;
    QMenu                    *layoutPopup;
    QAction                  *layoutActions[6];
    QMenu                    *activeWindowPopup;
    QAction                  *activeWindowPopupAct;
    QMenu                    *topCopyPopup;
    QAction                  *topCopyPopupAct;
    QAction                  *copyPopupAct[5];
    QMenu                    *copyPopup[5];
    QMenu                    *clearPopup;
    QAction                  *clearPopupAct;
    QMenu                    *lockPopup;

    QAction                  *lockPopupAct;
    QAction                  *lockTimeAct;
    QAction                  *lockToolsAct;
    QAction                  *lockViewAct;
    QAction                  *spinModeAct;
    QAction                  *fullFrameModeAct;
    QMenu                    *helpPopup;

    QTimer                    *recoveryFileTimer;
    bool                       okayToSaveRecoveryFile;

    QAction                  *connectToDDTAct;

    // Subjects that the window observes.
    GlobalAttributes          *globalAtts;
    MessageAttributes         *viewerMessageAtts;
    MessageAttributes         *fileserverMessageAtts;
    MessageAttributes         *guiMessageAtts;
    StatusAttributes          *statusAtts;
    PlotList                  *plotList;
    WindowInformation         *windowInfo;
};

#endif
