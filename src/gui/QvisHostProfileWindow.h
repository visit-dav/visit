// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_HOST_PROFILE_WINDOW_H
#define QVIS_HOST_PROFILE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <QMap>
#include <QModelIndex>

// Forward declrations
class HostProfileList;
class MachineProfile;
class LaunchProfile;
class QPushButton;
class QLabel;
class QLineEdit;
class QSpinBox;
class QGroupBox;
class QLabel;
class QComboBox;
class QTabWidget;
class QListWidget;
class QListWidgetItem;
class QCheckBox;
class QButtonGroup;
class QRadioButton;
class QNetworkReply;
class QTableWidget;
class QTreeWidget;
class QNetworkAccessManager;

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
//    Brad Whitlock, Wed Apr  9 10:57:10 PDT 2008
//    QString for caption, shortName.
//
//    Dave Bremer, Wed Apr 16 17:54:14 PDT 2008
//    Added fields for commands to run pre and post the mpi command.
//
//    Cyrus Harrison, Tue Jun 24 11:15:28 PDT 2008
//    Initial Qt4 Port.
//
//    Hank Childs, Thu May  7 19:05:36 PDT 2009
//    Added field for host nickname.
//
//    Jeremy Meredith, Thu Feb 18 15:54:11 EST 2010
//    Overhauled window entirely.  Split HostProfile into
//    MachineProfile and LaunchProfile.
//
//    Eric Brugger, Tue Apr 19 20:37:15 PDT 2011
//    I added the ability to specify a gateway machine to use to get to the
//    remote host.
//
//    Tom Fogal, Fri May  6 18:14:43 MDT 2011
//    New widgets for parallel GPU configuration.
//    Removed deprecated/broken precommand/postcommand settings.
//
//    Brad Whitlock, Wed Aug 15 13:55:13 PDT 2012
//    I added controls to set the ssh command.
//
//    David Camp, Mon Aug  4 10:46:09 PDT 2014
//    Added controls to set the threads option.
//
//    Kathleen Biagas, Wed Dec 16 11:07:43 MST 2015
//    Replace slot 'sshCommandChanged' with 'sshCommandRetPressed', so that
//    sshCommand is only processed once editing has finished.  Will be
//    triggered by 'returnPressed'/'editingFinished' signals from the widget.
//
// ****************************************************************************

class GUI_API QvisHostProfileWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
    friend class DropListWidget; //todo remove this dependency..
public:
    QvisHostProfileWindow(HostProfileList *profiles, 
                          const QString &caption = QString(),
                          const QString &shortName = QString(),
                          QvisNotepadArea *notepad = 0);
    virtual ~QvisHostProfileWindow();
    virtual void CreateWindowContents();
private:
    void UpdateWindow(bool doAll);
    void UpdateMachineProfile();
    void UpdateLaunchProfile();
    void UpdateWindowSensitivity();
    bool GetCurrentValues();
    void Apply(bool val = false);
    void ReplaceLocalHost();
    void addRemoteProfile(const QString& inputUrl, const QString& results);
    void ListWidgetDropEvent(QDropEvent * event);
    void addChildren(const QModelIndex& list, QStringList& suffixList, QStringList& globalList);
    void selectProfiles(const QModelIndex& index);
    void ResizeNodeProcs(int newSize, bool blank);
private slots:
    void apply();

    void currentHostChanged();
    void currentLaunchChanged();

    void addMachineProfile();
    void delMachineProfile();
    void copyMachineProfile();
    void exportMachineProfile();

    void addLaunchProfile();
    void delLaunchProfile();
    void copyLaunchProfile();
    void makeDefaultLaunchProfile();
    void selectProfiles();
    void retrieveLatestProfiles();
    void downloadHosts(QNetworkReply* reply);

    void processProfileNameText(const QString &);
    void processDirectoryText(const QString &);
    void processEngineArgumentsText(const QString &);
    void processPartitionNameText(const QString &);
    void processBankNameText(const QString &);
    void processTimeLimitText(const QString &);
    void processMachinefileText(const QString &);
    void processLaunchArgsText(const QString &);
    void processSublaunchArgsText(const QString &);
    void processSublaunchPreCmdText(const QString &);
    void processSublaunchPostCmdText(const QString &);
    void numProcessorsChanged(int value);
    void timeoutChanged(int value);
    void threadsChanged(int value);
    void launchMethodChanged(const QString &method);
    void numNodesChanged(int value);
    void toggleLaunch(bool);
    void toggleNumNodes(bool);
    void togglePartitionName(bool);
    void toggleBankName(bool);
    void toggleTimeLimit(bool);
    void toggleMachinefile(bool);
    void toggleLaunchArgs(bool);
    void toggleSublaunchArgs(bool);
    void toggleSublaunchPreCmd(bool);
    void toggleSublaunchPostCmd(bool);
    void toggleParallel(bool);
    void toggleShareMDServer(bool);
    void toggleUseVisItScriptForEnv(bool);
    void loadBalancingChanged(int);
    void hostNameChanged(const QString &host);
    void hostAliasesChanged(const QString &host);
    void hostNicknameChanged(const QString &host);
    void userNameChanged(const QString &username);
    void toggleSSHPort(bool);
    void sshPortChanged(const QString &port);
    void toggleSSHCommand(bool);
    void sshCommandRetPressed();
    void toggleUseGateway(bool);
    void gatewayHostChanged(const QString &port);
    void clientHostNameMethodChanged(int);
    void clientHostNameChanged(const QString &);
    void toggleCanDoHW(bool);
    void nGPUsChanged(const QString&);
    void toggleTunnelSSH(bool);
    void toggleXArgs(bool);
    void xArgsChanged(const QString&);
    void toggleLaunchX(bool);
    void xDisplayChanged(const QString&);
    void toggleUseMaxNodes(bool);
    void maxNodesChanged(int);
    void toggleUseMaxProcessors(bool);
    void maxProcessorsChanged(int);
    void allowableNodeProcsAddRow();
    void allowableNodeProcsDeleteRow();
    void toggleAllowableNodeProcs(bool);
private:
    // Main Window
    QListWidget *hostList;
    QPushButton *addHost;
    QPushButton *delHost;
    QPushButton *copyHost;
    QPushButton *exportHost;

    QTabWidget *machineTabs;
    QWidget    *machineSettingsGroup;
    QWidget    *launchProfilesGroup;
    QWidget    *remoteProfilesGroup;
    QWidget    *CreateMachineSettingsGroup();
    QWidget    *CreateLaunchProfilesGroup();
    QWidget    *CreateRemoteProfilesGroup();

    // Launch Profiles
    QListWidget *profileList;
    QPushButton *addProfile;
    QPushButton *delProfile;
    QPushButton *copyProfile;
    QPushButton *makeDefaultProfile;

    QLabel       *profileNameLabel;
    QLineEdit    *profileName;
    QCheckBox    *parallelCheckBox;

    QTabWidget *profileTabs;
    QWidget    *basicSettingsGroup;
    QWidget    *parallelSettingsGroup;
    QWidget    *hwAccelSettingsGroup;
    QWidget    *CreateBasicSettingsGroup();
    QWidget    *CreateParallelSettingsGroup();
    QWidget    *CreateLaunchSettingsGroup();
    QWidget    *CreateAdvancedSettingsGroup();
    QWidget    *CreateHWAccelSettingsGroup();

    // Parallel Basic Settings
    QTabWidget   *parallelTabs;
    QCheckBox    *launchCheckBox;
    QComboBox    *launchMethod;
    QCheckBox    *partitionCheckBox;
    QLineEdit    *partitionName;
    QLabel       *numProcLabel;
    QSpinBox     *numProcessors;
    QCheckBox    *numNodesCheckBox;
    QSpinBox     *numNodes;
    QCheckBox    *bankCheckBox;
    QLineEdit    *bankName;
    QCheckBox    *timeLimitCheckBox;
    QLineEdit    *timeLimit;
    QCheckBox    *machinefileCheckBox;
    QLineEdit    *machinefile;
    QGroupBox    *constraintGroup;
    QTableWidget *allowableNodeProcs;

    // Parallel Advanced Settings
    QCheckBox    *useVisItScriptForEnvCheckBox;
    QLabel       *loadBalancingLabel;
    QComboBox    *loadBalancing;
    QCheckBox    *launchArgsCheckBox;
    QLineEdit    *launchArgs;
    QCheckBox    *sublaunchArgsCheckBox;
    QLineEdit    *sublaunchArgs;
    QCheckBox    *sublaunchPreCmdCheckBox;
    QLineEdit    *sublaunchPreCmd;
    QCheckBox    *sublaunchPostCmdCheckBox;
    QLineEdit    *sublaunchPostCmd;
    QLabel       *timeoutLabel;
    QSpinBox     *timeout;
    QLabel       *threadsLabel;
    QSpinBox     *threads;

    // Launch HW Accel Settings
    QLabel       *hwdisclaimer;
    QGroupBox    *hardwareGroup;
    QCheckBox    *cbLaunchX;
    QSpinBox     *sbNGPUs;
    QCheckBox    *cbXArgs;
    QLineEdit    *txtXArgs;
    QLineEdit    *txtXDisplay;

    // Machine Settings
    QLabel       *hostNameLabel;
    QLineEdit    *hostName;
    QLabel       *hostAliasesLabel;
    QLineEdit    *hostAliases;
    QLabel       *hostNicknameLabel;
    QLineEdit    *hostNickname;
    QLabel       *userNameLabel;
    QLineEdit    *userName;
    QCheckBox    *shareMDServerCheckBox;
    QCheckBox    *tunnelSSH;
    QButtonGroup *clientHostNameMethod;
    QRadioButton *chnMachineName;
    QRadioButton *chnParseFromSSHClient;
    QRadioButton *chnSpecifyManually;
    QLineEdit    *clientHostName;
    QLabel       *clientHostNameMethodLabel;
    QCheckBox    *sshPortCheckBox;
    QLineEdit    *sshPort;
    QCheckBox    *sshCommandCheckBox;
    QLineEdit    *sshCommand;
    QCheckBox    *useGatewayCheckBox;
    QLineEdit    *gatewayHost;
    QLabel       *engineArgumentsLabel;
    QLineEdit    *engineArguments;
    QLabel       *directoryLabel;
    QLineEdit    *directory;
    QCheckBox    *maxNodesCheckBox;
    QSpinBox     *maxNodes;
    QCheckBox    *maxProcessorsCheckBox;
    QSpinBox     *maxProcessors;

    // other stuff
    QComboBox     *remoteUrl;
    QTreeWidget     *remoteTree;
    QMap<QString, QString> remoteData;
    QMap<QString, QString> profileMap;
    QNetworkAccessManager *manager;
    int          profileCounter;
    MachineProfile *currentMachine;
    LaunchProfile  *currentLaunch;
};
#endif
