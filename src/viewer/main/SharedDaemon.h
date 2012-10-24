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

#ifndef SHARED_DAEMON_H
#define SHARED_DAEMON_H

#include <viewer_exports.h>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <string>
#include <vectortypes.h>

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
class VIEWER_API SharedDaemon : public QTcpServer
{
    Q_OBJECT
public:
    SharedDaemon(ViewerSubject* subject,
                 const int& listen_port,
                 const std::string& password);
    ~SharedDaemon();

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

   //HKTODO: I need to figure out a good way to have Connections figure out the
   //the type of communication system..
   //I can do this in RemoteProcess/ParentProcess but that will force more changes
   //than I comfortable with..
   //the fewer files that are modified the better..
   enum ConnectionType { TcpConnection, WSocketConnection };
   static Connection* createCustomConnection(int descriptor,void* data);
   int listen_port;
   std::string password;
   ViewerSubject* subject;
};

#endif
