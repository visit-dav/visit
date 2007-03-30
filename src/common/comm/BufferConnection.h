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
// ****************************************************************************

class COMM_API BufferConnection : public Connection
{
public:
    BufferConnection();
    virtual ~BufferConnection();

    virtual int  Fill();
    virtual void Flush();
    virtual long Size();

    virtual void Write(unsigned char value);
    virtual void Read(unsigned char *address);
    virtual void Append(const unsigned char *buf, int count);
    virtual long DirectRead(unsigned char *buf, long len);
    virtual long DirectWrite(const unsigned char *buf, long len);
private:
    std::deque<unsigned char> buffer;
};

#endif
