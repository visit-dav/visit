#ifndef QUERY_RPC_H 
#define QUERY_RPC_H 
#include <vector>
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
//  Modifications:
//
//    Hank Childs, Thu Oct  2 16:22:29 PDT 2003
//    Allow for queries to involve multiple networks.
//
// ****************************************************************************

class ENGINE_RPC_API QueryRPC : public NonBlockingRPC
{
public:
    QueryRPC();
    virtual ~QueryRPC() { };

    // Invokation method
    void operator() (const std::vector<int> &netids, const QueryAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void    SetNetworkIds(const std::vector<int> &netIds)
        { networkIds = netIds; Select(0, (void *)&networkIds); };

    void SetQueryAtts(const QueryAttributes*);

    // Property getting methods
    const std::vector<int>    &GetNetworkIds() const { return networkIds; };

    // Property getting methods
    QueryAttributes *GetQueryAtts();

    // Property getting methods
    QueryAttributes GetReturnAtts() { return returnAtts; } ; 

    QueryAttributes returnAtts; 

private:
    std::vector<int>  networkIds;
    QueryAttributes   queryAtts; 
};

#endif
