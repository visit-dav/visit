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

#include <PlotAndOperatorActionsUI.h>
#include <PlotAndOperatorActions.h>

#include <ParsingExprList.h>
#include <ViewerFileServerInterface.h>
#include <ViewerPlot.h>
#include <ViewerPlotList.h>
#include <ViewerProperties.h>
#include <ViewerQueryManager.h>
#include <ViewerState.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>

#include <ViewerActionLogic.h>

#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <GlobalAttributes.h>

#include <snprintf.h>

#include <QApplication>
#include <QAction>
#include <QIcon>
#include <QMainWindow>
#include <QMenuBar>
#include <QPixmapCache>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>

#include <QvisVariablePopupMenu.h>

#include <DebugStream.h>
#include <InvalidExpressionException.h>

#define VMAX(A,B) (((A) > (B)) ? (A) : (B))

//
// Include icons
//
#include <removelastoperator.xpm>
#include <removealloperators.xpm>

// ****************************************************************************
// Method: AddOperatorActionUI::AddOperatorActionUI
//
// Purpose: 
//   Constructor for the AddOperator action.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:12:08 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jul 27 18:11:58 PST 2004
//   Added code to prevent pixmaps from being created when in -nowin mode.
//
//   Brad Whitlock, Tue Apr 29 11:22:40 PDT 2008
//   Use the menu name from the plugin info since it's translatable. Use
//   GetUserSelectable from the plugin interface instead of checking for
//   a plugin name.
//
//   Brad Whitlock, Fri Sep 12 12:20:02 PDT 2014
//   Do the menu name translation here instead of in the plugin.
//
// ****************************************************************************

AddOperatorActionUI::AddOperatorActionUI(ViewerActionLogic *L) :
    ViewerActionUIMultiple(L), graphicalPlugins()
{
    SetAllText(tr("Add operator"));
    SetExclusive(false);

    //
    // Iterate through all of the loaded operator plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerOperatorPluginInfo *info = 0;
    OperatorPluginManager *pluginMgr = GetOperatorPluginManager();
    int nTypes = pluginMgr->GetNEnabledPlugins();
    for (int i = 0; i < nTypes; ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!GetViewerProperties()->GetNowin() && 
               info->XPMIconData() != 0 &&
               info->GetUserSelectable())
            {
                QString menuName(qApp->translate("OperatorNames",
                                                 info->GetMenuName()));

                if(!GetViewerProperties()->GetNowin())
                {
                    // Create a pixmap for the operator or get its pixmap from
                    // the pixmap cache.
                    QString key;
                    key.sprintf("operator_icon_%s", info->GetName());
                    QPixmap pix;
                    if(!QPixmapCache::find(key, pix))
                    {
                        pix = QPixmap(info->XPMIconData());
                        QPixmapCache::insert(key, pix);
                    }

                    // Add a choice for operator so that it has an icon.
                    QString tip(tr("Add %1 operator").arg(menuName));
                    AddChoice(menuName, tip, pix);
                }
                else
                    AddChoice(menuName);

                // Record that this plugin has an icon.
                graphicalPlugins.push_back(i);
            }
        }
    }
}

// ****************************************************************************
// Method: AddOperatorActionUI::~AddOperatorActionUI
//
// Purpose: 
//   Destructor for the AddOperatorActionUI class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:20:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AddOperatorActionUI::~AddOperatorActionUI()
{
}

// ****************************************************************************
// Method: AddOperatorActionUI::Enabled()
//
// Purpose: 
//   This method indicates when the action is enabled.
//
// Returns:    A bool indicating when the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:22:22 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 14:10:07 PST 2003
//   I disabled the action in curve windows.
//
//   Eric Brugger, Wed Aug 20 10:53:00 PDT 2003
//   I removed the disabling of the action in curve windows.
//
// ****************************************************************************

bool
AddOperatorActionUI::Enabled() const
{
    return ViewerActionUIMultiple::Enabled() &&
           (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

// ****************************************************************************
// Method: AddOperatorActionUI::ChoiceEnabled
//
// Purpose: 
//   This method indicates when individual choices in the action are enabled.
//
// Returns:    true
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:22:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
AddOperatorActionUI::ChoiceEnabled(int) const
{
    return true;
}

// ****************************************************************************
// Method: AddOperatorActionUI::ConstructToolbar
//
// Purpose: 
//   Adds the action's operators to the toolbar.
//
// Arguments:
//   toolbar : The toolbar to which the operators will be added.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:48:44 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu May 22 13:49:04 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
AddOperatorActionUI::ConstructToolbar(QToolBar *toolbar)
{
    ViewerActionUIMultiple::ConstructToolbar(toolbar);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveLastOperatorActionUI::RemoveLastOperatorActionUI
//
// Purpose: 
//   Constructor for the RemoveLastOperatorActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveLastOperatorActionUI::RemoveLastOperatorActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Remove last operator"));
    if(!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(removelastoperator_xpm)));
}

// ****************************************************************************
// Method: RemoveLastOperatorActionUI::Enabled
//
// Purpose: 
//   Returns when this action is enabled.
//
// Returns:    Returns true when the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   Eric Brugger, Wed Aug 20 10:53:00 PDT 2003
//   I removed the disabling of the action in curve windows.
//   
// ****************************************************************************

bool
RemoveLastOperatorActionUI::Enabled() const
{
    return GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveAllOperatorsActionUI::RemoveAllOperatorsActionUI
//
// Purpose: 
//   Constructor for the RemoveAllOperatorsActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveAllOperatorsActionUI::RemoveAllOperatorsActionUI(ViewerActionLogic *L) : 
    ViewerActionUISingle(L)
{
    SetAllText(tr("Remove all operators"));
    if(!GetViewerProperties()->GetNowin())
        SetIcon(QIcon(QPixmap(removealloperators_xpm)));
}

// ****************************************************************************
// Method: RemoveAllOperatorsActionUI::Enabled
//
// Purpose: 
//   Returns when this action is enabled.
//
// Returns:    True if the action is enabled; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 14:21:24 PST 2003
//   I made the action inactive in curve windows.
//
//   Eric Brugger, Wed Aug 20 10:53:00 PDT 2003
//   I removed the disabling of the action in curve windows.
//   
// ****************************************************************************

bool
RemoveAllOperatorsActionUI::Enabled() const
{
    return GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddPlotActionUI::AddPlotActionUI
//
// Purpose: 
//   Constructor for the AddPlotActionUI class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Sep 29 17:39:17 PST 2003
//   Initialized host.
//
//   Brad Whitlock, Tue Jul 27 18:09:16 PST 2004
//   Changed so pixmaps are not created when we're in -nowin mode.
//
//   Brad Whitlock, Tue Apr 29 11:41:42 PDT 2008
//   Added tr(), remove code to disable Curve plot. Made the code use the
//   plot's menu name.
//
//   Brad Whitlock, Thu May 22 13:53:02 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Fri Sep 12 12:20:02 PDT 2014
//   Do the menu name translation here instead of in the plugin.
//
// ****************************************************************************

AddPlotActionUI::AddPlotActionUI(ViewerActionLogic *L) : ViewerActionUIMultiple(L),
    pluginEntries(), menuPopulator()
{
    SetAllText(tr("Add plot"));
    SetExclusive(false);

    maxPixmapWidth = maxPixmapHeight = 0;

    //
    // Iterate through all of the loaded plot plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerPlotPluginInfo *info = 0;
    PlotPluginManager *pluginMgr = GetPlotPluginManager();
    for(int i = 0; i < pluginMgr->GetNEnabledPlugins(); ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!GetViewerProperties()->GetNowin() && info->XPMIconData() != 0)
            {
                QString menuName(qApp->translate("PlotNames",
                                                 info->GetMenuName()));

                // Create a pixmap for the plot or get its pixmap from
                // the pixmap cache.
                if(!GetViewerProperties()->GetNowin())
                {
                    QString key;
                    key.sprintf("plot_icon_%s", info->GetName());
                    QPixmap pix;
                    if(!QPixmapCache::find(key, pix))
                    {
                        pix = QPixmap(info->XPMIconData());
                        QPixmapCache::insert(key, pix);
                    }

                    // Find the maximum pixmap width and height
                    maxPixmapWidth = VMAX(maxPixmapWidth, pix.width());
                    maxPixmapHeight = VMAX(maxPixmapHeight, pix.height());

                    // Add a choice for plot so that it has an icon.
                    QString tip(tr("Add %1 plot").arg(menuName));
                    AddChoice(menuName, tip, pix);
                }
                else
                    AddChoice(menuName);

                // Record the plugin entry.
                PluginEntry p;
                p.index = i;
                p.varMenu = 0;
                p.varTypes = info->GetVariableTypes();
                pluginEntries.push_back(p);
            }
        }
    }
}

// ****************************************************************************
// Method: AddPlotActionUI::~AddPlotActionUI
//
// Purpose: 
//   Destructor for the AddPlotActionUI class.
//
// Note:       We manually delete the menus because they have no parent widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:42:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AddPlotActionUI::~AddPlotActionUI()
{
    for(size_t i = 0; i < pluginEntries.size(); ++i)
        delete pluginEntries[i].varMenu;
}

// ****************************************************************************
// Method: AddPlotActionUI::Update
//
// Purpose: 
//   This method is called when the action needs to be updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:43:45 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Sep 29 17:03:33 PST 2003
//   I changed the code so it no longer will attempt to get the metadata and
//   the SIL for an invalid database name.
//
//   Brad Whitlock, Mon Sep 29 17:39:31 PST 2003
//   I separated the database into host and database and queried it from the
//   plot list in such a way that it is more likely to be valid.
//
//   Brad Whitlock, Tue Feb 24 16:37:30 PST 2004
//   I changed the code so the menu is only updated then the variable menu
//   populator indicates that it needed an update due to a different
//   database name or a different expression list.
//
//   Brad Whitlock, Wed Dec 8 15:28:59 PST 2004
//   I updated the code to use a new interface to
//   VariableMenuPopulator::UpdateSingleVariableMenu.
//
//   Brad Whitlock, Fri Apr 15 13:39:32 PST 2005
//   Added code to clear the menu since the menu populator no longer does
//   menu clearing. I also made the variable menu populator responsible for
//   deciding when the menu must be cleared since it does a better overall
//   job.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added bool to treat all databases as time varying to call to
//   PopulateVariableLists
//
//   Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//   Changed how treatAllDBsAsTimeVarying to be obtained from
//   ViewerWindowManager which manages the GlobalAttributes object
//
//   Brad Whitlock, Thu May 29 15:50:33 PDT 2008
//   Qt 4.
//
//   Rob Sisneros, Sun Aug 29 20:13:10 CDT 2010
//   Put expressions from operators into the pipeline.
//
//   Brad Whitlock, Fri Nov 19 15:05:34 PST 2010
//   I changed the code so it deletes and recreates the menu since clearing
//   it did not free memory.
//
// ****************************************************************************

void
AddPlotActionUI::Update()
{
    if(pluginEntries.size() > 0)
    {
        ViewerPlotList *plotList = GetLogic()->GetWindow()->GetPlotList();
        const std::string &host = plotList->GetHostName();
        const std::string &database = plotList->GetDatabaseName();

        if(host.size() > 0 && database.size() > 0)
        {
            // Get the metadata and SIL for the file.
            const avtDatabaseMetaData *md = GetViewerFileServer()->GetMetaData(host, database);
            const avtSIL *sil = GetViewerFileServer()->GetSIL(host, database);
            const ExpressionList *exprList = ParsingExprList::Instance()->GetList();

            //
            // Repopulate the menu variable list using information from the
            // new file.
            //

            OperatorPluginManager *oPM = GetOperatorPluginManager();
            bool treatAllDBsAsTimeVarying =
                GetViewerState()->GetGlobalAttributes()->GetTreatAllDBsAsTimeVarying();
            if(menuPopulator.PopulateVariableLists(plotList->GetHostDatabaseName(),
                                                   md, sil, exprList,
                                                   oPM,
                                                   treatAllDBsAsTimeVarying))
            {
                // Print to the debug logs.
                debug4 << "AddPlotActionUI::Update: Either the host or the database " << endl
                       << "changed so we need to update the variable menu!" << endl
                       << "\thost=" << host.c_str() << endl
                       << "\tdb=" << database.c_str() << endl;

                //
                // Update the variable menus for the actions.
                //
                bool menuEnabled = false;
                for(int i = 0; i < (int)pluginEntries.size(); ++i)
                {
                    DeletePlotMenu(i);
                    CreatePlotMenu(i);

                    int varCount = menuPopulator.UpdateSingleVariableMenu(
                        pluginEntries[i].varMenu,
                        pluginEntries[i].varTypes, this, 
                        SLOT(addPlot(int, const QString &)));
                    bool hasEntries = (varCount > 0);

                    // Set the new menu's enabled state based on the variable type.
                    if(hasEntries != pluginEntries[i].varMenu->isEnabled())
                        pluginEntries[i].varMenu->setEnabled(hasEntries);
                    menuEnabled |= hasEntries;
                }
            }
        }
    }

    ViewerActionUIMultiple::Update();
}

// ****************************************************************************
// Method: AddPlotActionUI::Enabled
//
// Purpose: 
//   This method lets callers know if the action's menu should be enabled.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:45:14 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 4 14:11:58 PST 2003
//   I prevented the action from being enabled in curve windows.
//
//   Eric Brugger, Wed Aug 20 10:53:00 PDT 2003
//   I removed the disabling of the action in curve windows.
//   
// ****************************************************************************

bool
AddPlotActionUI::Enabled() const
{
    bool dbIsOpen = (GetLogic()->GetWindow()->GetPlotList()->GetHostDatabaseName().length() > 0);
    return ViewerActionUIMultiple::Enabled() && dbIsOpen;
}

// ****************************************************************************
// Method: AddPlotActionUI::ChoiceEnabled
//
// Purpose: 
//   This method lets callers know a choice in the action's menu should
//   be enabled.
//
// Arguments:
//   i : The index of the menu item that we're checking.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:46:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

bool
AddPlotActionUI::ChoiceEnabled(int i) const
{
    return menuPopulator.ItemEnabled(pluginEntries[i].varTypes);
}

// ****************************************************************************
// Method: AddPlotActionUI::CreatePlotMenu
//
// Purpose: 
//   This method creates the i'th plot menu.
//
// Arguments:
//   i : The index of the plot menu we're creating.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 15:09:34 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddPlotActionUI::CreatePlotMenu(int i)
{
    // Create the menu for the plot.
    pluginEntries[i].varMenu = new QvisVariablePopupMenu(i, 0);
    pluginEntries[i].varMenu->setIcon(children[i]->icon());
    pluginEntries[i].varMenu->setTitle(children[i]->text());
    connect(pluginEntries[i].varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(addPlot(int, const QString &)));

    // Set the variable menu into the action
    children[i]->setMenu(pluginEntries[i].varMenu);
}

// ****************************************************************************
// Method: AddPlotActionUI::DeletePlotMenu
//
// Purpose: 
//   This method deletes the i'th plot menu.
//
// Arguments:
//   i : The index of the plot menu we're deleting.
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 19 15:09:34 PST 2010
//
// Modifications:
//   
// ****************************************************************************

void
AddPlotActionUI::DeletePlotMenu(int i)
{
    if(pluginEntries[i].varMenu != 0)
    {
        delete pluginEntries[i].varMenu;
        pluginEntries[i].varMenu = 0;
    }
}

// ****************************************************************************
// Method: AddPlotActionUI::ConstructMenu
//
// Purpose: 
//   This method constructs a menu that contains the plots that are available.
//
// Arguments:
//   menu : The parent menu.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:47:03 PDT 2003
//
// Modifications:
//   Brad Whitlock, Wed May 28 16:50:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
AddPlotActionUI::ConstructMenu(QMenu *menu)
{
    // Create a new menu and add all of the actions to it.
    actionMenu = new QMenu("Add plot", menu);

    for(int i = 0; i < (int)pluginEntries.size(); ++i)
    {
        // Create the menu for the plot.
        CreatePlotMenu(i);

        // Add the action into the actionMenu
        actionMenu->addAction(children[i]);
    }

    // Insert the new menu into the old menu.
    if(iconSpecified)
        actionMenu->setIcon(icon);
    actionMenu->setTitle(menuText);
    menu->addMenu(actionMenu);
}

// ****************************************************************************
// Method: AddPlotActionUI::RemoveFromMenu
//
// Purpose: 
//   Removes the action from the menu.
//
// Arguments:
//   menu : The menu from which the action should be removed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:47:43 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddPlotActionUI::RemoveFromMenu(QMenu *menu)
{
    // NOT IMPLEMENTED. Remove the action from the menu.
}

// ****************************************************************************
// Method: AddPlotActionUI::ConstructToolbar
//
// Purpose: 
//   Adds the action's plots to the toolbar.
//
// Arguments:
//   toolbar : The toolbar to which the plots will be added.
//
// Note:       We add the plots as a QMenuBar so we can have nice popup menu
//             selection. This presented some problems in menu orientation,
//             and toolbars but those problems are pretty much solved.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:48:44 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Mar 16 15:34:28 PST 2004
//   I added code to create the toolbar using the right icon size. The new
//   code also connects a signal from the vis window that tells this object
//   to update its icons when the icon size in the vis window changes.
//
//   Brad Whitlock, Tue Apr 29 11:45:08 PDT 2008
//   Added tr()
//
//   Brad Whitlock, Thu May 29 16:37:07 PDT 2008
//   Qt 4. Ditched menu-based implementation.
//
// ****************************************************************************

void
AddPlotActionUI::ConstructToolbar(QToolBar *toolbar)
{
    for(size_t i = 0; i < children.size(); ++i)
        toolbar->addAction(children[i]);
}

// ****************************************************************************
// Method: AddPlotActionUI::addPlot
//
// Purpose: 
//   This is a Qt slot function, which is private to this class, that is called
//   when the user makes a selection from one of the variable menus.
//
// Arguments:
//   index : The index of the selected plot type in the toolbar/menu.
//   var   : The name of the variable that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:50:41 PDT 2003
//
// Modifications:
//   Brad Whitlock, Fri Apr 15 13:37:32 PST 2005
//   Changed to the no argument Activate call.
//
// ****************************************************************************

void
AddPlotActionUI::addPlot(int index, const QString &var)
{
    // Hide the menu if it is active.
    int plotType = pluginEntries[index].index;
    pluginEntries[index].varMenu->hide();

    // Set the plot type and variable name into the args.
    AddPlotAction *a = dynamic_cast<AddPlotAction *>(GetLogic());
    if(a != NULL)
        a->SetArguments(plotType, var.toStdString());

    // Execute the action.
    Activate();
}

// ****************************************************************************
// Method: AddPlotActionUI::changeMenuIconSize
//
// Purpose: 
//   This method is called when the vis window changes its icon size.
//
// Arguments:
//   large : Whether the vis window is using large icons.
//
// Notes:      This code should also update the tool tips on the menu widget
//             but that can be done later.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 16 15:26:21 PST 2004
//
// Modifications:
//   Brad Whitlock, Mon Aug 2 09:57:20 PDT 2004
//   Prevented any action in -nowin mode.
//
//   Brad Whitlock, Thu May 22 14:16:17 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
AddPlotActionUI::changeMenuIconSize(bool large)
{
#if 0
    if(!GetViewerProperties()->GetNowin())
    {
        if(large)
        {
            for(size_t i = 0; i < pluginEntries.size(); ++i)
            {
                QPixmap pix(children[i]->icon().pixmap(Qt::Large,
                            QIcon::Normal));
                menu->changeItem(i, pix);
            }
        }
        else
        {
            for(size_t i = 0; i < pluginEntries.size(); ++i)
            {
                QPixmap pix(children[i]->icon().pixmap(Qt::Small,
                            QIcon::Normal));
                menu->changeItem(i, pix);
            }
        }
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DrawPlotsActionUI::DrawPlotsActionUI
//
// Purpose: 
//   Constructor for the DrawPlotsActionUI class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:55:52 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DrawPlotsActionUI::DrawPlotsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Draw plots"));
}

// ****************************************************************************
// Method: DrawPlotsActionUI::Enabled
//
// Purpose: 
//   Returns whether the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DrawPlotsActionUI::Enabled() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HideActivePlotsActionUI::HideActivePlotsActionUI
//
// Purpose: 
//   Constructor for the HideActivePlotsActionUI class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:54:54 PST 2003
//
// Modifications:
//   
// ****************************************************************************

HideActivePlotsActionUI::HideActivePlotsActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Hide active plots"));
    // Think of an icon...
}

// ****************************************************************************
// Method: HideActivePlotsActionUI::Enabled
//
// Purpose: 
//   Returns whether the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
HideActivePlotsActionUI::Enabled() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteActivePlotsActionUI::DeleteActivePlotsActionUI
//
// Purpose: 
//   Constructor for the DeleteActivePlotsActionUI class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:50:38 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DeleteActivePlotsActionUI::DeleteActivePlotsActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Delete active plots"));
    // Think of an icon...
}

// ****************************************************************************
// Method: DeleteActivePlotsActionUI::Enabled
//
// Purpose: 
//   Returns whether the action is enabled.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

bool
DeleteActivePlotsActionUI::Enabled() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: CopyPlotActionUI::CopyPlotActionUI
//
// Purpose: 
//   Constructor for the CopyPlotActionUI class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Ellen Tarwater
// Creation:   Fri Sept 28 15:38:54 PST 2007
//
// Modifications:
//   
// ****************************************************************************

CopyPlotActionUI::CopyPlotActionUI(ViewerActionLogic *L) : ViewerActionUISingle(L)
{
    SetAllText(tr("Copy active plots"));
    // Think of an icon...
}

// ****************************************************************************
// Method: CopyPlotActionUI::Enabled
//
// Purpose: 
//   Returns whether the action is enabled.
//
// Programmer: Ellen Tarwater
// Creation:   Fri Sept 28 15:54:27 PST 2007
//
// Modifications:
//   
// ****************************************************************************

bool
CopyPlotActionUI::Enabled() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotFollowsTimeActionUI::SetPlotFollowsTimeActionUI
//
// Purpose: 
//   Constructor for the SetPlotFollowsTimeActionUI class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Ellen Tarwater
// Creation:   Thurs, Dec 6, 2007
//
// Modifications:
//   Brad Whitlock, Tue Mar 29 11:16:58 PDT 2011
//   Change the description.
//
// ****************************************************************************

SetPlotFollowsTimeActionUI::SetPlotFollowsTimeActionUI(ViewerActionLogic *L) :
    ViewerActionUISingle(L)
{
    SetAllText(tr("Plot follows time slider"));
    // Think of an icon...
}

// ****************************************************************************
// Method: SetPlotFollowsTimeActionUI::Enabled
//
// Purpose: 
//   Returns whether the action is enabled.
//
// Programmer: Ellen Tarwater
// Creation:   Thurs, Dec 6, 2007
//
// Modifications:
//   
// ****************************************************************************

bool
SetPlotFollowsTimeActionUI::Enabled() const
{
    return (GetLogic()->GetWindow()->GetPlotList()->GetNumPlots() > 0);
}
