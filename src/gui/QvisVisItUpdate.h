// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_VISIT_UPDATE_H
#define QVIS_VISIT_UPDATE_H
#include <GUIBase.h>
#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QProcess>

class QUrlInfo;
class QvisDownloader;

// ****************************************************************************
// Class: QvisVisItUpdate
//
// Purpose:
//   This class knows how to connect to the VisIt Web site and download new
//   versions of VisIt.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 9 17:34:59 PST 2005
//
// Modifications:
//   Brad Whitlock, Wed Mar 2 11:08:25 PDT 2005
//   I made it use QvisFtp instead of qftp.
//
//   Brad Whitlock, Thu May 25 12:12:56 PDT 2006
//   I added some new members.
//
//   Hank Childs, Tue Sep 26 14:10:20 PDT 2006
//   Added keyword `public' for inheritance of GUIBase.  This addresses
//   compiler warning on xlC.
//
//   Brad Whitlock, Thu Oct  2 10:45:14 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class QvisVisItUpdate : public QObject, public GUIBase
{
    Q_OBJECT
public:
    QvisVisItUpdate(QObject *parent = 0);
    virtual ~QvisVisItUpdate();

public slots:
    void startUpdate();
signals:
    void updateNotAllowed();
    void installationComplete(const QString &);
private slots:
    void initiateStage();
    void determineReleaseHTML(bool);
    void determineLatestDownload(bool error);
    void downloadDone(bool error);
    void getRequiredFiles();
    void reportDownloadProgress(qint64 done, qint64 total);
    void readInstallerStdout();
    void readInstallerStderr();
    void emitInstallationComplete(int);
private:
    void    nextStage();
    QString localTempDirectory() const;
    QString getInstallationDir() const;
    void    installVisIt();
    void    cleanUp();
    QString remoteToLocalName(const QString &remote) const;

    QvisDownloader *downloader;
    QProcess       *installProcess;
    int             stage;
    QString         distName;
    QString         configName;
    QString         bankName;
    QString         latestVersion;
    QString         releaseHTML;
    QStringList     files;
    QStringList     downloads;
    QByteArray      bytes;
};

#endif
