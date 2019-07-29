// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    Brad Whitlock, Fri Dec  7 13:26:30 PST 2007
//    Added TypeName override.
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

    virtual const std::string TypeName() const;
  private:
    stringVector launchArgs;
    std::string  simHost;
    int          simPort;
    std::string  simSecurityKey;
};


#endif
