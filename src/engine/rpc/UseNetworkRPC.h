#ifndef USE_NETWORK_RPC_H
#define USE_NETWORK_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>

// ****************************************************************************
//  Class:  UseNetworkRPC
//
//  Purpose:
//    Implements an RPC to signal reusing of an existing network.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2001
//
// ****************************************************************************
class ENGINE_RPC_API UseNetworkRPC : public BlockingRPC
{
public:
    UseNetworkRPC();
    virtual ~UseNetworkRPC();

    // Invokation method
    void operator()(int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetID(int);

    // Property getting methods
    int   GetID() const;

private:
    int id;
};

#endif
