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

#ifndef LAUNCHER_APPLICATION_H
#define LAUNCHER_APPLICATION_H
#include <ParentProcess.h>
#include <Xfer.h>
#include <QuitRPC.h>
#include <KeepAliveRPC.h>
#include <LaunchRPC.h>
#include <ConnectSimRPC.h>
#include <RPCExecutor.h>
#include <map>

// ****************************************************************************
// Class: LauncherApplication
//
// Purpose:
//   This class contains the launcher application that is responsible for
//   launching VisIt components.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 16:55:28 PST 2003
//
// Modifications:
//   Brad Whitlock, Fri Mar 12 10:35:29 PDT 2004
//   I added KeepAliveRPC.
//
//   Jeremy Meredith, Tue Mar 30 10:06:33 PST 2004
//   I added support for connecting to running simulations.
//
//   Jeremy Meredith, Mon May  9 16:09:06 PDT 2005
//   I added security protocols for simulations.
//
// ****************************************************************************

class LauncherApplication
{
public:
    static LauncherApplication *Instance();
    virtual ~LauncherApplication();
    void Execute(int *argc, char **argv[]);
    void LaunchProcess(const stringVector &launchArgs);
    void ConnectSimulation(const stringVector &launchArgs,
                           const std::string &simHost, int simPort,
                           const std::string &simSecurityKey);

protected:
    LauncherApplication();
    void ProcessArguments(int *argc, char **argv[]);
    void Connect(int *argc, char **argv[]);
    void MainLoop();
    bool ProcessInput();
    void TurnOnAlarm();
    void TurnOffAlarm();
    void TerminateConnectionRequest(int, char **);

    static void AlarmHandler(int);
    static void DeadChildHandler(int);
private:
    static LauncherApplication *instance;
    static std::map<int, bool>  childDied;

    ParentProcess               parent;
    Xfer                        xfer;

    QuitRPC                     quitRPC;
    KeepAliveRPC                keepAliveRPC;
    LaunchRPC                   launchRPC;
    ConnectSimRPC               connectSimRPC;

    RPCExecutor<QuitRPC>       *quitExecutor;
    RPCExecutor<KeepAliveRPC>  *keepAliveExecutor;
    RPCExecutor<LaunchRPC>     *launchExecutor;
    RPCExecutor<ConnectSimRPC> *connectSimExecutor;

    bool                        keepGoing;
    int                         timeout;
};

#endif
