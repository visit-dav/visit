// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisDownloader.h>

#include <QByteArray>
#include <QFile>
#include <QNetworkReply>

// ****************************************************************************
// Method: QvisDownloader::QvisDownloader
//
// Purpose: 
//   Constructor.
//
// Arguments:
//   parent : The object's parent.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:28:55 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Nov  1 13:02:42 PDT 2011
//   Rewrite.
//   
// ****************************************************************************

QvisDownloader::QvisDownloader(QObject *parent) : QObject(parent)
{
    file = NULL;
    bytes = NULL;

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply *)),
            this, SLOT(finished(QNetworkReply *)));
}

// ****************************************************************************
// Method: QvisDownloader::~QvisDownloader
//
// Purpose: 
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:29:19 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

QvisDownloader::~QvisDownloader()
{
}

// ****************************************************************************
// Method: QvisDownloader::get
//
// Purpose: 
//   Gets the named remote file and puts its contents in the passed byte array.
//
// Arguments:
//   remoteFile : The name of the remote file.
//   b          : A pointer to the byte array that will be used to store the
//                file's bytes.
//
// Returns:    True.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:35:28 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Nov  1 13:02:42 PDT 2011
//   Rewrite.
//   
// ****************************************************************************

bool
QvisDownloader::get(const QUrl &url, QByteArray *b)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SIGNAL(downloadProgress(qint64,qint64)));

    bytes = b;
    file = NULL;

    return true;
}

// ****************************************************************************
// Method: QvisDownloader::get
//
// Purpose: 
//   Gets the named remote file and puts its contents in the passed byte array.
//
// Arguments:
//   remoteFile : The name of the remote file.
//   localFile  : The name of the local file to create.
//
// Returns:    True if the local file can be created; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:35:28 PDT 2008
//
// Modifications:
//   Brad Whitlock, Tue Nov  1 13:02:42 PDT 2011
//   Rewrite.
//
// ****************************************************************************


bool
QvisDownloader::get(const QUrl &url, const QString &localFile)
{
    bytes = NULL;
    file = new QFile(localFile);
    if (!file->open(QIODevice::WriteOnly))
    {
        delete file;
        file = NULL;
        return false;
    }

    QNetworkRequest request(url);
    QNetworkReply *reply = manager->get(request);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SIGNAL(downloadProgress(qint64,qint64)));

    return true;
}

//
// Slots
//

// ****************************************************************************
// Method: QvisDownloader::finished
//
// Purpose: 
//   This method gets called when our network request is finished.
//
// Arguments:
//   reply : The network reply.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov  1 13:02:57 PDT 2011
//
// Modifications:
//   
// ****************************************************************************

void
QvisDownloader::finished(QNetworkReply *reply)
{
    disconnect(reply, SIGNAL(downloadProgress(qint64,qint64)),
               this, SIGNAL(downloadProgress(qint64,qint64)));

    if(!reply->error())
    {
        if(bytes != NULL)
        {
             *bytes = reply->readAll();
             bytes = NULL;
        }
        if(file != NULL)
        {
            file->write(reply->readAll());
            file->close();
        }

        bytes = NULL;
        file = NULL;
    }

    emit done(reply->error());
}
