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
//  Modifications:
//
//    Mark C. Miller, Mon Mar 29 14:11:42 PST 2004 
//    Added bool controlling 3D annotations
//
//    Mark C. Miller, Wed Oct  6 18:12:29 PDT 2004
//    Changed 3D annotation flag to integer mode
// ****************************************************************************
class ENGINE_RPC_API RenderRPC : public NonBlockingRPC
{
public:
    RenderRPC();
    virtual ~RenderRPC();

    // Invokation method
    void operator()(const intVector&,bool,int);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetIDs(const intVector&);
    void SetSendZBuffer(bool);
    void SetAnnotMode(int);

    // Property getting methods
    const intVector& GetIDs() const;
    bool GetSendZBuffer() const;
    int  GetAnnotMode() const;

private:
    intVector ids;
    bool sendZBuffer;
    int annotMode;
};

#endif
