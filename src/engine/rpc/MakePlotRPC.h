#ifndef MAKE_PLOT_RPC_H
#define MAKE_PLOT_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

class MakePlotRPC;

// ****************************************************************************
//  Class:  PreparePlotRPC
//
//  Purpose:
//    Signals the name of the plot about to be created so that
//    the MakePlotRPC has space to store the correct attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  2, 2001
//
// ****************************************************************************
class ENGINE_RPC_API PreparePlotRPC : public BlockingRPC
{
  public:
    PreparePlotRPC();
    ~PreparePlotRPC();

    void SetMakePlotRPC(MakePlotRPC*);
    MakePlotRPC *GetMakePlotRPC();

    void operator()(const std::string &n);
    void SelectAll();
    std::string GetID();
  private:
    MakePlotRPC *makePlotRPC;
    std::string id;
};


// ****************************************************************************
//  Class:  MakePlotRPC
//
//  Purpose:
//    Make a plot.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  2, 2001
//
//  Modifications:
//    Jeremy Meredith, Fri Nov  9 10:16:51 PST 2001
//    Added a NetworkID as a return type of the rpc.
//
//    Brad Whitlock, Mon Mar 25 09:56:24 PDT 2002
//    Removed SetSocket.
//
// ****************************************************************************

class ENGINE_RPC_API MakePlotRPC : public BlockingRPC
{
  public:
    struct ENGINE_RPC_API NetworkID : public AttributeSubject
    {
        int id;
      public:
        NetworkID();
        NetworkID(int id_);
        virtual ~NetworkID();
        virtual void SelectAll();
    };

  public:
    MakePlotRPC();
    virtual ~MakePlotRPC();

    int operator()(const std::string&, const AttributeSubject*);

    virtual void SelectAll();

    std::string GetID();
    AttributeSubject *GetAtts();
    PreparePlotRPC &GetPreparePlotRPC();

    void SetAtts(AttributeSubject*);

    virtual void SetXfer(Xfer *x);

  private:
    AttributeSubject *atts;
    PreparePlotRPC preparePlotRPC;
    NetworkID networkID;
};

#endif
