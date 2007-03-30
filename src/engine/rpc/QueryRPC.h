#ifndef QUERY_RPC_H 
#define QUERY_RPC_H 
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <QueryAttributes.h>

// ****************************************************************************
//  Class:  QueryRPC
//
//  Purpose:
//    Implements an RPC to perform a query. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 6, 2002 
//
// ****************************************************************************

class ENGINE_RPC_API QueryRPC : public NonBlockingRPC
{
public:
    QueryRPC();
    virtual ~QueryRPC() { };

    // Invokation method
    void operator() (const int netid, const QueryAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void    SetNetworkId(const int netId)
        { networkId = netId; Select(0, (void *)&networkId); };

    void SetQueryAtts(const QueryAttributes*);

    // Property getting methods
    int     GetNetworkId() const { return networkId; };

    // Property getting methods
    QueryAttributes *GetQueryAtts();

    // Property getting methods
    QueryAttributes GetReturnAtts() { return returnAtts; } ; 

    QueryAttributes returnAtts; 

private:
    int         networkId;
    QueryAttributes queryAtts; 
};

#endif
