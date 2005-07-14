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

