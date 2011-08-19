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

#ifndef QVIS_MAC_APPBUNDLE_SETUP_WINDOW_H
#define QVIS_MAC_APPBUNDLE_SETUP_WINDOW_H

#include <string>
#include <list>

#include <QvisDelayedWindow.h>

class QCheckBox;
class QRadioButton;

class GUI_API QvisMacAppBundleSetupWindow : public QvisDelayedWindow
{
    Q_OBJECT

public:
    QvisMacAppBundleSetupWindow(const QString&);
    virtual ~QvisMacAppBundleSetupWindow();
    virtual void CreateWindowContents();

private:
    struct NetworkInfo
    {
        std::string longName;
        std::string shortName;
        std::list<std::string> configFilenameList;
        QCheckBox *checkBox;

        NetworkInfo(const std::string& lN, const std::string& sN)
            : longName(lN), shortName(sN), configFilenameList(), checkBox(0) {}
    };
    std::list<NetworkInfo> networkList;
    struct DefaultConfigInfo
    {
        std::string longName;
        std::string shortName;
        bool config;
        bool guiconfig;
        bool rc;
        QRadioButton *radioButton;

        DefaultConfigInfo(const std::string& lN, const std::string& sN, bool c, bool g, bool r)
            : longName(lN), shortName(sN), config(c), guiconfig(g), rc(r) {}
    };
    std::list<DefaultConfigInfo> defaultConfigList;

    void readNetworkList();
    void readDefaultConfigList();
    void installConfigFile(const std::string&, const std::string&);

private slots:
    void performSetup();
};

#endif
