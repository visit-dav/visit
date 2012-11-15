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

#ifdef _WIN32
#include <win32commhelpers.h>
#endif

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
QString matched_input = "";
void SharedDaemon::getPasswordMessage(QString message)
{
    //std::cout << "password " << message.toStdString() << std::endl;
    matched_input = message;
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
bool
SharedDaemon::ParseInput(const QString& input, std::string& lpasswd, bool& canRender)
{
    if(input.startsWith("{"))
    {
        JSONNode node;
        node.Parse(input.toStdString());

        if(node.GetType() != JSONNode::JSONOBJECT ||
           !node.HasKey("password") ||
            node.GetObject()["password"].GetString() != password)
            return false;

        lpasswd = node.GetObject().at("password").GetString();

        if(node.HasKey("canRender") == true &&
           node.GetObject().at("canRender").GetType() == JSONNode::JSONBOOL)
            canRender = node.GetObject().at("canRender").GetBool();

        return true;
    }

    return false;
}

void SharedDaemon::handleConnection()
{
    QTcpSocket *socket = nextPendingConnection();

    if ( !socket ) return;

    //the connecting socket should have sent password..
    //the client should be sending a password..
    socket->waitForReadyRead();

    if (!socket->bytesAvailable())
    {
        //std::cout << "no bytes available to read" << std::endl;
        socket->close();
        return;
    }

    QAbstractSocket* finalSocket = NULL;
    ConnectionType typeOfConnection = TcpConnection;

    QByteArray result = socket->readAll();
    QString input(result);

    /// initial connection must pass password, but can optionally pass
    /// whether the client canRender and what the threshold value should be..
    std::string lpasswd = "";
    bool canRender = false;

    /// check if this is a WebSocketConnection
    QString response = "";

    if(input.startsWith("{") && ParseInput(input,lpasswd,canRender))
    {
        finalSocket = socket;
        typeOfConnection = TcpConnection;
    } /// check if this is a WebSocketConnection..
    else if(QWsSocket::initializeWebSocket(result,response))
    {
        /// this is a websocket connection, respond and get frame..
        socket->write(response.toAscii());
        socket->flush();

        QEventLoop loop;

        matched_input = "";

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

        //std::cout << matched_input.toStdString() << std::endl;

        if( !ParseInput(matched_input,lpasswd,canRender) )
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

    } /// not sure what connection this is, reject it..
    else
    {
        //send rejection notice..
        std::string errorString = "Unknown connection..";
        socket->write(errorString.c_str(),errorString.length());
        socket->disconnectFromHost();
        socket->waitForDisconnected();
        return;
    }

    //passwords match enable RemoteProcess and get port remote Process is listening to.
    //send host,port,security_key and whatever else so that remote machine can successfully reverse connect
    std::string program = "remoteApp";
    std::string clientName = "newclient1";

    ViewerClientConnection *newClient = new
            ViewerClientConnection(subject->GetViewerState(),
                                   this,
                                   clientName.c_str(),
                                   true);

    newClient->SetExternalClient(true);
    newClient->SetAdvancedRendering(canRender);
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
    QAbstractSocket* socket = static_cast<QAbstractSocket*>(data[1]);

    JSONNode node;

    node["host"] = args[5]; //host
    node["port"]  = args[7]; //port
    node["version"] = args[2]; //version
    node["securityKey"] = args[9]; //key

    if(typeOfConnection == TcpConnection)
    {
        QTcpSocket *tsocket = dynamic_cast<QTcpSocket*>(socket);

        std::string message = node.ToString();
        tsocket->write(message.c_str(),message.length());
        tsocket->waitForBytesWritten();
        tsocket->disconnectFromHost();
        tsocket->waitForDisconnected();
        tsocket->deleteLater();
    }
    else
    {
        QWsSocket *wsocket = dynamic_cast<QWsSocket*>(socket);

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
        const char *mName = "SharedDaemon, SingleThreadedAcceptSocket: ";
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
