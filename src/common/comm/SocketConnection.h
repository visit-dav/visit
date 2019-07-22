// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                              SocketConnection.h                           //
// ************************************************************************* //

#ifndef SOCKET_CONNECTION_H
#define SOCKET_CONNECTION_H
#include <comm_exports.h>

#include <Connection.h>
#include <deque>
#include <JSONNode.h>

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
//   Brad Whitlock, Tue Oct 14 14:43:59 PDT 2014
//   I removed some json/mapnode code and added code to send variable length
//   data in a set of fixed size buffers.
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
    virtual void Reset();
    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
    virtual void Append(const unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
    virtual long ReadHeader(unsigned char *buf, long len);
    virtual long WriteHeader(const unsigned char *buf, long len);
    virtual bool NeedsRead(bool blocking = false) const;
    virtual int  GetDescriptor() const;

    void SetFixedBufferMode(bool val);
    bool GetFixedBufferMode();

    static const int FIXED_BUFFER_SIZE;
protected:
    int  DecodeSize(const unsigned char *buf, int &offset) const;
    void EncodeSize(int sz, unsigned char *buf, int &offset) const;
    void WaitForDescriptor(bool input);

    std::deque<unsigned char> buffer;
    DESCRIPTOR                descriptor;
    int                       zeroesRead;
    bool                      fixedBufferMode;
};

class AttributeSubject;
class COMM_API AttributeSubjectSocketConnection : public SocketConnection
{
public:
    AttributeSubjectSocketConnection(DESCRIPTOR descriptor_): SocketConnection(descriptor_) {}
    virtual ~AttributeSubjectSocketConnection() {}

    virtual void FlushAttr(AttributeSubject*) = 0;
    virtual int Fill() = 0;

    static std::string serializeMetaData(AttributeSubject*);
    static std::string serializeAttributeSubject(AttributeSubject*);
};
#endif
