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
