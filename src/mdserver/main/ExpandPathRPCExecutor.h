#ifndef EXPAND_PATH_RPC_EXECUTOR_H
#define EXPAND_PATH_RPC_EXECUTOR_H
#include <Observer.h>

class MDServerConnection;

// ****************************************************************************
// Class: ExpandPathRPCExecutor
//
// Purpose:
//   This class handles the RPC request for expanding a path.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:34:38 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class ExpandPathRPCExecutor : public Observer
{
public:
    ExpandPathRPCExecutor(MDServerConnection *parent_, Subject *s);
    ~ExpandPathRPCExecutor();

    virtual void Update(Subject *s);
private:
    MDServerConnection *parent;
};

#endif
