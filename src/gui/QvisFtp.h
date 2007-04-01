#ifndef QVIS_FTP_H
#define QVIS_FTP_H
#include <qftp.h>

#if QT_VERSION < 0x030100
class QUrlOperator;
class QIODevice;
#endif

// ****************************************************************************
// Class: QvisFtp
//
// Purpose:
//   This is a facade class that allows us to use QFtp from Qt 3.1 and later
//   but still have FTP with the same class interface in earlier versions
//   of Qt that did FTP a different way.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 2 10:39:12 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisFtp : public QFtp
{
    Q_OBJECT
public:
    typedef enum {Unconnected, HostLookup, Connecting, Connected,
                  LoggedIn, Closing } QvisFtpState;

    QvisFtp(QObject *parent, const char *name = 0);
    virtual ~QvisFtp();

    void ConnectToHost(const QString &url, int port);
    void Login(const QString &username = QString::null,
               const QString &password = QString::null);
    void List(const QString &path);

    QvisFtpState State();
    void Close();
    bool Get(const QString &remoteFile, QIODevice *localFile = 0);

    QString ErrorString();
    void DeleteCurrentDevice();
signals:
    void CommandStarted();
    void CommandFinished();
    void Done(bool);
    void ListInfo(const QUrlInfo &);
    void DataTransferProgress(int,int);
    void StateChanged(int);
protected slots:
    // These slots are only used for older versions of Qt. (pre 3.3.1)
    void emit_commandStarted(QNetworkOperation *);
    void emit_commandFinished(QNetworkOperation *);
    void emit_ListInfo(const QValueList<QUrlInfo> &, QNetworkOperation *);
    void readData(const QByteArray &, QNetworkOperation *);
    void connectionStateChanged(int,const QString &);
    void tryList();

private:
#if QT_VERSION < 0x030100
    // These variables are only used for older versions of Qt. (pre 3.3.1)
    QUrlOperator *ftp;
    QIODevice    *iodevice;
    QTimer       *timer;
    QvisFtpState  connectionState;
    bool          okayToEmitListInfo;
#endif
};

#endif
