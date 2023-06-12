// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              WebSocketConnection.h                        //
// ************************************************************************* //

#ifndef WEBSOCKET_CONNECTION_H
#define WEBSOCKET_CONNECTION_H
#include <viewer_exports.h>

#include <SocketConnection.h>
#include <deque>

#include <QString>
#include <QTcpSocket>
#include <QElapsedTimer>
class QWsSocket : public QAbstractSocket
{
    using QAbstractSocket::close;
    Q_OBJECT

public:
    enum EOpcode
    {
        OpContinue = 0x0,
        OpText = 0x1,
        OpBinary = 0x2,
        OpReserved1 = 0x3,
        OpReserved2 = 0x4,
        OpReserved3 = 0x5,
        OpReserved4 = 0x6,
        OpReserved5 = 0x7,
        OpClose = 0x8,
        OpPing = 0x9,
        OpPong = 0xA,
        OpReserved6 = 0xB,
        OpReserved7 = 0xC,
        OpReserved8 = 0xD,
        OpReserved9 = 0xE,
        OpReserved10 = 0xF
    };

public:
    // ctor
    QWsSocket(QTcpSocket * socket = 0, QObject * parent = 0, quint8 protVersion = 13);
    // dtor
    virtual ~QWsSocket();

    // Public methods
    qint64 write ( const QString & string, int maxFrameBytes = 0 ); // write data as text
    qint64 write ( const QByteArray & byteArray, int maxFrameBytes = 0 ); // write data as binary

    QTcpSocket* internalSocket() { return tcpSocket; }
public slots:
    virtual void close( QString reason );
    void ping();

signals:
    void frameReceived(QString frame);
    void frameReceived(QByteArray frame);
    void pong(quint64 elapsedTime);
    void framesWritten();

protected:
    qint64 writeFrames ( QList<QByteArray> framesList );
    qint64 writeFrame ( const QByteArray & byteArray );
    void dataReceivedV0();

protected slots:
    void dataReceived();
    void dataReceivedAll();

private slots:
    // private func
    void tcpSocketAboutToClose();
    void tcpSocketDisconnected();

private:
    // private vars
    QTcpSocket * tcpSocket;
    QByteArray currentFrame;
    QElapsedTimer pingTimer;
    quint8 protocolVersion;

public:
    // Static functions
//    static QByteArray generateMaskingKey();
    static QByteArray mask( QByteArray data, QByteArray maskingKey );
    static QList<QByteArray> composeFrames( QByteArray byteArray, bool asBinary = false, int maxFrameBytes = 0 );
    static QByteArray composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey = QByteArray() );
//    static QString composeOpeningHandShake( QString ressourceName, QString host, QString origin, QString extensions, QString key );

    // static vars
private:
    static int maxBytesPerFrame;
    static QString serializeInt( quint32 number, quint8 nbBytes  = 4);
    static QString computeAcceptV2(QString key);
    static QString computeAcceptV1( QString key1, QString key2, QString key3 );
public:
    static bool initializeWebSocket(const QString& request, QString& response);
};

// ****************************************************************************
// Class: WebSocketConnection
//
// Purpose:
//   This is a buffered connection that can write itself onto a
//   socket.
//
// Notes:      
//
// Programmer: Hari Krishnan
// Creation:   October 13, 2012
//
// Modifications:
//
// ****************************************************************************

class AttributeSubject;
class MapNode;
class JSONNode;
class VIEWER_API WebSocketConnection : public QObject, public AttributeSubjectSocketConnection
{
    Q_OBJECT
public:
    //WebSocketConnection(DESCRIPTOR descriptor_);
    WebSocketConnection(QTcpSocket* socket,const QString& response);
    virtual ~WebSocketConnection();

    virtual int  Fill();
    virtual void Flush();
    virtual void FlushAttr(AttributeSubject*);
    virtual long Size();
    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
    virtual void Append(const unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
    virtual long ReadHeader(unsigned char *buf, long len);
    virtual long WriteHeader(const unsigned char *buf, long len);
    virtual bool NeedsRead(bool blocking = false) const;
    virtual int  GetDescriptor() const;
private slots:
    void ReadFrame( QByteArray& array );
    void ReadFrame( const QString &str );
    void closeConnection();
private:
    QWsSocket                 *socket;
    QString messageRead;
    std::vector<QString> messages;

signals:
    void frameRead(int);
};

#endif
