#ifndef PROC_INFO_RPC_H 
#define PROC_INFO_RPC_H 
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <ProcessAttributes.h>

// ****************************************************************************
//  Class:  ProcInfoRPC
//
//  Purpose:
//    Implements an RPC to get process information 
//
//  Programmer:  Mark C. Miller 
//  Creation:    November 15, 2004 
//
// ****************************************************************************

class ENGINE_RPC_API ProcInfoRPC : public NonBlockingRPC
{
public:
    ProcInfoRPC();
    virtual ~ProcInfoRPC() { };

    // Invokation method
    void operator()() { Execute(); };

    void SelectAll() { };

    // Property getting methods
    ProcessAttributes GetReturnAtts() { return returnAtts; } ; 

    ProcessAttributes returnAtts; 
};

#endif
