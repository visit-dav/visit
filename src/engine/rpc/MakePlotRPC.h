// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MAKE_PLOT_RPC_H
#define MAKE_PLOT_RPC_H
#include <engine_rpc_exports.h>

#include <VisItRPC.h>
#include <string>
#include <vector>

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
//  Modifications:
//    Brad Whitlock, Wed Mar 21 22:48:12 PST 2007
//    Added name.
//
// ****************************************************************************
class ENGINE_RPC_API PreparePlotRPC : public BlockingRPC
{
  public:
    PreparePlotRPC();
    ~PreparePlotRPC();

    virtual const std::string TypeName() const { return "PreparePlotRPC"; }

    void SetMakePlotRPC(MakePlotRPC*);
    MakePlotRPC *GetMakePlotRPC();

    void operator()(const std::string &plotName, const std::string &pluginID);
    void SelectAll();
    std::string GetID();
    std::string GetName();
  private:
    MakePlotRPC *makePlotRPC;
    std::string id;
    std::string name;
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
//    Jeremy Meredith, Fri Mar 19 14:56:55 PST 2004
//    Modified the rpc to pass data extents.
//
//    Mark C. Miller, Wed Dec 15 21:56:26 PST 2004
//    Added windowID
//
//    Brad Whitlock, Wed Mar 21 22:46:44 PST 2007
//    Added plotName arg to operator().
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

        virtual const std::string TypeName() const
            { return "MakePlotRPC::NetworkID"; }
    };

  public:
    MakePlotRPC();
    virtual ~MakePlotRPC();

    virtual const std::string TypeName() const { return "MakePlotRPC"; }

    int operator()(const std::string &, const std::string&,
                   const AttributeSubject*,
                   const std::vector<double> &, int);

    virtual void SelectAll();

    std::string GetName();
    std::string GetID();
    AttributeSubject *GetAtts();
    const std::vector<double> &GetDataExtents() const;
    PreparePlotRPC &GetPreparePlotRPC();
    int GetWindowID() const;

    void SetAtts(AttributeSubject*);
    void SetDataExtents(const std::vector<double> &);
    void SetWindowID(int id);

    virtual void SetXfer(Xfer *x);

  private:
    AttributeSubject *atts;
    std::vector<double> dataExtents;
    PreparePlotRPC preparePlotRPC;
    NetworkID networkID;
    int       windowID;
};

#endif
