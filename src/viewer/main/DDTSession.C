/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#include <DDTSession.h>
#include <DDTManager.h>
#include <ViewerMethods.h>
#include <ViewerPlotList.h>
#include <ViewerWindow.h>
#include <ViewerWindowManager.h>
#include <ViewerEngineManager.h>
#include <EngineKey.h>

#include <QLocalSocket>
#include <QTextStream>

// ****************************************************************************
// Method: DDTSession::DDTSession
//
// Purpose:
//    Constructor for the DDTSession class. Creates a socket connection
//    to DDT.
//
// Arguments:
//    server: The file path to the named socket used to extablish
//            connections to DDT
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTSession::DDTSession(const QString server) :
    mErrorText(QString::null), mServer(server), mSocket(NULL)
{
    mSocket = new QLocalSocket(this);

    connect(mSocket,SIGNAL(stateChanged(QLocalSocket::LocalSocketState)),
            this,SIGNAL(statusChanged()));
    connect(mSocket,SIGNAL(error(QLocalSocket::LocalSocketError)),
            this, SLOT(errorHandler(QLocalSocket::LocalSocketError)));
    connect(mSocket,SIGNAL(connected()),
            this, SLOT(completeConnection()));

    mSocket->connectToServer(mServer,QIODevice::ReadWrite);
}

// ****************************************************************************
// Method: DDTSession::~DDTSession
//
// Purpose:
//    Destructor for the DDTSession class. Disconnects from DDT
//    if not already disconnected.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

DDTSession::~DDTSession()
{
    //if (mSocket->state() != QLocalSocket::UnconnectedState)
    //    Error("(DDTSession) Socket deleted before being disconnected");

    if (mSocket)
        delete mSocket;
}

// ****************************************************************************
// Method: DDTSession::connected()
//
// Purpose:
//    Determines the socket to DDT is currently in its 'connected' state
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

bool
DDTSession::connected()
{
    return mSocket->state() == QLocalSocket::ConnectedState;
}

// ****************************************************************************
// Method: DDTSession::disconnect
//
// Purpose:
//    Disconnects the socket to DDT
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTSession::disconnect()
{
    mSocket->disconnectFromServer();
}

// ****************************************************************************
// Method: DDTSession::disconnected
//
// Purpose:
//    Determines if the socket connection to DDT has been disconnected /
//    never was connected
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

bool
DDTSession::disconnected()
{
    return mSocket->state() == QLocalSocket::UnconnectedState;
}

// ****************************************************************************
// Method: DDTSession::completeConnection
//
// Purpose:
//    Completes a connection to DDT by identifying this socket as coming
//    from VisIt.
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void DDTSession::completeConnection()
{
    connect(this,SIGNAL(statusChanged()),
            DDTManager::getInstance(),SIGNAL(statusChanged()));

    connect(mSocket,SIGNAL(readyRead()),this,SLOT(readDataFromDDT()));
    mSocket->readAll();  // Clear socket's buffer so signal gets send readReady next time input received

    mSocket->write("[VisIt]\n");
    mSocket->flush();

    emit statusChanged();
}

// ****************************************************************************
// Method: DDTSession::statusString
//
// Purpose:
//     Provides a human readable string summarising the state of
//     the connection to DDT
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

QString
DDTSession::statusString()
{
    if (!mErrorText.isEmpty())
        return mErrorText;

    switch(mSocket->state())
    {
    case QLocalSocket::UnconnectedState:
        return tr("Not connected");
    case QLocalSocket::ConnectingState:
        return tr("Connecting...");
    case QLocalSocket::ConnectedState:
        return tr("Connected");
    case QLocalSocket::ClosingState:
        return tr("Disconnecting...");
    default:
        return tr("Not connected");
    }
}

// ****************************************************************************
// Method: DDTSession::setFocusOnDomain
//
// Purpose:
//     Sends a command over the socket instructing DDT to focus on a
//     specific domain
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTSession::setFocusOnDomain(const int domain)
{
    if (domain == -1)
        Error(tr("No domain information, may not be a parallel simulation. DDT's focus will not be changed."));

    if (mSocket->state() == QLocalSocket::ConnectedState)
    {
        QString str = QString("[VisIt] focus domain %0\n").arg(domain);
        mSocket->write(str.toLatin1().constData());
        mSocket->flush();
    }
    else
    {
        Error(tr("Cannot focus DDT on domain %0 as VisIt is not currently connected to DDT.").arg(domain));
    }
}

// ****************************************************************************
// Method: DDTSession::errorHandler
//
// Purpose:
//     Handles any error from the socket, creating a human-readable
//     error message
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTSession::errorHandler(QLocalSocket::LocalSocketError err)
{
    switch(err)
    {
    case QLocalSocket::ConnectionRefusedError:
        mErrorText = tr("Connection refused"); break;
    case QLocalSocket::PeerClosedError:
        mErrorText = tr("Connection closed by DDT"); break;
    case QLocalSocket::ServerNotFoundError:
        mErrorText = tr("DDT not found"); break;
    case QLocalSocket::SocketAccessError:
        mErrorText = tr("Insufficient permissions"); break;
    case QLocalSocket::SocketResourceError:
        mErrorText = tr("Too many sockets"); break;
    case QLocalSocket::SocketTimeoutError:
        mErrorText = tr("Timed out"); break;
    case QLocalSocket::ConnectionError:
        mErrorText = tr("Connection error"); break;
    default:
        mErrorText = mSocket->errorString(); break;
    }

    emit statusChanged();
}

// ****************************************************************************
// Method: DDTSession::readDataFromDDT
//
// Purpose:
//     Handles instructions received from DDT over the socket
//
// Programmer: Jonathan Byrd
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

void
DDTSession::readDataFromDDT()
{
    QTextStream in(mSocket);

    QString message = in.readLine();
    while(!message.isNull())       // For each line of text received from socket
    {
        if (message=="raise")
        {
            // There could be multiple viewer windows. We don't know which one
            // corresponds to the DDT instance that sent this request, so
            // raise all of them that come from DDT-named database
            const int numWindows = ViewerWindowManager::Instance()->GetNumWindows();
            for (int i=0; i<numWindows; ++i)
            {
                ViewerWindow* win = ViewerWindowManager::Instance()->GetWindow(i);
                if (DDTManager::isDatabaseDDTSim(win->GetPlotList()->GetDatabaseName()))
                    win->ActivateWindow();
            }
        }
        else if (message=="release")
        {
            // There could be multiple viewer windows. We don't know which one
            // corresponds to the DDT instance that sent this request, so send
            // command to all of them that come from DDT-named database
            const int numWindows = ViewerWindowManager::Instance()->GetNumWindows();
            for (int i=0; i<numWindows; ++i)
            {
                ViewerWindow* win = ViewerWindowManager::Instance()->GetWindow(i);
                if (DDTManager::isDatabaseDDTSim(win->GetPlotList()->GetDatabaseName()))
                {
                    const EngineKey &key = win->GetPlotList()->GetEngineKey();
                    if (key.IsSimulation())
                        ViewerEngineManager::Instance()->SendSimulationCommand(
                                key, "DDT", "");
                }
            }
        }
        else
            Error(QString("Unrecognised message from DDT: %0").arg(message));

        message = in.readLine();
    }
}
