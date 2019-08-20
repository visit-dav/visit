// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef STARTQUERY_RPC_H
#define STARTQUERY_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>

// ****************************************************************************
//  Class:  StartQueryRPC
//
//  Purpose:
//    Implements an RPC to either start or stop query mode (depending on value
//    of startFlag. 
//
//  Programmer:  Hank Childs 
//  Creation:    February 28, 2005
//
// ****************************************************************************
class ENGINE_RPC_API StartQueryRPC : public BlockingRPC
{
  public:

    StartQueryRPC();
    virtual ~StartQueryRPC();

    virtual const std::string TypeName() const { return "StartQueryRPC"; }

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
