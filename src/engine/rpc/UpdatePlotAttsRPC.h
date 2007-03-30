#ifndef UPDATE_PLOT_ATTS_RPC_H
#define UPDATE_PLOT_ATTS_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>

class UpdatePlotAttsRPC;

// ****************************************************************************
//  Class:  PrepareUpdatePlotAttsRPC
//
//  Purpose:
//    Signals the name of the plot about to be created so that
//    the UpdatePlotAttsRPC has space to store the correct attributes.
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
// ****************************************************************************
class ENGINE_RPC_API PrepareUpdatePlotAttsRPC : public BlockingRPC
{
  public:
    PrepareUpdatePlotAttsRPC();
    ~PrepareUpdatePlotAttsRPC();

    void SetUpdatePlotAttsRPC(UpdatePlotAttsRPC*);
    UpdatePlotAttsRPC *GetUpdatePlotAttsRPC();

    void operator()(const std::string &n);
    void SelectAll();
    std::string GetID();
  private:
    std::string id;
    UpdatePlotAttsRPC *updatePlotAttsRPC;
};


// ****************************************************************************
//  Class:  UpdatePlotAttsRPC
//
//  Purpose:
//      Update plot attributs.
//
//  Programmer:  Hank Childs
//  Creation:    November 30, 2001
//
//  Modifications:
//    Brad Whitlock, Mon Mar 25 09:55:56 PDT 2002
//    Removed SetSocket.
//
// ****************************************************************************

class ENGINE_RPC_API UpdatePlotAttsRPC : public BlockingRPC
{
  public:
    UpdatePlotAttsRPC();
    virtual ~UpdatePlotAttsRPC();

    void operator()(const std::string&, int, const AttributeSubject*);

    virtual void SelectAll();

    std::string GetID();
    const AttributeSubject *GetAtts();
    int GetPlotIndex() { return index; };
    PrepareUpdatePlotAttsRPC &GetPrepareUpdatePlotAttsRPC();

    void SetAtts(AttributeSubject*);

    virtual void SetXfer(Xfer *x);

  private:
    AttributeSubject *atts;
    PrepareUpdatePlotAttsRPC prepareUpdatePlotAttsRPC;
    int index;
};

#endif
