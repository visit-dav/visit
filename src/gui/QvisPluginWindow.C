#include <QvisPluginWindow.h>
#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qvbox.h>
#include <qradiobutton.h>
#include <qlistview.h>

#include <PluginManagerAttributes.h>
#include <DataNode.h>
#include <ViewerProxy.h>


// ****************************************************************************
//  Method: QvisPluginWindow::QvisPluginWindow
//
//  Purpose: 
//    This is the constructor for the QvisPluginWindow class.
//
//  Arguments:
//    subj    : The PluginAttributes object that the window observes.
//    caption : The string that appears in the window decorations.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:52:35 PDT 2001
//    Removed default settings capabilities.
//
// ****************************************************************************

QvisPluginWindow::QvisPluginWindow(PluginManagerAttributes *subj,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false)
{
    pluginAtts = subj;
    activeTab = 0;
    pluginsInitialized = false;
}

// ****************************************************************************
//  Method: QvisPluginWindow::~QvisPluginWindow
//
//  Purpose: 
//    This is the destructor for the QvisPluginWindow class.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//   
// ****************************************************************************

QvisPluginWindow::~QvisPluginWindow()
{
    pluginAtts = 0;
}

// ****************************************************************************
//  Method: QvisPluginWindow::CreateWindowContents
//
//  Purpose: 
//    This method creates all of the widgets for the window.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Mar 22 14:02:59 PST 2005
//    Remove the databases tab.
//
// ****************************************************************************

void
QvisPluginWindow::CreateWindowContents()
{
    // Create the tab widget.
    QTabWidget *tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->addWidget(tabs);    

    //
    // Create the plot page
    //
    pagePlots = new QVBox(central, "pagePlots");
    pagePlots->setSpacing(5);
    pagePlots->setMargin(10);
    tabs->addTab(pagePlots, "Plots");

    listPlots = new QListView(pagePlots, "listPlots");
    listPlots->addColumn("  ");
    listPlots->addColumn("Name");
    listPlots->addColumn("Version");
    listPlots->setAllColumnsShowFocus(true);
    listPlots->setColumnAlignment(0, Qt::AlignHCenter);
    listPlots->setColumnAlignment(2, Qt::AlignHCenter);

    //
    // Create the operator page
    //
    pageOperators = new QVBox(central, "pageOperators");
    pageOperators->setSpacing(10);
    pageOperators->setMargin(10);
    tabs->addTab(pageOperators, "Operators");

    listOperators = new QListView(pageOperators, "listOperators");
    listOperators->addColumn("  ");
    listOperators->addColumn("Name");
    listOperators->addColumn("Version");
    listOperators->setAllColumnsShowFocus(true);
    listOperators->setColumnAlignment(0, Qt::AlignHCenter);
    listOperators->setColumnAlignment(2, Qt::AlignHCenter);

    //
    // Create the database page
    //
    pageDatabases = new QVBox(central, "pageDatabases");
    //pageDatabases->setFrameStyle(QFrame::NoFrame);
    pageDatabases->setSpacing(10);
    pageDatabases->setMargin(10);
    tabs->addTab(pageDatabases, "Databases");

    new QLabel("Database plugins are not yet accessible from the GUI",
               pageDatabases);
    /*
    listDatabases = new QListView(pageDatabases, "listDatabases");
    listDatabases->addColumn("  ");
    listDatabases->addColumn("Name");
    listDatabases->addColumn("Version");
    listDatabases->setAllColumnsShowFocus(true);
    listDatabases->setColumnAlignment(0, Qt::AlignHCenter);
    listDatabases->setColumnAlignment(2, Qt::AlignHCenter);
    */

    // Show the appropriate page based on the activeTab setting.
    tabs->blockSignals(true);
    if(activeTab == 0)
        tabs->showPage(pagePlots);
    else if(activeTab == 1)
        tabs->showPage(pageOperators);
    else
        tabs->showPage(pageDatabases);
    tabs->blockSignals(false);
}

// ****************************************************************************
// Method: QvisPluginWindow::Update
//
// Purpose:
//     Override the parent so we know when the plugin attributes 
//     have been initialized by the viewer
//
// Programmer: Jeremy Meredith
// Creation:   September 27, 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::Update(Subject *s)
{
    QvisPostableWindowObserver::Update(s);
    if (!pluginsInitialized)
    {
        pluginsInitialized = true;
        emit pluginSettingsChanged();
    }
}

// ****************************************************************************
//  Method: QvisPluginWindow::UpdateWindow
//
//  Purpose: 
//    This method is called when the plugin attributes object that the
//    window observes is changed. It is this method's responsibility to set the
//    state of the window's widgets to match the state of the plugin
//    attributes.
//
//  Arguments:
//    doAll : A flag that tells the method to ignore which attributes are
//            selected. All widgets are updated if doAll is true.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Sep 6 22:58:34 PST 2001
//    Added checks for Qt's version
//
//    Jeremy Meredith, Fri Sep 28 13:52:35 PDT 2001
//    Added ability to keep track of which items are checked/unchecked.
//
// ****************************************************************************

void
QvisPluginWindow::UpdateWindow(bool doAll)
{
    listPlots->clear();
    listPlots->setSorting(1, true);
    int i;
    for (i=0; i<pluginAtts->GetName().size(); i++)
    {
        if (pluginAtts->GetType()[i] == "plot")
        {
            QCheckListItem *item = new QCheckListItem(listPlots, "", QCheckListItem::CheckBox);
            item->setOn(pluginAtts->GetEnabled()[i]);
            item->setText(1,pluginAtts->GetName()[i].c_str());
            item->setText(2,pluginAtts->GetVersion()[i].c_str());

            plotItems.push_back(item);
            plotIDs.push_back(pluginAtts->GetId()[i]);
        }
    }

    listOperators->clear();
    listOperators->setSorting(1, true);
    for (i=0; i<pluginAtts->GetName().size(); i++)
    {
        if (pluginAtts->GetType()[i] == "operator")
        {
            QCheckListItem *item = new QCheckListItem(listOperators, "", QCheckListItem::CheckBox);
            item->setOn(pluginAtts->GetEnabled()[i]);
            item->setText(1,pluginAtts->GetName()[i].c_str());
            item->setText(2,pluginAtts->GetVersion()[i].c_str());

            operatorItems.push_back(item);
            operatorIDs.push_back(pluginAtts->GetId()[i]);
        }
    }
}

// ****************************************************************************
//  Method: QvisPluginWindow::Apply
//
//  Purpose: 
//    This method tells the plugin attributes to notify all observers of
//    changes in the object.
//
//  Arguments:
//    dontIgnore : If this is true, the new state takes effect immediately.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Sep 28 13:52:35 PDT 2001
//    Added ability to keep track of which items are checked/unchecked.
//
//    Jeremy Meredith, Fri Mar 19 13:46:31 PST 2004
//    Added warning to inform user about the need to save settings and
//    restart visit to pick up changes to plugins.
//
// ****************************************************************************

void
QvisPluginWindow::Apply(bool dontIgnore)
{
    int i;
    for (i=0; i<plotItems.size(); i++)
    {
        pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(plotIDs[i])] =
            plotItems[i]->isOn();
    }
    for (i=0; i<operatorItems.size(); i++)
    {
        pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(operatorIDs[i])] =
            operatorItems[i]->isOn();
    }

    pluginAtts->Notify();

    GUIBase::Warning("Note:  Plugins are loaded at startup.  Please save "
                     "your settings and restart VisIt to apply your changes.");
}

// ****************************************************************************
//  Method: QvisPostableWindow::CreateNode
//
//  Purpose: 
//    Writes the window's extra information to the config file.
//
//  Arguments:
//    parentNode : The node to which the window's attributes are added.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowObserver::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(std::string(caption().latin1()));

        // Save the current tab.
        node->AddNode(new DataNode("activeTab", activeTab));
    }
}

// ****************************************************************************
//  Method: QvisPluginWindow::SetFromNode
//
//  Purpose: 
//    Reads window attributes from the DataNode representation of the config
//    file.
//
//  Arguments:
//    parentNode : The data node that contains the window's attributes.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(std::string(caption().latin1()));
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node;
    if((node = winNode->GetNode("activeTab")) != 0)
    {
        activeTab = node->AsInt();
        if(activeTab != 0 && activeTab != 1 && activeTab != 2)
            activeTab = 0;
    }

    // Call the base class's function.
    QvisPostableWindowObserver::SetFromNode(parentNode, borders);
}

// ****************************************************************************
//  Method: QvisPluginWindow::apply
//
//  Purpose: 
//    This is a Qt slot function that applies the plugin attributes
//    unconditionally.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::apply()
{
    //SetUpdate(false);
    Apply(true);
}

// ****************************************************************************
//  Method: QvisPluginWindow::tabSelected
//
//  Purpose: 
//    This is a Qt slot function that is called when the tabs are changed.
//
//  Arguments:
//    index : The new active tab.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 31, 2001
//
//  Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::tabSelected(const QString &tabLabel)
{
    if(tabLabel == QString("Plots"))
        activeTab = 0;
    else if(tabLabel == QString("Operators"))
        activeTab = 1;
    else
        activeTab = 2;
}
