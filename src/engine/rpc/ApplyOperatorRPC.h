#ifndef APPLY_OPERATOR_RPC_H
#define APPLY_OPERATOR_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

class ApplyOperatorRPC;

// ****************************************************************************
//  Class:  PrepareOperatorRPC
//
//  Purpose:
//    Signals the name of the operator about to be created so that
//    the ApplyOperatorRPC has space to store the correct attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  2, 2001
//
// ****************************************************************************

class ENGINE_RPC_API PrepareOperatorRPC : public BlockingRPC
{
  public:
    PrepareOperatorRPC();
    ~PrepareOperatorRPC();

    void SetApplyOperatorRPC(ApplyOperatorRPC*);
    ApplyOperatorRPC *GetApplyOperatorRPC();

    void operator()(const std::string &n);
    void SelectAll();
    std::string GetID();
  private:
    ApplyOperatorRPC *applyOperatorRPC;
    std::string id;
};


// ****************************************************************************
//  Class:  ApplyOperatorRPC
//
//  Purpose:
//    Apply an operator.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  2, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 25 09:56:52 PDT 2002
//    Removed SetSocket.
//
// ****************************************************************************

class ENGINE_RPC_API ApplyOperatorRPC : public BlockingRPC
{
  public:
    ApplyOperatorRPC();
    virtual ~ApplyOperatorRPC();

    void operator()(const std::string&, const AttributeSubject*);

    virtual void SelectAll();

    std::string GetID();
    AttributeSubject *GetAtts();
    PrepareOperatorRPC &GetPrepareOperatorRPC();

    void SetAtts(AttributeSubject*);

    virtual void SetXfer(Xfer *x);

  private:
    AttributeSubject *atts;
    PrepareOperatorRPC prepareOperatorRPC;
};

#endif
