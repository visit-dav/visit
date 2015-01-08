/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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

#ifndef MDSERVER_STATE_H
#define MDSERVER_STATE_H

#include <mdsrpc_exports.h>

#include <GetDirectoryRPC.h>
#include <ChangeDirectoryRPC.h>
#include <GetFileListRPC.h>
#include <GetMetaDataRPC.h>
#include <GetSILRPC.h>
#include <ConnectRPC.h>
#include <CreateGroupListRPC.h>
#include <ExpandPathRPC.h>
#include <CloseDatabaseRPC.h>
#include <LoadPluginsRPC.h>
#include <GetPluginErrorsRPC.h>
#include <GetDBPluginInfoRPC.h>
#include <SetMFileOpenOptionsRPC.h>

#include <maptypes.h>
#include <vectortypes.h>

class Xfer;
class VisItRPC;

class MDSERVER_RPC_API MDServerState
{
public:
    MDServerState();
    ~MDServerState();
    size_t                     GetNumStateObjects();
    VisItRPC*                  GetStateObject(int i);

    void                       DetermineSeparator();
    char                       separator;

    void SetupComponentRPCs(Xfer* xfer);


    GetDirectoryRPC&            GetGetDirectoryRPC() { return getDirectoryRPC; }
    ChangeDirectoryRPC&         GetChangeDirectoryRPC() { return changeDirectoryRPC; }
    GetFileListRPC&             GetGetFileListRPC() { return getFileListRPC; }
    GetMetaDataRPC&             GetGetMetaDataRPC() { return getMetaDataRPC; }
    GetSILRPC&                  GetGetSILRPC() { return getSILRPC; }
    ConnectRPC&                 GetConnectRPC() { return connectRPC; }
    CreateGroupListRPC&         GetCreateGroupListRPC() { return createGroupListRPC; }
    ExpandPathRPC&              GetExpandPathRPC() { return expandPathRPC; }
    CloseDatabaseRPC&           GetCloseDatabaseRPC() { return closeDatabaseRPC; }
    LoadPluginsRPC&             GetLoadPluginsRPC() { return loadPluginsRPC; }
    GetPluginErrorsRPC&         GetGetPluginErrorsRPC() { return getPluginErrorsRPC; }
    GetDBPluginInfoRPC&         GetGetDBPluginInfoRPC() { return getDBPluginInfoRPC; }
    SetMFileOpenOptionsRPC&     GetSetMFileOpenOptionsRPC() { return setMFileOpenOptionsRPC; }

private:
    GetDirectoryRPC            getDirectoryRPC;
    ChangeDirectoryRPC         changeDirectoryRPC;
    GetFileListRPC             getFileListRPC;
    GetMetaDataRPC             getMetaDataRPC;
    GetSILRPC                  getSILRPC;
    ConnectRPC                 connectRPC;
    CreateGroupListRPC         createGroupListRPC;
    ExpandPathRPC              expandPathRPC;
    CloseDatabaseRPC           closeDatabaseRPC;
    LoadPluginsRPC             loadPluginsRPC;
    GetPluginErrorsRPC         getPluginErrorsRPC;
    GetDBPluginInfoRPC         getDBPluginInfoRPC;
    SetMFileOpenOptionsRPC     setMFileOpenOptionsRPC;

    std::vector<VisItRPC*> objVector;
    friend class MDServerMethods;
};

#endif
