// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ENGINE_PROPERTIES_RPC_H 
#define ENGINE_PROPERTIES_RPC_H 
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <EngineProperties.h>

// ****************************************************************************
//  Class:  EnginePropertiesRPC
//
//  Purpose:
//    Implements an RPC to get engine properties.
//
//  Programmer:  Brad Whitlock
//  Creation:    Mon Oct 10 11:11:05 PDT 2011
//
// ****************************************************************************

class ENGINE_RPC_API EnginePropertiesRPC : public NonBlockingRPC
{
public:
    EnginePropertiesRPC();
    virtual ~EnginePropertiesRPC() { };

    virtual const std::string TypeName() const { return "EnginePropertiesRPC"; }

    // Invokation method
    void operator()() { Execute(); };

    void SelectAll() { };

    // Property getting methods
    EngineProperties GetReturnAtts() { return returnAtts; } ; 

    EngineProperties returnAtts; 
};

#endif
