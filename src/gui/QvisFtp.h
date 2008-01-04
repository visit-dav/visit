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
