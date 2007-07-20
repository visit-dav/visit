/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisHostProfileWindow.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qvbox.h>

#include <snprintf.h>

#include <HostProfile.h>
#include <HostProfileList.h>
#include <ViewerProxy.h>

#define HOST_PROFILE_SPACING 8

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
// ****************************************************************************

QvisHostProfileWindow::QvisHostProfileWindow(HostProfileList *profiles,
    const char *caption, const char *shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(profiles, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton),
    hostTabMap()
{
    profileCounter = 0;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::~QvisHostProfileWindow
//
// Purpose: 
//   This is the destructor for the QvisHostProfileWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:20:11 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:11:40 PST 2002
//   Deleted parentless widgets.
//
//   Brad Whitlock, Wed Jun 6 09:35:57 PDT 2007
//   Removed a button group.
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
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:18:32 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 17:42:07 PDT 2001
//   Added numNodes, launchMethod, and partitionName, as well as checkboxes
//   for each.  Also reworded "default profile" to be "active profile".
//
//   Jeremy Meredith, Thu Sep 13 17:33:17 PDT 2001
//   Changed the blank launch option to read "default" instead.
//
//   Jeremy Meredith, Fri Sep 21 14:29:59 PDT 2001
//   Added buttons for forcing static and dynamic load balancing.
//
//   Brad Whitlock, Mon Sep 24 11:07:58 PDT 2001
//   Changed the way some signals/slots are connected. Made the minimum
//   height of the hostTabs depend on the font size.
//
//   Jeremy Meredith, Thu Feb 14 15:25:21 PST 2002
//   Change hostName to a combo box.  Change the type of callback for the
//   tabwidget page turn.  Add "Pool" to partition label.
//
//   Sean Ahern, Thu Feb 21 15:54:40 PST 2002
//   Added timeout support.  Added the "row" variable to make adding new
//   widgets easier.
//
//   Jeremy Meredith, Wed Mar  6 15:35:18 PST 2002
//   Renamed "Host" to say "Full host name" so it is more obvious what
//   we are looking for.
//
//   Brad Whitlock, Fri May 3 16:42:34 PST 2002
//   Removed the word "Engine" from the last line edit label.
//
//   Jeremy Meredith, Wed Aug 14 10:49:25 PDT 2002
//   Added a couple parallel options and restructured the window.
//
//   Jeremy Meredith, Thu Nov 21 11:28:39 PST 2002
//   Connected profileName signal textChanged() to a new function.
//
//   Jeremy Meredith, Fri Jan 24 14:43:28 PST 2003
//   Added optional arguments to the parallel launcher.
//
//   Jeremy Meredith, Sat Apr 12 13:42:30 PDT 2003
//   Added a couple variants on psub for new systems.
//
//   Jeremy Meredith, Mon Apr 14 17:20:25 PDT 2003
//   Changed the psub variants to be more "shell-friendly", i.e. no spaces
//   and no parentheses.  Added hostAliases.
//
//   Jeremy Meredith, Fri May  9 12:39:50 PDT 2003
//   Added bsub as a launcher option.  This is in preparation for the
//   Q machine.
//
//   Jeremy Meredith, Mon May 19 12:48:38 PDT 2003
//   Added toggle button for sharing mdserver / engines in a single job.
//
//   Jeremy Meredith, Thu Jun 26 10:37:09 PDT 2003
//   Enabled the share-batch-job toggle.
//
//   Jeremy Meredith, Mon Aug 18 13:38:40 PDT 2003
//   Changed username field to update on any text change, not just
//   a return press.
//
//   Jeremy Meredith, Thu Oct  9 15:47:00 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.  Moved "shareMDServer" to a new
//   advanced tab, on which these other options were also placed.
//
//   Jeremy Meredith, Thu Nov 13 16:02:47 PST 2003
//   Added "srun" launcher.
//
//   Jeremy Meredith, Thu Nov 20 16:29:54 PST 2003
//   Adding psub/srun option.
//
//   Kathleen Bonnell, Tue Jan 11 16:14:14 PST 2005 
//   Set minimum for numNodes spinbox to 1. 
//
//   Jeremy Meredith, Thu Sep 15 16:37:24 PDT 2005
//   Added machinefile and useVisItScriptForEnv.
//
//   Kathleen Bonnell, Thu Nov 10 16:58:34 PST 2005 
//   Added "qsub" launcher.
//
//   Hank Childs, Fri Dec  2 10:59:47 PST 2005
//   Added new tab for hardware acceleration.
//
//   Hank Childs, Fri Dec  9 07:13:44 PST 2005
//   Change the word 'window' to 'tab' in disclaimer for hardware acceleration.
//
//   Eric Brugger, Tue Nov 28 13:48:04 PST 2006
//   Added a psub/mpirun launch option.
//
//   Eric Brugger, Thu Feb 15 12:14:34 PST 2007
//   Added support for additional sublauncher arguments.  Replaced the qsub
//   launch option with qsub/mpiexec and qsub/srun.
//
//   Jeremy Meredith, Thu May 24 11:05:45 EDT 2007
//   Added support for SSH port tunneling.
//
//   Brad Whitlock, Tue Jun 5 15:53:20 PST 2007
//   Moved individual tabs into their own creation functions.
//
// ****************************************************************************

void
QvisHostProfileWindow::CreateWindowContents()
{
    QLabel *hostProfileLabel = new QLabel("Host profiles", central,
        "hostProfileLabel");
    topLayout->addWidget(hostProfileLabel);

    hostTabs = new QTabWidget(central, "hostTabs");
    hostTabs->setMinimumHeight(hostTabs->fontMetrics().height() * 6);
    connect(hostTabs, SIGNAL(currentChanged(QWidget *)),
            this, SLOT(pageTurned(QWidget *)));
    topLayout->addWidget(hostTabs, 20);
    emptyListBox = 0;

    QHBoxLayout *buttonLayout = new QHBoxLayout(topLayout);
    newButton = new QPushButton("New profile", central, "newButton");
    connect(newButton, SIGNAL(clicked()), this, SLOT(newProfile()));
    buttonLayout->addWidget(newButton);

    deleteButton = new QPushButton("Delete profile", central, "deleteButton");
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteProfile()));
    buttonLayout->addWidget(deleteButton);
    buttonLayout->addStretch(10);

    optionsTabs = new QTabWidget(central, "optionsTabs");
    topLayout->addWidget(optionsTabs);

    //
    // Create a tab for the general options
    //
    activeProfileGroup = CreateSelectedTab(central);
    optionsTabs->addTab(activeProfileGroup,"Selected profile");

    //
    // Create a tab for Parallel options.
    //
    parGroup = CreateParallelTab(central);
    optionsTabs->addTab(parGroup, "Parallel options");

    //
    // Create a tab for Advanced options
    //
    advancedGroup = CreateAdvancedTab(central);
    optionsTabs->addTab(advancedGroup, "Advanced options");
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateSelectedTab
//
// Purpose: 
//   Creates the widgets for the "Selected profile" tab.
//
// Arguments:
//   parent : The parent widget for the tab.
//
// Returns:    The widget that contains all controls for the tab.
//
// Note:       
//
// Programmer: VisIt team
// Creation:   Wed Jun 6 10:14:17 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 10:14:35 PDT 2007
//   I moved the code to create the tab to this function so I could focus on
//   less code. I also improved the layout.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateSelectedTab(QWidget *parent)
{
    activeProfileGroup = new QWidget(parent, "activeProfileGroup");

    QVBoxLayout *innerLayout = new QVBoxLayout(activeProfileGroup);
    innerLayout->setMargin(10);
    innerLayout->addSpacing(5);
    QGridLayout *profileLayout = new QGridLayout(innerLayout, 8, 2);
    profileLayout->setSpacing(HOST_PROFILE_SPACING);
    innerLayout->addStretch(5);

    QLabel *disclaimer = new QLabel("<i>* Applies to all profiles for a given host</i>",
                           activeProfileGroup, "disclaimer");
    disclaimer->setTextFormat(Qt::RichText);
    innerLayout->addWidget(disclaimer);

    int row = 0;

    profileName = new QLineEdit(activeProfileGroup, "profileName");
    connect(profileName, SIGNAL(textChanged(const QString&)),
            this, SLOT(processProfileNameText(const QString&)));
    profileNameLabel = new QLabel(profileName, "Profile name",
        activeProfileGroup, "profileNameLabel");
    profileLayout->addWidget(profileNameLabel, row, 0);
    profileLayout->addWidget(profileName, row, 1);
    row++;

    activeProfileCheckBox = new QCheckBox("Default profile for host",
        activeProfileGroup, "activeProfileCheckBox");
    connect(activeProfileCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(makeActiveProfile(bool)));
    profileLayout->addWidget(activeProfileCheckBox, row, 1);
    row++;

    hostName = new QComboBox(true, activeProfileGroup, "hostName");
    hostName->setDuplicatesEnabled(false);
    connect(hostName, SIGNAL(activated(const QString &)),
            this, SLOT(hostNameChanged(const QString &)));
    hostNameLabel = new QLabel(hostName, "Remote host name",
        activeProfileGroup, "hostNameLabel");
    profileLayout->addWidget(hostNameLabel, row, 0);
    profileLayout->addWidget(hostName, row, 1);
    row++;

    hostAliases = new QLineEdit(activeProfileGroup, "hostAliases");
    connect(hostAliases, SIGNAL(textChanged(const QString &)),
            this, SLOT(hostAliasesChanged(const QString &)));
    hostAliasesLabel = new QLabel(hostAliases, "*Host name aliases",
        activeProfileGroup, "hostAliasesLabel");
    profileLayout->addWidget(hostAliasesLabel, row, 0);
    profileLayout->addWidget(hostAliases, row, 1);
    row++;

    userName = new QLineEdit(activeProfileGroup, "userName");
    connect(userName, SIGNAL(textChanged(const QString &)),
            this, SLOT(userNameChanged(const QString &)));
    userNameLabel = new QLabel(userName, "*Username",
        activeProfileGroup, "userNameLabel");
    profileLayout->addWidget(userNameLabel, row, 0);
    profileLayout->addWidget(userName, row, 1);
    row++;

    timeout = new QSpinBox(1, 1440, 1, activeProfileGroup, "timeout");
    connect(timeout, SIGNAL(valueChanged(int)),
            this, SLOT(timeoutChanged(int)));
    timeoutLabel = new QLabel(timeout, "Timeout (minutes)",
        activeProfileGroup, "timeoutLabel");
    profileLayout->addWidget(timeoutLabel, row, 0);
    profileLayout->addWidget(timeout, row, 1);
    row++;

    engineArguments = new QLineEdit(activeProfileGroup, "engineArguments");
    connect(engineArguments, SIGNAL(textChanged(const QString &)),
            this, SLOT(processEngineArgumentsText(const QString &)));
    engineArgumentsLabel = new QLabel(engineArguments, "Additional options",
        activeProfileGroup, "engineArgumentsLabel");
    profileLayout->addWidget(engineArgumentsLabel, row, 0);
    profileLayout->addWidget(engineArguments, row, 1);
    row++;

    parallelCheckBox = new QCheckBox("Parallel computation engine", 
                                     activeProfileGroup, "parallelCheckBox");
    connect(parallelCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleParallel(bool)));
    profileLayout->addMultiCellWidget(parallelCheckBox, row, row, 0, 1);
    row++;

    return activeProfileGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateParallelTab
//
// Purpose: 
//   Creates the widgets for the "Parallel options" tab.
//
// Arguments:
//   parent : The parent widget for the tab.
//
// Returns:    The widget that contains all controls for the tab.
//
// Note:       
//
// Programmer: VisIt team
// Creation:   Wed Jun 6 10:14:17 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 10:14:35 PDT 2007
//   I moved the code to create the tab to this function so I could focus on
//   less code. I also improved the layout.
//
//   Jeremy Meredith, Thu Jun 28 13:17:50 EDT 2007
//   Allowed "1" for the minimum number of processors. This is how we specify
//   serial jobs on batch nodes.
//
//   Eric Brugger, Wed Jul 18 13:22:20 PDT 2007
//   Removed support for psub/prun.  Added support for msub/srun and
//   qsub/mpirun.  Alphabetized the list of options.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateParallelTab(QWidget *parent)
{
    parGroup = new QWidget(parent, "parGroup");

    QVBoxLayout *innerParLayout = new QVBoxLayout(parGroup);
    innerParLayout->setMargin(10);
    innerParLayout->addSpacing(5);

    QGridLayout *parLayout = new QGridLayout(innerParLayout, 9, 2);
    parLayout->setSpacing(HOST_PROFILE_SPACING);

    int prow = 0;

    launchMethod = new QComboBox(false, parGroup, "launchMethod");
    launchMethod->insertItem("(default)");
    launchMethod->insertItem("bsub");
    launchMethod->insertItem("dmpirun");
    launchMethod->insertItem("mpirun");
    launchMethod->insertItem("poe");
    launchMethod->insertItem("prun");
    launchMethod->insertItem("psub");
    launchMethod->insertItem("srun");
    launchMethod->insertItem("yod");
    launchMethod->insertItem("msub/srun");
    launchMethod->insertItem("psub/mpirun");
    launchMethod->insertItem("psub/poe");
    launchMethod->insertItem("psub/srun");
    launchMethod->insertItem("qsub/mpiexec");
    launchMethod->insertItem("qsub/mpirun");
    launchMethod->insertItem("qsub/srun");
    connect(launchMethod, SIGNAL(activated(const QString &)),
            this, SLOT(launchMethodChanged(const QString &)));
    launchCheckBox = new QCheckBox("Parallel launch method",
                                   parGroup, "launchCheckBox");
    connect(launchCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleLaunch(bool)));
    parLayout->addWidget(launchCheckBox, prow, 0);
    parLayout->addWidget(launchMethod, prow, 1);
    prow++;

    launchArgs = new QLineEdit(parGroup, "launchArgs");
    connect(launchArgs, SIGNAL(textChanged(const QString &)),
            this, SLOT(processLaunchArgsText(const QString &)));
    launchArgsCheckBox = new QCheckBox("Launcher arguments",
                                      parGroup, "launchArgsLabel");
    connect(launchArgsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleLaunchArgs(bool)));
    parLayout->addWidget(launchArgsCheckBox, prow, 0);
    parLayout->addWidget(launchArgs, prow, 1);
    prow++;

    sublaunchArgs = new QLineEdit(parGroup, "sublaunchArgs");
    connect(sublaunchArgs, SIGNAL(textChanged(const QString &)),
            this, SLOT(processSublaunchArgsText(const QString &)));
    sublaunchArgsCheckBox = new QCheckBox("Sublauncher arguments",
                                      parGroup, "sublaunchArgsLabel");
    connect(sublaunchArgsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSublaunchArgs(bool)));
    parLayout->addWidget(sublaunchArgsCheckBox, prow, 0);
    parLayout->addWidget(sublaunchArgs, prow, 1);
    prow++;

    partitionName = new QLineEdit(parGroup, "partitionName");
    connect(partitionName, SIGNAL(textChanged(const QString &)),
            this, SLOT(processPartitionNameText(const QString &)));
    partitionCheckBox = new QCheckBox("Partition / Pool",
                                      parGroup, "partitionLabel");
    connect(partitionCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePartitionName(bool)));
    parLayout->addWidget(partitionCheckBox, prow, 0);
    parLayout->addWidget(partitionName, prow, 1);
    prow++;

    loadBalancing = new QComboBox(false, parGroup, "loadBalancing");
    loadBalancing->insertItem("Auto");
    loadBalancing->insertItem("Static");
    loadBalancing->insertItem("Dynamic");
    connect(loadBalancing, SIGNAL(activated(int)),
            this, SLOT(loadBalancingChanged(int)));

    loadBalancingLabel = new QLabel("Load balancing", parGroup, "loadBalancingLabel");
    parLayout->addWidget(loadBalancingLabel, prow, 0);
    parLayout->addWidget(loadBalancing, prow, 1);
    prow++;

    numProcessors = new QSpinBox(1, 99999, 1, parGroup, "numProcessors");
    connect(numProcessors, SIGNAL(valueChanged(int)),
            this, SLOT(numProcessorsChanged(int)));
    numProcLabel = new QLabel(numProcessors, "Default number of processors",
        parGroup, "numProcLabel");
    parLayout->addWidget(numProcLabel, prow, 0);
    parLayout->addWidget(numProcessors, prow, 1);
    prow++;

    numNodes = new QSpinBox(1, 99999, 1, parGroup, "numNodes");
    connect(numNodes, SIGNAL(valueChanged(int)),
            this, SLOT(numNodesChanged(int)));
    numNodesCheckBox = new QCheckBox("Default number of nodes",
                                     parGroup, "numNodesCheckBox");
    connect(numNodesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleNumNodes(bool)));
    parLayout->addWidget(numNodesCheckBox, prow, 0);
    parLayout->addWidget(numNodes, prow, 1);
    prow++;

    bankName = new QLineEdit(parGroup, "bankName");
    connect(bankName, SIGNAL(textChanged(const QString &)),
            this, SLOT(processBankNameText(const QString &)));
    bankCheckBox = new QCheckBox("Default Bank",
                                 parGroup, "bankLabel");
    connect(bankCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleBankName(bool)));
    parLayout->addWidget(bankCheckBox, prow, 0);
    parLayout->addWidget(bankName, prow, 1);
    prow++;

    timeLimit = new QLineEdit(parGroup, "timeLimit");
    connect(timeLimit, SIGNAL(textChanged(const QString &)),
            this, SLOT(processTimeLimitText(const QString &)));
    timeLimitCheckBox = new QCheckBox("Default Time Limit",
                                      parGroup, "timeLimitLabel");
    connect(timeLimitCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleTimeLimit(bool)));
    parLayout->addWidget(timeLimitCheckBox, prow, 0);
    parLayout->addWidget(timeLimit, prow, 1);
    prow++;

    machinefile = new QLineEdit(parGroup, "machinefile");
    connect(machinefile, SIGNAL(textChanged(const QString &)),
            this, SLOT(processMachinefileText(const QString &)));
    machinefileCheckBox = new QCheckBox("Default Machine File",
                                      parGroup, "machinefileLabel");
    connect(machinefileCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleMachinefile(bool)));
    parLayout->addWidget(machinefileCheckBox, prow, 0);
    parLayout->addWidget(machinefile, prow, 1);
    prow++;

    return parGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateAdvancedTab
//
// Purpose: 
//   Creates the widgets for the "Advanced options" tab.
//
// Arguments:
//   parent : The parent widget for the tab.
//
// Returns:    The widget that contains all controls for the tab.
//
// Note:       
//
// Programmer: VisIt team
// Creation:   Wed Jun 6 10:14:17 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 10:14:35 PDT 2007
//   I moved the code to create the tab to this function so I could focus on
//   less code.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateAdvancedTab(QWidget *parent)
{
    advancedGroup = new QWidget(parent, "advancedGroup");

    QVBoxLayout *innerAdvLayout = new QVBoxLayout(advancedGroup);
    innerAdvLayout->setMargin(10);
    innerAdvLayout->addSpacing(5);

    QVBoxLayout *advLayout = new QVBoxLayout(innerAdvLayout);
    advLayout->setSpacing(HOST_PROFILE_SPACING);

    shareMDServerCheckBox = new QCheckBox("Share batch job with Metadata Server",
                                          advancedGroup, "shareMDServerCheckBox");
    advLayout->addWidget(shareMDServerCheckBox);
    connect(shareMDServerCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleShareMDServer(bool)));

    useVisItScriptForEnvCheckBox = new QCheckBox(
                             "Use VisIt script to set up parallel environment",
                             advancedGroup, "useVisItScriptForEnvCheckBox");
    advLayout->addWidget(useVisItScriptForEnvCheckBox);
    connect(useVisItScriptForEnvCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleUseVisItScriptForEnv(bool)));

    QTabWidget *advancedOptionsTabs = new QTabWidget(advancedGroup, "advancedOptionsTabs");
    advLayout->addWidget(advancedOptionsTabs);

    //
    // Create a tab for Networking
    //
    networkingGroup = CreateNetworkingTab(advancedGroup);
    advancedOptionsTabs->addTab(networkingGroup, "Networking");

    //
    // Create a tab for Hardware acceleration
    //
    hwGroup = CreateHardwareAccelerationTab(advancedGroup);
    advancedOptionsTabs->addTab(hwGroup, "Hardware acceleration");
    
    return advancedGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateNetworkingTab
//
// Purpose: 
//   Creates the widgets for the "Networking" tab.
//
// Arguments:
//   parent : The parent widget for the tab.
//
// Returns:    The widget that contains all controls for the tab.
//
// Note:       
//
// Programmer: Jeremy Meredith
// Creation:   Wed Jun 6 10:14:17 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 10:14:35 PDT 2007
//   I moved the code to create the tab to this function so I could focus on
//   less code. I also improved the layout.
//
//   Jeremy Meredith, Thu Jun 28 13:18:36 EDT 2007
//   Rearranged to put SSH tunneling above client host name method, as SSH
//   tunneling overrides it.  Changed host name method text to clarify this.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateNetworkingTab(QWidget *parent)
{
    int nrow = 0;

    networkingGroup = new QWidget(parent, "networkingGroup");

    QVBoxLayout *innerNetLayout = new QVBoxLayout(networkingGroup);
    innerNetLayout->setMargin(10);
    innerNetLayout->addSpacing(5);

    QGridLayout *netLayout = new QGridLayout(innerNetLayout, 7, 4);
    netLayout->setSpacing(HOST_PROFILE_SPACING);
    innerNetLayout->addStretch(5);

    QLabel *disclaimer = new QLabel("<i>Networking options apply to "
        "all profiles for a given host.</i>", networkingGroup, "disclaimer");
    disclaimer->setTextFormat(Qt::RichText);
    netLayout->addMultiCellWidget(disclaimer, nrow, nrow, 0, 3);
    nrow++;

    tunnelSSH = new QCheckBox("Tunnel data connections through SSH",
                              networkingGroup, "tunnelSSH");
    netLayout->addMultiCellWidget(tunnelSSH, nrow,nrow, 0,3);
    connect(tunnelSSH, SIGNAL(toggled(bool)),
            this, SLOT(toggleTunnelSSH(bool)));
    nrow++;

    clientHostNameMethod = new QButtonGroup(0, "clientHostNameMethod");
    connect(clientHostNameMethod, SIGNAL(clicked(int)),
            this, SLOT(clientHostNameMethodChanged(int)));
    chnMachineName = new QRadioButton("Use local machine name",
                                      networkingGroup, "chnMachineName");
    chnParseFromSSHClient = new QRadioButton("Parse from SSH_CLIENT environment variable",
                                             networkingGroup, "chnParseFromSSHClient");
    chnSpecifyManually = new QRadioButton("Specify manually:",
                                          networkingGroup, "chnSpecifyManually");
    chnMachineName->setChecked(true);
    clientHostNameMethod->insert(chnMachineName);
    clientHostNameMethod->insert(chnParseFromSSHClient);
    clientHostNameMethod->insert(chnSpecifyManually);
    clientHostNameMethodLabel =
        new QLabel("Method used to determine local host name when not tunneling:",
                   networkingGroup, "clientHostNameMethodLabel");
    netLayout->addMultiCellWidget(clientHostNameMethodLabel,
                                  nrow, nrow, 0, 3);
    nrow++;
    netLayout->addMultiCellWidget(chnMachineName, nrow, nrow, 1, 3);
    nrow++;
    netLayout->addMultiCellWidget(chnParseFromSSHClient, nrow, nrow, 1, 3);
    nrow++;
    netLayout->addMultiCellWidget(chnSpecifyManually, nrow, nrow, 1, 2);
    clientHostName = new QLineEdit(networkingGroup, "clientHostName");
    connect(clientHostName, SIGNAL(textChanged(const QString &)),
            this, SLOT(clientHostNameChanged(const QString &)));
    netLayout->addMultiCellWidget(clientHostName, nrow, nrow, 3,3);
    nrow++;

    sshPort = new QLineEdit(networkingGroup, "sshPort");
    sshPortCheckBox = new QCheckBox("Specify SSH port", networkingGroup,
                                    "sshPortCheckBox");
    connect(sshPort, SIGNAL(textChanged(const QString &)),
            this, SLOT(sshPortChanged(const QString &)));
    connect(sshPortCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(toggleSSHPort(bool)));
    netLayout->addMultiCellWidget(sshPortCheckBox, nrow, nrow, 0, 1);
    netLayout->addMultiCellWidget(sshPort, nrow, nrow, 2, 3);
    nrow++;

    return networkingGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::CreateHardwareAccelerationTab
//
// Purpose: 
//   Creates the widgets for the "Hardware acceleration" tab.
//
// Arguments:
//   parent : The parent widget for the tab.
//
// Returns:    The widget that contains all controls for the tab.
//
// Note:       
//
// Programmer: Hank Childs
// Creation:   Wed Jun 6 10:14:17 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Jun 6 10:14:35 PDT 2007
//   I moved the code to create the tab to this function so I could focus on
//   less code. I also improved the layout.
//
// ****************************************************************************

QWidget *
QvisHostProfileWindow::CreateHardwareAccelerationTab(QWidget *parent)
{
    hwGroup = new QWidget(parent, "hwGroup");

    QVBoxLayout *innerHwLayout = new QVBoxLayout(hwGroup);
    innerHwLayout->setMargin(10);
    innerHwLayout->addSpacing(5);

    QGridLayout *hwLayout = new QGridLayout(innerHwLayout, 4, 2);
    hwLayout->setSpacing(HOST_PROFILE_SPACING);
    innerHwLayout->addStretch(5);

    QString str1("<i>These options are for hardware accelerating the scalable rendering "
                 "feature on a parallel cluster. In other modes, VisIt will automatically "
                 "use hardware acceleration. This tab only needs to be modified for "
                 "parallel clusters that have graphics cards.</i>");

    QLabel *disclaimer = new QLabel(str1, hwGroup, "disclaimer1");
    disclaimer->setTextFormat(Qt::RichText);
    int hrow = 0;
    hwLayout->addMultiCellWidget(disclaimer, hrow, hrow, 0, 1);
    hrow++;

    canDoHW = new QCheckBox("Use cluster's graphics cards",
                                     hwGroup, "canDoHW");
    connect(canDoHW, SIGNAL(toggled(bool)),
            this, SLOT(toggleCanDoHW(bool)));
    hwLayout->addMultiCellWidget(canDoHW, hrow, hrow, 0, 1);
    hrow++;

    preCommand = new QLineEdit(hwGroup, "preCommand");
    preCommandCheckBox = new QCheckBox("Pre-command", hwGroup,
                                    "preCommandCheckBox");
    connect(preCommand, SIGNAL(textChanged(const QString &)),
            this, SLOT(preCommandChanged(const QString &)));
    connect(preCommandCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePreCommand(bool)));
    hwLayout->addMultiCellWidget(preCommandCheckBox, hrow, hrow, 0, 0);
    hwLayout->addMultiCellWidget(preCommand, hrow, hrow, 1, 1);
    hrow++;

    postCommand = new QLineEdit(hwGroup, "postCommand");
    postCommandCheckBox = new QCheckBox("Post-command", hwGroup,
                                    "postCommandCheckBox");
    connect(postCommand, SIGNAL(textChanged(const QString &)),
            this, SLOT(postCommandChanged(const QString &)));
    connect(postCommandCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(togglePostCommand(bool)));
    hwLayout->addMultiCellWidget(postCommandCheckBox, hrow, hrow, 0, 0);
    hwLayout->addMultiCellWidget(postCommand, hrow, hrow, 1, 1);
    hrow++;

    return hwGroup;
}

// ****************************************************************************
// Method: QvisHostProfileWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window so it reflects the current state
//   of the HostProfileList object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:13:03 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::UpdateWindow(bool doAll)
{
    // Cast the subject into something useful.
    HostProfileList *profiles = (HostProfileList *)subject;

    // Replace any localhost machine names.
    ReplaceLocalHost();

    // Update the profile name list.
    if(profiles->IsSelected(0) || doAll)
        UpdateProfileList();
    // Update the active profile area.
    if(profiles->IsSelected(1) || doAll)
        UpdateActiveProfile();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::UpdateProfileList
//
// Purpose: 
//   This method updates the window's profile list.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:13:58 PST 2000
//
// Modifications:
//   Brad Whitlock, Wed Oct 18 11:17:05 PDT 2000
//   I completely rewrote the method to support a tabwidget containing
//   list boxes instead of just a single listview.
//
//   Brad Whitlock, Mon Sep 24 15:57:47 PST 2001
//   I moved the code to create the empty listbox to this method.
//
//   Jeremy Meredith, Thu Feb 14 15:27:39 PST 2002
//   Try to get a good short host name to stick in the tab widget.
//   Fixed a bug where tabs were removed but later referenced anyway.
//   Added code to populate the hostName combo box.
//
//   Jeremy Meredith, Wed Feb 27 11:14:57 PST 2002
//   Made it pull the old host name from the widget before
//   clearing the text.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateProfileList()
{
    HostProfileList *profiles = (HostProfileList *)subject;

    int i;
    HostTabMap::Iterator pos;

    // If there are host profiles, add the empty tab. Otherwise remove it.
    hostTabs->blockSignals(true);
    if(profiles->GetNumProfiles() < 1)
    {
        // Clear the tab map.
        for(pos = hostTabMap.begin(); pos != hostTabMap.end(); ++pos)
        {
            // Disconnect the signals of the listboxes we're deleting.
            disconnect(pos.data(), SIGNAL(doubleClicked(QListBoxItem *)),
                this, SLOT(activateProfile(QListBoxItem *)));
            disconnect(pos.data(), SIGNAL(selectionChanged(QListBoxItem *)),
                this, SLOT(activateProfile(QListBoxItem *)));
            pos.data()->clear();
            hostTabs->removePage(pos.data());
        }
        hostTabMap.clear();
        if(emptyListBox == 0)
        {
            emptyListBox = new QListBox(hostTabs, "emptyList");
            emptyListBox->setVScrollBarMode(QScrollView::Auto);
            emptyListBox->setHScrollBarMode(QScrollView::Auto);
        }
        hostTabs->insertTab(emptyListBox, "    ");

        return;
    }
    else if(emptyListBox != 0)
        hostTabs->removePage(emptyListBox);

    // Find a list of hosts that no longer need a tab.
    HostTabMap removal;
    for(pos = hostTabMap.begin(); pos != hostTabMap.end(); ++pos)
    {
        // If the host has no profiles, add it to the remove list.
        if(profiles->GetNumProfilesForHost(std::string(pos.key().latin1())) == 0)
        {
            removal.insert(pos.key(), pos.data());
        }
    }

    // Find a list of hosts that need a tab and do not have one.
    HostTabMap additional;
    for(i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        QString host(profiles->operator[](i).GetHost().c_str());
        if(hostTabMap.find(host) == hostTabMap.end())
        {
            additional.insert(host, 0);
        }
    }

    // If there are any tabs to be added, try and use tabs already in
    // the widget. If none are available, create new tabs.
    for(pos = additional.begin(); pos != additional.end(); ++pos)
    {
        QListBox *newListBox;
        QString shortHostName(HostProfile::GetShortHostname(pos.key().latin1()).c_str());

        if(removal.count() > 0)
        {
            // Reuse the tab
            newListBox = removal.begin().data();
            hostTabs->changeTab(newListBox, shortHostName);
            hostTabMap.remove(hostTabMap.find(removal.begin().key()));
            removal.remove(removal.begin().key());
        }
        else
        {
            newListBox = new QListBox(hostTabs);
            connect(newListBox, SIGNAL(doubleClicked(QListBoxItem *)),
                this, SLOT(activateProfile(QListBoxItem *)));
            connect(newListBox, SIGNAL(selectionChanged(QListBoxItem *)),
                this, SLOT(activateProfile(QListBoxItem *)));
            hostTabs->addTab(newListBox, shortHostName);
        }

        // Associate the host with the list box.
        hostTabMap.insert(pos.key(), newListBox);
    }

    // If the remove map is not empty, remove the tabs associated with
    // the hosts in the removal map.
    for(pos = removal.begin(); pos != removal.end(); ++pos)
    {
        hostTabs->removePage(pos.data());
        hostTabMap.remove(pos.key());

        // Disconnect the signals of the listboxes we're deleting.
        disconnect(pos.data(), SIGNAL(doubleClicked(QListBoxItem *)),
            this, SLOT(activateProfile(QListBoxItem *)));
        disconnect(pos.data(), SIGNAL(selectionChanged(QListBoxItem *)),
            this, SLOT(activateProfile(QListBoxItem *)));
    }
    hostTabs->blockSignals(false);

    // Clear all of the listboxes in the hostTabMap so there are no
    // duplicates when we add the entire list of profiles to the
    // various listboxes.
    for(pos = hostTabMap.begin(); pos != hostTabMap.end(); ++pos)
        pos.data()->clear();

    // Now that the tabs are settled, go through and add all of the 
    // profiles to the appropriate tab.
    hostTabs->blockSignals(true);
    for(i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        const HostProfile &current = profiles->operator[](i);

        QString profileString(current.GetProfileName().c_str());
        QString hostName(current.GetHost().c_str());
        HostTabMap::Iterator pos;

        if((pos = hostTabMap.find(hostName)) != hostTabMap.end())
        {
            int newIndex = pos.data()->count();
            pos.data()->insertItem(profileString, newIndex);
            
            if(i == profiles->GetActiveProfile())
            {
                hostTabs->showPage(pos.data());
                pos.data()->blockSignals(true);
                pos.data()->setSelected(newIndex, true);
                pos.data()->setCurrentItem(newIndex);
                pos.data()->blockSignals(false);
                pos.data()->triggerUpdate(false);
            }
            else
            {
                pos.data()->blockSignals(true);
                pos.data()->setSelected(newIndex, false);
                pos.data()->blockSignals(false);
            }
        }
    }
    hostTabs->blockSignals(false);

    hostName->blockSignals(true);
    QString oldHostName = hostName->currentText();
    hostName->clear();
    for(pos = hostTabMap.begin(); pos != hostTabMap.end(); ++pos)
        hostName->insertItem(pos.key());
    hostName->setEditText(oldHostName);
    hostName->blockSignals(false);
}

// ****************************************************************************
// Method: QvisHostProfileWindow::UpdateActiveProfile
//
// Purpose: 
//   This window updates the window's active profile area.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:14:29 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 17:58:51 PDT 2001
//   Added numNodes, launchMethod, and partitionName, as well as checkboxes
//   for each.
//
//   Jeremy Meredith, Thu Sep 13 17:33:48 PDT 2001
//   Added calls to block signals for and to set the launch check box.
//
//   Jeremy Meredith, Fri Sep 21 14:29:59 PDT 2001
//   Added buttons for forcing static and dynamic load balancing.
//
//   Brad Whitlock, Mon Sep 24 11:36:09 PDT 2001
//   Changed code so it replaces localhost with a real machine name.
//
//   Jeremy Meredith, Thu Feb 14 15:29:16 PST 2002
//   Changed hostName to a combo box.
//
//   Brad Whitlock, Thu Feb 21 10:18:04 PDT 2002
//   Replaced defaultUserName string with a method call to the viewer.
//
//   Sean Ahern, Thu Feb 21 16:37:28 PST 2002
//   Added timeout support.
//
//   Brad Whitlock, Thu Apr 11 15:34:51 PST 2002
//   Added support for default user names.
//
//   Jeremy Meredith, Wed Jul 10 13:00:02 PDT 2002
//   Made it set the right load balancing checkbox no matter if
//   parallel is enabled or not.
//
//   Jeremy Meredith, Wed Aug 14 10:50:12 PDT 2002
//   Changed the way many widgets are displayed.
//
//   Jeremy Meredith, Fri Jan 24 14:43:28 PST 2003
//   Added optional arguments to the parallel launcher.
//
//   Jeremy Meredith, Mon Apr 14 17:21:25 PDT 2003
//   Added hostAliases.
//
//   Jeremy Meredith, Thu Jun 26 10:37:38 PDT 2003
//   Update the share-batch-job toggle.
//
//   Jeremy Meredith, Thu Oct  9 15:47:00 PDT 2003
//   Added ability to manually specify a client host name or to have it
//   parsed from the SSH_CLIENT (or related) environment variables.  Added
//   ability to specify an SSH port.
//
//   Jeremy Meredith, Thu Sep 15 16:39:13 PDT 2005
//   Added machinefile and useVisItScriptForEnv.
//
//   Jeremy Meredith, Tue Sep 27 14:10:02 PDT 2005
//   Forgot to set a couple checkbox values to false if not a parallel profile.
//
//   Hank Childs, Sat Dec  3 20:55:49 PST 2005
//   Added support for new hardware acceleration options.
//
//   Eric Brugger, Thu Feb 15 12:14:34 PST 2007
//   Added support for additional sublauncher arguments.
//
//   Jeremy Meredith, Thu May 24 11:05:45 EDT 2007
//   Added support for SSH port tunneling.
//
//   Brad Whitlock, Wed Jun 6 09:37:06 PDT 2007
//   Changed a widget to be a QComboBox.
//
// ****************************************************************************

void
QvisHostProfileWindow::UpdateActiveProfile()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    int i = profiles->GetActiveProfile();

    // Block signals for certain widgets.
    numProcessors->blockSignals(true);
    timeout->blockSignals(true);
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
    launchCheckBox->blockSignals(true);
    launchMethod->blockSignals(true);
    activeProfileCheckBox->blockSignals(true);
    hostName->blockSignals(true);
    hostAliases->blockSignals(true);
    userName->blockSignals(true);
    loadBalancing->blockSignals(true);
    engineArguments->blockSignals(true);
    profileName->blockSignals(true);
    clientHostNameMethod->blockSignals(true);
    clientHostName->blockSignals(true);
    sshPortCheckBox->blockSignals(true);
    sshPort->blockSignals(true);
    canDoHW->blockSignals(true);
    preCommand->blockSignals(true);
    preCommandCheckBox->blockSignals(true);
    postCommand->blockSignals(true);
    postCommandCheckBox->blockSignals(true);
    tunnelSSH->blockSignals(true);

    // If there is no active profile, set some "default" values.
    if(i < 0)
    {
        profileName->setText("");
        hostName->setEditText(GetViewerProxy()->GetLocalHostName().c_str());
        hostAliases->setText("");
        userName->setText(GetViewerProxy()->GetLocalUserName().c_str());
        numProcessors->setValue(1);
        timeout->setValue(60*4);   // 4 hour default
        parallelCheckBox->setChecked(false);
        launchCheckBox->setChecked(false);
        launchMethod->setCurrentItem(0);
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
        loadBalancing->setCurrentItem(0);
        engineArguments->setText("");
        activeProfileCheckBox->setChecked(false);
        clientHostNameMethod->setButton(0);
        clientHostName->setText("");
        sshPortCheckBox->setChecked(false);
        sshPort->setText("");
        tunnelSSH->setChecked(false);
        shareMDServerCheckBox->setChecked(false);
        useVisItScriptForEnvCheckBox->setChecked(false);
    }
    else
    {
        const HostProfile &current = profiles->operator[](i);

        profileName->setText(current.GetProfileName().c_str());
        // Replace the "localhost" machine name.
        hostName->setEditText(current.GetHost().c_str());
        hostAliases->setText(current.GetHostAliases().c_str());
        // If there is no user name then give it a valid user name.
        if(current.GetUserName() == "notset")
            userName->setText(GetViewerProxy()->GetLocalUserName().c_str());
        else
            userName->setText(current.GetUserName().c_str());

        timeout->setValue(current.GetTimeout());
        parallelCheckBox->setChecked(current.GetParallel());
        bool parEnabled = current.GetParallel();
        if (parEnabled)
        {
            shareMDServerCheckBox->setChecked(current.GetShareOneBatchJob());
            useVisItScriptForEnvCheckBox->setChecked(current.GetVisitSetsUpEnv());
        }
        else
        {
            shareMDServerCheckBox->setChecked(false);
            useVisItScriptForEnvCheckBox->setChecked(false);
        }
        launchCheckBox->setChecked(parEnabled && current.GetLaunchMethodSet());
        if (parEnabled && current.GetLaunchMethodSet())
        {
            int index = 0;
            for (int j=0; j < launchMethod->count() ; j++)
            {
                if (launchMethod->text(j) == current.GetLaunchMethod().c_str())
                    index = j;
            }
            launchMethod->setCurrentItem(index);
        }
        else
        {
            launchMethod->setCurrentItem(0);
        }
        launchArgsCheckBox->setChecked(parEnabled && current.GetLaunchArgsSet());
        if (parEnabled && current.GetLaunchArgsSet())
            launchArgs->setText(current.GetLaunchArgs().c_str());
        else
            launchArgs->setText("");
        sublaunchArgsCheckBox->setChecked(parEnabled && current.GetSublaunchArgsSet());
        if (parEnabled && current.GetSublaunchArgsSet())
            sublaunchArgs->setText(current.GetSublaunchArgs().c_str());
        else
            sublaunchArgs->setText("");
        if (parEnabled)
            numProcessors->setValue(current.GetNumProcessors());
        else
            numProcessors->setValue(1);
        numNodesCheckBox->setChecked(parEnabled && current.GetNumNodesSet());
        if (parEnabled && current.GetNumNodesSet())
            numNodes->setValue(current.GetNumNodes());
        else
            numNodes->setValue(0);
        partitionCheckBox->setChecked(parEnabled && current.GetPartitionSet());
        if (parEnabled && current.GetPartitionSet())
            partitionName->setText(current.GetPartition().c_str());
        else
            partitionName->setText("");
        bankCheckBox->setChecked(parEnabled && current.GetBankSet());
        if (parEnabled && current.GetBankSet())
            bankName->setText(current.GetBank().c_str());
        else
            bankName->setText("");
        timeLimitCheckBox->setChecked(parEnabled && current.GetTimeLimitSet());
        if (parEnabled && current.GetTimeLimitSet())
            timeLimit->setText(current.GetTimeLimit().c_str());
        else
            timeLimit->setText("");
        machinefileCheckBox->setChecked(parEnabled && current.GetMachinefileSet());
        if (parEnabled && current.GetMachinefileSet())
            machinefile->setText(current.GetMachinefile().c_str());
        else
            machinefile->setText("");
        activeProfileCheckBox->setChecked(current.GetActive());
        int lb = 0;
        if (current.GetForceStatic())
            lb = 1;
        if (current.GetForceDynamic())
            lb = 2;
        loadBalancing->setCurrentItem(lb);
        // Turn the string list into a single QString.
        QString temp;
        stringVector::const_iterator pos;
        for(pos = current.GetArguments().begin();
            pos != current.GetArguments().end(); ++pos)
        {
            temp += QString(pos->c_str());
            temp += " ";
        }

        engineArguments->setText(temp);

        canDoHW->setChecked(current.GetCanDoHWAccel());
        preCommandCheckBox->setChecked(current.GetHavePreCommand());
        preCommand->setText(current.GetHwAccelPreCommand().c_str());
        postCommandCheckBox->setChecked(current.GetHavePostCommand());
        postCommand->setText(current.GetHwAccelPostCommand().c_str());

        switch (current.GetClientHostDetermination())
        {
          case HostProfile::MachineName:
            clientHostNameMethod->setButton(0);
            break;
          case HostProfile::ParsedFromSSHCLIENT:
            clientHostNameMethod->setButton(1);
            break;
          case HostProfile::ManuallySpecified:
            clientHostNameMethod->setButton(2);
            break;
        }
        clientHostName->setText(current.GetManualClientHostName().c_str());
        sshPortCheckBox->setChecked(current.GetSshPortSpecified());
        char portStr[256];
        SNPRINTF(portStr, 256, "%d", current.GetSshPort());
        sshPort->setText(portStr);
        tunnelSSH->setChecked(current.GetTunnelSSH());
    }

    // Set the widgets' sensitivity
    UpdateWindowSensitivity();

    // Restore signals.
    numProcessors->blockSignals(false);
    timeout->blockSignals(false);
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
    launchCheckBox->blockSignals(false);
    launchMethod->blockSignals(false);
    activeProfileCheckBox->blockSignals(false);
    hostName->blockSignals(false);
    hostAliases->blockSignals(false);
    userName->blockSignals(false);
    loadBalancing->blockSignals(false);
    engineArguments->blockSignals(false);
    profileName->blockSignals(false);
    clientHostNameMethod->blockSignals(false);
    clientHostName->blockSignals(false);
    sshPortCheckBox->blockSignals(false);
    sshPort->blockSignals(false);
    canDoHW->blockSignals(false);
    preCommand->blockSignals(false);
    preCommandCheckBox->blockSignals(false);
    postCommand->blockSignals(false);
    postCommandCheckBox->blockSignals(false);
    tunnelSSH->blockSignals(false);
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::ReplaceLocalHost()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    std::string      lh("localhost");
    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &current = profiles->operator[](i);
        if(current.GetHost() == lh)
            current.SetHost(GetViewerProxy()->GetLocalHostName());
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
// ****************************************************************************

void
QvisHostProfileWindow::UpdateWindowSensitivity()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    int i = profiles->GetActiveProfile();
    bool enabled = (i >= 0);
    const HostProfile *current = NULL;
    if (enabled)
        current = &profiles->operator[](i);
    bool parEnabled = enabled ? (current->GetParallel()) : false;

    // Set the widget sensitivity.
    activeProfileGroup->setEnabled(enabled);
    profileNameLabel->setEnabled(enabled);
    profileName->setEnabled(enabled);
    hostNameLabel->setEnabled(enabled);
    hostName->setEnabled(enabled);
    hostAliasesLabel->setEnabled(enabled);
    hostAliases->setEnabled(enabled);
    userNameLabel->setEnabled(enabled);
    userName->setEnabled(enabled);
    timeout->setEnabled(enabled);
    parGroup->setEnabled(parEnabled);
    optionsTabs->setTabEnabled(parGroup, parEnabled);
    launchCheckBox->setEnabled(parEnabled);
    launchMethod->setEnabled(parEnabled && current->GetLaunchMethodSet());
    launchArgsCheckBox->setEnabled(parEnabled);
    launchArgs->setEnabled(parEnabled && current->GetLaunchArgsSet());
    sublaunchArgsCheckBox->setEnabled(parEnabled);
    sublaunchArgs->setEnabled(parEnabled && current->GetSublaunchArgsSet());
    numProcLabel->setEnabled(parEnabled);
    numProcessors->setEnabled(parEnabled);
    numNodesCheckBox->setEnabled(parEnabled);
    numNodes->setEnabled(parEnabled && current->GetNumNodesSet());
    partitionCheckBox->setEnabled(parEnabled);
    partitionName->setEnabled(parEnabled && current->GetPartitionSet());
    bankCheckBox->setEnabled(parEnabled);
    bankName->setEnabled(parEnabled && current->GetBankSet());
    timeLimitCheckBox->setEnabled(parEnabled);
    timeLimit->setEnabled(parEnabled && current->GetTimeLimitSet());
    machinefileCheckBox->setEnabled(parEnabled);
    machinefile->setEnabled(parEnabled && current->GetMachinefileSet());
#if 0 // disabling dynamic load balancing for now
    loadBalancingLabel->setEnabled(parEnabled);
    loadBalancing->setEnabled(parEnabled);
#else
    loadBalancingLabel->setEnabled(false);
    loadBalancing->setEnabled(false);
#endif
    optionsTabs->setTabEnabled(advancedGroup, enabled);
    advancedGroup->setEnabled(enabled);

    canDoHW->setEnabled(enabled);
    preCommandCheckBox->setEnabled(enabled && current->GetCanDoHWAccel());
    postCommandCheckBox->setEnabled(enabled && current->GetCanDoHWAccel());
    preCommand->setEnabled(enabled && current->GetHavePreCommand() && current->GetCanDoHWAccel());
    postCommand->setEnabled(enabled && current->GetHavePostCommand() && current->GetCanDoHWAccel());
    engineArgumentsLabel->setEnabled(enabled);
    engineArguments->setEnabled(enabled);
    deleteButton->setEnabled(enabled);
    chnMachineName->setEnabled(enabled && current->GetTunnelSSH() == false);
    chnParseFromSSHClient->setEnabled(enabled && current->GetTunnelSSH() == false);
    chnSpecifyManually->setEnabled(enabled && current->GetTunnelSSH() == false);
    clientHostNameMethodLabel->setEnabled(enabled && current->GetTunnelSSH() == false);
    clientHostName->setEnabled(enabled &&
                               current->GetTunnelSSH() == false &&
                               current->GetClientHostDetermination() ==
                                              HostProfile::ManuallySpecified);
    sshPort->setEnabled(enabled && current->GetSshPortSpecified());
    tunnelSSH->setEnabled(enabled);

    shareMDServerCheckBox->setEnabled(parEnabled);
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
// ****************************************************************************
bool
QvisHostProfileWindow::GetCurrentValues(int which_widget)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    bool doAll = (which_widget == -1);
    bool needNotify = false;
    int widget = 0;

    // If there are no profiles, get out.
    if((profiles->GetNumProfiles() < 1) ||
       (profiles->GetActiveProfile() < 0))
        return needNotify;

    // Create a reference to the active profile.
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());    
    QString msg, temp;

    // Do the profile name
    if(which_widget == widget || doAll)
    {
        temp = profileName->displayText();
        if(!temp.isEmpty())
        {
            current.SetProfileName(std::string(temp.latin1()));

            // The profile name changed; we need to mark the profile list
            // so the new name appears in the list the next time update
            // is called.
            profiles->MarkHostProfiles();
        }
        else
        {
            needNotify = true;
            msg.sprintf("Profile name cannot be empty, reverting to \"%s\".",
                current.GetProfileName().c_str());
            Message(msg);
        }
    }
    widget++;

    // Do the host name
    if(which_widget == widget || doAll)
    {
        temp = hostName->currentText();
        temp = temp.stripWhiteSpace();
        if(!temp.isEmpty())
        {
            std::string newHost(temp.latin1());
            if(newHost == "localhost")
            {
                newHost = GetViewerProxy()->GetLocalHostName();
                hostName->setEditText(newHost.c_str());
            }
            if (newHost != current.GetHost())
                needNotify = true;
            profiles->ChangeHostForActiveProfile(newHost);
        }
        else
        {
            needNotify = true;
            msg.sprintf("Host name cannot be empty, reverting to \"%s\".",
                current.GetHost().c_str());
            Message(msg);
        }
    }
    widget++;

    // Do the user name
    if(which_widget == widget || doAll)
    {
        temp = userName->displayText();
        temp = temp.stripWhiteSpace();
        if(!temp.isEmpty())
        {
            current.SetUserName(std::string(temp.latin1()));
        }
        else
        {
            needNotify = true;
            msg.sprintf("Username cannot be empty, reverting to \"%s\".",
                current.GetUserName().c_str());
            Message(msg);
        }
    }
    widget++;

    // Do the launch method
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = launchMethod->currentText();
        temp = temp.stripWhiteSpace();
        if (temp == "(default)")
            temp = "";
        current.SetLaunchMethod(std::string(temp.latin1()));
    }
    widget++;

    // Do the number of processors
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        bool okay = false;
        temp = numProcessors->text();
        temp = temp.stripWhiteSpace();
        if(!temp.isEmpty())
        {
            int nProc = temp.toInt(&okay);
            if(okay)
            {
                if (nProc != current.GetNumProcessors())
                    needNotify = true;
                current.SetNumProcessors(nProc);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg.sprintf("An invalid number of processors was specified,"
                " reverting to %d processors.",
                current.GetNumProcessors());
            Message(msg);
        }
    }
    widget++;

    // Do the number of nodes
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        bool okay = false;
        temp = numNodes->text();
        temp = temp.stripWhiteSpace();
        if(!temp.isEmpty())
        {
            int nNodes = temp.toInt(&okay);
            if(okay)
            {
                current.SetNumNodes(nNodes);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg.sprintf("An invalid number of nodes was specified,"
                " reverting to %d nodes.",
                current.GetNumNodes());
            Message(msg);
        }
    }
    widget++;

    // Do the partition name
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = partitionName->displayText();
        temp = temp.stripWhiteSpace();
        current.SetPartition(std::string(temp.latin1()));
    }
    widget++;

    // Do the bank name
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = bankName->displayText();
        temp = temp.stripWhiteSpace();
        current.SetBank(std::string(temp.latin1()));
    }
    widget++;

    // Do the time limit
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = timeLimit->displayText();
        temp = temp.stripWhiteSpace();
        current.SetTimeLimit(std::string(temp.latin1()));
    }
    widget++;

    // Do the engine command line arguments.
    if(which_widget == widget || doAll)
    {
        stringVector arguments;
        QString temp(engineArguments->displayText());
        temp = temp.simplifyWhiteSpace();
        if(!(temp.isEmpty()))
        {
            // Split the arguments into a string list.
            QStringList str = QStringList::split(' ', temp);

            // Fill the arguments vector.
            for(int i = 0; i < str.count(); ++i)
            {
                arguments.push_back(std::string(str[i].latin1()));
            }
        }
        // Set the arguments.
        current.SetArguments(arguments);
    }
    widget++;

    // Do the launcher args
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = launchArgs->displayText();
        temp = temp.stripWhiteSpace();
        current.SetLaunchArgs(std::string(temp.latin1()));
    }
    widget++;

    // Do the timeout
    if(which_widget == widget || doAll)
    {
        bool okay = false;
        temp = timeout->text();
        temp = temp.stripWhiteSpace();
        if(!temp.isEmpty())
        {
            int tOut = temp.toInt(&okay);
            if(okay)
            {
                if (tOut != current.GetTimeout())
                    needNotify = true;
                current.SetTimeout(tOut);
            }
        }
 
        if(!okay)
        {
            needNotify = true;
            msg.sprintf("An invalid timeout was specified,"
                " reverting to %d minutes.",
                current.GetTimeout());
            Message(msg);
        }
    }
    widget++;

    // Do the host aliases
    if(which_widget == widget || doAll)
    {
        temp = hostAliases->text();
        temp = temp.stripWhiteSpace();

        std::string newAliases(temp.latin1());
        if (newAliases != current.GetHostAliases())
            needNotify = true;

        // Change all profiles with the same hostname
        for(int i = 0; i < profiles->GetNumProfiles(); ++i)
        {
            HostProfile &prof = profiles->operator[](i);

            if (prof.GetHost() == current.GetHost())
                prof.SetHostAliases(newAliases);
        }
    }
    widget++;

    // Do the manual client host name
    if(which_widget == widget || doAll)
    {
        temp = clientHostName->text();
        temp = temp.stripWhiteSpace();

        std::string newClientHostName(temp.latin1());
        if (newClientHostName != current.GetManualClientHostName())
            needNotify = true;

        // Change all profiles with the same hostname
        for(int i = 0; i < profiles->GetNumProfiles(); ++i)
        {
            HostProfile &prof = profiles->operator[](i);

            if (prof.GetHost() == current.GetHost())
                prof.SetManualClientHostName(newClientHostName);
        }
    }
    widget++;

    // Do the ssh port
    if(which_widget == widget || doAll)
    {
        temp = sshPort->text();

        int newPort = temp.toInt();
        if (current.GetSshPort() != newPort)
            needNotify = true;

        // Change all profiles with the same hostname
        for(int i = 0; i < profiles->GetNumProfiles(); ++i)
        {
            HostProfile &prof = profiles->operator[](i);

            if (prof.GetHost() == current.GetHost())
                prof.SetSshPort(newPort);
        }
    }
    widget++;

    // Do the machine file
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = machinefile->displayText();
        temp = temp.stripWhiteSpace();
        current.SetMachinefile(std::string(temp.latin1()));
    }
    widget++;

    // Do the sublauncher args
    if(current.GetParallel() && (which_widget == widget || doAll))
    {
        temp = sublaunchArgs->displayText();
        temp = temp.stripWhiteSpace();
        current.SetSublaunchArgs(std::string(temp.latin1()));
    }
    widget++;

    // There was an error with some of the input.
    if(needNotify)
    {
        // Mark the active profile so it will force the active profile
        // area to be updated.
        profiles->MarkActiveProfile();
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Update all of the values and call Notify.
        GetCurrentValues(-1);
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
// Method: QvisHostProfileWindow::newProfile
//
// Purpose: 
//   This is a Qt slot function that creates a new host profile, adds
//   it to the profile list and makes it the active profile.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:54:11 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 18:00:10 PDT 2001
//   Reworded "default profile" to be "active profile".
//
//   Brad Whitlock, Mon Sep 24 11:51:57 PDT 2001
//   Added code to set the default hostname to the localhost name if there
//   are no host profiles.
//
//   Brad Whitlock, Thu Feb 21 10:18:45 PDT 2002
//   I replaced defaultUserName with a method call to the viewer proxy.
//
//    Jeremy Meredith, Wed Jul 10 13:30:11 PDT 2002
//    Made the first created profile active.
//
// ****************************************************************************

void
QvisHostProfileWindow::newProfile()
{
    HostProfileList *profiles = (HostProfileList *)subject;
    ++profileCounter;

    HostProfile temp;
    // If there is a profile from which to copy, copy from it.
    if(profiles->GetNumProfiles() > 0)
    {
        const HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        temp = current;
        temp.SetActive(false);
    }
    else
    {
        // Set the default user name.
        temp.SetUserName(GetViewerProxy()->GetLocalUserName());
        // Set the default host name.
        temp.SetHost(GetViewerProxy()->GetLocalHostName());
        // Make the first created profile active.
        temp.SetActive(true);
    }

    QString profileName;
    profileName.sprintf("New profile #%d", profileCounter);
    temp.SetProfileName(std::string(profileName.latin1()));

    // Add the new profile to the list and make it the active profile.
    profiles->AddProfiles(temp);
    profiles->SetActiveProfile(profiles->GetNumProfiles() - 1);
    profiles->Notify();
}

// ****************************************************************************
// Method: QvisHostProfileWindow::deleteProfile
//
// Purpose: 
//   This is a Qt slot function that deletes the active profile and
//   makes the first profile the new active one.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:53:04 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
QvisHostProfileWindow::deleteProfile()
{
    HostProfileList *profiles = (HostProfileList *)subject;

    if(profiles->GetActiveProfile() != -1)
    {
        profiles->RemoveActiveProfile();
        profiles->Notify();
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::makeActiveProfile
//
// Purpose: 
//   This is a Qt slot function that makes the selected profile the 
//   active profile for its host.
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 18 12:37:28 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 18:00:10 PDT 2001
//   Reworded "default profile" to be "active profile".
//
//   Brad Whitlock, Mon Sep 24 09:25:48 PDT 2001
//   Added code to prevent the window from updating.
//
// ****************************************************************************

void
QvisHostProfileWindow::makeActiveProfile(bool)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    profiles->SetAsActiveProfile(profiles->GetActiveProfile());
    SetUpdate(false);
    Apply();
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
// ****************************************************************************

void
QvisHostProfileWindow::userNameChanged(const QString &u)
{
    if (u.isEmpty())
        return;

    // Update the user name.

    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
            prof.SetUserName(u.latin1());
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::toggleLaunch(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;

    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetLaunchMethodSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::launchMethodChanged(const QString &method)
{
    // Update the launch method.
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        QString temp(method.stripWhiteSpace());
        if(temp == "(default)")
            temp = "";
        current.SetLaunchMethod(std::string(temp.latin1()));
        profiles->MarkActiveProfile();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::numProcessorsChanged(int value)
{
    // Update the number of processors.
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetNumProcessors(value);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//
// ****************************************************************************
void
QvisHostProfileWindow::timeoutChanged(int value)
{
    // Update the timeout
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetTimeout(value);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::toggleNumNodes(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetNumNodesSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::numNodesChanged(int)
{
    // Update the number of nodes.
    if(!GetCurrentValues(5))
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
// ****************************************************************************

void
QvisHostProfileWindow::togglePartitionName(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetPartitionSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::processPartitionNameText(const QString &)
{
    // Update the partition name.
    if(!GetCurrentValues(6))
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleBankName(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetBankSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::processBankNameText(const QString &)
{
    // Update the bank name.
    if(!GetCurrentValues(7))
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleTimeLimit(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetTimeLimitSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleMachinefile(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetMachinefileSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::processTimeLimitText(const QString &)
{
    // Update the timeLimit name.
    if(!GetCurrentValues(8))
        SetUpdate(false);
    Apply();
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::processMachinefileText(const QString &)
{
    // Update the machinefile name.
    if(!GetCurrentValues(15))
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleLaunchArgs(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetLaunchArgsSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::processLaunchArgsText(const QString &)
{
    // Update the launch args text.
    if(!GetCurrentValues(10))
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleSublaunchArgs(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetSublaunchArgsSet(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
//   
// ****************************************************************************

void
QvisHostProfileWindow::processSublaunchArgsText(const QString &)
{
    // Update the sublaunch args text.
    if(!GetCurrentValues(16))
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
// ****************************************************************************

void
QvisHostProfileWindow::toggleParallel(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        GetCurrentValues(-1);
        Apply();
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetParallel(state);
        profiles->MarkActiveProfile();
        UpdateActiveProfile();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void 
QvisHostProfileWindow::loadBalancingChanged(int val)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

        switch (val)
        {
          case 0:
            current.SetForceStatic(false);
            current.SetForceDynamic(false);
            break;
          case 1:
            current.SetForceStatic(true);
            current.SetForceDynamic(false);
            break;
          case 2:
            current.SetForceStatic(false);
            current.SetForceDynamic(true);
            break;
        }

        profiles->MarkActiveProfile();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::hostNameChanged(const QString &n)
{
    // Update the host name.
    if(!GetCurrentValues(1))
        SetUpdate(false);
    Apply();
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
// ****************************************************************************

void
QvisHostProfileWindow::hostAliasesChanged(const QString &aliases)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
            prof.SetHostAliases(aliases.latin1());
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
// ****************************************************************************

void
QvisHostProfileWindow::processEngineArgumentsText(const QString &)
{
    // Update the engine arguments.
    if(!GetCurrentValues(9))
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleShareMDServer(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetShareOneBatchJob(state);
        profiles->MarkActiveProfile();
        SetUpdate(false);
        Apply();
    }
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
//
// ****************************************************************************

void
QvisHostProfileWindow::toggleUseVisItScriptForEnv(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetVisitSetsUpEnv(state);
        profiles->MarkActiveProfile();
        SetUpdate(false);
        Apply();
    }
}


// ****************************************************************************
// Method: QvisHostProfileWindow::activateProfile
//
// Purpose: 
//   This is a Qt slot function that is called when the active profile
//   changes. This translates the widget action into a state object
//   change.
//
// Arguments:
//   item : A pointer to the listviewitem that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Wed Sep 20 15:50:57 PST 2000
//
// Modifications:
//    Jeremy Meredith, Fri Dec 20 10:57:11 PST 2002
//    Added code to get the current values in case they were changed
//    before changing profiles.
//   
//    Jeremy Meredith, Mon Apr 14 18:28:04 PDT 2003
//    Removed last change.  It caused stability problems.
//
// ****************************************************************************

void
QvisHostProfileWindow::activateProfile(QListBoxItem *item)
{
    if(item == 0)
        return;

    // Cast the subject pointer to something useful.
    HostProfileList *profiles = (HostProfileList *)subject;

    // Get the host asociated with the item.
    std::string hostName;
    HostTabMap::Iterator pos;
    bool keepSearching = true;
    for(pos = hostTabMap.begin(); pos != hostTabMap.end() && keepSearching; ++pos)
    {
        if(pos.data() == item->listBox())
        {
            hostName = std::string(pos.key().latin1());
            keepSearching = false;
        }
    }

    // Get the index of the selected profile in the profile list.
    keepSearching = true;
    int index = 0;
    for(int i = 0; i < profiles->GetNumProfiles() && keepSearching; ++i)
    {
        const HostProfile &current = profiles->operator[](i);
        if(current.GetHost() == hostName &&
           current.GetProfileName() == std::string(item->text().latin1()))
        {
            index = i;
            keepSearching = false;
        }
    }

    // We now have the index. If it is good, and is not equal to the
    // current active profile's index, change the active profile.
    if((index < profiles->GetNumProfiles()) &&
       (index != profiles->GetActiveProfile()))
    {
        profiles->SetActiveProfile(index);
        profiles->Notify();
    }
}

// ****************************************************************************
// Method: QvisHostProfileWindow::pageTurned
//
// Purpose: 
//   This is a Qt slot function that selects the default profile in
//   the host's profile list when the pages of the host tab are
//   turned.
//
// Arguments:
//   hostName : The host name of the tab that was made visible.
//
// Programmer: Brad Whitlock
// Creation:   Tue Oct 17 16:51:11 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Jul 17 18:00:10 PDT 2001
//   Reworded "default profile" to be "active profile".
//
//   Brad Whitlock, Mon Sep 24 12:25:09 PDT 2001
//   Added code to get the current values before changing the tab that
//   we're displaying.
//
//   Jeremy Meredith, Thu Feb 14 15:30:18 PST 2002
//   Changed to a different style callback since the hostname in the
//   tab is only a shortened version of the ones in the profiles.
//
//   Jeremy Meredith, Thu Jun 24 10:09:40 PDT 2004
//   Forced an update of the profile list to fix a bug.  ('5083)
//
// ****************************************************************************

void
QvisHostProfileWindow::pageTurned(QWidget *tab)
{
    QString host = "";
    HostTabMap::Iterator pos = hostTabMap.begin();
    while (pos != hostTabMap.end())
    {
        if (pos.data() == tab)
        {
            host = pos.key();
            break;
        }
        pos++;
    }

    if(pos != hostTabMap.end())
    {
        // Get the current attributes in case they were changed.
        GetCurrentValues(-1);

        // If the user has changed the host name but never hit apply
        // or return, then a dismiss/show of this window causes a
        // pageTurned event but can leave the tab names out of date;
        // force an update here.
        UpdateProfileList();

        // Cast the subject pointer to something useful.
        HostProfileList *profiles = (HostProfileList *)subject;

        // Get a pointer to the active profile for this host.
        HostProfile *activeProfile = (HostProfile *)profiles->GetProfileForHost(
            std::string(host.latin1()));
        if(activeProfile == 0)
        {
            return;
        }

        // Get the index of the active profile and activate it.
        int index = 0;
        bool keepGoing = true;
        for(int i = 0; i < profiles->GetNumProfiles() && keepGoing; ++i)
        {
            HostProfile &current = profiles->operator[](i);
            if(current == *activeProfile)
            {
                index = i;
                keepGoing = false;
            }
        }

        // We now have the index. If it is good, and is not equal to the
        // current active profile's index, change the active profile.
        if(index < profiles->GetNumProfiles())
        {
            profiles->MarkHostProfiles();
            profiles->SetActiveProfile(index);
            profiles->Notify();
        }
    }
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
// ****************************************************************************
void
QvisHostProfileWindow::processProfileNameText(const QString &name)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());    
    QString temp = profileName->displayText();
    if (!temp.isEmpty())
    {
        current.SetProfileName(temp.latin1());
        hostTabMap[current.GetHost().c_str()]->changeItem(temp,
                        hostTabMap[current.GetHost().c_str()]->currentItem());
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
// ****************************************************************************
void
QvisHostProfileWindow::toggleSSHPort(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
            prof.SetSshPortSpecified(state);
    }

    profiles->MarkActiveProfile();
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
// ****************************************************************************
void
QvisHostProfileWindow::sshPortChanged(const QString &portStr)
{
    if (portStr.isEmpty())
        return;

    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        int port = atoi(portStr.latin1());

        if (prof.GetHost() == current.GetHost())
            prof.SetSshPort(port);
    }
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
// ****************************************************************************
void
QvisHostProfileWindow::clientHostNameMethodChanged(int m)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
        {
            switch (m)
            {
              case 0:
                prof.SetClientHostDetermination(HostProfile::MachineName);
                break;
              case 1:
                prof.SetClientHostDetermination(HostProfile::ParsedFromSSHCLIENT);
                break;
              case 2:
                prof.SetClientHostDetermination(HostProfile::ManuallySpecified);
                break;
            }
        }
    }

    profiles->MarkActiveProfile();
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
// ****************************************************************************
void
QvisHostProfileWindow::clientHostNameChanged(const QString &h)
{
    if (h.isEmpty())
        return;

    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
            prof.SetManualClientHostName(h.latin1());
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::toggleTunnelSSH(bool tunnel)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if (profiles->GetActiveProfile() < 0)
        return;
    HostProfile &current = profiles->operator[](profiles->GetActiveProfile());

    for(int i = 0; i < profiles->GetNumProfiles(); ++i)
    {
        HostProfile &prof = profiles->operator[](i);

        if (prof.GetHost() == current.GetHost())
        {
            prof.SetTunnelSSH(tunnel);
            if (tunnel)
            {
                prof.SetClientHostDetermination(HostProfile::MachineName);
                prof.SetManualClientHostName("");
                clientHostNameMethod->blockSignals(true);
                clientHostName->blockSignals(true);
                clientHostNameMethod->setButton(0);
                clientHostName->setText("");
                clientHostNameMethod->blockSignals(false);
                clientHostName->blockSignals(false);
            }
        }
    }

    profiles->MarkActiveProfile();
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
// ****************************************************************************
void
QvisHostProfileWindow::toggleCanDoHW(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetCanDoHWAccel(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************
void
QvisHostProfileWindow::togglePreCommand(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetHavePreCommand(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************
void
QvisHostProfileWindow::togglePostCommand(bool state)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        current.SetHavePostCommand(state);
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************

void
QvisHostProfileWindow::preCommandChanged(const QString &portStr)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        QString temp, msg;
        temp = preCommand->displayText();
        if(!temp.isEmpty())
        {
            current.SetHwAccelPreCommand(std::string(temp.latin1()));
        }
        else
        {
            msg.sprintf("Pre-command cannot be empty, turning off pre-command.");
            current.SetHavePreCommand(false);
        }
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
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
// ****************************************************************************
void
QvisHostProfileWindow::postCommandChanged(const QString &portStr)
{
    HostProfileList *profiles = (HostProfileList *)subject;
    if(profiles->GetActiveProfile() >= 0)
    {
        HostProfile &current = profiles->operator[](profiles->GetActiveProfile());
        QString temp, msg;
        temp = postCommand->displayText();
        if(!temp.isEmpty())
        {
            current.SetHwAccelPostCommand(std::string(temp.latin1()));
        }
        else
        {
            msg.sprintf("Post-command cannot be empty, turning off post-command.");
            current.SetHavePostCommand(false);
        }
        profiles->MarkActiveProfile();
        UpdateWindowSensitivity();
        SetUpdate(false);
        Apply();
    }
}
