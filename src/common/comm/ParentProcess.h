#ifndef PARENT_PROCESS_H
#define PARENT_PROCESS_H
#include <comm_exports.h>
#include <string>
#include <vector>

class Connection;

// ****************************************************************************
// Class: ParentProcess
//
// Purpose:
//   This class sets up communication with a parent process. It first
//   reads in the command line arguments that tell about the process
//   that launched it, then it sets up connections with that process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 14:59:26 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//   Changed it to allow more than one read/write socket.   
//
//   Brad Whitlock, Thu Apr 26 15:57:46 PST 2001
//   Added version stuff.
// 
//   Jeremy Meredith, Fri Apr 27 15:28:42 PDT 2001
//   Added fail bit.
//
//   Brad Whitlock, Thu Apr 11 15:42:42 PST 2002
//   Added a GetUserName method.
//
//   Brad Whitlock, Mon Mar 25 15:11:28 PST 2002
//   Made it use regular connections.
//
//   Brad Whitlock, Mon Dec 16 14:27:22 PST 2002
//   Added a security key.
//
// ****************************************************************************

class COMM_API ParentProcess
{
public:
    ParentProcess();
    ~ParentProcess();
    void Connect(int *argc, char **argv[], bool createSockets, int failCode=0);
    const std::string &GetHostName() const;
    std::string        GetTheUserName() const;
    Connection  *GetReadConnection(int i=0) const;
    Connection  *GetWriteConnection(int i=0) const;
    void         SetVersion(const std::string &ver);
private:
    void ExchangeTypeRepresentations(int failCode=0);
    int  GetClientSocketDescriptor(int port);
    void GetHostInfo();
private:
    std::string      hostName;
    void             *hostInfo;
    Connection     **readConnections, **writeConnections;
    int              nReadConnections,  nWriteConnections;
    std::string      version;
    std::string      securityKey;
};

#endif
