#ifndef GET_DIRECTORY_RPC_EXECUTOR_H
#define GET_DIRECTORY_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: GetDirectoryRPCExecutor
//
// Purpose:
//   This class handles the RPC request for getting the path of the
//   current working directory.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 16:03:35 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Nov 17 15:33:59 PST 2000
//   I added the parent argument to the constructor.
//
// ****************************************************************************

class GetDirectoryRPCExecutor : public Observer
{
public:
    GetDirectoryRPCExecutor(MDServerConnection *parent_, Subject *s);
    virtual ~GetDirectoryRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
