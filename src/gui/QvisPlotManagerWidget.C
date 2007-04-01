#include <QvisPlotManagerWidget.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qiconset.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpixmapcache.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>

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
#include <QvisVariablePopupMenu.h>
#include <PlotPluginInfo.h>
#include <WindowInformation.h>

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
// ****************************************************************************

QvisPlotManagerWidget::QvisPlotManagerWidget(QMenuBar *menuBar,
    QWidget *parent, const char *name) : QWidget(parent, name), GUIBase(),
    SimpleObserver(), menuPopulator(), varMenuPopulator(), plotPlugins()
{
    metaData = 0;
    plotList = 0;
    globalAtts = 0;
    windowInfo = 0;
    exprList = 0;
    pluginAtts = 0;

    pluginsLoaded = false;
    updatePlotVariableMenuEnabledState = false;
    updateOperatorMenuEnabledState = false;
    updateVariableMenuEnabledState = false;
    maxVarCount = 0;
    varMenuFlags = 0;
    sourceVisible = false;

    topLayout = new QGridLayout(this, 5, 4);
    topLayout->setSpacing(5);

    // Create the source combobox.
    sourceComboBox = new QComboBox(this, "sourceComboBox");
    sourceComboBox->hide();
    connect(sourceComboBox, SIGNAL(activated(int)),
            this, SLOT(sourceChanged(int)));
    sourceLabel = new QLabel(sourceComboBox, "Source", this, "sourceLabel");
    sourceLabel->hide();
    topLayout->addWidget(sourceLabel, 0, 0);
    topLayout->addMultiCellWidget(sourceComboBox, 0, 0, 1, 3);

    activePlots = new QLabel("Active plots", this, "activePlots");
    topLayout->addWidget(activePlots, 1, 0);

    // Create the hide/show button.
    hideButton = new QPushButton("Hide/Show", this, "hideButton");
    hideButton->setEnabled(false);
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hidePlots()));
    topLayout->addWidget(hideButton, 1, 1);

    // Create the delete button.
    deleteButton = new QPushButton("Delete", this, "deleteButton");
    deleteButton->setEnabled(false);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deletePlots()));
    topLayout->addWidget(deleteButton, 1, 2);

    // Create the draw button.
    drawButton = new QPushButton("Draw", this, "drawButton");
    drawButton->setEnabled(false);
    connect(drawButton, SIGNAL(clicked()), this, SLOT(drawPlots()));
    topLayout->addWidget(drawButton, 1, 3);

    // Create the plot list box.
    plotListBox = new QvisPlotListBox(this, "plotListBox");
    plotListBox->setSelectionMode(QListBox::Extended);
    plotListBox->setMinimumHeight(fontMetrics().boundingRect("X").height() * 6);
    connect(plotListBox, SIGNAL(selectionChanged()),
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


    topLayout->addMultiCellWidget(plotListBox, 2, 2, 0, 3);

    // Create the "Apply operator to all plots" toggle.
    applyOperatorToggle = new QCheckBox("Apply operators and selection to all plots", this,
        "applyOperatorToggle");
    connect(applyOperatorToggle, SIGNAL(toggled(bool)),
            this, SLOT(applyOperatorToggled(bool)));
    topLayout->addMultiCellWidget(applyOperatorToggle, 4, 4, 0, 3);

    // Create the plot and operator menus. Note that they will be empty until
    // they are populated by the main application.
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
// ****************************************************************************

QvisPlotManagerWidget::~QvisPlotManagerWidget()
{
    if(metaData)
        metaData->Detach(this);

    if(plotList)
        plotList->Detach(this);

    if(fileServer)
        fileServer->Detach(this);

    if(globalAtts)
        globalAtts->Detach(this);

    if(exprList)
        exprList->Detach(this);

    if(pluginAtts)
        pluginAtts->Detach(this);

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
    plotMenuBar = new QMenuBar(this, "plotMenu");
    topLayout->addMultiCellWidget(plotMenuBar, 3, 3, 0, 3);
#endif

    // Create the Plot menu. Each time we highlight a plot, we
    // update the current plot type.
    plotMenu = new QPopupMenu(plotMenuBar);

    // Add the whole "Plots" menu to the menu bar.
    plotMenuId = plotMenuBar->insertItem( tr("Plots"), plotMenu );
    plotMenuBar->setItemEnabled(plotMenuId, false);

    //
    // Create the operator menu.
    //
    operatorMenu = new QPopupMenu(plotMenuBar);
    connect(operatorMenu, SIGNAL(activated(int)), this, SLOT(operatorAction(int)));
    QPixmap removeLast(removelastoperator_xpm);
    QIconSet removeLastIcon(removeLast);
    QPixmap removeAll(removealloperators_xpm);
    QIconSet removeAllIcon(removeAll);
    operatorMenu->insertSeparator();
    operatorMenu->insertItem(removeLast, "Remove last", REMOVE_LAST_OPERATOR_ID);
    operatorMenu->insertItem(removeAll, "Remove all", REMOVE_ALL_OPERATORS_ID);
    operatorMenuId = plotMenuBar->insertItem( tr("Operators"), operatorMenu );
    plotMenuBar->setItemEnabled(operatorMenuId, false);

    //
    // Create the Plot attributes menu.
    //
    plotAttsMenu = new QPopupMenu( plotMenuBar );
    connect(plotAttsMenu, SIGNAL(activated(int)),
            this, SIGNAL(activatePlotWindow(int)));
    plotAttsMenuId = plotMenuBar->insertItem(
#ifdef __APPLE__
        tr("Plot Attributes"),
#else
        tr("PlotAtts"),
#endif
        plotAttsMenu);
    plotMenuBar->setItemEnabled(plotAttsMenuId, false);

    //
    // Create the Operator attributes menu.
    //
    operatorAttsMenu = new QPopupMenu( plotMenuBar );
    connect(operatorAttsMenu, SIGNAL(activated(int)),
            this, SIGNAL(activateOperatorWindow(int)));
    operatorAttsMenuId = plotMenuBar->insertItem(
#ifdef __APPLE__
        tr("Operator Attributes"),
#else
        tr("OpAtts"),
#endif
        operatorAttsMenu );
    plotMenuBar->setItemEnabled(operatorAttsMenuId, false);

    //
    // Create an empty variable menu.
    //
    varMenu = new QvisVariablePopupMenu(-1, plotMenuBar, "varMenu");
    connect(varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(changeVariable(int, const QString &)));
    plotMenuBar->insertSeparator();
    varMenuId = plotMenuBar->insertItem( tr("Variables"), varMenu );
    plotMenuBar->setItemEnabled(varMenuId, false);
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
// ****************************************************************************

void
QvisPlotManagerWidget::Update(Subject *TheChangedSubject)
{
    if(plotList == 0 || fileServer == 0 || globalAtts == 0 ||
       pluginAtts == 0 || windowInfo == 0 || metaData == 0)
    {
        return;
    }

    // Get whether or not we are allowed to modify things.
    bool canChange = !globalAtts->GetExecuting();

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

            UpdatePlotVariableMenu();
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
        if(globalAtts->IsSelected(0))
            UpdateSourceList(false);

        // Set the "Apply operator toggle."
        applyOperatorToggle->blockSignals(true);
        applyOperatorToggle->setChecked(globalAtts->GetApplyOperator());
        applyOperatorToggle->blockSignals(false);

        // Set the enabled state for the whole widget based on whether
        // or not the engine is busy.
        activePlots->setEnabled(canChange);
        plotListBox->setEnabled(canChange);
        UpdateHideDeleteDrawButtonsEnabledState();
        applyOperatorToggle->setEnabled(canChange);

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
        if(windowInfo->IsSelected(0))
        {
            UpdateSourceList(true);

            // If the active source changed then the variable list needs
            // to change.
            UpdatePlotVariableMenu();
        }
    }
    else if(TheChangedSubject == pluginAtts)
    {
        // do nothing yet -- JSM 9/5/01
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
            plotListBox->insertItem(newPlot);
            plotListBox->setSelected(i, current.GetActiveFlag());
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
            plotListBox->setSelected(i, current.GetActiveFlag());
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
            sourceComboBox->insertItem(shortSources[i].c_str());
    }

    //
    // Set the current item.
    //
    if(sourceIndex != -1 && sourceIndex != sourceComboBox->currentItem())
        sourceComboBox->setCurrentItem(sourceIndex);

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

    bool canChange = !globalAtts->GetExecuting();
    hideButton->setEnabled(nHideablePlots > 0 && canChange);
    deleteButton->setEnabled(plotList->GetNumPlots() > 0 && canChange);
    drawButton->setEnabled(plotList->GetNumPlots() > 0 && canChange);
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
// ****************************************************************************

void
QvisPlotManagerWidget::AddPlotType(const char *plotName, const int varTypes,
    const char **iconData)
{
    PluginEntry entry;

    // Create the sub-menus.
    entry.varMenu = new QvisVariablePopupMenu(plotPlugins.size(), plotMenu,
                                              plotName);
    entry.varTypes = varTypes;
    connect(entry.varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(addPlot(int, const QString &)));
    // Add the plot plugin information to the plugin list.
    plotPlugins.push_back(entry);

    // Add the plot to the menus.
    int id;
    QString menuName(plotName);
    menuName += QString(" . . .");
    if(iconData)
    {
        // Add the plot type to the plot menu.
        QPixmap iconPixmap(iconData);
        QIconSet icon(iconPixmap);

        id = plotMenu->insertItem(icon, plotName, entry.varMenu,
                                  plotMenu->count());

        // Add the plot type to the plot attributes list.
        plotAttsMenu->insertItem(icon, menuName, plotAttsMenu->count());
    }
    else
    {
        // Add the plot type to the plot menu.
        id = plotMenu->insertItem(plotName, entry.varMenu, plotMenu->count());

        // Add the plot type to the plot attributes list.
        plotAttsMenu->insertItem(menuName, plotAttsMenu->count());
    }

    plotMenu->setItemEnabled(id, false);
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
// ****************************************************************************

void
QvisPlotManagerWidget::AddOperatorType(const char *operatorName,
    const char **iconData)
{
    QString menuName(operatorName);
    menuName += QString(" . . .");
    int id = operatorMenu->count() - 3;

    if(iconData)
    {
        // Add the operator type to the operator menu.
        QPixmap  iconPixmap(iconData);
        QIconSet icon(iconPixmap);

        // Create the sub-menus.
        operatorMenu->insertItem(icon, operatorName, id, id);

        // Add the operator type to the operator attributes list.
        operatorAttsMenu->insertItem(icon, menuName, operatorAttsMenu->count());
    }
    else
    {
        // Create the sub-menus.
        operatorMenu->insertItem(operatorName, id, id);
        // Add the operator type to the operator attributes list.
        operatorAttsMenu->insertItem(menuName, operatorAttsMenu->count());
    }

    //
    // HACK!! Want to disable lineout from user selection.
    //
    if (strcmp(operatorName,  "Lineout") == 0)
    {
        operatorMenu->setItemEnabled(id, false);
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
// ****************************************************************************

void
QvisPlotManagerWidget::EnablePluginMenus()
{
    pluginsLoaded = true;
    plotMenuBar->setItemEnabled(plotAttsMenuId, plotAttsMenu->count() > 0);
    plotMenuBar->setItemEnabled(operatorAttsMenuId, operatorAttsMenu->count() > 0);
    plotMenuBar->update();
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
// ****************************************************************************

bool
QvisPlotManagerWidget::PopulateVariableLists(VariableMenuPopulator &populator,
    const QualifiedFilename &filename)
{
    // Get a pointer to the specified file's metadata object.
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename);

    // get MetaData directly from server if its not invariant
    if (md && md->GetMustRepopulateOnStateChange())
    {
        md = fileServer->GetMetaDataFromMDServer(
                             fileServer->GetOpenFile(),
                             fileServer->GetOpenFileTimeState());
    }

    const avtSIL *sil = fileServer->GetSIL(filename);
    
    return populator.PopulateVariableLists(filename.FullName(),
                                           md, sil, exprList);
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
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotVariableMenu()
{
    //
    // Update the menu populator so it uses the current file. If it changed
    // then needsUpdate will be true and we need to update the variable menu.
    //
    bool needsUpdate = PopulateVariableLists(menuPopulator,
        fileServer->GetOpenFile());

    // Update the various menus
    if(needsUpdate)
    {
        this->maxVarCount = 0;
        for(int i = 0; i < plotPlugins.size(); ++i)
        {
            int varCount = menuPopulator.UpdateSingleVariableMenu(
                plotPlugins[i].varMenu, this, plotPlugins[i].varTypes, false);
            this->maxVarCount = (varCount > this->maxVarCount) ? varCount : this->maxVarCount;
            bool hasEntries = (varCount > 0);
            // If the menu has a different enabled state, set it now.
            if(hasEntries != plotMenu->isItemEnabled(i))
                plotMenu->setItemEnabled(i, hasEntries);
        }

        //
        // Set the flag to indicate that we need to update the enabled
        // state for the plot variable menu.
        //
        this->updatePlotVariableMenuEnabledState = true;
    }
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
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotAndOperatorMenuEnabledState()
{
    //
    // These values will be used to set the enabled state for the items in
    // the plot and operator menu.
    //
    bool plotMenuEnabled = plotMenuBar->isItemEnabled(plotMenuId);
    bool plotAttsMenuEnabled = plotMenuBar->isItemEnabled(plotAttsMenuId);
    bool operatorMenuEnabled = plotMenuBar->isItemEnabled(operatorMenuId);
    bool operatorAttsMenuEnabled = plotMenuBar->isItemEnabled(operatorAttsMenuId);
    bool varMenuEnabled = plotMenuBar->isItemEnabled(varMenuId);

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
        for(i = 0; i < operatorMenu->count(); ++i)
            someOperatorMenusEnabled |= operatorMenu->isItemEnabled(i);

        bool haveAvailablePlots = plotAttsMenu->count() > 0;
        bool haveAvailableOperators = operatorAttsMenu->count() > 0;
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
    // If the number of variables is less than the cutoff then we want to
    // disable the menus when the viewer is executing. If the number of
    // variables is too large then we don't want to change the menu enabled
    // states because it is too expensive.
    //
    if(this->maxVarCount < VARIABLE_CUTOFF)
    {
        plotMenuEnabled &= !globalAtts->GetExecuting();
        operatorMenuEnabled &= !globalAtts->GetExecuting();
        varMenuEnabled &= !globalAtts->GetExecuting();
        plotAttsMenuEnabled = !globalAtts->GetExecuting();
        operatorAttsMenuEnabled = !globalAtts->GetExecuting();
    }

    //
    // Check each menu that we want to update to see if we need to update.
    //
    bool needUpdate = false;
    bool different = false;
    if(this->updatePlotVariableMenuEnabledState)
    {
        different = plotMenuBar->isItemEnabled(plotMenuId) != plotMenuEnabled;
        if(different)
            plotMenuBar->setItemEnabled(plotMenuId, plotMenuEnabled);
        needUpdate |= different;
    }

    if(this->updateOperatorMenuEnabledState)
    {
        different = plotMenuBar->isItemEnabled(operatorMenuId) != operatorMenuEnabled;
        if(different)
            plotMenuBar->setItemEnabled(operatorMenuId, operatorMenuEnabled);
        needUpdate |= different;
    }

    if(this->updateVariableMenuEnabledState)
    {
        different = plotMenuBar->isItemEnabled(varMenuId) != varMenuEnabled;
        if(different)
            plotMenuBar->setItemEnabled(varMenuId, varMenuEnabled);
        needUpdate |= different;
    }

    different = plotMenuBar->isItemEnabled(operatorAttsMenuId) != operatorAttsMenuEnabled;
    if(different)
    {
        plotMenuBar->setItemEnabled(operatorAttsMenuId, operatorAttsMenuEnabled);
        needUpdate |= different;
    }

    different = plotMenuBar->isItemEnabled(plotAttsMenuId) != plotAttsMenuEnabled;
    if(different)
    {
        plotMenuBar->setItemEnabled(plotAttsMenuId, plotAttsMenuEnabled);
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
            bool flagsDiffer = (plotVarFlags != varMenuFlags);
            if(changeVarLists || flagsDiffer || varMenu->count() == 0)
            {
                // Set the variable list based on the first active plot.
                int varCount = varMenuPopulator.UpdateSingleVariableMenu(varMenu,
                    this, plotVarFlags, true);
                varMenuFlags = plotVarFlags;

                //
                // Set the flag that indicates that we need to update the
                // enabled state for the variables menu.
                //
                this->updateVariableMenuEnabledState |= (varCount > 0);
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
    else if(TheRemovedSubject == pluginAtts)
        pluginAtts = 0;
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
QvisPlotManagerWidget::ConnectPluginManagerAttributes(PluginManagerAttributes *pa)
{
    pluginAtts = pa;
    pluginAtts->Attach(this);
}

void
QvisPlotManagerWidget::ConnectWindowInformation(WindowInformation *wi)
{
    windowInfo = wi;
    windowInfo->Attach(this);
}

void
QvisPlotManagerWidget::ConnectDatabaseMetaData(avtDatabaseMetaData *md)
{
    metaData = md;
    metaData->Attach(this);
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
// ****************************************************************************

void
QvisPlotManagerWidget::keyReleaseEvent(QKeyEvent *key)
{
    if(key->key() == Qt::Key_Delete)
        deletePlots();
    else if(key->key() == Qt::Key_Enter || key->key() == Qt::Key_Return)
    {
        // Activate the windows for all of the selected plots.
        int top = QMAX(plotListBox->count(), plotList->GetNumPlots());
        for(int i = 0; i < top; ++i)
        {
            if(plotListBox->isSelected(i))
            {
                int plotType = plotList->GetPlot(i).GetPlotType();
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
    viewer->HideActivePlots();
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
    viewer->DeleteActivePlots();
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
    viewer->DrawPlots();
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
        if(plotListBox->isSelected(i))
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
            viewer->SetActivePlots(newPlotSelection, newOperatorSelection,
                                   newExpandedPlots);
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
    viewer->PromoteOperator(operatorIndex);
    if(AutoUpdate())
        viewer->DrawPlots();
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
    viewer->DemoteOperator(operatorIndex);
    if(AutoUpdate())
        viewer->DrawPlots();
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
    viewer->RemoveOperator(operatorIndex);
    if(AutoUpdate())
        viewer->DrawPlots();
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
    viewer->ChangeActivePlotsVar(varName.latin1());
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::addPlot
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
//   Brad Whitlock, Mon Mar 4 14:16:12 PST 2002
//   Added auto update support.
//
//   Brad Whitlock, Mon Mar 4 16:48:51 PST 2002
//   Added code to set the cursor to the wait cursor.
//
//   Brad Whitlock, Wed Mar 13 14:37:39 PST 2002
//   Upgraded to Qt 3.0
//
//   Brad Whitlock, Thu May 2 15:21:52 PST 2002
//   Changed how the cursor gets set.
//
//   Kathleen Bonnell, Wed Sep  4 16:14:12 PDT 2002  
//   Removed call to ParseVariable. 
//
// ****************************************************************************

void
QvisPlotManagerWidget::addPlot(int plotType, const QString &varName)
{
    // Set the cursor.
    SetWaitCursor();

    // Tell the viewer to add a plot.
    viewer->AddPlot(plotType, varName.latin1());

    // If we're in auto update mode, tell the viewer to draw the plot.
    if(AutoUpdate())
        viewer->DrawPlots();
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
// ****************************************************************************

void
QvisPlotManagerWidget::operatorAction(int index)
{
    if(index == REMOVE_LAST_OPERATOR_ID)
        viewer->RemoveLastOperator();
    else if(index == REMOVE_ALL_OPERATORS_ID)
        viewer->RemoveAllOperators();
    else
        viewer->AddOperator(index);
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
        viewer->ActivateDatabase(sources[index]);
}
