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
// ****************************************************************************

class ENGINE_RPC_API SimulationCommandRPC : public BlockingRPC
{
  public:
    SimulationCommandRPC();
    virtual ~SimulationCommandRPC();

    // Invokation methods
    /*
    void operator()(const std::string &command);
    void operator()(const std::string &command, int);
    void operator()(const std::string &command, float);
    void operator()(const std::string &command, const std::string&);
    */
    void operator()(const std::string &command, int,float,const std::string&);

    // Property selection methods
    virtual void SelectAll();

    // Property setting methods
    void SetCommand(const std::string&);
    void SetIntData(int);
    void SetFloatData(float);
    void SetStringData(const std::string&);

    // Property getting methods
    std::string GetCommand()  const;
    int         GetIntData() const;
    float       GetFloatData() const;
    std::string GetStringData() const;

  private:
    std::string command;
    int         int_data;
    float       float_data;
    std::string string_data;
};


#endif
