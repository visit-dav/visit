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

#include <WebSocketConnection.h>
#include <visit-config.h>

#include <AttributeSubject.h>
#include <AttributeSubjectSerialize.h>
#include <MapNode.h>
#include <cstdlib>

#include <iostream>

#include <QStringList>
#include <QEventLoop>

#include <JSONNode.h>

#include <QAbstractSocket>
#include <QTcpSocket>
#include <QTime>

#include <QRegExp>
#include <QStringList>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>

#if defined(_WIN32)
#include <winsock2.h>
#include <win32commhelpers.h>
#else
#include <strings.h>             // bzero by way of FD_ZERO
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#endif
#ifdef HAVE_SELECT
#include <SysCall.h>
#endif


void replace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

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


int QWsSocket::maxBytesPerFrame = 1400;

QWsSocket::QWsSocket( QTcpSocket * socket, QObject * parent, quint8 protVers ) :
    QAbstractSocket( QAbstractSocket::UnknownSocketType, parent )
{
    tcpSocket = socket;
    protocolVersion = protVers;

    //setSocketState( QAbstractSocket::UnconnectedState );
    setSocketState( socket->state() );

    connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()) );
    //connect( tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceivedAll()) );
    connect( tcpSocket, SIGNAL(disconnected()), this, SLOT(tcpSocketDisconnected()) );
    connect( tcpSocket, SIGNAL(aboutToClose()), this, SLOT(tcpSocketAboutToClose()) );
}

QWsSocket::~QWsSocket()
{
}

void QWsSocket::dataReceived()
{
    if ( protocolVersion == 0 )
    {
        dataReceivedV0();
        return;
    }

    QByteArray BA; // ReadBuffer
    quint8 byte; // currentByteBuffer

    // FIN, RSV1-3, Opcode
    BA = tcpSocket->read(1);
    byte = BA[0];
    quint8 FIN = (byte >> 7);
    EOpcode Opcode = (EOpcode)(byte & 0x0F);

    // Mask, PayloadLength
    BA = tcpSocket->read(1);
    byte = BA[0];
    quint8 Mask = (byte >> 7);
    quint64 PayloadLength = (byte & 0x7F);
    // Extended PayloadLength
    if ( PayloadLength == 126 )
    {
        BA = tcpSocket->read(2);
        PayloadLength = ((quint8)BA[0] << 8) + (quint8)BA[1];
    }
    else if ( PayloadLength == 127 )
    {
        BA = tcpSocket->read(8);
        PayloadLength = 0;
        quint64 plbyte;
        for ( int i=0 ; i<8 ; i++ )
        {
            plbyte = (quint8)BA[i];
            PayloadLength += ( plbyte << (7-i)*8 );
        }
    }

    // MaskingKey
    QByteArray MaskingKey;
    if ( Mask )
    {
        MaskingKey = tcpSocket->read(4);
    }

    // Extension // UNSUPPORTED FOR NOW

    // ApplicationData
    if ( PayloadLength )
    {
        QByteArray ApplicationData = tcpSocket->read( PayloadLength );
        if ( Mask )
            ApplicationData = QWsSocket::mask( ApplicationData, MaskingKey );
        currentFrame.append( ApplicationData );
    }

    if ( FIN )
    {
        if ( Opcode == OpBinary )
        {
            emit frameReceived( currentFrame );
        }
        else if ( Opcode == OpText )
        {
            QString byteString;
            byteString.reserve(currentFrame.size());
            for (int i=0 ; i<currentFrame.size() ; i++)
                byteString[i] = currentFrame[i];
            emit frameReceived( byteString );
        }
        else if ( Opcode == OpPing )
        {
            QByteArray pongRequest = QWsSocket::composeHeader( true, OpPong, 0 );
            write( pongRequest );
        }
        else if ( Opcode == OpPong )
        {
            quint64 ms = pingTimer.elapsed();
            emit pong(ms);
        }
        else if ( Opcode == OpClose )
        {
            tcpSocket->close();
        }
        currentFrame.clear();
    }

    if ( tcpSocket->bytesAvailable() )
        dataReceived();
}

void QWsSocket::dataReceivedAll()
{

    QByteArray data = tcpSocket->readAll();

    QByteArray BA; // ReadBuffer
    quint8 byte; // currentByteBuffer

    int offset = 0;

    while(offset < data.size())
    {
        // FIN, RSV1-3, Opcode
        BA = data.mid(offset,1); offset += 1;
        byte = BA[0];
        quint8 FIN = (byte >> 7);
//        quint8 RSV1 = ((byte & 0x7F) >> 6);
//        quint8 RSV2 = ((byte & 0x3F) >> 5);
//        quint8 RSV3 = ((byte & 0x1F) >> 4);
        EOpcode Opcode = (EOpcode)(byte & 0x0F);

        // Mask, PayloadLength
        BA = data.mid(offset,1); offset += 1;
        byte = BA[0];
        quint8 Mask = (byte >> 7);
        quint64 PayloadLength = (byte & 0x7F);
        // Extended PayloadLength
        if ( PayloadLength == 126 )
        {
            BA = data.mid(offset,2); offset += 2; //tcpSocket->read(2);
            PayloadLength = ((quint8)BA[0] << 8) + (quint8)BA[1];
        }
        else if ( PayloadLength == 127 )
        {
            BA = data.mid(offset,8); offset += 8; //tcpSocket->read(8);
            PayloadLength = 0;
            quint64 plbyte;
            for ( int i=0 ; i<8 ; i++ )
            {
                plbyte = (quint8)BA[i];
                PayloadLength += ( plbyte << (7-i)*8 );
            }
        }

        // MaskingKey
        QByteArray MaskingKey;
        if ( Mask )
        {
            MaskingKey = data.mid(offset,4); offset += 4; //tcpSocket->read(4);
        }

        // Extension // UNSUPPORTED FOR NOW

        // ApplicationData
        if ( PayloadLength )
        {
            QByteArray ApplicationData = data.mid(offset, PayloadLength); offset += PayloadLength; //tcpSocket->read( PayloadLength );
            if ( Mask )
                ApplicationData = QWsSocket::mask( ApplicationData, MaskingKey );
            currentFrame.append( ApplicationData );
        }

        if ( FIN )
        {
            if ( Opcode == OpBinary )
            {
                emit frameReceived( currentFrame );
            }
            else if ( Opcode == OpText )
            {
                QString byteString;
                byteString.reserve(currentFrame.size());
                for (int i=0 ; i<currentFrame.size() ; i++)
                    byteString[i] = currentFrame[i];
                emit frameReceived( byteString );
            }
            else if ( Opcode == OpPing )
            {
                QByteArray pongRequest = QWsSocket::composeHeader( true, OpPong, 0 );
                write( pongRequest );
            }
            else if ( Opcode == OpPong )
            {
                quint64 ms = pingTimer.elapsed();
                emit pong(ms);
            }
            else if ( Opcode == OpClose )
            {
                tcpSocket->close();
            }
            currentFrame.clear();
        }
    }

//    if(tcpSocket->bytesAvailable())
//        dataReceivedAll();
}


void QWsSocket::dataReceivedV0()
{
    QByteArray BA, rawData;
    quint8 type, b = 0x00;

    BA = tcpSocket->read(1);
    type = BA[0];

    if ( ( type & 0x80 ) == 0x00 ) // MSB of type not set
    {
        if ( type != 0x00 )
        {
            // ABORT CONNEXION
            tcpSocket->readAll();
            return;
        }

        // read data
        do
        {
            BA = tcpSocket->read(1);
            b = BA[0];
            if ( b != 0xFF )
                rawData.append( b );
        } while ( b != 0xFF );

        currentFrame.append( rawData );
    }
    else // MSB of type set
    {
        if ( type != 0xFF )
        {
            // ABORT CONNEXION
            tcpSocket->readAll();
            return;
        }

        quint8 length = 0x00;

        bool bIsNotZero = true;
        do
        {
            BA = tcpSocket->read(1);
            b = BA[0];
            bIsNotZero = ( b != 0x00 ? true : false );
            if ( bIsNotZero ) // b must be != 0
            {
                quint8 b_v = b & 0x7F;
                length *= 128;
                length += b_v;
            }
        } while ( ( ( b & 0x80 ) == 0x80 ) && bIsNotZero );

        BA = tcpSocket->read(length); // discard this bytes
    }

    if ( currentFrame.size() > 0 )
    {
        QString byteString;
        byteString.reserve( currentFrame.size() );
        for (int i=0 ; i<currentFrame.size() ; i++)
            byteString[i] = currentFrame[i];
        emit frameReceived( byteString );
        currentFrame.clear();
    }

    if ( tcpSocket->bytesAvailable() )
        dataReceived();
}

qint64 QWsSocket::write ( const QString & string, int maxFrameBytes )
{
    if ( protocolVersion == 0 )
    {
        return QWsSocket::write( string.toLatin1(), maxFrameBytes );
    }

    if ( maxFrameBytes == 0 )
        maxFrameBytes = maxBytesPerFrame;

    QList<QByteArray> framesList = QWsSocket::composeFrames( string.toUtf8(), false, maxFrameBytes );
    return writeFrames( framesList );
}

qint64 QWsSocket::write ( const QByteArray & byteArray, int maxFrameBytes )
{
    if ( protocolVersion == 0 )
    {
        QByteArray BA;
        BA.append( (char)0x00 );
        BA.append( byteArray );
        BA.append( (char)0xFF );
        return writeFrame( BA );
    }

    if ( maxFrameBytes == 0 )
        maxFrameBytes = maxBytesPerFrame;

    QList<QByteArray> framesList = QWsSocket::composeFrames( byteArray, true, maxFrameBytes );
    return writeFrames( framesList );
}

qint64 QWsSocket::writeFrame ( const QByteArray & byteArray )
{
    return tcpSocket->write( byteArray );
}

qint64 QWsSocket::writeFrames ( QList<QByteArray> framesList )
{
    qint64 nbBytesWritten = 0;
    for ( int i=0 ; i<framesList.size() ; i++ )
    {
        nbBytesWritten += writeFrame( framesList[i] );
    }
    return nbBytesWritten;
}

void QWsSocket::close( QString reason )
{
    if ( protocolVersion == 0 )
    {
        QByteArray BA;
        BA.append( (char)0xFF );
        BA.append( (char)0x00 );
        tcpSocket->write(BA);
        tcpSocket->close();
        return;
    }

    // Compose and send close frame
    QByteArray BA;

    QByteArray header = QWsSocket::composeHeader( true, OpClose, 0 );
    BA.append( header );

    // Reason // UNSUPPORTED FOR NOW

    tcpSocket->write( BA );

    tcpSocket->close();
}

void QWsSocket::tcpSocketAboutToClose()
{
    emit aboutToClose();
}

void QWsSocket::tcpSocketDisconnected()
{
    emit disconnected();
}

//QByteArray QWsSocket::generateMaskingKey()
//{
//    QByteArray key;
//    for ( int i=0 ; i<4 ; i++ )
//    {
//        key.append( qrand() % 0x100 );
//    }

//    return key;
//}

QByteArray QWsSocket::mask( QByteArray data, QByteArray maskingKey )
{
    for ( int i=0 ; i<data.size() ; i++ )
    {
        data[i] = ( data[i] ^ maskingKey[ i % 4 ] );
    }

    return data;
}

QList<QByteArray> QWsSocket::composeFrames( QByteArray byteArray, bool asBinary, int maxFrameBytes )
{
    if ( maxFrameBytes == 0 )
        maxFrameBytes = maxBytesPerFrame;

    QList<QByteArray> framesList;

    QByteArray maskingKey;// = generateMaskingKey();

    int nbFrames = byteArray.size() / maxFrameBytes + 1;

    for ( int i=0 ; i<nbFrames ; i++ )
    {
        QByteArray BA;

        // fin, size
        bool fin = false;
        quint64 size = maxFrameBytes;
        EOpcode opcode = OpContinue;
        if ( i == nbFrames-1 ) // for multi-frames
        {
            fin = true;
            size = byteArray.size();
        }
        if ( i == 0 )
        {
            if ( asBinary )
                opcode = OpBinary;
            else
                opcode = OpText;
        }

        // Header
        QByteArray header = QWsSocket::composeHeader( fin, opcode, size, maskingKey );
        BA.append( header );

        // Application Data
        QByteArray dataForThisFrame = byteArray.left( size );
        byteArray.remove( 0, size );

        //dataForThisFrame = QWsSocket::mask( dataForThisFrame, maskingKey );
        BA.append( dataForThisFrame );

        framesList << BA;
    }

    return framesList;
}

QByteArray QWsSocket::composeHeader( bool fin, EOpcode opcode, quint64 payloadLength, QByteArray maskingKey )
{
    QByteArray BA;
    quint8 byte;

    // FIN, RSV1-3, Opcode
    byte = 0x00;
    // FIN
    if ( fin )
        byte = (byte | 0x80);
    // Opcode
    byte = (byte | opcode);
    BA.append( byte );

    // Mask, PayloadLength
    byte = 0x00;
    QByteArray BAsize;
    // Mask
    if ( maskingKey.size() == 4 )
        byte = (byte | 0x80);
    // PayloadLength
    if ( payloadLength <= 125 )
    {
        byte = (byte | payloadLength);
    }
    // Extended payloadLength
    else
    {
        // 2 bytes
        if ( payloadLength <= 0xFFFF )
        {
            byte = ( byte | 126 );
            BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
        }
        // 8 bytes
        else if ( payloadLength <= 0x7FFFFFFF )
        {
            byte = ( byte | 127 );
            BAsize.append( ( payloadLength >> 7*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 6*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 5*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 4*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 3*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 2*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 1*8 ) & 0xFF );
            BAsize.append( ( payloadLength >> 0*8 ) & 0xFF );
        }
    }
    BA.append( byte );
    BA.append( BAsize );

    // Masking
    if ( maskingKey.size() == 4 )
        BA.append( maskingKey );

    return BA;
}

void QWsSocket::ping()
{
    pingTimer.restart();
    QByteArray pingFrame = QWsSocket::composeHeader( true, OpPing, 0 );
    writeFrame( pingFrame );
}

//QString QWsSocket::composeOpeningHandShake( QString ressourceName, QString host, QString origin, QString extensions, QString key )
//{
//    QString hs;
//    hs.append("GET /ws HTTP/1.1\r\n");
//    hs.append("Host: pmx\r\n");
//    hs.append("Upgrade: websocket\r\n");
//    hs.append("Connection: Upgrade\r\n");
//    hs.append("Sec-WebSocket-Version: 6\r\n");
//    hs.append("Sec-WebSocket-Origin: http://pmx\r\n");
//    hs.append("Sec-WebSocket-Extensions: deflate-stream\r\n");
//    hs.append("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n");
//    hs.append("\r\n");
//    return hs;
//}

const QString regExpResourceNameStr( "GET\\s(.*)\\sHTTP/1.1\r\n" );
const QString regExpHostStr( "Host:\\s(.+(:\\d+)?)\r\n" );
const QString regExpKeyStr( "Sec-WebSocket-Key:\\s(.{24})\r\n" );
const QString regExpKey1Str( "Sec-WebSocket-Key1:\\s(.+)\r\n" );
const QString regExpKey2Str( "Sec-WebSocket-Key2:\\s(.+)\r\n" );
const QString regExpKey3Str( "(.{8})(\r\n)*$" );
const QString regExpVersionStr( "Sec-WebSocket-Version:\\s(\\d+)\r\n" );
const QString regExpOriginStr( "Origin:\\s(.+)\r\n" );
const QString regExpOriginV6Str( "Sec-WebSocket-Origin:\\s(.+)\r\n" );
const QString regExpProtocolStr( "Sec-WebSocket-Protocol:\\s(.+)\r\n" );
const QString regExpExtensionsStr( "Sec-WebSocket-Extensions:\\s(.+)\r\n" );

QString
QWsSocket::serializeInt( quint32 number, quint8 nbBytes)
{
    QString bin;
    quint8 currentNbBytes = 0;
    while (number > 0 && currentNbBytes < nbBytes)
    {
        bin.prepend( QChar::fromLatin1(number) );
        number = number >> 8;
        currentNbBytes++;
    }
    while (currentNbBytes < nbBytes)
    {
        bin.prepend( QChar::fromLatin1(0) );
        currentNbBytes++;
    }
    return bin;
}

QString
QWsSocket::computeAcceptV2(QString key)
{
    key += "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    QByteArray hash = QCryptographicHash::hash ( key.toUtf8(), QCryptographicHash::Sha1 );
    return hash.toBase64();
}

QString
QWsSocket::computeAcceptV1( QString key1, QString key2, QString key3 )
{
    QString numStr1;
    QString numStr2;

    QChar carac;
    for ( int i=0 ; i<key1.size() ; i++ )
    {
        carac = key1[ i ];
        if ( carac.isDigit() )
            numStr1.append( carac );
    }
    for ( int i=0 ; i<key2.size() ; i++ )
    {
        carac = key2[ i ];
        if ( carac.isDigit() )
            numStr2.append( carac );
    }

    quint32 num1 = numStr1.toUInt();
    quint32 num2 = numStr2.toUInt();

    //qDebug() << QString::number(num1);
    //qDebug() << QString::number(num2);

    int numSpaces1 = key1.count( ' ' );
    int numSpaces2 = key2.count( ' ' );

    //qDebug() << QString::number(numSpaces1);
    //qDebug() << QString::number(numSpaces2);

    num1 /= numSpaces1;
    num2 /= numSpaces2;

    QString concat = serializeInt( num1 ) + serializeInt( num2 ) + key3;

    QByteArray md5 = QCryptographicHash::hash( concat.toLatin1(), QCryptographicHash::Md5 );

    return QString( md5 );
}

bool
QWsSocket::initializeWebSocket(const QString &request, QString &response)
{
//    std::cout << request.toStdString() << std::endl;
    QRegExp regExp;
    regExp.setMinimal( true );

    // Extract mandatory datas

    // Version
    regExp.setPattern( regExpVersionStr );
    regExp.indexIn(request);
    QString versionStr = regExp.cap(1);
    int version = 0;
    if ( ! versionStr.isEmpty() )
        version = versionStr.toInt();

    // Resource name
    regExp.setPattern( regExpResourceNameStr );
    regExp.indexIn(request);
    QString resourceName = regExp.cap(1);

    // Host (address & port)
    regExp.setPattern( regExpHostStr );
    regExp.indexIn(request);
    QStringList sl = regExp.cap(1).split(':');
    QString hostAddress = sl[0];
    QString hostPort;
    if ( sl.size() > 1 )
        hostPort = sl[1];

    // Key
    QString key, key1, key2, key3;
    if ( version >= 6 )
    {
        regExp.setPattern( regExpKeyStr );
        regExp.indexIn(request);
        key = regExp.cap(1);
    }
    else
    {
        regExp.setPattern( regExpKey1Str );
        regExp.indexIn(request);
        key1 = regExp.cap(1);
        regExp.setPattern( regExpKey2Str );
        regExp.indexIn(request);
        key2 = regExp.cap(1);
        regExp.setPattern( regExpKey3Str );
        regExp.indexIn(request);
        key3 = regExp.cap(1);
    }

    // Extract optional datas
    // Origin
    QString origin;
    if ( version < 6 || version > 8 )
    {
        regExp.setPattern( regExpOriginStr );
        regExp.indexIn(request);
        origin = regExp.cap(1);
    }
    else
    {
        regExp.setPattern( regExpOriginV6Str );
        regExp.indexIn(request);
        origin = regExp.cap(1);
    }

    // Protocol
    regExp.setPattern( regExpProtocolStr );
    regExp.indexIn(request);
    QString protocol = regExp.cap(1);

    // Extensions
    regExp.setPattern( regExpExtensionsStr );
    regExp.indexIn(request);
    QString extensions = regExp.cap(1);

    ////////////////////////////////////////////////////////////////////

//    std::cout << "hostaddr: " << hostAddress.toStdString() << " "
//              << "resource: " << resourceName.toStdString() << " "
//              << "key: " << key.toStdString() << " -- 1: "
//              << key1.toStdString() << "2: "
//              << key2.toStdString() << "3: "
//              << key3.toStdString() << std::endl;

    // If the mandatory params are not set, we abort the connection to the Websocket server
    if ( hostAddress.isEmpty() || resourceName.isEmpty()) return false;
    if( key.isEmpty() && ( key1.isEmpty() || key2.isEmpty() || key3.isEmpty() ) )
        return false;

    ////////////////////////////////////////////////////////////////////

    // Compose handshake answer

    QString answer;

    QString accept;
    if ( version >= 6 )
    {
        accept = computeAcceptV2( key );
        answer.append("HTTP/1.1 101 Switching Protocols\r\n");
        answer.append("Upgrade: websocket\r\n");
        answer.append("Connection: Upgrade\r\n");
        answer.append("Sec-WebSocket-Accept: " + accept + "\r\n" + "\r\n");
    }
    else if ( version < 6 )
    {
        accept = computeAcceptV1( key1, key2, key3 );
        answer.append("HTTP/1.1 101 WebSocket Protocol Handshake\r\n");
        answer.append("Upgrade: Websocket\r\n");
        answer.append("Connection: Upgrade\r\n");
        answer.append("Sec-WebSocket-Origin: " + origin + "\r\n");
        answer.append("Sec-WebSocket-Location: ws://" + hostAddress + ( hostPort.isEmpty() ? "" : (":"+hostPort) ) + resourceName + "\r\n");
        if ( !protocol.isEmpty() )
            answer.append("Sec-WebSocket-Protocol: " + protocol + "\r\n");
        answer.append("\r\n");
        answer.append( accept );
    }

    // Handshake OK, new connection
    //disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));

    // Send handshake answer
    response = answer;
    return true;
}


// ****************************************************************************
// Method: WebSocketConnection::WebSocketConnection
//
// Purpose: 
//   Constructor for the WebSocketConnection class.
//
// Arguments:
//   _descriptor : The file descriptor of the socket we want to use.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:16:31 PDT 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 25 12:33:03 PDT 2002
//   Initialized the zeroesRead member.
//
// ****************************************************************************

WebSocketConnection::WebSocketConnection(QTcpSocket* tcpSocket,const QString& request) : AttributeSubjectSocketConnection(tcpSocket->socketDescriptor()) //DESCRIPTOR descriptor_) : SocketConnection(descriptor_) /*: buffer()*/
{
    QString response = "";

    bool res = QWsSocket::initializeWebSocket(request,response);

    /// HKTODO: Throw Exception?
    if(res == false)
    {
        std::cerr << "handshake did not match??" << std::endl;
    }

    tcpSocket->write(response.toLatin1());
    tcpSocket->flush();
    /// end handshake

    socket = new QWsSocket( tcpSocket );

    QObject::connect(socket,SIGNAL(frameReceived(QByteArray)),this, SLOT(ReadFrame(QByteArray)));
    QObject::connect(socket,SIGNAL(frameReceived(QString)),this, SLOT(ReadFrame(QString)));
    QObject::connect(socket,SIGNAL(aboutToClose()),this, SLOT(closeConnection()));
}

void
WebSocketConnection::closeConnection()
{
    //std::cout << "Close this connection" << std::endl;
    QObject::disconnect(socket,SIGNAL(frameReceived(QByteArray)),this, SLOT(ReadFrame(QByteArray)));
    QObject::disconnect(socket,SIGNAL(frameReceived(QString)),this, SLOT(ReadFrame(QString)));
    QObject::disconnect(socket,SIGNAL(aboutToClose()),this, SLOT(closeConnection()));
    socket->close("closing the connection");
    socket->internalSocket()->close();
}

void
WebSocketConnection::ReadFrame(QByteArray &array)
{
    std::cout << "binary" << std::endl;
}

void
WebSocketConnection::ReadFrame(const QString &str)
{
    //test if object..
    if(!str.startsWith("{") || !str.endsWith("}"))
    {
        std::cout << "Incomplete Object..:" << str.toStdString() << std::endl;
        while(socket->internalSocket()->bytesAvailable())
            socket->internalSocket()->readAll();
        return;
    }

    messageRead = str;
    messages.push_back(str);

    emit frameRead(socket->internalSocket()->socketDescriptor());
}

// ****************************************************************************
// Method: WebSocketConnection::~WebSocketConnection
//
// Purpose: 
//   Destructor for the WebSocketConnection class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:17:12 PDT 2000
//
// Modifications:
//   Brad Whitlock, Fri Mar 22 17:07:16 PST 2002
//   Made it close the socket.
//
// ****************************************************************************

WebSocketConnection::~WebSocketConnection()
{
    socket->close("closing the connection");
    delete socket;
}

// ****************************************************************************
// Method: WebSocketConnection::GetDescriptor
//
// Purpose: 
//   Returns the socket descriptor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Oct 5 18:30:32 PST 2000
//
// Modifications:
//   
// ****************************************************************************

int
WebSocketConnection::GetDescriptor() const
{
    return int(descriptor);
}

long
WebSocketConnection::ReadHeader(unsigned char *buf, long len)
{
    //std::cout << "Reading Header" << std::endl;
    messageRead = "";

    QEventLoop loop;

//    QObject::connect(socket,SIGNAL(frameReceived(QString)),
//                     this,SLOT(ReadFrame(QString)));

    QObject::connect(socket,SIGNAL(frameReceived(QString)),
                     &loop,SLOT(quit()));

    loop.exec();

    JSONNode node;
    node.Parse(messageRead.toStdString());

    std::string version = node["version"].GetString();
    std::string socketKey = node["socketKey"].GetString();
    std::string securityKey = node["securityKey"].GetString();

    char* buffer = (char*)buf;
    buffer[0] = TypeRepresentation::ASCIIFORMAT;
    buffer[1] = srcFormat.IntFormat;
    buffer[2] = srcFormat.LongFormat;
    buffer[3] = srcFormat.FloatFormat;
    buffer[4] = srcFormat.DoubleFormat;
    buffer[5] = 0; //no fail..
    /// version is 10 bytes in len
    strcpy(buffer+6,version.c_str());
    /// security key is 21 bytes in len
    strcpy(buffer+6+10,securityKey.c_str());
    /// socket key is 21 in len..
    strcpy(buffer+6+10+21,socketKey.c_str());

    return len;
}

long
WebSocketConnection::WriteHeader(const unsigned char *buf, long len)
{

    //if not connected then sending a message would be useless
    if(socket->state() != QAbstractSocket::ConnectedState) return 0;

    JSONNode node;
    QString version = (const char *)(buf+6);
    QString socketKey = (const char *)(buf+6+10+21);

    node["version"] = version.toStdString();
    node["socketKey"] = socketKey.toStdString();

    socket->write(QString(node.ToString().c_str()));
    socket->flush();

    /// wait for bytes written
    if(socket->state() != QAbstractSocket::UnconnectedState)
        socket->internalSocket()->waitForBytesWritten();

    return len;
}

// ****************************************************************************
// Method: WebSocketConnection::Fill
//
// Purpose: 
//   Tries to fill up the connection with new data.
//
// Returns:    The number of bytes read.
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 20 17:28:27 PST 2002
//
// Modifications:
//   
// ****************************************************************************


int
WebSocketConnection::Fill()
{
    //if not connected then sending a message would be useless
    if(socket->state() != QAbstractSocket::ConnectedState) return 0;

    if(messages.size() == 0)
        socket->internalSocket()->waitForReadyRead();

    //std::cout << "message: " << messageRead.toStdString() << std::endl;

    int res = 0;
    buffer.clear();

    for(size_t i = 0; i < messages.size(); ++i)
    {  
        QString messageRead = messages[i];
        //std::cout << messageRead.toStdString() << std::endl;
        if(messageRead.size() == 0 || !messageRead.startsWith("{") || !messageRead.endsWith("}"))
            continue;

        //std::cout << messageRead.toStdString() << std::endl;
        std::string message = messageRead.toStdString();
        messageRead = "";

        JSONNode node;
        try{
        node.Parse(message);
        }
        catch(...)
        {
            std::cerr << "Exception caught while parsing JSON object" << std::endl;
            std::cerr << "message: " << message << std::endl;
            continue;
        }

        int guido = node["id"].GetInt();
        JSONNode contents = node["contents"];
        JSONNode metadata = node["metadata"];

        /// With the information I have I could probably
        /// just use JSONNode to convert completely..
        /// but that would leave MapNode incomplete..

        //MapNode mapnode(contents, metadata, false);

        //std::cout << mapnode.ToXML(false) << std::endl;
        //std::cout << mapnode.ToJSON(false) << std::endl;

        AttributeSubjectSerialize ser;
        ser.SetConnection(this);
        res += ser.Write(guido, contents, metadata); //Write(guido,&mapnode);
    }

    messages.clear();
    return res;
}


// ****************************************************************************
// Method: WebSocketConnection::Flush
//
// Purpose: 
//   Writes the entire contents of the connection's buffer onto the
//   socket file descriptor in chunks. It then clears the buffer.
//
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:17:37 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Mar 26 13:29:20 PST 2002
//   Made it use socket functions so it is more portable.
//
//   Brad Whitlock, Thu Jan 25 18:42:50 PST 2007
//   I made it use MSG_NOSIGNAL so we don't get a signal in the event that
//   we can't write to the socket.
//
//   Eric Brugger, Tue Mar 13 09:18:48 PDT 2007
//   I made the use of MSG_NOSIGNAL conditional on its definition.
//
// ****************************************************************************
void
WebSocketConnection::FlushAttr(AttributeSubject *subject)
{
//    std::cout << subject->TypeName() << " "
//              << subject->CalculateMessageSize(*this)
//              << std::endl;

    //if not connected then sending a message would be useless
    if(socket->state() != QAbstractSocket::ConnectedState) return;
    /// write meta object..
    try{
        if(subject->GetSendMetaInformation())
        {
            QString qoutput = serializeMetaData(subject).c_str();
            socket->write(qoutput);

            if(socket->internalSocket()->state() != QAbstractSocket::UnconnectedState)
                socket->internalSocket()->waitForBytesWritten();
        }

        QString qoutput = serializeAttributeSubject(subject).c_str();
        socket->write(qoutput);

        if(socket->internalSocket()->state() != QAbstractSocket::UnconnectedState)
            socket->internalSocket()->waitForBytesWritten();
    }
    catch(...)
    {
        std::cerr << "Exception occurred in Write out.." << subject->TypeName() << std::endl;
    }
    buffer.clear();
}

void
WebSocketConnection::Flush()
{
    /// do nothing, yet..
    buffer.clear();
}

// ****************************************************************************
// Method: WebSocketConnection::Size
//
// Purpose: 
//   Returns the size of the connection's buffer. This is how many
//   bytes can still be consumed.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:18:32 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

long
WebSocketConnection::Size()
{
    return (long)buffer.size();
}

// ****************************************************************************
// Method: WebSocketConnection::Write
//
// Purpose: 
//   Appends a character to the connection's buffer.
//
// Arguments:
//   value : The character to append.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:19:23 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
WebSocketConnection::Write(unsigned char value)
{
    buffer.push_back(value);
}

// ****************************************************************************
// Method: WebSocketConnection::Read
//
// Purpose: 
//   Reads a character from the start of the connection's buffer.
//
// Arguments:
//   address : The memory location in which to store the character.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:20:03 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
WebSocketConnection::Read(unsigned char *address)
{
   *address = buffer.front();
    buffer.pop_front();
}

// ****************************************************************************
// Method: WebSocketConnection::Append
//
// Purpose: 
//   Adds characters to the end of the connection's buffer.
//
// Arguments:
//   buf   : An array of characters to append.
//   count : The length of the array.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:20:50 PDT 2000
//
// Modifications:
//   
// ****************************************************************************

void
WebSocketConnection::Append(const unsigned char *buf, int count)
{
    const unsigned char *temp = buf;
    for(int i = 0; i < count; ++i)
        buffer.push_back(*temp++);
}

// ****************************************************************************
// Method: WebSocketConnection::DirectRead
//
// Purpose: 
//   Reads the specified number of bytes from the connection.
//
// Arguments:
//   buf    : The destination buffer.
//   ntotal : The number of bytes to read.
//
// Returns:    The number of bytes read.
//
// Note:       This method does not return until the specified number of
//             bytes have been read. This method was adapted from Jeremy's
//             read_n function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:28:39 PST 2002
//
// Modifications:
//   Brad Whitlock, Mon Jul 19 11:41:13 PDT 2004
//   I fixed a problem reading large messages on Windows.
//
// ****************************************************************************

long
WebSocketConnection::DirectRead(unsigned char *buf, long ntotal)
{
    std::cout << "Direct Reading not allowed" << std::endl;
    return 0;
}

// ****************************************************************************
// Method: WebSocketConnection::DirectWrite
//
// Purpose: 
//   Writes a buffer to the connection without doing any buffering.
//
// Arguments:
//   buf    : The buffer that we want to write.
//   ntotal : The length of the buffer.
//
// Returns:    The number of bytes written.
//
// Note:       This method was adapted from Jeremy's write_n function.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 14:33:40 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Jan 30 17:39:47 PST 2004
//    Fixed a bug where failed sends would restart from the beginning.
//
// ****************************************************************************

long
WebSocketConnection::DirectWrite(const unsigned char *buf, long ntotal)
{
    std::cout << "Direct Writing not allowed" << std::endl;
    return 0;
}

// ****************************************************************************
// Method: WebSocketConnection::NeedsRead
//
// Purpose: 
//   Returns whether or not the connection has data that could be read.
//
// Arguments:
//   blocking : Specifies whether we should block until input needs to be
//              read or whether we should poll.
//
// Returns:  True if there is data, false otherwise.  
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 25 16:01:17 PST 2002
//
// Modifications:
//    Tom Fogal, Sat Feb 16 15:47:15 EST 2008
//    Restart the system call if it gets interrupted.
//
//    Brad Whitlock, Thu Jun 11 15:14:50 PST 2009
//    Don't call select if we don't have it.
//
// ****************************************************************************

bool
WebSocketConnection::NeedsRead(bool blocking) const
{
    return true;
}
