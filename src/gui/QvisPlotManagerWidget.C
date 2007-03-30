#include <QvisPlotManagerWidget.h>
#include <qapplication.h>
#include <qcheckbox.h>
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
#include <PluginManagerAttributes.h>
#include <QvisPlotListBoxItem.h>
#include <QvisPlotListBox.h>
#include <QvisVariablePopupMenu.h>
#include <PlotPluginInfo.h>

#include <stdio.h>
#include <stdlib.h>

#include <icons/removelastoperator.xpm>
#include <icons/removealloperators.xpm>

#include <DebugStream.h>
//#define DEBUG_PRINT

#define REMOVE_LAST_OPERATOR_ID   1000
#define REMOVE_ALL_OPERATORS_ID   1001

using std::string;
using std::vector;

// ****************************************************************************
// Method: QvisPlotManagerWidget::QvisPlotManagerWidget
//
// Purpose: 
//   This is the constructor for the QvisPlotManagerWidget class.
//
// Arguments:
//   plotList : This is the PlotList object that the widget observes.
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
// ****************************************************************************

QvisPlotManagerWidget::QvisPlotManagerWidget(QWidget *parent, const char *name)
    : QWidget(parent, name), GUIBase(), SimpleObserver(), menuPopulator(),
    plotPlugins()
{
    pluginsLoaded = false;

    topLayout = new QGridLayout(this, 4, 4);
    topLayout->setSpacing(5);

    activePlots = new QLabel("Active plots", this, "activePlots");
    topLayout->addWidget(activePlots, 0, 0);

    // Create the hide/show button.
    hideButton = new QPushButton("Hide/Show", this, "hideButton");
    hideButton->setEnabled(false);
    connect(hideButton, SIGNAL(clicked()), this, SLOT(hidePlots()));
    topLayout->addWidget(hideButton, 0, 1);

    // Create the delete button.
    deleteButton = new QPushButton("Delete", this, "deleteButton");
    deleteButton->setEnabled(false);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deletePlots()));
    topLayout->addWidget(deleteButton, 0, 2);

    // Create the draw button.
    drawButton = new QPushButton("Draw", this, "drawButton");
    drawButton->setEnabled(false);
    connect(drawButton, SIGNAL(clicked()), this, SLOT(drawPlots()));
    topLayout->addWidget(drawButton, 0, 3);

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


    topLayout->addMultiCellWidget(plotListBox, 1, 1, 0, 3);

    // Create the "Apply operator to all plots" toggle.
    applyOperatorToggle = new QCheckBox("Apply operator to all plots", this,
        "applyOperatorToggle");
    connect(applyOperatorToggle, SIGNAL(toggled(bool)),
            this, SLOT(applyOperatorToggled(bool)));
    topLayout->addMultiCellWidget(applyOperatorToggle, 2, 2, 0, 3);

    // Create the plot and operator menus. Note that they will be empty until
    // they are populated by the main application.
    CreateMenus();
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
// ****************************************************************************

QvisPlotManagerWidget::~QvisPlotManagerWidget()
{
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
// ****************************************************************************

void
QvisPlotManagerWidget::CreateMenus()
{
    //
    // Create the Plots Menu
    //
    plotMenuBar = new QMenuBar(this, "plotMenu");
    topLayout->addMultiCellWidget(plotMenuBar, 3, 3, 0, 3);

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
    plotAttsMenuId = plotMenuBar->insertItem( tr("PlotAtts"), plotAttsMenu);
    plotMenuBar->setItemEnabled(plotAttsMenuId, false);

    //
    // Create the Operator attributes menu.
    //
    operatorAttsMenu = new QPopupMenu( plotMenuBar );
    connect(operatorAttsMenu, SIGNAL(activated(int)),
            this, SIGNAL(activateOperatorWindow(int)));
    operatorAttsMenuId = plotMenuBar->insertItem( tr("OpAtts"), operatorAttsMenu );
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
// ****************************************************************************

void
QvisPlotManagerWidget::Update(Subject *TheChangedSubject)
{
    if(plotList == 0 || fileServer == 0 || globalAtts == 0 || pluginAtts == 0)
        return;

    // Get whether or not we are allowed to modify things.
    bool canChange = !globalAtts->GetExecuting();

    // Enable the plot and operator attributes menus.
    bool havePlots = plotAttsMenu->count() > 0;
    plotMenuBar->setItemEnabled(plotAttsMenuId, pluginsLoaded && canChange &&
                                havePlots);
    bool haveOperators = operatorAttsMenu->count() > 0;
    plotMenuBar->setItemEnabled(operatorAttsMenuId, pluginsLoaded &&
                                canChange && haveOperators);
    plotMenuBar->update();

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

                UpdatePlotVariableMenu();
                UpdateVariableMenu();
            }
        }
        else if(fileServer->FileChanged())
        {
            UpdatePlotVariableMenu();
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
        // Set the "Apply operator toggle."
        applyOperatorToggle->blockSignals(true);
        applyOperatorToggle->setChecked(globalAtts->GetApplyOperator());
        applyOperatorToggle->blockSignals(false);

        // Set the enabled state for the whole widget based on whether
        // or not the engine is busy.
        activePlots->setEnabled(canChange);
        plotListBox->setEnabled(canChange);
        hideButton->setEnabled(canChange);
        deleteButton->setEnabled(canChange);
        drawButton->setEnabled(canChange);
        applyOperatorToggle->setEnabled(canChange);

        bool havePlots = (plotAttsMenu->count() > 0);
        bool haveOperators = (operatorAttsMenu->count() > 0);
        plotMenuBar->setItemEnabled(plotMenuId, canChange && havePlots);
        plotMenuBar->setItemEnabled(plotAttsMenuId, canChange && havePlots);
        plotMenuBar->setItemEnabled(operatorMenuId, canChange && haveOperators);
        plotMenuBar->setItemEnabled(operatorAttsMenuId, canChange && haveOperators);
        plotMenuBar->setItemEnabled(varMenuId, (varMenu->count() > 0) && canChange);
        plotMenuBar->update();
    }
    else if(TheChangedSubject == pluginAtts)
    {
        // do nothing yet -- JSM 9/5/01
    }
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
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotList()
{
    int nHideablePlots = 0;

    if(plotListBox->NeedsUpdated(plotList))
    {
        // Update the plot list.
        plotListBox->clear();
        blockSignals(true);
        plotListBox->blockSignals(true);
        for(int i = 0; i < plotList->GetNumPlots(); ++i)
        {
            // Create a constant reference to the current plot.
            const Plot &current = plotList->operator[](i);

            // Figure out the prefix that should be applied to the plot.
            QString prefix;
            QualifiedFilename qualifiedFile(current.GetDatabaseName());
            int index = fileServer->GetFileIndex(qualifiedFile);
            if(index < 0)
                prefix.sprintf("%s:", qualifiedFile.filename.c_str());
            else
                prefix.sprintf("%d:", index);               

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

            // Figure out how many plots are selected and complete. These are
            // the plots that can be hidden.
            bool hideableState = (current.GetStateType() == Plot::Completed) ||
                                 (current.GetStateType() == Plot::Error);
            bool canHide = hideableState || current.GetHiddenFlag();
            if(current.GetActiveFlag() && canHide)
                ++nHideablePlots;
        } // end for
    }

    // Set the enabled states for the hide, delete buttons.
    bool canChange = !globalAtts->GetExecuting();
    hideButton->setEnabled(nHideablePlots > 0 && canChange);
    deleteButton->setEnabled(plotList->GetNumPlots() > 0 && canChange);
    drawButton->setEnabled(plotList->GetNumPlots() > 0 && canChange);

    plotListBox->blockSignals(false);
    blockSignals(false);
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
//   filename : The file for which to get metadata.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 14:28:43 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 17 15:44:44 PST 2003
//   I made it use a menu populator.
//
// ****************************************************************************

void
QvisPlotManagerWidget::PopulateVariableLists(const QualifiedFilename &filename)
{
    // Get a pointer to the specified file's metadata object.
    const avtDatabaseMetaData *md = fileServer->GetMetaData(filename);
    const avtSIL *sil = fileServer->GetSIL(filename);

    menuPopulator.PopulateVariableLists(md, sil, exprList);
}

// ****************************************************************************
// Method: QvisPlotManagerWidget::UpdateVariableMenus
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
// ****************************************************************************

void
QvisPlotManagerWidget::UpdatePlotVariableMenu()
{
    // Update the metadata to use the current file.
    PopulateVariableLists(fileServer->GetOpenFile());

    // Update the various menus
    bool someMenusEnabled = false;
    for(int i = 0; i < plotPlugins.size(); ++i)
    {
        int varCount = menuPopulator.UpdateSingleVariableMenu(
            plotPlugins[i].varMenu, this, plotPlugins[i].varTypes, false);
        bool hasEntries = (varCount > 0);

        plotMenu->setItemEnabled(i, hasEntries);
        // HACK!! WANT TO TEMPORARILY DISABLE CURVES
        if (plotMenu->text(i) == "Curve")
            plotMenu->setItemEnabled(i, false);

        someMenusEnabled |= hasEntries;
    }

    // Set the enabled state of the Plot and Operator menus based on how
    // many variables are in the variable lists.
    bool enableMenu = someMenusEnabled && pluginsLoaded;
    plotMenuBar->setItemEnabled(plotMenuId, enableMenu &&
        (plotAttsMenu->count() > 0));
    plotMenuBar->setItemEnabled(operatorMenuId, enableMenu &&
        (operatorAttsMenu->count() > 0));
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
// ****************************************************************************

void
QvisPlotManagerWidget::UpdateVariableMenu()
{
    // Update the variable lists using the type of the first plot as the
    // kind of variable that is displayed.
    varMenu->clear();
    int varCount = 0;
    bool first = true;
    for(int i = 0; i < plotList->GetNumPlots(); ++i)
    {
        // Create a constant reference to the current plot.
        const Plot &current = plotList->operator[](i);

        if(current.GetActiveFlag())
        {
            // If this is the first active plot, put its variables
            // in the variable lists.
            if(first)
            {
                PopulateVariableLists(current.GetDatabaseName());
                first = false;
                varMenu->setPlotType(current.GetPlotType());
            }

            // Set the variable list based on the first active plot.
            varCount = menuPopulator.UpdateSingleVariableMenu(varMenu,
                this, plotPlugins[current.GetPlotType()].varTypes, true);

            // Get out of the for loop.
            break;
        }
    }
    
    // Set the enabled state of the varMenu based on how many variables
    // are in it.
    bool canChange = !globalAtts->GetExecuting();
    plotMenuBar->setItemEnabled(varMenuId, (varCount > 0) && canChange);
    plotMenuBar->update();
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

        // If there was at least 1 selected plot, select that plot's file in
        // file fileServer if it is not already selected.
        if(found)
        {
            // Create a constant reference to the first selected plot.
            const Plot &selFile = plotList->operator[](firstSelectedFile);

            QualifiedFilename qualifiedFile(selFile.GetDatabaseName());

            // Set the fileServer's active file and notify observers.
            // This is mostly to sync them up to the fileServer in case
            // They got out of sync.
            int timeState = globalAtts->GetCurrentState();
            fileServer->OpenFile(qualifiedFile, timeState);
            fileServer->Notify();

            // Tell the viewer to also open the file for the plot. If we don't
            // do this then when we try and create a new plot using what the
            // GUI considers to be the open file, the viewer will use the
            // wrong database.
            viewer->OpenDatabase(qualifiedFile.FullName().c_str(), timeState);
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
