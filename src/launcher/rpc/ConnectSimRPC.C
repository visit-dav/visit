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

#include <ConnectSimRPC.h>

// ****************************************************************************
//  Method: ConnectSimRPC::ConnectSimRPC
//
//  Purpose: 
//    Constructor for the ConnectSimRPC class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

ConnectSimRPC::ConnectSimRPC() : BlockingRPC("s*sis"), launchArgs()
{
}

// ****************************************************************************
//  Method: ConnectSimRPC::~ConnectSimRPC
//
//  Purpose: 
//    Destructor for the ConnectSimRPC class.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

ConnectSimRPC::~ConnectSimRPC()
{
}

// ****************************************************************************
//  Method: ConnectSimRPC::operator()
//
//  Purpose: 
//    This is the invokation method for this function object. It copies the
//    arguments into local attributes and calls RPC's Execute method.
//
//  Arguments:
//    args : The arguments to launch the program.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
ConnectSimRPC::operator()(const stringVector &args,
                          const std::string  &host,
                          int port,
                          const std::string  &securityKey)
{
    launchArgs     = args;
    simHost        = host;
    simPort        = port;
    simSecurityKey = securityKey;
    Execute();
}

// ****************************************************************************
//  Method: ConnectSimRPC::SelectAll
//
//  Purpose: 
//    Gets the addresses of the local attributes so the attributes can be
//    transmitted over the network as AttributeGroups.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//    Jeremy Meredith, Mon May  9 14:39:44 PDT 2005
//    Added security key to simulation connection.
//
// ****************************************************************************

void
ConnectSimRPC::SelectAll()
{
    Select(0, (void *)&launchArgs);
    Select(1, (void *)&simHost);
    Select(2, (void *)&simPort);
    Select(3, (void *)&simSecurityKey);
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetLaunchArgs
//
//  Purpose: 
//    Returns the launch arguments vector.
//
//  Returns:    The launch arguments vector.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

const stringVector &
ConnectSimRPC::GetLaunchArgs() const
{
    return launchArgs;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimHost
//
//  Purpose: 
//    Returns the host name the simulation is listening on.
//
//  Returns:    The host name.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

const std::string &
ConnectSimRPC::GetSimHost() const
{
    return simHost;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimPort
//
//  Purpose: 
//    Returns the port number the simulation is listening on.
//
//  Returns:    The port number.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 23, 2004
//
//  Modifications:
//
// ****************************************************************************

int
ConnectSimRPC::GetSimPort() const
{
    return simPort;
}

// ****************************************************************************
//  Method: ConnectSimRPC::GetSimSecurityKey
//
//  Purpose: 
//    Returns the security key simulation is waiting for.
//
//  Returns:    The security key.
//
//  Programmer: Jeremy Meredith
//  Creation:   May  9, 2005
//
//  Modifications:
//
// ****************************************************************************

const std::string &
ConnectSimRPC::GetSimSecurityKey() const
{
    return simSecurityKey;
}

