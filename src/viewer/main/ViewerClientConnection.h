#ifndef VIEWER_CLIENT_CONNECTION_H
#define VIEWER_CLIENT_CONNECTION_H
#include <qobject.h>
#include <ConnectCallback.h>
#include <SimpleObserver.h>

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
                      ConnectCallback cb, void *cbData = 0,
                      bool (*connectProgressCB)(void *, int) = 0,
                      void *connectProgressCBData = 0);

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

