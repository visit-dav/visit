/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
