#ifndef PICK_RPC_H
#define PICK_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <PickAttributes.h>

// ****************************************************************************
//  Class:  PickRPC
//
//  Purpose:
//    Implements an RPC to 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 20, 2001
//
// ****************************************************************************
class ENGINE_RPC_API PickRPC : public BlockingRPC
{
  public:

    PickRPC();
    virtual ~PickRPC();

    // Invokation method
    PickAttributes operator()(const int, const PickAttributes*);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPickAtts(const PickAttributes*);
    void SetNetId(const int);

    // Property getting methods
    PickAttributes *GetPickAtts() ;
    int GetNetId();
   
    PickAttributes returnAtts;
private:
    PickAttributes pickAtts;
    int            netId;
};

#endif
