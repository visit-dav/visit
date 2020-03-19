// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SHARED_DAEMON_H
#define SHARED_DAEMON_H

#include <viewer_exports.h>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <string>
#include <vectortypes.h>
#include <JSONNode.h>
class Connection;
class ViewerSubject;
// ****************************************************************************
// Class: SharedDaemon
//
// Purpose:
//        Starts a listener thread to accept thirdparty clients or new connections
//
// Notes:
//
// Programmer: Hari Krishnan
// Creation:   October 13, 2012
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API SharedDaemon : public QTcpServer
{
    Q_OBJECT
public:
    SharedDaemon(ViewerSubject* subject,
                 const int& listen_port,
                 const std::string& password);
    virtual ~SharedDaemon();

    virtual void incomingConnection (int sd);

public slots:
    void init();
protected slots:
    void handleConnection();
    void getPasswordMessage(QString message);
private:
   static void AddNewClient(const std::string &,
                            const stringVector &,
                            void *);

   bool ParseInput(const QString &input, JSONNode& output);

   //HKTODO: I need to figure out a good way to have Connections figure out the
   //the type of communication system..
   //I can do this in RemoteProcess/ParentProcess but that will force more changes
   //than I comfortable with..
   //the fewer files that are modified the better..
   enum ConnectionType { TcpConnection, WSocketConnection };
   static Connection* createCustomConnection(int descriptor,void* data);
   int listen_port;
   QString password;
   QString matched_input;
   ViewerSubject* subject;
};

#endif
