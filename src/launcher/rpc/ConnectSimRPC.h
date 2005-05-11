#ifndef CONNECT_SIM_RPC_H
#define CONNECT_SIM_RPC_H
#include <vclrpc_exports.h>
#include <VisItRPC.h>
#include <vectortypes.h>

// ****************************************************************************
//  Class: ConnectSimRPC
//
//  Purpose:
//    This class encodes an RPC that tells the launcher to tell a simulation
//    to connect back to the viewer.
//
//  Notes:      
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:41:35 PDT 2005
//    Added security key.
//
// ****************************************************************************

class LAUNCHER_RPC_API ConnectSimRPC : public BlockingRPC
{
  public:
    ConnectSimRPC();
    virtual ~ConnectSimRPC();

    // Invokation method
    void operator()(const stringVector &args, const std::string&, int,
                    const std::string&);

    // Property selection methods
    virtual void SelectAll();

    // Methods to access private data.
    const stringVector &GetLaunchArgs() const;
    const std::string  &GetSimHost() const;
    int                 GetSimPort() const;
    const std::string  &GetSimSecurityKey() const;

  private:
    stringVector launchArgs;
    std::string  simHost;
    int          simPort;
    std::string  simSecurityKey;
};


#endif
