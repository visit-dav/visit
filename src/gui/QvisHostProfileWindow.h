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

#ifndef QVIS_HOST_PROFILE_WINDOW_H
#define QVIS_HOST_PROFILE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <qmap.h>

// Forward declrations
class HostProfileList;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QLabel;
class QComboBox;
class QTabWidget;
class QListBox;
class QListBoxItem;
class QCheckBox;
class QButtonGroup;
class QRadioButton;

// ****************************************************************************
//  Class: QvisHostProfileWindow
//
//  Purpose:
//    This window manages the host profiles that are used to launch
//    remote VisIt engines.
//
//  Notes:      
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Sep 19 18:06:49 PST 2000
//
//  Modifications:
//    Brad Whitlock, Wed May 2 11:41:28 PDT 2001
//    Made it postable by switching the class from which it inherits.
//
//    Jeremy Meredith, Tue Jul 17 17:42:07 PDT 2001
//    Added numNodes, launchMethod, and partitionName, as well as checkboxes
//    for each.  Also reworded "default profile" to be "active profile".
//
//    Jeremy Meredith, Fri Sep 21 14:29:59 PDT 2001
//    Added buttons for forcing static and dynamic load balancing.
//
//    Brad Whitlock, Mon Sep 24 09:22:24 PDT 2001
//    Added the internal Apply, ReplaceLocalHost methods.
//
//    Jeremy Meredith, Thu Feb 14 15:14:32 PST 2002
//    I made hostName a combo box instead of a line edit.
//    I also changed the type of callback for the tab page changed.
//
//    Brad Whitlock, Thu Feb 21 10:16:38 PDT 2002
//    I removed the defaultUserName string.
//
//    Jeremy Meredith, Mon Jul 22 11:38:55 PDT 2002
//    Added bank name.
//
//    Jeremy Meredith, Thu Nov 21 11:28:29 PST 2002
//    Added processProfileNameText.
//
//    Jeremy Meredith, Fri Jan 24 14:43:28 PST 2003
//    Added optional arguments to the parallel launcher.
//
//    Jeremy Meredith, Mon Apr 14 18:25:17 PDT 2003
//    Added host aliases.
//
//    Jeremy Meredith, Mon May 19 12:49:17 PDT 2003
//    Added shareMDServer toggle.
//
//    Jeremy Meredith, Mon Aug 18 13:37:25 PDT 2003
//    Changed processUserNameText to userNameChanged.
//
//    Jeremy Meredith, Thu Oct  9 15:43:39 PDT 2003
//    Added ability to manually specify a client host name or to have it
//    parsed from the SSH_CLIENT (or related) environment variables.  Added
//    ability to specify an SSH port.
//
//    Jeremy Meredith, Thu Sep 15 15:20:55 PDT 2005
//    Added machine file support and the ability to use the VisIt script
//    to set up env variables in parallel.
//
//    Hank Childs, Fri Dec  2 11:01:41 PST 2005
//    Added support for hardware acceleration.
//
//    Eric Brugger, Thu Feb 15 12:10:21 PST 2007
//    Added support for additional sublauncher arguments.
//
//    Jeremy Meredith, Thu May 24 11:05:45 EDT 2007
//    Added support for SSH port tunneling.
//
//    Brad Whitlock, Tue Jun 5 15:53:20 PST 2007
//    Changed all slot functions for QLineEdit so they modify the host profile
//    as text is typed to avoid losing data when editing multiple profiles
//    and not clicking Apply.
//
//    Jeremy Meredith, Thu Jun 28 13:31:38 EDT 2007
//    Have the ssh tunneling option override and disable the client host
//    name determination method.
//
// ****************************************************************************

class GUI_API QvisHostProfileWindow : public QvisPostableWindowObserver
{
    Q_OBJECT

    typedef QMap<QString, QListBox*> HostTabMap;
public:
    QvisHostProfileWindow(HostProfileList *profiles, 
                          const char *caption = 0,
                          const char *shortName = 0,
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisHostProfileWindow();
    virtual void CreateWindowContents();
private:
    void UpdateWindow(bool doAll);
    void UpdateProfileList();
    void UpdateActiveProfile();
    void UpdateWindowSensitivity();
    bool GetCurrentValues(int);
    void Apply(bool val = false);
    void ReplaceLocalHost();
private slots:
    void newProfile();
    void deleteProfile();
    void apply();

    void activateProfile(QListBoxItem *item);
    void pageTurned(QWidget *);
    void processProfileNameText(const QString &name);
    void processEngineArgumentsText(const QString &);
    void processPartitionNameText(const QString &);
    void processBankNameText(const QString &);
    void processTimeLimitText(const QString &);
    void processMachinefileText(const QString &);
    void processLaunchArgsText(const QString &);
    void processSublaunchArgsText(const QString &);
    void numProcessorsChanged(int value);
    void timeoutChanged(int value);
    void launchMethodChanged(const QString &method);
    void numNodesChanged(int value);
    void makeActiveProfile(bool);
    void toggleLaunch(bool);
    void toggleNumNodes(bool);
    void togglePartitionName(bool);
    void toggleBankName(bool);
    void toggleTimeLimit(bool);
    void toggleMachinefile(bool);
    void toggleLaunchArgs(bool);
    void toggleSublaunchArgs(bool);
    void toggleParallel(bool);
    void toggleShareMDServer(bool);
    void toggleUseVisItScriptForEnv(bool);
    void loadBalancingChanged(int);
    void hostNameChanged(const QString &host);
    void hostAliasesChanged(const QString &host);
    void userNameChanged(const QString &username);
    void toggleSSHPort(bool);
    void sshPortChanged(const QString &port);
    void clientHostNameMethodChanged(int);
    void clientHostNameChanged(const QString &);
    void toggleCanDoHW(bool);
    void preCommandChanged(const QString &);
    void togglePreCommand(bool);
    void postCommandChanged(const QString &);
    void togglePostCommand(bool);
    void toggleTunnelSSH(bool);
private:
    QWidget *CreateSelectedTab(QWidget *parent);
    QWidget *CreateParallelTab(QWidget *parent);
    QWidget *CreateAdvancedTab(QWidget *parent);
    QWidget *CreateHardwareAccelerationTab(QWidget *parent);
    QWidget *CreateNetworkingTab(QWidget *parent);

    QTabWidget   *hostTabs;
    QListBox     *emptyListBox;
    HostTabMap   hostTabMap;

    QTabWidget   *optionsTabs;
    QPushButton  *newButton;
    QPushButton  *deleteButton;
    QWidget      *activeProfileGroup;
    QLabel       *profileNameLabel;
    QLineEdit    *profileName;
    QCheckBox    *activeProfileCheckBox;
    QLabel       *hostNameLabel;
    QComboBox    *hostName;
    QLabel       *hostAliasesLabel;
    QLineEdit    *hostAliases;
    QLabel       *userNameLabel;
    QLineEdit    *userName;
    QLabel       *numProcLabel;
    QLabel       *timeoutLabel;
    QSpinBox     *numProcessors;
    QSpinBox     *timeout;
    QCheckBox    *launchCheckBox;
    QComboBox    *launchMethod;
    QCheckBox    *numNodesCheckBox;
    QSpinBox     *numNodes;
    QCheckBox    *partitionCheckBox;
    QLineEdit    *partitionName;
    QCheckBox    *bankCheckBox;
    QLineEdit    *bankName;
    QCheckBox    *timeLimitCheckBox;
    QLineEdit    *timeLimit;
    QCheckBox    *machinefileCheckBox;
    QLineEdit    *machinefile;
    QLabel       *loadBalancingLabel;
    QComboBox    *loadBalancing;
    QLabel       *engineArgumentsLabel;
    QLineEdit    *engineArguments;
    QCheckBox    *parallelCheckBox;
    QWidget      *parGroup;
    QCheckBox    *launchArgsCheckBox;
    QLineEdit    *launchArgs;
    QCheckBox    *sublaunchArgsCheckBox;
    QLineEdit    *sublaunchArgs;
    QWidget      *advancedGroup;
    QCheckBox    *shareMDServerCheckBox;
    QCheckBox    *useVisItScriptForEnvCheckBox;
    QButtonGroup *clientHostNameMethod;
    QRadioButton *chnMachineName;
    QRadioButton *chnParseFromSSHClient;
    QRadioButton *chnSpecifyManually;
    QLineEdit    *clientHostName;
    QLabel       *clientHostNameMethodLabel;
    QCheckBox    *sshPortCheckBox;
    QLineEdit    *sshPort;
    QWidget      *hwGroup;
    QCheckBox    *canDoHW;
    QLineEdit    *preCommand;
    QCheckBox    *preCommandCheckBox;
    QLineEdit    *postCommand;
    QCheckBox    *postCommandCheckBox;
    QCheckBox    *tunnelSSH;
    QWidget      *networkingGroup;
    int          profileCounter;
};

#endif
