#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include <QvisMainWindow.h>
#include <QvisFilePanel.h>
#include <QvisNotepadArea.h>
#include <QvisPlotManagerWidget.h>

#include <StatusSubject.h>
#include <FileServerList.h>
#include <GlobalAttributes.h>
#include <MessageAttributes.h>
#include <PlotList.h>
#include <StatusAttributes.h>
#include <WindowInformation.h>
#include <ViewerProxy.h>

#define VIEWER_NOT_IMPLEMENTED

#include <icons/expression.xpm>
#include <icons/computer.xpm>
#include <icons/fileopen.xpm>
#include <icons/filesave.xpm>
#include <icons/fileprint.xpm>
#include <icons/animate.xpm>
#include <icons/annot.xpm>
#include <icons/light.xpm>
#include <icons/pick.xpm>
#include <icons/plugin.xpm>
#include <icons/rainbow.xpm>
#include <icons/subset.xpm>
#include <icons/view.xpm>
#include <icons/output_blue.xpm>
#include <icons/output_red.xpm>

#include <icons/layout1x1.xpm>
#include <icons/layout1x2.xpm>
#include <icons/layout2x2.xpm>
#include <icons/layout2x4.xpm>
#include <icons/layout3x3.xpm>
#include <icons/layout4x4.xpm>
#include <icons/newwindow.xpm>
#include <icons/deletewindow.xpm>
#include <icons/copymenu.xpm>
#include <icons/lock.xpm>

// ****************************************************************************
// Method: QvisMainWindow::QvisMainWindow
//
// Purpose: 
//    Constructor for the QvisMainWindow class. It creates the widgets
//    that are part of the main window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 09:43:11 PDT 2000
//
// Modifications:
//    Brad Whitlock, Wed Aug 30 13:56:11 PST 2000
//    I made it inherit from QvisWindowBase.
//
//    Brad Whitlock, Mon Apr 23 09:55:19 PDT 2001
//    I made it observe the viewer's message and status attributes.
//
//    Brad Whitlock, Wed Jun 20 11:09:13 PDT 2001
//    I added an icon for the color table menu entry and enabled it.
//
//    Brad Whitlock, Fri Jun 22 12:46:13 PST 2001
//    I added an entry for the subset window in the controls menu.
//   
//    Jeremy Meredith, Fri Jul 20 11:25:56 PDT 2001
//    Removed the "VisIt is Ready" message.  It should occur later.
//
//    Brad Whitlock, Thu Jul 26 15:46:54 PST 2001
//    Added the view window to the Controls menu.
//
//    Brad Whitlock, Thu Aug 30 16:33:30 PST 2001
//    Added icons for certain menu items.
//
//    Jeremy Meredith, Wed Sep  5 14:00:51 PDT 2001
//    Added plugin manager window.
//
//    Jeremy Meredith, Fri Sep 14 13:52:16 PDT 2001
//    Made it position the main window at 100,100 to prevent some window
//    managers adjusting its position to keep all decorations onscreen.
//    We need it in a known position to take some measurements.
//    The main window is repositioned correctly later.
//
//    Brad Whitlock, Wed Oct 17 09:25:00 PDT 2001
//    Enabled the lighting menu option.
//
//    Sean Ahern, Thu Oct 25 11:10:50 PDT 2001
//    Added the Expression window.
//
//    Eric Brugger, Mon Nov 19 15:06:02 PST 2001
//    Enabled the animation menu option.
//
//    Kathleen Bonnell, Wed Dec 12 12:06:20 PST 2001
//    Added the pick window.
//
//    Brad Whitlock, Tue Jan 29 13:38:07 PST 2002
//    Modified code so it supports multiple main window orientations.
//
//    Brad Whitlock, Wed Feb 6 16:52:33 PST 2002
//    Added icons for a few more menu options.
//
//    Brad Whitlock, Tue Feb 19 10:31:45 PDT 2002
//    Added copyright window.
//
//    Brad Whitlock, Wed Feb 20 10:22:52 PDT 2002
//    Removed the simulation option and enabled print options.
//
//    Brad Whitlock, Mon Mar 4 14:19:00 PST 2002
//    Completed autoupdate support.
//
//    Brad Whitlock, Thu Apr 18 13:09:21 PST 2002
//    Prevented the window from moving on Windows.
//
//    Brad Whitlock, Thu May 2 15:18:40 PST 2002
//    Slightly modified autoupdate support.
//
//    Jeremy Meredith, Wed May  8 15:24:21 PDT 2002
//    Added keyframe window.
//
//    Brad Whitlock, Thu May 9 16:51:03 PST 2002
//    I removed the globalState argument.
//
//    Brad Whitlock, Thu Jul 11 16:52:56 PST 2002
//    I added the help window.
//
//    Brad Whitlock, Tue Aug 20 13:43:21 PST 2002
//    I added the file information window.
//
//    Brad Whitlock, Thu Sep 12 17:38:01 PST 2002
//    I added a bunch of viewer options to the Windows menu. I also added
//    the menu item for the Rendering preferences window.
//
//    Brad Whitlock, Tue Oct 15 16:11:00 PST 2002
//    I added a window option to clone a window.
//
//    Jeremy Meredith, Thu Oct 24 16:03:39 PDT 2002
//    Added material options window.
//
//    Brad Whitlock, Mon Nov 11 12:22:37 PDT 2002
//    I added locking options to the window menu.
//
//    Brad Whitlock, Tue Feb 18 11:13:16 PDT 2003
//    I fixed a widget parenting bug with some menus that caused
//    VisIt to crash on exit.
//
//    Kathleen Bonnell, Wed Feb 19 13:13:24 PST 2003  
//    I added activateGlobalLineoutWindow. 
//
//    Eric Brugger, Thu Mar 13 12:00:12 PST 2003
//    I added the preferences window.
//
//    Brad Whitlock, Tue May 6 10:49:07 PDT 2003
//    I moved the host profiles window so it is under options.
//
//    Brad Whitlock, Wed May 21 07:43:52 PDT 2003
//    I added fullFrame mode to the Windows menu.
//
//    Brad Whitlock, Fri Jun 20 17:46:13 PST 2003
//    I made the splashscreen available all the time.
//
//    Brad Whitlock, Mon Jun 23 10:09:51 PDT 2003
//    I added a button to refresh the file list.
//
//    Brad Whitlock, Mon Jul 14 11:47:34 PDT 2003
//    I added menu options for saving and restoring the session.
//
//    Brad Whitlock, Wed Jul 30 16:50:12 PST 2003
//    I made the main window re-emit a signal from the file panel.
//
//    Kathleen Bonnell, Tue Aug 26 13:47:34 PDT 2003 
//    Changed 'Material' to 'Material Options'.
//
// ****************************************************************************

QvisMainWindow::QvisMainWindow(int orientation, const char *captionString)
    : QvisWindowBase(captionString, WDestructiveClose)
{
    int     id;
    QPixmap openIcon, saveIcon, computerIcon, printIcon, rainbowIcon;
    QPixmap annotIcon, lightIcon, subsetIcon, viewIcon;
    QPixmap exprIcon, animIcon, pluginIcon, pickIcon, copyIcon, lockIcon;

    // Make the main window observe the global status subject. This is
    // part of the mechanism that allows other windows to display status
    // messages in the main window's status bar.
    statusSubject->Attach(this);

    // Make the main window observe the viewer's message attributes. This is
    // how messages from the viewer get put into the error window.
    viewerMessageAtts = viewer->GetMessageAttributes();
    viewerMessageAtts->Attach(this);

    // Make the main window observe the file server's message attributes.
    fileserverMessageAtts = fileServer->GetMessageAttributes();
    fileserverMessageAtts->Attach(this);

    // Make sure the status attributes are initialized to NULL.
    statusAtts = NULL;
    guiMessageAtts = NULL;
    plotList = NULL;
    globalAtts = NULL;
    windowInfo = NULL;

    // Create some pixmaps from the XPM images included at the top of the file.
    openIcon = QPixmap(fileopen_xpm);
    saveIcon = QPixmap(filesave_xpm);
    printIcon = QPixmap(fileprint_xpm);
    computerIcon = QPixmap(computer_xpm);
    rainbowIcon = QPixmap(rainbow_xpm);
    annotIcon = QPixmap(annot_xpm);
    lightIcon = QPixmap(light_xpm);
    subsetIcon = QPixmap(subset_xpm);
    viewIcon = QPixmap(view_xpm);
    exprIcon = QPixmap(expression_xpm);
    animIcon = QPixmap(animate_xpm);
    pluginIcon = QPixmap(plugin_xpm);
    pickIcon = QPixmap(pick_xpm);
    copyIcon = QPixmap(copymenu_xpm);
    lockIcon = QPixmap(lock_xpm);

    outputBlue = new QPixmap( output_blue_xpm );
    outputRed = new QPixmap( output_red_xpm );

    //
    // Add the File menu.
    //
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( tr("&File"), file );
    file->insertItem(openIcon, tr("Select &file . . ."), this, SIGNAL(activateFileWindow()), CTRL+Key_F );
    file->insertItem( tr("Refresh file list"), this, SIGNAL(refreshFileList()), CTRL+Key_R);
    file->insertItem( tr("File &information . . ."), this, SIGNAL(activateFileInformationWindow()), CTRL+Key_I);
    file->insertItem( tr("Compute &engines . . ."), this, SIGNAL(activateEngineWindow()), CTRL+Key_E);
    file->insertSeparator();
    file->insertItem(saveIcon, tr("&Save window"), this, SIGNAL(saveWindow()), CTRL+Key_S );
    file->insertItem( tr("Set Save &options . . ."), this, SIGNAL(activateSaveWindow()), CTRL+Key_O);
//    id = file->insertItem( tr("Save movie . . ."), this, SIGNAL(saveMovie()));
//    file->setItemEnabled(id, false);
    id = file->insertItem(printIcon, tr("Print window"), this, SIGNAL(printWindow()));
    id = file->insertItem(tr("Set Print options . . ."), this, SIGNAL(activatePrintWindow()));
    file->insertSeparator();
    id = file->insertItem(tr("Restore session . . ."), this, SIGNAL(restoreSession()));
    id = file->insertItem(tr("Save session . . ."), this, SIGNAL(saveSession()));
    file->insertSeparator();
    file->insertItem( tr("E&xit"), qApp, SLOT(quit()), CTRL+Key_X );

    //
    // Add the Controls menu.
    //
    QPopupMenu *ctrls = new QPopupMenu(this);
    menuBar()->insertItem( tr("&Controls"), ctrls);
    id = ctrls->insertItem(animIcon, tr("&Animation . . ."), this, SIGNAL(activateAnimationWindow()), CTRL+Key_A);
    id = ctrls->insertItem(annotIcon, tr("A&nnotation . . ."), this, SIGNAL(activateAnnotationWindow()), CTRL+Key_N);
    id = ctrls->insertItem(rainbowIcon, tr("Color &table . . ."), this, SIGNAL(activateColorTableWindow()), CTRL+Key_T);
//    id = ctrls->insertItem( tr("Command line . . ."), this, SIGNAL(activateCommandLineWindow()));
//    ctrls->setItemEnabled(id, false);
    id = ctrls->insertItem(exprIcon, tr("&Expressions . . ."), this, SIGNAL(activateExpressionsWindow()), CTRL+SHIFT+Key_E );
    id = ctrls->insertItem( tr("&Keyframing . . ."), this, SIGNAL(activateKeyframeWindow()), CTRL+Key_K);
    id = ctrls->insertItem( tr("&Material Options . . ."), this, SIGNAL(activateMaterialWindow()), CTRL+Key_M);
    id = ctrls->insertItem(lightIcon, tr("&Lighting . . ."), this, SIGNAL(activateLightingWindow()), CTRL+Key_L );
    id = ctrls->insertItem(tr("&Lineout . . ."), this, SIGNAL(activateGlobalLineoutWindow()), CTRL+SHIFT+Key_L );
    id = ctrls->insertItem(pickIcon, tr("&Pick . . ."), this, SIGNAL(activatePickWindow()), CTRL+Key_P );
    id = ctrls->insertItem(tr("&Query . . ."), this, SIGNAL(activateQueryWindow()), CTRL+Key_Q );
    id = ctrls->insertItem(subsetIcon, tr("S&ubset . . ."), this, SIGNAL(activateSubsetWindow()), CTRL+Key_U);
    id = ctrls->insertItem(viewIcon, tr("&View . . ."), this, SIGNAL(activateViewWindow()), CTRL+Key_V);

    //
    // Add the Options menu.
    //
    QPopupMenu * pref = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Options"), pref );
    pref->insertItem( tr("&Appearance . . ."), this, SIGNAL(activateAppearanceWindow()), CTRL+SHIFT+Key_A);
    pref->insertItem(computerIcon, tr("&Host profiles . . ."), this, SIGNAL(activateHostWindow()), CTRL+Key_H );
    pref->insertItem(pluginIcon, tr("&Plugin Manager . . ."), this, SIGNAL(activatePluginWindow()), CTRL+SHIFT+Key_P);
    pref->insertItem(tr("Preferences . . ."), this, SIGNAL(activatePreferencesWindow()));
    pref->insertItem(tr("Rendering . . ."), this, SIGNAL(activateRenderingWindow()));
    pref->insertSeparator();
    pref->insertItem( tr("Save Settings"), this, SIGNAL(saveSettings()));

    //
    // Add the Windows menu.
    //
    winPopup = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Windows"), winPopup );
    winPopup->insertItem(QPixmap(newwindow_xpm), tr("Add"), this, SLOT(windowAdd()), CTRL+Key_Insert);
    winPopup->insertItem(copyIcon, tr("Clone"), this, SLOT(windowClone()));
    winPopup->insertItem(QPixmap(deletewindow_xpm), tr("Delete"), this, SLOT(windowDelete()), CTRL+Key_Delete);
    winPopup->insertItem( tr("Clear all"), this, SLOT(windowClearAll()));
    // Layout sub menu
    layoutPopup = new QPopupMenu( winPopup );
    layoutPopup->insertItem(QPixmap(layout1x1_xpm), tr("1x1"), this, SLOT(windowLayout1x1()), 0, 0);
    layoutPopup->insertItem(QPixmap(layout1x2_xpm), tr("1x2"), this, SLOT(windowLayout1x2()), 0, 1);
    layoutPopup->insertItem(QPixmap(layout2x2_xpm), tr("2x2"), this, SLOT(windowLayout2x2()), 0, 2);
    layoutPopup->insertItem(QPixmap(layout2x4_xpm), tr("2x4"), this, SLOT(windowLayout2x4()), 0, 3);
    layoutPopup->insertItem(QPixmap(layout3x3_xpm), tr("3x3"), this, SLOT(windowLayout3x3()), 0, 4);
    layoutPopup->insertItem(QPixmap(layout4x4_xpm), tr("4x4"), this, SLOT(windowLayout4x4()), 0, 5);
    winPopup->insertItem(QPixmap(layout2x2_xpm), tr("Layouts"), layoutPopup);
    // Active window sub menu
    activeWindowPopup = new QPopupMenu( winPopup );
    connect(activeWindowPopup, SIGNAL(activated(int)),
            this, SLOT(winset2(int)));
    activeWindowPopupId = winPopup->insertItem(tr("Active window"), activeWindowPopup);
    winPopup->setItemEnabled(activeWindowPopupId, false);
    winPopup->insertSeparator();

    // Copy sub menu.
    topCopyPopup = new QPopupMenu(winPopup, "topCopyPopup");
    copyPopup[0] = new QPopupMenu(topCopyPopup, "copyView");
    connect(copyPopup[0], SIGNAL(activated(int)),
            this, SLOT(copyView(int)));
    copyPopupId[0] = topCopyPopup->insertItem("View from", copyPopup[0], 0);
    topCopyPopup->setItemEnabled(copyPopupId[0], false);
    copyPopup[1] = new QPopupMenu(topCopyPopup, "copyLighting");
    connect(copyPopup[1], SIGNAL(activated(int)),
            this, SLOT(copyLighting(int)));
    copyPopupId[1] = topCopyPopup->insertItem("Lighting from", copyPopup[1], 1);
    topCopyPopup->setItemEnabled(copyPopupId[1], false);
    copyPopup[2] = new QPopupMenu(topCopyPopup, "copyAnnotations");
    connect(copyPopup[2], SIGNAL(activated(int)),
            this, SLOT(copyAnnotations(int)));
    copyPopupId[2] = topCopyPopup->insertItem("Annotations from", copyPopup[2], 2);
    topCopyPopup->setItemEnabled(copyPopupId[2], false);
    copyPopup[3] = new QPopupMenu(topCopyPopup, "copyPlots");
    connect(copyPopup[3], SIGNAL(activated(int)),
            this, SLOT(copyPlots(int)));
    copyPopupId[3] = topCopyPopup->insertItem("Plots from", copyPopup[3], 3);
    topCopyPopup->setItemEnabled(copyPopupId[3], false);
    copyPopup[4] = new QPopupMenu(topCopyPopup, "copyAll");
    connect(copyPopup[4], SIGNAL(activated(int)),
            this, SLOT(copyAll(int)));
    copyPopupId[4] = topCopyPopup->insertItem("Everything from", copyPopup[4], 4);
    topCopyPopup->setItemEnabled(copyPopupId[4], false);
    topCopyPopupId = winPopup->insertItem(copyIcon, "Copy", topCopyPopup);
    winPopup->setItemEnabled(id, false);

    // Clear sub menu
    QPopupMenu *clearPopup = new QPopupMenu( winPopup );
    clearPopup->insertItem( tr("Pick points"), this, SLOT(clearPickPoints()));
    clearPopup->insertItem( tr("Plots"), this, SLOT(clearPlots()));
    clearPopup->insertItem( tr("Reference lines"), this, SLOT(clearReferenceLines()));
    clearPopupId = winPopup->insertItem(tr("Clear"), clearPopup);

    // Lock sub menu
    lockPopup = new QPopupMenu( winPopup );
    lockTimeId  = lockPopup->insertItem( tr("Time"), this, SLOT(lockTime()));
    lockToolsId = lockPopup->insertItem( tr("Tools"), this, SLOT(lockTools()));
    lockViewId  = lockPopup->insertItem( tr("View"), this, SLOT(lockView()));
    lockPopupId = winPopup->insertItem(lockIcon, tr("Lock"), lockPopup);

    // Other options.
    fullFrameModeId = winPopup->insertItem("Full frame", this, SLOT(toggleFullFrameMode()));
    navigateModeId = winPopup->insertItem("Navigate bbox", this, SLOT(toggleNavigateMode()));
    spinModeId = winPopup->insertItem("Spin", this, SLOT(toggleSpinMode()));
    
    //
    // Add the Help menu
    //
    QPopupMenu *help = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), help);
    help->insertItem( tr("About . . ."), this, SIGNAL(activateAboutWindow()));
    help->insertItem( tr("Copyright . . ."), this, SIGNAL(activateCopyrightWindow()));
    help->insertItem( tr("Help . . ."), this, SIGNAL(activateHelpWindow()), Key_F1);
    help->insertItem( tr("Release notes . . ."), this, SIGNAL(activateReleaseNotesWindow()));

    // Make a central widget to contain the other widgets
    central = new QWidget( this );
    setCentralWidget( central );

    //
    // Create the top layout manager. It will vary depending on the 
    // main window's layout mode.
    //
    if(orientation < 2)
        topLayout = new QVBoxLayout(central, 5);
    else
        topLayout = new QHBoxLayout(central, 5);

    //
    // Create the file panel and make it an observer of the file server.
    //
    QVBoxLayout *middleLayout = new QVBoxLayout(topLayout);
    QvisFilePanel *filePanel = new QvisFilePanel(central, "FilePanel");
    connect(filePanel, SIGNAL(reopenOnNextFrame()),
            this, SIGNAL(reopenOnNextFrame()));
    filePanel->ConnectFileServer(fileServer);
    filePanel->ConnectGlobalAttributes(viewer->GetGlobalAttributes());
    middleLayout->addWidget(filePanel);

    // Create the global area.
    CreateGlobalArea(middleLayout);

    // Create the plot Manager.
    plotManager = new QvisPlotManagerWidget(central, "plotManager");
    plotManager->ConnectPlotList(viewer->GetPlotList());
    plotManager->ConnectFileServer(fileServer);
    plotManager->ConnectGlobalAttributes(viewer->GetGlobalAttributes());
    plotManager->ConnectExpressionList(viewer->GetExpressionList());
    plotManager->ConnectPluginManagerAttributes(viewer->GetPluginManagerAttributes());
    plotManager->viewer = viewer;
    topLayout->addWidget(plotManager, 150);

    // Create the notepad widget. Use a big stretch factor so the
    // notpad widget will fill all the remaining space.
    notepad = new QvisNotepadArea( central );
    topLayout->addWidget(notepad, 300);

    // Create the output button and put it in the status bar as a
    // permanent widget.
    outputButton = new QPushButton(statusBar(), "outputButton");
    connect(outputButton, SIGNAL(clicked()), this, SLOT(emitActivateOutputWindow()));
    outputButton->setPixmap(*outputBlue);
    outputButton->setFixedSize(32 , 32);
    QToolTip::add(outputButton, "Output window" );
    statusBar()->addWidget(outputButton, 0, true);
    statusBar()->setSizeGripEnabled(false);
    unreadOutputFlag = false;

#if !defined(_WIN32)
    // Move the window to a known position on the screen
    // so we can take some measurements later
    move(QPoint(100,100));
#endif
}

// ****************************************************************************
// Method: QvisMainWindow::~QvisMainWindow
//
// Purpose: 
//   Destructor for the QvisMainWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 09:43:52 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 23 11:23:06 PDT 2001
//   Detached from the viewer's message and status atts.
//
//   Brad Whitlock, Thu Oct 25 18:32:09 PST 2001
//   Added a new subject.
//
//   Brad Whitlock, Fri Feb 1 14:25:56 PST 2002
//   Added code to delete the window's pixmaps.
//
//   Brad Whitlock, Mon Sep 16 15:57:29 PST 2002
//   Added a couple new subjects.
//
// ****************************************************************************

QvisMainWindow::~QvisMainWindow()
{
    if(statusSubject)
       statusSubject->Detach(this);

    if(statusAtts)
       statusAtts->Detach(this);

    if(globalAtts)
       globalAtts->Detach(this);

    if(viewerMessageAtts)
       viewerMessageAtts->Detach(this);

    if(fileserverMessageAtts)
       fileserverMessageAtts->Detach(this);

    if(plotList)
       plotList->Detach(this);

    if(windowInfo)
       windowInfo->Detach(this);

    // Inherited from GUIBase.
    if(msgAttr)
       msgAttr->Detach(this);

    // Delete the pixmaps.
    delete outputButton;
    delete outputRed;
    delete outputBlue;
}

// ****************************************************************************
// Method: QvisMainWindow::CreateGlobalArea
//
// Purpose: 
//   This method creates the "global area" which is a bunch of widgets
//   that show stuff like autoupdate and the active window list.
//
// Arguments:
//   tl : The layout under which we're going to add the global area.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 13:03:40 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 11:43:20 PDT 2002
//   Enabled autoupdate since it is going to be a GUI concept now.
//
//   Brad Whitlock, Thu Mar 7 14:46:41 PST 2002
//   Enabled replacePlots since it is implemented in the viewer.
//
//   Brad Whitlock, Tue Sep 17 15:43:22 PST 2002
//   I removed maintain view/data.
//
//   Eric Brugger, Fri Apr 18 09:10:03 PDT 2003 
//   I added maintain view limits.
//
// ****************************************************************************

void
QvisMainWindow::CreateGlobalArea(QLayout *tl)
{
    QGridLayout *globalLayout = new QGridLayout(tl, 2, 7);
    globalLayout->setSpacing(0);
    globalLayout->setColStretch(2, 50);
    globalLayout->setColStretch(5, 50);

    activeWindowComboBox = new QComboBox(false, central,
        "activeWindowComboBox");
    connect(activeWindowComboBox, SIGNAL(activated(int)),
            this, SLOT(winset(int)));
    activeWindowComboBox->insertItem("1");
    QLabel *activeWindowLabel = new QLabel(activeWindowComboBox, 
       "Active window", central, "activeWindowLabel");
    globalLayout->addMultiCellWidget(activeWindowLabel, 0, 0, 0, 1, Qt::AlignCenter);
    globalLayout->addMultiCellWidget(activeWindowComboBox, 1, 1, 0, 1);

    QLabel *maintainLabel = new QLabel("Maintain limits", central);
    globalLayout->addMultiCellWidget(maintainLabel, 0, 0, 3, 4, Qt::AlignCenter)
;

    maintainViewCheckBox = new QCheckBox("view", central,
        "maintainViewCheckBox");
    connect(maintainViewCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(maintainViewToggled(bool)));
    globalLayout->addWidget(maintainViewCheckBox, 1, 3);

    replacePlotsCheckBox = new QCheckBox("Replace plots", central,
        "replacePlotsCheckBox");
    connect(replacePlotsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(replacePlotsToggled(bool)));
    globalLayout->addWidget(replacePlotsCheckBox, 0, 6);

    autoUpdateCheckBox = new QCheckBox("Auto update", central,
        "autoUpdateCheckBox");
    connect(autoUpdateCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(autoUpdateToggled(bool)));

    globalLayout->addWidget(autoUpdateCheckBox, 1, 6);
}

// ****************************************************************************
// Method: QvisMainWindow::Update
//
// Purpose: 
//   Updates the window when one of the subjects that it observes
//   is changed.
//
// Arguments:
//   TheChangedSubject : This is a pointer to the subject that changed.
//
// Notes:
//   This method contains calls that process events for the Qt
//   application object (qApp). This is to ensure that the statusbar
//   messages are displayed before doing an expensive operation. Anyway,
//   this is potentially hazardous.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 13:10:21 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 23 11:23:27 PDT 2001
//   Added code to re-send messages that come from the viewer.
//
//   Brad Whitlock, Mon Apr 30 15:08:11 PST 2001
//   Added code to put status messages from the viewer into the status bar.
//
//   Jeremy Meredith, Fri Jun 29 15:18:06 PDT 2001
//   Changed status reporting a little.  Also, key off of MessageType instead.
//
//   Jeremy Meredith, Thu Jul  5 12:40:54 PDT 2001
//   Changed the status message to prefix a total percent.
//
//   Brad Whitlock, Fri Sep 21 13:35:19 PST 2001
//   Made the status messages from the viewer last as long as the sender
//   wanted them to.
//
//   Brad Whitlock, Fri Oct 26 09:39:15 PDT 2001
//   Modified messaging a little bit.
//
//   Brad Whitlock, Fri Aug 23 16:26:47 PST 2002
//   I replaced the event processing loops with code to force the status
//   bar to redraw. This prevents bad re-entrant things from happening.
//
//   Brad Whitlock, Mon Sep 16 15:50:31 PST 2002
//   I added code to update the window when some new subjects are updated.
//
//   Brad Whitlock, Wed Sep 25 11:43:22 PDT 2002
//   I fixed a memory leak.
//
//   Brad Whitlock, Tue May 20 15:05:18 PST 2003
//   Made it work with the regenerated MessageAttributes.
//
// ****************************************************************************

void
QvisMainWindow::Update(Subject *TheChangedSubject)
{
    // If any of the objects that the main window observes is
    // not set, get out.
    if(statusSubject == 0 || globalAtts == 0 ||
       statusAtts == 0 || viewerMessageAtts == 0)
        return;

    if(TheChangedSubject == statusSubject)
    {
        // Update the status bar.
        if(statusSubject->clearing)
            statusBar()->clear();
        else
            statusBar()->message(statusSubject->text, statusSubject->milliseconds);

        // Force the status bar to redraw.
        QPaintEvent *pe = new QPaintEvent(statusBar()->visibleRect(), true);
        QApplication::sendEvent(statusBar(), pe);
        delete pe;
#ifdef QT_WS_X11
        QApplication::flushX();
#endif
    }
    else if(TheChangedSubject == statusAtts)
    {
        // Update the status bar.
        if(statusAtts->GetClearStatus())
        {
            statusBar()->clear();
        }
        else
        {
            QString statusMsg;

            // If the message field is selected, use it. Otherwise, make up
            // a message based on the other fields.
            if(statusAtts->GetMessageType() == 1)
                statusMsg = QString(statusAtts->GetMessage().c_str());
            else if (statusAtts->GetMessageType() == 2)
            {
                int total;
                if (statusAtts->GetMaxStage() > 0)
                {
                    float pct0  = float(statusAtts->GetPercent()) / 100.;
                    float pct1  = float(statusAtts->GetCurrentStage()-1) / float(statusAtts->GetMaxStage());
                    float pct2  = float(statusAtts->GetCurrentStage())   / float(statusAtts->GetMaxStage());
                    total = int(100. * ((pct0 * pct2) + ((1.-pct0) * pct1)));
                }
                else
                    total = 0;

                statusMsg.sprintf("%d%% done: %s (%d%% of stage %d/%d)",
                    total,
                    statusAtts->GetCurrentStageName().c_str(),
                    statusAtts->GetPercent(),
                    statusAtts->GetCurrentStage(),
                    statusAtts->GetMaxStage());
            }

            statusBar()->message(statusMsg, statusAtts->GetDuration());
        }

        // Force the status bar to redraw.
        QPaintEvent *pe = new QPaintEvent(statusBar()->visibleRect(), true);
        QApplication::sendEvent(statusBar(), pe);
        delete pe;
#ifdef QT_WS_X11
        QApplication::flushX();
#endif
    }
    else if(TheChangedSubject == viewerMessageAtts)
    {
        SetUpdate(false);
        unreadOutput(true);

        // Get the message from the viewer's message attributes and send it
        // through the GUI's error message methods.
        if(viewerMessageAtts->GetSeverity() == MessageAttributes::Error)
            Error(QString(viewerMessageAtts->GetText().c_str()));
        else if(viewerMessageAtts->GetSeverity() == MessageAttributes::Warning)
            Warning(QString(viewerMessageAtts->GetText().c_str()));
        else if(viewerMessageAtts->GetSeverity() == MessageAttributes::Message)
            Message(QString(viewerMessageAtts->GetText().c_str()));
    }
    else if(TheChangedSubject == fileserverMessageAtts)
    {
        SetUpdate(false);
        unreadOutput(true);

        // Get the message from the fileserver's message attributes and send
        // it through the GUI's error message methods.
        if(fileserverMessageAtts->GetSeverity() == MessageAttributes::Error)
            Error(QString(fileserverMessageAtts->GetText().c_str()));
        else if(fileserverMessageAtts->GetSeverity() == MessageAttributes::Warning)
            Warning(QString(fileserverMessageAtts->GetText().c_str()));
        else if(fileserverMessageAtts->GetSeverity() == MessageAttributes::Message)
            Message(QString(fileserverMessageAtts->GetText().c_str()));
    }
    else if(TheChangedSubject == msgAttr)
    {
        // Indicate that there is unread output.
        unreadOutput(true);
    }
    else if(TheChangedSubject == globalAtts)
    {
        // Update the things in the main window that need the
        // viewer proxy's global attributes.
        UpdateGlobalArea(false);
        UpdateWindowMenu(globalAtts->IsSelected(0) ||
                         globalAtts->IsSelected(1));
    }
    else if(TheChangedSubject == plotList)
    {
        UpdateWindowMenu(false);
    }
    else if(TheChangedSubject == windowInfo)
    {
        UpdateWindowMenu(false);
    }
}

// ****************************************************************************
// Method: QvisMainWindow::UpdateGlobalArea
//
// Purpose: 
//   This method updates the parts of the main window that depend on
//   the GlobalAttributes state object.
//
// Note:       
//   Note that the main window does not care about all of the attributes
//   in the GlobalAttributes state object.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 11:43:20 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 11:44:27 PDT 2002
//   Enabled auto update.
//
//   Brad Whitlock, Thu Mar 7 14:47:20 PST 2002
//   Enabled replacePlots.
//
//   Brad Whitlock, Thu May 2 15:19:53 PST 2002
//   Removed code to set a flag in the plot manager widget.
//
//   Brad Whitlock, Mon Sep 16 15:40:04 PST 2002
//   I added code to check the window layout.
//
//   Brad Whitlock, Mon Nov 4 13:57:33 PST 2002
//   I added code to set the makeDefaultConfirm flag.
//
//   Brad Whitlock, Fri Nov 15 11:43:01 PDT 2002
//   I fixed a bug on Windows.
//
//   Brad Whitlock, Thu Mar 20 11:05:52 PDT 2003
//   I updated the numbering in the switch statement because I removed
//   some fields from the middle of GlobalAttributes.
//   
//   Eric Brugger, Fri Apr 18 09:10:03 PDT 2003 
//   I added maintain view limits.
//   
// ****************************************************************************

void
QvisMainWindow::UpdateGlobalArea(bool doAll)
{
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    for(int i = 0; i < globalAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!globalAtts->IsSelected(i))
            continue;
        }

        switch(i)
        {
        case 0: // windows
            UpdateWindowList(true);
            break;
        case 1: // activeWindow
            UpdateWindowList(false);
            break;
        case 2: // iconifiedFlag
            break;
        case 3: // autoUpdateflag
            autoUpdateCheckBox->blockSignals(true);
            autoUpdateCheckBox->setChecked(globalAtts->GetAutoUpdateFlag());
            autoUpdateCheckBox->blockSignals(false);

            // Set GUIBase's autoupdate flag.
            autoUpdate = globalAtts->GetAutoUpdateFlag();
            break;
        case 4: // replacePlots
            replacePlotsCheckBox->blockSignals(true);
            replacePlotsCheckBox->setChecked(globalAtts->GetReplacePlots());
            replacePlotsCheckBox->blockSignals(false);
            break;
        case 5: // applyOperator
            break;
        case 6: // currentFile
            break;
        case 7: // currentState
            break;
        case 8: // nStates
            break;
        case 9: // currentFrame
            break;
        case 10: // nFrames
            break;
        case 11: // animationMode
            break;
        case 12: // executing
            break;
        case 13: // windowLayout
        { // new scope
            layoutPopup->setItemChecked(0,globalAtts->GetWindowLayout() == 1);
            layoutPopup->setItemChecked(1,globalAtts->GetWindowLayout() == 2);
            layoutPopup->setItemChecked(2,globalAtts->GetWindowLayout() == 4);
            layoutPopup->setItemChecked(3,globalAtts->GetWindowLayout() == 8);
            layoutPopup->setItemChecked(4,globalAtts->GetWindowLayout() == 9);
            layoutPopup->setItemChecked(5,globalAtts->GetWindowLayout() == 16);
            for(int j = 0; j < 6; ++j)
                layoutPopup->setItemEnabled(j, true);
        }
            break;
        case 14: // makeDefaultConfirm
            // Set GUIBase's makeDefaultConfirm flag.
            makeDefaultConfirm = globalAtts->GetMakeDefaultConfirm();
            break;
        case 15: // viewKeyframes
            break;
        case 16: // cloneWindowOnFirstRef
            break;
        case 17: // maintainView
            maintainViewCheckBox->blockSignals(true);
            maintainViewCheckBox->setChecked(globalAtts->GetMaintainView());
            maintainViewCheckBox->blockSignals(false);
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisMainWindow::UpdateWindowList
//
// Purpose: 
//   Updates the active window list.
//
// Arguments:
//   doList : If this is true then the list is populated. Otherwise,
//            this routine only sets the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 11:54:33 PDT 2000
//
// Modifications:
//   Eric Brugger, Fri Sep 22 10:05:24 PDT 2000
//   I modified the setting of the active window to work properly.
//   
//   Brad Whitlock, Fri Sep 13 15:48:27 PST 2002
//   I changed it so it also updates the new Active window menu.
//
// ****************************************************************************

void
QvisMainWindow::UpdateWindowList(bool doList)
{
    int i;
    int index = globalAtts->GetActiveWindow();
    const intVector &indices = globalAtts->GetWindows();

    if(doList)
    {
        // Update the Active window combo box.
        activeWindowComboBox->clear();
        for(i = 0; i < indices.size(); ++i)
        {
            QString temp; temp.sprintf("%d", indices[i]);
            activeWindowComboBox->insertItem(temp);
        }

        // Update the Active window menu
        activeWindowPopup->clear();
        for(i = 0; i < indices.size(); ++i)
        {
            QString str; str.sprintf("Window %d", indices[i]);
            activeWindowPopup->insertItem(str, i);
            activeWindowPopup->setItemChecked(i, indices[i] == index);
        }
    }
    else
    {
        // Set the active item in the Active window combobox.
        if(index >= 0 && index < activeWindowComboBox->count())
            activeWindowComboBox->setCurrentItem(index);

        // Set the active item in the Active window menu.
        for(i = 0; i < activeWindowPopup->count(); ++i)
            activeWindowPopup->setItemChecked(i, index == i);
    }

    // Only enable the active window menu when there is more than one
    // vis window.
    winPopup->setItemEnabled(activeWindowPopupId, indices.size() > 1);
}

// ****************************************************************************
// Method: QvisMainWindow::UpdateWindowMenu
//
// Purpose: 
//   Updates the items in the window's sub-menus.
//
// Arguments:
//   updateNumbers : Tells the method to update the numbers in the copy menu.
//
// Programmer: Brad Whitlock
// Creation:   Fri Sep 13 10:19:30 PDT 2002
//
// Modifications:
//   Brad Whitlock, Tue Oct 15 16:17:57 PST 2002
//   I added another copy menu.
//
//   Brad Whitlock, Mon Nov 11 13:04:38 PST 2002
//   I added a lock menu.
//
//   Brad Whitlock, Wed May 21 07:46:24 PDT 2003
//   I added fullframe mode.
//
// ****************************************************************************

void
QvisMainWindow::UpdateWindowMenu(bool updateNumbers)
{
    const intVector &indices = globalAtts->GetWindows();

    //
    // Fill in the copy sub menus.
    //
    if(updateNumbers)
    {
        int i, j, windowIndex = globalAtts->GetActiveWindow();        

        // Update the copy menus
        for(i = 0; i < 5; ++i)
        {
            int n = 0;
            copyPopup[i]->clear();
            for(j = 0; j < indices.size(); ++j)
            {
                if(j == windowIndex)
                    continue;

                QString str;
                str.sprintf("Window %d", indices[j]);
                copyPopup[i]->insertItem(str, n++);
            }

            topCopyPopup->setItemEnabled(i, n > 0);
        }
    }

    // Set the enabled state of the copy and clear menus.
    bool enoughPlots = (plotList->GetNumPlots() > 0);
    bool enoughWindows = (indices.size() > 1);
    winPopup->setItemEnabled(topCopyPopupId, enoughWindows);
    topCopyPopup->setItemEnabled(copyPopupId[0], enoughPlots && enoughWindows);
    topCopyPopup->setItemEnabled(copyPopupId[1], enoughWindows);
    topCopyPopup->setItemEnabled(copyPopupId[2], enoughWindows);
    topCopyPopup->setItemEnabled(copyPopupId[3], enoughWindows);
    topCopyPopup->setItemEnabled(copyPopupId[4], enoughWindows);

    winPopup->setItemEnabled(clearPopupId, enoughPlots);
    winPopup->setItemEnabled(lockPopupId, enoughPlots);
    lockPopup->setItemChecked(lockTimeId, windowInfo->GetLockTime());
    lockPopup->setItemChecked(lockToolsId, windowInfo->GetLockTools());
    lockPopup->setItemChecked(lockViewId, windowInfo->GetLockView());

    winPopup->setItemEnabled(fullFrameModeId, enoughPlots);
    winPopup->setItemChecked(fullFrameModeId,
        windowInfo->GetFullFrame());
    winPopup->setItemEnabled(navigateModeId, enoughPlots);
    winPopup->setItemChecked(navigateModeId,
        windowInfo->GetBoundingBoxNavigate());
    winPopup->setItemEnabled(spinModeId, enoughPlots);
    winPopup->setItemChecked(spinModeId, windowInfo->GetSpin());
}

// ****************************************************************************
// Method: QvisMainWindow::ConnectGlobalAttributes
//
// Purpose: 
//   Registers the main window as an observer of the viewer proxy's
//   GlobalAttributes object.
//
// Arguments:
//   globalAtts_ : A pointer to the viewer proxy's GlobalAttributes
//                 object.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 13:23:20 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::ConnectGlobalAttributes(GlobalAttributes *globalAtts_)
{
    globalAtts = globalAtts_;
    globalAtts->Attach(this);

    // Update the global area.
    UpdateGlobalArea(true);
}

// ****************************************************************************
// Method: QvisMainWindow::ConnectViewerStatusAttributes
//
// Purpose: 
//   Connects the viewer's status attributes so the main window observes them.
//
// Arguments:
//   s : A pointer to the viewer's status attributes.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 2 15:24:36 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::ConnectViewerStatusAttributes(StatusAttributes *s)
{
    statusAtts = s;
    statusAtts->Attach(this);
}

// ****************************************************************************
// Method: QvisMainWindow::ConnectGUIMessageAttributes
//
// Purpose: 
//   Makes the window observe GUIBase's message attributes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 25 18:30:44 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::ConnectGUIMessageAttributes()
{
    msgAttr->Attach(this);
}

// ****************************************************************************
// Method: QvisMainWindow::ConnectPlotList
//
// Purpose: 
//   Makes the window observe the viewer's plot list.
//
// Arguments:
//   pl : A pointer to the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 12 18:04:23 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::ConnectPlotList(PlotList *pl)
{
    plotList = pl;
    plotList->Attach(this);
}

// ****************************************************************************
// Method: QvisMainWindow::ConnectWindowInformation
//
// Purpose: 
//   Makes the window observe the viewer's window information.
//
// Arguments:
//   w : A pointer to the window information.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 15:55:45 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::ConnectWindowInformation(WindowInformation *w)
{
    windowInfo = w;
    windowInfo->Attach(this);
}

// ****************************************************************************
// Method: QvisMainWindow::SubjectRemoved
//
// Purpose: 
//   Unregisters the main window as an observer of TheRemovedSubject.
//
// Arguments:
//   TheRemovedSubject : The subject that we're no longer observing.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 13:24:39 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Apr 30 15:18:36 PST 2001
//   Added code to remove subjects that were missing.
//
//   Brad Whitlock, Wed Sep 19 15:45:08 PST 2001
//   I changed a misplaced assignment into a comparison.
//
//   Brad Whitlock, Thu Oct 25 18:31:44 PST 2001
//   Added a new subject.
//
//   Brad Whitlock, Thu Sep 12 18:05:05 PST 2002
//   Added the plot list and the window info.
//
// ****************************************************************************

void
QvisMainWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == globalAtts)
        globalAtts = 0;
    else if(TheRemovedSubject == viewerMessageAtts)
        viewerMessageAtts = 0;
    else if(TheRemovedSubject == fileserverMessageAtts)
        fileserverMessageAtts = 0;
    else if(TheRemovedSubject == msgAttr)
        msgAttr = 0;
    else if(TheRemovedSubject == statusAtts)
        statusAtts = 0;
    else if(TheRemovedSubject == plotList)
        plotList = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

// ****************************************************************************
// Method: QvisMainWindow::GetNotepad
//
// Purpose: 
//   Returns a pointer to the notepad area so postable windows can
//   know the widget that they post to.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 09:44:24 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisNotepadArea *
QvisMainWindow::GetNotepad()
{
    return notepad;
}

// ****************************************************************************
// Method: QvisMainWindow::GetPlotManager
//
// Purpose: 
//   Returns a pointer to the main window's plot manager widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 7 12:48:07 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

QvisPlotManagerWidget *
QvisMainWindow::GetPlotManager()
{
    return plotManager;
}

// ****************************************************************************
// Method: QvisMainWindow::SetOrientation
//
// Purpose: 
//   Set the orientation to vertical or horizontal.
//
// Arguments:
//   vertical : Whether or not to set the window to a vertical layout.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 12:50:49 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::SetOrientation(int orientation)
{
    topLayout->setDirection((orientation < 2) ? QBoxLayout::TopToBottom :
                            QBoxLayout::LeftToRight);
}

//
// Event handlers
//

// ****************************************************************************
// Method: QvisMainWindow::closeEvent
//
// Purpose: 
//   This is a Qt slot function that tells the application that it
//   is okay to quit when a close event is received.
//
// Arguments:
//   ce : The close event.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul 26 09:45:08 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 13:54:35 PST 2001
//   Added code to tell the application to quit.
//
// ****************************************************************************

void
QvisMainWindow::closeEvent( QCloseEvent* ce )
{
    // Accept the close event and exit the program.
    ce->accept();

    // Tell the application to quit.
    qApp->quit();
}

// ****************************************************************************
// Method: QvisMainWindow::hideEvent
//
// Purpose: 
//   This is an event handler that emits an iconifyWindows signal if the
//   hide event was initiated by the window manager.
//
// Arguments:
//   e : The hide event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:40:34 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::hideEvent(QHideEvent *e)
{
    // Hide the window
    QMainWindow::hideEvent(e);

    // If the event was caused by the window manager, emit a signal.
    if(e->spontaneous())
        emit iconifyWindows();
}

// ****************************************************************************
// Method: QvisMainWindow::showEvent
//
// Purpose: 
//   This is an event handler that emits a deIconifyWindows signal if the
//   show event was initiated by the window manager.
//
// Arguments:
//   e : The show event.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 19 10:40:34 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::showEvent(QShowEvent *e)
{
    // Show the window.
    QMainWindow::showEvent(e);

    // If the event was caused by the window manager, emit a signal.
    if(e->spontaneous())
        emit deIconifyWindows();
}

//
// Qt slot functions
//

void
QvisMainWindow::windowAdd()
{
    // Tell the viewer to add a window
    viewer->AddWindow();
}

void
QvisMainWindow::windowClone()
{
    // Tell the viewer to clone a window
    viewer->CloneWindow();
}

void
QvisMainWindow::windowDelete()
{
    // Tell the viewer to delete a window
    viewer->DeleteWindow();
}

void
QvisMainWindow::windowClearAll()
{
    // Tell the viewer to clear all windows.
    viewer->ClearAllWindows();    
}

void
QvisMainWindow::windowLayout1x1()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(1);
}

void
QvisMainWindow::windowLayout1x2()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(2);
}

void
QvisMainWindow::windowLayout2x2()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(4);
}

void
QvisMainWindow::windowLayout2x4()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(8);
}

void
QvisMainWindow::windowLayout3x3()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(9);
}

void
QvisMainWindow::windowLayout4x4()
{
    // Tell the viewer to change the layout.
    viewer->SetWindowLayout(16);
}

// ****************************************************************************
// Method: QvisMainWindow::emitActivateOutputWindow
//
// Purpose: 
//   This is a Qt slot function that is called when the window's
//   output button is clicked. It sets the outputButton's pixmap to
//   the blue icon and emits a signal to activate the output window.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 12:16:03 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::emitActivateOutputWindow()
{
    // Indicate that the output was read.
    if(unreadOutputFlag)
    {
        unreadOutputFlag = false;
        outputButton->setPixmap(*outputBlue);
    }

    // Activate the output window.
    emit activateOutputWindow();
}

// ****************************************************************************
// Method: QvisMainWindow::IndicateUnreadOutput
//
// Purpose: 
//   This is a Qt slot function that tells the main window to set the output
//   button's icon.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 11:50:35 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::unreadOutput(bool val)
{
    // Change to the read icon.
    unreadOutputFlag = val;
    if(val)
        outputButton->setPixmap(*outputRed);
    else
        outputButton->setPixmap(*outputBlue);
}

// ****************************************************************************
// Method: QvisMainWindow::maintainViewToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the maintain view limits
//   checkbox is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Eric Brugger
// Creation:   Fri Apr 18 09:10:03 PDT 2003 
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::maintainViewToggled(bool)
{
    viewer->ToggleMaintainViewMode();
}

// ****************************************************************************
// Method: QvisMainWindow::replacePlotsToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the replace plots checkbox
//   is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 4 11:45:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::replacePlotsToggled(bool val)
{
    globalAtts->SetReplacePlots(val);
    globalAtts->Notify();
}

// ****************************************************************************
// Method: QvisMainWindow::autoUpdateToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the auto update check box
//   is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 4 11:45:12 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::autoUpdateToggled(bool val)
{
    globalAtts->SetAutoUpdateFlag(val);
    // Set GUIBase's autoupdate flag.
    autoUpdate = val;

    SetUpdate(false);
    globalAtts->Notify();
}

// ****************************************************************************
// Method: QvisMainWindow::winset
//
// Programmer: Brad Whitlock
//
// Modifications:
//   Eric Brugger, Fri Sep 22 10:05:24 PDT 2000
//   I modified the routine to work properly.
//
//   Brad Whitlock, Mon Mar 4 11:46:27 PDT 2002
//   Added code to prevent updates to this window.
//
// ****************************************************************************

void
QvisMainWindow::winset(int index)
{
    if(globalAtts->GetActiveWindow() != index)
    {
        globalAtts->SetActiveWindow(index);
        SetUpdate(false);
        globalAtts->Notify();

        // Tell the viewer...
        viewer->SetActiveWindow(globalAtts->GetWindows()[index]);
    }
}

// ****************************************************************************
// Method: QvisMainWindow::winset2
//
// Purpose: 
//   This is a Qt slot function that is called when an item in the Window
//   menu's "Active window" menu is clicked.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:22:55 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::winset2(int id)
{
    QString name(activeWindowPopup->text(id));
    name = name.right(name.length() - 7);
    int index = name.toInt() - 1;
    winset(index);
}

// ****************************************************************************
// Method: QvisMainWindow::copyView
//
// Purpose: 
//   This is a Qt slot function that causes the view from another window to
//   be copied to the active window.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:23:59 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::copyView(int id)
{
    QString name(copyPopup[0]->text(id));
    name = name.right(name.length() - 7);
    int from = name.toInt();
    int to = globalAtts->GetActiveWindow() + 1;
    viewer->CopyViewToWindow(from, to);
}

// ****************************************************************************
// Method: QvisMainWindow::copyLighting
//
// Purpose: 
//   This is a Qt slot function that causes the lighting from another window
//   to be copied to the active window.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:23:59 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::copyLighting(int id)
{
    QString name(copyPopup[1]->text(id));
    name = name.right(name.length() - 7);
    int from = name.toInt();
    int to = globalAtts->GetActiveWindow() + 1;
    viewer->CopyLightingToWindow(from, to);
}

// ****************************************************************************
// Method: QvisMainWindow::copyAnnotations
//
// Purpose: 
//   This is a Qt slot function that causes the annotations from another
//   window to be copied to the active window.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:23:59 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::copyAnnotations(int id)
{
    QString name(copyPopup[2]->text(id));
    name = name.right(name.length() - 7);
    int from = name.toInt();
    int to = globalAtts->GetActiveWindow() + 1;
    viewer->CopyAnnotationsToWindow(from, to);
}

// ****************************************************************************
// Method: QvisMainWindow::copyAnnotations
//
// Purpose: 
//   This is a Qt slot function that causes the plots from another
//   window to be copied to the active window.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 15 16:15:46 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::copyPlots(int id)
{
    QString name(copyPopup[3]->text(id));
    name = name.right(name.length() - 7);
    int from = name.toInt();
    int to = globalAtts->GetActiveWindow() + 1;
    viewer->CopyPlotsToWindow(from, to);
}

// ****************************************************************************
// Method: QvisMainWindow::copyView
//
// Purpose: 
//   This is a Qt slot function that causes the view, annotations, lighting
//   from another window to be copied to the active window.
//
// Arguments:
//   id : The id of the button that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:23:59 PST 2002
//
// Modifications:
//   Brad Whitlock, Tue Oct 15 16:15:18 PST 2002
//   Added plot copying.
//
// ****************************************************************************

void
QvisMainWindow::copyAll(int id)
{
    QString name(copyPopup[4]->text(id));
    name = name.right(name.length() - 7);
    int from = name.toInt();
    int to = globalAtts->GetActiveWindow() + 1;
    viewer->DisableRedraw();
    viewer->CopyViewToWindow(from, to);
    viewer->CopyLightingToWindow(from, to);
    viewer->CopyAnnotationsToWindow(from, to);
    viewer->CopyPlotsToWindow(from, to);
    viewer->RedrawWindow();
}

// ****************************************************************************
// Method: QvisMainWindow::clearPlots
//
// Purpose: 
//   This is a Qt slot function that clears the plots from the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:25:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::clearPlots()
{
    viewer->ClearWindow();
}

// ****************************************************************************
// Method: QvisMainWindow::clearReferenceLines
//
// Purpose: 
//   This is a Qt slot function that clears the reflines from the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:25:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::clearReferenceLines()
{
    viewer->ClearReferenceLines();
}

// ****************************************************************************
// Method: QvisMainWindow::clearPickPoints
//
// Purpose: 
//   This is a Qt slot function that clears the picks from the active window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:25:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::clearPickPoints()
{
    viewer->ClearPickPoints();
}

// ****************************************************************************
// Method: QvisMainWindow::toggleNavigateMode
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   bounding box navigation mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:25:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::toggleNavigateMode()
{
    viewer->ToggleBoundingBoxMode();
}

// ****************************************************************************
// Method: QvisMainWindow::toggleSpinMode
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   spin mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 16 17:25:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************


void
QvisMainWindow::toggleSpinMode()
{
    viewer->ToggleSpinMode();
}

// ****************************************************************************
// Method: QvisMainWindow::toggleFullFrameMode
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   fullframe mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 07:44:49 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::toggleFullFrameMode()
{
    viewer->ToggleFullFrameMode();
}

// ****************************************************************************
// Method: QvisMainWindow::lockTime
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   lockTime mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 13:02:58 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::lockTime()
{
    viewer->ToggleLockTime();
}

// ****************************************************************************
// Method: QvisMainWindow::lockTools
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   lockTools mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 13:02:58 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::lockTools()
{
    viewer->ToggleLockTools();
}

// ****************************************************************************
// Method: QvisMainWindow::lockView
//
// Purpose: 
//   This is a Qt slot function that causes the active window to toggle its
//   lockView mode.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 11 13:02:58 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::lockView()
{
    viewer->ToggleLockViewMode();
}
