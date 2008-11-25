/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
#include <QvisDownloader.h>

#include <QByteArray>
#include <QFile>
#include <QHttp>
#include <QPushButton>
#include <QLayout>

#define VISIT_DOWNLOAD_URL "wci.llnl.gov"

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
//   
// ****************************************************************************

QvisDownloader::QvisDownloader(QObject *parent) : QObject(parent)
{
    file = 0;
    bytes = 0;

    http = new QHttp(this);
#ifdef DEBUG_VISIT_DOWNLOADER
    connect(http, SIGNAL(requestStarted(int)),
            this, SLOT(requestStarted(int)));
    connect(http, SIGNAL(requestFinished(int,bool)),
            this, SLOT(requestFinished(int,bool)));
    connect(http, SIGNAL(stateChanged(int)),
            this, SLOT(stateChanged(int)));
#endif
    connect(http, SIGNAL(dataReadProgress(int,int)),
            this, SIGNAL(downloadProgress(int,int)));
    connect(http, SIGNAL(readyRead(const QHttpResponseHeader &)),
            this, SLOT(readyRead(const QHttpResponseHeader &)));
    connect(http, SIGNAL(done(bool)),
            this, SLOT(httpdone(bool)));
    connect(http, SIGNAL(sslErrors(const QList<QSslError> &)),
            this, SLOT(sslErrors(const QList<QSslError> &)));
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
//   
// ****************************************************************************

bool
QvisDownloader::get(const QString &remoteFile, QByteArray *b)
{
    file = 0;
    bytes = b;

    http->setHost(VISIT_DOWNLOAD_URL, QHttp::ConnectionModeHttps);
    http->get(remoteFile);
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
//   
// ****************************************************************************


bool
QvisDownloader::get(const QString &remoteFile, const QString &localFile)
{
    bytes = 0;
    file = new QFile(localFile);
    if (!file->open(QIODevice::WriteOnly))
    {
        delete file;
        file = 0;
        return false;
    }

    http->setHost(VISIT_DOWNLOAD_URL, QHttp::ConnectionModeHttps);
    http->get(remoteFile, file);
    return true;
}

//
// Slots
//

// ****************************************************************************
// Method: QvisDownloader::readyRead
//
// Purpose: 
//   This Qt slot function is called when there is new data to be read from
//   the http object. We use this occasion to save off the bytes if we're
//   storing them in a byte array.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:40:37 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisDownloader::readyRead(const QHttpResponseHeader &responseHeader)
{
#ifdef DEBUG_VISIT_DOWNLOADER
    qDebug("readyRead()");
#endif
    switch (responseHeader.statusCode())
    {
    case 200:                   // Ok
    case 301:                   // Moved Permanently
    case 302:                   // Found
    case 303:                   // See Other
    case 307:                   // Temporary Redirect
        // these are not error conditions
        if(bytes != 0)
            bytes->append(http->readAll());
        break;
    default:
#ifdef DEBUG_VISIT_DOWNLOADER
        qDebug("Read aborted: %s", responseHeader.reasonPhrase().toStdString().c_str());
#endif
        http->abort();
        if(bytes != 0)
            bytes->clear();
    }
}

// ****************************************************************************
// Method: QvisDownloader::httpdone
//
// Purpose: 
//   This Qt slot function is called when the http object is done processing
//   the get request.
//
// Arguments:
//   error : True if an error occurred during download.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:41:46 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisDownloader::httpdone(bool error)
{
#ifdef DEBUG_VISIT_DOWNLOADER
    qDebug("done(%s)", error?"true":"false");
#endif

    if(file != 0)
    {
        file->close();
        file = 0;
    }
    bytes = 0;

    emit done(error);
}

// ****************************************************************************
// Method: QvisDownloader::sslErrors
//
// Purpose: 
//   This Qt slot function is called when there are SSL concerns during a
//   download. We just tell the http object to ignore those errors and
//   keep going.
//
// Arguments:
//   errors : The SSL errors that have occurred.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct  2 10:42:50 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisDownloader::sslErrors(const QList<QSslError> &errors)
{
#ifndef QT_NO_OPENSSL
#ifdef DEBUG_VISIT_DOWNLOADER
    qDebug("sslErrors()");
    QString errorString;
    foreach (const QSslError &error, errors)
    {
         if (!errorString.isEmpty())
             errorString += ", ";
         errorString += error.errorString();
    }

    qDebug("SSL Errors: %s", errorString.toStdString().c_str());
#endif
    http->ignoreSslErrors();
#endif
}

#ifdef DEBUG_VISIT_DOWNLOADER
// Keep for debugging.
void
QvisDownloader::requestStarted(int val)
{
    qDebug("requestStarted(%d)", val);
}

void
QvisDownloader::requestFinished(int id, bool error)
{
    qDebug("requestFinished(%d, %s)", id, error?"true":"false");
}

void
QvisDownloader::stateChanged(int state)
{
    switch(state)
    {
    case QHttp::Unconnected:
        qDebug("stateChanged(Unconnected)");
        break;
    case QHttp::HostLookup:
        qDebug("stateChanged(HostLookup)");
        break;
    case QHttp::Connecting:
        qDebug("stateChanged(Connecting)");
        break;
    case QHttp::Sending:
        qDebug("stateChanged(Sending)");
        break;
    case QHttp::Reading:
        qDebug("stateChanged(Reading)");
        break;
    case QHttp::Connected:
        qDebug("stateChanged(Connected)");
        break;
    case QHttp::Closing:
        qDebug("stateChanged(Closing)");
        break;
    }
}
#endif
