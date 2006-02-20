#ifndef CONSTRUCT_DDF_RPC_H
#define CONSTRUCT_DDF_RPC_H

#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <ConstructDDFAttributes.h>

// ****************************************************************************
//  Class:  ConstructDDFRPC
//
//  Purpose:
//    Implements an RPC to construct a derived data function.
//
//  Programmer:  Hank Childs 
//  Creation:    February 13, 2006
//
// ****************************************************************************
class ENGINE_RPC_API ConstructDDFRPC : public BlockingRPC
{
  public:
    ConstructDDFRPC();
    virtual ~ConstructDDFRPC();

    // Invocation method
    void operator()(const int, const ConstructDDFAttributes *);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetID(const int);
    void SetConstructDDFAtts(const ConstructDDFAttributes *);

    // Property getting methods
    int                     GetID() const;
    ConstructDDFAttributes *GetConstructDDFAtts();


  private:
    int                      id;
    ConstructDDFAttributes   constructDDFAtts; 
};

#endif
