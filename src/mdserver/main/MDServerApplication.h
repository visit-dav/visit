/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef MDSERVER_APPLICATION_H
#define MDSERVER_APPLICATION_H
#include <vector>
#include <string>

class MDServerConnection;

// ****************************************************************************
// Class: MDServerApplication
//
// Purpose:
//   This class manages several MDServerConnections which are connections to
//   programs that drive the MDServer.
//
// Notes:     This class follows the singleton design pattern. 
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 16:44:32 PST 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 15 12:47:22 PDT 2001
//   Added a method to remove dead client connections.
//
//   Brad Whitlock, Fri Apr 18 14:57:17 PST 2003
//   I added a debug method that reads the file list and quits.
//
//   Jeremy Meredith, Wed Dec 31 14:12:12 PST 2003
//   Added code to keep track of the client host.  This lets us guess
//   host names better.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added method to set cycle number regular expression
// ****************************************************************************

class MDServerApplication
{
    typedef std::vector<MDServerConnection *> MDServerConnectionVector;
public:
    ~MDServerApplication();
    void AddConnection(int *argc, char **argv[]);
    void Execute();
    void ExecuteDebug();
    void SetTimeout(long t) {timeout = t;}
    void SetCycleFromFilenameRegex(const char *cfnre);

    static MDServerApplication *Instance();
    static void AlarmHandler(int signal);
private:
    MDServerApplication();
    void DisconnectDeadConnections(const std::vector<int> &deadList);
private:
    static MDServerApplication *instance;
    std::string                 clientHost;
    MDServerConnectionVector    clients;
    bool                        keepGoing;
    long                        timeout;
};

#endif
