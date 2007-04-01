#include <QvisFtp.h>
#if QT_VERSION < 0x030100
#include <qurloperator.h>
#include <qiodevice.h>
#include <qnetwork.h>
#include <qtimer.h>
#endif

#include <DebugStream.h>

const char *
GetNetworkOperationState(const QNetworkOperation *operation)
{
    const char *retval = "";

    if(operation != 0)
    {
        if(operation->state() == QNetworkProtocol::StDone)
            retval = "StDone";
        else if(operation->state() == QNetworkProtocol::StFailed)
            retval = "StFailed";
        else if(operation->state() == QNetworkProtocol::StInProgress)
            retval = "StInProgress";
        else if(operation->state() == QNetworkProtocol::StWaiting)
            retval = "StWaiting";
        else if(operation->state() == QNetworkProtocol::StStopped)
            retval = "StStopped";
    }

    return retval;
}

// ****************************************************************************
// Method: QvisFtp::QvisFtp
//
// Purpose: 
//   Constructor for the QvisFtp compatibility class.
//
// Arguments:
//   parent : The parent object.
//   name   : The name of the object.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:52:36 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisFtp::QvisFtp(QObject *parent, const char *name) : QFtp(
#if QT_VERSION >= 0x030100
    parent, name
#endif
    )
{
#if QT_VERSION >= 0x030100
    // Re-emit signals with a different name.
    connect(this, SIGNAL(commandStarted(int)),
            this, SIGNAL(CommandStarted()));
    connect(this, SIGNAL(commandFinished(int,bool)),
            this, SIGNAL(CommandFinished()));
    connect(this, SIGNAL(done(bool)),
            this, SIGNAL(Done(bool)) );
    connect(this, SIGNAL(stateChanged(int)),
            this, SIGNAL(StateChanged(int)));
    connect(this, SIGNAL(listInfo(const QUrlInfo &)),
            this, SIGNAL(ListInfo(const QUrlInfo &)));
    connect(this, SIGNAL(dataTransferProgress(int,int)),
            this, SIGNAL(DataTransferProgress(int,int)));
#else
    // Pre 3.1 Qt.
    ftp = 0;
    iodevice = 0;
    timer = 0;
    connectionState = Unconnected;
    okayToEmitListInfo = false;
#endif
}

// ****************************************************************************
// Method: QvisFtp::~QvisFtp
//
// Purpose: 
//   Destructor for the QvisFtp class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:53:13 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisFtp::~QvisFtp()
{
}

// ****************************************************************************
// Method: QvisFtp::ConnectToHost
//
// Purpose: 
//   Connects to an FTP site.
//
// Arguments:
//   url  : The URL of the FTP site.
//   port : The port to use when connecting to the site.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:53:38 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::ConnectToHost(const QString &url, int port)
{
#if QT_VERSION >= 0x030100
    connectToHost(url, port);
#else
    // Pre 3.1 implementation.
    qInitNetworkProtocols();

    // If the url does not begin with "ftp://" then add that.
    QString URL(url);
    if(url.find("ftp.") != -1 && url.left(5) != "ftp://")
        URL = QString("ftp://") + url;
    debug1 << "Pre 3.1 implementation for ConnectToHost: url="
           << URL.latin1() << ", port=" << port << endl;

    // Create a timer that we'll use to keep trying to list in case
    // it does not work.
    timer = new QTimer(this, "tryList timer");
    connect(timer, SIGNAL(timeout()),
            this, SLOT(tryList()));

    // Create the URL operator.
    ftp = new QUrlOperator(URL);
    ftp->setPort(port);
    ftp->setPath("/");
    connect(ftp, SIGNAL(start(QNetworkOperation *)),
            this, SLOT(emit_commandStarted(QNetworkOperation *)));
    connect(ftp, SIGNAL(finished(QNetworkOperation *)),
            this, SLOT(emit_commandFinished(QNetworkOperation *)));
    connect(ftp, SIGNAL(newChildren(const QValueList<QUrlInfo> &, QNetworkOperation *)),
            this, SLOT(emit_ListInfo(const QValueList<QUrlInfo> &, QNetworkOperation *)));

    connect(ftp, SIGNAL(data(const QByteArray &, QNetworkOperation *)),
            this, SLOT(readData(const QByteArray &, QNetworkOperation *)));
    connect(ftp, SIGNAL(dataTransferProgress(int,int)),
            this, SIGNAL(DataTransferProgress(int,int)));
    connect(ftp, SIGNAL(connectionStateChanged(int, const QString &)),
            this, SLOT(connectionStateChanged(int,const QString &)));

    // Emit command started, finished.
    emit CommandStarted();
    emit CommandFinished();
#endif
}

// ****************************************************************************
// Method: QvisFtp::Login
//
// Purpose: 
//   Logs the user into an FTP site.
//
// Arguments:
//   username : The username to use when logging in.
//   password : The password to use when logging in.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:54:31 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::Login(const QString &username, const QString &password)
{
#if QT_VERSION >= 0x030100
    login(username, password);
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for Login" << endl;
    ftp->setUser(username);
    ftp->setPassword(password);
    tryList();
#endif
}

// ****************************************************************************
// Method: QvisFtp::List
//
// Purpose: 
//   Lists the files at the specified path.
//
// Arguments:
//   path : The path for which we want the file list.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:55:22 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::List(const QString &path)
{
#if QT_VERSION >= 0x030100
    list(path);
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for List" << endl;

    // Set the path and list the children. Note that we set okayToEmitListInfo
    // to true so we emit the ListInfo signal when we get newChildren
    // signals from the URL operator.
    okayToEmitListInfo = true;
    ftp->setPath(path);
    tryList();
#endif
}

// ****************************************************************************
// Method: QvisFtp::State
//
// Purpose: 
//   Gets the state of the connection.
//
// Returns:    The state.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:56:06 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QvisFtp::QvisFtpState
QvisFtp::State()
{
#if QT_VERSION >= 0x030100
    return (QvisFtpState)((int)state());
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for State" << endl;
    return connectionState;
#endif
}

// ****************************************************************************
// Method: QvisFtp::Close
//
// Purpose: 
//   Closes the connection to the FTP site.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:56:34 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::Close()
{
#if QT_VERSION >= 0x030100
    close();
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for Close" << endl;
#endif
}

// ****************************************************************************
// Method: QvisFtp::Get
//
// Purpose: 
//   Gets a file and copies it to the local disk.
//
// Arguments:
//   remoteFile : The name of the remote file to fetch.
//   localFile  : The open file object to which the file will be saved.
//
// Returns:    True if successful; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:56:55 PST 2005
//
// Modifications:
//   
// ****************************************************************************

bool
QvisFtp::Get(const QString &remoteFile, QIODevice *localFile)
{
#if QT_VERSION >= 0x030100
    return get(remoteFile, localFile);
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for Get" << endl;
    if(iodevice != 0)
        delete iodevice;
    iodevice = localFile;
    return ftp->get(remoteFile) != 0;
#endif
}

// ****************************************************************************
// Method: QvisFtp::ErrorString
//
// Purpose: 
//   Returns the error string.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:58:12 PST 2005
//
// Modifications:
//   
// ****************************************************************************

QString
QvisFtp::ErrorString()
{
#if QT_VERSION >= 0x030100
    return errorString();
#else
    // Pre 3.1 implementation.
    debug1 << "Pre 3.1 implementation for ErrorString" << endl;
    return "no error";
#endif
}

// ****************************************************************************
// Method: QvisFtp::DeleteCurrentDevice
//
// Purpose: 
//   Deletes the current io device.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 2 11:16:57 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::DeleteCurrentDevice()
{
#if QT_VERSION >= 0x030100
    delete currentDevice();
#else
    if(iodevice != 0)
    {
        delete iodevice;
        iodevice = 0;
    }
#endif
}

//
// Qt slot functions.
//
// Note:
//    All of these methods are used to implement the behavior of QFtp that
//    is available in Qt 3.2+ in earlier versions of Qt.
//

// ****************************************************************************
// Method: QvisFtp::emit_commandStarted
//
// Purpose: 
//   This is a Qt slot function that is called at the start of a URL operation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:59:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::emit_commandStarted(QNetworkOperation *op)
{
#if QT_VERSION < 0x030100
    debug1 << "emit_commandStarted: state="
           << GetNetworkOperationState(op) << endl;
    emit CommandStarted();
#endif
}

// ****************************************************************************
// Method: QvisFtp::emit_commandStarted
//
// Purpose: 
//   This is a Qt slot function that is called at the end of a URL operation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 17:59:24 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::emit_commandFinished(QNetworkOperation *operation)
{
#if QT_VERSION < 0x030100
    debug1 << "emit_commandFinished" << endl;

    // Stop the timer.
    if(operation->operation() == QNetworkProtocol::OpListChildren)
    {
        debug1 << "\tStopping the timer." << endl;
        timer->stop();
    }

    if(operation != 0)
    {
        if(operation->state() == QNetworkProtocol::StDone)
        {
            debug1 << "\tstate = StDone, emitting CommandFinished."<<endl;
            emit CommandFinished();
        }
        else if(operation->state() == QNetworkProtocol::StFailed)
        {
            debug1 << "\tstate = StFailed, emitting CommandFinished."<<endl;
            emit CommandFinished();
        }
        else if(operation->state() == QNetworkProtocol::StInProgress)
        {
            debug1 << "\tstate = StInProgress"<<endl;
        }
        else if(operation->state() == QNetworkProtocol::StWaiting)
        {
            debug1 << "\tstate = StWaiting"<<endl;
        }
        else if(operation->state() == QNetworkProtocol::StStopped)
        {
            debug1 << "\tstate = StStopped"<<endl;
        }
    }
#endif
}

// ****************************************************************************
// Method: QvisFtp::tryList
//
// Purpose: 
//   This method tells the UrlOperator to try and list the current directory.
//
// Note:       This method starts a timer that will cause it to be called
//             again in ten seconds in the event that the listChildren
//             method did not actually cause anything to happen.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 2 10:26:31 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::tryList()
{
#if QT_VERSION < 0x030100
    if(timer->isActive())
    {
        debug1 << "The timer is active so we're calling tryList to make "
               << "the UrlOperator try and list the children again." << endl;
        timer->stop();
        ftp->stop();
    }
    timer->start(10000);
    const QNetworkOperation *op = ftp->listChildren();
    if(op == 0)
        debug1 << "Could not create listchildren operation!" << endl;
    else
    {
        debug1 << "Created listchildren operation! state="
               << GetNetworkOperationState(op) << endl;
    }
#endif
}

// ****************************************************************************
// Method: QvisFtp::emit_ListInfo
//
// Purpose: 
//   This method is called when we list a new directory.
//
// Arguments:
//   items : The items in the new directory.
//
// Note:       The items list is often just 1 element long.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 18:00:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::emit_ListInfo(const QValueList<QUrlInfo> &items, QNetworkOperation *op)
{
#if QT_VERSION < 0x030100
    debug1 << "emit_ListInfo: state=" << GetNetworkOperationState(op) << endl;

    for(int i = 0; i < items.count(); ++i)
    {
        debug1 << "\t" << items[i].name().latin1() << endl;
        if(okayToEmitListInfo)
            emit ListInfo(items[i]);
    }
#endif
}

// ****************************************************************************
// Method: QvisFtp::readData
//
// Purpose: 
//   This method is called when reading data from the remote file into the
//   local file.
//
// Arguments:
//   arr : The byte array to add to the file.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 18:01:21 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::readData(const QByteArray &arr, QNetworkOperation *)
{
#if QT_VERSION < 0x030100
    if(iodevice != 0)
        iodevice->writeBlock(arr);
#endif
}

// ****************************************************************************
// Method: QvisFtp::connectionStateChanged
//
// Purpose: 
//   This method updates the connection state variable.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 1 18:02:19 PST 2005
//
// Modifications:
//   
// ****************************************************************************

void
QvisFtp::connectionStateChanged(int val,const QString &data)
{
#if QT_VERSION < 0x030100
    if(val == 0)
        connectionState = Connecting;
    else if(val == 1)
        connectionState = Connected;
#endif
}

