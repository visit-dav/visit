// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CLONE_NETWORK_RPC_H
#define CLONE_NETWORK_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <QueryOverTimeAttributes.h>

// ****************************************************************************
//  Class:  CloneNetworkRPC
//
//  Purpose:
//    Implements an RPC to signal an existing network to create a clone of 
//    itself. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    March 18, 2004 
//
// ****************************************************************************
class ENGINE_RPC_API CloneNetworkRPC : public BlockingRPC
{
public:
    CloneNetworkRPC();
    virtual ~CloneNetworkRPC();

    virtual const std::string TypeName() const { return "CloneNetworkRPC"; }

    // Invocation method
    void operator()(const int, const QueryOverTimeAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetID(const int);
    void SetQueryOverTimeAtts(const QueryOverTimeAttributes *);

    // Property getting methods
    int              GetID() const;
    QueryOverTimeAttributes *GetQueryOverTimeAtts();


private:
    int               id;
    QueryOverTimeAttributes   queryAtts; 
};

#endif
