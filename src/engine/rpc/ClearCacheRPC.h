// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLEAR_CACHE_RPC_H
#define CLEAR_CACHE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
// Class: ClearCacheRPC
//
// Purpose:
//   Implements an RPC that tells the engine to clear cached information for
//   the specified database.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 12:55:37 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

class ENGINE_RPC_API ClearCacheRPC : public BlockingRPC
{
public:
    ClearCacheRPC();
    virtual ~ClearCacheRPC();

    virtual const std::string TypeName() const { return "ClearCacheRPC";}


    // Invokation methods.
    void operator()(const std::string &filename, bool clearAllCaches);

    virtual void SelectAll();

    // Property getting routines.
    const std::string &GetDatabaseName() const { return dbName; }
    bool GetClearAll() const { return clearAll; }
private:
    std::string dbName;
    bool        clearAll;
};

#endif
