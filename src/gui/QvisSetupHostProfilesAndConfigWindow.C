/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <QvisSetupHostProfilesAndConfigWindow.h>

#include <iostream>
#include <fstream>

#include <QCheckBox>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <InstallationFunctions.h>

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
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::CreateWindowContents()
{
    readNetworkList();
    readDefaultConfigList();

    QLabel *label = new QLabel(tr("To finish the VisIt install on this computer select "
                "any computing centers whose resources you are using to "
                "configure host profiles automatically for their machines."));
    label->setWordWrap(true);
    topLayout->addWidget(label);
    QGroupBox *networkGroup = new QGroupBox(
            tr("Select computing centers used"));
    topLayout->addWidget(networkGroup);
    QVBoxLayout *networkGroupLayout = new QVBoxLayout;
    networkGroup->setLayout(networkGroupLayout);

    for(std::list<NetworkInfo>::iterator it = networkList.begin();
            it != networkList.end(); ++it)
    {
        it->checkBox = new QCheckBox(it->longName);
        networkGroupLayout->addWidget(it->checkBox);
    }

    QGroupBox *configGroup = new QGroupBox(
            tr("Select default configuration"));
    topLayout->addWidget(configGroup);
    QVBoxLayout *configGroupLayout = new QVBoxLayout;
    configGroup->setLayout(configGroupLayout);

    QRadioButton *radioButton = new QRadioButton(
            tr("None (use VisIt's standard defaults)"));
    radioButton->setChecked(true);
    configGroupLayout->addWidget(radioButton);
    for(std::list<DefaultConfigInfo>::iterator it = defaultConfigList.begin();
            it != defaultConfigList.end(); ++it)
    {
        it->radioButton = new QRadioButton(it->longName);
        configGroupLayout->addWidget(it->radioButton);
    }

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
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::readNetworkList()
{
    std::ifstream is(GetSystemConfigFile("networks.dat"));

    std::string line;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        int splitPos = line.find(':');
        if (splitPos == std::string::npos)
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }

        networkList.push_back(
                NetworkInfo(line.substr(0, splitPos), line.substr(splitPos+1)));
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
//
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::readDefaultConfigList()
{
    std::ifstream is(GetSystemConfigFile("default_configs.dat"));

    std::string line;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        int splitPos = line.find(':');
        if (splitPos == std::string::npos)
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }

        defaultConfigList.push_back(DefaultConfigInfo(
                    line.substr(0, splitPos), line.substr(splitPos+1)));
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
    QFile::copy(srcFilename, destFilename);
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
// ****************************************************************************

void
QvisSetupHostProfilesAndConfigWindow::performSetup()
{
    QString hostsInstallDirectory =
        QString::fromStdString(GetAndMakeUserVisItHostsDirectory());

    for (std::list<NetworkInfo>::iterator it = networkList.begin();
            it != networkList.end(); ++it)
    {
        if (it->checkBox->isChecked())
        {
            QString srcHostProfileDirName = QString::fromStdString(
                    GetIsDevelopmentVersion() ?
                    GetSystemConfigFile("") :
                    GetSystemConfigFile("allhosts/"));
            QDir srcHostProfileDir(
                    srcHostProfileDirName,
                    QString("host_") + it->shortName + QString("_*.xml"));
            QStringList srcConfigFilenameList = srcHostProfileDir.entryList();
            bool closed = it->shortName.contains("closed");
            for (int i = 0; i < srcConfigFilenameList.size(); ++ i)
            {
                const QString &thisProfile = srcConfigFilenameList.at(i);
                if(!closed && thisProfile.contains("closed"))
                    continue;

                installConfigFile(
                        srcHostProfileDirName + "/" + thisProfile,
                        hostsInstallDirectory + "/" + thisProfile);
            }
        }
    }
    for (std::list<DefaultConfigInfo>::iterator it = defaultConfigList.begin();
            it != defaultConfigList.end(); ++it)
    {
        if (it->radioButton->isChecked())
        {
            const char *configFilename[] = {
                "config", "guiconfig", "visitrc", 0 };

            for (int i = 0; configFilename[i] != 0; ++i)
            {
                std::string srcCfgName =
                    std::string("visit-") +
                    std::string(configFilename[i]) +
                    std::string("-") +
                    it->shortName.toStdString() +
                    std::string(".ini");
                QString srcCfgPath = GetSystemConfigFile(srcCfgName.c_str());
                if (QFile::exists(srcCfgPath))
                {
                    installConfigFile(
                            srcCfgPath, GetDefaultConfigFile(configFilename[i]));
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
