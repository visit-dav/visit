// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_CLIENT_CONNECTION_H
#define VIEWER_CLIENT_CONNECTION_H
#include <ViewerBaseUI.h>
#include <SimpleObserver.h>
#include <vectortypes.h>
#include <ViewerClientAttributes.h>

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
//   Brad Whitlock, Mon Feb 12 17:56:51 PST 2007
//   Changed base class.
//
//   Brad Whitlock, Fri May  9 14:42:00 PDT 2008
//   Added name since Qt 4 does not have it.
//
//   Brad Whitlock, Thu Feb 24 23:49:34 PST 2011
//   I added a slot that we can use to trickle state objects out to the client
//   that we launch.
//
//   Kevin Griffin, Wed Jun 17 15:08:15 PDT 2020
//   Changed the return type of LaunchClient from void to bool to determine
//   if the client was successfully launched.
//
// ****************************************************************************

class ViewerClientConnection : public ViewerBaseUI, public SimpleObserver
{
    Q_OBJECT
public:
    ViewerClientConnection(const ViewerState *, QObject *parent, 
                           const QString &name, const bool _allState = false);
    ViewerClientConnection(ParentProcess *, QSocketNotifier *, const ViewerState *,
                           QObject *parent, const QString &name, const bool _allState = false);
    virtual ~ViewerClientConnection();

    bool LaunchClient(const std::string &program,
                      const stringVector &args,
                      void (*)(const std::string &, const stringVector &, void *),
                      void *,
                      bool (*)(void *, int),
                      void *);

    void SetupSpecialOpcodeHandler(void (*cb)(int,void*), void *data);

    void BroadcastToClient(AttributeSubject *);

    virtual void Update(Subject *);

    const QString &Name() const;

    void SetViewerClientAttributes(const ViewerClientAttributes& atts) { clientAtts.CopyAttributes(&atts); }
    ViewerClientAttributes& GetViewerClientAttributes() { return clientAtts; }
    static const int FreelyExchangedState;
signals:
    void DisconnectClient(ViewerClientConnection *);
    void InputFromClient(ViewerClientConnection *, AttributeSubject *);
private slots:
    void ForceDisconnectClient();
    void ReadFromClientAndProcess(int);
    void sendInitialState();
private:
    QString            name;
    Xfer              *xfer;
    QSocketNotifier   *notifier;
    RemoteProcess     *remoteProcess;
    ParentProcess     *parentProcess;
    ViewerState       *viewerState;
    bool               emitSignalsOnUpdate;
    bool               ownsNotifier;
    bool               allState; //whether to send all the state during initial connection
    int                initialStateStage;
    ViewerClientAttributes clientAtts;
};

#endif

