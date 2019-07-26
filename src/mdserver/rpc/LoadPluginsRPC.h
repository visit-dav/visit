// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
// Modifications:
//   Brad Whitlock, Fri Dec  7 11:47:50 PST 2007
//   Added TypeName override.
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

    virtual const std::string TypeName() const;
};


#endif
