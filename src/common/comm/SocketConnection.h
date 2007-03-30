// ************************************************************************* //
//                              SocketConnection.h                           //
// ************************************************************************* //

#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H
#include <comm_exports.h>

#include <Connection.h>
#include <deque>

// ****************************************************************************
// Class: SocketConnection
//
// Purpose:
//   This is a buffered connection that can write itself onto a
//   socket.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 12:11:22 PDT 2000
//
// Modifications:
//   Brad Whitlock, Wed Mar 20 17:23:43 PST 2002
//   Added Fill, DirectRead, DirectWrite, NeedsRead methods.
//
// ****************************************************************************

class COMM_API SocketConnection : public Connection
{
public:
    SocketConnection(DESCRIPTOR descriptor_);
    virtual ~SocketConnection();

    virtual int  Fill();
    virtual void Flush();
    virtual long Size();
    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
    virtual void Append(const unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
    virtual bool NeedsRead(bool blocking = false) const;
    virtual int  GetDescriptor() const;
private:
    std::deque<unsigned char> buffer;
    DESCRIPTOR                descriptor;
    int                       zeroesRead;
};

#endif
