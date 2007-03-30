#ifndef RENDER_RPC_H
#define RENDER_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>

// ****************************************************************************
//  Class:  RenderRPC
//
//  Purpose:
//    Implements an RPC to render the plots on the specified networks 
//
//  Programmer:  Mark C. Miller 
//  Creation:    07Apr03
//
// ****************************************************************************
class ENGINE_RPC_API RenderRPC : public NonBlockingRPC
{
public:
    RenderRPC();
    virtual ~RenderRPC();

    // Invokation method
    void operator()(const intVector&,bool);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetIDs(const intVector&);
    void SetSendZBuffer(bool);

    // Property getting methods
    const intVector& GetIDs() const;
    bool GetSendZBuffer() const;

private:
    intVector ids;
    bool sendZBuffer;
};

#endif
