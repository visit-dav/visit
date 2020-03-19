// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//  Modifications:
//    Kathleen Bonnell, Wed Jun  2 09:45:23 PDT 2004
//    Added forZones.
//
// ****************************************************************************
class ENGINE_RPC_API StartPickRPC : public BlockingRPC
{
  public:

    StartPickRPC();
    virtual ~StartPickRPC();

    virtual const std::string TypeName() const { return "StartPickRPC"; }

    // Invokation method
    void operator()(const bool, const bool, const int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetStartFlag(const bool);
    void SetNetId(const int);
    void SetForZones(const bool);

    // Property getting methods
    bool GetStartFlag();
    int  GetNetId();
    bool GetForZones();
   
private:
    int            netId;
    bool           startFlag;
    bool           forZones;
};

#endif
