/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef PARENT_PROCESS_H
#define PARENT_PROCESS_H
#include <comm_exports.h>
#include <string>
#include <vector>

class Connection;

// ****************************************************************************
// Class: ParentProcess
//
// Purpose:
//   This class sets up communication with a parent process. It first
//   reads in the command line arguments that tell about the process
//   that launched it, then it sets up connections with that process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jul 21 14:59:26 PST 2000
//
// Modifications:
//   Jeremy Meredith, Tue Aug  8 13:49:42 PDT 2000
//   Changed it to allow more than one read/write socket.   
//
//   Brad Whitlock, Thu Apr 26 15:57:46 PST 2001
//   Added version stuff.
// 
//   Jeremy Meredith, Fri Apr 27 15:28:42 PDT 2001
//   Added fail bit.
//
//   Brad Whitlock, Thu Apr 11 15:42:42 PST 2002
//   Added a GetUserName method.
//
//   Brad Whitlock, Mon Mar 25 15:11:28 PST 2002
//   Made it use regular connections.
//
//   Brad Whitlock, Mon Dec 16 14:27:22 PST 2002
//   Added a security key.
//
//   Brad Whitlock, Tue Jul 29 10:54:29 PDT 2003
//   Added numRead and numWrite to the Connect method so we know how many
//   connections to create without having to pass the numbers on the command
//   line.
//
//   Brad Whitlock, Wed Jan 11 17:00:35 PST 2006
//   Added localUserName.
//
// ****************************************************************************

class COMM_API ParentProcess
{
public:
    ParentProcess();
    ~ParentProcess();
    void Connect(int numRead, int numWrite, int *argc, char **argv[],
                 bool createSockets, int failCode=0);
    const std::string &GetHostName() const;
    std::string        GetTheUserName() const;
    Connection  *GetReadConnection(int i=0) const;
    Connection  *GetWriteConnection(int i=0) const;
    void         SetVersion(const std::string &ver);
    const std::string &GetLocalUserName();
private:
    void ExchangeTypeRepresentations(int failCode=0);
    int  GetClientSocketDescriptor(int port);
    void GetHostInfo();
private:
    std::string      hostName;
    void             *hostInfo;
    Connection     **readConnections, **writeConnections;
    int              nReadConnections,  nWriteConnections;
    std::string      version;
    std::string      securityKey;
    std::string      localUserName;
};

#endif
