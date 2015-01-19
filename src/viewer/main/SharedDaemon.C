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

#include <SharedDaemon.h>

#include <ViewerClientConnection.h>
#include <ViewerSubject.h>
#include <ViewerState.h>
#include <QMap>
#include <QTcpSocket>
#include <QTcpServer>
#include <QTimer>
#include <QEventLoop>
#include <QMessageBox>
#include <QHostInfo>
#include <QNetworkInterface>
#include <JSONNode.h>
#include <SocketConnection.h>
#include <RemoteProcess.h>
#include <WebSocketConnection.h>
#include <CouldNotConnectException.h>

#include <iostream>
#include <string>

#include <ViewerRPC.h>

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
/// global variables to handle static callbacks..
QMap<QString,QString> hostMap;

SharedDaemon::SharedDaemon(ViewerSubject *lsubject, const int &lport,
    const std::string &lpassword): QTcpServer()
{
    subject = lsubject;
    listen_port = lport;
    password = lpassword.c_str();
    matched_input = "";

    connect(this,SIGNAL(newConnection()),this,SLOT(handleConnection()));

    QList<QHostAddress> entries = QNetworkInterface::allAddresses();
    foreach(const QHostAddress& entry, entries)
        hostMap.insert(entry.toString(), QHostInfo::fromName(entry.toString()).hostName());
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
//   Kathleen Biagas, Mon Dec 3 12:01:15 PST 2012
//   Use operator[] instead of 'at' to support older MSVC compiler.
//
// ****************************************************************************
bool
SharedDaemon::ParseInput(const QString& input, JSONNode& output)
{
    if(input.startsWith("{"))
    {
        JSONNode node;
        node.Parse(input.toStdString());
        //std::cout << node.ToString() << std::endl;
        /// also check to make sure password is coorect..
        if(node.GetType() != JSONNode::JSONOBJECT ||
           !node.HasKey("password") ||
            node.GetJsonObject()["password"].GetString() != password.toStdString())
            return false;

        output = node;
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
    std::cout << "user: "
              << socket->peerAddress().toString().toStdString()
              << " is attempting to connect" << std::endl;

    QAbstractSocket* finalSocket = NULL;
    ConnectionType typeOfConnection = TcpConnection;

    QByteArray result = socket->readAll();
    QString input(result);

    /// initial connection must pass password, but can optionally pass
    /// whether the client canRender and what the threshold value should be..
    JSONNode output;

    /// check if this is a WebSocketConnection
    QString response = "";
    if(input.startsWith("{") && ParseInput(input,output))
    {
        finalSocket = socket;
        typeOfConnection = TcpConnection;
    } /// check if this is a WebSocketConnection..
    else if(QWsSocket::initializeWebSocket(result,response))
    {
        /// this is a websocket connection, respond and get frame..
        socket->write(response.toLatin1());
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

        if( !ParseInput(matched_input,output) )
        {
            //std::cout << "passwords do not match: "
            //          << matched_password.toStdString()
            //          << " " << password << std::endl;

            wssocket->close("passwords do not match or operation timed out");

            if(socket->state() != QAbstractSocket::UnconnectedState)
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

    ViewerClientAttributes& clientAtts = newClient->GetViewerClientAttributes();
    JSONNode::JSONObject jo = output.GetJsonObject();

    clientAtts.SetExternalClient(true);

    if(jo.count("name") == 0 || jo["name"].GetString().size() == 0)
        clientAtts.SetTitle(socket->peerAddress().toString().toStdString());
    else
        clientAtts.SetTitle(jo["name"].GetString());

    if(jo.count("windowIds") > 0 && jo["windowIds"].GetType() == JSONNode::JSONARRAY)
    {
        const JSONNode::JSONArray& array = jo["windowIds"].GetArray();

        for(size_t i = 0; i < array.size(); ++i)
        {
            const JSONNode& node = array[i];

            if(node.GetType() != JSONNode::JSONINTEGER)
                continue;

            std::cout << clientAtts.GetTitle() <<  " requesting window: " << node.GetInt() << " " << std::endl;
            clientAtts.GetWindowIds().push_back(node.GetInt());
        }
    }

    if(jo.count("geometry") > 0)
    {
        std::string geometry = jo["geometry"].GetString();

        /// split into width & height...
        size_t index = geometry.find("x");
        if(index != std::string::npos && index != 0 && index != geometry.size()-1)
        {
            int geometryWidth = atoi(geometry.substr(0,index).c_str());
            int geometryHeight = atoi(geometry.substr(index+1).c_str());

            clientAtts.SetImageWidth(geometryWidth);
            clientAtts.SetImageHeight(geometryHeight);
            //std::cout << "geometry: " << clientAtts.clientWidth << " " << clientAtts.clientHeight << std::endl;
        }
    }

    /// advanced rendering can be true or false (image only), or string none,image,data
    if(jo.count("canRender") == 0) {
        clientAtts.SetRenderingType(ViewerClientAttributes::None);
        clientAtts.GetRenderingTypes().push_back(ViewerClientAttributes::None);
    }
    else
    {
        const JSONNode& node = jo["canRender"];
        QString type = node.GetString().c_str();
        type = type.toLower();

        /// TODO: remove the boolean check and make all current clients comply..
        if(node.GetType() == JSONNode::JSONBOOL) {
            clientAtts.SetRenderingType( node.GetBool() ? ViewerClientAttributes::Image :
                                                          ViewerClientAttributes::None);
            clientAtts.GetRenderingTypes().push_back(node.GetBool() ? ViewerClientAttributes::Image :
                                                                      ViewerClientAttributes::None);
        }
        else if(node.GetType() == JSONNode::JSONSTRING)
        {
            if(type == "image") {
                clientAtts.SetRenderingType(ViewerClientAttributes::Image);
                clientAtts.GetRenderingTypes().push_back((int)ViewerClientAttributes::Image);
            }
            else if(type == "data") {
                clientAtts.SetRenderingType(ViewerClientAttributes::Data);
                clientAtts.GetRenderingTypes().push_back((int)ViewerClientAttributes::Data);
            }
            else {
                clientAtts.SetRenderingType(ViewerClientAttributes::None);
                clientAtts.GetRenderingTypes().push_back((int)ViewerClientAttributes::None);
            }
        }
        else
        {
            clientAtts.SetRenderingType(ViewerClientAttributes::None);
            clientAtts.GetRenderingTypes().push_back((int)ViewerClientAttributes::None);
        }
    }
    stringVector args;

    /// assign whether connection is of type WebSocket or TCPConnection
    /// Register Type & Register Callback
    RemoteProcess::SetCustomConnectionCallback(createCustomConnection,&typeOfConnection);

    TRY
    {
        void* data[3];
        data[0] = &typeOfConnection;
        data[1] = (void*)finalSocket;
        data[2] = (void*)subject->GetViewerState();

        newClient->LaunchClient(program,args,AddNewClient,data,0,0);

        /// Now that client has launched RemoveCallback..
        subject->AddNewViewerClientConnection(newClient);
        std::cout << "user: " 
                  << socket->peerAddress().toString().toStdString()
                  << " successfully connected" << std::endl;
    }
    CATCHALL
    {
        std::cout << "user: " 
                  << socket->peerAddress().toString().toStdString()
                  << " failed to connected" << std::endl;
        delete newClient;
    }
    ENDTRY

    RemoteProcess::SetCustomConnectionCallback(0,0); /// reset connection..
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
    ViewerState* viewerState = static_cast<ViewerState*>(data[2]);

    JSONNode node;

    QString hostname = typeOfConnection == TcpConnection ? socket->localAddress().toString():
                        dynamic_cast<QWsSocket*>(socket)->internalSocket()->localAddress().toString();

    if(hostMap.contains(hostname)) hostname = hostMap[hostname];

    node["host"] = hostname.toStdString(); //host
    node["port"]  = args[7]; //port
    node["version"] = args[2]; //version
    node["securityKey"] = args[9]; //key
    node["numStates"] = viewerState->GetNumStateObjects(); //number of states

    JSONNode::JSONArray rpc_array = JSONNode::JSONArray();

    for(size_t i = 0; i < ViewerRPC::MaxRPC; ++i) {
        rpc_array.push_back(ViewerRPC::ViewerRPCType_ToString((ViewerRPC::ViewerRPCType)i));
    }
    node["rpc_array"] = rpc_array;

    if(typeOfConnection == TcpConnection)
    {
        QTcpSocket *tsocket = dynamic_cast<QTcpSocket*>(socket);

        std::string message = node.ToString();
        tsocket->write(message.c_str(),message.length());

        if(tsocket->state() != QAbstractSocket::UnconnectedState)
            tsocket->waitForBytesWritten();
        tsocket->disconnectFromHost();
        if(tsocket->state() != QAbstractSocket::UnconnectedState)
            tsocket->waitForDisconnected();
        //HKTODO: Do not delete connection (test fix for ORNL machines)
        //tsocket->deleteLater();
    }
    else
    {
        QWsSocket *wsocket = dynamic_cast<QWsSocket*>(socket);

        wsocket->write(QString(node.ToString().c_str()));
        wsocket->flush();
        if(wsocket->internalSocket()->state() != QAbstractSocket::UnconnectedState)
            wsocket->internalSocket()->waitForBytesWritten();

        wsocket->close("");
        wsocket->internalSocket()->disconnectFromHost();
        if(wsocket->internalSocket()->state() != QAbstractSocket::UnconnectedState)
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
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    if (setsockopt (listenSocketNum, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        printf("setsockopt failed\n");

    if (setsockopt (listenSocketNum, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
                sizeof(timeout)) < 0)
        printf("setsockopt failed\n");

    //do
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
    //while (desc == -1);

    if(desc == -1)
    {
        std::cerr << "Client failed to connect in time" << std::endl;
        EXCEPTION0(CouldNotConnectException);
    }

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
            if(tcpSocket->state() != QAbstractSocket::UnconnectedState)
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
