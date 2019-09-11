// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SIMULATION_COMMAND_RPC_H
#define SIMULATION_COMMAND_RPC_H

#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>
#include <vector>

// ****************************************************************************
//  Class:  SimulationCommandRPC
//
//  Purpose:
//    Executes a simulation command
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 18, 2005
//
//  Modifications:
//    Brad Whitlock, Fri Mar 27 11:29:39 PDT 2009
//    I simplified it so it only accepts string arguments. I also changed it
//    to nonblocking.
//
// ****************************************************************************

class ENGINE_RPC_API SimulationCommandRPC : public NonBlockingRPC
{
  public:
    SimulationCommandRPC();
    virtual ~SimulationCommandRPC();

    virtual const std::string TypeName() const { return "SimulationCommandRPC"; }

    // Invokation methods
    void operator()(const std::string &command, const std::string&);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetCommand(const std::string&);
    void SetStringData(const std::string&);

    // Property getting methods
    std::string GetCommand()  const;
    std::string GetStringData() const;

  private:
    std::string command;
    std::string string_data;
};


#endif
