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

#include <QvisPluginWindow.h>
#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>
#include <QWidget>
#include <QRadioButton>
#include <QTreeWidget>
#include <QHeaderView>

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
//    Brad Whitlock, Wed Apr  9 11:04:01 PDT 2008
//    QString for caption, shortName.
//
// ****************************************************************************

QvisPluginWindow::QvisPluginWindow(const QString &caption, const QString &shortName,
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
//    Brad Whitlock, Wed Nov 19 14:11:32 PST 2008
//    Detach if the subjects are not NULL.
//
// ****************************************************************************

QvisPluginWindow::~QvisPluginWindow()
{
    if(pluginAtts != 0)
        pluginAtts->Detach(this);
    if(fileOpenOptions != 0)
        fileOpenOptions->Detach(this);
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Thu Jul 10 13:58:23 PDT 2008
//    Fixed porting mistake and look and feel of tree widgets.
//
// ****************************************************************************

void
QvisPluginWindow::CreateWindowContents()
{
    // Create the tab widget.
    QTabWidget *tabs = new QTabWidget(central);
    
    connect(tabs, SIGNAL(currentChanged(int)),
            this, SLOT(tabSelected(int)));
            
    topLayout->addWidget(tabs,10000);

    //
    // Create the plot page
    //
    
    pagePlots = new QWidget(central);
    QVBoxLayout *plots_layout= new QVBoxLayout(pagePlots);
    tabs->addTab(pagePlots, tr("Plots"));

    listPlots = new QTreeWidget(pagePlots);
    listPlots->setRootIsDecorated(false);
    // add header item
    QStringList plotHeaders;
    plotHeaders << tr("Enabled") << tr("Name") << tr("Version");
    listPlots->setHeaderLabels(plotHeaders);
    listPlots->headerItem()->setTextAlignment(1, Qt::AlignHCenter);
    listPlots->headerItem()->setTextAlignment(2, Qt::AlignHCenter);
    listPlots->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    listPlots->header()->setResizeMode(1,QHeaderView::ResizeToContents);
    plots_layout->addWidget(listPlots);
    
    
    //
    // Create the operator page
    //
    pageOperators = new QWidget(central);
    QVBoxLayout *ops_layout= new QVBoxLayout(pageOperators);
    tabs->addTab(pageOperators, tr("Operators"));

    listOperators = new QTreeWidget(pageOperators);
    listOperators->setRootIsDecorated(false);

    QStringList operatorHeaders;
    operatorHeaders << tr("Enabled") << tr("Name") << tr("Version");
    listOperators->setHeaderLabels(operatorHeaders);
    listOperators->headerItem()->setTextAlignment(1, Qt::AlignHCenter);
    listOperators->headerItem()->setTextAlignment(2, Qt::AlignHCenter);
    listOperators->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    listOperators->header()->setResizeMode(1,QHeaderView::ResizeToContents);
    ops_layout->addWidget(listOperators);
    
    //
    // Create the database page
    //
    pageDatabases = new QWidget(central);
    QVBoxLayout *db_layout= new QVBoxLayout(pageDatabases);
    
    tabs->addTab(pageDatabases, tr("Databases"));

    listDatabases = new QTreeWidget(pageDatabases);
    listDatabases->setRootIsDecorated(false);

    QStringList dbHeaders;
    dbHeaders << tr("Enabled") << tr("Name") << tr("Open Options");
    listDatabases->setHeaderLabels(dbHeaders);
    listDatabases->headerItem()->setTextAlignment(1, Qt::AlignHCenter);
    listDatabases->headerItem()->setTextAlignment(2, Qt::AlignHCenter);
    listDatabases->header()->setResizeMode(0,QHeaderView::ResizeToContents);
    listDatabases->header()->setResizeMode(1,QHeaderView::ResizeToContents);
    db_layout->addWidget(listDatabases);

    QGroupBox *grpBox = new QGroupBox(central);
    QHBoxLayout *grp_layout = new QHBoxLayout(grpBox);

    // Add select all and unselect all buttons.
    selectAllReadersButton = new QPushButton(tr("Select all"), grpBox);
    connect(selectAllReadersButton, SIGNAL(clicked()),
            this, SLOT(selectAllReadersButtonClicked()));
    grp_layout->addWidget(selectAllReadersButton);

    unSelectAllReadersButton = new QPushButton(tr("Unselect all"), grpBox);
    connect(unSelectAllReadersButton, SIGNAL(clicked()),
            this, SLOT(unSelectAllReadersButtonClicked()));
    grp_layout->addWidget(unSelectAllReadersButton);
    
    databaseOptionsSetButton = new QPushButton(tr("Set Default Open Options"), grpBox);
    connect(databaseOptionsSetButton, SIGNAL(clicked()),
            this, SLOT(databaseOptionsSetButtonClicked()));
    connect(listDatabases, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), 
            this, SLOT(databaseSelectedItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    
    db_layout->addWidget(grpBox);
    db_layout->addWidget(databaseOptionsSetButton);
    
    // Show the appropriate page based on the activeTab setting.
    tabs->blockSignals(true);
    tabs->setCurrentIndex(activeTab);
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Tue Jul  8 16:03:44 PDT 2008
//    Fixed problem where database were shown in the operators tab.
//
// ****************************************************************************

void
QvisPluginWindow::UpdateWindow(bool doAll)
{
    int i;
    if (doAll || selectedSubject == pluginAtts)
    {
        listPlots->clear();
        listPlots->setSortingEnabled(true);
        listPlots->sortByColumn(1,Qt::AscendingOrder);
        plotIDs.clear();
        plotItems.clear();
        
        for (i=0; i<pluginAtts->GetName().size(); i++)
        {
            if (pluginAtts->GetType()[i] == "plot")
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(listPlots);
                item->setCheckState(0,pluginAtts->GetEnabled()[i] ? Qt::Checked : Qt::Unchecked);
                item->setText(1,pluginAtts->GetName()[i].c_str());
                item->setText(2,pluginAtts->GetVersion()[i].c_str());
                
                plotItems.push_back(item);
                plotIDs.push_back(pluginAtts->GetId()[i]);
            }
        }

        listOperators->clear();
        listOperators->setSortingEnabled(true);
        listOperators->sortByColumn(1,Qt::AscendingOrder);
        operatorIDs.clear();
        operatorItems.clear();
        for (i=0; i<pluginAtts->GetName().size(); i++)
        {
            if (pluginAtts->GetType()[i] == "operator")
            {
                QTreeWidgetItem *item = new QTreeWidgetItem(listOperators);
                item->setCheckState(0,pluginAtts->GetEnabled()[i] ? Qt::Checked : Qt::Unchecked);
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
        listDatabases->setSortingEnabled(true);
        listDatabases->sortByColumn(1,Qt::AscendingOrder);

        databaseItems.clear();
        databaseIndexes.clear();
        for (i=0; i<fileOpenOptions->GetNumOpenOptions(); i++)
        {
            
            QTreeWidgetItem *item = new QTreeWidgetItem(listDatabases);
            item->setCheckState(0,fileOpenOptions->GetEnabled()[i] ? Qt::Checked : Qt::Unchecked);
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
//   Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//   Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPluginWindow::Apply(bool dontIgnore)
{
    bool dirty = false;
    int i;
    for (i=0; i<plotItems.size(); i++)
    {
        bool newvalue = plotItems[i]->checkState(0);
        int  &value =
            pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(plotIDs[i])];
        if (bool(value) != newvalue)
            dirty = true;
        value = newvalue;
    }
    for (i=0; i<operatorItems.size(); i++)
    {
        bool newvalue = operatorItems[i]->checkState(0);
        int  &value =
            pluginAtts->GetEnabled()[pluginAtts->GetIndexByID(operatorIDs[i])];
        if (bool(value) != newvalue)
            dirty = true;
        value = newvalue;
    }

    for (i=0; i<databaseItems.size(); i++)
    {
        bool newvalue = databaseItems[i]->checkState(0);
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
        GUIBase::Warning(tr("Note:  Plugins are loaded at startup.  Please save "
                     "your settings and restart VisIt to apply your changes."));
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPluginWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

    if(saveWindowDefaults)
    {
        DataNode *node = parentNode->GetNode(windowTitle().toStdString());

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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPluginWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
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
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************

void
QvisPluginWindow::tabSelected(int tab)
{
    activeTab = tab;
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
//  Modifications:
//    Brad Whitlock, Tue Apr  8 15:26:49 PDT 2008
//    Support for internationalization.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisPluginWindow::databaseOptionsSetButtonClicked()
{
    QTreeWidgetItem *item = listDatabases->currentItem();
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
        QvisDBOptionsDialog *optsdlg = new QvisDBOptionsDialog(&opts, NULL);
        QString caption = tr("Default file opening options for %1 reader").
                          arg(fileOpenOptions->GetTypeNames()[index].c_str());
        optsdlg->setWindowTitle(caption);
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
//  Modifications:
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisPluginWindow::databaseSelectedItemChanged(QTreeWidgetItem *item,
                                              QTreeWidgetItem *prev_item)
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
//  Modifications:
//    Dave Pugmire, Thu Feb 14 14:08:50 EST 2008
//    Bug fix. The message to save settings and restart visit on Apply was not
//    being shown. Just set the checkbox state. No need to call update window.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisPluginWindow::selectAllReadersButtonClicked()
{
    for (int i=0; i<databaseItems.size(); i++)
        databaseItems[i]->setCheckState(0,Qt::Checked);
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
//  Modifications:
//    Dave Pugmire, Thu Feb 14 14:08:50 EST 2008
//    Bug fix. The message to save settings and restart visit on Apply was not
//    being shown. Just set the checkbox state. No need to call update window.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
// ****************************************************************************
void
QvisPluginWindow::unSelectAllReadersButtonClicked()
{
    for (int i=0; i<databaseItems.size(); i++)
        databaseItems[i]->setCheckState(0,Qt::Unchecked);
}
