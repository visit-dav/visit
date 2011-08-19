/*****************************************************************************
*
* Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC
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

#include <QvisMacAppBundleSetupWindow.h>

#include <iostream>
#include <fstream>

#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>

#include <InstallationFunctions.h>

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::QvisMacAppBundleSetupWindow
//
// Purpose: 
//   Constructor for the QvisMacAppBundleSetupWindow class.
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

QvisMacAppBundleSetupWindow::QvisMacAppBundleSetupWindow(const QString &captionString) :
    QvisDelayedWindow(captionString)
{
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::~QvisMacAppBundleSetupWindow
//
// Purpose: 
//   Destructor for the QvisMacAppBundleSetupWindow class.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//
// ****************************************************************************

QvisMacAppBundleSetupWindow::~QvisMacAppBundleSetupWindow()
{
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::CreateWindowContents
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
QvisMacAppBundleSetupWindow::CreateWindowContents()
{
    readNetworkList();
    readDefaultConfigList();

    QLabel *label = new QLabel(tr("To finish the VisIt install on this Mac select any "
                "computing centers whose resources you are using to automatically "
                "configure host profiles for their machines."));
    label->setWordWrap(true);
    topLayout->addWidget(label);
    QGroupBox *networkGroup = new QGroupBox(tr("Select computing centers used"));
    topLayout->addWidget(networkGroup);
    QVBoxLayout *networkGroupLayout = new QVBoxLayout;
    networkGroup->setLayout(networkGroupLayout);

    for(std::list<NetworkInfo>::iterator it = networkList.begin(); it != networkList.end(); ++it)
    {
        it->checkBox = new QCheckBox(it->longName.c_str());
        networkGroupLayout->addWidget(it->checkBox);
    }

    QGroupBox *configGroup = new QGroupBox(tr("Select default configuration"));
    topLayout->addWidget(configGroup);
    QVBoxLayout *configGroupLayout = new QVBoxLayout;
    configGroup->setLayout(configGroupLayout);

    QRadioButton *radioButton = new QRadioButton(tr("None (use VisIt's standard defaults)"));
    radioButton->setChecked(true);
    configGroupLayout->addWidget(radioButton);
    for(std::list<DefaultConfigInfo>::iterator it = defaultConfigList.begin(); it != defaultConfigList.end(); ++it)
    {
        it->radioButton = new QRadioButton(it->longName.c_str());
        configGroupLayout->addWidget(it->radioButton);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    topLayout->addLayout(buttonLayout);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch(1);
    QObject::connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    QPushButton *installButton = new QPushButton(tr("Install"));
    buttonLayout->addWidget(installButton);
    QObject::connect(installButton, SIGNAL(clicked()), this, SLOT(performSetup()));
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::readNetworkList()
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
QvisMacAppBundleSetupWindow::readNetworkList()
{
    //std::cout << "Config file: " << GetSystemConfigFile("networks.dat") << std::endl;
    std::ifstream is(GetSystemConfigFile("networks.dat"));

    std::string line;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        int posL = line.find(':');
        if (posL >= line.size())
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }
        int posS= line.find(':', posL + 1);
        if (posS >= line.size())
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }

        NetworkInfo ni(line.substr(0, posL), line.substr(posL+1, posS-(posL+1)));
        int currPos = posS+1;
        int endPos;
        while ((endPos = line.find(' ', currPos)) != std::string::npos)
        {
            ni.configFilenameList.push_back(line.substr(currPos, endPos - currPos));
            currPos = endPos + 1;
        }
        ni.configFilenameList.push_back(line.substr(currPos));
        networkList.push_back(ni);
    }
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::readDefaultConfigList()
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
QvisMacAppBundleSetupWindow::readDefaultConfigList()
{
    std::ifstream is(GetSystemConfigFile("default_configs.dat"));

    std::string line;
    while(std::getline(is, line))
    {
        // Find two deliminiting ":"
        int posL = line.find(':');
        if (posL >= line.size())
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }
        int posS= line.find(':', posL + 1);
        if (posS >= line.size())
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }

        std::string longName = line.substr(0, posL);
        std::string shortName = line.substr(posL+1, posS-(posL+1));
        std::string configString = line.substr(posS+1);
        if (configString.size() != 3)
        {
            std::cerr << "Igonoring invalid line: " << line << std::endl;
            continue;
        }
        bool config = (configString[0] == '1');
        bool guiconfig = (configString[1] == '1');
        bool rc = (configString[2] == '1');

        defaultConfigList.push_back(DefaultConfigInfo(longName, shortName, config, guiconfig, rc));
    }
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::installConfigFile()
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
QvisMacAppBundleSetupWindow::installConfigFile(const std::string& srcFilename,
        const std::string& destFilename)
{
    bool install = true;
    if (access(destFilename.c_str(), F_OK) == 0)
    {
        QMessageBox msgBox;

        QString msg = tr("The configuration file ");
        msg += QString(destFilename.c_str());
        msg += tr(" already exists.");
        msgBox.setText(msg);
        msgBox.setInformativeText(tr("Replace existing configuration file?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        int ret = msgBox.exec();
        install = (ret == QMessageBox::Yes);
    }

    if (install)
    {
        std::string cmd = "/bin/cp -f " + srcFilename + " " + destFilename;
        system(cmd.c_str()); 
    }
}

// ****************************************************************************
// Method: QvisMacAppBundleSetupWindow::performSetup()
//
// Purpose: 
//   Perform the actual setup (i.e., copy selected host profiles and
//   configuration into a user's .visit directory.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Aug 11 17:48:10 PDT 2011
//
// Modifications:
//
// ****************************************************************************

void
QvisMacAppBundleSetupWindow::performSetup()
{
    std::string hostsInstallDirectory = GetAndMakeUserVisItHostsDirectory();

    for (std::list<NetworkInfo>::iterator it = networkList.begin();
            it != networkList.end(); ++it)
    {
        if (it->checkBox->isChecked())
        {
            for (std::list<std::string>::iterator cFIt = it->configFilenameList.begin();
                    cFIt != it->configFilenameList.end(); ++ cFIt)
            {
                std::string srcName =
                    GetIsDevelopmentVersion() ? *cFIt : std::string("allhosts/") + *cFIt;
                std::string srcHostProfilePath = GetSystemConfigFile(srcName.c_str());
                installConfigFile(srcHostProfilePath, hostsInstallDirectory + "/" + *cFIt);
            }
        }
    }
    for (std::list<DefaultConfigInfo>::iterator it = defaultConfigList.begin();
            it != defaultConfigList.end(); ++it)
    {
        if (it->radioButton->isChecked())
        {
            if (it->config)
            {
                std::string cfgname =
                    std::string("visit-config-") + it->shortName + std::string(".ini");
                installConfigFile(
                        GetSystemConfigFile(cfgname.c_str()),
                        GetDefaultConfigFile("config")
                        );
            }
            if (it->guiconfig)
            {
                std::string cfgname =
                    std::string("visit-guiconfig-") + it->shortName + std::string(".ini");
                installConfigFile(
                        GetSystemConfigFile(cfgname.c_str()),
                        GetDefaultConfigFile("guiconfig")
                        );
            }
            if (it->rc)
            {
                std::string cfgname =
                    std::string("visit-visitrc-") + it->shortName + std::string(".ini");
                installConfigFile(
                        GetSystemConfigFile(cfgname.c_str()),
                        GetDefaultConfigFile("visitrc")
                        );
            }

        }
    }

    QMessageBox msgBox;
    msgBox.setText(tr("Host profiles and configuration files have been installed and will be available "
                " when VisIt is restarted."));
    msgBox.exec();

    close();
}
