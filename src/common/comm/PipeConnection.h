// ************************************************************************* //
//                              PipeConnection.h                             //
// ************************************************************************* //

#ifndef PIPE_CONNECTION_H
#define PIPE_CONNECTION_H
#include <SocketConnection.h>
#include <string>

// ****************************************************************************
// Class: PipeConnection
//
// Purpose:
//   This is a buffered connection that can write itself onto a pipe.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed May 21 13:45:35 PST 2003
//
// Modifications:
//
// ****************************************************************************

class COMM_API PipeConnection : public SocketConnection
{
public:
    PipeConnection(bool writeMode);
    PipeConnection(const std::string &name, bool writeMode);
    virtual ~PipeConnection();

    virtual std::string GetDescriptorName(int = 0) const;
    virtual void Initialize();

protected:
    virtual void CloseDescriptor(DESCRIPTOR d);
    virtual int  SendBytes(DESCRIPTOR d, const unsigned char *buf, long count);
    virtual int  ReadBytes(DESCRIPTOR d, unsigned char *buf, long bufSize);

    std::string pipeName;
    bool        ownsPipe;
    bool        writeMode;

    static int  connectionCount;
};

#endif
