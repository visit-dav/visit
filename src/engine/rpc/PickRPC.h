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
//  Modifications:
//
//    Mark C. Miller, Tue Jan  4 10:23:19 PST 2005
//    Added winId
//
// ****************************************************************************
class ENGINE_RPC_API PickRPC : public BlockingRPC
{
  public:

    PickRPC();
    virtual ~PickRPC();

    // Invokation method
    PickAttributes operator()(const int, const PickAttributes*, const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetPickAtts(const PickAttributes*);
    void SetNetId(const int);
    void SetWinId(const int);

    // Property getting methods
    PickAttributes *GetPickAtts() ;
    int GetNetId();
    int GetWinId() const;
   
    PickAttributes returnAtts;
private:
    PickAttributes pickAtts;
    int            netId;
    int            winId;
};

#endif
