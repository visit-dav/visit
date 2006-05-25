#ifndef QVIS_VISIT_UPDATE_H
#define QVIS_VISIT_UPDATE_H
#include <GUIBase.h>
#include <qobject.h>
#include <qstringlist.h>
#include <QvisFtp.h>

class QProcess;
class QUrlInfo;

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
// ****************************************************************************

class QvisVisItUpdate : public QObject, GUIBase
{
    Q_OBJECT
public:
    QvisVisItUpdate(QObject *parent = 0, const char *name = 0);
    virtual ~QvisVisItUpdate();

public slots:
    void startUpdate();
signals:
    void updateNotAllowed();
    void installationComplete(const QString &);
private slots:
    void initiateStage();
    void initiateDownload();
    void ftp_commandStarted();
    void ftp_commandFinished();
    void ftp_done(bool);
    void ftp_stateChanged(int);
    void ftp_listInfo(const QUrlInfo &);
    void ftp_reportDownloadProgress(int,int);

    void readInstallerStdout();
    void readInstallerStderr();
    void emitInstallationComplete();
private:
    void    provideLogin();
    void    nextStage();
    QString latestDirectory() const;
    QString localTempDirectory() const;
    QString getInstallationDir() const;
    void    getRequiredFiles();
    void    installVisIt();
    void    cleanUp();

    QvisFtp     *ftp;
    QProcess    *installProcess;
    int          stage;
    QString      distName;
    QString      configName;
    QString      bankName;
    QString      latestVersion;
    QStringList  files;
    QStringList  downloads;
};

#endif
