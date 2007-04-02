#include <SimulationCommandRPC.h>
#include <DebugStream.h>

SimulationCommandRPC::SimulationCommandRPC() : BlockingRPC("sifs")
{
    command="";
    int_data=0;
    float_data=0;
    string_data="";
}

SimulationCommandRPC::~SimulationCommandRPC()
{
}


void
SimulationCommandRPC::operator()(const std::string &c,
                                 int i,float f,const std::string &s)
{
    debug3 << "Executing SimulationCommandRPC"
           << "\n\t command='"<<c.c_str()<<"'"
           << "\n\t int_data="<<i
           << "\n\t float_data="<<f
           << "\n\t string_data='"<<s.c_str()<<"'"
           << endl;

    SetCommand(c);
    SetIntData(i);
    SetFloatData(f);
    SetStringData(s);

    Execute();
}

void
SimulationCommandRPC::SelectAll()
{
    Select(0, (void*)&command);
    Select(1, (void*)&int_data);
    Select(2, (void*)&float_data);
    Select(3, (void*)&string_data);
}

void
SimulationCommandRPC::SetCommand(const std::string &c)
{
    command = c;
}

void
SimulationCommandRPC::SetIntData(int i)
{
    int_data = i;
}

void
SimulationCommandRPC::SetFloatData(float f)
{
    float_data = f;
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

int
SimulationCommandRPC::GetIntData() const
{
    return int_data;
}

float     
SimulationCommandRPC::GetFloatData() const
{
    return float_data;
}

std::string
SimulationCommandRPC::GetStringData() const
{
    return string_data;
}
