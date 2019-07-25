// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SETUP_HOST_PROFILES_AND_CONFIG_WINDOW_H
#define QVIS_SETUP_HOST_PROFILES_AND_CONFIG_WINDOW_H

#include <string>
#include <list>

#include <QvisDelayedWindow.h>
#include <QString>

class QCheckBox;
class QRadioButton;

class GUI_API QvisSetupHostProfilesAndConfigWindow : public QvisDelayedWindow
{
    Q_OBJECT

public:
    QvisSetupHostProfilesAndConfigWindow(const QString&);
    virtual ~QvisSetupHostProfilesAndConfigWindow();
    virtual void CreateWindowContents();

private:
    struct NetworkInfo
    {
        QString longName;
        QString shortName;
        QCheckBox *checkBox;

        NetworkInfo(const std::string& lN, const std::string& sN)
            : longName(QString::fromStdString(lN)),
              shortName(QString::fromStdString(sN)),
              checkBox(0) {}
    };
    std::list<NetworkInfo> networkList;
    struct DefaultConfigInfo
    {
        QString longName;
        QString shortName;
        QRadioButton *radioButton;

        DefaultConfigInfo(const std::string& lN, const std::string& sN)
            : longName(QString::fromStdString(lN)),
              shortName(QString::fromStdString(sN)),
              radioButton(0) {}
    };
    std::list<DefaultConfigInfo> defaultConfigList;

    void readNetworkList();
    void readDefaultConfigList();
    void installConfigFile(const QString&, const QString&);

private slots:
    void performSetup();
};

#endif
