#include <PlotAndOperatorActions.h>
#include <ViewerExpressionList.h>
#include <ViewerFileServer.h>
#include <ViewerQueryManager.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerAnimation.h>
#include <ViewerPlotList.h>

#include <OperatorPluginManager.h>
#include <OperatorPluginInfo.h>
#include <PlotPluginManager.h>
#include <PlotPluginInfo.h>
#include <GlobalAttributes.h>

#include <snprintf.h>

#include <qaction.h>
#include <qiconset.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qobjectlist.h>
#include <qpixmapcache.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <QvisVariablePopupMenu.h>

//
// Include icons
//
#include <removelastoperator.xpm>
#include <removealloperators.xpm>

// ****************************************************************************
// Method: AddOperatorAction::AddOperatorAction
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
//   
// ****************************************************************************

AddOperatorAction::AddOperatorAction(ViewerWindow *win) :
    ViewerMultipleAction(win, "AddOperator"), graphicalPlugins()
{
    SetAllText("Add operator");
    SetExclusive(false);

    //
    // Iterate through all of the loaded operator plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerOperatorPluginInfo *info = 0;
    OperatorPluginManager *pluginMgr = OperatorPluginManager::Instance();
    int nTypes = pluginMgr->GetNEnabledPlugins();
    std::string Lineout("Lineout");
    for (int i = 0; i < nTypes; ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!window->GetNoWinMode() && info->XPMIconData() != 0 &&
               std::string(info->GetName()) != Lineout)
            {
                char tip[200];
                SNPRINTF(tip, 200, "Add %s operator", info->GetName());

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
                AddChoice(info->GetName(), tip, pix);

                // Record that this plugin has an icon.
                graphicalPlugins.push_back(i);
            }
        }
    }
}

// ****************************************************************************
// Method: AddOperatorAction::~AddOperatorAction
//
// Purpose: 
//   Destructor for the AddOperatorAction class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:20:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AddOperatorAction::~AddOperatorAction()
{
}

// ****************************************************************************
// Method: AddOperatorAction::Setup
//
// Purpose: 
//   This method is called when the toolbar button is clicked. It stores the
//   information about the button into the args so that the Execute method
//   can work for both toolbar clicks and commands from the client.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:20:22 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddOperatorAction::Setup()
{
    args.SetOperatorType(graphicalPlugins[activeAction]);
}

// ****************************************************************************
// Method: AddOperatorAction::Execute
//
// Purpose: 
//   Adds an operator to the plots in the window's plot list.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 17 09:21:25 PDT 2003
//
// Modifications:
//    Jeremy Meredith, Tue Jun 17 18:17:55 PDT 2003
//    Fixed a bug where disabling an operator would cause crashes when
//    using unrelated operators; it was using AllID instead of EnabledID.
//   
// ****************************************************************************

void
AddOperatorAction::Execute(int)
{
    //
    // Get the rpc arguments.
    //
    int type = args.GetOperatorType();

    OperatorPluginManager *opMgr = OperatorPluginManager::Instance();
    std::string name(opMgr->GetPluginName(opMgr->GetEnabledID(type)));
    if (name == "Lineout")
    {
        ViewerQueryManager::Instance()->Lineout(window);
    }
    else
    {
        //
        // Add the operator to the window's plot list.
        //
        bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
        window->GetAnimation()->GetPlotList()->AddOperator(type, applyToAll);
    }
}

// ****************************************************************************
// Method: AddOperatorAction::Enabled()
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
// ****************************************************************************

bool
AddOperatorAction::Enabled() const
{
    return ViewerMultipleAction::Enabled() &&
           (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0) &&
           !window->GetTypeIsCurve();
}

// ****************************************************************************
// Method: AddOperatorAction::ChoiceEnabled
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
AddOperatorAction::ChoiceEnabled(int) const
{
    return true;
}

// ****************************************************************************
// Method: AddOperatorAction::ConstructToolbar
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
//   
// ****************************************************************************

void
AddOperatorAction::ConstructToolbar(QToolBar *toolbar)
{
    ViewerMultipleAction::ConstructToolbar(toolbar);

#if 1
    // This is a hack. I eventually need to put the toolbar orientation
    // into the ViewerWindowManagerAttributes.

    // Make the toolbar be vertical.
    toolbar->setOrientation(Qt::Vertical);
    if(toolbar->mainWindow())
    {
        toolbar->mainWindow()->moveToolBar(toolbar, QMainWindow::Left);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: PromoteOperatorAction::PromoteOperatorAction
//
// Purpose: 
//   Constructor for the PromoteOperatorAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:47:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PromoteOperatorAction::PromoteOperatorAction(ViewerWindow *win) :
    ViewerAction(win, "PromoteOperatorAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: PromoteOperatorAction::~PromoteOperatorAction
//
// Purpose: 
//   Destructor for the PromoteOperatorAction class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:36 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

PromoteOperatorAction::~PromoteOperatorAction()
{
}

// ****************************************************************************
// Method: PromoteOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
PromoteOperatorAction::Execute()
{
    bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->PromoteOperator(
        args.GetOperatorType(), applyToAll);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DemoteOperatorAction::DemoteOperatorAction
//
// Purpose: 
//   Constructor for the DemoteOperatorAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:47:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DemoteOperatorAction::DemoteOperatorAction(ViewerWindow *win) :
    ViewerAction(win, "DemoteOperatorAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: DemoteOperatorAction::~DemoteOperatorAction
//
// Purpose: 
//   Destructor for the DemoteOperatorAction class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:36 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DemoteOperatorAction::~DemoteOperatorAction()
{
}

// ****************************************************************************
// Method: DemoteOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DemoteOperatorAction::Execute()
{
    bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->DemoteOperator(
        args.GetOperatorType(), applyToAll);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveOperatorAction::RemoveOperatorAction
//
// Purpose: 
//   Constructor for the RemoveOperatorAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:47:19 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveOperatorAction::RemoveOperatorAction(ViewerWindow *win) :
    ViewerAction(win, "RemoveOperatorAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: RemoveOperatorAction::~RemoveOperatorAction
//
// Purpose: 
//   Destructor for the RemoveOperatorAction class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:36 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveOperatorAction::~RemoveOperatorAction()
{
}

// ****************************************************************************
// Method: RemoveOperatorAction::Execute
//
// Purpose: 
//   This method executes the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Apr 10 09:48:59 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveOperatorAction::Execute()
{
    bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->RemoveOperator(
        args.GetOperatorType(), applyToAll);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveLastOperatorAction::RemoveLastOperatorAction
//
// Purpose: 
//   Constructor for the RemoveLastOperatorAction class.
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

RemoveLastOperatorAction::RemoveLastOperatorAction(ViewerWindow *win) :
    ViewerAction(win, "RemoveLastOperatorAction")
{
    SetAllText("Remove last operator");
    if(!window->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(removelastoperator_xpm)));
}

// ****************************************************************************
// Method: RemoveLastOperatorAction::~RemoveLastOperatorAction
//
// Purpose: 
//   Destructor for the RemoveLastOperatorAction class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveLastOperatorAction::~RemoveLastOperatorAction()
{
}

// ****************************************************************************
// Method: RemoveLastOperatorAction::Execute
//
// Purpose: 
//   Executes the work for RemoveLastOperatorAction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveLastOperatorAction::Execute()
{
    bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->RemoveLastOperator(applyToAll);
}

// ****************************************************************************
// Method: RemoveLastOperatorAction::Enabled
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
//   
// ****************************************************************************

bool
RemoveLastOperatorAction::Enabled() const
{
    return !window->GetTypeIsCurve() &&
           window->GetAnimation()->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: RemoveAllOperatorsAction::RemoveAllOperatorsAction
//
// Purpose: 
//   Constructor for the RemoveAllOperatorsAction class.
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

RemoveAllOperatorsAction::RemoveAllOperatorsAction(ViewerWindow *win) : 
    ViewerAction(win, "RemoveAllOperatorsAction")
{
    SetAllText("Remove all operators");
    if(!window->GetNoWinMode())
        SetIconSet(QIconSet(QPixmap(removealloperators_xpm)));
}

// ****************************************************************************
// Method: RemoveAllOperatorsAction::~RemoveAllOperatorsAction
//
// Purpose: 
//   Denstructor for the RemoveAllOperatorsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

RemoveAllOperatorsAction::~RemoveAllOperatorsAction()
{
}

// ****************************************************************************
// Method: RemoveAllOperatorsAction::Execute
//
// Purpose: 
//   Executes the RemoveAllOperatorsAction.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
RemoveAllOperatorsAction::Execute()
{
    bool applyToAll = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->RemoveAllOperators(applyToAll);
}

// ****************************************************************************
// Method: RemoveAllOperatorsAction::Enabled
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
// ****************************************************************************

bool
RemoveAllOperatorsAction::Enabled() const
{
    return !window->GetTypeIsCurve() &&
            window->GetAnimation()->GetPlotList()->GetNumPlots() > 0;
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetOperatorOptionsAction::SetOperatorOptionsAction
//
// Purpose: 
//   Constructor for the SetOperatorOptionsAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:58:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetOperatorOptionsAction::SetOperatorOptionsAction(ViewerWindow *win) : 
   ViewerAction(win, "SetOperatorOptionsAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetOperatorOptionsAction::~SetOperatorOptionsAction
//
// Purpose: 
//   Destructor for the SetOperatorOptionsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:02:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetOperatorOptionsAction::~SetOperatorOptionsAction()
{
}

// ****************************************************************************
// Method: SetOperatorOptionsAction::Execute
//
// Purpose: 
//   Sets the operator attributes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:02:54 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetOperatorOptionsAction::Execute()
{
    //
    // Set the operator attributes.
    //
    int  oper = args.GetOperatorType();
    bool apply = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->SetPlotOperatorAtts(oper, apply);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: AddPlotAction::AddPlotAction
//
// Purpose: 
//   Constructor for the AddPlotAction class.
//
// Arguments:
//   win : The window that owns the action.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AddPlotAction::AddPlotAction(ViewerWindow *win) : ViewerMultipleAction(win,
    "AddPlotAction"), database(), pluginEntries(), menuPopulator()
{
    SetAllText("Add plot");
    SetExclusive(false);

    maxPixmapWidth = maxPixmapHeight = 0;
    menu = 0;

    //
    // Iterate through all of the loaded plot plugins and add a
    // choice for all of the ones that have icons.
    //
    ViewerPlotPluginInfo *info = 0;
    PlotPluginManager *pluginMgr = PlotPluginManager::Instance();
    for(int i = 0; i < pluginMgr->GetNEnabledPlugins(); ++i)
    {
        info = pluginMgr->GetViewerPluginInfo(pluginMgr->GetEnabledID(i));
        if(info)
        {
            if(!window->GetNoWinMode() && info->XPMIconData() != 0 &&
               std::string(info->GetName()) != "Curve")
            {
                char tip[200];
                SNPRINTF(tip, 200, "Add %s plot", info->GetName());

                // Create a pixmap for the plot or get its pixmap from
                // the pixmap cache.
                QString key;
                key.sprintf("plot_icon_%s", info->GetName());
                QPixmap pix;
                if(!QPixmapCache::find(key, pix))
                {
                    pix = QPixmap(info->XPMIconData());
                    QPixmapCache::insert(key, pix);
                }

                // Find the maximum pixmap width and height
                maxPixmapWidth = QMAX(maxPixmapWidth, pix.width());
                maxPixmapHeight = QMAX(maxPixmapHeight, pix.height());

                // Add a choice for plot so that it has an icon.
                AddChoice(info->GetName(), tip, pix);

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
// Method: AddPlotAction::~AddPlotAction
//
// Purpose: 
//   Destructor for the AddPlotAction class.
//
// Note:       We manually delete the menus because they have no parent widget.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:42:34 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

AddPlotAction::~AddPlotAction()
{
    for(int i = 0; i < pluginEntries.size(); ++i)
        delete pluginEntries[i].varMenu;
}

// ****************************************************************************
// Method: AddPlotAction::Update
//
// Purpose: 
//   This method is called when the action needs to be updated.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:43:45 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
AddPlotAction::Update()
{
    if(pluginEntries.size() > 0)
    {
        const std::string &hostdb = window->GetAnimation()->GetPlotList()->GetHostDatabaseName();

        if(database != hostdb)
        {
            database = hostdb;

            ViewerFileServer *fileServer = ViewerFileServer::Instance();
            std::string host, db;
            ViewerPlotList::SplitHostDatabase(database, host, db);
            const avtDatabaseMetaData *md = fileServer->GetMetaData(host, db);
            const avtSIL *sil = fileServer->GetSIL(host, db);
            const ExpressionList *exprList = ViewerExpressionList::Instance()->GetList();

            //
            // Repopulate the menu variable list using information from the
            // new file.
            //
            menuPopulator.PopulateVariableLists(md, sil, exprList);

            //
            // Update the variable menus for the actions.
            //
            bool menuEnabled = false;
            for(int i = 0; i < pluginEntries.size(); ++i)
            {
                menuPopulator.UpdateSingleVariableMenu(pluginEntries[i].varMenu,
                    this, pluginEntries[i].varTypes);

                bool enabled = menuPopulator.ItemEnabled(pluginEntries[i].varTypes);

                // Set the new menu's enabled state based on the variable type.
                actionMenu->setItemEnabled(i, enabled);
                menu->setItemEnabled(i, enabled);
                menuEnabled |= enabled;
            }

            menu->setEnabled(menuEnabled);
        }
    }

    ViewerMultipleAction::Update();
}

// ****************************************************************************
// Method: AddPlotAction::Execute
//
// Purpose: 
//   This method is called when the AddPlotAction must be executed.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:44:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddPlotAction::Execute(int)
{
    bool replacePlots = windowMgr->GetClientAtts()->GetReplacePlots();
    bool applyOperator = windowMgr->GetClientAtts()->GetApplyOperator();

    //
    // Try and create the plot.
    //
    window->GetAnimation()->GetPlotList()->AddPlot(args.GetPlotType(),
        args.GetVariable().c_str(), replacePlots, applyOperator);
}

// ****************************************************************************
// Method: AddPlotAction::Enabled
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
// ****************************************************************************

bool
AddPlotAction::Enabled() const
{
    bool dbIsOpen = (window->GetAnimation()->GetPlotList()->GetHostDatabaseName().length() > 0);
    return ViewerMultipleAction::Enabled() &&
           dbIsOpen &&
           !window->GetTypeIsCurve();
}

// ****************************************************************************
// Method: AddPlotAction::ChoiceEnabled
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
AddPlotAction::ChoiceEnabled(int i) const
{
    return menuPopulator.ItemEnabled(pluginEntries[i].varTypes);
}

// ****************************************************************************
// Method: AddPlotAction::ConstructMenu
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
//   
// ****************************************************************************

void
AddPlotAction::ConstructMenu(QPopupMenu *menu)
{
    // Create a new menu and add all of the actions to it.
    actionMenu = new QPopupMenu(menu, "AddPlotAction");

    for(int i = 0; i < pluginEntries.size(); ++i)
    {
        // Create the menu for the plot.
        pluginEntries[i].varMenu = new QvisVariablePopupMenu(i, 0,
            children[i]->menuText());
        connect(pluginEntries[i].varMenu, SIGNAL(activated(int, const QString &)),
            this, SLOT(addPlot(int, const QString &)));

        // Add the menu to the action menu.
        actionMenu->insertItem(children[i]->iconSet(),
            children[i]->menuText(), pluginEntries[i].varMenu,
            actionMenu->count());
    }

    // Insert the new menu into the old menu.
    if(iconSpecified)
        actionMenuId = menu->insertItem(action->iconSet(), action->menuText(), actionMenu);
    else
        actionMenuId = menu->insertItem(action->menuText(), actionMenu);
}

// ****************************************************************************
// Method: AddPlotAction::RemoveFromMenu
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
AddPlotAction::RemoveFromMenu(QPopupMenu *menu)
{
    // NOT IMPLEMENTED. Remove the action from the menu.
}

// ****************************************************************************
// Method: AddPlotAction::ConstructToolbar
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
//   
// ****************************************************************************

void
AddPlotAction::ConstructToolbar(QToolBar *toolbar)
{
    // If we don't have any plugin entries, return.
    if(pluginEntries.size() < 1)
        return;

    //
    // Connect the toolbar to a slot function that lets this object know when
    // it changes orientations.
    //
    connect(toolbar, SIGNAL(orientationChanged(Orientation)),
            this, SLOT(orientationChanged(Orientation)));

    // Add a menu bar to the toolbar.
    menu = new QMenuBar(toolbar, "AddPlotAction");
    menu->setFrameStyle(QFrame::NoFrame);
    QRect tipRectH(0,0,0,0);
    tipRectH.setX(menu->contentsRect().x());
    tipRectH.setY(menu->contentsRect().y());
    QRect tipRectV(0,0,0,0);
    tipRectV.setX(menu->contentsRect().x());
    tipRectV.setY(menu->contentsRect().y());
    for(int i = 0; i < pluginEntries.size(); ++i)
    {
        // Create the menu for the plot.
        if(pluginEntries[i].varMenu == 0)
        {
            pluginEntries[i].varMenu = new QvisVariablePopupMenu(i, 0,
                children[i]->menuText());
            connect(pluginEntries[i].varMenu, SIGNAL(activated(int, const QString &)),
                this, SLOT(addPlot(int, const QString &)));
        }

        // Add the menu to the action menu.
        QPixmap pix(children[i]->iconSet().pixmap(QIconSet::Small,
                    QIconSet::Normal));
        int id = menu->insertItem(pix, pluginEntries[i].varMenu, menu->count());
        menu->setItemEnabled(id, false);

        // Create the plot's tool tip string.
        QString tip; tip.sprintf("Add %s plot", children[i]->menuText().latin1());

        // Add a tooltip in the horizontal direction.
        tipRectH.setWidth(pix.width());
        tipRectH.setHeight(pix.height());
        QToolTip::add(menu, tipRectH, tip);
        tipRectH.setX(tipRectH.x() + pix.width() + 4);

        // Add a tooltip in the vertical direction.
        tipRectV.setWidth(pix.width());
        tipRectV.setHeight(pix.height());
        QToolTip::add(menu, tipRectV, tip);
        tipRectV.setY(tipRectV.y() + pix.height() + 4);
    }

    menu->setMaximumWidth(maxPixmapWidth);

#if 1
    // This is a hack. I eventually need to put the toolbar orientation
    // into the ViewerWindowManagerAttributes.

    // Make the toolbar be vertical.
    toolbar->setOrientation(Qt::Vertical);
    if(toolbar->mainWindow())
    {
        toolbar->mainWindow()->moveToolBar(toolbar, QMainWindow::Left);
    }
#endif
}

// ****************************************************************************
// Method: AddPlotAction::addPlot
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
//   
// ****************************************************************************

void
AddPlotAction::addPlot(int index, const QString &var)
{
    // Hide the menu if it is active.
    int plotType = pluginEntries[index].index;
    pluginEntries[index].varMenu->hide();

    // Set the plot type and variable name into the args.
    args.SetPlotType(plotType);
    args.SetVariable(var.latin1());

    // Execute the action.
    Activate(false);
}

// ****************************************************************************
// Method: AddPlotAction::orientationChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the toolbar changes
//   orientations. We use this information to set the maximum height and
//   widths on the menu so it will stack vertically if we need it to do that.
//
// Arguments:
//   o : The orientation of the toolbar.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 20 12:52:51 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
AddPlotAction::orientationChanged(Qt::Orientation o)
{
    if(o == Horizontal)
    {
        menu->setMaximumWidth(1024);
        menu->setMaximumHeight(maxPixmapHeight);
    }
    else
    {
        menu->setMaximumWidth(maxPixmapWidth);
        menu->setMaximumHeight(1024);
    }
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DrawPlotsAction::DrawPlotsAction
//
// Purpose: 
//   Constructor for the DrawPlotsAction class.
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

DrawPlotsAction::DrawPlotsAction(ViewerWindow *win) : ViewerAction(win,
    "DrawPlotsAction")
{
    SetAllText("Draw plots");
}

// ****************************************************************************
// Method: DrawPlotsAction::~DrawPlotsAction
//
// Purpose: 
//   Destructor for the DrawPlotsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:55:34 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DrawPlotsAction::~DrawPlotsAction()
{
}

// ****************************************************************************
// Method: DrawPlotsAction::Execute
//
// Purpose: 
//   Draws the plots
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
DrawPlotsAction::Execute()
{
    window->GetAnimation()->GetPlotList()->RealizePlots();
}

// ****************************************************************************
// Method: DrawPlotsAction::Enabled
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
DrawPlotsAction::Enabled() const
{
    return (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: HideActivePlotsAction::HideActivePlotsAction
//
// Purpose: 
//   Constructor for the HideActivePlotsAction class.
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

HideActivePlotsAction::HideActivePlotsAction(ViewerWindow *win) : ViewerAction(win,
    "HideActivePlotsAction")
{
    SetAllText("Hide active plots");
    // Think of an icon...
}

// ****************************************************************************
// Method: HideActivePlotsAction::~HideActivePlotsAction
//
// Purpose: 
//   Destructor for the HideActivePlotsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:54:27 PST 2003
//
// Modifications:
//   
// ****************************************************************************

HideActivePlotsAction::~HideActivePlotsAction()
{
}

// ****************************************************************************
// Method: HideActivePlotsAction::Execute
//
// Purpose: 
//   Hides the active plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
HideActivePlotsAction::Execute()
{
    window->GetAnimation()->GetPlotList()->HideActivePlots();
}

// ****************************************************************************
// Method: HideActivePlotsAction::Enabled
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
HideActivePlotsAction::Enabled() const
{
    return (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeleteActivePlotsAction::DeleteActivePlotsAction
//
// Purpose: 
//   Constructor for the DeleteActivePlotsAction class.
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

DeleteActivePlotsAction::DeleteActivePlotsAction(ViewerWindow *win) :
    ViewerAction(win, "DeleteActivePlotsAction")
{
    SetAllText("Delete active plots");
    // Think of an icon...
}

// ****************************************************************************
// Method: DeleteActivePlotsAction::~DeleteActivePlotsAction
//
// Purpose: 
//   Destructor for the DeleteActivePlotsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:14 PST 2003
//
// Modifications:
//   
// ****************************************************************************

DeleteActivePlotsAction::~DeleteActivePlotsAction()
{
}

// ****************************************************************************
// Method: DeleteActivePlotsAction::Execute
//
// Purpose: 
//   Executes the DeleteActivePlots action.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 21 15:51:48 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
DeleteActivePlotsAction::Execute()
{
    window->GetAnimation()->GetPlotList()->DeleteActivePlots();
    if(window->GetAnimation()->GetPlotList()->GetNumPlots() == 0)
    {
        windowMgr->ClearPickPoints();
        windowMgr->ClearRefLines();
        windowMgr->ResetLineoutDesignation();
        windowMgr->DisableAllTools();
    }
}

// ****************************************************************************
// Method: DeleteActivePlotsAction::Enabled
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
DeleteActivePlotsAction::Enabled() const
{
    return (window->GetAnimation()->GetPlotList()->GetNumPlots() > 0);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetActivePlotsAction::SetActivePlotsAction
//
// Purpose: 
//   Constructor for the SetActivePlotsAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:50:03 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetActivePlotsAction::SetActivePlotsAction(ViewerWindow *win) :
    ViewerAction(win, "SetActivePlotsAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetActivePlotsAction::~SetActivePlotsAction
//
// Purpose: 
//   Destructor for the SetActivePlotsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:50:31 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetActivePlotsAction::~SetActivePlotsAction()
{
}

// ****************************************************************************
// Method: SetActivePlotsAction::Execute
//
// Purpose: 
//   Sets the active plots for the window's plot list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:50:49 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetActivePlotsAction::Execute()
{
    //
    // Set the active plots through the window manager.
    //
    const intVector &activePlots     = args.GetActivePlotIds();
    const intVector &activeOperators = args.GetActiveOperatorIds();
    const intVector &expandedPlots   = args.GetExpandedPlotIds();
    bool moreThanPlotsValid          = args.GetBoolFlag();
    window->GetAnimation()->GetPlotList()->SetActivePlots(activePlots,
        activeOperators, expandedPlots, moreThanPlotsValid);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: ChangeActivePlotsVarAction::ChangeActivePlotsVarAction
//
// Purpose: 
//   Constructor for the ChangeActivePlotsVarAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:51:26 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ChangeActivePlotsVarAction::ChangeActivePlotsVarAction(ViewerWindow *win) : 
   ViewerAction(win, "ChangeActivePlotsVarAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: ChangeActivePlotsVarAction::~ChangeActivePlotsVarAction
//
// Purpose: 
//   Destructor for the ChangeActivePlotsVarAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:51:53 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

ChangeActivePlotsVarAction::~ChangeActivePlotsVarAction()
{
}

// ****************************************************************************
// Method: ChangeActivePlotsVarAction::Execute
//
// Purpose: 
//   Changes the active plots' variables.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:52:13 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
ChangeActivePlotsVarAction::Execute()
{
    //
    // Set the plot variable for the selected plots.
    //
    const char *var = args.GetVariable().c_str();
    window->GetAnimation()->GetPlotList()->SetPlotVar(var);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotSILRestrictionAction::SetPlotSILRestrictionAction
//
// Purpose: 
//   Constructor for the SetPlotSILRestrictionAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:54:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotSILRestrictionAction::SetPlotSILRestrictionAction(ViewerWindow *win) : 
   ViewerAction(win, "SetPlotSILRestrictionAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetPlotSILRestrictionAction::~SetPlotSILRestrictionAction
//
// Purpose: 
//   Destructor for the SetPlotSILRestrictionAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:55:00 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotSILRestrictionAction::~SetPlotSILRestrictionAction()
{
}

// ****************************************************************************
// Method: SetPlotSILRestrictionAction::Execute
//
// Purpose: 
//   Sets the SIL restriction for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 07:55:20 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetPlotSILRestrictionAction::Execute()
{
    //
    // Tell the plot list set the current SIL restriction for the
    // selected plots.
    //
    bool apply = windowMgr->GetClientAtts()->GetApplyOperator();
    window->GetAnimation()->GetPlotList()->SetPlotSILRestriction(apply);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotOptionsAction::SetPlotOptionsAction
//
// Purpose: 
//   Constructor for the SetPlotOptionsAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:23:28 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotOptionsAction::SetPlotOptionsAction(ViewerWindow *win) : 
   ViewerAction(win, "SetPlotOptionsAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetPlotOptionsAction::~ChangeActivePlotsVarAction
//
// Purpose: 
//   Destructor for the SetPlotOptionsAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:24:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotOptionsAction::~SetPlotOptionsAction()
{
}

// ****************************************************************************
// Method: SetPlotOptionsAction::Execute
//
// Purpose: 
//   Sets the plot options for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:24:25 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetPlotOptionsAction::Execute()
{
    window->GetAnimation()->GetPlotList()->SetPlotAtts(args.GetPlotType());
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotFrameRangeAction::SetPlotFrameRangeAction
//
// Purpose: 
//   Constructor for the SetPlotFrameRangeAction class.
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:27:01 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotFrameRangeAction::SetPlotFrameRangeAction(ViewerWindow *win) : 
   ViewerAction(win, "SetPlotFrameRangeAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetPlotFrameRangeAction::~SetPlotFrameRangeAction
//
// Purpose: 
//   Destructor for the SetPlotFrameRangeAction class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:27:26 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotFrameRangeAction::~SetPlotFrameRangeAction()
{
}

// ****************************************************************************
// Method: SetPlotFrameRangeAction::Execute
//
// Purpose: 
//   Sets the frame range for the selected plots.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:27:45 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetPlotFrameRangeAction::Execute()
{
    //
    // Set the plot frame range.
    //
    int plotId = args.GetIntArg1();
    int frame0 = args.GetIntArg2();
    int frame1 = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetAnimation()->GetPlotList();
    plotList->SetPlotFrameRange(plotId, frame0, frame1);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeletePlotKeyframeAction::DeletePlotKeyframeAction
//
// Purpose: 
//   Constructor
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:33:28 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DeletePlotKeyframeAction::DeletePlotKeyframeAction(ViewerWindow *win) : 
   ViewerAction(win, "DeletePlotKeyframeAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: DeletePlotKeyframeAction::~DeletePlotKeyframeAction
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:33:52 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DeletePlotKeyframeAction::~DeletePlotKeyframeAction()
{
}

// ****************************************************************************
// Method: DeletePlotKeyframeAction::Execute
//
// Purpose: 
//   Deletes a plot keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:34:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DeletePlotKeyframeAction::Execute()
{
    //
    // Delete the plot keyframe.
    //
    int plotId = args.GetIntArg1();
    int frame = args.GetIntArg2();
    window->GetAnimation()->GetPlotList()->DeletePlotKeyframe(plotId, frame);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovePlotKeyframeAction::MovePlotKeyframeAction
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   win : the window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:35:39 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MovePlotKeyframeAction::MovePlotKeyframeAction(ViewerWindow *win) : 
   ViewerAction(win, "MovePlotKeyframeAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: MovePlotKeyframeAction::~MovePlotKeyframeAction
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:36:03 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MovePlotKeyframeAction::~MovePlotKeyframeAction()
{
}

// ****************************************************************************
// Method: MovePlotKeyframeAction::Execute
//
// Purpose: 
//   Moves a plot keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:36:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MovePlotKeyframeAction::Execute()
{
    //
    // Move the plot keyframe.
    //
    int plotId = args.GetIntArg1();
    int oldFrame = args.GetIntArg2();
    int newFrame = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetAnimation()->GetPlotList();
    plotList->MovePlotKeyframe(plotId, oldFrame, newFrame);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: SetPlotDatabaseStateAction::SetPlotDatabaseStateAction
//
// Purpose: 
//   Constructor
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:38:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotDatabaseStateAction::SetPlotDatabaseStateAction(ViewerWindow *win) : 
   ViewerAction(win, "SetPlotDatabaseStateAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: SetPlotDatabaseStateAction::~SetPlotDatabaseStateAction
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:38:55 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

SetPlotDatabaseStateAction::~SetPlotDatabaseStateAction()
{
}

// ****************************************************************************
// Method: SetPlotDatabaseStateAction::Execute
//
// Purpose: 
//   Sets a database keyframe for the plot.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:39:14 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
SetPlotDatabaseStateAction::Execute()
{
    //
    // Perform the rpc.
    //
    int plotId = args.GetIntArg1();
    int frame = args.GetIntArg2();
    int state = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetAnimation()->GetPlotList();
    plotList->SetPlotDatabaseState(plotId, frame, state);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: DeletePlotDatabaseKeyframeAction::DeletePlotDatabaseKeyframeAction
//
// Purpose: 
//   Constructor
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:41:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DeletePlotDatabaseKeyframeAction::DeletePlotDatabaseKeyframeAction(
   ViewerWindow *win) : ViewerAction(win, "DeletePlotDatabaseKeyframeAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: DeletePlotDatabaseKeyframeAction::~DeletePlotDatabaseKeyframeAction
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:41:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

DeletePlotDatabaseKeyframeAction::~DeletePlotDatabaseKeyframeAction()
{
}

// ****************************************************************************
// Method: DeletePlotDatabaseKeyframeAction::Execute
//
// Purpose: 
//   Deletes a database keyframe.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:42:04 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
DeletePlotDatabaseKeyframeAction::Execute()
{
    //
    // Delete the database keyframe
    //
    int plotId = args.GetIntArg1();
    int frame = args.GetIntArg2();
    ViewerPlotList *plotList = window->GetAnimation()->GetPlotList();
    plotList->DeletePlotDatabaseKeyframe(plotId, frame);
}

///////////////////////////////////////////////////////////////////////////////
///
///////////////////////////////////////////////////////////////////////////////

// ****************************************************************************
// Method: MovePlotDatabaseKeyframeAction::MovePlotDatabaseKeyframeAction
//
// Purpose: 
//   Constructor
//
// Arguments:
//   win : The window to which the action belongs.
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:44:42 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MovePlotDatabaseKeyframeAction::MovePlotDatabaseKeyframeAction(
   ViewerWindow *win) : ViewerAction(win, "MovePlotDatabaseKeyframeAction")
{
    DisableVisual();
}

// ****************************************************************************
// Method: MovePlotDatabaseKeyframeAction::~MovePlotDatabaseKeyframeAction
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:45:06 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MovePlotDatabaseKeyframeAction::~MovePlotDatabaseKeyframeAction()
{
}

// ****************************************************************************
// Method: MovePlotDatabaseKeyframeAction::Execute
//
// Purpose: 
//   Moves a database keyframe.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Apr 11 08:45:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MovePlotDatabaseKeyframeAction::Execute()
{
    //
    // Move the database keyframe.
    //
    int plotId = args.GetIntArg1();
    int oldFrame = args.GetIntArg2();
    int newFrame = args.GetIntArg3();
    ViewerPlotList *plotList = window->GetAnimation()->GetPlotList();
    plotList->MovePlotDatabaseKeyframe(plotId, oldFrame, newFrame);
}
