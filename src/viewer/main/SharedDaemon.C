/*****************************************************************************
*
* Copyright (c) 2000 - 2012, Lawrence Livermore National Security, LLC
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

#include <SharedDaemon.h>

#include <ViewerClientConnection.h>
#include <ViewerSubject.h>

#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>

#include <SocketConnection.h>
#include <RemoteProcess.h>

#include <iostream>
#include <string>

#include <WebSocketConnection.h>

#include <QEventLoop>

#include <JSONNode.h>

// ****************************************************************************
// Method: SharedDaemon::SharedDaemon
//
// Purpose:
//  Constructor, initiates class with listener port and password
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

SharedDaemon::SharedDaemon(ViewerSubject *lsubject, const int &lport,
    const std::string &lpassword): QTcpServer()
{
    subject = lsubject;
    listen_port = lport;
    password = lpassword;

    connect(this,SIGNAL(newConnection()),this,SLOT(handleConnection()));
}

// ****************************************************************************
// Method: SharedDaemon::SharedDaemon
//
// Purpose:
//  Constructor, initiates class with listener port and password
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

SharedDaemon::~SharedDaemon()
{
    close();
}

// ****************************************************************************
// Method: SharedDaemon::SharedDaemon
//
// Purpose:
//  Constructor, initiates class with listener port and password
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

void SharedDaemon::init()
{
    listen(QHostAddress::Any,listen_port);

    if(isListening())
        std::cerr << "Starting to listen on port: " << listen_port << std::endl;
    else
        std::cerr << "WARNING: Failed to start listening server on port: " << listen_port << std::endl;
}

// ****************************************************************************
// Method: SharedDaemon::SharedDaemon
//
// Purpose:
//  Constructor, initiates class with listener port and password
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

void SharedDaemon::incomingConnection( int sd )
{
    //TODO: replace with threaded version so each socket can take care of incoming connections
    // on their own time..
    //For now get one thing to work..
    QTcpServer::incomingConnection(sd);
}

// ****************************************************************************
// Method: SharedDaemon::SharedDaemon
//
// Purpose:
//  Constructor, initiates class with listener port and password
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

/// TODO: remove the external dependency all together..
QString matched_password = "";
void SharedDaemon::getPasswordMessage(QString message)
{
    //std::cout << "password " << message.toStdString() << std::endl;
    matched_password = message;
}

// ****************************************************************************
// Method: SharedDaemon::handleConnection
//
// Purpose:
//  Handle the incomming connection..
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

void SharedDaemon::handleConnection()
{
    QTcpSocket *socket = nextPendingConnection();

    if ( !socket ) 
        return;
    //the connecting socket should have sent password..
    //the client should be sending a password..
    socket->waitForReadyRead();

    if (!socket->bytesAvailable())
    {
        //std::cout << "no bytes available to read" << std::endl;
        socket->close();
        return;
    }

    QByteArray result = socket->readAll();

    QString lpasswd(result);

    //std::cout << "password is: " << lpasswd.toStdString() << std::endl;

    QAbstractSocket* finalSocket = socket;
    ConnectionType typeOfConnection = TcpConnection;

    if(password != lpasswd.toStdString())
    {
        /// check if this is a WebSocketConnection
        QString response;
        if(QWsSocket::initializeWebSocket(result,response))
        {
            /// this is a websocket connection, respond and get frame..
            socket->write(response.toAscii());
            socket->flush();

            /// do a password check..

            QEventLoop loop;

            matched_password = "";

            QWsSocket* wssocket = new QWsSocket(socket);

            connect(wssocket,SIGNAL(frameReceived(QString)),
                    this,SLOT(getPasswordMessage(QString)));

            connect(wssocket,SIGNAL(frameReceived(QString)),
                    &loop,SLOT(quit()));

            /// wait for password to be sent ..
            /// std::cout << "waiting for password from websocket" << std::endl;
            loop.exec();

            disconnect(wssocket,SIGNAL(frameReceived(QString)),
                    this,SLOT(getPasswordMessage(QString)));

            disconnect(wssocket,SIGNAL(frameReceived(QString)),
                    &loop,SLOT(quit()));


            //if( password == matched_password.toStdString())
            //{
            //    std::cout << "passwords match: "
            //              << matched_password.toStdString()
            //            << " " << password << std::endl;
            //}

            if( password != matched_password.toStdString())
            {

                //std::cout << "passwords do not match: "
                //          << matched_password.toStdString()
                //          << " " << password << std::endl;

                disconnect(wssocket,SIGNAL(frameReceived(QString)),
                        this,SLOT(getPasswordMessage(QString)));
                wssocket->close("passwords do not match or operation timed out");
                socket->waitForDisconnected();

                wssocket->deleteLater();
                return;
            }

            finalSocket = wssocket;
            typeOfConnection = WSocketConnection;

        }
        else
        {
            //std::cout << "passwords do not match/or unknown connection detected" << std::endl;

            //send rejection notice..
            std::string errorString = "Passwords do not match";
            socket->write(errorString.c_str(),errorString.length());
            socket->disconnectFromHost();
            socket->waitForDisconnected();
            return;
        }
    }

    //passwords match enable RemoteProcess and get port remote Process is listening to.
    //send host,port,security_key and whatever else so that remote machine can successfully reverse connect
    std::string clientName = "newclient1";

    ViewerClientConnection *newClient = new
            ViewerClientConnection(subject->GetViewerState(),
                                   this,
                                   clientName.c_str(),
                                   true);

    newClient->SetAdvancedRendering(true);

    std::string program = "remoteApp";
    stringVector args;

    /// assign whether connection is of type WebSocket or TCPConnection
    /// Register Type & Register Callback
    RemoteProcess::SetCustomConnectionCallback(createCustomConnection,&typeOfConnection);

    void* data[2];
    data[0] = &typeOfConnection;
    data[1] = (void*)finalSocket;
    newClient->LaunchClient(program,args,AddNewClient,data,0,0);

    RemoteProcess::SetCustomConnectionCallback(0,0); /// reset connection..

    /// Now that client has launched RemoveCallback..
    subject->AddNewViewerClientConnection(newClient);
}

// ****************************************************************************
// Method: SharedDaemon::AddNewClient
//
// Purpose:
//  Callback function to tell client where and how VisIt will accept information
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

void SharedDaemon::AddNewClient(const std::string &host, const stringVector &args, void *cbdata)
{
    /// Send appropriate message for TCP or WebConnection
    void** data = (void**)cbdata;
    ConnectionType typeOfConnection = *((ConnectionType*)(data[0]));
    std::string message = "";

    //std::cout << host << " "
    //          << (typeOfConnection ? "WebSocketConnection" : "TcpSocketConnection")
    //          << std::endl;

    if(args.size() > 1)
    {
        for(size_t i = 0; i < args.size() - 1; ++i)
            message += args[i] + std::string(",");
        message += args.back();
    }
    message += std::string(",-dv");

    QAbstractSocket* socket = static_cast<QAbstractSocket*>(data[1]);

    if(typeOfConnection == TcpConnection)
    {
        QTcpSocket *tsocket = dynamic_cast<QTcpSocket*>(socket);

        //host,port,key
        //std::cout << "writing message: " << message << std::endl;
        tsocket->write(message.c_str(),message.length());
        tsocket->waitForBytesWritten();
        tsocket->disconnectFromHost();
        tsocket->waitForDisconnected();
        tsocket->deleteLater();
    }
    else
    {
        QWsSocket *wsocket = dynamic_cast<QWsSocket*>(socket);

        //host,port,key
        //std::cout << "writing WebSocket message: "
        //          << message << " " << std::endl;

        JSONNode node;

        node["host"] = args[5]; //host
        node["port"]  = args[7]; //port
        node["version"] = args[2]; //version
        node["securityKey"] = args[9]; //key

        wsocket->write(QString(node.ToString().c_str()));
        wsocket->flush();
        wsocket->internalSocket()->waitForBytesWritten();

        wsocket->close();
        wsocket->internalSocket()->disconnectFromHost();
        wsocket->internalSocket()->waitForDisconnected();
        wsocket->deleteLater();
    }
}

// ****************************************************************************
// Method: SingleThreadedAcceptSocket
//
// Purpose:
//   Does Accept call, may need to be special for future cases (i.e., adding a timeout).
//   For now this is a copy of the ParentProcess SingleThreadedAcceptSocket function
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

int
SingleThreadedAcceptSocket(int listenSocketNum)
{
    struct sockaddr_in       sin;
    int desc = -1;

    // Wait for the socket to become available on the other side.
    do
    {
#ifdef HAVE_SOCKLEN_T
        socklen_t len;
#else
        int len;
#endif
        len = sizeof(struct sockaddr);
        desc = accept(listenSocketNum, (struct sockaddr *)&sin, &len);
#if defined(_WIN32)
        if(desc == INVALID_SOCKET)
            LogWindowsSocketError(mName, "accept");
#endif
    }
    while (desc == -1);

    return desc;
}

// ****************************************************************************
// Method: SharedDaemon::createCustomConnection
//
// Purpose:
//  Overrides Remote Process connection to establish custom ways to connecting
//  VisIt
//
// Arguments:
//
// Returns:
//
// Note:
//
// Programmer: Hari Krishnan
// Creation:   Oct 13, 2012
//
// Modifications:
//
// ****************************************************************************

Connection* SharedDaemon::createCustomConnection(int listenSocketNum, void *data)
{
    SharedDaemon::ConnectionType typeOfConnection = *((SharedDaemon::ConnectionType*)(data));
    if(typeOfConnection == SharedDaemon::WSocketConnection )
    {
        int descriptor = SingleThreadedAcceptSocket(listenSocketNum);

        QTcpSocket * tcpSocket = new QTcpSocket();
        tcpSocket->setSocketDescriptor( descriptor, QAbstractSocket::ConnectedState );
        tcpSocket->waitForReadyRead();

        QString request = QString(tcpSocket->readAll());

        if(request.size() == 0) /// firefox works differently, and sends 0 size messages
        {
            tcpSocket->close();
            tcpSocket->disconnectFromHost();
            tcpSocket->waitForDisconnected();
            tcpSocket->deleteLater();

            descriptor = SingleThreadedAcceptSocket(listenSocketNum);
            tcpSocket = new QTcpSocket();
            tcpSocket->setSocketDescriptor( descriptor, QAbstractSocket::ConnectedState );
            tcpSocket->waitForReadyRead();
            request = QString(tcpSocket->readAll());
        }

        return new WebSocketConnection(tcpSocket,request);
    }
    else
    {
        int descriptor = SingleThreadedAcceptSocket(listenSocketNum);
        return new SocketConnection(descriptor);
    }
}
