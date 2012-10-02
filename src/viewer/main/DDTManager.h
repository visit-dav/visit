/*****************************************************************************
*
* Copyright (c) 2011, Allinea
* All rights reserved.
*
* This file has been contributed to the VisIt project, which is
* Copyright (c) Lawrence Livermore National Security, LLC. For  details, see
* https://visit.llnl.gov/.  The full copyright notice is contained in the 
* file COPYRIGHT located at the root of the VisIt distribution or at 
* http://www.llnl.gov/visit/copyright.html.
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

#ifndef DDTMANAGER_H
#define DDTMANAGER_H

#include <ViewerBase.h>
#include <string>

// Command strings to send to ddtsim
#define DDTSIM_CMD_STOP "stop"
#define DDTSIM_CMD_PLAY "run"
#define DDTSIM_CMD_STEP "step"

class DDTSession;
class ViewerWindow;

// ****************************************************************************
// Class: DDTManager
//
// Purpose:
//   This singleton class manages VisIt's interaction with DDT and provides
//   helper methods for detecting ddtsim-based simulations.
//
// Notes:
//   Currently only supports connection to only one DDT application at a time
//
// Programmer: Jonathan Byrd (Allinea Software)
// Creation:   Sun Dec 18, 2011
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API DDTManager : public ViewerBase
{
    Q_OBJECT
public:
    static DDTManager* getInstance();
    static bool isDatabaseDDTSim(std::string db);
    static bool isDDTSim(ViewerWindow*);

    DDTSession* getSession();
    DDTSession* getSessionNC();
    void disconnect();

public slots:
    DDTSession* makeConnection();
    void statusChanged();

private:
    DDTManager();
    void updateWindowInfo();
    static DDTManager* instance;

    DDTSession *mSession;
};

#endif // DDTMANAGER_H
