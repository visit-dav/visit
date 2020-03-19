// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <SimulationCommandRPC.h>
#include <DebugStream.h>

SimulationCommandRPC::SimulationCommandRPC() : NonBlockingRPC("ss")
{
    command="";
    string_data="";
}

SimulationCommandRPC::~SimulationCommandRPC()
{
}


void
SimulationCommandRPC::operator()(const std::string &c,
                                 const std::string &s)
{
    debug3 << "Executing SimulationCommandRPC"
           << "\n\t command='"<<c.c_str()<<"'"
           << "\n\t string_data='"<<s.c_str()<<"'"
           << endl;

    SetCommand(c);
    SetStringData(s);

    Execute();
}

void
SimulationCommandRPC::SelectAll()
{
    Select(0, (void*)&command);
    Select(1, (void*)&string_data);
}

void
SimulationCommandRPC::SetCommand(const std::string &c)
{
    command = c;
}

void
SimulationCommandRPC::SetStringData(const std::string &s)
{
    string_data = s;
}

std::string
SimulationCommandRPC::GetCommand() const
{
    return command;
}

std::string
SimulationCommandRPC::GetStringData() const
{
    return string_data;
}
