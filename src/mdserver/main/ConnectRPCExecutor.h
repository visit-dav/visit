#ifndef CONNECT_RPC_EXECUTOR_H
#define CONNECT_RPC_EXECUTOR_H
#include <Observer.h>

// ****************************************************************************
// Class: ConnectRPCExecutor
//
// Purpose:
//   This RPC causes the MDServer to connect to another process. The other
//   process can then use the functions provided by the MDServer.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 15:45:42 PST 2000
//
// Modifications:
//   
// ****************************************************************************

class ConnectRPCExecutor : public Observer
{
public:
    ConnectRPCExecutor(Subject *s);
    ~ConnectRPCExecutor();

    virtual void Update(Subject *s);
private:
    char *StrDup(const char *str);
};

#endif
