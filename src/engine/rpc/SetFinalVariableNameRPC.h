#ifndef SET_FINAL_VARIABLE_NAME_RPC_H
#define SET_FINAL_VARIABLE_NAME_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>

// ****************************************************************************
//  Class:  ApplyNamedFunctionRPC
//
//  Purpose:
//    Implements an RPC to set the final variable name of a pipeline.
//
//  Programmer:  Sean Ahern
//  Creation:    Thu Jun 13 15:02:26 PDT 2002
//
// ****************************************************************************

class ENGINE_RPC_API SetFinalVariableNameRPC : public BlockingRPC
{
public:
    SetFinalVariableNameRPC();
    virtual ~SetFinalVariableNameRPC() { };

    // Invokation method
    void    operator() (const std::string & inname);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void    SetName(const std::string & inname)
        { name = inname; Select(0, (void *)&name); };

    // Property getting methods
    std::string GetName()const { return name; };

private:
    std::string name;
};

#endif
