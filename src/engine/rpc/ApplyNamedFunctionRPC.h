#ifndef APPLY_NAMED_FUNCTION_RPC_H
#define APPLY_NAMED_FUNCTION_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
//  Class:  ApplyNamedFunctionRPC
//
//  Purpose:
//    Implements an RPC to instance a named function
//
//  Programmer:  Sean Ahern
//  Creation:    Thu Apr 18 17:16:23 PDT 2002
//
// ****************************************************************************

class ENGINE_RPC_API ApplyNamedFunctionRPC : public BlockingRPC
{
public:
    ApplyNamedFunctionRPC();
    virtual ~ApplyNamedFunctionRPC() { };

    // Invokation method
    void    operator() (const std::string & inname, const int innargs);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void    SetName(const std::string & inname)
        { name = inname; Select(0, (void *)&name); };
    void    SetNArgs(int innargs)
        { nargs = innargs; Select(1, (void *)&nargs); };

    // Property getting methods
    std::string GetName()const { return name; };
    int     GetNArgs() const { return nargs; };

private:
    std::string name;
    int     nargs;
};

#endif
