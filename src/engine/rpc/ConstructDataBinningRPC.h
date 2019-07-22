// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CONSTRUCT_DATA_BINNING_RPC_H
#define CONSTRUCT_DATA_BINNING_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <ConstructDataBinningAttributes.h>

// ****************************************************************************
//  Class:  ConstructDataBinningRPC
//
//  Purpose:
//    Implements an RPC to construct a derived data function.
//
//  Programmer:  Hank Childs 
//  Creation:    February 13, 2006
//
//  Modifications:
//
//    Hank Childs, Sat Aug 21 14:35:47 PDT 2010
//    Rename DDF to DataBinning.
//
// ****************************************************************************
class ENGINE_RPC_API ConstructDataBinningRPC : public BlockingRPC
{
  public:
    ConstructDataBinningRPC();
    virtual ~ConstructDataBinningRPC();

    virtual const std::string TypeName() const { return "ConstructDataBinningRPC"; }

    // Invocation method
    void operator()(const int, const ConstructDataBinningAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetID(const int);
    void SetConstructDataBinningAtts(const ConstructDataBinningAttributes *);

    // Property getting methods
    int                     GetID() const;
    ConstructDataBinningAttributes *GetConstructDataBinningAtts();


  private:
    int                      id;
    ConstructDataBinningAttributes   constructDataBinningAtts; 
};

#endif
