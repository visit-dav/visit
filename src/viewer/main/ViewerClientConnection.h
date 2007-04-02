/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef VIEWER_CLIENT_CONNECTION_H
#define VIEWER_CLIENT_CONNECTION_H
#include <qobject.h>
#include <SimpleObserver.h>
#include <vectortypes.h>

class QSocketNotifier;
class AttributeSubject;
class ParentProcess;
class RemoteProcess;
class Xfer;
class ViewerState;

// ****************************************************************************
// Class: ViewerClientConnection
//
// Purpose:
//   This class encapsulates the stuff needed to launch and talk to a viewer
//   client.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 4 10:41:51 PDT 2005
//
// Modifications:
//   Brad Whitlock, Fri Jul 8 10:33:23 PDT 2005
//   I changed the ConnectCallback argument into a function pointer argument
//   instead of using the typedef because that was causing a mysterious
//   compilation error in MSVC6.0.
//
// ****************************************************************************

class ViewerClientConnection : public QObject, public SimpleObserver
{
    Q_OBJECT
public:
    ViewerClientConnection(const ViewerState *, QObject *parent, const char *name=0);
    ViewerClientConnection(ParentProcess *, QSocketNotifier *, const ViewerState *,
                           QObject *parent, const char *name=0);
    virtual ~ViewerClientConnection();

    void LaunchClient(const std::string &program,
                      const stringVector &args,
                      void (*)(const std::string &, const stringVector &, void *),
                      void *,
                      bool (*)(void *, int),
                      void *);

    void SetupSpecialOpcodeHandler(void (*cb)(int,void*), void *data);

    void BroadcastToClient(AttributeSubject *);

    virtual void Update(Subject *);

    static const int FreelyExchangedState;
signals:
    void DisconnectClient(ViewerClientConnection *);
    void InputFromClient(ViewerClientConnection *, AttributeSubject *);
private slots:
    void ReadFromClientAndProcess(int);
private:
    Xfer              *xfer;
    QSocketNotifier   *notifier;
    RemoteProcess     *remoteProcess;
    ParentProcess     *parentProcess;
    ViewerState       *viewerState;
    bool               emitSignalsOnUpdate;
    bool               ownsNotifier;
};

#endif

