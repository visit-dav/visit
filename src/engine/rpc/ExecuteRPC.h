#ifndef EXECUTE_RPC_H
#define EXECUTE_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>

class ENGINE_RPC_API ExecuteRPC : public NonBlockingRPC
{
public:
    ExecuteRPC();
    virtual ~ExecuteRPC();

    // Invokation method
    void operator()(bool);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetRespondWithNull(bool);

    // Property getting methods
    bool GetRespondWithNull() const;

private:
    bool respondWithNullDataObject;

};

#endif
