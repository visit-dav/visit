// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Brad Whitlock, Tue Jul 29 10:54:29 PDT 2003
//   Added numRead and numWrite to the Connect method so we know how many
//   connections to create without having to pass the numbers on the command
//   line.
//
//   Brad Whitlock, Wed Jan 11 17:00:35 PST 2006
//   Added localUserName.
//
//   Jeremy Meredith, Wed Apr 30 12:26:25 EDT 2008
//   Added apparent host name (tries to resolve localhost into a real name).
//
//   Brad Whitlock, Tue Apr 14 15:13:20 PDT 2009
//   I made Connect return bool as to whether any connections were created
//   since it is possible to call Connect and not create any connections.
//
//   Eric Brugger, Wed Sep 16 16:52:52 PDT 2015
//   I corrected a bug where setting up of the connections might hang.
//   This was caused by the connections not being formed in the same order
//   between the two processes, which resulted in the read and write
//   connections being mismatched between the local and remote processes,
//   resulting in hangs. This only appeared to happen going from Windows
//   to linux with ssh forwarding over a gateway. To solve the issue I
//   added code that wrote the index of the creation on the local side
//   over each connection so that the order could be duplicated on the
//   remote side.
//
// ****************************************************************************

class COMM_API ParentProcess
{
public:
    ParentProcess();
    ~ParentProcess();
    bool Connect(int numRead, int numWrite, int *argc, char **argv[],
                 bool createSockets, int failCode=0);
    const std::string &GetHostName() const;
    const std::string &GetApparentHostName();
    std::string        GetTheUserName() const;
    Connection  *GetReadConnection(int i=0) const;
    Connection  *GetWriteConnection(int i=0) const;
    void         SetVersion(const std::string &ver);
    const std::string &GetLocalUserName();
private:
    void OrderConnections();
    void ExchangeTypeRepresentations(int failCode=0);
    int  GetClientSocketDescriptor(int port);
    void GetHostInfo();
private:
    std::string      hostName;
    std::string      apparentHostName;
    void             *hostInfo;
    Connection     **readConnections, **writeConnections;
    int              nReadConnections,  nWriteConnections;
    std::string      version;
    std::string      securityKey;
    std::string      localUserName;
};

#endif
