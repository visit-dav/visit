// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_SETUP_HOST_PROFILES_AND_CONFIG_WINDOW_H
#define QVIS_SETUP_HOST_PROFILES_AND_CONFIG_WINDOW_H


#include <QvisDelayedWindow.h>
#include <QString>
#include <QStringList>

class QListWidget;

class GUI_API QvisSetupHostProfilesAndConfigWindow : public QvisDelayedWindow
{
    Q_OBJECT

public:
    QvisSetupHostProfilesAndConfigWindow(const QString&);
    virtual ~QvisSetupHostProfilesAndConfigWindow();
    virtual void CreateWindowContents();

private:

    QStringList networkShortNames;
    QListWidget *networkList;

    QStringList defaultConfigShortNames;
    QListWidget *defaultConfigList;

    void readNetworkList();
    void readDefaultConfigList();
    void installConfigFile(const QString&, const QString&);

private slots:
    void performSetup();
};

#endif
