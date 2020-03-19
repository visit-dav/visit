// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef RELEASE_DATA_RPC_H
#define RELEASE_DATA_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>

// ****************************************************************************
//  Class:  ReleaseDataRPC
//
//  Purpose:
//    Implements an RPC to signal an existing network to release its data.
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    September 18, 2002 
//
// ****************************************************************************
class ENGINE_RPC_API ReleaseDataRPC : public BlockingRPC
{
public:
    ReleaseDataRPC();
    virtual ~ReleaseDataRPC();

    virtual const std::string TypeName() const { return "ReleaseDataRPC"; }

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
