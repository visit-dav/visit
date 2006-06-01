/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
