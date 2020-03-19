// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisSetupHostProfilesAndConfigWindow.h>

#include <iostream>
#include <fstream>

#include <QDir>
#include <QFile>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <InstallationFunctions.h>
#include <DebugStream.h>

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::QvisSetupHostProfilesAndConfigWindow
//
// Purpose:
//   Constructor for the QvisSetupHostProfilesAndConfigWindow class.
//
// Arguments:
//   captionString : The window caption.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//
// ****************************************************************************

QvisSetupHostProfilesAndConfigWindow::QvisSetupHostProfilesAndConfigWindow(
        const QString &captionString) : QvisDelayedWindow(captionString)
{
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::~QvisSetupHostProfilesAndConfigWindow
//
// Purpose:
//   Destructor for the QvisSetupHostProfilesAndConfigWindow class.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//
// ****************************************************************************

QvisSetupHostProfilesAndConfigWindow::~QvisSetupHostProfilesAndConfigWindow()
{
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::CreateWindowContents
//
// Purpose:
//   Creates the widgets for the Mac app bundle setup window.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//   Kathleen Biagas, Thu Nov  7 12:54:14 PST 2019
//   Use QListWidgets for scrollability and window height control. 
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::CreateWindowContents()
{
    networkList = new QListWidget();
    defaultConfigList = new QListWidget();
    defaultConfigShortNames.append("None");

    QRadioButton *radioButton = new QRadioButton(
            tr("None (use VisIt's standard defaults)"));
    radioButton->setChecked(true);

    QListWidgetItem *item = new QListWidgetItem();
    defaultConfigList->addItem(item);
    defaultConfigList->setItemWidget(item, radioButton);

    readNetworkList();
    readDefaultConfigList();

    QLabel *label = new QLabel(tr("To finish the VisIt install on this computer select "
                "any computing centers whose resources you are using to "
                "configure host profiles automatically for their machines."));
    label->setWordWrap(true);
    topLayout->addWidget(label);

    QLabel *l = new QLabel(tr("Select computing centers used"));
    topLayout->addWidget(l);

    // finish setting up the networkList as a checkable list with
    // nothing checked initially.
    for (int i = 0; i < networkList->count(); ++i)
    {
        item = networkList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
    networkList->setMinimumWidth(networkList->sizeHintForColumn(0));
    topLayout->addWidget(networkList);

    QLabel *clabel = new QLabel(tr("Select default configuration"));
    topLayout->addWidget(clabel);

    // this will help constrain the height, since the list is currently short.
    // if the list gets too long, may need to remove this.
    defaultConfigList->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    topLayout->addWidget(defaultConfigList);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    topLayout->addLayout(buttonLayout);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch(1);
    QObject::connect(
            cancelButton, SIGNAL(clicked()),
            this, SLOT(close()));
    QPushButton *installButton = new QPushButton(tr("Install"));
    buttonLayout->addWidget(installButton);
    QObject::connect(
            installButton, SIGNAL(clicked()),
            this, SLOT(performSetup()));
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::readNetworkList()
//
// Purpose:
//   Read list of "network/computing center" configurations.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//   Brad Whitlock, Thu Sep  6 11:05:19 PDT 2012
//   Get the file from resources/hosts.
//
//   Kathleen Biagas, Thu Nov  7 12:54:14 PST 2019
//   Info now stored in QListWidgets and QStringLists.
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::readNetworkList()
{
    std::ifstream is(GetVisItResourcesFile(VISIT_RESOURCES_HOSTS, "networks.dat").c_str());

    std::string line;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        size_t splitPos = line.find(':');
        if (splitPos == std::string::npos)
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }
        networkList->addItem(line.substr(0, splitPos).c_str());
        networkShortNames.append(line.substr(splitPos+1).c_str());
    }
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::readDefaultConfigList()
//
// Purpose:
//   Read list of default configurations.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//   Brad Whitlock, Thu Sep  6 11:11:40 PDT 2012
//   Get the file from the resources/hosts directory.
//
//   Kathleen Biagas, Thu Nov  7 12:54:14 PST 2019
//   Info now stored in QListWidgets and QStringLists.
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::readDefaultConfigList()
{
    std::ifstream is(GetVisItResourcesFile(VISIT_RESOURCES_HOSTS, "default_configs.dat").c_str());

    std::string line;
    QListWidgetItem *item = 0;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        size_t splitPos = line.find(':');
        if (splitPos == std::string::npos)
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }
        item = new QListWidgetItem();
        defaultConfigList->addItem(item);
        defaultConfigList->setItemWidget(item, new QRadioButton(line.substr(0, splitPos).c_str()));

        defaultConfigShortNames.append(line.substr(splitPos+1).c_str());
    }
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::installConfigFile()
//
// Purpose:
//   Install a configuration file. Ask via dialog if a file should be
//   overwritten if it exists..
//
// Arguments:
//   srcFilename   Filename of the file to copy (full path)
//   destFilename  Filename at the destination (full path)
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//
//    Kevin Griffin, Thu Nov  7 17:43:56 PST 2019
//    Added a check to see if the files were successfully copied. If not, a
//    debug log message is generated to aid in troubleshooting.
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::installConfigFile(const QString& srcFilename,
        const QString& destFilename)
{
    if (QFile::exists(destFilename))
    {
        QMessageBox msgBox;

        msgBox.setText(tr("The configuration file ") + destFilename +
                tr(" already exists."));
        msgBox.setInformativeText(tr("Replace existing configuration file?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes)
        {
            QFile::remove(destFilename);
        }
    }

    // Note: Copy will not overwrite existing files
    bool success = QFile::copy(srcFilename, destFilename);
    if(!success)
    {
        debug1 << "Installing " << srcFilename.toStdString() << " to " << destFilename.toStdString() << " was not successful" << endl;
    }
}

// ****************************************************************************
// Method: QvisSetupHostProfilesAndConfigWindow::performSetup()
//
// Purpose:
//   Perform the actual setup (i.e., copy selected host profiles and
//   configuration into a user's .visit directory).
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//   Brad Whitlock, Fri Nov 11 09:52:13 PST 2011
//   Prevent copying of "closed" profiles when we did not ask for them.
//
//   Brad Whitlock, Thu Sep  6 11:24:41 PDT 2012
//   Adapt to hosts being in separate directories. Also we're no longer
//   installing all configs to the system config directory so we need to
//   get the configs from the resources directory.
//
//   Mark C. Miller, Mon Sep 17 08:46:24 PDT 2012
//   Fixed leak from using GetDefaultConfigFile directly as arg
//   in installConfigFile.
//
//   Kathleen Biagas, Thu Nov  7 11:51:53 PST 2019
//   Info now stored in QListWidgets and QStringlists.
//
//   Kevin Griffin, Thu Nov  7 17:43:56 PST 2019
//   In rare cases when the user moves or deletes their .visit directory
//   all the directories in the path need to be recreated. The call to mkdir
//   in the previous GetAndMakeUserVisItHostsDirectory called didn't do that
//   and there were no checks for successful creation of the hosts directory.
//   The call to get the hosts directory was split from making the directory
//   so mkpath could be used to create all the parent directories if needed.
//   Successful creation is now checked and if it fails the appropriate debug
//   log message is created.
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::performSetup()
{
    QString hostsInstallDirectory =
        QString::fromStdString(GetVisItHostsDirectory());

    // mkpath will create all parent directories necessary to create the directory
    // this is needed in rare cases where the .visit directory is deleted or moved and the
    // user wants to install the hosts profiles.
    QDir dir;
    bool success = dir.mkpath(hostsInstallDirectory);
    if(success)
    {
        for (int i =0; i < networkList->count(); ++i)
        {
            if (networkList->item(i)->checkState() == Qt::Checked)
            {
                QString srcDir(GetVisItResourcesFile(VISIT_RESOURCES_HOSTS,
                                  networkShortNames.at(i).toStdString()).c_str());
                QDir srcHostProfileDir(srcDir, "host*.xml");
                QStringList files = srcHostProfileDir.entryList();
                for (int i = 0; i < files.size(); ++ i)
                {
                    const QString &thisProfile = files.at(i);
                    installConfigFile(srcDir + "/" + thisProfile,
                                      hostsInstallDirectory + "/" + thisProfile);
                }
            }
        }

        // the 0'th radio button is for 'None', so start loop at 1.
        for (int i =1; i < defaultConfigList->count(); ++i)
        {
            QRadioButton *rb = qobject_cast<QRadioButton *>(defaultConfigList->
                               itemWidget(defaultConfigList->item(i)));
            if (rb && rb->isChecked())
            {
                const char *configFilename[] = {
                    "config", "guiconfig", "visitrc", 0 };
                for (int j = 0; configFilename[j] != 0; ++j)
                {
                    std::string srcCfgName =
                        defaultConfigShortNames.at(i).toStdString() +
                        "/" + std::string(configFilename[j]);
                    QString srcCfgPath(GetVisItResourcesFile(VISIT_RESOURCES_HOSTS, srcCfgName).c_str());
                    if (QFile::exists(srcCfgPath))
                    {
                        char *srcCfgFile = GetDefaultConfigFile(configFilename[j]);
                        installConfigFile(srcCfgPath, srcCfgFile);
                        delete [] srcCfgFile;
                    }
                }
            }
        }

        QMessageBox msgBox;
        msgBox.setText(tr("Host profiles and configuration files have been installed"
                   " and will be available after VisIt is restarted."));
        msgBox.exec();

        close();
    }
    else 
    {
        debug1 << "Hosts directory (" << hostsInstallDirectory.toStdString() << ") was not successfully created." << endl;
        
        QMessageBox msgBox;
        msgBox.setText(tr("Error: Host profiles and configuration files have not been installed. See debug logs for more information."));
        msgBox.exec();
        
        close();
    }
}
