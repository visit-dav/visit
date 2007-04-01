#ifndef LOAD_PLUGINS_RPC_H
#define LOAD_PLUGINS_RPC_H
#include <mdsrpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: LoadPluginsRPC
//
// Purpose:
//     Tells the mdserver that now is a good time to load its plugins.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

class MDSERVER_RPC_API LoadPluginsRPC : public NonBlockingRPC
{
public:
    LoadPluginsRPC();
    virtual ~LoadPluginsRPC();

    // Invokation method
    void operator()(void);

    // Property selection methods
    virtual void SelectAll();

private:
};


#endif
