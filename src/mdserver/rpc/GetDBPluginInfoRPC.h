#ifndef _GETDB_PLUGIN_INFO_RPC_H_
#define _GETDB_PLUGIN_INFO_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>
#include <DBPluginInfoAttributes.h>

// ****************************************************************************
// Class: GetDBPluginInfoRPC
//
// Purpose:
//   This class encapsulates a call to get the DBPluginInfo for a database
//   from a remote file system.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   May 23, 2005
//
// ****************************************************************************

class MDSERVER_RPC_API GetDBPluginInfoRPC : public BlockingRPC
{
public:
    GetDBPluginInfoRPC();
    virtual ~GetDBPluginInfoRPC();

    virtual const std::string TypeName() const;

    // Invokation method
    const DBPluginInfoAttributes *operator()();

    // Property selection methods
    virtual void SelectAll();
private:
    DBPluginInfoAttributes    dbPluginInfo;
};


#endif
