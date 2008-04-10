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

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>

#include <QvisMainWindow.h>
#include <QvisFilePanel.h>
#include <QvisNotepadArea.h>
#include <QvisPostableWindow.h>
#include <QvisPlotManagerWidget.h>

#include <DataNode.h>
#include <StatusSubject.h>
#include <FileServerList.h>
#include <GlobalAttributes.h>
#include <MessageAttributes.h>
#include <NameSimplifier.h>
#include <PlotList.h>
#include <StatusAttributes.h>
#include <TimeFormat.h>
#include <WindowInformation.h>
#include <ViewerProxy.h>
#include <DebugStream.h>

#define VIEWER_NOT_IMPLEMENTED

#include <icons/expression.xpm>
#include <icons/computer.xpm>
#include <icons/fileopen.xpm>
#include <icons/filesave.xpm>
#include <icons/fileprint.xpm>
#include <icons/animate.xpm>
#include <icons/annot.xpm>
#include <icons/command.xpm>
#include <icons/correlation.xpm>
#include <icons/globallineout.xpm>
#include <icons/light.xpm>
#include <icons/keyframe.xpm>
#include <icons/matoptions.xpm>
#include <icons/pick.xpm>
#include <icons/plugin.xpm>
#include <icons/rainbow.xpm>
#include <icons/savemovie.xpm>
#include <icons/subset.xpm>
#include <icons/view.xpm>
#include <icons/output_blue.xpm>
#include <icons/output_red.xpm>

#include <icons/layout1x1.xpm>
#include <icons/layout1x2.xpm>
#include <icons/layout2x2.xpm>
#include <icons/layout2x3.xpm>
#include <icons/layout2x4.xpm>
#include <icons/layout3x3.xpm>
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
//    Brad Whitlock, Fri Aug 15 15:05:24 PST 2003
//    I passed a pointer to the menu bar to the plot manager widget.
//
//    Kathleen Bonnell, Tue Aug 26 13:47:34 PDT 2003 
//    Changed 'Material' to 'Material Options'.
//
//    Brad Whitlock, Sat Jan 24 23:51:40 PST 2004
//    I added support for next generation file handling, including close
//    and reopen menus.
//
//    Kathleen Bonnell, Wed Mar 31 10:13:43 PST 2004 
//    I added query over time window.
//
//    Brad Whitlock, Mon Apr 5 15:28:04 PST 2004
//    I added support for putting reopen and close in an "Advanced"
//    pullright menu.
//
//    Kathleen Bonnell, Thu Jun 17 13:50:58 PDT 2004 
//    Changed QueryOverTime's accelerator key from CTL-T to CTL-SHIFT-Q
//    so that it would not collide with the ColorTable's accelerator. 
// 
//    Kathleen Bonnell, Wed Aug 18 09:44:09 PDT 2004 
//    Added Interactors window.
//
//    Jeremy Meredith, Wed Aug 25 09:52:06 PDT 2004
//    Connect the viewer proxy's database metadata to the plotmanager.
//
//    Brad Whitlock, Wed Feb 9 17:49:22 PST 2005
//    Added a menu option to update VisIt.
//
//    Brad Whitlock, Mon Mar 21 15:17:31 PST 2005
//    I enabled the save movie option, the command window, and made quitting
//    emit a quit signal.
//
//    Jeremy Meredith, Mon Apr  4 16:35:02 PDT 2005
//    Added the simulations window to the menu.
//
//    Hank Childs, Tue May 24 17:02:15 PDT 2005
//    Added export database.
//
//    Eric Brugger, Thu Jun 30 09:17:12 PDT 2005
//    Added a 2x3 window layout and removed the 4x4 window layout.
//
//    Brad Whitlock, Fri Sep 9 11:50:55 PDT 2005
//    Added more icons.
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added mesh management attributes window
//
//    Brad Whitlock, Mon Mar 13 17:19:28 PST 2006
//    Changed print option to Ctrl+P.
//
//    Brad Whitlock, Mon Jul 24 17:43:18 PST 2006
//    Added splitters.
//
//    Jeremy Meredith, Mon Aug 28 17:28:42 EDT 2006
//    Added File Open window.
//
//    Brad Whitlock, Tue Nov 14 15:35:44 PST 2006
//    I added restoreSessionWithSources.
//
//    Brad Whitlock, Fri Jun 15 09:32:23 PDT 2007
//    Added Macro window.
//
//    Brad Whitlock, Tue Jan 22 16:49:30 PST 2008
//    Don't even create the notepad on short displays.
//
//    Brad Whitlock, Thu Jan 31 10:45:58 PST 2008
//    Added crash recovery timer.
//
//    Sean Ahern, Wed Feb 27 20:32:55 EST 2008
//    Made the Mac use Command-Q to quit, just like other Mac applications.
//    I had to move Query and Query-over-time to use 'Y' instead.
//
// ****************************************************************************

QvisMainWindow::QvisMainWindow(int orientation, const char *captionString)
    : QvisWindowBase(captionString, WDestructiveClose)
{
    int     id;
    QPixmap openIcon, saveIcon, computerIcon, printIcon, rainbowIcon;
    QPixmap annotIcon, lightIcon, subsetIcon, viewIcon;
    QPixmap exprIcon, animIcon, pluginIcon, pickIcon, copyIcon, lockIcon;
    QPixmap saveMovieIcon, commandIcon, keyframeIcon, materialIcon;
    QPixmap globalLineoutIcon, correlationIcon;

    // Make the main window observe the global status subject. This is
    // part of the mechanism that allows other windows to display status
    // messages in the main window's status bar.
    statusSubject->Attach(this);

    // Make the main window observe the viewer's message attributes. This is
    // how messages from the viewer get put into the error window.
    viewerMessageAtts = GetViewerState()->GetMessageAttributes();
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
    saveMovieIcon = QPixmap(savemovie_xpm);
    commandIcon = QPixmap(command_xpm);
    keyframeIcon = QPixmap(keyframe_xpm);
    materialIcon = QPixmap(matoptions_xpm);
    globalLineoutIcon = QPixmap(globallineout_xpm);
    correlationIcon = QPixmap(correlation_xpm);

    outputBlue = new QPixmap( output_blue_xpm );
    outputRed = new QPixmap( output_red_xpm );

    //
    // Add the File menu.
    //
    filePopup = new QPopupMenu( this );
    menuBar()->insertItem( tr("&File"), filePopup );
    filePopup->insertItem(openIcon, tr("Select &file . . ."), this, SIGNAL(activateFileWindow()), CTRL+Key_F );
    filePopup->insertItem(openIcon, tr("Open file . . ."), this, SIGNAL(activateFileOpenWindow()), CTRL+Key_O);

    // Advanced pull-right menu.
    fileAdvancedPopup = new QPopupMenu(filePopup, "fileAdvancedPopup");
    fileAdvancedPopupId = filePopup->insertItem(tr("Advanced file options"), fileAdvancedPopup);
    advancedMenuShowing = true;

    // ReOpen pull-right menu.
    reopenPopup = new QPopupMenu(fileAdvancedPopup, "reopenPopup");
    connect(reopenPopup, SIGNAL(activated(int)),
            this, SLOT(reopenFile(int)));
    reopenPopupId = fileAdvancedPopup->insertItem(tr("ReOpen file"), reopenPopup);
    fileAdvancedPopup->setItemEnabled(reopenPopupId, false);

    // Close pull-right menu
    closePopup = new QPopupMenu(fileAdvancedPopup, "closePopup");
    connect(closePopup, SIGNAL(activated(int)),
            this, SLOT(closeFile(int)));
    closePopupId = fileAdvancedPopup->insertItem(tr("Close file"), closePopup);
    fileAdvancedPopup->setItemEnabled(closePopupId, false);

    filePopup->insertItem( tr("Refresh file list"), this, SIGNAL(refreshFileList()), CTRL+Key_R);
    filePopup->insertItem( tr("File &information . . ."), this, SIGNAL(activateFileInformationWindow()), CTRL+Key_I);
    filePopup->insertItem( tr("Compute &engines . . ."), this, SIGNAL(activateEngineWindow()), CTRL+Key_E);
    filePopup->insertItem( tr("Simulations . . ."), this, SIGNAL(activateSimulationWindow()), CTRL+SHIFT+Key_S);
    filePopup->insertSeparator();
    filePopup->insertItem(saveIcon, tr("&Save window"), this, SIGNAL(saveWindow()), CTRL+Key_S );
    filePopup->insertItem( tr("Set Save &options . . ."), this, SIGNAL(activateSaveWindow()), CTRL+SHIFT+Key_O);
    filePopup->insertItem(saveMovieIcon, tr("Save movie . . ."), this, SIGNAL(saveMovie()));
    id = filePopup->insertItem(tr("Export database . . ."), this, SIGNAL(activateExportDBWindow()));
    id = filePopup->insertItem(printIcon, tr("Print window"), this, SIGNAL(printWindow()));
    id = filePopup->insertItem(tr("Set Print options . . ."), this, SIGNAL(activatePrintWindow()), CTRL+Key_P);
    filePopup->insertSeparator();
    id = filePopup->insertItem(tr("Restore session . . ."), this, SIGNAL(restoreSession()));
    id = filePopup->insertItem(tr("Restore session with sources . . ."), this, SIGNAL(restoreSessionWithSources()));
    id = filePopup->insertItem(tr("Save session . . ."), this, SIGNAL(saveSession()));
    filePopup->insertSeparator();
#ifdef Q_WS_MACX
    filePopup->insertItem( tr("&Quit"), this, SIGNAL(quit()), CTRL+Key_Q );
#else
    filePopup->insertItem( tr("E&xit"), this, SIGNAL(quit()), CTRL+Key_X );
#endif

    //
    // Add the Controls menu.
    //
    QPopupMenu *ctrls = new QPopupMenu(this);
    menuBar()->insertItem( tr("&Controls"), ctrls);
    id = ctrls->insertItem(animIcon, tr("&Animation . . ."), this, SIGNAL(activateAnimationWindow()), CTRL+Key_A);
    id = ctrls->insertItem(annotIcon, tr("A&nnotation . . ."), this, SIGNAL(activateAnnotationWindow()), CTRL+Key_N);
    id = ctrls->insertItem(rainbowIcon, tr("Color &table . . ."), this, SIGNAL(activateColorTableWindow()), CTRL+Key_T);
    id = ctrls->insertItem(commandIcon, tr("Command . . ."), this, SIGNAL(activateCommandWindow()), CTRL+SHIFT+Key_C);
    id = ctrls->insertItem(correlationIcon, tr("&Database correlations . . ."), this, SIGNAL(activateCorrelationListWindow()), CTRL+Key_D);
    id = ctrls->insertItem(exprIcon, tr("&Expressions . . ."), this, SIGNAL(activateExpressionsWindow()), CTRL+SHIFT+Key_E );
    id = ctrls->insertItem(keyframeIcon, tr("&Keyframing . . ."), this, SIGNAL(activateKeyframeWindow()), CTRL+Key_K);
    id = ctrls->insertItem(lightIcon, tr("&Lighting . . ."), this, SIGNAL(activateLightingWindow()), CTRL+Key_L );
    id = ctrls->insertItem(globalLineoutIcon, tr("&Lineout . . ."), this, SIGNAL(activateGlobalLineoutWindow()), CTRL+SHIFT+Key_L );
    id = ctrls->insertItem(tr("Macros . . ."), this, SIGNAL(activateMacroWindow()));
    id = ctrls->insertItem(materialIcon, tr("&Material Options . . ."), this, SIGNAL(activateMaterialWindow()), CTRL+Key_M);
    id = ctrls->insertItem(tr("&Mesh management . . ."), this, SIGNAL(activateMeshManagementWindow()), CTRL+SHIFT+Key_M);
    id = ctrls->insertItem(pickIcon, tr("&Pick . . ."), this, SIGNAL(activatePickWindow()), CTRL+SHIFT+Key_P );
#ifdef Q_WS_MACX
    id = ctrls->insertItem(tr("Quer&y . . ."), this, SIGNAL(activateQueryWindow()), CTRL+Key_Y );
    id = ctrls->insertItem(tr("Quer&y over time options . . ."), this, SIGNAL(activateQueryOverTimeWindow()), CTRL+SHIFT+Key_Y );
#else
    id = ctrls->insertItem(tr("&Query over time options . . ."), this, SIGNAL(activateQueryOverTimeWindow()), CTRL+SHIFT+Key_Q );
    id = ctrls->insertItem(tr("&Query . . ."), this, SIGNAL(activateQueryWindow()), CTRL+Key_Q );
#endif
    id = ctrls->insertItem(subsetIcon, tr("S&ubset . . ."), this, SIGNAL(activateSubsetWindow()), CTRL+Key_U);
    id = ctrls->insertItem(viewIcon, tr("&View . . ."), this, SIGNAL(activateViewWindow()), CTRL+Key_V);

    //
    // Add the Options menu.
    //
    QPopupMenu * pref = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Options"), pref );
    pref->insertItem( tr("&Appearance . . ."), this, SIGNAL(activateAppearanceWindow()), CTRL+SHIFT+Key_A);
    pref->insertItem(computerIcon, tr("&Host profiles . . ."), this, SIGNAL(activateHostWindow()), CTRL+Key_H );
    pref->insertItem(tr("&Interactors . . ."), this, SIGNAL(activateInteractorWindow()), CTRL+SHIFT+Key_I );
    pref->insertItem(pluginIcon, tr("&Plugin Manager . . ."), this, SIGNAL(activatePluginWindow()));
    pref->insertItem(tr("Preferences . . ."), this, SIGNAL(activatePreferencesWindow()));
    pref->insertItem(tr("Rendering . . ."), this, SIGNAL(activateRenderingWindow()));
    pref->insertSeparator();
    pref->insertItem( tr("Save Settings"), this, SIGNAL(saveSettings()));

    //
    // Add the Windows menu.
    //
    winPopup = new QPopupMenu( this );
    menuBar()->insertItem( tr("&Windows"), winPopup );
    winPopup->insertItem(QPixmap(newwindow_xpm), tr("Add"), this, SLOT(windowAdd())
#ifndef Q_WS_MACX
                         , CTRL+Key_Insert
#endif
                        );
    winPopup->insertItem(copyIcon, tr("Clone"), this, SLOT(windowClone()));
    winPopup->insertItem(QPixmap(deletewindow_xpm), tr("Delete"), this, SLOT(windowDelete()), CTRL+Key_Delete);
    winPopup->insertItem( tr("Clear all"), this, SLOT(windowClearAll()));
    // Layout sub menu
    layoutPopup = new QPopupMenu( winPopup );
    layoutPopup->insertItem(QPixmap(layout1x1_xpm), tr("1x1"), this, SLOT(windowLayout1x1()), 0, 0);
    layoutPopup->insertItem(QPixmap(layout1x2_xpm), tr("1x2"), this, SLOT(windowLayout1x2()), 0, 1);
    layoutPopup->insertItem(QPixmap(layout2x2_xpm), tr("2x2"), this, SLOT(windowLayout2x2()), 0, 2);
    layoutPopup->insertItem(QPixmap(layout2x3_xpm), tr("2x3"), this, SLOT(windowLayout2x3()), 0, 3);
    layoutPopup->insertItem(QPixmap(layout2x4_xpm), tr("2x4"), this, SLOT(windowLayout2x4()), 0, 4);
    layoutPopup->insertItem(QPixmap(layout3x3_xpm), tr("3x3"), this, SLOT(windowLayout3x3()), 0, 5);
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
    copyPopupId[0] = topCopyPopup->insertItem(tr("View from"), copyPopup[0], 0);
    topCopyPopup->setItemEnabled(copyPopupId[0], false);
    copyPopup[1] = new QPopupMenu(topCopyPopup, "copyLighting");
    connect(copyPopup[1], SIGNAL(activated(int)),
            this, SLOT(copyLighting(int)));
    copyPopupId[1] = topCopyPopup->insertItem(tr("Lighting from"), copyPopup[1], 1);
    topCopyPopup->setItemEnabled(copyPopupId[1], false);
    copyPopup[2] = new QPopupMenu(topCopyPopup, "copyAnnotations");
    connect(copyPopup[2], SIGNAL(activated(int)),
            this, SLOT(copyAnnotations(int)));
    copyPopupId[2] = topCopyPopup->insertItem(tr("Annotations from"), copyPopup[2], 2);
    topCopyPopup->setItemEnabled(copyPopupId[2], false);
    copyPopup[3] = new QPopupMenu(topCopyPopup, "copyPlots");
    connect(copyPopup[3], SIGNAL(activated(int)),
            this, SLOT(copyPlots(int)));
    copyPopupId[3] = topCopyPopup->insertItem(tr("Plots from"), copyPopup[3], 3);
    topCopyPopup->setItemEnabled(copyPopupId[3], false);
    copyPopup[4] = new QPopupMenu(topCopyPopup, "copyAll");
    connect(copyPopup[4], SIGNAL(activated(int)),
            this, SLOT(copyAll(int)));
    copyPopupId[4] = topCopyPopup->insertItem(tr("Everything from"), copyPopup[4], 4);
    topCopyPopup->setItemEnabled(copyPopupId[4], false);
    topCopyPopupId = winPopup->insertItem(copyIcon, tr("Copy"), topCopyPopup);
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
    lockPopup->insertSeparator();
    lockPopup->insertItem(tr("Unlock everything"), this, SLOT(unlockEverything()));
    lockPopupId = winPopup->insertItem(lockIcon, tr("Lock"), lockPopup);

    // Other options.
    fullFrameModeId = winPopup->insertItem(tr("Full frame"), this, SLOT(toggleFullFrameMode()));
    navigateModeId = winPopup->insertItem(tr("Navigate bbox"), this, SLOT(toggleNavigateMode()));
    spinModeId = winPopup->insertItem(tr("Spin"), this, SLOT(toggleSpinMode()));
    
    //
    // Add the Help menu
    //
    helpPopup = new QPopupMenu( this );
    menuBar()->insertSeparator();
    menuBar()->insertItem( tr("&Help"), helpPopup);
    helpPopup->insertItem( tr("About . . ."), this, SIGNAL(activateAboutWindow()));
    helpPopup->insertItem( tr("Copyright . . ."), this, SIGNAL(activateCopyrightWindow()));
    helpPopup->insertItem( tr("Help . . ."), this, SIGNAL(activateHelpWindow()), Key_F1);
    helpPopup->insertItem( tr("Release notes . . ."), this, SIGNAL(activateReleaseNotesWindow()));
    helpPopup->insertSeparator();
    updateVisItId = helpPopup->insertItem( tr("Check for new version . . ."), this, SIGNAL(updateVisIt()));

    // Make a central widget to contain the other widgets
    splitter = new QSplitter(this);
    splitter->setOrientation(QSplitter::Vertical);
    setCentralWidget(splitter);

    //
    // Create the file panel and make it an observer of the file server.
    //
    QVBox *panel1 = new QVBox(splitter);
    panel1->setMargin(5);
    filePanel = new QvisFilePanel(panel1, "FilePanel");
    connect(filePanel, SIGNAL(reopenOnNextFrame()),
            this, SIGNAL(reopenOnNextFrame()));
    filePanel->ConnectFileServer(fileServer);
    filePanel->ConnectWindowInformation(GetViewerState()->GetWindowInformation());

    // Create the global area.
    QVBox *panel2 = new QVBox(splitter);
    panel2->setMargin(5);
    QWidget *topOfHBox = new QWidget(panel2);
    CreateGlobalArea(topOfHBox);
    splitter->setResizeMode(panel2,QSplitter::Stretch);

    // Create the plot Manager.
    plotManager = new QvisPlotManagerWidget(menuBar(), panel2, "plotManager");
    plotManager->ConnectPlotList(GetViewerState()->GetPlotList());
    plotManager->ConnectFileServer(fileServer);
    plotManager->ConnectGlobalAttributes(GetViewerState()->GetGlobalAttributes());
    plotManager->ConnectExpressionList(GetViewerState()->GetExpressionList());
    plotManager->ConnectPluginManagerAttributes(GetViewerState()->GetPluginManagerAttributes());
    plotManager->ConnectWindowInformation(GetViewerState()->GetWindowInformation());
    plotManager->ConnectDatabaseMetaData(GetViewerState()->GetDatabaseMetaData());

    QValueList<int> splitterSizes;
    int nVisiblePanels = 2;

    if(qApp->desktop()->height() < 1024)
    {
        // No notepad
        notepad = 0;

        debug1 << "The screen's vertical resolution is less than 1024 "
                  "so the notepad will not be available." << endl;
        QvisPostableWindow::SetPostEnabled(false);
    }
    else 
    {
        // Create the notepad widget. Use a big stretch factor so the
        // notpad widget will fill all the remaining space.
        notepad = new QvisNotepadArea( splitter );
        ++nVisiblePanels;
    }

    // May want to read these from the config file but here are the defaults.
    int hgt = qApp->desktop()->height();
    if(nVisiblePanels == 2)
    {
        splitterSizes.push_back(int(hgt * 0.5));
        splitterSizes.push_back(int(hgt * 0.5));
    }
    else
    {
        splitterSizes.push_back(int(hgt * 0.3));
        splitterSizes.push_back(int(hgt * 0.3));
        splitterSizes.push_back(int(hgt * 0.4));
    }
    splitter->setSizes(splitterSizes);

    // Create the output button and put it in the status bar as a
    // permanent widget.
    outputButton = new QPushButton(statusBar(), "outputButton");
    connect(outputButton, SIGNAL(clicked()), this, SLOT(emitActivateOutputWindow()));
    outputButton->setPixmap(*outputBlue);
    outputButton->setFixedSize(32 , 32);
    QToolTip::add(outputButton, tr("Output window"));
    statusBar()->addWidget(outputButton, 0, true);
    statusBar()->setSizeGripEnabled(false);
    unreadOutputFlag = false;

    // Create a timer.
    okayToSaveRecoveryFile = false;
    recoveryFileTimer = new QTimer(this);
    connect(recoveryFileTimer, SIGNAL(timeout()),
            this, SIGNAL(saveCrashRecoveryFile()));

#ifdef Q_WS_X11
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
//   Eric Brugger, Mon Mar 29 13:07:58 PST 2004
//   I added maintain data limits.
//
// ****************************************************************************

void
QvisMainWindow::CreateGlobalArea(QWidget *par)
{
    QGridLayout *globalLayout = new QGridLayout(par, 2, 7);
    globalLayout->setSpacing(0);
    globalLayout->setColStretch(2, 50);
    globalLayout->setColStretch(5, 50);

    activeWindowComboBox = new QComboBox(false, par,
        "activeWindowComboBox");
    connect(activeWindowComboBox, SIGNAL(activated(int)),
            this, SLOT(winset(int)));
    activeWindowComboBox->insertItem("1");
    QLabel *activeWindowLabel = new QLabel(activeWindowComboBox, 
       tr("Active window"), par, "activeWindowLabel");
    globalLayout->addMultiCellWidget(activeWindowLabel, 0, 0, 0, 1, Qt::AlignCenter);
    globalLayout->addMultiCellWidget(activeWindowComboBox, 1, 1, 0, 1);

    QLabel *maintainLabel = new QLabel(tr("Maintain limits"), par);
    globalLayout->addMultiCellWidget(maintainLabel, 0, 0, 3, 4, Qt::AlignCenter);

    maintainViewCheckBox = new QCheckBox(tr("view"), par,
        "maintainViewCheckBox");
    connect(maintainViewCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(maintainViewToggled(bool)));
    globalLayout->addWidget(maintainViewCheckBox, 1, 3);

    maintainDataCheckBox = new QCheckBox(tr("data"), par,
        "maintainDataCheckBox");
    connect(maintainDataCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(maintainDataToggled(bool)));
    globalLayout->addWidget(maintainDataCheckBox, 1, 4);

    replacePlotsCheckBox = new QCheckBox(tr("Replace plots"), par,
        "replacePlotsCheckBox");
    connect(replacePlotsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(replacePlotsToggled(bool)));
    globalLayout->addWidget(replacePlotsCheckBox, 0, 6);

    autoUpdateCheckBox = new QCheckBox(tr("Auto update"), par,
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
//   Brad Whitlock, Fri Jan 23 17:29:21 PST 2004
//   Made it work with the regenerated GlobalAttributes.
//
//   Brad Whitlock, Thu Jan 31 13:06:19 PST 2008
//   Added code to handle the crash recovery timer.
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
#ifdef Q_WS_X11
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
                statusMsg = QString(statusAtts->GetStatusMessage().c_str());
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

                QString done(tr("done"));
                QString ofStage(tr("of stage"));
                QString totalPct; totalPct.sprintf("%d%% ", total);
                QString progress; progress.sprintf(" (%d%% ", statusAtts->GetPercent());
                QString progress2; progress2.sprintf(" %d/%d)", statusAtts->GetCurrentStage(),
                    statusAtts->GetMaxStage());
                statusMsg = totalPct + done + ": " + QString(statusAtts->GetCurrentStageName().c_str())
                            + progress + ofStage + progress2;
            }

            statusBar()->message(statusMsg, statusAtts->GetDuration());
        }

        // Force the status bar to redraw.
        QPaintEvent *pe = new QPaintEvent(statusBar()->visibleRect(), true);
        QApplication::sendEvent(statusBar(), pe);
        delete pe;
#ifdef Q_WS_X11
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
        UpdateWindowMenu(globalAtts->IsSelected(GlobalAttributes::ID_windows) ||
                         globalAtts->IsSelected(GlobalAttributes::ID_activeWindow));

        // Set the crash recovery timer.
        if(okayToSaveRecoveryFile && globalAtts->IsSelected(GlobalAttributes::ID_saveCrashRecoveryFile))
            UpdateCrashRecoveryTimer();
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
//   Brad Whitlock, Fri Jan 23 17:32:11 PST 2004
//   I updated the code due to changes in GlobalAttributes. I also added code
//   to set the contents of the source list.
//
//   Eric Brugger, Mon Mar 29 13:07:58 PST 2004
//   I added maintain data limits.
//
//   Brad Whitlock, Mon Apr 5 16:19:18 PST 2004
//   Renamed a method.
//
//   Eric Brugger, Thu Jun 30 09:17:12 PDT 2005
//   Added a 2x3 window layout and removed the 4x4 window layout.
//
//   Brad Whitlock, Fri Dec 14 17:34:28 PST 2007
//   Made it use ids.
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
        case GlobalAttributes::ID_sources:
            UpdateFileMenuPopup(reopenPopup, reopenPopupId);
            UpdateFileMenuPopup(closePopup, closePopupId);
            break;
        case GlobalAttributes::ID_windows:
            UpdateWindowList(true);
            break;
        case GlobalAttributes::ID_activeWindow:
            UpdateWindowList(false);
            break;
        case GlobalAttributes::ID_iconifiedFlag:
            break;
        case GlobalAttributes::ID_autoUpdateFlag:
            autoUpdateCheckBox->blockSignals(true);
            autoUpdateCheckBox->setChecked(globalAtts->GetAutoUpdateFlag());
            autoUpdateCheckBox->blockSignals(false);

            // Set GUIBase's autoupdate flag.
            autoUpdate = globalAtts->GetAutoUpdateFlag();
            break;
        case GlobalAttributes::ID_replacePlots:
            replacePlotsCheckBox->blockSignals(true);
            replacePlotsCheckBox->setChecked(globalAtts->GetReplacePlots());
            replacePlotsCheckBox->blockSignals(false);
            break;
        case GlobalAttributes::ID_applyOperator:
            break;
        case GlobalAttributes::ID_executing:
            break;
        case GlobalAttributes::ID_windowLayout:
        { // new scope
            layoutPopup->setItemChecked(0,globalAtts->GetWindowLayout() == 1);
            layoutPopup->setItemChecked(1,globalAtts->GetWindowLayout() == 2);
            layoutPopup->setItemChecked(2,globalAtts->GetWindowLayout() == 4);
            layoutPopup->setItemChecked(3,globalAtts->GetWindowLayout() == 6);
            layoutPopup->setItemChecked(4,globalAtts->GetWindowLayout() == 8);
            layoutPopup->setItemChecked(5,globalAtts->GetWindowLayout() == 9);
            for(int j = 0; j < 6; ++j)
                layoutPopup->setItemEnabled(j, true);
        }
            break;
        case GlobalAttributes::ID_makeDefaultConfirm:
            // Set GUIBase's makeDefaultConfirm flag.
            makeDefaultConfirm = globalAtts->GetMakeDefaultConfirm();
            break;
        case GlobalAttributes::ID_cloneWindowOnFirstRef:
            break;
        case GlobalAttributes::ID_maintainView:
            maintainViewCheckBox->blockSignals(true);
            maintainViewCheckBox->setChecked(globalAtts->GetMaintainView());
            maintainViewCheckBox->blockSignals(false);
            break;
        case GlobalAttributes::ID_maintainData:
            maintainDataCheckBox->blockSignals(true);
            maintainDataCheckBox->setChecked(globalAtts->GetMaintainData());
            maintainDataCheckBox->blockSignals(false);
            break;
        case GlobalAttributes::ID_automaticallyAddOperator:
            break;
        }
    } // end for
}

// ****************************************************************************
// Method: QvisMainWindow::UpdateFileMenuPopup
//
// Purpose: 
//   Updates the contents of the specified file menu.
//
// Arguments:
//   menu   : The menu to update.
//   menuId : The menu's id in the file menu.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 10:49:30 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 15:33:39 PST 2004
//   I renamed the method.
//
// ****************************************************************************

void
QvisMainWindow::UpdateFileMenuPopup(QPopupMenu *m, int menuId)
{
    stringVector simpleNames;
    const stringVector &sources = globalAtts->GetSources();

    // Simplify the names in the list.
    NameSimplifier simple;
    int i;
    for(i = 0; i < sources.size(); ++i)
        simple.AddName(sources[i]);
    simple.GetSimplifiedNames(simpleNames);

    // Clear out the old list and add the new list.
    m->clear();
    for(i = 0; i < simpleNames.size(); ++i)
        m->insertItem(simpleNames[i].c_str(), i, i);

    //
    // Set the menu's enabled state.
    //
    bool menuEnabled = (m->count() > 0);
    if(m->parent()->inherits("QPopupMenu"))
    {
        QPopupMenu *p = (QPopupMenu *)m->parent();
        if(p->isItemEnabled(menuId) != menuEnabled)
            p->setItemEnabled(menuId, menuEnabled);
    }
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
//   Brad Whitlock, Fri Apr 15 09:42:58 PDT 2005
//   I added some code that prevents an update unless it is really needed.
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
        intVector oldIndices;
        for(i = 0; i < activeWindowComboBox->count(); ++i)
            oldIndices.push_back(activeWindowComboBox->text(i).toInt());

        // Update the Active window combo box.
        if(oldIndices != indices)
        {
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
                QString str; str.sprintf("%d", indices[i]);
                activeWindowPopup->insertItem(tr("Window ") + str, i);
                activeWindowPopup->setItemChecked(i, indices[i] == index);
            }
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
                str.sprintf("%d", indices[j]);
                copyPopup[i]->insertItem(tr("Window ") + str, n++);
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
//   Brad Whitlock, Mon Jul 24 17:43:44 PST 2006
//   I made it use a splitter.
//
// ****************************************************************************

void
QvisMainWindow::SetOrientation(int orientation)
{
    splitter->setOrientation((orientation < 2) ? QSplitter::Vertical :
                             QBoxLayout::Horizontal);
}

// ****************************************************************************
// Method: QvisMainWindow::CreateNode
//
// Purpose: 
//   Saves the main window's attributes to the config file.
//
// Arguments:
//   parentNode : The node in which to save the attributes.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 10:14:53 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Sep 19 12:06:45 PDT 2006
//   Compensate for window decorations on the Mac.
//
// ****************************************************************************

void
QvisMainWindow::CreateNode(DataNode *parentNode)
{
    DataNode *node = new DataNode("MainWin");
    parentNode->AddNode(node);

    // Add generic window attributes
#if defined(Q_WS_MACX)
    int yval = y() > 0 ? 20 : 0;
    node->AddNode(new DataNode("x", x()));
    node->AddNode(new DataNode("y", y() - yval));
    node->AddNode(new DataNode("width", width()));
    node->AddNode(new DataNode("height", height() + yval));
#else
    node->AddNode(new DataNode("x", x()));
    node->AddNode(new DataNode("y", y()));
    node->AddNode(new DataNode("width", width()));
    node->AddNode(new DataNode("height", height()));
#endif

    // Add splitter values as a proportion of the window height.
    QValueList<int> splitterSizes(splitter->sizes());
    floatVector ss;
    for(int i = 0; i < splitterSizes.size(); ++i)
        ss.push_back(float(splitterSizes[i]) / 
                     float(splitter->height()));
    if(ss.size() >= 2)
        node->AddNode(new DataNode("SPLITTER_VALUES", ss));
}

// ****************************************************************************
// Method: QvisMainWindow::SetFromNode
//
// Purpose: 
//   Sets the size, position, and splitter values for the main window.
//
// Arguments:
//   parentNode : The node containing the MainWin node.
//   borders    : Window borders.
//
// Notes:      This method only reads the height component of the window if
//             it is available because it uses the height for determining
//             how the splitter should be placed. If the overrideGeometry
//             flag is passed in then it means that the geometry was already
//             passed on the command line or was already read from the
//             config file.
// 
// Programmer: Brad Whitlock
// Creation:   Tue Jul 25 10:13:38 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Jan 22 16:03:57 PST 2008
//   Check for NULL notepad.
//
// ****************************************************************************

void
QvisMainWindow::SetFromNode(DataNode *parentNode, bool overrideGeometry,
    const int *overrideGUISize, const int *overrideGUILocation,
    const int *borders)
{
    const char *mName = "QvisMainWindow::SetFromNode: ";

    DataNode *winNode = 0, *node = 0;
    int w = width();
    int h = height();
    QValueList<int> splitterSizes;

    if((winNode = parentNode->GetNode("MainWin")) != 0)
    {
        DataNode *node = 0;
        bool xy_set = false, wh_set = false;
        int x = 0, y = 0;

        debug1 << mName << "Initializing the main window because it was "
                  "in the config file." << endl;

        // See if any attributes are set.
        if((node = winNode->GetNode("height")) != 0)
        {
            h = node->AsInt();
            wh_set = true;
        }

        // Override the settings in the file.
        if(overrideGeometry)
        {
            debug1 << mName << "Overriding geometry from config file." << endl;
            w = overrideGUISize[0];
            h = overrideGUISize[1];
            x = overrideGUILocation[0];
            y = overrideGUILocation[1];
            wh_set = true;
            xy_set = true;
        }

        // Make sure that the window will fit on the screen.
        FitToScreen(x, y, w, h);

        // Look for the splitter values for this screen size.
        if((node = winNode->GetNode("SPLITTER_VALUES")) != 0)
        {
            const floatVector &ss = node->AsFloatVector();
    
            if(ss.size() >= 2)
            {
                float sum = 0.;
                int i;
                for(i = 0; i < ss.size(); ++i)
                    sum += ss[i];

                if(sum >= 0.9 && sum <= 1.)
                {
                    debug1 << mName << "Using saved splitter values {";
                    for(i = 0; i < ss.size(); ++i)
                    {
                        splitterSizes.push_back(int(ss[i] * h));
                        debug1 << ", " << ss[i];
                    }
                    debug1 << "}" << endl;
                }
                else
                {
                    debug1 << mName << "Ignoring splitter values because "
                        "their sum is not close enough to 1." << endl;
                }
            }
            else
            {
                debug1 << mName << "Ignoring splitter values because there "
                    "are not enough of them." << endl;
            }
        }
        else
        {
            debug1 << mName << "Could not locate key SPLITTER_VALUES"  
                   " so the default splitter values will be used."
                   << endl;
        }
    }

    // Default splitter values.
    if(splitterSizes.size() == 0)
    {
        debug1 << mName << "Using default splitter values." << endl;
        if(notepad != 0 && notepad->isVisible())
        {
            splitterSizes.push_back(int(0.3 * h));
            splitterSizes.push_back(int(0.3 * h));
            splitterSizes.push_back(int(0.4 * h));
        }
        else
        {
            splitterSizes.push_back(int(0.5 * h));
            splitterSizes.push_back(int(0.5 * h));
        }
    }
    splitter->setSizes(splitterSizes);
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
//   Hank Childs, Thu Jan 13 13:19:31 PST 2005
//   Add argument to iconify windows that indicates if the request is
//   spontaneous.
//
// ****************************************************************************

void
QvisMainWindow::hideEvent(QHideEvent *e)
{
    // Hide the window
    QMainWindow::hideEvent(e);

    // If the event was caused by the window manager, emit a signal.
    if(e->spontaneous())
    {
        emit iconifyWindows(e->spontaneous());
    }
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

// ****************************************************************************
// Method: QvisMainWindow::show
//
// Purpose: 
//   Shows the window and raises it.
//
// Note:       We override this method from QvisWindowBase because we don't
//             want the main window to obey the new window anchor stuff.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 22 10:54:52 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::show()
{
    // Indicate that the window should be saved.
    saveWindowDefaults = true;

    QMainWindow::show();
    QMainWindow::raise();
}

// ****************************************************************************
// Method: QvisMainWindow::reopenFile
//
// Purpose: 
//   Tells the viewer to reopen the specified file. This makes the reopened
//   file be the new active source.
//
// Arguments:
//   fileIndex : The index of the source that we're reopening in the source list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 11:42:41 PDT 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::reopenFile(int fileIndex)
{
    const stringVector &sources = globalAtts->GetSources();

    if(fileIndex >= 0 && fileIndex < sources.size())
    {
        //
        // Make the file that we reopened be the new open file. Since we're
        // reopening, this will take care of freeing the old metadata and SIL.
        //
        QualifiedFilename fileName(sources[fileIndex]);
        int timeState = GetStateForSource(fileName);
        SetOpenDataFile(fileName, timeState, 0, true);

        // Tell the viewer to replace all of the plots having
        // databases that match the file we're re-opening.
        GetViewerMethods()->ReOpenDatabase(sources[fileIndex], false);
    }
}

// ****************************************************************************
// Method: QvisMainWindow::closeFile
//
// Purpose: 
//   Tells the viewer to close the specified file.
//
// Arguments:
//   fileIndex : The index of the file that we want to close.
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 27 11:51:59 PDT 2004
//
// Modifications:
//   Brad Whitlock, Mon Jun 27 14:53:14 PST 2005
//   Added a little code to update the file panel.
//
// ****************************************************************************

void
QvisMainWindow::closeFile(int fileIndex)
{
    const stringVector &sources = globalAtts->GetSources();

    if(fileIndex >= 0 && fileIndex < sources.size())
    {
        //
        // Clear out the metadata and SIL for the file.
        //
        fileServer->ClearFile(sources[fileIndex]);

        //
        // Tell the viewer to replace close the specified database. If the
        // file is not being used then the viewer will allow it. Otherwise
        // the viewer will issue a warning message.
        //
        GetViewerMethods()->CloseDatabase(sources[fileIndex]);

        // Update the file panel based on what's selected.
        filePanel->UpdateOpenButtonState();
    }
}

void
QvisMainWindow::windowAdd()
{
    // Tell the viewer to add a window
    GetViewerMethods()->AddWindow();
}

void
QvisMainWindow::windowClone()
{
    // Tell the viewer to clone a window
    GetViewerMethods()->CloneWindow();
}

void
QvisMainWindow::windowDelete()
{
    // Tell the viewer to delete a window
    GetViewerMethods()->DeleteWindow();
}

void
QvisMainWindow::windowClearAll()
{
    // Tell the viewer to clear all windows.
    GetViewerMethods()->ClearAllWindows();    
}

void
QvisMainWindow::windowLayout1x1()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(1);
}

void
QvisMainWindow::windowLayout1x2()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(2);
}

void
QvisMainWindow::windowLayout2x2()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(4);
}

void
QvisMainWindow::windowLayout2x3()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(6);
}

void
QvisMainWindow::windowLayout2x4()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(8);
}

void
QvisMainWindow::windowLayout3x3()
{
    // Tell the viewer to change the layout.
    GetViewerMethods()->SetWindowLayout(9);
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
// Method: QvisMainWindow::updateNotAllowed
//
// Purpose: 
//   This is a Qt slot function that disables the "update visit" help option.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 15 14:16:28 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::updateNotAllowed()
{
    helpPopup->setItemEnabled(updateVisItId, false);
}

// ****************************************************************************
// Method: QvisMainWindow::SetTimeStateFormat
//
// Purpose: 
//   This a Qt slot function that sets the file panel's timestate format.
//
// Arguments:
//   fmt : The new timestate format.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:15:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::SetTimeStateFormat(const TimeFormat &fmt)
{
    filePanel->SetTimeStateFormat(fmt);
}

// ****************************************************************************
// Method: QvisMainWindow::GetTimeStateFormat
//
// Purpose: 
//   Returns the file panel's timestate format
//
// Returns:    The file panel's timestate format.
//
// Programmer: Brad Whitlock
// Creation:   Mon Oct 13 17:20:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

const TimeFormat &
QvisMainWindow::GetTimeStateFormat() const
{
    return filePanel->GetTimeStateFormat();
}

// ****************************************************************************
// Method: QvisMainWindow::SetShowSelectedFiles
//
// Purpose: 
//   This is a Qt slot function that sets whether or not the selected files
//   should be showing.
//
// Arguments:
//   val : If true, show the selected files. If false, don't show them.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:36:47 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Apr 5 15:50:24 PST 2004
//   Added support for menus that hide depending on if the selected files
//   are showing.
//
//   Jeremy Meredith, Mon Aug 28 17:30:47 EDT 2006
//   Changed positions of some menu entries to account for the new File
//   Open entry.
//
// ****************************************************************************

void
QvisMainWindow::SetShowSelectedFiles(bool val)
{
    filePanel->SetShowSelectedFiles(val);
    plotManager->SetSourceVisible(!val);

    if(val)
    {
        if(!advancedMenuShowing)
        {
            // Show selected files. Put reopen, close in an advanced menu.
            filePopup->removeItem(reopenPopupId);
            filePopup->removeItem(closePopupId);

            fileAdvancedPopup = new QPopupMenu(filePopup, "fileAdvancedPopup");
            fileAdvancedPopupId = filePopup->insertItem(tr("Advanced file options"),
                                  fileAdvancedPopup, -1, 2);

            // ReOpen pull-right menu.
            reopenPopup = new QPopupMenu(fileAdvancedPopup, "reopenPopup");
            connect(reopenPopup, SIGNAL(activated(int)),
                    this, SLOT(reopenFile(int)));
            reopenPopupId = fileAdvancedPopup->insertItem(tr("ReOpen file"), reopenPopup);
            fileAdvancedPopup->setItemEnabled(reopenPopupId, false);

            // Close pull-right menu
            closePopup = new QPopupMenu(fileAdvancedPopup, "closePopup");
            connect(closePopup, SIGNAL(activated(int)),
                    this, SLOT(closeFile(int)));
            closePopupId = fileAdvancedPopup->insertItem(tr("Close file"), closePopup);
            fileAdvancedPopup->setItemEnabled(closePopupId, false);
            advancedMenuShowing = true;

            //
            // Update the new visible menus with the active sources.
            //
            UpdateFileMenuPopup(reopenPopup, reopenPopupId);
            UpdateFileMenuPopup(closePopup, closePopupId);
        }
    }
    else
    {
        if(advancedMenuShowing)
        {
            // No selected files. Put reopen and close in the file menu.
            filePopup->removeItem(fileAdvancedPopupId);
            fileAdvancedPopupId = -1;

            // ReOpen pull-right menu.
            reopenPopup = new QPopupMenu(filePopup, "reopenPopup");
            connect(reopenPopup, SIGNAL(activated(int)),
                    this, SLOT(reopenFile(int)));
            reopenPopupId = filePopup->insertItem(tr("ReOpen file"), reopenPopup, -1, 2);
            filePopup->setItemEnabled(reopenPopupId, false);

            // Close pull-right menu
            closePopup = new QPopupMenu(filePopup, "closePopup");
            connect(closePopup, SIGNAL(activated(int)),
                    this, SLOT(closeFile(int)));
            closePopupId = filePopup->insertItem(tr("Close file"), closePopup, -1, 3);
            filePopup->setItemEnabled(closePopupId, false);
            advancedMenuShowing = false;

            //
            // Update the new visible menus with the active sources.
            //
            UpdateFileMenuPopup(reopenPopup, reopenPopupId);
            UpdateFileMenuPopup(closePopup, closePopupId);
        }
    }
}

// ****************************************************************************
// Method: QvisMainWindow::GetShowSelectedFiles
//
// Purpose: 
//   Returns whether the selected files should be showing.
//
// Programmer: Brad Whitlock
// Creation:   Fri Jan 30 14:36:24 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
QvisMainWindow::GetShowSelectedFiles() const
{
    return filePanel->GetShowSelectedFiles();
}

// ****************************************************************************
// Method: QvisMainWindow::GetAllowFileSelectionChange
//
// Purpose: 
//   Returns whether the selected files' selection should be changed when
//   the source changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 9 14:37:38 PST 2004
//
// Modifications:
//   
// ****************************************************************************

bool
QvisMainWindow::GetAllowFileSelectionChange() const
{
    return filePanel->GetAllowFileSelectionChange();
}

// ****************************************************************************
// Method: QvisMainWindow::SetAllowFileSelectionChange
//
// Purpose: 
//   Sets whether the file panel is allowed to change the file selection.
//
// Arguments:
//   val : True if it the file panel should be allowed to change the file
//         selection; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Apr 6 14:36:53 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::SetAllowFileSelectionChange(bool val)
{
    filePanel->SetAllowFileSelectionChange(val);
}

// ****************************************************************************
// Method: QvisMainWindow::OkayToSaveRecoveryFile
//
// Purpose: 
//   Sets whether VisIt is launched enough to consider saving the recovery file.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 10:48:01 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::OkayToSaveRecoveryFile()
{
    okayToSaveRecoveryFile = true;
    UpdateCrashRecoveryTimer();
}

// ****************************************************************************
// Method: QvisMainWindow::UpdateCrashRecoveryTimer
//
// Purpose: 
//   Updates the crash recovery timer based on the preference in globalAtts.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 31 12:28:33 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::UpdateCrashRecoveryTimer()
{
    if(globalAtts->GetSaveCrashRecoveryFile())
    {
        // Start the timer to save the recovery file every 5 minutes
        if(!recoveryFileTimer->isActive())
        {
            debug1 << "Starting crash recovery file timer." << endl;
            recoveryFileTimer->start(5 * 60 * 1000);
        }
    }
    else
    {
        debug1 << "Stopping crash recovery file timer." << endl;
        recoveryFileTimer->stop();
    }
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
    GetViewerMethods()->ToggleMaintainViewMode();
}

// ****************************************************************************
// Method: QvisMainWindow::maintainDataToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the maintain data limits
//   checkbox is toggled.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Eric Brugger
// Creation:   Mon Mar 29 13:07:58 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::maintainDataToggled(bool)
{
    GetViewerMethods()->ToggleMaintainDataMode();
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
        GetViewerMethods()->SetActiveWindow(globalAtts->GetWindows()[index]);
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
    GetViewerMethods()->CopyViewToWindow(from, to);
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
    GetViewerMethods()->CopyLightingToWindow(from, to);
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
    GetViewerMethods()->CopyAnnotationsToWindow(from, to);
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
    GetViewerMethods()->CopyPlotsToWindow(from, to);
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
    GetViewerMethods()->DisableRedraw();
    GetViewerMethods()->CopyViewToWindow(from, to);
    GetViewerMethods()->CopyLightingToWindow(from, to);
    GetViewerMethods()->CopyAnnotationsToWindow(from, to);
    GetViewerMethods()->CopyPlotsToWindow(from, to);
    GetViewerMethods()->RedrawWindow();
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
    GetViewerMethods()->ClearWindow();
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
    GetViewerMethods()->ClearReferenceLines();
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
    GetViewerMethods()->ClearPickPoints();
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
    GetViewerMethods()->ToggleBoundingBoxMode();
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
    GetViewerMethods()->ToggleSpinMode();
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
    GetViewerMethods()->ToggleFullFrameMode();
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
    GetViewerMethods()->ToggleLockTime();
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
    GetViewerMethods()->ToggleLockTools();
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
    GetViewerMethods()->ToggleLockViewMode();
}

// ****************************************************************************
// Method: QvisMainWindow::unlockEverything
//
// Purpose: 
//   This is a Qt slot function that turns off all of the locks.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jan 23 10:48:12 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisMainWindow::unlockEverything()
{
    GetViewerMethods()->TurnOffAllLocks();
}
