#ifndef CONNECTION_GROUP_H
#define CONNECTION_GROUP_H
#include <comm_exports.h>

class Connection;

// ****************************************************************************
// Class: ConnectionGroup
//
// Purpose:
//   This class allows connections to be grouped to determine if they need
//   to be read.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 26 11:50:07 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class COMM_API ConnectionGroup
{
public:
    ConnectionGroup();
    ~ConnectionGroup();
    void AddConnection(Connection *conn);
    bool NeedsRead(int i) const;
    bool CheckInput();
private:
    int         nConnections;
    Connection *connections[10];
    bool        needsRead[10];
};

#endif
