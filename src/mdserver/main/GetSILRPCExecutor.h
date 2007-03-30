#ifndef GET_SIL_RPC_EXECUTOR_H
#define GET_SIL_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// *******************************************************************
// Class: GetSILRPCExecutor
//
// Purpose:
//   This class handles the RPC request for getting the SIL
//   for a file (which may be in the CWD if no path is specified).
//
// Programmer: Hank Childs
// Creation:   March 29, 2001
//
// *******************************************************************

class GetSILRPCExecutor : public Observer
{
public:
    GetSILRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~GetSILRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
