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
class QvisFilePanel;
class QvisNotepadArea;
class QvisPlotManagerWidget;
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
// ****************************************************************************

class GUI_API QvisMainWindow : public QvisWindowBase, public SimpleObserver
{
    Q_OBJECT
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
signals:
    void iconifyWindows();
    void deIconifyWindows();

    // These signals are emitted when opening windows from the
    // VisIt menu bar.
    void activateEngineWindow();
    void activateHostWindow();
    void activateFileWindow();
    void activateFileInformationWindow();
    void activateSimulationWindow();
    void activatePrintWindow();
    void activateSaveWindow();
    void activateMovieWindow();
    void activateAnimationWindow();
    void activateAnnotationWindow();
    void activateColorTableWindow();
    void activateCorrelationListWindow();
    void activateExpressionsWindow();
    void activateCommandLineWindow();
    void activateKeyframeWindow();
    void activateLightingWindow();
    void activateMaterialWindow();
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

    void saveSettings();
    void saveWindow();
    void saveMovie();
    void printWindow();

    void refreshFileList();
    void saveSession();
    void restoreSession();
    void reopenOnNextFrame();
public slots:
    void unreadOutput(bool);
    void SetTimeStateFormat(const TimeFormat &fmt);
    void SetShowSelectedFiles(bool);
    void SetAllowFileSelectionChange(bool);
protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void hideEvent(QHideEvent *);
    virtual void showEvent(QShowEvent *);
private slots:
    void reopenFile(int);
    void closeFile(int);

    void windowAdd();
    void windowClone();
    void windowDelete();
    void windowClearAll();
    void windowLayout1x1();
    void windowLayout1x2();
    void windowLayout2x2();
    void windowLayout2x4();
    void windowLayout3x3();
    void windowLayout4x4();

    void copyView(int);
    void copyLighting(int);
    void copyAnnotations(int);
    void copyPlots(int);
    void copyAll(int);
    void clearPlots();
    void clearReferenceLines();
    void clearPickPoints();

    void emitActivateOutputWindow();

    void maintainViewToggled(bool);
    void maintainDataToggled(bool);
    void replacePlotsToggled(bool);
    void autoUpdateToggled(bool);

    void winset(int);
    void winset2(int);

    void toggleNavigateMode();
    void toggleSpinMode();
    void toggleFullFrameMode();

    void lockTime();
    void lockTools();
    void lockView();
private:
    void CreateGlobalArea(QLayout *tl);
    void UpdateFileMenuPopup(QPopupMenu *, int);
    void UpdateGlobalArea(bool doAll);
    void UpdateWindowList(bool doList);
    void UpdateWindowMenu(bool updateWindowNums);
private:
    QWidget                   *central;
    QBoxLayout                *topLayout;
    QvisFilePanel             *filePanel;
    QvisPlotManagerWidget     *plotManager;
    QvisNotepadArea           *notepad;

    bool                      unreadOutputFlag;
    QPushButton               *outputButton;
    QPixmap                   *outputRed;
    QPixmap                   *outputBlue;

    QComboBox                 *activeWindowComboBox;
    QCheckBox                 *maintainViewCheckBox;
    QCheckBox                 *maintainDataCheckBox;
    QCheckBox                 *replacePlotsCheckBox;
    QCheckBox                 *autoUpdateCheckBox;

    QPopupMenu                *filePopup;
    QPopupMenu                *fileAdvancedPopup;
    int                        fileAdvancedPopupId;
    QPopupMenu                *reopenPopup;
    int                        reopenPopupId;
    QPopupMenu                *closePopup;
    int                        closePopupId;
    bool                       advancedMenuShowing;

    QPopupMenu                *winPopup;
    QPopupMenu                *layoutPopup;
    QPopupMenu                *activeWindowPopup;
    int                       activeWindowPopupId;
    QPopupMenu                *topCopyPopup;
    int                       topCopyPopupId;
    int                       copyPopupId[5];
    QPopupMenu                *copyPopup[5];
    int                       clearPopupId;
    QPopupMenu                *lockPopup;
    int                       lockPopupId;
    int                       lockTimeId;
    int                       lockToolsId;
    int                       lockViewId;
    int                       navigateModeId;
    int                       spinModeId;
    int                       fullFrameModeId;

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
