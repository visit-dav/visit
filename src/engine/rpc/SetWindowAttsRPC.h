#ifndef SET_WINDOW_ATTS_RPC_H
#define SET_WINDOW_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <WindowAttributes.h>

// ****************************************************************************
//  Class:  SetWindowAttsRPC
//
//  Purpose:
//    Implements an RPC to set the current window attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November  7, 2001
//
// ****************************************************************************
class ENGINE_RPC_API SetWindowAttsRPC : public BlockingRPC
{
public:
    SetWindowAttsRPC();
    virtual ~SetWindowAttsRPC();

    // Invokation method
    void operator()(const WindowAttributes*);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetWindowAtts(const WindowAttributes*);

    // Property getting methods
    const WindowAttributes &GetWindowAtts() const;

private:
    WindowAttributes win;
};

#endif
