/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QvisPlotManagerWidget.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMap>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmapCache>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>

#include <ViewerProxy.h>
#include <PlotList.h>
#include <FileServerList.h>
#include <ExpressionList.h>
#include <Expression.h>
#include <GlobalAttributes.h>
#include <NameSimplifier.h>
#include <PluginManagerAttributes.h>
#include <QvisPlotListBoxItem.h>
#include <QvisPlotListBox.h>
#include <QvisVariableButton.h>
#include <QvisVariablePopupMenu.h>
#include <PlotPluginInfo.h>
#include <WindowInformation.h>
#include <TimingsManager.h>

#include <stdio.h>
#include <stdlib.h>
#include <snprintf.h>

#include <icons/removelastoperator.xpm>
#include <icons/removealloperators.xpm>

#include <DebugStream.h>
//#define DEBUG_PRINT

#define REMOVE_LAST_OPERATOR_ID   1000
#define REMOVE_ALL_OPERATORS_ID   1001

#define VARIABLE_CUTOFF           100

#define DELETE_MENU_TO_FREE_POPUPS

using std::string;
using std::vector;

// ****************************************************************************
// Method: QvisPlotManagerWidget::QvisPlotManagerWidget
//
// Purpose: 
//   This is the constructor for the QvisPlotManagerWidget class.
//
// Arguments:
//   menuBar  : A pointer to the main window's menu bar.
//   parent   : The widget's parent.
//   name     : The widget's name.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:18:55 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 23 17:24:27 PST 2001
//   Added initialization of the plot plugin list. Removed the code that
//   created the plot & operator menus.
//
//   Kathleen Bonnell, Fri Oct 19 15:33:35 PDT 2001
//   Added subsetVars.
//   
//   Eric Brugger, Thu Nov 29 12:53:11 PST 2001
//   I removed the private data member matVars.
//
//   Jeremy Meredith, Tue Dec 18 10:32:13 PST 2001
//   Added some species var support.
//
//   Brad Whitlock, Wed Feb 6 16:23:01 PST 2002
//   Removed the "Apply to all plots toggle"
//
//   Brad Whitlock, Mon Mar 4 14:13:00 PST 2002
//   Added autoupdate support.
//
//   Brad Whitlock, Thu May 2 15:13:20 PST 2002
//   Made it also inherit from GUIBase.
//
//   Brad Whitlock, Thu May 9 13:34:17 PST 2002
//   Made activePlots a class member.
//
//   Brad Whitlock, Mon Mar 17 15:51:27 PST 2003
//   I removed some member initialization and added menuPopulator.
//
//   Brad Whitlock, Thu Apr 10 15:53:17 PST 2003
//   I connected some new signals and slots for plotListBox.
//
//   Brad Whitlock, Mon Aug 25 09:38:58 PDT 2003
//   I changed a label string.
//
//   Brad Whitlock, Fri Aug 15 15:07:59 PST 2003
//   I added a QMenuBar argument to the constructor.
//
//   Brad Whitlock, Tue Feb 24 16:27:15 PST 2004
//   I added varMenuPopulator and a few other new members.
//
//   Brad Whitlock, Thu Jan 29 19:55:51 PST 2004
//   I added the activeSource combo box.
//
//   Brad Whitlock, Mon Mar 15 11:46:34 PDT 2004
//   I added varMenuFlags.
//
//   Jeremy Meredith, Tue Aug 24 16:20:40 PDT 2004
//   Made it observe metadata directly so it knows when to update things.
//
//   Brad Whitlock, Mon Nov 14 13:23:37 PST 2005
//   Changed layout a little.
//
//   Brad Whitlock, Tue Apr 25 16:34:46 PST 2006
//   Added operatorPlugins.
//
//   Gunther H. Weber, Mon Jan 28 15:35:16 PST 2008
//   Split "Apply operators and selections ..." checkbox into an apply
//   operators and an apply selection checkbox.
//
//   Gunther H. Weber, Fri Feb 29 15:38:57 PST 2008
//   Fixed Qt warning caused by previous change.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
//   Brad Whitlock, Tue Sep  9 10:40:33 PDT 2008
//   Removed metaData and pluginAtts since they were not used.
//
//   Brad Whitlock, Tue Oct 20 15:43:40 PDT 2009
//   I added controls to change the plot list ordering.
//
// ****************************************************************************

QvisPlotManagerWidget::QvisPlotManagerWidget(QMenuBar *menuBar,QWidget *parent) 
: QWidget(parent), GUIBase(), SimpleObserver(), menuPopulator(), 
  varMenuPopulator(), plotPlugins(), operatorPlugins()
{
    plotList = 0;
    globalAtts = 0;
    windowInfo = 0;
    exprList = 0;

    pluginsLoaded = false;
    updatePlotVariableMenuEnabledState = false;
    updateOperatorMenuEnabledState = false;
    updateVariableMenuEnabledState = false;
    maxVarCount = 0;
    varMenuFlags = 0;
    sourceVisible = false;

    QVBoxLayout *veryTopLayout = new QVBoxLayout(this);
    veryTopLayout->setSpacing(5);
    veryTopLayout->setMargin(0);
    topLayout = new QGridLayout();
    veryTopLayout->addLayout(topLayout);
    topLayout->setSpacing(5);
    topLayout->setMargin(0);

    // Create the source combobox.
    sourceComboBox = new QComboBox(this);
    sourceComboBox->hide();
    connect(sourceComboBox, SIGNAL(activated(int)),
            this, SLOT(sourceChanged(int)));
    sourceLabel = new QLabel(tr("Source"), this);
    sourceLabel->hide();
    topLayout->addWidget(sourceLabel, 0, 0);
    topLayout->addWidget(sourceComboBox, 0, 1, 1, 3);

    activePlots = new QLabel(tr("Active plots"), this);
    topLayout->addWidget(activePlots, 1, 0);

    // Create the hide/show button.
    hideButton = new QPushButton(tr("Hide/Show"), this);
    hideButton->setEnabled(false);
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hidePlots()));
    topLayout->addWidget(hideButton, 1, 1);

    // Create the delete button.
    deleteButton = new QPushButton(tr("Delete"), this);
    deleteButton->setEnabled(false);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deletePlots()));
    topLayout->addWidget(deleteButton, 1, 2);

    // Create the draw button.
    drawButton = new QPushButton(tr("Draw"), this);
    drawButton->setEnabled(false);
    connect(drawButton, SIGNAL(clicked()), this, SLOT(drawPlots()));
    topLayout->addWidget(drawButton, 1, 3);

    // Create the plot list box.
    plotListBox = new QvisPlotListBox(this);
    plotListBox->setSelectionMode(QAbstractItemView::ExtendedSelection);
    plotListBox->setMinimumHeight(fontMetrics().boundingRect("X").height() * 6);
    
    connect(plotListBox, SIGNAL(itemSelectionChanged()),
            this, SLOT(setActivePlots()));
    connect(plotListBox, SIGNAL(itemExpansionChanged()),
            this, SLOT(setActivePlots())); 

    connect(plotListBox, SIGNAL(activatePlotWindow(int)),
            this, SIGNAL(activatePlotWindow(int)));
    connect(plotListBox, SIGNAL(activateOperatorWindow(int)),
            this, SIGNAL(activateOperatorWindow(int)));

    connect(plotListBox, SIGNAL(activateSubsetWindow()),
            this, SIGNAL(activateSubsetWindow()));
    connect(plotListBox, SIGNAL(promoteOperator(int)),
            this, SLOT(promoteOperator(int)));
    connect(plotListBox, SIGNAL(demoteOperator(int)),
            this, SLOT(demoteOperator(int)));
    connect(plotListBox, SIGNAL(removeOperator(int)),
            this, SLOT(removeOperator(int)));

    connect(plotListBox, SIGNAL(hideThisPlot()),
            this, SLOT(hideThisPlot()));
    connect(plotListBox, SIGNAL(deleteThisPlot()),
            this, SLOT(deleteThisPlot()));
    connect(plotListBox, SIGNAL(drawThisPlot()),
            this, SLOT(drawThisPlot()));
    connect(plotListBox, SIGNAL(clearThisPlot()),
            this, SLOT(clearThisPlot()));
    connect(plotListBox, SIGNAL(copyThisPlot()),
            this, SLOT(copyThisPlot()));
    connect(plotListBox, SIGNAL(copyToWinThisPlot()),
            this, SLOT(copyToWinThisPlot()));
    connect(plotListBox, SIGNAL(redrawThisPlot()),
            this, SLOT(redrawThisPlot()));
    connect(plotListBox, SIGNAL(disconnectThisPlot()),
            this, SLOT(disconnectThisPlot()));

    connect(plotListBox, SIGNAL(renamePlot(int, const QString &)),
            this, SLOT(setPlotDescription(int, const QString &)));
    connect(plotListBox, SIGNAL(moveThisPlotTowardFirst()),
            this, SLOT(moveThisPlotTowardFirst()));
    connect(plotListBox, SIGNAL(moveThisPlotTowardLast()),
            this, SLOT(moveThisPlotTowardLast()));
    connect(plotListBox, SIGNAL(makeThisPlotFirst()),
            this, SLOT(makeThisPlotFirst()));
    connect(plotListBox, SIGNAL(makeThisPlotLast()),
            this, SLOT(makeThisPlotLast()));

    topLayout->addWidget(plotListBox, 2, 0, 1, 4);

    QWidget *applyOpsAndSelection = new QWidget(this);
    QHBoxLayout *applyLayout = new QHBoxLayout(applyOpsAndSelection);
    applyLayout->setMargin(0);
#if defined(__APPLE__)
    topLayout->addWidget(applyOpsAndSelection, 3, 0, 1, 4);
#else
    topLayout->addWidget(applyOpsAndSelection, 4, 0, 1, 4);
#endif
    // Create the "Apply operator to all plots" toggle.
    applyOperatorToggle = new QCheckBox(tr("Apply operators"), applyOpsAndSelection);
    connect(applyOperatorToggle, SIGNAL(toggled(bool)),
            this, SLOT(applyOperatorToggled(bool)));
    applyLayout->addWidget(applyOperatorToggle);
    applyLayout->addWidget(new QLabel("/", applyOpsAndSelection));

    // Create the "Apply selection to all plots" toggle.
    applySelectionToggle = new QCheckBox(tr("selection to all plots"), this);
    connect(applySelectionToggle, SIGNAL(toggled(bool)),
            this, SLOT(applySelectionToggled(bool)));
    applyLayout->addWidget(applySelectionToggle);
    applyLayout->addStretch(1);

    // Create the plot and operator menus. Note that they will be empty until
    // they are populated by the main application.
    operatorRemoveLastAct = 0;
    operatorRemoveAllAct = 0;
    CreateMenus(menuBar);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::~QvisPlotManagerWidget
//
// Purpose: 
//   This is the destructor for the QvisPlotManagerWidget class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:20:31 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 1 15:25:07 PST 2002
//   Added exprList, pluginAtts to prevent a memory error.
//
//   Brad Whitlock, Thu Jan 29 21:40:31 PST 2004
//   Added windowInfo.
//
//   Jeremy Meredith, Tue Aug 24 16:21:00 PDT 2004
//   Made it observe metadata directly so it knows when to update things.
//
//   Brad Whitlock, Tue Sep  9 10:40:33 PDT 2008
//   Removed metaData and pluginAtts since they were not used.
//
//   Hank Childs, Thu Dec  4 10:19:11 PST 2008
//   Commit fix for memory leak contributed by David Camp of UC Davis.
//
// ****************************************************************************

QvisPlotManagerWidget::~QvisPlotManagerWidget()
{
    for (int i = 0 ; i < plotPlugins.size() ; i++)
        DestroyPlotMenuItem(i);

    if(plotList)
        plotList->Detach(this);

    if(fileServer)
        fileServer->Detach(this);

    if(globalAtts)
        globalAtts->Detach(this);

    if(exprList)
        exprList->Detach(this);

    if(windowInfo)
        windowInfo->Detach(this);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::SetSourceVisible
//
// Purpose: 
//   Sets whether of not the source combo box is visible.
//
// Arguments:
//   val : True to make the source visible; false to hide it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 21:49:12 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::SetSourceVisible(bool val)
{
    if(sourceVisible != val)
    {
        sourceVisible = val;

        if(sourceVisible)
        {
            sourceLabel->show();
            sourceComboBox->show();
        }
        else
        { 
            sourceLabel->hide();
            sourceComboBox->hide();
        }

        updateGeometry();
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::CreateMenus
//
// Purpose: 
//   Creates the plot, plot atts, operator, operator atts, variable
//   menus.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 13 18:37:27 PST 2000
//
// Modifications:
//  
//   Kathleen Bonnell, Tue Oct 10 15:54:26 PDT 2000
//   Enabled OnionPeel in operator and operator attributes menu
// 
//   Brad Whitlock, Fri Nov 10 16:24:05 PST 2000
//   Modified for the material plot.
//
//   Brad Whitlock, Tue Dec 12 15:01:18 PST 2000
//   I enabled the material selection operator.
//
//   Hank Childs, Tue Jan 16 15:35:51 PST 2001
//   Added volume plots.
//
//   Brad Whitlock, Fri Feb 16 14:45:02 PST 2001
//   I enabled the contour plot.
//
//   Eric Brugger, Mon Mar  5 14:46:55 PST 2001
//   Disable the Range Volume and Iso Surface Volume plots.
//
//   Kathleen Bonnell, Tue Mar  6 13:40:59 PST 2001 
//   Enabled surface plot. 
//
//   Hank Childs, Fri Mar 23 10:01:51 PST 2001
//   Enabled vector plot.
//
//   Brad Whitlock, Fri Mar 23 17:12:18 PST 2001
//   Rewrote to support plot and operator plugins.
//
//   Brad Whitlock, Fri Oct 26 16:52:04 PST 2001
//   Disabled the plot and operator attribute menus.
//
//   Brad Whitlock, Wed Feb 6 11:15:45 PDT 2002
//   Changed the type of the variable menu.
//
//   Brad Whitlock, Mon Mar 17 13:33:40 PST 2003
//   I added icons for some options in the operator menu.
//
//   Brad Whitlock, Fri Aug 15 15:10:00 PST 2003
//   I added support for MacOS X.
//
//   Brad Whitlock, Mon Jul 24 17:45:04 PST 2006
//   I wrapped the menu in a widget to prevent it from shrinking.
//
//   Brad Whitlock, Thu Dec 20 12:13:33 PST 2007
//   Moved variable menu creation to a helper method.
//  
//   Gunther H. Weber, Fri Feb 29 18:35:18 PST 2008
//   Fixed disappearing variables menu problem.
//
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::CreateMenus(QMenuBar *menuBar)
{
    //
    // Create the Plots Menu
    //
#if defined(__APPLE__)
    // On MacOS, we want to have all of the menu options together since they
    // run along the top of the screen instead of being part of each window.
    plotMenuBar = menuBar;
#else
    plotMenuBar = new QMenuBar(this);
    topLayout->addWidget(plotMenuBar, 3, 0, 1, 4);
#endif

    // Create the Plot menu. Each time we highlight a plot, we
    // update the current plot type.
    plotMenu = new QMenu(tr("Plots"),plotMenuBar);

    // Add the whole "Plots" menu to the menu bar.
    plotMenuAct = plotMenuBar->addMenu(plotMenu);
    plotMenuAct->setEnabled(false);

    //
    // Create the operator menu.
    //
    operatorMenu = new QMenu(tr("Operators"),plotMenuBar);
    connect(operatorMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(operatorAction(QAction *)));   
    operatorMenuAct = plotMenuBar->addMenu(operatorMenu);
    operatorMenuAct->setEnabled(false);

    //
    // Create the Plot attributes menu.
    //
    
    QString mname;
#ifdef __APPLE__
    mname = tr("Plot Attributes"),
#else
    mname = tr("PlotAtts"),
#endif    
    
    plotAttsMenu = new QMenu(mname, plotMenuBar );
    
    connect(plotAttsMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(activatePlotWindow(QAction *)));
    
    plotAttsMenuAct = plotMenuBar->addMenu(plotAttsMenu);
    plotAttsMenuAct->setEnabled(false);
    //plotMenuBar->setItemEnabled(plotAttsMenuId, false);

    //
    // Create the Operator attributes menu.
    //
#ifdef __APPLE__
    mname = tr("Operator Attributes"),
#else
    mname = tr("OpAtts"),
#endif
    
    operatorAttsMenu = new QMenu(mname, plotMenuBar );
    connect(operatorAttsMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(activateOperatorWindow(QAction *)));
    operatorAttsMenuAct = plotMenuBar->addMenu( operatorAttsMenu );
    operatorAttsMenuAct->setEnabled(false);
    //plotMenuBar->setItemEnabled(operatorAttsMenuId, false);

    //
    // Create an empty variable menu.
    //
    plotMenuBar->addSeparator();
    CreateVariableMenu();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::CreateVariableMenu
//
// Purpose: 
//   Create the variable menu.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:13:16 PST 2007
//
// Modifications:
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::CreateVariableMenu()
{
    // Add an empty variable menu to the plot menu bar
    // need to be able to set the name! tr("Variables")
    varMenu = new QvisVariablePopupMenu(-1, 0);
    varMenu->setTitle(tr("Variables"));

    connect(varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    varMenuAct = plotMenuBar->addMenu(varMenu);
    varMenuAct->setEnabled(false);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::DestroyVariableMenu
//
// Purpose: 
//   Destroy the variable menu.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:13:04 PST 2007
//
// Modifications:
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Thu Dec  4 09:13:50 PST 2008
//   Removed unnecssary todo comment.
//
// ****************************************************************************

void
QvisPlotManagerWidget::DestroyVariableMenu()
{
    if(varMenu)
    {
        // Remove the variable menu from the plot menu bar
        plotMenuBar->removeAction(varMenuAct);

        // Delete the variable menu.
        delete varMenu;
        varMenu = 0;
        varMenuAct = 0;
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::Update
//
// Purpose: 
//   This method is called when the PlotList or FileServerList that
//   the widget watches is updated.
//
// Note:       
//   This method should update the widget to accurately reflect the
//   PlotList object that the widget watches.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:21:32 PST 2000
//
// Modifications:
//   Jeremy Meredith, Wed Sep  5 14:02:49 PDT 2001
//   Added plugin manager atts observation.
//
//   Brad Whitlock, Fri Oct 26 16:55:18 PST 2001
//   Added code to enable the plot and operator atts menus. Also removed the
//   "Apply to all plots" toggle.
//
//   Brad Whitlock, Tue Feb 26 16:16:38 PST 2002
//   Re-added the "Apply to all plots" toggle.
//
//   Brad Whitlock, Mon Mar 4 16:52:27 PST 2002
//   Added code to restore the cursor.
//
//   Brad Whitlock, Tue May 7 17:39:16 PST 2002
//   Added code to set the enabled state of the widget when the globalAtts
//   change.
//
//   Brad Whitlock, Wed Jul 3 12:56:52 PDT 2002
//   Added code to force the plot menu to redraw so it works with Qt 3.0.
//
//   Brad Whitlock, Fri Aug 23 16:43:11 PST 2002
//   I fixed a bug where the variable menu was incorrectly enabled.
//
//   Eric Brugger, Mon Dec 16 13:39:22 PST 2002
//   I removed the code that sets the number of states and the current state
//   in the global attributes.  This was unnecessary since the viewer is
//   responsible for doing that.
//
//   Brad Whitlock, Thu Mar 20 12:19:07 PDT 2003
//   I made the number of plugins be taken into account when enabling the
//   plot and operator attributes menus.
//
//   Brad Whitlock, Mon Jul 28 17:52:29 PST 2003
//   I moved the code that sets the enabled state for the Plots and Operators
//   menus to UpdatePlotAndOperatorMenuEnabledState.
//
//   Brad Whitlock, Wed Sep 10 09:04:34 PDT 2003
//   I moved the code that sets the enabled state for the Hide, Delete,
//   and Draw buttons to UpdateHideDeleteDrawButtonsEnabledState.
//
//   Brad Whitlock, Thu Jan 29 21:45:34 PST 2004
//   I added code to update the source list.
//
//   Jeremy Meredith, Tue Aug 24 16:21:15 PDT 2004
//   Made it observe metadata directly so it knows when to update things.
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Moved an UpdatePlotVariableMenu call out of a too constraining if test 
//   Added conditional for changes in winInfo's time slider's current states
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Gunther H. Weber, Mon Jan 28 15:35:16 PST 2008
//   Split "Apply operators and selections ..." checkbox into an apply
//   operators and an apply selection checkbox.
//
//   Brad Whitlock, Tue Sep  9 10:40:33 PDT 2008
//   Removed metaData and pluginAtts since they were not used.
//
// ****************************************************************************

void
QvisPlotManagerWidget::Update(Subject *TheChangedSubject)
{
    if(plotList == 0 || fileServer == 0 || globalAtts == 0 || windowInfo == 0)
    {
        return;
    }

    //
    // Initialize the class members that are used in various methods to tell
    // us whether updating the menu enabled state is required.
    //
    this->updatePlotVariableMenuEnabledState = false;
    this->updateOperatorMenuEnabledState = false;
    this->updateVariableMenuEnabledState = false;

    if(TheChangedSubject == plotList)
    {
        UpdatePlotList();
        UpdateVariableMenu();
        UpdatePlotVariableMenu();

        // If the number of plots is zero and the currently active file
        // is no longer in the applied file list, set the fileServer to
        // no active file. We set this observer so it gets no update
        // and we call notify to tell the other observers.
        if(plotList->GetNumPlots() == 0 &&
           (fileServer->GetFileIndex(fileServer->GetOpenFile()) == -1))
        {
            SetUpdate(false);
            fileServer->CloseFile();
            fileServer->Notify();
        }

        // If we have set the cursor in the past, then restore it now.
        RestoreCursor();
    }
    else if(TheChangedSubject == fileServer)
    {
        if(fileServer->AppliedFileListChanged())
        {
            UpdatePlotList();

            // If the number of plots is zero and the currently active file
            // is no longer in the applied file list, set the fileServer to
            // no active file. We set this observer so it gets no update
            // and we call notify to tell the other observers.
            if(plotList->GetNumPlots() == 0 &&
               (fileServer->GetFileIndex(fileServer->GetOpenFile()) == -1))
            {
                SetUpdate(false);
                fileServer->CloseFile();
                fileServer->Notify();

                // We closed the file but we don't want to notify this object
                // so we need to clear out the menu populators.
                menuPopulator.ClearDatabaseName();
                varMenuPopulator.ClearDatabaseName();

                UpdatePlotVariableMenu();
                UpdateVariableMenu();
            }
        }
        else if(fileServer->FileChanged())
        {
            //
            // If the file changed and we closed it, we probably did a reopen
            // so we should clear the cached database name so the next time
            // that the menu populators are asked to populate their variables
            // they will do it instead of returning early.
            //
            if(fileServer->ClosedFile())
            {
                menuPopulator.ClearDatabaseName();
                varMenuPopulator.ClearDatabaseName();
            }

            UpdatePlotVariableMenu();
            UpdateSourceList(false);
        }
    }
    else if(TheChangedSubject == exprList)
    {
        // The list of expressions changed.
        UpdatePlotVariableMenu();
        UpdateVariableMenu();
    }
    else if(TheChangedSubject == globalAtts)
    {
        // Update the source list.
        if(globalAtts->IsSelected(GlobalAttributes::ID_sources))
            UpdateSourceList(false);

        // Set the "Apply operator toggle."
        applyOperatorToggle->blockSignals(true);
        applyOperatorToggle->setChecked(globalAtts->GetApplyOperator());
        applyOperatorToggle->blockSignals(false);

        // Set the "Apply selection toggle."
        applySelectionToggle->blockSignals(true);
        applySelectionToggle->setChecked(globalAtts->GetApplySelection());
        applySelectionToggle->blockSignals(false);

        //
        // When the globalAtts change, we might have to update the
        // enabled state for the entire plot and operator menu bar.
        //
        this->updatePlotVariableMenuEnabledState = true;
        this->updateOperatorMenuEnabledState = true;
        this->updateVariableMenuEnabledState = true;
    }
    else if(TheChangedSubject == windowInfo)
    {
        // Update the source list when the active source changes.
        if(windowInfo->IsSelected(WindowInformation::ID_activeSource))
        {
            UpdateSourceList(true);

            // If the active source changed then the variable list needs
            // to change.
            UpdatePlotVariableMenu();
        }

        // handle changes in time slider current states
        if (windowInfo->IsSelected(WindowInformation::ID_timeSliderCurrentStates))
        {
            const avtDatabaseMetaData *md =
                fileServer->GetMetaData(fileServer->GetOpenFile(),
                                GetStateForSource(fileServer->GetOpenFile()),
                                FileServerList::ANY_STATE,
                               !FileServerList::GET_NEW_MD);

            // Although the innards of the called routines, here, also
            // check MustRepopulationOnStateChange, checking here saves
            // some GUI work, too.
            if (fileServer->GetTreatAllDBsAsTimeVarying() ||
	        (md && md->GetMustRepopulateOnStateChange()))
            {
                UpdateVariableMenu();
                UpdatePlotVariableMenu();
            }
        }
    }

    // Update the enabled state for the menu bar.
    UpdatePlotAndOperatorMenuEnabledState();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdatePlotList
//
// Purpose: 
//   Updates the plot manager part of the widget to reflect the 
//   current state of the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 10:29:20 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Thu Jul 26 03:03:11 PDT 2001
//   Removed reference to GetPlotTYpeName.
//
//   Brad Whitlock, Tue May 7 13:50:08 PST 2002
//   I fixed a problem with the hide button being disabled incorrectly.
//
//   Brad Whitlock, Tue Jul 30 17:19:30 PST 2002
//   I made plots with errors hideable.
//
//   Brad Whitlock, Wed Apr 16 14:06:28 PST 2003
//   I added a check to prevent updates to the listbox if they are not
//   necessary.
//
//   Brad Whitlock, Wed Sep 10 08:57:46 PDT 2003
//   I changed the code so the Hide button is enabled when is should be enabled.
//
//   Brad Whitlock, Fri Dec 5 16:24:36 PST 2003
//   I separated the logic to regenerate the plot list and just change its
//   selections so it's not so hard to select multiple items
//
//   Brad Whitlock, Thu Feb 26 11:17:22 PDT 2004
//   I changed how the plot and operator menu's enabled state is set.
//
//   Brad Whitlock, Wed Jul 28 17:44:19 PST 2004
//   I added code to make sure that the prefixes are taken into account when
//   setting generating the items in the list box.
//
//   Brad Whitlock, Tue Sep 30 15:00:28 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotList()
{
    blockSignals(true);

    //
    // Create the vector of prefixes for the new plot list.
    //
    int i;
    stringVector prefixes;
    for(i = 0; i < plotList->GetNumPlots(); ++i)
    {
        // Create a constant reference to the current plot.
        const Plot &current = plotList->operator[](i);

        // Figure out the prefix that should be applied to the plot.
        char prefix[200];
        QualifiedFilename qualifiedFile(current.GetDatabaseName());
        int index = fileServer->GetFileIndex(qualifiedFile);
        if(index < 0)
            SNPRINTF(prefix, 200, "%s:", qualifiedFile.filename.c_str());
        else
            SNPRINTF(prefix, 200, "%d:", index); 

        prefixes.push_back(prefix);
    }

    if(plotListBox->NeedsToBeRegenerated(plotList, prefixes))
    {
        // Update the plot list.
        plotListBox->blockSignals(true);
        plotListBox->clear();
        for(i = 0; i < plotList->GetNumPlots(); ++i)
        {
            // Create a constant reference to the current plot.
            const Plot &current = plotList->operator[](i);

            // Figure out the prefix that should be applied to the plot.
            QString prefix(prefixes[i].c_str());

            // Create a new plot item in the list.
            QvisPlotListBoxItem *newPlot = new QvisPlotListBoxItem(prefix,
                current);
            // Store "this" in the item's data so the delegate can callback into it.
            qulonglong addr = (qulonglong)(void*)newPlot;
            newPlot->setData(Qt::UserRole, QVariant(addr));

            plotListBox->addItem(newPlot);
            plotListBox->item(i)->setSelected(current.GetActiveFlag());
            if(current.GetActiveFlag())
                plotListBox->setCurrentItem(newPlot);

#ifdef DEBUG_PRINT
            qDebug("Plot[%d]={active=%d, hidden=%d, state=%d, dbName=%s, var=%s}",
                   i,
                   current.GetActiveFlag(),
                   current.GetHiddenFlag(), (int)current.GetStateType(),
                   current.GetDatabaseName().c_str(),current.GetPlotVar().c_str());
#endif
        } // end for
        plotListBox->blockSignals(false);
    }
    else if(plotListBox->NeedToUpdateSelection(plotList))
    {
        plotListBox->blockSignals(true);
        for(i = 0; i < plotList->GetNumPlots(); ++i)
        {
            // Create a constant reference to the current plot.
            const Plot &current = plotList->operator[](i);
            plotListBox->item(i)->setSelected(current.GetActiveFlag());
        } // end for
        plotListBox->blockSignals(false);        
    }

    // If there are no variables, clear out the variable menu.
    if(plotList->GetNumPlots() == 0 && varMenu->count() > 0)
        varMenu->clear();

    //
    // The operator menu can be disabled if there are no plots. Same with
    // the variable list so we check them both.
    //
    this->updateOperatorMenuEnabledState = true;
    this->updateVariableMenuEnabledState = true;

    // Set the enabled states for the hide, delete, and draw buttons.
    UpdateHideDeleteDrawButtonsEnabledState();

    blockSignals(false);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdateSourceList
//
// Purpose: 
//   Updates the source list.
//
// Arguments:
//   updateActiveSourceOnly : Tells the method to only update the active
//                            source and not the list of sources.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 21:58:49 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::UpdateSourceList(bool updateActiveSourceOnly)
{
    const stringVector &sources = globalAtts->GetSources();
    const std::string &activeSource = windowInfo->GetActiveSource();

    // See if the active source is in the list.
    int i, sourceIndex = -1;
    for(i = 0; i < sources.size(); ++i)
    {
        if(activeSource == sources[i])
        {
            sourceIndex = i;
            break;
        }
    }

    sourceComboBox->blockSignals(true);

    //
    // Populate the menu if we were not told to only update the active source.
    //
    if(!updateActiveSourceOnly)
    {
        //
        // Simplify the current source names and put the short names into
        // the source combo box.
        //
        NameSimplifier simplifier;
        for(i = 0; i < sources.size(); ++i)
            simplifier.AddName(sources[i]);
        stringVector shortSources;
        simplifier.GetSimplifiedNames(shortSources);
        sourceComboBox->clear();
        for(i = 0; i < shortSources.size(); ++i)
            sourceComboBox->addItem(shortSources[i].c_str());
    }

    //
    // Set the current item.
    //
    if(sourceIndex != -1 && sourceIndex != sourceComboBox->currentIndex())
        sourceComboBox->setCurrentIndex(sourceIndex);

    sourceComboBox->blockSignals(false);

    // Set the enabled state on the source combo box.
    bool enabled = (sources.size() > 1);
    sourceLabel->setEnabled(enabled);
    sourceComboBox->setEnabled(enabled);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdateHideDeleteDrawButtonsEnabledState
//
// Purpose: 
//   Updates the enabled state for the Hide, Delete, and Draw buttons so it
//   is always done the same way.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 10 09:05:26 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 09:18:35 PDT 2005
//   Removed the code that prevented the controls from being enabled when
//   the engine is busy.
//
// ****************************************************************************

void
QvisPlotManagerWidget::UpdateHideDeleteDrawButtonsEnabledState() const
{
    //
    // Figure out the number of plots that can be hidden.
    //
    int nHideablePlots = 0;
    for(int i = 0; i < plotList->GetNumPlots(); ++i)
    {
        // Create a constant reference to the current plot.
       const Plot &current = plotList->operator[](i);
       bool hideableState = (current.GetStateType() == Plot::Completed) ||
                            (current.GetStateType() == Plot::Error);
       bool canHide = hideableState || current.GetHiddenFlag();
       if(current.GetActiveFlag() && canHide)
           ++nHideablePlots;
    }

    hideButton->setEnabled(nHideablePlots > 0);
    deleteButton->setEnabled(plotList->GetNumPlots() > 0);
    drawButton->setEnabled(plotList->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::AddPlotType
//
// Purpose: 
//   Adds a new plot type to the plot type list.
//
// Arguments:
//   plotName : The name of the plot to add to the list.
//   varTypes : The types of variables that the plot will allow into its
//              variable list.
//   iconData : A pointer to icon data. This pointer can be NULL.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 09:46:23 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:16:41 PDT 2002
//   Made slight modifications to support cascading variable menus.
//
//   Brad Whitlock, Thu Mar 13 09:29:15 PDT 2003
//   I added support for icons in the menu.
//
//   Brad Whitlock, Thu Aug 21 13:36:25 PST 2003
//   I disabled icon support on MacOS X since applications don't get
//   to put icons in the top menu.
//
//   Brad Whitlock, Mon Apr 26 16:39:47 PST 2004
//   I enabled icons since the new version of Qt on MacOS X supports icons
//   in the top menu.
//
//   Brad Whitlock, Tue Dec 14 10:59:40 PDT 2004
//   I changed the name of a slot.
//
//   Brad Whitlock, Tue Apr 25 16:30:59 PST 2006
//   I set the new varMask member in the PluginEntry struct.
//
//   Brad Whitlock, Thu Dec 20 10:32:59 PST 2007
//   Changed the code so it calls other methods that we can reuse.
//
//   Brad Whitlock, Fri Apr 25 10:19:43 PDT 2008
//   I made plotName be a QString so we can internationalize the plot names.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::AddPlotType(const QString &plotName, const int varTypes,
    const char **iconData)
{
    PluginEntry entry;
    entry.pluginName = plotName;
    entry.menuName = plotName + QString(" . . .");
    entry.varMenu = 0;
    entry.varTypes = varTypes;
    entry.varMask = 1;
    entry.action = 0;

    if(iconData)
    {
        QPixmap iconPixmap(iconData);
        QPixmapCache::insert(plotName, iconPixmap);
        entry.icon = QIcon(iconPixmap);

        // Add the plot type to the plot attributes list.
        plotAttsMenu->addAction(entry.icon, entry.menuName);
    }
    else
    {
        // Add the plot type to the plot attributes list.
        //plotAttsMenu->insertItem(entry.menuName, plotAttsMenu->count());
        plotAttsMenu->addAction(entry.menuName);
    }

    // Add the plot plugin information to the plugin list.
    plotPlugins.push_back(entry);

    // Create the variable menu part of the plot
    CreatePlotMenuItem(plotPlugins.size()-1);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::DestroyPlotMenuItem
//
// Purpose: 
//   Destroys the i'th plot menu.
//
// Arguments:
//   index : The index of the plot menu to destroy.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:02:32 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::DestroyPlotMenuItem(int index)
{
    PluginEntry &entry = plotPlugins[index];
    if(entry.varMenu != 0)
    {
        disconnect(entry.varMenu, SIGNAL(activated(int, const QString &)),
                   this, SLOT(addPlotHelper(int, const QString &)));
        delete entry.varMenu;
        entry.varMenu = 0;
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::CreatePlotMenuItem
//
// Purpose: 
//   Creates the i'th plot menu.
//
// Arguments:
//   index : The index of the plot menu to create.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Dec 20 12:03:16 PST 2007
//
// Modifications:
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::CreatePlotMenuItem(int index)
{
    PluginEntry &entry = plotPlugins[index];
   
    entry.varMenu = new QvisVariablePopupMenu(index, 0);
    entry.varMenu->setTitle(entry.pluginName);
    if(!entry.icon.isNull())
        entry.varMenu->setIcon(entry.icon);   
    connect(entry.varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(addPlotHelper(int, const QString &)));
   
    entry.action = plotMenu->addMenu(entry.varMenu);
    entry.action->setEnabled(false);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::AddOperatorType
//
// Purpose: 
//   Adds a new operator to the operator menu.
//
// Arguments:
//   operatorName : The name of the new operator.
//   iconData     : A pointer to icon data. This pointer can be NULL.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 09:47:42 PDT 2001
//
// Modifications:
//   Kathleen Bonnell, Thu Jun  6 14:53:21 PDT 2002
//   Force Lineout operator to be disabled.
//
//   Brad Whitlock, Thu Mar 13 09:47:38 PDT 2003
//   I added support for icons in the menu.
//
//   Brad Whitlock, Thu Aug 21 13:41:21 PST 2003
//   I prevented icons from being created since applications on MacOS X don't
//   get to put icons in the top menu.
//
//   Brad Whitlock, Tue Apr 25 16:32:06 PST 2006
//   Added support for operators that set the contents of the variable menu.
//
//   Brad Whitlock, Thu Dec 20 10:34:29 PST 2007
//   Fill in some new PluginEntry members.
//
//   Brad Whitlock, Fri Apr 25 10:20:31 PDT 2008
//   Made operatorName be a QString so we can internationalize the operator names.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::AddOperatorType(const QString &operatorName,
    const int varTypes, const int varMask, bool userSelectable,
    const char **iconData)
{
    //int id = operatorMenu->count() - 3;

    // Add the operator plugin information to the operator plugin list.
    PluginEntry entry;
    entry.pluginName = operatorName;
    entry.menuName = operatorName + QString(" . . .");
    entry.varMenu = 0;
    entry.varTypes = varTypes;
    entry.varMask = varMask;

    if(iconData)
    {
        // Add the operator type to the operator menu.
        QPixmap  iconPixmap(iconData);
        QPixmapCache::insert(operatorName, iconPixmap);
        QIcon icon(iconPixmap);

        // Create the sub-menus.
        entry.action = operatorMenu->addAction(icon, entry.pluginName);

        // Add the operator type to the operator attributes list.
        operatorAttsMenu->addAction(icon, entry.menuName);
    }
    else
    {
        // Create the sub-menus.
        entry.action = operatorMenu->addAction(entry.pluginName);

        // Add the operator type to the operator attributes list.
        operatorAttsMenu->addAction(entry.menuName);
    }

    operatorPlugins.push_back(entry);
    entry.action->setEnabled(userSelectable);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::FinishAddingOperators
//
// Purpose: 
//   Adds some menu options to the operator menu.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Jul  9 13:35:18 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::FinishAddingOperators()
{
    if(operatorRemoveLastAct == 0)
    {
        QPixmap removeLast(removelastoperator_xpm);
        QPixmap removeAll(removealloperators_xpm);

        operatorMenu->addSeparator();    
        operatorRemoveLastAct = operatorMenu->addAction(QIcon(removeLast), tr("Remove last"));
        operatorRemoveAllAct  = operatorMenu->addAction(QIcon(removeAll), tr("Remove all"));
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::EnablePluginMenus
//
// Purpose: 
//   Enables the plot and operator attribute menus.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 6 15:57:53 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Jul 3 12:56:52 PDT 2002
//   Added code to force the plot menu to redraw so it works with Qt 3.0.
//   
//   Brad Whitlock, Thu Mar 20 12:16:08 PDT 2003
//   I prevented the menus from being enabled if there are no plugins.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::EnablePluginMenus()
{
    pluginsLoaded = true;
    plotAttsMenuAct->setEnabled(plotAttsMenu->actions().count() > 0 );
    operatorAttsMenuAct->setEnabled(operatorAttsMenu->actions().count() > 0 );
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::PopulateVariableLists
//
// Purpose: 
//   Reads the variable metadata for the specified file from the
//   fileServer and puts the results in some class variables.
//
// Arguments:
//   populator : The menu populator that we want to update.
//   filename  : The file for which to get metadata.
//
// Returns:    True if an updates is needed; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:28:43 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 17 15:44:44 PST 2003
//   I made it use a menu populator.
//
//   Brad Whitlock, Tue Feb 24 16:25:25 PST 2004
//   I added the populator argument and I made the method return bool.
//
//   Mark C. Miller, Thu Mar 18 20:36:59 PST 2004
//   Added code to get metadata directly from server if it is not invariant
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Changed interfaces to GetMetaData and GetSIL
//   Added else case to GetSIL for ANY_STATE
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
// ****************************************************************************

bool
QvisPlotManagerWidget::PopulateVariableLists(VariableMenuPopulator &populator,
    const QualifiedFilename &filename)
{
    // Get a pointer to the specified file's metadata object.
    const avtDatabaseMetaData *md =
        fileServer->GetMetaData(filename,
                                GetStateForSource(filename),
                                 FileServerList::ANY_STATE,
                                !FileServerList::GET_NEW_MD);

    if (fileServer->GetTreatAllDBsAsTimeVarying() ||
        (md && md->GetMustRepopulateOnStateChange()))
    {
        // we need metadata and sil for current state
        md = fileServer->GetMetaData(filename,
                                     GetStateForSource(filename),
                                    !FileServerList::ANY_STATE,
                                     FileServerList::GET_NEW_MD);

        const avtSIL *sil =
            fileServer->GetSIL(filename,
                               GetStateForSource(filename),
                              !FileServerList::ANY_STATE,
                               FileServerList::GET_NEW_MD);
    
        return populator.PopulateVariableLists(filename.FullName(),
                                               md, sil, exprList,
                         fileServer->GetTreatAllDBsAsTimeVarying());
    }
    else
    {
        // any metadata and sil will do
        const avtSIL *sil =
            fileServer->GetSIL(filename,
                               GetStateForSource(filename),
                               FileServerList::ANY_STATE,
                              !FileServerList::GET_NEW_MD);
    
        return populator.PopulateVariableLists(filename.FullName(),
                                               md, sil, exprList,
                         fileServer->GetTreatAllDBsAsTimeVarying());
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdatePlotVariableMenu
//
// Purpose: 
//   Updates all of the variable lists in the plots menu.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 18:24:22 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 10 16:22:35 PST 2000
//   Updated for the material plot.
//
//   Hank Childs, Tue Jan 16 16:38:52 PST 2001
//   Add volume plots.
//
//   Brad Whitlock, Fri Feb 16 14:46:02 PST 2001
//   I enabled the contour plot.
//
//   Eric Brugger, Mon Mar  5 14:46:55 PST 2001
//   Disable the Range Volume and Iso Surface Volume plots.
//
//   Kathleen Bonnell,  Tue Mar  6 13:40:59 PST 2001
//   Enabled Surface plot. 
//
//   Hank Childs, Fri Mar 23 10:01:51 PST 2001
//   Enabled vector plot.
//
//   Brad Whitlock, Mon Mar 26 11:01:28 PDT 2001
//   I rewrote the code so it is more general. This allows the widget to
//   handle plot plugins.
//
//   Eric Brugger, Thu Nov 29 12:53:11 PST 2001
//   I removed the private data member matVars.
//
//   Kathleen Bonnell, Fri Jun  7 09:44:45 PDT 2002      
//   Temporarily disable CurvePlots. 
//
//   Brad Whitlock, Wed Jul 3 12:56:52 PDT 2002
//   Added code to force the plot menu to redraw so it works with Qt 3.0.
//
//   Brad Whitlock, Mon Mar 17 15:45:49 PST 2003
//   I made it use a variable menu populator.
//
//   Hank Childs, Fri Aug  1 21:30:30 PDT 2003
//   Re-enable Curve plots, since they now require Curve variables.
//
//   Brad Whitlock, Tue Feb 24 16:06:46 PST 2004
//   I improved the code to it does not update the variable menus each time.
// 
//   Brad Whitlock, Fri Dec 3 13:53:20 PST 2004
//   Moved the code to clear the menu out of the variable menu populator
//   into this method. I also changed how the slot function to add a plot
//   gets hooked up by the variable menu populator. Finally, I added code
//   to update variable buttons that use the active source for their
//   variable list.
//
//   Brad Whitlock, Wed Mar 22 09:08:31 PDT 2006
//   I added code to time menu creation.
//
//   Brad Whitlock, Thu Dec 20 12:01:40 PST 2007
//   I changed how the menus are made so that we destroy menus when needed
//   since clearing them does not seem to really destroy them.
//
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotVariableMenu()
{
    const char *mName = "QvisPlotManagerWidget::UpdatePlotVariableMenu: ";
    int total = visitTimer->StartTimer();
    int id = visitTimer->StartTimer();

    //
    // Update the menu populator so it uses the current file. If it changed
    // then needsUpdate will be true and we need to update the variable menu.
    //
    bool needsUpdate = PopulateVariableLists(menuPopulator,
        fileServer->GetOpenFile());
    visitTimer->StopTimer(id, "PopulateVariableLists");

    debug4 << mName << "Need to update menus: " << (needsUpdate?"true":"false") << endl;

    // Update the various menus
    if(needsUpdate)
    {
        id = visitTimer->StartTimer();

        // Clear out the plot menu and destroy all of the plot menus in it so
        // we can recreate the menu,
#ifdef DELETE_MENU_TO_FREE_POPUPS
        plotMenu->clear();
        for(int i = 0; i < plotPlugins.size(); ++i)
            DestroyPlotMenuItem(i);
#endif

        // Recreate the plot menu and update the menus so they have the right 
        // variables.
        this->maxVarCount = 0;
        for(int i = 0; i < plotPlugins.size(); ++i)
        {
#ifdef DELETE_MENU_TO_FREE_POPUPS
            CreatePlotMenuItem(i);
#else
            plotPlugins[i].varMenu->clear();
#endif
            int varCount = menuPopulator.UpdateSingleVariableMenu(
                plotPlugins[i].varMenu, plotPlugins[i].varTypes,
                this, SLOT(addPlotHelper(int, const QString &)));
            this->maxVarCount = (varCount > this->maxVarCount) ? varCount : this->maxVarCount;
            bool hasEntries = (varCount > 0);
            plotMenu->actions()[i]->setEnabled(hasEntries);
        }
        visitTimer->StopTimer(id, "Updating menus");

        //
        // Set the flag to indicate that we need to update the enabled
        // state for the plot variable menu.
        //
        this->updatePlotVariableMenuEnabledState = true;

        //
        // Update the variable buttons that use the active source.
        //
        id = visitTimer->StartTimer();
        QvisVariableButton::UpdateActiveSourceButtons(&menuPopulator);
        visitTimer->StopTimer(id, "Updating active source buttons");

        //
        // If there are no plots then update the variable buttons that
        // use the plot source with the active source.
        //
        id = visitTimer->StartTimer();
        if(plotList->GetNumPlots() < 1)
            QvisVariableButton::UpdatePlotSourceButtons(&menuPopulator);
        visitTimer->StopTimer(id, "Updating plot source buttons");
    }

    visitTimer->StopTimer(total, "QvisPlotManagerWidget::UpdatePlotVariableMenu");
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdatePlotAndOperatorMenuEnabledState
//
// Purpose: 
//   Determines when the "Plots" and "Operators" options in the Plot menu
//   should be enabled and sets their enabled state accordingly.
//
// Note:       You should only set the enabled state for the "Plots" and
//             "Operators" menu options by calling this method because when
//             it was being set all over the place, there was not consistent
//             rule and now we have one in the form of this method.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jul 28 17:49:35 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Feb 26 08:09:07 PDT 2004
//   I made this method be the central place where the menu enabled state is
//   set instead of doing it all over the place. This allows me to set the
//   menu state consistently without code duplication.
//
//   Brad Whitlock, Fri Apr 15 09:20:00 PDT 2005
//   I removed code to disable the widgets when the engine is executing.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotAndOperatorMenuEnabledState()
{
    //
    // These values will be used to set the enabled state for the items in
    // the plot and operator menu.
    //
    bool plotMenuEnabled = plotMenuAct->isEnabled();
    bool plotAttsMenuEnabled = plotAttsMenuAct->isEnabled();
    bool operatorMenuEnabled = operatorMenuAct->isEnabled();
    bool operatorAttsMenuEnabled = operatorAttsMenuAct->isEnabled();
    bool varMenuEnabled = varMenuAct->isEnabled();

    if(pluginsLoaded)
    {
        // Look through the menus for the available plots to see how many
        // are enabled. If any are enabled, then consider that we may want
        // to enable the plot menu.
        int i;
        bool somePlotMenusEnabled = false;
        for(i = 0; i < plotPlugins.size(); ++i)
            somePlotMenusEnabled |= (plotPlugins[i].varMenu->count() > 0);
        
        bool someOperatorMenusEnabled = false;
        
        foreach(QAction *action,operatorMenu->actions())
            someOperatorMenusEnabled |= action->isEnabled();        
        
        bool haveAvailablePlots = plotAttsMenu->actions().count() > 0;
        bool haveAvailableOperators = operatorAttsMenu->actions().count() > 0;
        bool haveOpenFile = !fileServer->GetOpenFile().Empty();

        plotMenuEnabled = haveAvailablePlots &&
                          somePlotMenusEnabled &&
                          haveOpenFile;
        operatorMenuEnabled = haveAvailableOperators &&
                              (plotList->GetNumPlots() > 0) &&
                              someOperatorMenusEnabled;
        varMenuEnabled = (varMenu->count() > 0);
    }

    //
    // Check each menu that we want to update to see if we need to update.
    //
    bool needUpdate = false;
    bool different = false;
    if(this->updatePlotVariableMenuEnabledState)
    {
        different = plotMenuAct->isEnabled() != plotMenuEnabled;
        if(different)
            plotMenuAct->setEnabled(plotMenuEnabled);
        needUpdate |= different;
    }

    if(this->updateOperatorMenuEnabledState)
    {
        different = operatorMenuAct->isEnabled() != operatorMenuEnabled;
        if(different)
            operatorMenuAct->setEnabled(operatorMenuEnabled);
        needUpdate |= different;
    }

    if(this->updateVariableMenuEnabledState)
    {
        different = varMenuAct->isEnabled() != varMenuEnabled;
        if(different)
            varMenuAct->setEnabled(varMenuEnabled);
        needUpdate |= different;
    }

    different = operatorAttsMenuAct->isEnabled() != operatorAttsMenuEnabled;
    if(different)
    {
        operatorAttsMenuAct->setEnabled(operatorAttsMenuEnabled);
        needUpdate |= different;
    }

    different = plotAttsMenuAct->isEnabled() != plotAttsMenuEnabled;
    if(different)
    {
        plotAttsMenuAct->setEnabled(plotAttsMenuEnabled);
        needUpdate |= different;
    }

    //
    // If we updated a menu's enabled state, update the menu bar.
    //
    if(needUpdate)
        plotMenuBar->update();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdateVariableMenu
//
// Purpose: 
//   Updates the variable menu so it contains the variable list for
//   the current plot's file and variable type.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:27:48 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 10 16:21:45 PST 2000
//   Updated for the material plot.
//
//   Brad Whitlock, Mon Mar 26 10:55:17 PDT 2001
//   Changed to be more generic so it will work with plugins.
//
//   Brad Whitlock, Wed Feb 6 10:49:18 PDT 2002
//   Modified to support cascading menus.
//
//   Brad Whitlock, Thu May 9 15:14:02 PST 2002
//   Made it so the menu isn't enabled when the engine is executing.
//
//   Brad Whitlock, Wed Jul 3 12:56:52 PDT 2002
//   Added code to force the plot menu to redraw so it works with Qt 3.0.
//
//   Brad Whitlock, Mon Mar 17 15:46:30 PST 2003
//   I made it use a menu populator.
//
//   Brad Whitlock, Tue Feb 24 16:20:36 PST 2004
//   I made it use a separate menu populator.
//
//   Brad Whitlock, Mon Mar 15 11:49:52 PDT 2004
//   I made it take the plot's varTypes into account when determining if the
//   variable menu needs to be updated.
//
//   Brad Whitlock, Fri Dec 3 13:53:20 PST 2004
//   Moved the code to clear the menu out of the variable menu populator
//   into this method. I also changed how the slot function to change
//   variables is hooked up to the menu. Finally, I added code to update
//   the variable buttons that use the plot source.
//
//   Brad Whitlock, Tue Apr 25 16:37:26 PST 2006
//   Added support for operators setting the type of variables that we want
//   to appear in the variable list.
//
//   Brad Whitlock, Thu Dec 20 12:15:21 PST 2007
//   Changed how the variable menu gets cleared.
//
//   Brad Whitlock, Tue Mar 25 15:54:45 PST 2008
//   Only recreate the menu if it is not empty to avoid a weird crash in Qt
//   under certain conditions.
//
// ****************************************************************************

void
QvisPlotManagerWidget::UpdateVariableMenu()
{
    // Update the variable lists using the type of the first plot as the
    // kind of variable that is displayed.
    for(int i = 0; i < plotList->GetNumPlots(); ++i)
    {
        // Create a constant reference to the current plot.
        const Plot &current = plotList->operator[](i);

        if(current.GetActiveFlag())
        {
            // Set the plot type for the variable menu.
            varMenu->setPlotType(current.GetPlotType());

            //
            // Update the variable menu's menu populator and if an update
            // is needed, update the menu.
            //
            bool changeVarLists = PopulateVariableLists(varMenuPopulator,
                current.GetDatabaseName());
            int plotVarFlags = plotPlugins[current.GetPlotType()].varTypes;
            for(int j = 0; j < current.GetOperators().size(); ++j)
            {
                int opid = current.GetOperators()[j];
                plotVarFlags &= operatorPlugins[opid].varMask;
                plotVarFlags |= operatorPlugins[opid].varTypes;
            }
            bool flagsDiffer = (plotVarFlags != varMenuFlags);
            if(changeVarLists || flagsDiffer || varMenu->count() == 0)
            {
                // Destroy and recreate the variable menu so we actually
                // delete menu items when we no longer need them.
#ifdef DELETE_MENU_TO_FREE_POPUPS
                if(varMenu->count() > 0)
                {
                    DestroyVariableMenu();
                    CreateVariableMenu();
                }
#else
                varMenu->clear();
#endif
                // Set the variable list based on the first active plot.
                int varCount = varMenuPopulator.UpdateSingleVariableMenu(varMenu,
                    plotVarFlags, this, SLOT(changeVariable(int, const QString &)));
                varMenuFlags = plotVarFlags;

                //
                // Set the flag that indicates that we need to update the
                // enabled state for the variables menu.
                //
                this->updateVariableMenuEnabledState |= (varCount > 0);

                //
                // Update all variable buttons that use the plot source.
                //
                QvisVariableButton::UpdatePlotSourceButtons(&varMenuPopulator);
            }

            // Get out of the for loop.
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::SubjectRemoved
//
// Purpose: 
//   Removes the plotlist or fileserver subjects that this widget
//   observes.
//
// Arguments:
//   TheRemovedSubject : The subject that is being removed.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 11 16:28:44 PST 2000
//
// Modifications:
//   Jeremy Meredith, Wed Sep  5 14:03:23 PDT 2001
//   Added pluginAtts.
//
//   Brad Whitlock, Fri Feb 1 15:05:18 PST 2002
//   Added exprList to prevent memory problems.
//
//   Brad Whitlock, Thu May 9 16:47:36 PST 2002
//   Removed fileServer since it is now a static member of the base class.
//
//   Brad Whitlock, Fri Jan 30 00:36:54 PDT 2004
//   Added windowInfo.
//
// ****************************************************************************

void
QvisPlotManagerWidget::SubjectRemoved(Subject *TheRemovedSubject)
{
    if(TheRemovedSubject == plotList)
        plotList = 0;
    else if(TheRemovedSubject == globalAtts)
        globalAtts = 0;
    else if(TheRemovedSubject == exprList)
        exprList = 0;
    else if(TheRemovedSubject == windowInfo)
        windowInfo = 0;
}

//
// Methods to attach to the plotlist and fileserver objects.
//

void
QvisPlotManagerWidget::ConnectPlotList(PlotList *plotList_)
{
    plotList = plotList_;
    plotList->Attach(this);
}

void
QvisPlotManagerWidget::ConnectFileServer(FileServerList *)
{
    fileServer->Attach(this);
}

void
QvisPlotManagerWidget::ConnectExpressionList(ExpressionList *exprList_)
{
    exprList = exprList_;
    exprList->Attach(this);
}

void
QvisPlotManagerWidget::ConnectGlobalAttributes(GlobalAttributes *ga)
{
    globalAtts = ga;
    globalAtts->Attach(this);
}

void
QvisPlotManagerWidget::ConnectWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    windowInfo->Attach(this);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::keyReleaseEvent
//
// Purpose: 
//   This is an event handler method that processes key release events.
//
// Arguments:
//   key : A pointer to a key event object.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 14 14:02:38 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Apr 10 15:59:22 PST 2003
//   I made it emit the activatePlotWindow signal directly.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::keyReleaseEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Delete)
        deletePlots();
    else if(key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return)
    {
        // Activate the windows for all of the selected plots.
        int top = qMax(plotListBox->count(), plotList->GetNumPlots());
        for(int i = 0; i < top; ++i)
        {
            if(plotListBox->item(i)->isSelected())
            {
                int plotType = plotList->GetPlots(i).GetPlotType();
                emit activatePlotWindow(plotType);
            }
        }
    }

    // Call the base class's method.
    QWidget::keyReleaseEvent(key);
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisPlotManagerWidget::hidePlots
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to hide the
//   active plots.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:24:49 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::hidePlots()
{
    // hide the active plots.
    GetViewerMethods()->HideActivePlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::deletePlots
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to delete the
//   active plots.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:24:49 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu May 2 15:24:19 PST 2002
//   Sets the wait cursor before deleting the plot.
//
// ****************************************************************************

void
QvisPlotManagerWidget::deletePlots()
{
    // Set the wait cursor. It will change back when the viewer returns
    // the updated plot list.
    SetWaitCursor();
 

    // delete the active plots.
    GetViewerMethods()->DeleteActivePlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::drawPlots
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to draw all of
//   the undrawn plots.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:24:49 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::drawPlots()
{
    // Draw the plots.
    GetViewerMethods()->DrawPlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::setActivePlots
//
// Purpose: 
//   This is a Qt slot function that tells the viewer which plots are active.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 6 20:26:08 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Sep 19 13:41:21 PST 2001
//   I made setting the active plots zero-origin.
//
//   Brad Whitlock, Fri Mar 1 14:35:43 PST 2002
//   I made the viewer open the same database as the GUI because it gets
//   out of sync with what we're doing.
//
//   Brad Whitlock, Thu Oct 24 15:36:19 PST 2002
//   I made it so the viewer is not told about a zero length active plot list
//   because it ended up causing bad things to happen with memory.
//
//   Brad Whitlock, Thu Dec 26 17:34:38 PST 2002
//   I made it use an intVector for the plots since the ViewerRPC now uses
//   an intVector.
//
//   Brad Whitlock, Wed Apr 9 13:00:24 PST 2003
//   I added code to make sure that the selected plots are different from
//   the currently selected plots before sending a new selection to the viewer.
//   I then extended it to take into account active operators and expanded
//   plots.
//
//   Brad Whitlock, Thu May 15 13:11:27 PST 2003
//   I made the viewer open the database at the current time state.
//
//   Eric Brugger, Mon Jul 28 16:38:12 PDT 2003
//   Removed the code that has the viewer open the database associated with
//   the first active plot since that is now done by the viewer when the
//   active plots are changed.
//
//   Brad Whitlock, Mon Dec 8 15:43:08 PST 2003
//   I removed code to open a database in the file server since we now do
//   not want the open database to change when we select active plots.
//
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::setActivePlots()
{
    int i;
    intVector existingPlotSelection,     newPlotSelection;
    intVector existingOperatorSelection, newOperatorSelection;
    intVector existingExpandedPlots,     newExpandedPlots;
    int firstSelectedFile = 0;
    bool found = false;
 
    //
    // Build a list of the active plots according to the plotListBox widget.
    //
    for(i = 0; i < plotListBox->count(); ++i)
    {
        if(plotListBox->item(i)->isSelected())
        {
            newPlotSelection.push_back(i);

            // Record the first selected file.
            if(!found)
            {
                found = true;
                firstSelectedFile = i;
            }
        }

        newOperatorSelection.push_back(plotListBox->activeOperatorIndex(i));
        newExpandedPlots.push_back(plotListBox->isExpanded(i));
    }

    //
    // Build a list of the active plots according to the plot list.
    //
    for(i = 0; i < plotList->GetNumPlots(); ++i)
    {
        const Plot &current = plotList->operator[](i);
        if(current.GetActiveFlag())
            existingPlotSelection.push_back(i);
        existingOperatorSelection.push_back(current.GetActiveOperator());
        existingExpandedPlots.push_back(current.GetExpandedFlag());
    }

    //
    // Only tell the viewer about a new plot selection if the new selection
    // is different from the old selection.
    //
    if(newPlotSelection != existingPlotSelection ||
       newOperatorSelection != existingOperatorSelection ||
       newExpandedPlots != existingExpandedPlots)
    {
        // Tell the viewer the new active plots.
        if(newPlotSelection.size() > 0)
        {
            GetViewerMethods()->SetActivePlots(newPlotSelection, 
                newOperatorSelection, newExpandedPlots);
        }
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::promoteOperator
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to promote an operator.
//
// Arguments:
//   operatorIndex : The index of the operator to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 14:31:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::promoteOperator(int operatorIndex)
{
    GetViewerMethods()->PromoteOperator(operatorIndex);
    if(AutoUpdate())
        GetViewerMethods()->DrawPlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::demoteOperator
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to demote an operator.
//
// Arguments:
//   operatorIndex : The index of the operator to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 14:31:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::demoteOperator(int operatorIndex)
{
    GetViewerMethods()->DemoteOperator(operatorIndex);
    if(AutoUpdate())
        GetViewerMethods()->DrawPlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::removeOperator
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to remove an operator.
//
// Arguments:
//   operatorIndex : The index of the operator to remove.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 14:31:11 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::removeOperator(int operatorIndex)
{
    GetViewerMethods()->RemoveOperator(operatorIndex);
    if(AutoUpdate())
        GetViewerMethods()->DrawPlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::activatePlotWindow
//
// Purpose: 
//   This is a Qt slot function called when the plot window is changed.
//
// Arguments:
//   action: The menu item that was clicked.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  8 13:38:08 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::activatePlotWindow(QAction *action)
{
    emit activatePlotWindow(plotAttsMenu->actions().indexOf(action));
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::activateOperatorWindow
//
// Purpose: 
//   This is a Qt slot function called when the operator window is changed.
//
// Arguments:
//   action: The menu item that was clicked.
//
// Programmer: Cyrus Harrison
// Creation:   Tue Jul  8 13:38:08 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::activateOperatorWindow(QAction *action)
{
    emit activateOperatorWindow(operatorAttsMenu->actions().indexOf(action));
}


// ****************************************************************************
// Method: QvisPlotManagerWidget::changeVariable
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to change the variable
//   for the selected plots.
//
// Arguments:
//   varName : The complete name of the variable to which we're switching.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 09:40:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 6 11:32:27 PDT 2002
//   Changed to support cascading menus.
//
//   Kathleen Bonnell, Wed Sep  4 16:14:12 PDT 2002  
//   Removed call to ParseVariable. 
//
// ****************************************************************************

void
QvisPlotManagerWidget::changeVariable(int, const QString &varName)
{
    // Tell the viewer to change the variables of the selected
    // plots to a new variable.
    GetViewerMethods()->ChangeActivePlotsVar(varName.toStdString());
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::addPlotHelper
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to add a new plot.
//
// Arguments:
//   plotType : The type of plot to create.
//   varName  : The name of the variable to use in creating the plot.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 09:40:50 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Dec 14 08:54:23 PDT 2004
//   Changed all of the code to an emitted signal. The code to handle
//   the signal is in QvisGUIApplication.
//
// ****************************************************************************

void
QvisPlotManagerWidget::addPlotHelper(int plotType, const QString &varName)
{
    emit addPlot(plotType, varName);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::operatorAction
//
// Purpose: 
//   This is a Qt slot function that adds an operator, or removes the
//   last operator, or removes all operators. It just tells the viewer
//   which action to perform.
//
// Arguments:
//   index : The index of the menu item. This tells the viewer which
//           action to perform.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 18 12:46:26 PDT 2000
//
// Modifications:
//   Kathleen Bonnell, Tue Oct 10 16:03:03 PDT 2000
//   Added onion peel operator case
//
//   Brad Whitlock, Tue Dec 12 15:02:50 PST 2000
//   Added matsel operator case.
//
//   Brad Whitlock, Mon Mar 26 09:54:00 PDT 2001
//   Rewrote to treat operators generically so they can come from plugins.
//
//   Brad Whitlock, Mon Mar 17 13:04:19 PST 2003
//   I changed how the index is used.
//
//   Brad Whitlock, Tue Dec 14 09:13:26 PDT 2004
//   I removed the code to add an operator and made it emit a signal instead.
//
//   Cyrus Harrison, Tue Jul  8 13:26:04 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPlotManagerWidget::operatorAction(QAction *action)
{
    if(action == operatorRemoveLastAct)
        GetViewerMethods()->RemoveLastOperator();
    else if(action == operatorRemoveAllAct)
        GetViewerMethods()->RemoveAllOperators();
    else
    {
        emit addOperator(operatorMenu->actions().indexOf(action));
    }
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::applyOperatorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the "Apply operator to
//   all plots" toggle is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 26 16:20:13 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::applyOperatorToggled(bool val)
{
    globalAtts->SetApplyOperator(val);
    SetUpdate(false);
    globalAtts->Notify();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::applySelectionToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the "Apply selection to
//   all plots" toggle is clicked.
//
// Arguments:
//   val : The new toggle value.
//
// Programmer: Gunther H. Weber
// Creation:   Wed Jan 23 16:16:09 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::applySelectionToggled(bool val)
{
    globalAtts->SetApplySelection(val);
    SetUpdate(false);
    globalAtts->Notify();
}


// ****************************************************************************
// Method: QvisPlotManagerWidget::sourceChanged
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to activate a source
//   that is already open.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 29 22:02:49 PST 2004
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::sourceChanged(int index)
{
    const stringVector &sources = globalAtts->GetSources();
    if(index >= 0 && index < sources.size())
        GetViewerMethods()->ActivateDatabase(sources[index]);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::hideThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to hide the
//   active plot. 
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Mon, June 25, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::hideThisPlot()
{
    // hide the active plot.
    GetViewerMethods()->HideActivePlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::deleteThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to delete the
//   active plot.
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Mon, June 25, 2007
//
// ****************************************************************************

void
QvisPlotManagerWidget::deleteThisPlot()
{
    
    // delete the active plots.
    GetViewerMethods()->DeleteActivePlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::drawThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to draw the active plot.
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Mon June 25, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::drawThisPlot()
{
    // Draw the active plots.
    GetViewerMethods()->DrawPlots(false);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::setPlotDescription
//
// Purpose: 
//   Sets the new name for a plot.
//
// Arguments:
//   index : The index of the plot to rename.
//   newName : The new name of the plot.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 22 11:03:44 PDT 2009
//
// Modifications:
//   
// ****************************************************************************


void
QvisPlotManagerWidget::setPlotDescription(int index, const QString &newName)
{
    GetViewerMethods()->SetPlotDescription(index, newName.toStdString());
    ClearStatus();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::moveThisPlotTowardFirst
//
// Purpose: 
//   Move the active plot around in the plot list.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 22 11:04:37 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::moveThisPlotTowardFirst()
{
    int index = GetViewerState()->GetPlotList()->FirstSelectedIndex();
    if(index >= 0)
        GetViewerMethods()->MovePlotOrderTowardFirst(index);
    ClearStatus();
}

void
QvisPlotManagerWidget::moveThisPlotTowardLast()
{
    int index = GetViewerState()->GetPlotList()->FirstSelectedIndex();
    if(index >= 0)
        GetViewerMethods()->MovePlotOrderTowardLast(index);
    ClearStatus();
}

void
QvisPlotManagerWidget::makeThisPlotFirst()
{
    int index = GetViewerState()->GetPlotList()->FirstSelectedIndex();
    if(index >= 0)
        GetViewerMethods()->SetPlotOrderToFirst(index);
    ClearStatus();
}

void
QvisPlotManagerWidget::makeThisPlotLast()
{
    int index = GetViewerState()->GetPlotList()->FirstSelectedIndex();
    if(index >= 0)
        GetViewerMethods()->SetPlotOrderToLast(index);
    ClearStatus();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::clearThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to clear 
//   the active plot.
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Thurs July 12, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisPlotManagerWidget::clearThisPlot()
{
    // Clear the active plots.
    GetViewerMethods()->ClearWindow(false);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::copyThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to copy 
//   the active plot to the plot list, but leave it undrawn, so user can 
//   manipulate it...
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Thurs Sept 27, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisPlotManagerWidget::copyThisPlot()
{
    // Copy the active plot.
    GetViewerMethods()->CopyActivePlots();
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::copyToWinThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to copy
//   the active plot to a new window.
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Thurs July 12, 2007
//
// Modifications:
//   Cyrus Harrison, Thu Jul  3 09:16:15 PDT 2008
//   Initial Qt4 Port.
//
// ****************************************************************************
void
QvisPlotManagerWidget::copyToWinThisPlot()
{
// this is part of the context menu - INCOMPLETE
    QMessageBox::information( this, tr("Copy This Plot To Window"), tr("Not Yet Implemented..."), 
                              QMessageBox::Cancel | QMessageBox::Default);
    
    // add in a pop-up menu to allow user to select target window:
    win1Act = new QAction(tr("Window 1"), 0);
    win1Act->setStatusTip(tr("Copy Plot to Window 1"));
    connect( win1Act, SIGNAL(toggled(bool)), this, SIGNAL(CopyPlotToWin(1)));
    
    win2Act = new QAction(tr("Window 2"), 0);
    win2Act->setStatusTip(tr("Copy Plot to Window 2"));
    connect( win2Act, SIGNAL(toggled(bool)), this, SIGNAL(copyPlotToWin(2)));

    WindowChoiceMenu = new QMenu(this);
    WindowChoiceMenu->addAction(win1Act);
    WindowChoiceMenu->addAction(win2Act);
    
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::copyPlotToWin
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to copy 
//   the active plot to the plot list, but leave it undrawn, so user can 
//   manipulate it...
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Mon Oct 22, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisPlotManagerWidget::copyPlotToWin(int winIndex)
{
// this is part of the context menu - INCOMPLETE...
    // Copy the active plot.

    QMessageBox::information( this, tr("Copy This Plot To Window"), tr("Not Yet Implemented..."), 
                              QMessageBox::Cancel | QMessageBox::Default);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::redrawThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to clear and then redraw
//   the active plot.
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Thurs July 12, 2007
//
// Modifications:
//   
// ****************************************************************************
void
QvisPlotManagerWidget::redrawThisPlot()
{
    // clear the plot:
    clearThisPlot();
    drawThisPlot();		      
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::disconnectThisPlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer to disconnect 
//   the active plot from the TimeSlider
//   (Used by the Right-Click Context Menu in the Plot List Box)
//
// Programmer: Ellen Tarwater
// Creation:   Weds, Nov 28, 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisPlotManagerWidget::disconnectThisPlot()
{

    // Disconnect the active plot from the time slider:
    GetViewerMethods()->SetPlotFollowsTime();			      
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::setActivePlot
//
// Purpose: 
//   This is a Qt slot function that tells the viewer which plots are active.
//
// Programmer: Ellen Tarwater
// Creation:   Wed July 18, 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisPlotManagerWidget::setActivePlot()
{
    int i;
    intVector existingPlotSelection,     newPlotSelection;
    int firstSelectedFile = 0;
    bool found = false;

    //
    // Build a list of the active plots according to the plotListBox widget.
    //
    for(i = 0; i < plotListBox->count(); ++i)
    {
        if(plotListBox->item(i)->isSelected())
        {
            newPlotSelection.push_back(i);

            // Record the first selected file.
            if(!found)
            {
                found = true;
                firstSelectedFile = i;
            }
        }

    }

   //
    // Only tell the viewer about a new plot selection if the new selection
    // is different from the old selection.
    //
    if(newPlotSelection != existingPlotSelection )
    {
        // Tell the viewer the new active plots.
        if(newPlotSelection.size() > 0)
        {
            GetViewerMethods()->SetActivePlots(newPlotSelection);
        }
    }
}
