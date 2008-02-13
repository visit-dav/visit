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
#include <FileOpenOptions.h>
#include <DBOptionsAttributes.h>
#include <DataNode.h>
#include <ViewerProxy.h>

#include <QvisDBOptionsDialog.h>

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
//    Jeremy Meredith, Wed Jan 23 15:35:39 EST 2008
//    We're observer two subjects now, so inherit from simpleobserver.
//
// ****************************************************************************

QvisPluginWindow::QvisPluginWindow(const char *caption, const char *shortName,
                                   QvisNotepadArea *notepad) :
    QvisPostableWindowSimpleObserver(caption, shortName, notepad,
                               QvisPostableWindowSimpleObserver::ApplyButton)
{
    pluginAtts = NULL;
    fileOpenOptions = NULL;
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
//    Jeremy Meredith, Wed Jan 23 15:36:03 EST 2008
//    Also observer fileOpenOptions for the database options.
//   
// ****************************************************************************

QvisPluginWindow::~QvisPluginWindow()
{
    pluginAtts = NULL;
    fileOpenOptions = NULL;
}


// ****************************************************************************
// Method: QvisPluginWindow::ConnectSubjects
//
// Purpose: 
//   This function connects subjects so that the window observes them.
//
// Programmer: Jeremy Meredith
// Creation:   January 23, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::ConnectSubjects(PluginManagerAttributes *p,
                                       FileOpenOptions *f)
{
    pluginAtts = p;
    pluginAtts->Attach(this);

    fileOpenOptions = f;
    fileOpenOptions->Attach(this);
}

// ****************************************************************************
// Method: QvisPluginWindow::SubjectRemoved
//
// Purpose: 
//   This function is called when a subject is removed.
//
// Arguments:
//   TheRemovedSubject : The subject being removed.
//
// Programmer: Jeremy Meredith
// Creation:   January 23, 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisPluginWindow::SubjectRemoved(Subject *TheRemovedSubject)
{
    if (TheRemovedSubject == pluginAtts)
        pluginAtts = NULL;
    else if (TheRemovedSubject == fileOpenOptions)
        fileOpenOptions = NULL;
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
//    Jeremy Meredith, Wed Jan 23 15:37:24 EST 2008
//    Enabled contents in the database tab; for this tab, you can
//    not disable plugins, but you can set the default options.
//    Also, made the tab widget resize instead of the empty space
//    when the window grows.
//
//    Dave Pugmire, Wed Feb 13 15:43:24 EST 2008
//    Allow the ability enable/disable DB plugins. Add two buttons to select
//    unselect all. Provide an "X" next to the plugin if read options are available.
//
// ****************************************************************************

void
QvisPluginWindow::CreateWindowContents()
{
    // Create the tab widget.
    QTabWidget *tabs = new QTabWidget(central, "tabs");
    connect(tabs, SIGNAL(selected(const QString &)),
            this, SLOT(tabSelected(const QString &)));
    topLayout->addWidget(tabs,10000);

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
    pageDatabases->setSpacing(10);
    pageDatabases->setMargin(10);
    tabs->addTab(pageDatabases, "Databases");

    listDatabases = new QListView(pageDatabases, "listDatabases");
    listDatabases->addColumn("  ");
    listDatabases->addColumn("Name");
    listDatabases->addColumn("Open Options");
    listDatabases->setAllColumnsShowFocus(true);
    listDatabases->setColumnAlignment(0, Qt::AlignHCenter);
    listDatabases->setColumnAlignment(2, Qt::AlignLeft);

    QGroupBox *grpBox = new QGroupBox( 2, Horizontal, "Selection", pageDatabases );
    QHBox *box = new QHBox( grpBox, "hBox" );
    box->setSpacing(10);
    box->setMargin(10);

    // Add select all and unselect all buttons.
    selectAllReadersButton = new QPushButton("Select all", box );
    connect( selectAllReadersButton, SIGNAL(clicked()), this, SLOT(selectAllReadersButtonClicked()));
    unSelectAllReadersButton = new QPushButton("Unselect all", box );
    connect( unSelectAllReadersButton, SIGNAL(clicked()), this, SLOT(unSelectAllReadersButtonClicked()));
    
    databaseOptionsSetButton = new QPushButton("Set Default Open Options", pageDatabases);
    connect(databaseOptionsSetButton, SIGNAL(clicked()),
            this, SLOT(databaseOptionsSetButtonClicked()));
    connect(listDatabases, SIGNAL(selectionChanged(QListViewItem*)), 
            this, SLOT(databaseSelectedItemChanged(QListViewItem*)));

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
//    Jeremy Meredith, Wed Jan 23 15:38:11 EST 2008
//    Handle two subjects.
//
// ****************************************************************************

void
QvisPluginWindow::Update(Subject *s)
{
    QvisPostableWindowSimpleObserver::Update(s);
    if (s==pluginAtts)
    {
        if (!pluginsInitialized)
        {
            pluginsInitialized = true;
            emit pluginSettingsChanged();
        }
    }
    else if (s==fileOpenOptions)
    {
        // GUI got new file open options
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
//    Jeremy Meredith, Wed Jan 23 15:38:27 EST 2008
//    Handle two observed subjects, including the new database opening options.
//
//    Jeremy Meredith, Thu Jan 24 09:54:25 EST 2008
//    Forgot to add doAll as a condition to force updates.  Fixed now.
//
//    Cyrus Harrison, Mon Feb  4 09:46:24 PST 2008
//    Resolved AIX linking error w/ auto std::string to QString conversion.
//
//    Dave Pugmire, Wed Feb 13 15:43:24 EST 2008
//    Update the FileOpenOptions for enable/disable DB plugins.
//
// ****************************************************************************

void
QvisPluginWindow::UpdateWindow(bool doAll)
{
    int i;
    if (doAll || selectedSubject == pluginAtts)
    {
        listPlots->clear();
        listPlots->setSorting(1, true);
        plotIDs.clear();
        plotItems.clear();
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
        operatorIDs.clear();
        operatorItems.clear();
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

    if (doAll || selectedSubject == fileOpenOptions)
    {
        listDatabases->clear();
        listDatabases->setSorting(0,true);
        databaseItems.clear();
        databaseIndexes.clear();
        for (i=0; i<fileOpenOptions->GetNumOpenOptions(); i++)
        {
            QCheckListItem *item = new QCheckListItem(listDatabases, "", QCheckListItem::CheckBox);
            item->setOn(fileOpenOptions->GetEnabled()[i]);
            item->setText(1,fileOpenOptions->GetTypeNames()[i].c_str());
            if (fileOpenOptions->GetOpenOptions(i).GetNumberOfOptions() == 0)
                item->setText(2, "  " );
            else
                item->setText(2, "  X " );
            databaseItems.push_back(item);
            databaseIndexes.push_back(i);
        }
        databaseOptionsSetButton->setEnabled(false);
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
//    Jeremy Meredith, Wed Jan 23 15:38:46 EST 2008
//    Only issue the warning if the enabled/disabled plugins are changed.
//    Do updates for the default file opening options.
//
//    Dave Pugmire, Wed Feb 13 15:43:24 EST 2008
//    Update the FileOpenOptions for enable/disable DB plugins.
//
// ****************************************************************************

void
QvisPluginWindow::Apply(bool dontIgnore)
{
    bool dirty = false;
    int i;
    for (i=0; i<plotItems.size(); i++)
    {
        bool newvalue = plotItems[i]->isOn();
        int  &value =
            pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(plotIDs[i])];
        if (bool(value) != newvalue)
            dirty = true;
        value = newvalue;
    }
    for (i=0; i<operatorItems.size(); i++)
    {
        bool newvalue = operatorItems[i]->isOn();
        int  &value =
            pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(operatorIDs[i])];
        if (bool(value) != newvalue)
            dirty = true;
        value = newvalue;
    }

    for (i=0; i<databaseItems.size(); i++)
    {
        bool newvalue = databaseItems[i]->isOn();
        int &value = fileOpenOptions->GetEnabled()[i];
        if (bool(value) != newvalue)
            dirty = true;
        value = newvalue;
    }

    // Notify the viewer about the plugin enabled attributes
    pluginAtts->Notify();

    // And then about the new file opening options
    fileOpenOptions->Notify();
    GetViewerMethods()->SetDefaultFileOpenOptions();

    if (dirty)
    {
        GUIBase::Warning("Note:  Plugins are loaded at startup.  Please save "
                     "your settings and restart VisIt to apply your changes.");
    }
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
//    Jeremy Meredith, Wed Jan 23 15:39:32 EST 2008
//    Handle two observed subjects.
//   
// ****************************************************************************

void
QvisPluginWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

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
//    Jeremy Meredith, Wed Jan 23 15:39:32 EST 2008
//    Handle two observed subjects.
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
    QvisPostableWindowSimpleObserver::SetFromNode(parentNode, borders);
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


// ****************************************************************************
//  Method:  QvisPluginWindow::databaseOptionsSetButtonClicked
//
//  Purpose:
//    Action to take when the user tries to set the default file opening
//    options for the selected plugin.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************
void
QvisPluginWindow::databaseOptionsSetButtonClicked()
{
    QListViewItem *item = listDatabases->selectedItem();
    if (!item)
        return;

    int index = -1;
    for (int i=0; i<databaseItems.size(); i++)
    {
        if (item == databaseItems[i])
        {
            index = i;
            break;
        }
    }
    if (index == -1)
        return;

    DBOptionsAttributes &opts =
        fileOpenOptions->GetOpenOptions(databaseIndexes[index]);
    if (opts.GetNumberOfOptions() > 0)
    {
        QvisDBOptionsDialog *optsdlg = new QvisDBOptionsDialog(&opts, NULL,
                                                               "opts");
        QString caption = std::string("Default file opening options for " +
                                      fileOpenOptions->GetTypeNames()[index] +
                                      " reader").c_str();
        optsdlg->setCaption(caption);
        int result = optsdlg->exec();
        delete optsdlg;
        if (result == QDialog::Accepted)
        {
            // We don't need to do anything here:
            //  First, because we set it up so the options dialog sets the
            //  default file opening options if and only if they hit okay.
            //  And second, we wait to notify the viewer about changes
            //  until the user hits Apply.
        }
        else // rejected
        {
            // Again, nothing to do because the options dialog is a
            // complete no-op if the user hit Cancel.
        }
    }
}


// ****************************************************************************
//  Method:  QvisPluginWindow::databaseSelectedItemChanged
//
//  Purpose:
//    Set the enabled state of the button to set the default opening options
//    based on whether or not there are any options to set.
//
//  Arguments:
//    item       the newly highlighted item
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 23, 2008
//
// ****************************************************************************
void
QvisPluginWindow::databaseSelectedItemChanged(QListViewItem *item)
{
    for (int i=0; i<databaseItems.size(); i++)
    {
        if (item == databaseItems[i])
        {
            const DBOptionsAttributes &opts =
                fileOpenOptions->GetOpenOptions(databaseIndexes[i]);
            databaseOptionsSetButton->setEnabled(opts.GetNumberOfOptions()>0);
            break;
        }
    }
}

// ****************************************************************************
//  Method:  QvisPluginWindow::selectAllReadersButtonClicked()
//
//  Purpose:
//    Enable all the DB plugins.
//
//  Arguments:
//    none
//
//  Programmer:  Dave Pugmire
//  Creation:    February 13, 2008
//
// ****************************************************************************
void
QvisPluginWindow::selectAllReadersButtonClicked()
{
    for ( int i = 0; i < fileOpenOptions->GetEnabled().size(); i++ )
        fileOpenOptions->GetEnabled()[i] = true;

    UpdateWindow(false);
}

// ****************************************************************************
//  Method:  QvisPluginWindow::unSelectAllReadersButtonClicked()
//
//  Purpose:
//    Disable all the DB plugins.
//
//  Arguments:
//    none
//
//  Programmer:  Dave Pugmire
//  Creation:    February 13, 2008
//
// ****************************************************************************
void
QvisPluginWindow::unSelectAllReadersButtonClicked()
{
    for ( int i = 0; i < fileOpenOptions->GetEnabled().size(); i++ )
        fileOpenOptions->GetEnabled()[i] = false;

    UpdateWindow(false);
}
