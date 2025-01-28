// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef BUFFER_CONNECTION_H
#define BUFFER_CONNECTION_H
#include <comm_exports.h>
#include <Connection.h>
#include <deque>

// ****************************************************************************
// Class: BufferConnection
//
// Purpose:
//   This class is a simple buffer that has the interface of a Connection.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 20 17:32:26 PST 2002
//
// Modifications:
//    Brad Whitlock, Wed Mar 20 17:23:43 PST 2002
//    Added Fill, DirectWrite, DirectRead methods.
//    
//    Cyrus Harrison, Tue Jan 28 09:55:51 PST 2025
//    Convert to size_t for buffer sizes
//
// ****************************************************************************

class COMM_API BufferConnection : public Connection
{
public:
    BufferConnection();
    virtual ~BufferConnection();

    virtual size_t Fill();
    virtual void   Flush();
    virtual size_t Size();
    virtual void   Reset();

    virtual void   Write(unsigned char value);
    virtual void   Read(unsigned char *address);
    virtual void   Append(const unsigned char *buf, size_t count);
    virtual size_t DirectRead(unsigned char *buf, size_t len);
    virtual size_t DirectWrite(const unsigned char *buf, size_t len);
private:
    std::deque<unsigned char> buffer;
};

#endif
