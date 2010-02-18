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

#include <QvisHostProfileWindow.h>

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QWidget>

#include <snprintf.h>

#include <MachineProfile.h>
#include <LaunchProfile.h>
#include <HostProfileList.h>
#include <StringHelpers.h>
#include <ViewerProxy.h>

#include <cstdlib>

#define HOST_PROFILE_SPACING 2

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
//
// ****************************************************************************

void
QvisHostProfileWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(NULL);
    topLayout->addLayout(mainLayout);
    topLayout->setStretchFactor(mainLayout, 100);

    mainLayout->addWidget(new QLabel(tr("Hosts")), 0,0, 1,3);
    hostList = new QListWidget(central);
    hostList->setSortingEnabled(true);
    mainLayout->addWidget(hostList, 1,0, 1,3);
    connect(hostList, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentHostChanged()));

    addHost = new QPushButton(tr("New"), central);
    mainLayout->addWidget(addHost, 2,0);
    connect(addHost, SIGNAL(clicked()), this, SLOT(addMachineProfile()));

    delHost = new QPushButton(tr("Del"), central);
    mainLayout->addWidget(delHost, 2,1);
    connect(delHost, SIGNAL(clicked()), this, SLOT(delMachineProfile()));

    copyHost = new QPushButton(tr("Copy"), central);
    mainLayout->addWidget(copyHost, 2,2);
    connect(copyHost, SIGNAL(clicked()), this, SLOT(copyMachineProfile()));

    machineTabs = new QTabWidget(central);
    mainLayout->addWidget(machineTabs, 0,3, 3,1);

    CreateLaunchProfilesGroup();
    machineTabs->addTab(launchProfilesGroup, tr("Launch Profiles"));

    CreateMachineSettingsGroup();
    machineTabs->addTab(machineSettingsGroup, tr("Host Settings"));
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
// ****************************************************************************
void
QvisHostProfileWindow::CreateMachineSettingsGroup()
{
    machineSettingsGroup = new QWidget();
    QWidget *currentGroup = machineSettingsGroup;

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
    layout->setSpacing(6);
    tmpLayout->addStretch(5);
    
    layout->setColumnMinimumWidth(0,15);
    layout->setColumnStretch(0,0);
    layout->setColumnStretch(1,0);
    layout->setColumnStretch(2,0);
    layout->setColumnStretch(3,100);

    hostName = new QLineEdit(currentGroup);
    connect(hostName, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostNameChanged(const QString &)));
    hostNameLabel = new QLabel(tr("Remote host name"), currentGroup);
    layout->addWidget(hostNameLabel, row, 0, 1,2);
    layout->addWidget(hostName, row, 2, 1,2);
    row++;

    hostAliases = new QLineEdit(currentGroup);
    connect(hostAliases, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostAliasesChanged(const QString &)));
    hostAliasesLabel = new QLabel(tr("Host name aliases"), currentGroup);
    layout->addWidget(hostAliasesLabel, row, 0, 1,2);
    layout->addWidget(hostAliases, row, 2, 1,2);
    row++;

    hostNickname = new QLineEdit(currentGroup);
    connect(hostNickname, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostNicknameChanged(const QString &)));
    hostNicknameLabel = new QLabel(tr("Host nickname"), currentGroup);
    layout->addWidget(hostNicknameLabel, row, 0, 1,2);
    layout->addWidget(hostNickname, row, 2, 1,2);
    row++;

    userName = new QLineEdit(currentGroup);
    connect(userName, SIGNAL(textChanged(const QString &)),
            this, SLOT(userNameChanged(const QString &)));
    userNameLabel = new QLabel(tr("Username"), currentGroup);
    layout->addWidget(userNameLabel, row, 0, 1,2);
    layout->addWidget(userName, row, 2, 1,2);
    row++;

    shareMDServerCheckBox = new QCheckBox(tr("Share batch job with Metadata Server"),
                                          currentGroup);
    layout->addWidget(shareMDServerCheckBox, row,0, 1,4);
    connect(shareMDServerCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleShareMDServer(bool)));
    row++;

    tunnelSSH = new QCheckBox(tr("Tunnel data connections through SSH"), currentGroup);
    layout->addWidget(tunnelSSH, row,0, 1,4);
    connect(tunnelSSH, SIGNAL(toggled(bool)),
            this, SLOT(toggleTunnelSSH(bool)));
    row++;

    clientHostNameMethod = new QButtonGroup(currentGroup);
    connect(clientHostNameMethod, SIGNAL(buttonClicked(int)),
            this, SLOT(clientHostNameMethodChanged(int)));
    chnMachineName = new QRadioButton(tr("Use local machine name"), currentGroup);
    chnParseFromSSHClient = new QRadioButton(tr("Parse from SSH_CLIENT environment variable"),
                                             currentGroup);
    chnSpecifyManually = new QRadioButton(tr("Specify manually:"),
                                          currentGroup);
    chnMachineName->setChecked(true);
    clientHostNameMethod->addButton(chnMachineName,0);

    clientHostNameMethod->addButton(chnParseFromSSHClient,1);
    clientHostNameMethod->addButton(chnSpecifyManually,2);
    clientHostNameMethodLabel =
        new QLabel(tr("Method used to determine local host name when not tunneling:"),
                   currentGroup);
    layout->addWidget(clientHostNameMethodLabel,
                                  row, 0, 1, 4);
    row++;
    layout->addWidget(chnMachineName, row, 1, 1, 3);
    row++;
    layout->addWidget(chnParseFromSSHClient, row, 1, 1, 3);
    row++;
    layout->addWidget(chnSpecifyManually, row, 1, 1, 1);
    
    clientHostName = new QLineEdit(currentGroup);
    connect(clientHostName, SIGNAL(textChanged(const QString &)),
            this, SLOT(clientHostNameChanged(const QString &)));
    layout->addWidget(clientHostName, row, 2, 1,2);
    row++;

    sshPort = new QLineEdit(currentGroup);
    sshPortCheckBox = new QCheckBox(tr("Specify SSH port"), currentGroup);
    connect(sshPort, SIGNAL(textChanged(const QString &)),
            this, SLOT(sshPortChanged(const QString &)));
    connect(sshPortCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSSHPort(bool)));
    layout->addWidget(sshPortCheckBox, row, 0, 1, 2);
    layout->addWidget(sshPort, row, 2, 1, 2);
    row++;

    directory = new QLineEdit(currentGroup);
    connect(directory, SIGNAL(textChanged(const QString &)),
            this, SLOT(processDirectoryText(const QString &)));
    directoryLabel = new QLabel(tr("Path to VisIt installation"),
                                      currentGroup);
    layout->addWidget(directoryLabel, row, 0, 1,2);
    layout->addWidget(directory, row, 2, 1,2);
    row++;

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
// ****************************************************************************
void
QvisHostProfileWindow::CreateLaunchProfilesGroup()
{
    launchProfilesGroup = new QWidget();
    QWidget *currentGroup = launchProfilesGroup;

    int row = 0;
    QGridLayout *layout = new QGridLayout(launchProfilesGroup);

    profileHeaderLabel = new QLabel("", launchProfilesGroup);
    layout->addWidget(profileHeaderLabel, row,0, 1,4);
    row++;

    profileList = new QListWidget(launchProfilesGroup);
    layout->addWidget(profileList, row,0, 1,4);
    connect(profileList, SIGNAL(itemSelectionChanged()),
            this, SLOT(currentLaunchChanged()));
    row++;

    addProfile = new QPushButton(tr("New"), launchProfilesGroup);
    layout->addWidget(addProfile, row,0);
    connect(addProfile, SIGNAL(clicked()), this, SLOT(addLaunchProfile()));
    delProfile = new QPushButton(tr("Del"), launchProfilesGroup);
    connect(delProfile, SIGNAL(clicked()), this, SLOT(delLaunchProfile()));
    layout->addWidget(delProfile, row,1);
    copyProfile = new QPushButton(tr("Copy"), launchProfilesGroup);
    connect(copyProfile, SIGNAL(clicked()), this, SLOT(copyLaunchProfile()));
    layout->addWidget(copyProfile, row,2);
    makeDefaultProfile = new QPushButton(tr("Make Default"),
                                         launchProfilesGroup);
    connect(makeDefaultProfile, SIGNAL(clicked()), this, SLOT(makeDefaultLaunchProfile()));
    layout->addWidget(makeDefaultProfile, row,3);
    row++;

    profileTabs = new QTabWidget(central);
    layout->addWidget(profileTabs, row,0, 1,4);
    row++;

    CreateBasicSettingsGroup();
    profileTabs->addTab(basicSettingsGroup, tr("Settings"));

    CreateLaunchSettingsGroup();
    profileTabs->addTab(launchSettingsGroup, tr("Parallel"));

    CreateAdvancedSettingsGroup();
    profileTabs->addTab(advancedSettingsGroup, tr("Advanced"));

    CreateHWAccelSettingsGroup();
    profileTabs->addTab(hwAccelSettingsGroup, tr("GPU Accel"));
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
// ****************************************************************************
void
QvisHostProfileWindow::CreateBasicSettingsGroup()
{
    basicSettingsGroup = new QWidget();
    QWidget *currentGroup = basicSettingsGroup;

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    QGridLayout *layout = new QGridLayout();
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
    timeout->setRange(1, 1440);
    timeout->setSingleStep(1);
    connect(timeout, SIGNAL(valueChanged(int)),
            this, SLOT(timeoutChanged(int)));
    timeoutLabel = new QLabel(tr("Timeout (minutes)"), currentGroup);
    layout->addWidget(timeoutLabel, row, 0, 1,1);
    layout->addWidget(timeout, row, 1, 1,3);
    row++;

    engineArguments = new QLineEdit(currentGroup);
    connect(engineArguments, SIGNAL(textChanged(const QString &)),
            this, SLOT(processEngineArgumentsText(const QString &)));
    engineArgumentsLabel = new QLabel(tr("Additional arguments"),
                                      currentGroup);
    layout->addWidget(engineArgumentsLabel, row, 0, 1,1);
    layout->addWidget(engineArguments, row, 1, 1,3);
    row++;

    parallelCheckBox = new QCheckBox(
                     tr("Launch (parallel) engine using job control system"),
                                     currentGroup);
    connect(parallelCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleParallel(bool)));
    layout->addWidget(parallelCheckBox, row, 0, 1,4);
    row++;
    
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
// ****************************************************************************
void
QvisHostProfileWindow::CreateLaunchSettingsGroup()
{
    launchSettingsGroup = new QWidget();
    QWidget *currentGroup = launchSettingsGroup;

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
    layout->setSpacing(HOST_PROFILE_SPACING);
    tmpLayout->addStretch(5);
    
    launchMethod = new QComboBox(currentGroup);
    launchMethod->addItem(tr("(default)"));
    launchMethod->addItem("bsub");
    launchMethod->addItem("dmpirun");
    launchMethod->addItem("ibrun");
    launchMethod->addItem("mpirun");
    launchMethod->addItem("msub");
    launchMethod->addItem("poe");
    launchMethod->addItem("prun");
    launchMethod->addItem("psub");
    launchMethod->addItem("srun");
    launchMethod->addItem("yod");
    launchMethod->addItem("msub/srun");
    launchMethod->addItem("psub/mpirun");
    launchMethod->addItem("psub/poe");
    launchMethod->addItem("psub/srun");
    launchMethod->addItem("qsub/aprun");
    launchMethod->addItem("qsub/ibrun");
    launchMethod->addItem("qsub/mpiexec");
    launchMethod->addItem("qsub/mpirun");
    launchMethod->addItem("qsub/srun");
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

    numProcessors = new QSpinBox(currentGroup);
    numProcessors->setRange(1,99999);
    numProcessors->setSingleStep(1);
    connect(numProcessors, SIGNAL(valueChanged(int)),
            this, SLOT(numProcessorsChanged(int)));
    numProcLabel = new QLabel(tr("Default number of processors"), currentGroup);
    layout->addWidget(numProcLabel, row, 0);
    layout->addWidget(numProcessors, row, 1);
    row++;

    numNodes = new QSpinBox(currentGroup);
    numNodes->setRange(1,99999);
    numNodes->setSingleStep(1);
    
    connect(numNodes, SIGNAL(valueChanged(int)),
            this, SLOT(numNodesChanged(int)));
    numNodesCheckBox = new QCheckBox(tr("Default number of nodes"), currentGroup);
    connect(numNodesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleNumNodes(bool)));
    layout->addWidget(numNodesCheckBox, row, 0);
    layout->addWidget(numNodes, row, 1);
    row++;

    bankName = new QLineEdit(currentGroup);
    connect(bankName, SIGNAL(textChanged(const QString &)),
            this, SLOT(processBankNameText(const QString &)));
    bankCheckBox = new QCheckBox(tr("Default Bank / Account"),currentGroup);
    connect(bankCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleBankName(bool)));
    layout->addWidget(bankCheckBox, row, 0);
    layout->addWidget(bankName, row, 1);
    row++;

    timeLimit = new QLineEdit(currentGroup);
    connect(timeLimit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processTimeLimitText(const QString &)));
    timeLimitCheckBox = new QCheckBox(tr("Default Time Limit"), currentGroup);
    connect(timeLimitCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleTimeLimit(bool)));
    layout->addWidget(timeLimitCheckBox, row, 0);
    layout->addWidget(timeLimit, row, 1);
    row++;

    machinefile = new QLineEdit(currentGroup);
    connect(machinefile, SIGNAL(textChanged(const QString &)),
            this, SLOT(processMachinefileText(const QString &)));
    machinefileCheckBox = new QCheckBox(tr("Default Machine File"), currentGroup);
    connect(machinefileCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleMachinefile(bool)));
    layout->addWidget(machinefileCheckBox, row, 0);
    layout->addWidget(machinefile, row, 1);
    row++;
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
// ****************************************************************************
void
QvisHostProfileWindow::CreateAdvancedSettingsGroup()
{
    advancedSettingsGroup = new QWidget();
    QWidget *currentGroup = advancedSettingsGroup;

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
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
// ****************************************************************************
void
QvisHostProfileWindow::CreateHWAccelSettingsGroup()
{
    hwAccelSettingsGroup = new QWidget();
    QWidget *currentGroup = hwAccelSettingsGroup;

    int row = 0;
    QVBoxLayout *tmpLayout = new QVBoxLayout(currentGroup);
    QGridLayout *layout = new QGridLayout();
    tmpLayout->addLayout(layout);
    layout->setSpacing(HOST_PROFILE_SPACING);
    tmpLayout->addStretch(5);

    QString str1(
       QString("<i>") + 
       tr("These options are for hardware accelerating the scalable rendering "
          "feature on a parallel cluster. In other modes, VisIt will automatically "
          "use hardware acceleration. This tab only needs to be modified for "
          "parallel clusters that have graphics cards.") +
       QString("</i>"));

    hwdisclaimer = new QLabel(str1, currentGroup);
    hwdisclaimer->setWordWrap(true);
    layout->addWidget(hwdisclaimer, row, 0, 1, 2);
    row++;

    canDoHW = new QCheckBox(tr("Use cluster's graphics cards"), currentGroup);
    connect(canDoHW, SIGNAL(toggled(bool)),
            this, SLOT(toggleCanDoHW(bool)));
    layout->addWidget(canDoHW, row, 0, 1, 2);
    row++;

    preCommand = new QLineEdit(currentGroup);
    preCommandCheckBox = new QCheckBox(tr("Pre-command"), currentGroup);
    connect(preCommand, SIGNAL(textChanged(const QString &)),
            this, SLOT(preCommandChanged(const QString &)));
    connect(preCommandCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePreCommand(bool)));
    layout->addWidget(preCommandCheckBox, row, 0, 1, 1);
    layout->addWidget(preCommand, row, 1, 1, 1);
    row++;

    postCommand = new QLineEdit(currentGroup);
    postCommandCheckBox = new QCheckBox(tr("Post-command"), currentGroup);
    connect(postCommand, SIGNAL(textChanged(const QString &)),
            this, SLOT(postCommandChanged(const QString &)));
    connect(postCommandCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePostCommand(bool)));
    layout->addWidget(postCommandCheckBox, row, 0, 1, 1);
    layout->addWidget(postCommand, row, 1, 1, 1);
    row++;



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
    currentMachine = false;
    hostList->blockSignals(false);
    for (int i=0; i<nMachines; i++)
    {
        MachineProfile *mp = (MachineProfile*)profiles->GetMachines()[i];
        if (mp->GetHostNickname() != "")
            hostList->addItem(mp->GetHostNickname().c_str());
        else
            hostList->addItem(mp->GetHost().c_str());
        if (oldMachine == mp)
        {
            hostList->blockSignals(true);
            hostList->item(i)->setSelected(true);
            hostList->blockSignals(false);
            currentMachine = oldMachine;
        }
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
    sshPortCheckBox->blockSignals(true);
    sshPort->blockSignals(true);
    tunnelSSH->blockSignals(true);
    directory->blockSignals(true);
    shareMDServerCheckBox->blockSignals(true);

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
        sshPortCheckBox->setChecked(false);
        sshPort->setText("");
        tunnelSSH->setChecked(false);
        shareMDServerCheckBox->setChecked(false);

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
        hostAliases->setText(mp.GetHostAliases().c_str());
        hostNickname->setText(mp.GetHostNickname().c_str());
        if(mp.GetUserName() == "notset")
            userName->setText(GetViewerProxy()->GetLocalUserName().c_str());
        else
            userName->setText(mp.GetUserName().c_str());
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
        sshPortCheckBox->setChecked(mp.GetSshPortSpecified());
        char portStr[256];
        SNPRINTF(portStr, 256, "%d", mp.GetSshPort());
        sshPort->setText(portStr);
        tunnelSSH->setChecked(mp.GetTunnelSSH());
        shareMDServerCheckBox->setChecked(mp.GetShareOneBatchJob());
        directory->setText(mp.GetDirectory().c_str());

        // Update the contents of the launch profiles tab
        // Keep track of the old selection if it's still there.
        int nProfiles = mp.GetNumLaunchProfiles();
        LaunchProfile *oldLaunch = currentLaunch;
        profileList->clear();
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
                profileList->item(i)->setSelected(true);
                currentLaunch = oldLaunch;
            }
        }
    }

    hostName->blockSignals(false);
    hostAliases->blockSignals(false);
    hostNickname->blockSignals(false);
    userName->blockSignals(false);
    clientHostNameMethod->blockSignals(false);
    clientHostName->blockSignals(false);
    sshPortCheckBox->blockSignals(false);
    sshPort->blockSignals(false);
    tunnelSSH->blockSignals(false);
    directory->blockSignals(false);
    shareMDServerCheckBox->blockSignals(false);


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
    canDoHW->blockSignals(true);
    preCommand->blockSignals(true);
    preCommandCheckBox->blockSignals(true);
    postCommand->blockSignals(true);
    postCommandCheckBox->blockSignals(true);
    timeout->blockSignals(true);
    engineArguments->blockSignals(true);

    if (currentLaunch == NULL)
    {
        profileName->setText("");
        numProcessors->setValue(1);
        timeout->setValue(60*4);   // 4 hour default
        
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
        engineArguments->setText("");
    }
    else
    {
        profileName->setText(currentLaunch->GetProfileName().c_str());
        // Replace the "localhost" machine name.
        // If there is no user name then give it a valid user name.

        timeout->setValue(currentLaunch->GetTimeout());
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
        canDoHW->setChecked(currentLaunch->GetCanDoHWAccel());
        preCommandCheckBox->setChecked(currentLaunch->GetHavePreCommand());
        preCommand->setText(currentLaunch->GetHwAccelPreCommand().c_str());
        postCommandCheckBox->setChecked(currentLaunch->GetHavePostCommand());
        postCommand->setText(currentLaunch->GetHwAccelPostCommand().c_str());
        timeout->setValue(currentLaunch->GetTimeout());

        QString temp;
        stringVector::const_iterator pos;
        for(pos = currentLaunch->GetArguments().begin();
            pos != currentLaunch->GetArguments().end(); ++pos)
        {
            temp += QString(pos->c_str());
            temp += " ";
        }
        engineArguments->setText(temp);

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
    canDoHW->blockSignals(false);
    preCommand->blockSignals(false);
    preCommandCheckBox->blockSignals(false);
    postCommand->blockSignals(false);
    postCommandCheckBox->blockSignals(false);
    timeout->blockSignals(false);
    engineArguments->blockSignals(false);
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
// ****************************************************************************

void
QvisHostProfileWindow::UpdateWindowSensitivity()
{
    HostProfileList *profiles = (HostProfileList *)subject;
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
    chnParseFromSSHClient->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
    chnSpecifyManually->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
    clientHostNameMethodLabel->setEnabled(hostEnabled && currentMachine->GetTunnelSSH() == false);
    clientHostName->setEnabled(hostEnabled &&
                               currentMachine->GetTunnelSSH() == false &&
                               currentMachine->GetClientHostDetermination() ==
                                              MachineProfile::ManuallySpecified);
    sshPort->setEnabled(hostEnabled && currentMachine->GetSshPortSpecified());
    tunnelSSH->setEnabled(hostEnabled);
    shareMDServerCheckBox->setEnabled(hostEnabled);
    

    profileNameLabel->setEnabled(launchEnabled);
    profileName->setEnabled(launchEnabled);
    timeout->setEnabled(launchEnabled);
    timeoutLabel->setEnabled(launchEnabled);
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
    numNodesCheckBox->setEnabled(parEnabled);
    numNodes->setEnabled(parEnabled && currentLaunch->GetNumNodesSet());
    partitionCheckBox->setEnabled(parEnabled);
    partitionName->setEnabled(parEnabled && currentLaunch->GetPartitionSet());
    bankCheckBox->setEnabled(parEnabled);
    bankName->setEnabled(parEnabled && currentLaunch->GetBankSet());
    timeLimitCheckBox->setEnabled(parEnabled);
    timeLimit->setEnabled(parEnabled && currentLaunch->GetTimeLimitSet());
    machinefileCheckBox->setEnabled(parEnabled);
    machinefile->setEnabled(parEnabled && currentLaunch->GetMachinefileSet());
#if 0 // disabling dynamic load balancing for now
    loadBalancingLabel->setEnabled(parEnabled);
    loadBalancing->setEnabled(parEnabled);
#else
    loadBalancingLabel->setEnabled(false);
    loadBalancing->setEnabled(false);
#endif

    hwdisclaimer->setEnabled(launchEnabled);
    canDoHW->setEnabled(launchEnabled);
    preCommandCheckBox->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());
    postCommandCheckBox->setEnabled(launchEnabled && currentLaunch->GetCanDoHWAccel());
    preCommand->setEnabled(launchEnabled && currentLaunch->GetHavePreCommand() && currentLaunch->GetCanDoHWAccel());
    postCommand->setEnabled(launchEnabled && currentLaunch->GetHavePostCommand() && currentLaunch->GetCanDoHWAccel());


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
            currentMachine->SetUserName(std::string(temp.toStdString()));
        }
        else
        {
            needNotify = true;
            msg = tr("Username cannot be empty, reverting to \"%1\".").
                  arg(currentMachine->GetUserName().c_str());
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
        bool okay = false;
        temp = numProcessors->text();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            int nProc = temp.toInt(&okay);
            if(okay)
            {
                if (nProc != currentLaunch->GetNumProcessors())
                    needNotify = true;
                currentLaunch->SetNumProcessors(nProc);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg = tr("An invalid number of processors was specified,"
                     " reverting to %1 processors.").
                  arg(currentLaunch->GetNumProcessors());
            Message(msg);
        }
    }

    // Do the number of nodes
    if (currentLaunch && currentLaunch->GetParallel())
    {
        bool okay = false;
        temp = numNodes->text();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            int nNodes = temp.toInt(&okay);
            if(okay)
            {
                currentLaunch->SetNumNodes(nNodes);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg = tr("An invalid number of nodes was specified,"
                     " reverting to %1 nodes.").
                  arg(currentLaunch->GetNumNodes());
            Message(msg);
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
                arguments.push_back(std::string(str[i].toStdString()));
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
        bool okay = false;
        temp = timeout->text();
        temp = temp.trimmed();
        if(!temp.isEmpty())
        {
            int tOut = temp.toInt(&okay);
            if(okay)
            {
                if (tOut != currentLaunch->GetTimeout())
                    needNotify = true;
                currentLaunch->SetTimeout(tOut);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg = tr("An invalid timeout was specified, reverting to %1 minutes.").
                  arg(currentLaunch->GetTimeout());
            Message(msg);
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

    // Do the ssh port
    if (currentMachine)
    {
        temp = sshPort->text();

        int newPort = temp.toInt();
        if (currentMachine->GetSshPort() != newPort)
            needNotify = true;

        currentMachine->SetSshPort(newPort);
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
            if (profiles->GetMachines(i).GetHostNickname() != "")
                hostList->item(i)->setText(currentMachine->GetHostNickname().c_str());
            else
                hostList->item(i)->setText(currentMachine->GetHost().c_str());
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
            if (profiles->GetMachines(i).GetHostNickname() != "")
                hostList->item(i)->setText(currentMachine->GetHostNickname().c_str());
            else
                hostList->item(i)->setText(currentMachine->GetHost().c_str());
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
// ****************************************************************************

void
QvisHostProfileWindow::toggleShareMDServer(bool state)
{
    if (currentMachine == NULL)
        return;

    currentMachine->SetShareOneBatchJob(state);
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
//  Method:  QvisHostProfileWindow::togglePreCommand
//
//  Purpose:
//      Toggles whether or not there is a pre-command
//
//  Arguments:
//      state    True if there is, false if there's not.
//
//  Programmer:  Hank Childs
//  Creation:    December 2, 2005
//
//  Modifications:
//   Jeremy Meredith, Thu Feb 18 15:54:50 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::togglePreCommand(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetHavePreCommand(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::togglePostCommand
//
//  Purpose:
//      Toggles whether or not there is a post-command
//
//  Arguments:
//      state    True if there is, false if there's not.
//
//  Programmer:  Hank Childs
//  Creation:    December 2, 2005
//
//  Modifications:
//   Jeremy Meredith, Thu Feb 18 15:54:50 EST 2010
//   Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::togglePostCommand(bool state)
{
    if (currentLaunch == NULL)
        return;

    currentLaunch->SetHavePostCommand(state);
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::preCommandChanged
//
//  Purpose:
//      Changes the text for the pre-command
//
//  Arguments:
//    portStr   the string indicating the port value
//
//  Programmer:  Hank Childs
//  Creation:    December 2, 2005
//
//  Modifications:
//    Jeremy Meredith, Mon Apr 10 13:24:13 PST 2006
//    Fixed preCommand/postCommand error.
//
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Jeremy Meredith, Thu Feb 18 15:54:50 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************

void
QvisHostProfileWindow::preCommandChanged(const QString &portStr)
{
    if (currentLaunch == NULL)
        return;

    QString temp, msg;
    temp = preCommand->displayText();
    if(!temp.isEmpty())
    {
        currentLaunch->SetHwAccelPreCommand(std::string(temp.toStdString()));
        preCommandCheckBox->setChecked(false);
    }
    else
    {
        msg = tr("Pre-command cannot be empty, turning off pre-command.");
        currentLaunch->SetHavePreCommand(false);
    }
    UpdateWindowSensitivity();
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisHostProfileWindow::postCommandChanged
//
//  Purpose:
//      Changes the text for the post-command
//
//  Arguments:
//    portStr   the string indicating the port value
//
//  Programmer:  Hank Childs
//  Creation:    December 2, 2005
//
//  Modifications:
//    Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//    Support for internationalization.
//
//    Jeremy Meredith, Thu Feb 18 15:54:50 EST 2010
//    Split HostProfile int MachineProfile and LaunchProfile.  Rewrote window.
//
// ****************************************************************************
void
QvisHostProfileWindow::postCommandChanged(const QString &portStr)
{
    if (currentLaunch == NULL)
        return;

    QString temp, msg;
    temp = postCommand->displayText();
    if(!temp.isEmpty())
    {
        currentLaunch->SetHwAccelPostCommand(std::string(temp.toStdString()));
        postCommandCheckBox->setChecked(false);
    }
    else
    {
        msg = tr("Post-command cannot be empty, turning off post-command.");
        currentLaunch->SetHavePostCommand(false);
    }
    UpdateWindowSensitivity();
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
    for (int i=0; i<sel.size() && machine<0; i++)
    {
        for (int j=0; j<nMachines && machine<0; j++)
        {
            if (sel[i] == hostList->item(j))
                machine = j;
        }
    }

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
// ****************************************************************************
void
QvisHostProfileWindow::addMachineProfile()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    MachineProfile mp = MachineProfile();
    mp.SetHostNickname(string("unknown host"));
    profiles->AddMachines(mp);
    ReplaceLocalHost();
    Apply();
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
    Apply();
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
// ****************************************************************************
void
QvisHostProfileWindow::copyMachineProfile()
{
    if (currentMachine == NULL)
        return;

    HostProfileList *profiles = (HostProfileList *)subject;
    MachineProfile mp = *currentMachine;
    mp.SetHostNickname(string("Copy of ")+mp.GetHostNickname());
    profiles->AddMachines(mp);
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
// ****************************************************************************
void
QvisHostProfileWindow::addLaunchProfile()
{
    if (currentMachine == NULL)
        return;

    LaunchProfile lp;
    QString profileName(tr("New profile"));
    QString num; num.sprintf(" #%d", profileCounter++);
    profileName += num;
    lp.SetProfileName(profileName.toStdString());

    currentMachine->AddLaunchProfiles(lp);
    if (currentMachine->GetActiveProfile() <  0 ||
        currentMachine->GetActiveProfile() >=
                                       currentMachine->GetNumLaunchProfiles())
    {
        currentMachine->SetActiveProfile(0);
    }
    Apply();
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
// ****************************************************************************
void
QvisHostProfileWindow::copyLaunchProfile()
{
    if (currentMachine == NULL || currentLaunch == NULL)
        return;
    LaunchProfile lp(*currentLaunch);
    lp.SetProfileName(std::string("Copy of ")+lp.GetProfileName());
    currentMachine->AddLaunchProfiles(lp);
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

