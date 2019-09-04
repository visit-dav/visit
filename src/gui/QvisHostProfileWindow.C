/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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

#include <QvisHostProfileWindow.h>

#include <visit-config.h>

#include <QAbstractTableModel>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFileInfo>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QUrl>
#include <QWidget>

#include <snprintf.h>

#include <XMLNode.h>
#include <SingleAttributeConfigManager.h>
#include <MachineProfile.h>
#include <LaunchProfile.h>
#include <HostProfileList.h>
#include <StringHelpers.h>
#include <ViewerProxy.h>
#include <InstallationFunctions.h>

#include <rapidjson/document.h>

#include <cstdlib>

#include <string>

#define HOST_PROFILE_SPACING 5

// The share batch job option has some issues presently.
#define SHARE_BATCH_JOB_HOST_ISSUES

// ****************************************************************************
// Method: QvisHostProfileWindow::QvisHostProfileWindow
//
// Purpose: 
//   This is the constructor for the QvisHostProfileWindow class.
//
// Arguments:
//   profiles : A pointer to the HostProfileList that the window
//              will observe.
//   caption  : The title of the window in the window decoration.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:15:44 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed May 2 11:43:12 PDT 2001
//   Made the window postable.
//
//   Brad Whitlock, Fri Feb 15 15:11:02 PST 2002
//   Initialized parentless widgets.
//
//   Brad Whitlock, Thu Feb 21 10:17:24 PDT 2002
//   I removed user name initialization.
//
//   Brad Whitlock, Wed Jun 6 09:35:57 PDT 2007
//   Removed a button group.
//
//   Brad Whitlock, Wed Apr  9 10:57:35 PDT 2008
//   QString for caption, shortName.
//
//   Jeremy Meredith, Thu Feb 18 15:54:50 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************

QvisHostProfileWindow::QvisHostProfileWindow(HostProfileList *profiles,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(profiles, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    profileCounter = 0;
    currentMachine = NULL;
    currentLaunch = NULL;

    remoteUrl = NULL;
    remoteTree = NULL;
    manager = NULL;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::~QvisHostProfileWindow
//
// Purpose: 
//   This is the destructor for the QvisHostProfileWindow class.
//
// Programmer: 
// Creation:   
//
// Modifications:
//
// ****************************************************************************

QvisHostProfileWindow::~QvisHostProfileWindow()
{
}

// ****************************************************************************
// Method: DropListWidget
//
// Purpose:
//   TODO: Remove this class as soon as the eventFilter works
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

class DropListWidget : public QListWidget
{
    QvisHostProfileWindow* window;
    friend class QvisHostProfileWindow;
public:

    DropListWidget(QWidget* parent) : QListWidget(parent){
    }
    ~DropListWidget() {}

    void dropEvent(QDropEvent *event){
        window->ListWidgetDropEvent(event);
    }
};

// ****************************************************************************
// Method: QvisHostProfileWindow::addChildren
//
// Purpose:
//   Find all paths that need to get copied into hostList
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::addChildren(const QModelIndex& list, QStringList& suffixList, QStringList& globalList)
{
    if(!remoteTree->model()->hasChildren(list)) {
        QString gList = "";
        for(int i = 0; i < suffixList.size(); ++i)
            gList += suffixList[i] + (i == suffixList.size()-1 ? "" : "/");
        globalList.push_back(gList);
        return;
    }

    for(int i = 0; i < remoteTree->model()->rowCount(list); ++i) {
        QModelIndex child = remoteTree->model()->index(i,0, list);
        suffixList.push_back(child.data().toString());
        addChildren(child, suffixList, globalList);
        suffixList.pop_back();
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::ListWidgetDropEvent
//
// Purpose:
//   DropEvent
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::selectProfiles(const QModelIndex& index)
{
    /// create prefix..
    QString prefix = index.data(Qt::DisplayRole).toString();

    QModelIndex parent = index.parent();

    while(parent.isValid())
    {
        prefix = parent.data(Qt::DisplayRole).toString() + "/" + prefix;
        parent = parent.parent();
    }

    /// add children..
    QStringList globalList;
    if(remoteTree->model()->hasChildren())
    {
        QStringList suffixList;
        suffixList.push_back(prefix);
        addChildren(index,suffixList,globalList);
    }
    else
    {
        globalList.push_back(prefix);
    }

    for(int x = 0; x < globalList.size(); ++x)
    {

        QString machine = globalList[x];

        if(!remoteData.contains(machine)) {
            /// std::cout << "Remote data not found: " << machine.toStdString() << std::endl;
            continue;
        }

        std::istringstream str(remoteData[machine].toStdString().c_str());

        MachineProfile mp;
        SingleAttributeConfigManager sac(&mp);
        sac.Import(str);

        bool skip = false;
        for(int y = 0; y < hostList->count(); ++y) {
            std::string inputName = hostList->item(y)->data(Qt::DisplayRole).toString().toStdString();

            if(inputName.length() == 0) continue;
            if(inputName == mp.GetHostNickname() || inputName == mp.GetHost())
            {
                int result = QMessageBox::question(this,
                                      tr("Duplicate detected"),
                                      tr("Entry already exists for %1. Allow Duplicate?").arg(inputName.c_str()),
                                      QMessageBox::No, QMessageBox::Yes);

                if(result == QMessageBox::No)
                    skip = true;
                break;
            }
        }

        /// user chose to skip duplicate entry..
        if(skip){
            continue;
        }

        HostProfileList* profiles = (HostProfileList*)subject;
        mp.SelectAll();
        profiles->AddMachines(mp);
        profiles->Notify();
    }
}


void
QvisHostProfileWindow::selectProfiles()
{
    QModelIndexList list = remoteTree->selectionModel()->selectedRows();

    for(int i = 0; i < list.size(); ++i) {
        selectProfiles(list[i]);
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::ListWidgetDropEvent
//
// Purpose:
//   DropEvent
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::ListWidgetDropEvent(QDropEvent *event)
{
    /// ensure event source is from remoteTree
    if(event->source() != remoteTree) return;

    QByteArray encoded = event->mimeData()->data("application/x-qabstractitemmodeldatalist");

    QDataStream stream(&encoded, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
        int row, col;
        QMap<int,  QVariant> roleDataMap;
        stream >> row >> col >> roleDataMap;

        QModelIndexList list = remoteTree->model()->match(remoteTree->model()->index(0,0), Qt::DisplayRole,
                                                          roleDataMap[Qt::DisplayRole], 1,
                                                          Qt::MatchExactly |
                                                          Qt::MatchWrap |
                                                          Qt::MatchRecursive);
        for(int i = 0; i < list.size(); ++i) {
            selectProfiles(list[i]);
        }
    }

    event->acceptProposedAction();
}


// ****************************************************************************
// Method: QvisHostProfileWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets and hooks up the slot
//   methods.
//
// Programmer: Jeremy Meredith, Thu Feb 18 15:56:00 EST 2010
// Creation:   February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:18 EST 2010
//   Changed order of two main tabs.
//
//   Hank Childs, Thu Nov 17 18:49:08 PST 2011
//   Rename New/Copy/Delete to New Host/Copy Host/Delete Host.
//   This removes ambiguity with profile controls.
//
//   Brad Whitlock, Wed Oct 22 15:46:11 PDT 2014
//   Rearrange some buttons so the window is less wide.
//
// ****************************************************************************

void
QvisHostProfileWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(NULL);
    topLayout->addLayout(mainLayout);
    topLayout->setStretchFactor(mainLayout, 100);

    mainLayout->addWidget(new QLabel(tr("Hosts")), 0,0, 1,2);
    hostList = new DropListWidget(central);
    hostList->setSortingEnabled(true);
    mainLayout->addWidget(hostList, 1,0, 1,2);
    connect(hostList, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentHostChanged()));

    hostList->setDropIndicatorShown(true);
    hostList->setDragDropMode(QAbstractItemView::DropOnly);
    hostList->setDefaultDropAction(Qt::CopyAction);
    hostList->setAcceptDrops(true);

    addHost = new QPushButton(tr("New Host"), central);
    mainLayout->addWidget(addHost, 2,0);
    connect(addHost, SIGNAL(clicked()), this, SLOT(addMachineProfile()));

    delHost = new QPushButton(tr("Delete Host"), central);
    mainLayout->addWidget(delHost, 2,1);
    connect(delHost, SIGNAL(clicked()), this, SLOT(delMachineProfile()));

    copyHost = new QPushButton(tr("Copy Host"), central);
    mainLayout->addWidget(copyHost, 3,0);
    connect(copyHost, SIGNAL(clicked()), this, SLOT(copyMachineProfile()));

    exportHost = new QPushButton(tr("Export Host"), central);
    mainLayout->addWidget(exportHost, 3,1);
    connect(exportHost, SIGNAL(clicked()), this, SLOT(exportMachineProfile()));

    QTabWidget* masterWidget = new QTabWidget(central);
    masterWidget->setTabPosition(QTabWidget::West);
    mainLayout->addWidget(masterWidget, 0,2, 4,1);

    machineTabs = new QTabWidget();
    masterWidget->addTab(machineTabs, tr("Machines"));

    machineSettingsGroup = CreateMachineSettingsGroup();
    machineTabs->addTab(machineSettingsGroup, tr("Host Settings"));

    launchProfilesGroup = CreateLaunchProfilesGroup();
    machineTabs->addTab(launchProfilesGroup, tr("Launch Profiles"));

    remoteProfilesGroup = CreateRemoteProfilesGroup();
    masterWidget->addTab(remoteProfilesGroup, tr("Remote Profiles"));

    ((DropListWidget*)hostList)->window = this;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateRemoteProfilesGroup
//
// Purpose:
//   Create the Remote Profiles Interface
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//    Kathleen Biagas, Thu Nov 15 12:39:41 PST 2018
//    Add current version's URL to remoteUrl combobox if not running from a
//    development version.
//
//    Kathleen Biagas, Tue Sep  3 20:07:10 PDT 2019
//    Change remote URL to github.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateRemoteProfilesGroup()
{
    QWidget* currentGroup = new QWidget();

    QGridLayout *gridLayout;
    QPushButton *pushButton;
    QPushButton *importButton;

    gridLayout = new QGridLayout(currentGroup);
    pushButton = new QPushButton(currentGroup);
    importButton = new QPushButton(currentGroup);

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(pushButton->sizePolicy().hasHeightForWidth());


    remoteUrl = new QComboBox(currentGroup);
    //remoteUrl->setEditable(true);
    gridLayout->addWidget(remoteUrl, 0, 0, 1, 1);

    pushButton->setSizePolicy(sizePolicy);
    pushButton->setText(tr("Update"));

    gridLayout->addWidget(pushButton, 0, 1, 1, 1);

    importButton->setSizePolicy(sizePolicy);
    importButton->setText(tr("Import"));

    gridLayout->addWidget(importButton, 0, 2, 1, 1);

    /// todo: use a configuration API to load remote url..
    if (!GetIsDevelopmentVersion())
    {
        QString relURL, ver(VISIT_VERSION);
        if(ver.right(1) == "b")
            ver = ver.left(ver.length()-1);
        relURL = QString("https://raw.githubusercontent.com/visit-dav/visit/v") + ver + QString("/src/resources/hosts/");
        remoteUrl->addItem(relURL);
    }
    remoteUrl->addItem("https://raw.githubusercontent.com/visit-dav/visit/develop/src/resources/hosts/");
    remoteUrl->setCurrentIndex(0);

    remoteTree = new QTreeWidget(currentGroup);
    remoteTree->setAlternatingRowColors(true);
    remoteTree->setHeaderLabel(tr("Profiles"));
    remoteTree->setDragEnabled(true);
    remoteTree->setDragDropMode(QAbstractItemView::DragOnly);
    remoteTree->setSortingEnabled(true);
    remoteTree->sortByColumn(0, Qt::AscendingOrder);

    gridLayout->addWidget(remoteTree, 1, 0, 1, 3);

    connect(pushButton, SIGNAL(clicked()), this, SLOT(retrieveLatestProfiles()));
    connect(importButton, SIGNAL(clicked()), this, SLOT(selectProfiles()));

    return currentGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::retrieveLatestProfiles
//
// Purpose:
//   Retrieve latest set of profiles from remote URL
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::retrieveLatestProfiles()
{
    ///get content from url..
    QUrl url(remoteUrl->currentText());

    if(!manager) {
        manager = new QNetworkAccessManager();

        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(downloadHosts(QNetworkReply*)));
    }

    remoteTree->clear();
    remoteData.clear();

    QNetworkRequest maprequest(QUrl(remoteUrl->currentText() + "/networks.dat"));
    QNetworkReply* reply = manager->get(maprequest);
    reply->waitForReadyRead(-1);

    QNetworkRequest request(url);
    manager->get(request);
}

// ****************************************************************************
// Method: QvisHostProfileWindow::addRemoteProfile
//
// Purpose:
//  Add Remote Profile to User Interface.
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::addRemoteProfile(const QString& inputUrl, const QString &results)
{
    /// remove prefix..
    QString iurl = inputUrl;
    QString treePath = iurl.remove(remoteUrl->currentText());

    QStringList treeList = treePath.split("/");

    if(treeList.size() > 0) {
        QTreeWidgetItem* item = 0;

        QString key = "";
        for(int index = 0; index < treeList.size(); ++index)
        {
            QString path = treeList[index].trimmed();

            if(path.length() == 0) continue;

            if(profileMap.contains(path))
                path = profileMap[path];

            if(key == "")
                key = path;
            else
                key += "/" + path;

            if(item == NULL)
            {
                for(int x = 0; x < remoteTree->topLevelItemCount(); ++x)
                {
                    if(remoteTree->topLevelItem(x)->text(0) == path) {
                        item = remoteTree->topLevelItem(x);
                        break;
                    }
                }
                if(!item)
                    item = new QTreeWidgetItem(remoteTree, QStringList() << path);
            }
            else
            {
                QTreeWidgetItem* child = 0;
                for(int x = 0; x < item->childCount(); ++x)
                {
                    if(item->child(x)->text(0) == path) {
                        child = item->child(x);
                        break;
                    }
                }

                if(!child)
                    child = new QTreeWidgetItem(item, QStringList() << path);
                item = child;
            }
        }
        remoteData[key] = results;
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::downloadHosts
//
// Purpose:
//   Download results from URL
//
// Programmer:
// Creation:   September 10, 2013
//
// Modifications:
//    Kathleen Biagas, Tue Sep  3 20:08:13 PDT 2019
//    Use networks.json file to parse for locations of .xml files.
//
// ****************************************************************************

void
QvisHostProfileWindow::downloadHosts(QNetworkReply *reply)
{
    QString results(reply->readAll());

    QString inputUrl = reply->url().toString();

    /// this is the result within an xml file..
    /// parse and store this entry.
    if(QFileInfo(inputUrl).fileName() == "networks.dat")
    {
        /// if the network has a mapping structure..
        QStringList maplist = results.split("\n");
        foreach(const QString& mp, maplist)
        {
            int mpindex = mp.indexOf(":");
            if(mpindex < 0) continue;

            QString values = mp.mid(0, mpindex);
            QString key = mp.mid(mpindex+1, mp.length() - mpindex);

            profileMap[key] = values;
        }
        // change the url to retieve hosts directory structure from json file
        QNetworkRequest request(QUrl(remoteUrl->currentText() + "/networks.json"));
        manager->get(request);
    }
    else if(QFileInfo(inputUrl).fileName() == "networks.json")
    {
        rapidjson::Document d;
        if(d.Parse<0>(results.toStdString().c_str()).HasParseError())
        {
            return;
        }
        if (d.IsObject() && d.HasMember("hosts"))
        {
            rapidjson::Value &hostsRoot = d["hosts"];
            for(rapidjson::SizeType i = 0; i < hostsRoot.Size(); ++i)
            {
                QString network(hostsRoot[i]["network"].GetString());
                if (hostsRoot[i].HasMember("files"))
                {
                    rapidjson::Value &hostDir = hostsRoot[i]["files"];
                    for(rapidjson::SizeType j = 0; j < hostDir.Size(); ++j)
                    {
                        QString fileN(hostDir[j]["name"].GetString());
                        QNetworkRequest request(QUrl(remoteUrl->currentText() + "/" + network + "/" + fileN));
                        manager->get(request);
                    }
                }
            }
        }
    }
    else if(inputUrl.contains(".xml"))
    {
        addRemoteProfile(inputUrl, results);
    }
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateMachineSettingsGroup
//
// Purpose:
//   Creates the main settings (not launch profiles) for a machine profile.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Eric Brugger, Tue Apr 19 20:39:50 PDT 2011
//   I added the ability to specify a gateway machine to use to get to the
//   remote host.
//
//   Brad Whitlock, Thu Oct  6 10:55:59 PDT 2011
//   I added max nodes and max processors.
//
//   Brad Whitlock, Wed Aug 15 13:58:14 PDT 2012
//   I added ssh command.
//
//    Kathleen Biagas, Wed Dec 16 11:07:43 MST 2015
//    Replace slot 'sshCommandChanged' with 'sshCommandRetPressed', so that
//    sshCommand is only processed once editing has finished.  Will be
//    triggered by 'returnPressed'/'editingFinished' signals from the widget.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateMachineSettingsGroup()
{
    QWidget *currentGroup = new QWidget();

    QVBoxLayout *layout = new QVBoxLayout(currentGroup);
    layout->setMargin(5);

    //
    // Machine group
    //
    QGroupBox *machineGroup = new QGroupBox(tr("Machine"), currentGroup);
    layout->addWidget(machineGroup);
    int mRow = 0;
    QGridLayout *mLayout = new QGridLayout(machineGroup);
    mLayout->setMargin(5);
    mLayout->setSpacing(HOST_PROFILE_SPACING);

    hostNickname = new QLineEdit(machineGroup);
    connect(hostNickname, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostNicknameChanged(const QString &)));
    hostNicknameLabel = new QLabel(tr("Host nickname"), machineGroup);
    mLayout->addWidget(hostNicknameLabel, mRow, 0);
    mLayout->addWidget(hostNickname, mRow, 1);
    mRow++;

    hostName = new QLineEdit(machineGroup);
    connect(hostName, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostNameChanged(const QString &)));
    hostNameLabel = new QLabel(tr("Remote host name"), machineGroup);
    mLayout->addWidget(hostNameLabel, mRow, 0);
    mLayout->addWidget(hostName, mRow, 1);
    mRow++;

    hostAliases = new QLineEdit(machineGroup);
    connect(hostAliases, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostAliasesChanged(const QString &)));
    hostAliasesLabel = new QLabel(tr("Host name aliases"), machineGroup);
    mLayout->addWidget(hostAliasesLabel, mRow, 0);
    mLayout->addWidget(hostAliases, mRow, 1);
    mRow++;

    QFrame *sep1 = new QFrame(machineGroup);
    sep1->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    mLayout->addWidget(sep1, mRow, 0, 1, 2);
    mRow++;

    maxNodesCheckBox = new QCheckBox(tr("Maximum nodes"), machineGroup);
    connect(maxNodesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleUseMaxNodes(bool)));
    maxNodes = new QSpinBox(machineGroup);
    maxNodes->setKeyboardTracking(false);
    maxNodes->setRange(1, 1000000);
    connect(maxNodes, SIGNAL(valueChanged(int)),
            this, SLOT(maxNodesChanged(int)));
    mLayout->addWidget(maxNodesCheckBox, mRow, 0);
    mLayout->addWidget(maxNodes, mRow, 1);
    mRow++;

    maxProcessorsCheckBox = new QCheckBox(tr("Maximum processors"), machineGroup);
    connect(maxProcessorsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleUseMaxProcessors(bool)));
    maxProcessors = new QSpinBox(machineGroup);
    maxProcessors->setKeyboardTracking(false);
    maxProcessors->setRange(1, 16000000);
    connect(maxProcessors, SIGNAL(valueChanged(int)),
            this, SLOT(maxProcessorsChanged(int)));
    mLayout->addWidget(maxProcessorsCheckBox, mRow, 0);
    mLayout->addWidget(maxProcessors, mRow, 1);
    mRow++;

    QFrame *sep2 = new QFrame(machineGroup);
    sep2->setFrameStyle(QFrame::HLine + QFrame::Sunken);
    mLayout->addWidget(sep2, mRow, 0, 1, 2);
    mRow++;

    directory = new QLineEdit(machineGroup);
    connect(directory, SIGNAL(textChanged(const QString &)),
            this, SLOT(processDirectoryText(const QString &)));
    directoryLabel = new QLabel(tr("Path to VisIt installation"),
                                      machineGroup);
    mLayout->addWidget(directoryLabel, mRow, 0);
    mLayout->addWidget(directory, mRow, 1);
    mRow++;

    //
    // Account group
    //
    QGroupBox *accountGroup = new QGroupBox(tr("Account"), currentGroup);
    layout->addWidget(accountGroup);
    int aRow = 0;
    QGridLayout *aLayout = new QGridLayout(accountGroup);
    aLayout->setMargin(5);
    aLayout->setSpacing(HOST_PROFILE_SPACING);

    userName = new QLineEdit(accountGroup);
    connect(userName, SIGNAL(textChanged(const QString &)),
            this, SLOT(userNameChanged(const QString &)));
    userNameLabel = new QLabel(tr("Username"), accountGroup);
    aLayout->addWidget(userNameLabel, aRow, 0);
    aLayout->addWidget(userName, aRow, 1);
    aRow++;

    //
    // Connection group
    //
    QGroupBox *connectionGroup = new QGroupBox(tr("Connection"), currentGroup);
    layout->addWidget(connectionGroup);
    int cRow = 0;
    QGridLayout *cLayout = new QGridLayout(connectionGroup);
    cLayout->setMargin(5);
    cLayout->setSpacing(HOST_PROFILE_SPACING);
    cLayout->setColumnMinimumWidth(0,15);
    cLayout->setColumnStretch(0,0);
    cLayout->setColumnStretch(1,0);
    cLayout->setColumnStretch(2,0);
    cLayout->setColumnStretch(3,100);

    shareMDServerCheckBox = new QCheckBox(tr("Share batch job with Metadata Server"),
                                          connectionGroup);
    cLayout->addWidget(shareMDServerCheckBox, cRow, 0, 1, 4);
    connect(shareMDServerCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleShareMDServer(bool)));
    cRow++;

    tunnelSSH = new QCheckBox(tr("Tunnel data connections through SSH"), connectionGroup);
    cLayout->addWidget(tunnelSSH, cRow,0, 1,4);
    connect(tunnelSSH, SIGNAL(toggled(bool)),
            this, SLOT(toggleTunnelSSH(bool)));
    cRow++;

    clientHostNameMethod = new QButtonGroup(connectionGroup);
    connect(clientHostNameMethod, SIGNAL(buttonClicked(int)),
            this, SLOT(clientHostNameMethodChanged(int)));
    chnMachineName = new QRadioButton(tr("Use local machine name"), connectionGroup);
    chnParseFromSSHClient = new QRadioButton(tr("Parse from SSH_CLIENT environment variable"),
                                             connectionGroup);
    chnSpecifyManually = new QRadioButton(tr("Specify manually:"),
                                          connectionGroup);
    chnMachineName->setChecked(true);
    clientHostNameMethod->addButton(chnMachineName,0);

    clientHostNameMethod->addButton(chnParseFromSSHClient,1);
    clientHostNameMethod->addButton(chnSpecifyManually,2);
    clientHostNameMethodLabel =
        new QLabel(tr("Method used to determine local host name when not tunneling:"),
                   connectionGroup);
    cLayout->addWidget(clientHostNameMethodLabel,
                                  cRow, 0, 1, 4);
    cRow++;
    cLayout->addWidget(chnMachineName, cRow, 1, 1, 3);
    cRow++;
    cLayout->addWidget(chnParseFromSSHClient, cRow, 1, 1, 3);
    cRow++;
    cLayout->addWidget(chnSpecifyManually, cRow, 1, 1, 1);
    
    clientHostName = new QLineEdit(connectionGroup);
    connect(clientHostName, SIGNAL(textChanged(const QString &)),
            this, SLOT(clientHostNameChanged(const QString &)));
    cLayout->addWidget(clientHostName, cRow, 2, 1,2);
    cRow++;

    sshCommand = new QLineEdit(connectionGroup);
    sshCommandCheckBox = new QCheckBox(tr("SSH command"), connectionGroup);
    connect(sshCommand, SIGNAL(returnPressed()),
            this, SLOT(sshCommandRetPressed()));
    connect(sshCommand, SIGNAL(editingFinished()),
            this, SLOT(sshCommandRetPressed()));
    connect(sshCommandCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSSHCommand(bool)));
    cLayout->addWidget(sshCommandCheckBox, cRow, 0, 1, 2);
    cLayout->addWidget(sshCommand, cRow, 2, 1, 2);
    cRow++;

    sshPort = new QLineEdit(connectionGroup);
    sshPortCheckBox = new QCheckBox(tr("SSH port"), connectionGroup);
    connect(sshPort, SIGNAL(textChanged(const QString &)),
            this, SLOT(sshPortChanged(const QString &)));
    connect(sshPortCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSSHPort(bool)));
    cLayout->addWidget(sshPortCheckBox, cRow, 0, 1, 2);
    cLayout->addWidget(sshPort, cRow, 2, 1, 2);
    cRow++;

    gatewayHost = new QLineEdit(connectionGroup);
    useGatewayCheckBox = new QCheckBox(tr("Use gateway"), connectionGroup);
    connect(gatewayHost, SIGNAL(textChanged(const QString &)),
            this, SLOT(gatewayHostChanged(const QString &)));
    connect(useGatewayCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleUseGateway(bool)));
    cLayout->addWidget(useGatewayCheckBox, cRow, 0, 1, 2);
    cLayout->addWidget(gatewayHost, cRow, 2, 1, 2);
    cRow++;

    layout->addStretch(5);

    return currentGroup;
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateLaunchProfilesGroup
//
// Purpose:
//   Creates the launch profiles tab to list profiles for a machine.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Brad Whitlock, Thu Oct  6 12:00:57 PDT 2011
//   I combined a couple tabs into the parallel tab.
//
//   Hank Childs, Thu Nov 17 18:49:08 PST 2011
//   Rename New/Copy/Delete to New Profile/Copy Profile/Delete Profile.
//   This removes ambiguity with host controls.
//
//   David Camp, Mon Aug  4 10:46:09 PDT 2014
//   Set the minimum height of the tab to keep all 3 tabs the same.
//   The GPU tab would drop the tab height down.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateLaunchProfilesGroup()
{
    QWidget *currentGroup = new QWidget();

    int row = 0;
    QGridLayout *layout = new QGridLayout(currentGroup);
    layout->setMargin(5);

    profileList = new QListWidget(currentGroup);
    layout->addWidget(profileList, row,0, 1,4);
    connect(profileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentLaunchChanged()));
    row++;

    addProfile = new QPushButton(tr("New Profile"), currentGroup);
    layout->addWidget(addProfile, row,0);
    connect(addProfile, SIGNAL(clicked()), this, SLOT(addLaunchProfile()));
    delProfile = new QPushButton(tr("Delete Profile"), currentGroup);
    connect(delProfile, SIGNAL(clicked()), this, SLOT(delLaunchProfile()));
    layout->addWidget(delProfile, row,1);
    copyProfile = new QPushButton(tr("Copy Profile"), currentGroup);
    connect(copyProfile, SIGNAL(clicked()), this, SLOT(copyLaunchProfile()));
    layout->addWidget(copyProfile, row,2);
    makeDefaultProfile = new QPushButton(tr("Make Default"),
                                         currentGroup);
    connect(makeDefaultProfile, SIGNAL(clicked()), this, SLOT(makeDefaultLaunchProfile()));
    layout->addWidget(makeDefaultProfile, row,3);
    row += 2;

    profileTabs = new QTabWidget(central);
    profileTabs->setMinimumHeight( 308 );
    layout->addWidget(profileTabs, row,0, 1,4);
    row++;

    basicSettingsGroup = CreateBasicSettingsGroup();
    profileTabs->addTab(basicSettingsGroup, tr("Settings"));

    parallelSettingsGroup = CreateParallelSettingsGroup();
    profileTabs->addTab(parallelSettingsGroup, tr("Parallel"));

    hwAccelSettingsGroup = CreateHWAccelSettingsGroup();
    profileTabs->addTab(hwAccelSettingsGroup, tr("GPU Acceleration"));

    return currentGroup;
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateBasicSettingsGroup
//
// Purpose:
//   Creates the basic settings tab for a launch profile.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Brad Whitlock, Thu Oct  6 11:49:38 PDT 2011
//   I moved the parallel check box to the parallel tab.
//
//   David Camp, Mon Aug  4 10:46:09 PDT 2014
//   Added the threads option.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateBasicSettingsGroup()
{
    QWidget *currentGroup = new QWidget();

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    tmpLayout->setMargin(0);
    QGridLayout *layout = new QGridLayout();
    layout->setMargin(5);
    tmpLayout->addLayout(layout);
    layout->setSpacing(7);
    tmpLayout->addStretch(5);
    
    profileName = new QLineEdit(currentGroup);
    connect(profileName, SIGNAL(textChanged(const QString&)),
            this, SLOT(processProfileNameText(const QString&)));
    profileNameLabel = new QLabel(tr("Profile name"),currentGroup);
    layout->addWidget(profileNameLabel, row, 0);
    layout->addWidget(profileName, row, 1, 1,3);
    row++;

    timeout = new QSpinBox(currentGroup);
    timeout->setKeyboardTracking(false);
    timeout->setRange(1, 1440);
    timeout->setSingleStep(1);
    connect(timeout, SIGNAL(valueChanged(int)),
            this, SLOT(timeoutChanged(int)));
    timeoutLabel = new QLabel(tr("Timeout (minutes)"), currentGroup);
    layout->addWidget(timeoutLabel, row, 0, 1,1);
    layout->addWidget(timeout, row, 1, 1,3);
    row++;

    threads = new QSpinBox(currentGroup);
    threads->setKeyboardTracking(false);
    threads->setRange(0, 256);
    threads->setSingleStep(1);
    connect(threads, SIGNAL(valueChanged(int)),
            this, SLOT(threadsChanged(int)));
    threadsLabel = new QLabel(tr("Number of threads per task"), currentGroup);
    layout->addWidget(threadsLabel, row, 0, 1,1);
    layout->addWidget(threads, row, 1, 1,3);
    row++;

    engineArguments = new QLineEdit(currentGroup);
    connect(engineArguments, SIGNAL(textChanged(const QString &)),
            this, SLOT(processEngineArgumentsText(const QString &)));
    engineArgumentsLabel = new QLabel(tr("Additional arguments"),
                                      currentGroup);
    layout->addWidget(engineArgumentsLabel, row, 0, 1,1);
    layout->addWidget(engineArguments, row, 1, 1,3);
    row++;

    return currentGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateParallelSettingsGroup
//
// Purpose: 
//   Create the parallel options together on this tab.
//
// Returns:    The widget that contains the parallel options.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  6 11:59:12 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateParallelSettingsGroup()
{
    QWidget *currentGroup = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(currentGroup);
    layout->setMargin(5);

    parallelCheckBox = new QCheckBox(
                     tr("Launch parallel engine"),
                                     currentGroup);
    connect(parallelCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleParallel(bool)));
    layout->addWidget(parallelCheckBox);

    parallelTabs = new QTabWidget(currentGroup);
    layout->addWidget(parallelTabs);

    parallelTabs->addTab(CreateLaunchSettingsGroup(), tr("Launch"));
    parallelTabs->addTab(CreateAdvancedSettingsGroup(), tr("Advanced"));
    parallelTabs->setCurrentIndex(0);

    return currentGroup;
}


// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateLaunchSettingsGroup
//
// Purpose:
//   Creates the main job control tab for a launch profile.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Hank Childs, Sat Mar 13 20:21:17 PST 2010
//   Add support for salloc.
//
//   Eric Brugger, Tue Apr 19 17:40:38 PDT 2011
//   Added support for aprun and msub/aprun.
//
//   Eric Brugger, Thu May 12 16:12:59 PDT 2011
//   Added support for sbatch/mpiexec.
//
//   Brad Whitlock, Thu Oct  6 11:56:32 PDT 2011
//   Return the created widget.
//
//   Brad Whitlock, Wed Nov 30 09:15:35 PST 2011
//   Add Windows HPC.
//
//   Brad Whitlock, Thu Mar 15 11:29:01 PDT 2012
//   Add sbatch/aprun.
//
//   Brad Whitlock, Wed Nov  7 10:53:27 PST 2018
//   Add lrun.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateLaunchSettingsGroup()
{
    QWidget *currentGroup = new QWidget();

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    tmpLayout->setMargin(5);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(HOST_PROFILE_SPACING+2);
    tmpLayout->addStretch(5);
    
    launchMethod = new QComboBox(currentGroup);
    launchMethod->addItem(tr("(default)"));
    launchMethod->addItem("aprun");
    launchMethod->addItem("bsub");
    launchMethod->addItem("bsub/mpirun");
    launchMethod->addItem("dmpirun");
    launchMethod->addItem("ibrun");
    launchMethod->addItem("mpiexec");
    launchMethod->addItem("mpirun");
    launchMethod->addItem("msub");
    launchMethod->addItem("poe");
    launchMethod->addItem("prun");
    launchMethod->addItem("psub");
    launchMethod->addItem("salloc");
    launchMethod->addItem("srun");
    launchMethod->addItem("WindowsHPC");
    launchMethod->addItem("yod");
    launchMethod->addItem("lrun");
    launchMethod->addItem("msub/aprun");
    launchMethod->addItem("msub/srun");
    launchMethod->addItem("psub/mpirun");
    launchMethod->addItem("psub/poe");
    launchMethod->addItem("psub/srun");
    launchMethod->addItem("qsub/aprun");
    launchMethod->addItem("qsub/ibrun");
    launchMethod->addItem("qsub/mpiexec");
    launchMethod->addItem("qsub/mpirun");
    launchMethod->addItem("qsub/srun");
    launchMethod->addItem("sbatch/aprun");
    launchMethod->addItem("sbatch/ibrun");
    launchMethod->addItem("sbatch/mpiexec");
    launchMethod->addItem("sbatch/mpirun");
    launchMethod->addItem("sbatch/srun");
    connect(launchMethod, SIGNAL(activated(const QString &)),
            this, SLOT(launchMethodChanged(const QString &)));
    launchCheckBox = new QCheckBox(tr("Parallel launch method"), currentGroup);
    connect(launchCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleLaunch(bool)));
    layout->addWidget(launchCheckBox, row, 0);
    layout->addWidget(launchMethod, row, 1);
    row++;

    partitionName = new QLineEdit(currentGroup);
    connect(partitionName, SIGNAL(textChanged(const QString &)),
            this, SLOT(processPartitionNameText(const QString &)));
    partitionCheckBox = new QCheckBox(tr("Partition / Pool / Queue"),currentGroup);
    connect(partitionCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePartitionName(bool)));
    layout->addWidget(partitionCheckBox, row, 0);
    layout->addWidget(partitionName, row, 1);
    row++;

    QWidget *h = new QWidget(currentGroup);
    layout->addWidget(h, row, 0, 1, 2);
    QHBoxLayout *hLayout = new QHBoxLayout(h);
    hLayout->setMargin(0);
    hLayout->setSpacing(HOST_PROFILE_SPACING);

    // Create the default value widgets.
    QGroupBox *defaultGroup = new QGroupBox(tr("Defaults"), h);
    hLayout->addWidget(defaultGroup);
    QGridLayout *dLayout = new QGridLayout(defaultGroup);
    dLayout->setMargin(5);
    dLayout->setSpacing(HOST_PROFILE_SPACING+2);
    row = 0;

    numProcessors = new QSpinBox(defaultGroup);
    numProcessors->setKeyboardTracking(false);
    numProcessors->setRange(1,999999);
    numProcessors->setSingleStep(1);
    connect(numProcessors, SIGNAL(valueChanged(int)),
            this, SLOT(numProcessorsChanged(int)));
    numProcLabel = new QLabel(tr("Number of processors"), defaultGroup);
    dLayout->addWidget(numProcLabel, row, 0);
    dLayout->addWidget(numProcessors, row, 1);
    row++;

    numNodes = new QSpinBox(defaultGroup);
    numNodes->setKeyboardTracking(false);
    numNodes->setRange(1,999999);
    numNodes->setSingleStep(1);
    
    connect(numNodes, SIGNAL(valueChanged(int)),
            this, SLOT(numNodesChanged(int)));
    numNodesCheckBox = new QCheckBox(tr("Number of nodes"), defaultGroup);
    connect(numNodesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleNumNodes(bool)));
    dLayout->addWidget(numNodesCheckBox, row, 0);
    dLayout->addWidget(numNodes, row, 1);
    row++;

    bankName = new QLineEdit(defaultGroup);
    connect(bankName, SIGNAL(textChanged(const QString &)),
            this, SLOT(processBankNameText(const QString &)));
    bankCheckBox = new QCheckBox(tr("Bank / Account"),defaultGroup);
    connect(bankCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleBankName(bool)));
    dLayout->addWidget(bankCheckBox, row, 0);
    dLayout->addWidget(bankName, row, 1);
    row++;

    timeLimit = new QLineEdit(defaultGroup);
    connect(timeLimit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processTimeLimitText(const QString &)));
    timeLimitCheckBox = new QCheckBox(tr("Time Limit"), defaultGroup);
    connect(timeLimitCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleTimeLimit(bool)));
    dLayout->addWidget(timeLimitCheckBox, row, 0);
    dLayout->addWidget(timeLimit, row, 1);
    row++;

    machinefile = new QLineEdit(defaultGroup);
    connect(machinefile, SIGNAL(textChanged(const QString &)),
            this, SLOT(processMachinefileText(const QString &)));
    machinefileCheckBox = new QCheckBox(tr("Machine File"), defaultGroup);
    connect(machinefileCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleMachinefile(bool)));
    dLayout->addWidget(machinefileCheckBox, row, 0);
    dLayout->addWidget(machinefile, row, 1);
    row++;

    // Create the constraint widgets.
    constraintGroup = new QGroupBox(tr("Constraints"), h);
    constraintGroup->setCheckable(true);
    hLayout->addWidget(constraintGroup);
    connect(constraintGroup, SIGNAL(toggled(bool)),
            this, SLOT(toggleAllowableNodeProcs(bool)));
    QGridLayout *cLayout = new QGridLayout(constraintGroup);
    cLayout->setMargin(5);
    row = 0;

    QPushButton *addRow = new QPushButton(tr("Add row"), constraintGroup);
    cLayout->addWidget(addRow, row,0);
    connect(addRow, SIGNAL(clicked()),
            this, SLOT(allowableNodeProcsAddRow()));
    QPushButton *deleteRow = new QPushButton(tr("Delete row"), constraintGroup);
    cLayout->addWidget(deleteRow, row,1);
    connect(deleteRow, SIGNAL(clicked()),
            this, SLOT(allowableNodeProcsDeleteRow()));
    row++;

    allowableNodeProcs = new QTableWidget(constraintGroup);
    allowableNodeProcs->setSelectionMode(QAbstractItemView::SingleSelection);
    allowableNodeProcs->setColumnCount(2);
    QStringList headerLabels;
    headerLabels << tr("Nodes")
                 << tr("Processors");
    allowableNodeProcs->setHorizontalHeaderLabels(headerLabels);
    allowableNodeProcs->verticalHeader()->hide();
    allowableNodeProcs->horizontalHeader()->setStretchLastSection(true);
    cLayout->addWidget(allowableNodeProcs, row,0, 3,2);
    allowableNodeProcs->setRowCount(1);
    allowableNodeProcs->setItem(0, 0, new QTableWidgetItem(""));
    allowableNodeProcs->setItem(0, 1, new QTableWidgetItem(""));

    row++;

    return currentGroup;
}


// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateAdvancedSettingsGroup
//
// Purpose:
//   Creates the advanced job control settings tab for a launch profile.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Brad Whitlock, Thu Oct  6 11:56:32 PDT 2011
//   Return the created widget.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateAdvancedSettingsGroup()
{
    QWidget *currentGroup = new QWidget();

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    tmpLayout->setMargin(5);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(HOST_PROFILE_SPACING);
    tmpLayout->addStretch(5);

    loadBalancing = new QComboBox(currentGroup);
    loadBalancing->addItem(tr("Auto"));
    loadBalancing->addItem(tr("Static"));
    loadBalancing->addItem(tr("Dynamic"));
    connect(loadBalancing, SIGNAL(activated(int)),
            this, SLOT(loadBalancingChanged(int)));

    loadBalancingLabel = new QLabel(tr("Load balancing"));
    layout->addWidget(loadBalancingLabel, row, 0);
    layout->addWidget(loadBalancing, row, 1);
    row++;

    useVisItScriptForEnvCheckBox = new QCheckBox(
                             tr("Use VisIt script to set up parallel environment"),
                             currentGroup);
    layout->addWidget(useVisItScriptForEnvCheckBox, row,0, 1,3);
    connect(useVisItScriptForEnvCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleUseVisItScriptForEnv(bool)));
    row++;

    launchArgs = new QLineEdit(currentGroup);
    connect(launchArgs, SIGNAL(textChanged(const QString &)),
            this, SLOT(processLaunchArgsText(const QString &)));
    launchArgsCheckBox = new QCheckBox(tr("Launcher arguments"),currentGroup);
    connect(launchArgsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleLaunchArgs(bool)));
    layout->addWidget(launchArgsCheckBox, row, 0);
    layout->addWidget(launchArgs, row, 1);
    row++;

    sublaunchArgs = new QLineEdit(currentGroup);
    connect(sublaunchArgs, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSublaunchArgsText(const QString &)));
    sublaunchArgsCheckBox = new QCheckBox(tr("Sublauncher arguments"),currentGroup);
    connect(sublaunchArgsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSublaunchArgs(bool)));
    layout->addWidget(sublaunchArgsCheckBox, row, 0);
    layout->addWidget(sublaunchArgs, row, 1);
    row++;

    sublaunchPreCmd = new QLineEdit(currentGroup);
    connect(sublaunchPreCmd, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSublaunchPreCmdText(const QString &)));
    sublaunchPreCmdCheckBox = new QCheckBox(tr("Sublauncher pre-mpi command"), currentGroup);
    connect(sublaunchPreCmdCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSublaunchPreCmd(bool)));
    layout->addWidget(sublaunchPreCmdCheckBox, row, 0);
    layout->addWidget(sublaunchPreCmd, row, 1);
    row++;

    sublaunchPostCmd = new QLineEdit(currentGroup);
    connect(sublaunchPostCmd, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSublaunchPostCmdText(const QString &)));
    sublaunchPostCmdCheckBox = new QCheckBox(tr("Sublauncher post-mpi command"), currentGroup);
    connect(sublaunchPostCmdCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSublaunchPostCmd(bool)));
    layout->addWidget(sublaunchPostCmdCheckBox, row, 0);
    layout->addWidget(sublaunchPostCmd, row, 1);
    row++;

    return currentGroup;
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::CreateHWAccelSettingsGroup
//
// Purpose:
//   Creates the hardware acceleration settings tab for a launch profile.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//
//   Tom Fogal, Fri May  6 18:13:49 MDT 2011
//   Add new X configuration widgets.
//   Remove preCommand/postCommand widgets.
//
//   Brad Whitlock, Thu Oct  6 11:56:32 PDT 2011
//   Return the created widget.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateHWAccelSettingsGroup()
{
    QWidget *currentGroup = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(currentGroup);
    layout->setMargin(5);

    QString str1(
       QString("<i>") + 
       tr("These options are for hardware accelerating the scalable rendering "
          "feature on a compute cluster. In other modes, VisIt will automatically "
          "use hardware acceleration. This tab only needs to be modified for "
          "compute clusters that have graphics cards.") +
       QString("</i>"));

    hwdisclaimer = new QLabel(str1, currentGroup);
    hwdisclaimer->setWordWrap(true);
    layout->addWidget(hwdisclaimer);

    // Spacer.
    layout->addWidget(new QLabel(currentGroup));

    //
    // Hardware group
    //
    hardwareGroup = new QGroupBox(tr("Use cluster's graphics cards"), currentGroup);
    hardwareGroup->setCheckable(true);
    connect(hardwareGroup, SIGNAL(toggled(bool)),
            this, SLOT(toggleCanDoHW(bool)));
    layout->addWidget(hardwareGroup);
    int hRow = 0;
    QGridLayout *hLayout = new QGridLayout(hardwareGroup);
    hLayout->setMargin(5);
    hLayout->setSpacing(HOST_PROFILE_SPACING);

    QLabel* lblXDisplay = new QLabel(tr("DISPLAY:"), hardwareGroup);
    txtXDisplay = new QLineEdit(hardwareGroup);
    txtXDisplay->setText(":%l");
    connect(txtXDisplay, SIGNAL(textChanged(const QString&)), this,
            SLOT(xDisplayChanged(const QString&)));
    hLayout->addWidget(lblXDisplay, hRow,0, 1,1);
    hLayout->addWidget(txtXDisplay, hRow,1, 1,1);
    hRow++;

    cbLaunchX = new QCheckBox(tr("Have VisIt launch X servers"), hardwareGroup);
    connect(cbLaunchX, SIGNAL(toggled(bool)), this, SLOT(toggleLaunchX(bool)));
    hLayout->addWidget(cbLaunchX, hRow,0, 1,2);
    hRow++;

    QLabel* lblNGPUs = new QLabel(tr("Number of GPUs per node:"), hardwareGroup);
    sbNGPUs = new QSpinBox();
    sbNGPUs->setKeyboardTracking(false);
    sbNGPUs->setRange(0, 2048);
    sbNGPUs->setEnabled(true);
    connect(sbNGPUs, SIGNAL(valueChanged(const QString&)), this,
            SLOT(nGPUsChanged(const QString&)));
    hLayout->addWidget(lblNGPUs, hRow,0, 1,1);
    hLayout->addWidget(sbNGPUs, hRow,1, 1,1);
    hRow++;

    cbXArgs = new QCheckBox(tr("X server arguments"), hardwareGroup);
    txtXArgs = new QLineEdit(hardwareGroup);
    connect(cbXArgs, SIGNAL(toggled(bool)), this, SLOT(toggleXArgs(bool)));
    connect(txtXArgs, SIGNAL(textChanged(const QString&)), this,
            SLOT(xArgsChanged(const QString&)));
    hLayout->addWidget(cbXArgs, hRow,0, 1,1);
    hLayout->addWidget(txtXArgs, hRow,1, 1,1);
    hRow++;

    layout->addStretch(5);

    return currentGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window so it reflects the current state
//   of the HostProfileList object.
//
// Programmer: Jeremy Meredith
// Creation:   February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Don't assume list index is same as host profile list index.  Add user
//   data to store the item's actual index in the host profile list.
//   Also, make sure at least one item is selected.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateWindow(bool doAll)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if (!doAll && !profiles->IsSelected(HostProfileList::ID_machines))
        return;

    int nMachines = profiles->GetNumMachines();
    MachineProfile *oldMachine = currentMachine;
    hostList->blockSignals(true);
    hostList->clear();
    currentMachine = NULL;
    hostList->blockSignals(false);
    for (int i=0; i<nMachines; i++)
    {
        MachineProfile *mp = (MachineProfile*)profiles->GetMachines()[i];
        QListWidgetItem *item;
        if (mp->GetHostNickname() != "")
            item = new QListWidgetItem(mp->GetHostNickname().c_str());
        else
            item = new QListWidgetItem(mp->GetHost().c_str());
        item->setData(Qt::UserRole,i);
        hostList->addItem(item);
        if (oldMachine == mp)
        {
            hostList->blockSignals(true);
            item->setSelected(true);
            hostList->blockSignals(false);
            currentMachine = oldMachine;
        }
    }
    if (currentMachine == NULL && nMachines > 0)
    {
        QListWidgetItem *item = hostList->item(0);
        hostList->blockSignals(true);
        item->setSelected(true);
        hostList->blockSignals(false);
        currentMachine = (MachineProfile*)profiles->GetMachines()[
                                             item->data(Qt::UserRole).toInt()];
    }

    UpdateMachineProfile();
    UpdateWindowSensitivity();
}


// ****************************************************************************
// Method:  QvisHostProfileWindow::UpdateMachineProfile
//
// Purpose:
//   Updates the machine profile areas of the window.
//   Tries to keep the old machine profile selected if possible.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Select one of the launch profiles if we wouldn't have otherwise.
//   Choose the active one if we have one, the first one otherwise.
//
//   Eric Brugger, Tue Apr 19 20:39:50 PDT 2011
//   I added the ability to specify a gateway machine to use to get to the
//   remote host.
//
//   Brad Whitlock, Thu Oct  6 12:08:27 PDT 2011
//   I added max node/processor widgets.
//
//   Brad Whitlock, Wed Aug 15 13:58:46 PDT 2012
//   I added ssh command.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateMachineProfile()
{
    hostName->blockSignals(true);
    hostAliases->blockSignals(true);
    hostNickname->blockSignals(true);
    userName->blockSignals(true);
    clientHostNameMethod->blockSignals(true);
    clientHostName->blockSignals(true);
    sshCommandCheckBox->blockSignals(true);
    sshCommand->blockSignals(true);
    sshPortCheckBox->blockSignals(true);
    sshPort->blockSignals(true);
    useGatewayCheckBox->blockSignals(true);
    gatewayHost->blockSignals(true);
    tunnelSSH->blockSignals(true);
    directory->blockSignals(true);
    shareMDServerCheckBox->blockSignals(true);
    maxNodesCheckBox->blockSignals(true);
    maxNodes->blockSignals(true);
    maxProcessorsCheckBox->blockSignals(true);
    maxProcessors->blockSignals(true);

    if (currentMachine == NULL)
    {
        machineTabs->setEnabled(false);
        hostName->setText("");
        hostName->setText(GetViewerProxy()->GetLocalHostName().c_str());
        hostAliases->setText("");
        hostNickname->setText("");
        userName->setText("");
        userName->setText(GetViewerProxy()->GetLocalUserName().c_str());
        clientHostNameMethod->button(0)->setChecked(true);
        clientHostName->setText("");
        sshCommandCheckBox->setChecked(false);
        sshCommand->setText("");
        sshPortCheckBox->setChecked(false);
        sshPort->setText("");
        useGatewayCheckBox->setChecked(false);
        gatewayHost->setText("");
        tunnelSSH->setChecked(false);
        shareMDServerCheckBox->setChecked(false);
        maxNodesCheckBox->setChecked(false);
        maxNodes->setValue(1);
        maxProcessorsCheckBox->setChecked(false);
        maxProcessors->setValue(1);

        currentLaunch = NULL;
    }
    else
    {
        const MachineProfile &mp = *currentMachine;

        // Replace any localhost machine names.
        ReplaceLocalHost();
    
        machineTabs->setEnabled(true);

        // Update the contents of the machine settings tab

        hostName->setText(mp.GetHost().c_str());
        hostName->setCursorPosition(0);
        hostAliases->setText(mp.GetHostAliases().c_str());
        hostAliases->setCursorPosition(0);
        hostNickname->setText(mp.GetHostNickname().c_str());
        hostNickname->setCursorPosition(0);
        if(mp.UserName() == "notset")
            userName->setText(GetViewerProxy()->GetLocalUserName().c_str());
        else
            userName->setText(mp.UserName().c_str());
        switch (mp.GetClientHostDetermination())
        {
          case MachineProfile::MachineName:
            clientHostNameMethod->button(0)->setChecked(true);
            break;
          case MachineProfile::ParsedFromSSHCLIENT:
            clientHostNameMethod->button(1)->setChecked(true);
            break;
          case MachineProfile::ManuallySpecified:
            clientHostNameMethod->button(2)->setChecked(true);
            break;
        }
        clientHostName->setText(mp.GetManualClientHostName().c_str());

        sshCommandCheckBox->setChecked(mp.GetSshCommandSpecified());
        QString sshcmd;
        for(size_t i = 0; i < mp.GetSshCommand().size(); ++i)
        {
            sshcmd += QString(mp.GetSshCommand()[i].c_str());
            if(i < mp.GetSshCommand().size()-1)
                sshcmd += QString(" ");
        }
        sshCommand->setText(sshcmd);
        sshPortCheckBox->setChecked(mp.GetSshPortSpecified());
        sshPort->setText(QString().setNum(mp.GetSshPort()));

        useGatewayCheckBox->setChecked(mp.GetUseGateway());
        gatewayHost->setText(mp.GetGatewayHost().c_str());
        tunnelSSH->setChecked(mp.GetTunnelSSH());
        shareMDServerCheckBox->setChecked(mp.GetShareOneBatchJob());
        directory->setText(mp.GetDirectory().c_str());
        maxNodesCheckBox->setChecked(mp.GetMaximumNodesValid());
        maxNodes->setValue(mp.GetMaximumNodes());
        maxProcessorsCheckBox->setChecked(mp.GetMaximumProcessorsValid());
        maxProcessors->setValue(mp.GetMaximumProcessors());

        // Update the contents of the launch profiles tab
        // Keep track of the old selection if it's still there.
        int nProfiles = mp.GetNumLaunchProfiles();
        LaunchProfile *oldLaunch = currentLaunch;
        profileList->blockSignals(true);
        profileList->clear();
        currentLaunch = NULL;
        profileList->blockSignals(false);
        for (int i=0; i<nProfiles; i++)
        {
            LaunchProfile *lp = (LaunchProfile*)mp.GetLaunchProfiles()[i];
            profileList->addItem(lp->GetProfileName().c_str());
            if (currentMachine->GetActiveProfile() == i)
                profileList->item(i)->setCheckState(Qt::Checked);
            else
                profileList->item(i)->setCheckState(Qt::Unchecked);
            profileList->item(i)->setFlags(Qt::ItemIsSelectable |
                                           Qt::ItemIsEnabled);
            if (oldLaunch == lp)
            {
                profileList->blockSignals(true);
                profileList->item(i)->setSelected(true);
                profileList->blockSignals(false);
                currentLaunch = oldLaunch;
            }
        }
        // If none is selected, select one of them, ideally the active one
        if (currentLaunch == NULL && nProfiles > 0)
        {
            int index = mp.GetActiveProfile();
            if (index < 0)
                index = 0;
            currentLaunch = (LaunchProfile*)mp.GetLaunchProfiles()[index];
            profileList->blockSignals(true);
            profileList->item(index)->setSelected(true);
            profileList->blockSignals(false);
        }
    }

    hostName->blockSignals(false);
    hostAliases->blockSignals(false);
    hostNickname->blockSignals(false);
    userName->blockSignals(false);
    clientHostNameMethod->blockSignals(false);
    clientHostName->blockSignals(false);
    sshCommandCheckBox->blockSignals(false);
    sshCommand->blockSignals(false);
    sshPortCheckBox->blockSignals(false);
    sshPort->blockSignals(false);
    useGatewayCheckBox->blockSignals(false);
    gatewayHost->blockSignals(false);
    tunnelSSH->blockSignals(false);
    directory->blockSignals(false);
    shareMDServerCheckBox->blockSignals(false);
    maxNodesCheckBox->blockSignals(false);
    maxNodes->blockSignals(false);
    maxProcessorsCheckBox->blockSignals(false);
    maxProcessors->blockSignals(false);

    UpdateLaunchProfile();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::UpdateLaunchProfile
//
// Purpose:
//   Updates the launch profile areas of the window.
//   Tries to keep the old launch profile selected if possible.
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//
//   Tom Fogal, Fri May  6 18:21:48 MDT 2011
//   Update for new parallel GPU GUI elements.
//
//   David Camp, Mon Aug  4 10:46:09 PDT 2014
//   Added the threads option. Removed duplicate set value calls on timeout.
//
//   Brad Whitlock, Wed Oct 22 17:07:49 PDT 2014
//   Added node/proc constraints.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateLaunchProfile()
{
    // Block signals for certain widgets.
    profileName->blockSignals(true);
    numProcessors->blockSignals(true);
    numNodes->blockSignals(true);
    numNodesCheckBox->blockSignals(true);
    parallelCheckBox->blockSignals(true);
    partitionCheckBox->blockSignals(true);
    partitionName->blockSignals(true);
    bankCheckBox->blockSignals(true);
    bankName->blockSignals(true);
    timeLimitCheckBox->blockSignals(true);
    timeLimit->blockSignals(true);
    machinefileCheckBox->blockSignals(true);
    machinefile->blockSignals(true);
    constraintGroup->blockSignals(true);
    allowableNodeProcs->blockSignals(true);
    launchArgsCheckBox->blockSignals(true);
    launchArgs->blockSignals(true);
    sublaunchArgsCheckBox->blockSignals(true);
    sublaunchArgs->blockSignals(true);
    sublaunchPreCmdCheckBox->blockSignals(true);
    sublaunchPreCmd->blockSignals(true);
    sublaunchPostCmdCheckBox->blockSignals(true);
    sublaunchPostCmd->blockSignals(true);
    launchCheckBox->blockSignals(true);
    launchMethod->blockSignals(true);
    loadBalancing->blockSignals(true);
    hardwareGroup->blockSignals(true);
    timeout->blockSignals(true);
    threads->blockSignals(true);
    engineArguments->blockSignals(true);
    cbLaunchX->blockSignals(true);
    sbNGPUs->blockSignals(true);
    cbXArgs->blockSignals(true);
    txtXArgs->blockSignals(true);
    txtXDisplay->blockSignals(true);

    if (currentLaunch == NULL)
    {
        profileName->setText("");
        numProcessors->setValue(1);
        
        parallelCheckBox->setChecked(false);
        launchCheckBox->setChecked(false);
        launchMethod->setCurrentIndex(0);
        numNodesCheckBox->setChecked(false);
        numNodes->setValue(1);
        partitionCheckBox->setChecked(false);
        partitionName->setText("");
        bankCheckBox->setChecked(false);
        bankName->setText("");
        timeLimitCheckBox->setChecked(false);
        timeLimit->setText("");
        machinefileCheckBox->setChecked(false);
        machinefile->setText("");
        constraintGroup->setChecked(false);
        ResizeNodeProcs(1, false);
        launchArgsCheckBox->setChecked(false);
        launchArgs->setText("");
        sublaunchArgsCheckBox->setChecked(false);
        sublaunchArgs->setText("");
        sublaunchPreCmdCheckBox->setChecked(false);
        sublaunchPreCmd->setText("");
        sublaunchPostCmdCheckBox->setChecked(false);
        sublaunchPostCmd->setText("");
        loadBalancing->setCurrentIndex(0);
        useVisItScriptForEnvCheckBox->setChecked(false);
        timeout->setValue(60*4);   // 4 hour default
        threads->setValue(4);      // 4 thread default
        engineArguments->setText("");
        cbLaunchX->setChecked(false);
        sbNGPUs->setValue(0);
        cbXArgs->setChecked(false);
        txtXArgs->setText("");
        txtXDisplay->setText(":%l");
    }
    else
    {
        profileName->setText(currentLaunch->GetProfileName().c_str());
        // Replace the "localhost" machine name.
        // If there is no user name then give it a valid user name.

        parallelCheckBox->setChecked(currentLaunch->GetParallel());
        bool parEnabled = currentLaunch->GetParallel();
        if (parEnabled)
        {
            useVisItScriptForEnvCheckBox->setChecked(currentLaunch->GetVisitSetsUpEnv());
        }
        else
        {
            useVisItScriptForEnvCheckBox->setChecked(false);
        }
        launchCheckBox->setChecked(parEnabled && currentLaunch->GetLaunchMethodSet());
        if (parEnabled && currentLaunch->GetLaunchMethodSet())
        {
            int index = 0;
            for (int j=0; j < launchMethod->count() ; j++)
            {
                if (launchMethod->itemText(j) == currentLaunch->GetLaunchMethod().c_str())
                    index = j;
            }
            launchMethod->setCurrentIndex(index);
        }
        else
        {
            launchMethod->setCurrentIndex(0);
        }
        launchArgsCheckBox->setChecked(parEnabled && currentLaunch->GetLaunchArgsSet());
        if (parEnabled && currentLaunch->GetLaunchArgsSet())
            launchArgs->setText(currentLaunch->GetLaunchArgs().c_str());
        else
            launchArgs->setText("");
        sublaunchArgsCheckBox->setChecked(parEnabled && currentLaunch->GetSublaunchArgsSet());
        if (parEnabled && currentLaunch->GetSublaunchArgsSet())
            sublaunchArgs->setText(currentLaunch->GetSublaunchArgs().c_str());
        else
            sublaunchArgs->setText("");
        sublaunchPreCmdCheckBox->setChecked(parEnabled && currentLaunch->GetSublaunchPreCmdSet());
        if (parEnabled && currentLaunch->GetSublaunchPreCmdSet())
            sublaunchPreCmd->setText(currentLaunch->GetSublaunchPreCmd().c_str());
        else
            sublaunchPreCmd->setText("");
        sublaunchPostCmdCheckBox->setChecked(parEnabled && currentLaunch->GetSublaunchPostCmdSet());
        if (parEnabled && currentLaunch->GetSublaunchPostCmdSet())
            sublaunchPostCmd->setText(currentLaunch->GetSublaunchPostCmd().c_str());
        else
            sublaunchPostCmd->setText("");
        if (parEnabled)
            numProcessors->setValue(currentLaunch->GetNumProcessors());
        else
            numProcessors->setValue(1);
        numNodesCheckBox->setChecked(parEnabled && currentLaunch->GetNumNodesSet());
        if (parEnabled && currentLaunch->GetNumNodesSet())
            numNodes->setValue(currentLaunch->GetNumNodes());
        else
            numNodes->setValue(0);
        partitionCheckBox->setChecked(parEnabled && currentLaunch->GetPartitionSet());
        if (parEnabled && currentLaunch->GetPartitionSet())
            partitionName->setText(currentLaunch->GetPartition().c_str());
        else
            partitionName->setText("");
        bankCheckBox->setChecked(parEnabled && currentLaunch->GetBankSet());
        if (parEnabled && currentLaunch->GetBankSet())
            bankName->setText(currentLaunch->GetBank().c_str());
        else
            bankName->setText("");
        timeLimitCheckBox->setChecked(parEnabled && currentLaunch->GetTimeLimitSet());
        if (parEnabled && currentLaunch->GetTimeLimitSet())
            timeLimit->setText(currentLaunch->GetTimeLimit().c_str());
        else
            timeLimit->setText("");
        machinefileCheckBox->setChecked(parEnabled && currentLaunch->GetMachinefileSet());
        if (parEnabled && currentLaunch->GetMachinefileSet())
            machinefile->setText(currentLaunch->GetMachinefile().c_str());
        else
            machinefile->setText("");
        int lb = 0;
        if (currentLaunch->GetForceStatic())
            lb = 1;
        if (currentLaunch->GetForceDynamic())
            lb = 2;
        loadBalancing->setCurrentIndex(lb);
        hardwareGroup->setChecked(currentLaunch->GetCanDoHWAccel());
        timeout->setValue(currentLaunch->GetTimeout());
        threads->setValue(currentLaunch->GetNumThreads());

        bool laFlag = false;
        QString temp;
        stringVector::const_iterator pos;

        for(pos = currentLaunch->GetArguments().begin();
            pos != currentLaunch->GetArguments().end(); ++pos)
        {
            // If the last arg was not -la process as normal
            if( !laFlag )
            {
              temp += QString(pos->c_str());
              temp += " ";

              // If the arg is -la the next arg must have quotes
              // placed around it so that it gets parced correctly.
              if( std::string(pos->c_str()) == "-la" )
                laFlag = true;
            }

            // If the last arg was -la so place quotes arount it ad
            // reset the flag.
            else if( laFlag )
            {
              temp += "\"";
              temp += QString(pos->c_str());
              temp += "\" ";
              
              laFlag = false;
            }
        }
        engineArguments->setText(temp);

        cbLaunchX->setChecked(currentLaunch->GetLaunchXServers());
        sbNGPUs->setValue(currentLaunch->GetGPUsPerNode());
        txtXArgs->setText(currentLaunch->GetXArguments().c_str());
        cbXArgs->setChecked(!currentLaunch->GetXArguments().empty());
        txtXDisplay->setText(currentLaunch->GetXDisplay().c_str());

        // Update the node/proc constraints.
        constraintGroup->setChecked(currentLaunch->GetConstrainNodeProcs());
        int nnodes = (int)currentLaunch->GetAllowableNodes().size();
        int nprocs = (int)currentLaunch->GetAllowableProcs().size();
        int n = std::min(nnodes, nprocs);
        ResizeNodeProcs(n, true);
        for(int i = 0; i < n; ++i)
        {
            QString nn, np;
            nn.setNum(currentLaunch->GetAllowableNodes()[i]);
            np.setNum(currentLaunch->GetAllowableProcs()[i]);

            allowableNodeProcs->item(i, 0)->setText(nn);
            allowableNodeProcs->item(i, 0)->setFlags(
                     Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);

            allowableNodeProcs->item(i, 1)->setText(np);
            allowableNodeProcs->item(i, 1)->setFlags(
                     Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        }
    }

    // Restore signals.
    profileName->blockSignals(false);
    numProcessors->blockSignals(false);
    parallelCheckBox->blockSignals(false);
    numNodes->blockSignals(false);
    numNodesCheckBox->blockSignals(false);
    partitionCheckBox->blockSignals(false);
    partitionName->blockSignals(false);
    bankCheckBox->blockSignals(false);
    bankName->blockSignals(false);
    timeLimitCheckBox->blockSignals(false);
    timeLimit->blockSignals(false);
    machinefileCheckBox->blockSignals(false);
    machinefile->blockSignals(false);
    constraintGroup->blockSignals(false);
    allowableNodeProcs->blockSignals(false);
    launchArgsCheckBox->blockSignals(false);
    launchArgs->blockSignals(false);
    sublaunchArgsCheckBox->blockSignals(false);
    sublaunchArgs->blockSignals(false);
    sublaunchPreCmdCheckBox->blockSignals(false);
    sublaunchPreCmd->blockSignals(false);
    sublaunchPostCmdCheckBox->blockSignals(false);
    sublaunchPostCmd->blockSignals(false);
    launchCheckBox->blockSignals(false);
    launchMethod->blockSignals(false);
    loadBalancing->blockSignals(false);
    hardwareGroup->blockSignals(false);
    timeout->blockSignals(false);
    threads->blockSignals(false);
    engineArguments->blockSignals(false);
    cbLaunchX->blockSignals(false);
    sbNGPUs->blockSignals(false);
    cbXArgs->blockSignals(false);
    txtXArgs->blockSignals(false);
    txtXDisplay->blockSignals(false);
}


// ****************************************************************************
// Method: QvisHostProfileWindow::ReplaceLocalHost
//
// Purpose: 
//   Looks through the host profile list and replaces all hosts that are
//   "localhost" with the correct local hostname.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 11:47:11 PDT 2001
//
// Modifications:
//    Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//    Initial Qt4 Port.
//
//    Cyrus Harrison, Tue Dec  9 14:55:00 PST 2008
//    Finished Qt4 port todo.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::ReplaceLocalHost()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    for(int i = 0; i < profiles->GetNumMachines(); ++i)
    {
        MachineProfile &current = profiles->operator[](i);
        if(current.GetHost() == "localhost")
        {
            current.SetHost(GetViewerProxy()->GetLocalHostName()); 
        }
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::UpdateWindowSensitivity
//
// Purpose: 
//   This method sets the sensitivity of the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:15:03 PST 2000
//
// Modifications:
//    Jeremy Meredith, Tue Jul 17 17:58:51 PDT 2001
//    Added numNodes, launchMethod, and partitionName, as well as checkboxes
//    for each.
//
//    Jeremy Meredith, Fri Sep 21 14:29:59 PDT 2001
//    Added buttons for forcing static and dynamic load balancing.
//
//    Jeremy Meredith, Wed Aug 14 10:51:34 PDT 2002
//    Changed the way things are updated.  Force static load balancing for now.
//
//    Jeremy Meredith, Fri Jan 24 14:43:28 PST 2003
//    Added optional arguments to the parallel launcher.
//
//    Jeremy Meredith, Mon Apr 14 18:26:05 PDT 2003
//    Added hostAliases.
//
//    Jeremy Meredith, Thu Oct  9 15:47:00 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Jeremy Meredith, Thu Sep 15 16:39:31 PDT 2005
//    Added machinefile and useVisItScriptForEnv.
//
//    Hank Childs, Sat Dec  3 20:55:49 PST 2005
//    Added support for new hardware acceleration options.
//
//    Eric Brugger, Thu Feb 15 12:14:34 PST 2007
//    Added support for additional sublauncher arguments.
//
//    Jeremy Meredith, Thu May 24 11:05:45 EDT 2007
//    Added support for SSH port tunneling.
//
//    Brad Whitlock, Wed Jun 6 09:34:56 PDT 2007
//    Removed load balancing radio buttons. Added code to set enabled state for
//    "batch job" and "parallel environment" advanced check boxes. Added code
//    to set enabled state of advancedGroup.
//
//    Jeremy Meredith, Thu Jun 28 13:19:55 EDT 2007
//    Disable client host name method determination widgets when SSH tunneling
//    is enabled.
// 
//    Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//    Initial Qt4 Port.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//    Eric Brugger, Tue Apr 19 20:39:50 PDT 2011
//    I added the ability to specify a gateway machine to use to get to the
//    remote host.
//
//    Tom Fogal, Thu May  5 15:21:19 MDT 2011
//    Enable/Disable new GPU options.
//
//    Brad Whitlock, Thu Oct  6 12:09:14 PDT 2011
//    Set max node/processor enabled state.
//
//    Brad Whitlock, Thu Dec  1 11:46:54 PST 2011
//    Update widget sensitivity based on whether we're sharing 1 batch job.
//
//    Brad Whitlock, Wed Aug 15 14:11:06 PDT 2012
//    Added sshCommand.
//
//    David Camp, Mon Aug  4 10:46:09 PDT 2014
//    Added the threads option.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateWindowSensitivity()
{
    bool hostEnabled = (currentMachine != NULL);
    bool launchEnabled = (currentLaunch != NULL);
    bool parEnabled = launchEnabled ? (currentLaunch->GetParallel()) : false;

    // Set the widget sensitivity.
    delHost->setEnabled(hostEnabled);
    copyHost->setEnabled(hostEnabled);

    addProfile->setEnabled(hostEnabled);
    delProfile->setEnabled(launchEnabled);
    copyProfile->setEnabled(launchEnabled);
    makeDefaultProfile->setEnabled(launchEnabled && hostEnabled &&
               currentMachine->GetActiveLaunchProfile() != currentLaunch);

    machineTabs->setEnabled(hostEnabled);
    hostNameLabel->setEnabled(hostEnabled);
    hostName->setEnabled(hostEnabled);
    hostAliasesLabel->setEnabled(hostEnabled);
    hostAliases->setEnabled(hostEnabled);
    hostNicknameLabel->setEnabled(hostEnabled);
    hostNickname->setEnabled(hostEnabled);
    userNameLabel->setEnabled(hostEnabled);
    userName->setEnabled(hostEnabled);
    chnMachineName->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
#ifdef SHARE_BATCH_JOB_HOST_ISSUES
    chnParseFromSSHClient->setEnabled(hostEnabled &&
                                      currentMachine->GetTunnelSSH() == false &&
                                      currentMachine->GetShareOneBatchJob() == false);
#else
    chnParseFromSSHClient->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
#endif
    chnSpecifyManually->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
    clientHostNameMethodLabel->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
    clientHostName->setEnabled(hostEnabled &&
                               currentMachine->GetTunnelSSH() == false &&
                               currentMachine->GetClientHostDetermination() ==
                                              MachineProfile::ManuallySpecified);
    sshCommand->setEnabled(hostEnabled && currentMachine->GetSshCommandSpecified());
    sshPort->setEnabled(hostEnabled && currentMachine->GetSshPortSpecified());
    gatewayHost->setEnabled(hostEnabled && currentMachine->GetUseGateway());
#ifdef SHARE_BATCH_JOB_HOST_ISSUES
    tunnelSSH->setEnabled(hostEnabled &&
                          !currentMachine->GetShareOneBatchJob());
    shareMDServerCheckBox->setEnabled(hostEnabled &&
                                      !currentMachine->GetTunnelSSH());
#else
    tunnelSSH->setEnabled(hostEnabled);
    shareMDServerCheckBox->setEnabled(hostEnabled);
#endif
    maxNodesCheckBox->setEnabled(hostEnabled);
    maxNodes->setEnabled(hostEnabled && currentMachine->GetMaximumNodesValid());
    maxProcessorsCheckBox->setEnabled(hostEnabled);
    maxProcessors->setEnabled(hostEnabled && currentMachine->GetMaximumProcessorsValid());
    

    profileNameLabel->setEnabled(launchEnabled);
    profileName->setEnabled(launchEnabled);
    timeout->setEnabled(launchEnabled);
    timeoutLabel->setEnabled(launchEnabled);
    threads->setEnabled(launchEnabled);
    threadsLabel->setEnabled(launchEnabled);
    parallelCheckBox->setEnabled(launchEnabled);
    engineArgumentsLabel->setEnabled(launchEnabled);
    engineArguments->setEnabled(launchEnabled);

    profileTabs->setEnabled(launchEnabled);

    launchCheckBox->setEnabled(parEnabled);
    launchMethod->setEnabled(parEnabled && currentLaunch->GetLaunchMethodSet());
    launchArgsCheckBox->setEnabled(parEnabled);
    launchArgs->setEnabled(parEnabled && currentLaunch->GetLaunchArgsSet());
    sublaunchArgsCheckBox->setEnabled(parEnabled);
    sublaunchArgs->setEnabled(parEnabled && currentLaunch->GetSublaunchArgsSet());
    sublaunchPreCmdCheckBox->setEnabled(parEnabled);
    sublaunchPreCmd->setEnabled(parEnabled && currentLaunch->GetSublaunchPreCmdSet());
    sublaunchPostCmdCheckBox->setEnabled(parEnabled);
    sublaunchPostCmd->setEnabled(parEnabled && currentLaunch->GetSublaunchPostCmdSet());
    numProcLabel->setEnabled(parEnabled);
    numProcessors->setEnabled(parEnabled);
    int maxP = 100000;
    if(hostEnabled && currentMachine->GetMaximumProcessorsValid())
        maxP = currentMachine->GetMaximumProcessors();
    numProcessors->setRange(1, maxP);
    numNodesCheckBox->setEnabled(parEnabled);
    numNodes->setEnabled(parEnabled && currentLaunch->GetNumNodesSet());
    int maxN = 100000;
    if(hostEnabled && currentMachine->GetMaximumNodesValid())
        maxN = currentMachine->GetMaximumNodes();
    numNodes->setRange(1, maxN);
    partitionCheckBox->setEnabled(parEnabled);
    partitionName->setEnabled(parEnabled && currentLaunch->GetPartitionSet());
    bankCheckBox->setEnabled(parEnabled);
    bankName->setEnabled(parEnabled && currentLaunch->GetBankSet());
    timeLimitCheckBox->setEnabled(parEnabled);
    timeLimit->setEnabled(parEnabled && currentLaunch->GetTimeLimitSet());
    machinefileCheckBox->setEnabled(parEnabled);
    machinefile->setEnabled(parEnabled && currentLaunch->GetMachinefileSet());
    constraintGroup->setEnabled(parEnabled);

#if 0 // disabling dynamic load balancing for now
    loadBalancingLabel->setEnabled(parEnabled);
    loadBalancing->setEnabled(parEnabled);
#else
    loadBalancingLabel->setEnabled(false);
    loadBalancing->setEnabled(false);
#endif

    hwdisclaimer->setEnabled(launchEnabled);
    hardwareGroup->setEnabled(launchEnabled);
    sbNGPUs->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());
    cbXArgs->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());
    txtXArgs->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel() &&
                         cbXArgs->isChecked());
    cbLaunchX->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());
    txtXDisplay->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());

    useVisItScriptForEnvCheckBox->setEnabled(parEnabled);
}

// ****************************************************************************
// Method: QvisHostProfileWindow::GetCurrentValues
//
// Purpose: 
//   Gets the current values from the widgets in the active profile area.
//
// Arguments:
//   which_widget : The number of the widget for which we want to get
//                  the inforamation, or -1 for all of them.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 11:34:57 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 17:58:51 PDT 2001
//   Added numNodes, launchMethod, and partitionName.
//
//   Jeremy Meredith, Thu Sep 13 15:31:02 PDT 2001
//   Added a translation from "(default)" for the launch method to the
//   empty string.  Also, fixed a compiler warning.
//
//   Brad Whitlock, Mon Sep 24 09:34:15 PDT 2001
//   Changed the return type and added code to replace "localhost" with a
//   real machine name.
//
//   Jeremy Meredith, Thu Feb 14 15:29:57 PST 2002
//   Changed hostName to a combo box.
//
//   Sean Ahern, Thu Feb 21 15:58:40 PST 2002
//   Added timeout support.  Added the "widget" counter to make adding
//   widgets easier.
//
//   Jeremy Meredith, Wed Aug 14 10:52:14 PDT 2002
//   Only update the parallel values if they are enabled.
//   Added banks and time limits.
//
//   Jeremy Meredith, Fri Jan 24 14:43:28 PST 2003
//   Added optional arguments to the parallel launcher.
//
//   Jeremy Meredith, Mon Apr 14 18:26:22 PDT 2003
//   Added hostAliases.
//
//   Jeremy Meredith, Fri May 16 10:59:08 PDT 2003
//   Allow empty hostAliases.
//
//   Jeremy Meredith, Thu Oct  9 15:47:00 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Jeremy Meredith, Thu Nov 13 15:59:59 PST 2003
//   Moved timeout to the right spot in the list.  Its placement moved
//   other widgets out of their required orderings.
//
//   Jeremy Meredith, Thu Sep 15 16:39:35 PDT 2005
//   Added machinefile and useVisItScriptForEnv.
//
//   Eric Brugger, Thu Feb 15 12:14:34 PST 2007
//   Added support for additional sublauncher arguments.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Dave Bremer, Wed Apr 16 17:54:14 PDT 2008
//   Added fields for commands to run pre and post the mpi command.
//
//   Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//   Initial Qt4 Port.
//
//   Cyrus Harrison, Tue Dec  9 14:55:27 PST 2008
//   Finished Qt4 Port todo.
//
//   Hank Childs, Fri May  8 05:52:11 PDT 2009
//   Add support for host nicknames.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//   Jeremy Meredith, Wed Apr 21 13:17:58 EDT 2010
//   If the username in the window is the same one we would have gotten
//   anyway, set the internal value back to the default "notset".
//
//   Eric Brugger, Tue Apr 19 20:39:50 PDT 2011
//   I added the ability to specify a gateway machine to use to get to the
//   remote host.
//
//   Brad Whitlock, Wed Aug 15 14:11:34 PDT 2012
//   Add ssh command.
//
//   David Camp, Mon Aug  4 10:46:09 PDT 2014
//   Added the threads option.
//
//   Kathleen Biagas, Wed Dec 16 11:07:43 MST 2015
//   Ensure quoted sshCommand is preserved, split args on ' ' only after the
//   end of the quoted command.
//
// ****************************************************************************

bool
QvisHostProfileWindow::GetCurrentValues()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    bool needNotify = false;

    if (currentLaunch == NULL && currentMachine == NULL)
        return needNotify;

    QString msg, temp;

    // Do the profile name
    if (currentLaunch)
    {
        temp = profileName->displayText();
        if(!temp.isEmpty())
        {
            currentLaunch->SetProfileName(std::string(temp.toStdString()));
        }
        else
        {
            needNotify = true;
            msg = tr("Profile name cannot be empty, reverting to \"%1\".").
                  arg(currentLaunch->GetProfileName().c_str());
            Message(msg);
        }
    }

    // Do the host name
    if (currentMachine)
    {
        temp = hostName->text();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            std::string newHost(temp.toStdString());
            if(newHost == "localhost")
            {
                newHost = GetViewerProxy()->GetLocalHostName(); 
                hostName->setText(newHost.c_str());
            }
            if (newHost != currentMachine->GetHost())
                needNotify = true;
            currentMachine->SetHost(newHost);
        }
        else
        {
            needNotify = true;
            msg = tr("Host name cannot be empty, reverting to \"%1\".").
                  arg(currentMachine->GetHost().c_str());
            Message(msg);
        }
    }

    // Do the user name
    if (currentMachine)
    {
        temp = userName->displayText();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            if (temp.toStdString() == GetViewerProxy()->GetLocalUserName())
                currentMachine->SetUserName("notset");
            else
                currentMachine->SetUserName(std::string(temp.toStdString()));
        }
        else
        {
            needNotify = true;
            msg = tr("Username cannot be empty, reverting to \"%1\".").
                  arg(currentMachine->UserName().c_str());
            Message(msg);
        }
    }

    // Do the launch method
    if (currentLaunch)
    {
        temp = launchMethod->currentText();
        temp = temp.trimmed();
        if (temp == tr("(default)"))
            temp = "";
        currentLaunch->SetLaunchMethod(std::string(temp.toStdString()));
    }


    // Do the number of processors
    if (currentLaunch && currentLaunch->GetParallel())
    {
        int nProc = numProcessors->value();
        if (nProc != currentLaunch->GetNumProcessors())
        {
            needNotify = true;
            currentLaunch->SetNumProcessors(nProc);
        }
    }

    // Do the number of nodes
    if (currentLaunch && currentLaunch->GetParallel())
    {
        int nNodes = numNodes->value();
        if (nNodes != currentLaunch->GetNumNodes())
        {
            needNotify = true;
            currentLaunch->SetNumNodes(nNodes);
        }
    }

    // Do the partition name
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = partitionName->displayText();
        temp = temp.trimmed();
        currentLaunch->SetPartition(std::string(temp.toStdString()));
    }

    // Do the bank name
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = bankName->displayText();
        temp = temp.trimmed();
        currentLaunch->SetBank(std::string(temp.toStdString()));
    }

    // Do the time limit
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = timeLimit->displayText();
        temp = temp.trimmed();
        currentLaunch->SetTimeLimit(std::string(temp.toStdString()));
    }

    // Do the engine command line arguments.
    if (currentLaunch)
    {
        // Terminal character for multiple space separated args sent
        // to the -la option which are in quotes.
        std::string terminal("\"");

        stringVector arguments;
        QString temp(engineArguments->displayText());
        temp = temp.simplified();

        if(!(temp.isEmpty()))
        {
            // Split the arguments into a string list.
            QStringList str = temp.split(' ');

            // Fill the arguments vector.
            for(int i = 0; i < str.count(); ++i)
            {
                if( std::string(str[i].toStdString()) != std::string("-la") )
                    arguments.push_back(std::string(str[i].toStdString()));

                // If the argument is -la the next args will have
                // spaces but should start and end with quotes but it
                // has been split so put it back together.

                // NOTE: if the quotes are missing the compositing
                // will go to the last arg.  This may not be correct
                // but that is what is interperted and does not fail.
                else 
                {
                    // Save the -la argument as normal.
                    arguments.push_back(std::string(str[i].toStdString()));

                    // Go to the next argument and start compositing it
                    if( ++i < str.count() )
                    {
                        std::string composite(str[i].toStdString());

                        // Strip the quote as it will get added back
                        // in when it is processed.
                        if(composite.find(terminal) == 0)
                            composite.erase( composite.find(terminal), 1 );

                        // Process the rest of the arguments stopping when
                        // another quote is found.
                        while( ++i < str.count() )
                        {
                          std::string tmp(str[i].toStdString());
                          composite += std::string(" ") + tmp;
                          
                          // Strip the quote as it will get added back
                          // in when it is processed. Then quit.
                          if( composite.find(terminal) == composite.size()-1 )
                          {
                              composite.erase( composite.find(terminal), 1 );
                              break;
                          }
                        }

                        // Send the composite argument.
                        arguments.push_back(composite);
                    }
                }
            }
        }

        // Set the arguments.
        currentLaunch->SetArguments(arguments);
    }

    // Do the launcher args
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = launchArgs->displayText();
        temp = temp.trimmed();
        currentLaunch->SetLaunchArgs(std::string(temp.toStdString()));
    }

    // Do the directory
    if (currentMachine)
    {
        temp = directory->displayText();
        temp = temp.trimmed();
        currentMachine->SetDirectory(temp.toStdString());
    }

    // Do the timeout
    if (currentLaunch)
    {
        int tOut = timeout->value();
        if (tOut != currentLaunch->GetTimeout())
        {
           needNotify = true;
           currentLaunch->SetTimeout(tOut);
        }
    }

    // Do the threads
    if (currentLaunch)
    {
        int tOut = threads->value();
        if (tOut != currentLaunch->GetNumThreads())
        {
            needNotify = true;
            currentLaunch->SetNumThreads(tOut);
        }
    }

    // Do the host aliases
    if (currentMachine)
    {
        temp = hostAliases->text();
        temp = temp.trimmed();

        std::string newAliases(temp.toStdString());
        if (newAliases != currentMachine->GetHostAliases())
            needNotify = true;

        currentMachine->SetHostAliases(newAliases);
    }

    // Do the host nickname
    if (currentMachine)
    {
        temp = hostNickname->displayText();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            currentMachine->SetHostNickname(std::string(temp.toStdString()));
        }
        else
        {
            needNotify = true;
            msg = tr("Username cannot be empty, reverting to \"%1\".").
                  arg(currentMachine->GetHostNickname().c_str());
            Message(msg);
        }

        temp = hostNickname->text();
        temp = temp.trimmed();

        std::string newNickname(temp.toStdString());
        if (newNickname != currentMachine->GetHostNickname())
            needNotify = true;

        currentMachine->SetHostNickname(newNickname);
    }

    // Do the manual client host name
    if (currentMachine)
    {
        temp = clientHostName->text();
        temp = temp.trimmed();

        std::string newClientHostName(temp.toStdString());
        if (newClientHostName != currentMachine->GetManualClientHostName())
            needNotify = true;

        currentMachine->SetManualClientHostName(newClientHostName);
    }

    // Do the ssh command
    if (currentMachine)
    {
        temp = sshCommand->text();

        stringVector newCommand;
        if (temp.startsWith('\"'))
        {
            if (temp.endsWith('\"'))
            {
                newCommand.push_back(temp.toStdString());
            }
            else
            {
                // split into command and args.
                int pos = temp.indexOf("\"", 1);
                QString cmd(temp.left(pos+1));
                newCommand.push_back(cmd.toStdString());
                QString args(temp.right(temp.size()-pos-2));
                QStringList arglist(args.split(' '));
                for(int i = 0; i < arglist.size(); ++i)
                    newCommand.push_back(arglist[i].toStdString());
            }
        }
        else
        {
            QStringList cmd(temp.split(' '));
            for(int i = 0; i < cmd.size(); ++i)
                newCommand.push_back(cmd[i].toStdString());
            if (currentMachine->GetSshCommand() != newCommand)
                needNotify = true;
        }

        currentMachine->SetSshCommand(newCommand);
    }

    // Do the ssh port
    if (currentMachine)
    {
        temp = sshPort->text();

        int newPort = temp.toInt();
        if (currentMachine->GetSshPort() != newPort)
            needNotify = true;

        currentMachine->SetSshPort(newPort);
    }

    // Do the gateway host
    if (currentMachine)
    {
        temp = gatewayHost->text();
        temp = temp.trimmed();
        currentMachine->SetGatewayHost(std::string(temp.toStdString()));
    }

    // Do the machine file
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = machinefile->displayText();
        temp = temp.trimmed();
        currentLaunch->SetMachinefile(std::string(temp.toStdString()));
    }

    // Do the sublauncher args
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = sublaunchArgs->displayText();
        temp = temp.trimmed();
        currentLaunch->SetSublaunchArgs(std::string(temp.toStdString()));
    }

    // Do the sublauncher pre cmd
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = sublaunchPreCmd->displayText();
        temp = temp.trimmed();
        currentLaunch->SetSublaunchPreCmd(std::string(temp.toStdString()));
    }

    // Do the sublauncher post cmd
    if (currentLaunch && currentLaunch->GetParallel())
    {
        temp = sublaunchPostCmd->displayText();
        temp = temp.trimmed();
        currentLaunch->SetSublaunchPostCmd(std::string(temp.toStdString()));
    }

    // There was an error with some of the input.
    if(needNotify)
    {
        // Mark the active profile so it will force the active profile
        // area to be updated.
        profiles->SelectMachines();
    }

    // Do the allowable nodes/procs
    if (currentLaunch && currentLaunch->GetParallel())
    {
        intVector nnodes, nprocs;
        for(int i = 0; i < allowableNodeProcs->rowCount(); ++i)
        {
            bool okay1 = false, okay2 = false;
            int nn = allowableNodeProcs->item(i, 0)->text().toInt(&okay1);
            int np = allowableNodeProcs->item(i, 1)->text().toInt(&okay2);
            if(nn < 0) nn = 1;
            if(np < 0) np = 1;
            if(okay1 && okay2)
            {
                nnodes.push_back(nn);
                nprocs.push_back(np);
            }
        }
        currentLaunch->SetAllowableNodes(nnodes);
        currentLaunch->SetAllowableProcs(nprocs);
    }

    return needNotify;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::Apply
//
// Purpose: 
//   This method is called when we want to apply the values from the window
//   to the state object.
//
// Arguments:
//   ignore : Whether to ignore the autoupdate flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep 24 09:20:47 PDT 2001
//
// Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//   
// ****************************************************************************

void
QvisHostProfileWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Update all of the values and call Notify.
        GetCurrentValues();
        subject->Notify();
    }
    else
    {
        subject->Notify();
    }
}

//
// Qt slot functions
//

// ****************************************************************************
// Method: QvisHostProfileWindow::apply
//
// Purpose: 
//   This is a Qt slot function that gets the current values for all
//   of the widgets in the window and then calls Notify to tell the
//   viewer.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 11:15:46 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:22:02 PDT 2001
//   Moved the guts into the Apply() method.
//
// ****************************************************************************

void
QvisHostProfileWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisHostProfileWindow::userNameChanged
//
// Purpose: 
//   This is a Qt slot function that sets the username for the active
//   host profile.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 12:38:39 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:16:12 PDT 2001
//   Prevented the window from being updated.
//
//    Jeremy Meredith, Mon Aug 18 13:36:20 PDT 2003
//    Made it apply without a return press, and 
//    renamed the method appropriately.
//
//    Jeremy Meredith, Thu Oct  9 15:48:43 PDT 2003
//    Made it apply to all profiles with the same host.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::userNameChanged(const QString &u)
{
    if (currentMachine == NULL)
        return;

    if (u.isEmpty())
        return;

    currentMachine->SetUserName(u.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::processDirectoryText
//
// Purpose:
//   Callback when directory text changes.
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************

void
QvisHostProfileWindow::processDirectoryText(const QString &d)
{
    if (currentMachine == NULL)
        return;

    if (d.isEmpty())
        return;

    currentMachine->SetDirectory(d.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleLaunch
//
// Purpose: 
//   This is a Qt slot function that enables the launch method widget.
//
// Programmer: Jeremy Meredith
// Creation:   July 17, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:26:59 PDT 2001
//   Changed Notify() to Apply().
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleLaunch(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetLaunchMethodSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processLaunchMethodText
//
// Purpose: 
//   This is a Qt slot function that sets the launch method for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   July 16, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:16:46 PDT 2001
//   Changed the code so we can update the window sensitivity without having
//   to update the entire window.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::launchMethodChanged(const QString &method)
{
    if (currentLaunch == NULL)
        return;

    QString tmp(method.trimmed());
    if (tmp == tr("(default"))
        currentLaunch->SetLaunchMethod("");
    else
        currentLaunch->SetLaunchMethod(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::numProcessorsChanged
//
// Purpose: 
//   This is a Qt slot function that sets the number of processors
//   for the active host profile.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 12:40:20 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:16:46 PDT 2001
//   Changed the code so we can update the window sensitivity without having
//   to update the entire window.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::numProcessorsChanged(int value)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetNumProcessors(value);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::timeoutChanged
//
// Purpose: 
//   This is a Qt slot function that sets the timeout for the active host
//   profile.
//
// Programmer: Sean Ahern
// Creation:   Thu Feb 21 15:55:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::timeoutChanged(int value)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetTimeout(value);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::threadsChanged
//
// Purpose: 
//   This is a Qt slot function that sets the threads for the active host
//   profile.
//
// Programmer: David Camp
// Creation:   Thu Jul 31 08:50:40 PDT 2014
//
// Modifications:
//
// ****************************************************************************
void
QvisHostProfileWindow::threadsChanged(int value)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetNumThreads(value);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleNumNodes
//
// Purpose: 
//   This is a Qt slot function that enables the NumNodes widget.
//
// Programmer: Jeremy Meredith
// Creation:   July 16, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:28:33 PDT 2001
//   Prevented the window from updating.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleNumNodes(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetNumNodesSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::numNodesChanged
//
// Purpose: 
//   This is a Qt slot function that sets the number of nodes
//   for the active host profile.
//
// Programmer: Jeremy Meredith
// Creation:   July 16, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:29:00 PDT 2001
//   Prevented the window from updating.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::numNodesChanged(int n)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetNumNodes(n);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::togglePartitionName
//
// Purpose: 
//   This is a Qt slot function that enables the PartitionName widget.
//
// Programmer: Jeremy Meredith
// Creation:   July 16, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:29:16 PDT 2001
//   Prevented the window from updating.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::togglePartitionName(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetPartitionSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processPartitionNameText
//
// Purpose: 
//   This is a Qt slot function that sets the partition name for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   July 16, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:29:16 PDT 2001
//   Prevented the window from updating.
//   
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processPartitionNameText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetPartition(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleBankName
//
// Purpose: 
//   This is a Qt slot function that enables the BankName widget.
//
// Programmer: Jeremy Meredith
// Creation:   July 22, 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleBankName(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetBankSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processBankNameText
//
// Purpose: 
//   This is a Qt slot function that sets the bank name for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   July 22, 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processBankNameText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetBank(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleTimeLimit
//
// Purpose: 
//   This is a Qt slot function that enables the TimeLimit widget.
//
// Programmer: Jeremy Meredith
// Creation:   July 22, 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleTimeLimit(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetTimeLimitSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processTimeLimitText
//
// Purpose: 
//   This is a Qt slot function that sets the timeLimit name for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   July 22, 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processTimeLimitText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetTimeLimit(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleMachinefile
//
// Purpose: 
//   This is a Qt slot function that enables the Machinefile widget.
//
// Programmer: Jeremy Meredith
// Creation:   September 15, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleMachinefile(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetMachinefileSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processMachinefileText
//
// Purpose: 
//   This is a Qt slot function that sets the machinefile name for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   September 15, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processMachinefileText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetMachinefile(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleLaunchArgs
//
// Purpose: 
//   This is a Qt slot function that enables the launchArgs widget.
//
// Programmer: Jeremy Meredith
// Creation:   January 24, 2003
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleLaunchArgs(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetLaunchArgsSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processLaunchArgsText
//
// Purpose: 
//   This is a Qt slot function that sets the launch args for the active
//   host profile.
//
// Programmer: Jeremy Meredith
// Creation:   January 24, 2003
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processLaunchArgsText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetLaunchArgs(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleSublaunchArgs
//
// Purpose: 
//   This is a Qt slot function that enables the sublaunchArgs widget.
//
// Programmer: Eric Brugger
// Creation:   February 15, 2007
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleSublaunchArgs(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchArgsSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processSublaunchArgsText
//
// Purpose: 
//   This is a Qt slot function that sets the sublaunch args for the active
//   host profile.
//
// Programmer: Eric Brugger
// Creation:   February 15, 2007
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processSublaunchArgsText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchArgs(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleSublaunchPreCmd
//
// Purpose: 
//   This is a Qt slot function that enables the sublaunchPreCmd widget.
//
// Programmer: Dave Bremer
// Creation:   April 16, 2008
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleSublaunchPreCmd(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchPreCmdSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processSublaunchPreCmdText
//
// Purpose: 
//   This is a Qt slot function that sets the sublaunch pre-mpi command for 
//   the active host profile.
//
// Programmer: Dave Bremer
// Creation:   April 16, 2008
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processSublaunchPreCmdText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchPreCmd(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleSublaunchPostCmd
//
// Purpose: 
//   This is a Qt slot function that enables the sublaunchPostCmd widget.
//
// Programmer: Dave Bremer
// Creation:   April 16, 2008
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleSublaunchPostCmd(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchPostCmdSet(state);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processSublaunchPostCmdText
//
// Purpose: 
//   This is a Qt slot function that sets the sublaunch post-mpi command for 
//   the active host profile.
//
// Programmer: Dave Bremer
// Creation:   April 16, 2008
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//   
// ****************************************************************************

void
QvisHostProfileWindow::processSublaunchPostCmdText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetSublaunchPostCmd(tmp.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleParallel
//
// Purpose: 
//   This is a Qt slot function that enables parallelism.
//
// Programmer: Jeremy Meredith
// Creation:   July 24, 2002
//
// Modifications:
//    Jeremy Meredith, Mon Apr 14 18:26:49 PDT 2003
//    Added code to set the current values before doing the toggle; toggling
//    the button is liable to cause updates that overwrite some old values.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleParallel(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetParallel(state);
    SetUpdate(false);
    Apply();
    UpdateLaunchProfile();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::loadBalancingChanged
//
// Purpose:
//   Qt slot function to change the type of load balancing.
//
// Arguments:
//   val        new type of load balancing (0=auto 1=static 2=dynamic)
//
// Programmer:  Jeremy Meredith
// Creation:    September 21, 2001
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:29:16 PDT 2001
//   Prevented the window from updating.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void 
QvisHostProfileWindow::loadBalancingChanged(int val)
{
    if (currentLaunch == NULL)
        return;

    switch (val)
    {
      case 0:
        currentLaunch->SetForceStatic(false);
        currentLaunch->SetForceDynamic(false);
        break;
      case 1:
        currentLaunch->SetForceStatic(true);
        currentLaunch->SetForceDynamic(false);
        break;
      case 2:
        currentLaunch->SetForceStatic(false);
        currentLaunch->SetForceDynamic(true);
        break;
    }

    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::hostNameChanged
//
// Purpose: 
//   This is a slot function that sets the host name for the current profile.
//
// Programmer: Jeremy Meredith
// Creation:   February 13, 2002
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Don't assume list index is same as host profile list index.  Add user
//   data to store the item's actual index in the host profile list.
//
// ****************************************************************************

void
QvisHostProfileWindow::hostNameChanged(const QString &n)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetHost(n.toStdString());

    HostProfileList *profiles = (HostProfileList *)subject;
    for (int i=0; i<profiles->GetNumMachines(); i++)
    {
        if ((MachineProfile*)profiles->GetMachines()[i] == currentMachine)
        {
            QString tmp;
            if (profiles->GetMachines(i).GetHostNickname() != "")
                tmp = currentMachine->GetHostNickname().c_str();
            else
                tmp = currentMachine->GetHost().c_str();

            for (int j=0; j<hostList->count(); j++)
            {
                if (hostList->item(j)->data(Qt::UserRole).toInt() == i)
                {
                    hostList->item(j)->setText(tmp);
                    break;
                }
            }
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::hostAliasesChanged
//
// Purpose: 
//   This is a slot function that sets the host aliases for the current
//   profile.
//
// Programmer: Jeremy Meredith
// Creation:   April 14, 2003
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::hostAliasesChanged(const QString &aliases)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetHostAliases(aliases.toStdString());
}

// ****************************************************************************
// Method: QvisHostProfileWindow::hostNicknameChanged
//
// Purpose: 
//   This is a slot function that sets the host nickname for the current
//   profile.
//
// Programmer: Hank Childs
// Creation:   May 7, 2009
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Don't assume list index is same as host profile list index.  Add user
//   data to store the item's actual index in the host profile list.
//
// ****************************************************************************

void
QvisHostProfileWindow::hostNicknameChanged(const QString &nickname)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetHostNickname(nickname.toStdString());

    HostProfileList *profiles = (HostProfileList *)subject;
    for (int i=0; i<profiles->GetNumMachines(); i++)
    {
        if ((MachineProfile*)profiles->GetMachines()[i] == currentMachine)
        {
            QString tmp;
            if (profiles->GetMachines(i).GetHostNickname() != "")
                tmp = currentMachine->GetHostNickname().c_str();
            else
                tmp = currentMachine->GetHost().c_str();

            for (int j=0; j<hostList->count(); j++)
            {
                if (hostList->item(j)->data(Qt::UserRole).toInt() == i)
                {
                    hostList->item(j)->setText(tmp);
                    break;
                }
            }
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::processEngineArgumentsText
//
// Purpose: 
//   This is a Qt slot function that sets the engine arguments for
//   the active host profile.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 12:39:36 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Sep 24 09:12:12 PDT 2001
//   I changed how the state is read.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::processEngineArgumentsText(const QString &tmp)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetArguments(
        StringHelpers::split(tmp.simplified().toStdString(),' '));
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisHostProfileWindow::toggleShareMDServer
//
// Purpose: 
//   This is a Qt slot function that is activated when the Share MDServer
//   check box is toggled.
//
// Programmer: Jeremy Meredith
// Creation:   May 16, 2003
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//   Brad Whitlock, Thu Dec  1 11:41:23 PST 2011
//   Indicate that sharing a batch job does not work with ssh tunneling 
//   right now.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleShareMDServer(bool state)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetShareOneBatchJob(state);

#ifdef SHARE_BATCH_JOB_HOST_ISSUES
    // Sharing a batch job is currently incompatible with some options.
    // Set them now to make it clear to the user.
    currentMachine->SetClientHostDetermination(MachineProfile::MachineName);
    currentMachine->SetTunnelSSH(false);
    clientHostNameMethod->blockSignals(true);
    clientHostNameMethod->button(0)->setChecked(true);
    clientHostNameMethod->blockSignals(false);
    UpdateWindowSensitivity();
#endif

    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisHostProfileWindow::toggleUseVisItScriptForEnv
//
// Purpose: 
//   This is a Qt slot function that is activated when the Use VisIt
//   to Set up Environment check box is toggled.
//
// Programmer: Jeremy Meredith
// Creation:   September 15, 2005
//
// Modifications:
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleUseVisItScriptForEnv(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetVisitSetsUpEnv(state);
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
//  Method:  QvisHostProfileWindow::processProfileNameText
//
//  Purpose:
//    Updates the profile name whenever text is changed.  This is needed
//    because of the susceptibility of this field to being changed
//    inadvertently without frequent updates.
//
//  Arguments:
//    name       the new profile name
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 21, 2002
//
//  Modifications:
//    Jeremy Meredith, Mon Apr 14 18:29:21 PDT 2003
//    Removed the shotgun-method call to UpdateProfileList.  It was causing
//    stability problems, and all I really needed to do was update some
//    text in a list box.  That's exactly what I'm now doing.
//
//    Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//    Initial Qt4 Port.
//
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::processProfileNameText(const QString &name)
{
    if (currentMachine == NULL || currentLaunch == NULL)
        return;

    QString temp = profileName->displayText();
    if (!temp.isEmpty())
    {
        currentLaunch->SetProfileName(temp.toStdString());
        for (int i=0; i<currentMachine->GetNumLaunchProfiles(); i++)
        {
            if ((LaunchProfile*)currentMachine->GetLaunchProfiles()[i] ==
                                                                currentLaunch)
            {
                profileList->item(i)->setText(temp);
                break;
            }
        }
    }
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleSSHPort
//
//  Purpose:
//    Change the flag to use the specified ssh port for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//    state      true to use the specified port, false to use the default (22)
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::toggleSSHPort(bool state)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetSshPortSpecified(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::sshPortChanged
//
//  Purpose:
//    Change the remote ssh port for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//    portStr   the string indicating the port value
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2003
//
//  Modifications:
//    Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//    Initial Qt4 Port.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::sshPortChanged(const QString &portStr)
{
    if (currentMachine == NULL)
        return;

    if (portStr.isEmpty())
        return;

    int port = atoi(portStr.toStdString().c_str());

    currentMachine->SetSshPort(port);
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleSSHCommand
//
//  Purpose:
//    Change the flag to use the specified ssh command for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//    state      true to use the specified command, false to use the default
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Aug 15 14:15:53 PDT 2012
//
//  Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleSSHCommand(bool state)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetSshCommandSpecified(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::sshCommandRetPressed
//
//  Purpose:
//    Change the remote ssh command for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//
//  Programmer:  Brad Whitlock
//  Creation:    Wed Aug 15 14:16:42 PDT 2012
//
//  Modifications:
//    Kathleen Biagas, Wed Dec 16 11:07:43 MST 2015
//    Changed name to sshCommandRetPressed, removed arg.
//    Ensure quoted command is preserved, split args on ' ' only after the
//    end of the quoted command.
//
// ****************************************************************************

void
QvisHostProfileWindow::sshCommandRetPressed()
{
    if (currentMachine == NULL)
        return;

    QString s(sshCommand->text());
    stringVector newCommand;
    // preserve surrounding quotes if present
    if (s.startsWith('\"'))
    {
        if (s.endsWith('\"'))
            newCommand.push_back(s.toStdString());
        else
        {
            // split into command and args.
            int pos = s.indexOf("\"", 1);
            QString cmd(s.left(pos+1));
            newCommand.push_back(cmd.toStdString());
            QString args(s.right(s.size()-pos-2));
            QStringList arglist(args.split(' '));
            for(int i = 0; i < arglist.size(); ++i)
                newCommand.push_back(arglist[i].toStdString());
        }
    }
    else
    {
        QStringList cmd(s.split(' '));
        for(int i = 0; i < cmd.size(); ++i)
            newCommand.push_back(cmd[i].toStdString());
    }

    currentMachine->SetSshCommand(newCommand);
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleUseGateway
//
//  Purpose:
//    Change the flag to use the use the gateway host for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//    state      true to use the gateway host, false to not use it
//
//  Programmer:  Eric Brugger
//  Creation:    April 19, 2011
//
//  Modifications:
//
// ****************************************************************************
void
QvisHostProfileWindow::toggleUseGateway(bool state)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetUseGateway(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::gatewayHostChanged
//
//  Purpose:
//    Change the gateway host name for all profiles with the
//    same remote host name based on a changed widget value.
//
//  Arguments:
//    hostStr   the string containing the host name
//
//  Programmer:  Eric Brugger
//  Creation:    April 19, 2011
//
//  Modifications:
//
// ****************************************************************************
void
QvisHostProfileWindow::gatewayHostChanged(const QString &hostStr)
{
    if (currentMachine == NULL)
        return;

    if (hostStr.isEmpty())
        return;

    // int port = atoi(portStr.toStdString().c_str());

    currentMachine->SetGatewayHost(hostStr.toStdString());
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::clientHostNameMethodChanged
//
//  Purpose:
//    Change the client host name determination method for all profiles
//    with the same remote host name based on a changed widget value.
//
//  Arguments:
//    m          the index of the new method in the button group
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::clientHostNameMethodChanged(int m)
{
    if (currentMachine == NULL)
        return;

    switch (m)
    {
      case 0:
        currentMachine->SetClientHostDetermination(MachineProfile::MachineName);
        break;
      case 1:
        currentMachine->SetClientHostDetermination(MachineProfile::ParsedFromSSHCLIENT);
        break;
      case 2:
        currentMachine->SetClientHostDetermination(MachineProfile::ManuallySpecified);
        break;
    }
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::clientHostNameChanged
//
//  Purpose:
//    Change the manually specified client host name for all profiles
//    with the same remote host name based on a changed widget value.
//
//  Arguments:
//    h          the new host name
//
//  Programmer:  Jeremy Meredith
//  Creation:    October  9, 2003
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::clientHostNameChanged(const QString &h)
{
    if (currentMachine == NULL)
        return;

    if (h.isEmpty())
        return;

    currentMachine->SetManualClientHostName(h.toStdString());
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleTunnelSSH
//
// Purpose: 
//   This is a Qt slot function that is activated when the tunnel SSH
//   check box is toggled.
//
// Programmer: Jeremy Meredith
// Creation:   May 22, 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 11:26:37 PDT 2007
//   I made it apply to all profiles for a host.
//
//   Jeremy Meredith, Thu Jun 28 13:20:48 EDT 2007
//   Force host name determination method to default values when tunneling
//   is enabled.  The two are incompatible.
//
//   Cyrus Harrison, Wed Jun 25 11:01:46 PDT 2008
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
//   Brad Whitlock, Thu Dec  1 11:41:23 PST 2011
//   Indicate that sharing a batch job does not work with ssh tunneling 
//   right now.
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleTunnelSSH(bool tunnel)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetTunnelSSH(tunnel);
    if (tunnel)
    {
        // Tunneling is incomaptible with some options.  Set them now
        // to make it clear to the user.
        currentMachine->SetClientHostDetermination(MachineProfile::MachineName);
        currentMachine->SetManualClientHostName("");
#ifdef SHARE_BATCH_JOB_HOST_ISSUES
        currentMachine->SetShareOneBatchJob(false);
        shareMDServerCheckBox->blockSignals(true);
        shareMDServerCheckBox->setChecked(false);
        shareMDServerCheckBox->blockSignals(false);
#endif
        clientHostNameMethod->blockSignals(true);
        clientHostName->blockSignals(true);
        clientHostNameMethod->button(0)->setChecked(true);
        clientHostName->setText("");
        clientHostNameMethod->blockSignals(false);
        clientHostName->blockSignals(false);
    }

    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleXArgs
//
//  Purpose:
//      Toggles where the user has X arguments to pass.
//
//  Arguments:
//      on       True if we can, false if we can't.
//
//  Programmer:  Tom Fogal
//  Creation:    May 6, 2011
//
//  Modifications:
// ****************************************************************************
void QvisHostProfileWindow::toggleXArgs(bool on)
{
    if(NULL == currentLaunch) { return; }

    txtXArgs->blockSignals(true);
      txtXArgs->setEnabled(on);
    txtXArgs->blockSignals(false);

    if(false == on)
    {
        currentLaunch->SetXArguments(std::string(""));
        SetUpdate(false);
        Apply();
    }
    // ignore the on/true case: we'll update currentLaunch when the
    // associated LineEdit changes.
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::xArgsChanged
//
//  Purpose:
//      Grab the X arguments when the user modifies them.
//
//  Arguments:
//      args     The arguments for the X server.
//
//  Programmer:  Tom Fogal
//  Creation:    May 6, 2011
//
//  Modifications:
// ****************************************************************************
void QvisHostProfileWindow::xArgsChanged(const QString& args)
{
    if(NULL == currentLaunch || args.isEmpty()) { return; }

    currentLaunch->SetXArguments(args.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleLaunchX
//
//  Purpose:
//      Set whether or not VisIt should launch the X servers.
//
//  Arguments:
//      on       true if VisIt should launch X servers
//
//  Programmer:  Tom Fogal
//  Creation:    May 6, 2011
//
//  Modifications:
// ****************************************************************************
void QvisHostProfileWindow::toggleLaunchX(bool on)
{
    if(NULL == currentLaunch) { return; }

    currentLaunch->SetLaunchXServers(on);

    cbXArgs->blockSignals(true);
    txtXArgs->blockSignals(true);
      cbXArgs->setEnabled(on);
      txtXArgs->setEnabled(on);
    txtXArgs->blockSignals(false);
    cbXArgs->blockSignals(false);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::xDisplayChanged
//
//  Purpose:
//      Grab the X display setting when the user changes it
//
//  Arguments:
//      display  the new display
//
//  Programmer:  Tom Fogal
//  Creation:    May 6, 2011
//
//  Modifications:
// ****************************************************************************
void QvisHostProfileWindow::xDisplayChanged(const QString& display)
{
    if(NULL == currentLaunch) { return; }

    currentLaunch->SetXDisplay(display.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::toggleCanDoHW
//
//  Purpose:
//      Toggles whether or not we can do hardware acceleration.
//
//  Arguments:
//      state    True if we can, false if we can't.
//
//  Programmer:  Hank Childs
//  Creation:    December 2, 2005
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 18 15:25:27 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile. Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::toggleCanDoHW(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetCanDoHWAccel(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::nGPUsChanged
//
//  Purpose:
//      Notification that the user changed the number of GPUs in use.
//
//  Arguments:
//    unused, required by Qt prototype.
//
//  Programmer:  Tom Fogal
//  Creation:    May 5, 2011
//
//  Modifications:
// ****************************************************************************
void
QvisHostProfileWindow::nGPUsChanged(const QString&)
{
    if(NULL == currentLaunch) { return; }

    currentLaunch->SetGPUsPerNode(sbNGPUs->value());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::currentHostChanged
//
// Purpose:
//   Callback when user selects a different host in the list widget.
//   Find the selected profile, save it off, and update the window if needed.
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Don't assume list index is same as host profile list index.  Add user
//   data to store the item's actual index in the host profile list.
//
// ****************************************************************************
void
QvisHostProfileWindow::currentHostChanged()
{
    MachineProfile *oldMachine = currentMachine;
    currentMachine = NULL;
    HostProfileList *profiles = (HostProfileList *)subject;
    int nMachines = profiles->GetNumMachines();

    QList<QListWidgetItem*> sel = hostList->selectedItems();
    int machine = -1;
    if (sel.size() > 0)
        machine = sel[0]->data(Qt::UserRole).toInt();

    if (machine>=0 && machine<nMachines)
        currentMachine = (MachineProfile*)profiles->GetMachines()[machine];

    if (currentMachine != oldMachine)
    {
        UpdateMachineProfile();
        UpdateWindowSensitivity();
    }
}


// ****************************************************************************
// Method:  QvisHostProfileWindow::currentHostChanged
//
// Purpose:
//   Callback when user selects a different launch profile in the list widget.
//   Find the selected profile, save it off, and update the window if needed.
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************
void
QvisHostProfileWindow::currentLaunchChanged()
{
    LaunchProfile *oldLaunch = currentLaunch;
    currentLaunch = NULL;
    if (currentMachine == NULL)
    {
        UpdateLaunchProfile();
        UpdateWindowSensitivity();
        return;
    }
    int nProfiles = currentMachine->GetNumLaunchProfiles();

    QList<QListWidgetItem*> sel = profileList->selectedItems();
    int launch = -1;
    for (int i=0; i<sel.size() && launch<0; i++)
    {
        for (int j=0; j<nProfiles && launch<0; j++)
        {
            if (sel[i] == profileList->item(j))
                launch = j;
        }
    }

    if (launch>=0 && launch<nProfiles)
        currentLaunch = (LaunchProfile*)currentMachine->GetLaunchProfiles()[launch];

    if (currentLaunch != oldLaunch)
    {
        UpdateLaunchProfile();
        UpdateWindowSensitivity();
    }
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::addMachineProfile
//
// Purpose:
//   callback for "new" machine profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Fix the way we set the host nickname.
//
//   Brad Whitlock, Thu Oct 27 14:54:10 PDT 2011
//   Set focus on the first thing you'd edit.
//
// ****************************************************************************
void
QvisHostProfileWindow::addMachineProfile()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    profiles->AddMachines(MachineProfile());
    ReplaceLocalHost();
    currentMachine = (MachineProfile*)profiles->GetMachines()[
                                              profiles->GetNumMachines()-1];
    currentMachine->SetHostNickname(currentMachine->GetHost());
    Apply();

    hostNickname->selectAll();
    hostNickname->setFocus();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::delMachineProfile
//
// Purpose:
//   callback for "delete" machine profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************
void
QvisHostProfileWindow::delMachineProfile()
{
    if (currentMachine == NULL)
        return;

    HostProfileList *profiles = (HostProfileList *)subject;
    for (int i=0; i<profiles->GetNumMachines(); i++)
    {
        if ((MachineProfile*)profiles->GetMachines()[i] == currentMachine)
        {
            profiles->RemoveMachines(i);
        }
    }
    currentMachine = NULL;
    currentLaunch = NULL;
    Apply();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::exportMachineProfile
//
// Purpose:
//   callback for "export" machine profile button
//
// Arguments:
//   none
//
// Programmer:
// Creation:    September 10, 2013
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::exportMachineProfile()
{
    QModelIndex index = hostList->currentIndex();
    if(!index.isValid()) return;

    std::string text = index.data().toString().toStdString();

    std::string s = text;

    for (size_t j=0; j<s.length(); j++)
    {
        if (s[j]>='A' && s[j]<='Z')
            s[j] += int('a')-int('A');
        if ((s[j]<'a'||s[j]>'z') && (s[j]<'0'||s[j]>'9'))
            s[j] = '_';
    }

    s = "host_" + s + ".xml";
    GetViewerMethods()->ExportHostProfile(text, s, true);
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::copyMachineProfile
//
// Purpose:
//   callback for "copy" machine profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:16:35 EST 2010
//   Fix the way we set the host nickname.
//
// ****************************************************************************
void
QvisHostProfileWindow::copyMachineProfile()
{
    if (currentMachine == NULL)
        return;

    HostProfileList *profiles = (HostProfileList *)subject;
    profiles->AddMachines(*currentMachine);
    currentMachine = (MachineProfile*)profiles->GetMachines()[
                                              profiles->GetNumMachines()-1];
    currentMachine->SetHostNickname(std::string("Copy of ") +
                                    currentMachine->GetHostNickname());
    ReplaceLocalHost();
    Apply();
}


// ****************************************************************************
// Method:  QvisHostProfileWindow::addLaunchProfile
//
// Purpose:
//   callback for "new" launch profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:20:48 EST 2010
//   Set the newly added launch profile as the active one.
//
//   Brad Whitlock, Thu Oct 27 14:54:28 PDT 2011
//   Set focus on the first thing you'd edit.
//
// ****************************************************************************
void
QvisHostProfileWindow::addLaunchProfile()
{
    if (currentMachine == NULL)
        return;

    LaunchProfile lp;
    QString name(tr("New profile"));
    QString num; num.sprintf(" #%d", profileCounter++);
    name += num;
    lp.SetProfileName(name.toStdString());

    currentMachine->AddLaunchProfiles(lp);
    if (currentMachine->GetActiveProfile() <  0 ||
        currentMachine->GetActiveProfile() >=
                                       currentMachine->GetNumLaunchProfiles())
    {
        currentMachine->SetActiveProfile(0);
    }
    currentLaunch = (LaunchProfile*)currentMachine->GetLaunchProfiles()[
                                     currentMachine->GetNumLaunchProfiles()-1];
    Apply();

    profileName->selectAll();
    profileName->setFocus();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::delLaunchProfile
//
// Purpose:
//   callback for "delete" launch profile button
//
// Arguments:
//    none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************
void
QvisHostProfileWindow::delLaunchProfile()
{
    if (currentMachine == NULL || currentLaunch == NULL)
        return;

    int index = -1;
    for (int i=0; i<currentMachine->GetNumLaunchProfiles(); i++)
    {
        if ((LaunchProfile*)currentMachine->GetLaunchProfiles()[i] == currentLaunch)
        {
            index = i;
        }
    }

    if (index < 0)
        return;

    for (int i=index; i<currentMachine->GetNumLaunchProfiles()-1; i++)
    {
        currentMachine->GetLaunchProfiles(i)=currentMachine->GetLaunchProfiles(i+1);
    }
    if (currentMachine->GetActiveProfile() >= index)
        currentMachine->SetActiveProfile(currentMachine->GetActiveProfile()-1);
    currentMachine->GetLaunchProfiles().resize(currentMachine->GetNumLaunchProfiles()-1);
    currentMachine->SelectLaunchProfiles();
    currentLaunch = NULL;
    Apply();
}
    
// ****************************************************************************
// Method:  QvisHostProfileWindow::copyLaunchProfile
//
// Purpose:
//   callback for "copy" launch profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// Modifications:
//   Jeremy Meredith, Fri Feb 19 13:20:48 EST 2010
//   Set the newly added launch profile as the active one.
//
// ****************************************************************************
void
QvisHostProfileWindow::copyLaunchProfile()
{
    if (currentMachine == NULL || currentLaunch == NULL)
        return;
    LaunchProfile lp(*currentLaunch);
    lp.SetProfileName(std::string("Copy of ")+lp.GetProfileName());
    currentMachine->AddLaunchProfiles(lp);
    currentLaunch = (LaunchProfile*)currentMachine->GetLaunchProfiles()[
                                     currentMachine->GetNumLaunchProfiles()-1];
    Apply();
}

// ****************************************************************************
// Method:  QvisHostProfileWindow::makeDefaultLaunchProfile
//
// Purpose:
//   callback for "make default" launch profile button
//
// Arguments:
//   none
//
// Programmer:  Jeremy Meredith
// Creation:    February 18, 2010
//
// ****************************************************************************
void
QvisHostProfileWindow::makeDefaultLaunchProfile()
{
    if (currentMachine == NULL || currentLaunch == NULL)
        return;

    int index = -1;
    for (int i=0; i<currentMachine->GetNumLaunchProfiles(); i++)
    {
        if ((LaunchProfile*)currentMachine->GetLaunchProfiles()[i] == currentLaunch)
        {
            index = i;
        }
    }

    if (index < 0)
        return;

    for (int i=0; i<currentMachine->GetNumLaunchProfiles(); i++)
        profileList->item(i)->setCheckState(i==index ? Qt::Checked : Qt::Unchecked);
    currentMachine->SetActiveProfile(index);
    makeDefaultProfile->setEnabled(false);
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleUseMaxNodes
//
// Purpose: 
//   Called when we click on the max nodes check box.
//
// Arguments:
//   val : Whether we want to set the max # nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  6 11:15:23 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::toggleUseMaxNodes(bool val)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetMaximumNodesValid(val);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::maxNodesChanged
//
// Purpose: 
//   Set a new max # nodes.
//
// Arguments:
//   val : The new max # nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  6 11:16:24 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::maxNodesChanged(int val)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetMaximumNodes(val);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleUseMaxProcessors
//
// Purpose: 
//   Called when we click on the max processors check box.
//
// Arguments:
//   val : Whether we want to set the max # nodes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  6 11:15:23 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::toggleUseMaxProcessors(bool val)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetMaximumProcessorsValid(val);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::maxProcessorsChanged
//
// Purpose: 
//   Set a new max # processors.
//
// Arguments:
//   val : The new max # processors.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  6 11:16:24 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::maxProcessorsChanged(int val)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetMaximumProcessors(val);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::ResizeNodeProcs
//
// Purpose:
//   Sets the number of rows in the allowableNodeProcs table.
//
// Arguments:
//   newsize : the new size.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:43:01 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::ResizeNodeProcs(int newSize, bool blank)
{
    int nRows = allowableNodeProcs->rowCount();
    if (nRows < newSize)
    {
        allowableNodeProcs->setRowCount(newSize);
        // Try and get the last row if it exists and we'll double
        QString nnodes, nprocs;
        if(!blank)
        {
            nnodes = QString("1");
            nprocs = QString("16");

            if(nRows > 0)
            {
                nnodes = allowableNodeProcs->item(nRows-1, 0)->text();
                nprocs = allowableNodeProcs->item(nRows-1, 1)->text();
                bool okay = false;
                int innodes = nnodes.toInt(&okay);
                if(okay)
                {
                    innodes *= 2;
                    nnodes.setNum(innodes);
                }
                int inprocs = nprocs.toInt(&okay);
                if(okay)
                {
                    inprocs *= 2;
                     nprocs.setNum(inprocs);
                }
            }
        }

        for (int i = nRows; i < newSize; ++i)
        {
            allowableNodeProcs->setItem(i, 0, new QTableWidgetItem(nnodes));
            allowableNodeProcs->setItem(i, 1, new QTableWidgetItem(nprocs));
        }
    }
    else if (nRows > newSize)
    {
        for (int i = nRows-1; i >= newSize; --i)
        {
            allowableNodeProcs->removeRow(i);
        }
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::allowableNodeProcsAddRow
//
// Purpose:
//   This is a Qt slot function called when we want to add a row to the 
//   allowableNodeProcs table.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:43:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::allowableNodeProcsAddRow()
{
    ResizeNodeProcs(allowableNodeProcs->rowCount() + 1, false);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::allowableNodeProcsDeleteRow
//
// Purpose:
//   This is a Qt slot function called when we want to delete a row from the 
//   allowableNodeProcs table.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:43:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::allowableNodeProcsDeleteRow()
{
    allowableNodeProcs->removeRow(allowableNodeProcs->currentRow());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::toggleAllowableNodeProcs
//
// Purpose:
//   This is a Qt slot function called when we want to toggle whether allowable
//   node/proc constraints are to be used.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 22 16:43:46 PDT 2014
//
// Modifications:
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleAllowableNodeProcs(bool val)
{
    currentLaunch->SetConstrainNodeProcs(val);
    SetUpdate(false);
    Apply();
}
