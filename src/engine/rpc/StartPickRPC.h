#ifndef STARTPICK_RPC_H
#define STARTPICK_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>

// ****************************************************************************
//  Class:  StartPickRPC
//
//  Purpose:
//    Implements an RPC to either start or stop pick mode (depending on value
//    of startFlag. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    November 26, 2001
//
// ****************************************************************************
class ENGINE_RPC_API StartPickRPC : public BlockingRPC
{
  public:

    StartPickRPC();
    virtual ~StartPickRPC();

    // Invokation method
    void operator()(const bool, const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetStartFlag(const bool);
    void SetNetId(const int);

    // Property getting methods
    bool GetStartFlag();
    int  GetNetId();
   
private:
    int            netId;
    bool           startFlag;
};

#endif
