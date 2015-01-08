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
#include <SimFileServer.h>

#include <DataNode.h>
#include <FileFunctions.h>
#include <NetworkManager.h>
#include <SimEngine.h>
#include <avtDatabaseMetaData.h>
#include <avtSIL.h>

#include <algorithm>

// ****************************************************************************
// Method: SimFileServer::SimFileServer
//
// Purpose:
//   Constructor
//
// Arguments:
//   e : The sim engine to use.
//
// Returns:    
//
// Note:       This class implements the ViewerFileServerInterface API using
//             engine "guts" so we can have viewer code in libsim that uses
//             engine functionality.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 15:31:02 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimFileServer::SimFileServer(SimEngine *e) : ViewerFileServerInterface(), engine(e)
{
}

// ****************************************************************************
// Method: SimFileServer::~SimFileServer
//
// Purpose:
//   Destructor.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 18 15:31:59 PDT 2014
//
// Modifications:
//
// ****************************************************************************

SimFileServer::~SimFileServer()
{
}

void
SimFileServer::SetSimulationMetaData(const std::string &/*host*/,
                                     const std::string &filename,
                                     const avtDatabaseMetaData &md)
{
    // Normally when the viewer is connected to a simulation, it receives new
    // metadata asynchronously from the simulation. That metadata is stashed into
    // the viewer's file server so we can return it from the GetMetaData methods.
    // We obtain metadata directly from the network manager's db cache in this case
    // so this method is not needed.

    // Does nothing.
}

void
SimFileServer::SetSimulationSILAtts(const std::string &/*host*/,
                                    const std::string &filename,
                                    const SILAttributes &silAtts)
{
    // Normally when the viewer is connected to a simulation, it receives new
    // SILs asynchronously from the simulation. That SIL is stashed into
    // the viewer's file server so we can return it from the GetSIL methods.
    // We obtain SILs directly from the network manager's db cache in this case
    // so this method is not needed.

    // Does nothing.
}

//
// NOTE: All of these GetMetaData, GetSIL methods attempt to get a database that
//       has been added to the network manager's database cache, which normally
//       happens in StartNetwork. These metadata-finding routines are more likely
//       to happen before that StartNetwork is ever called. This means that we're
//       inconsistent with how databases are being read. We may want to separate
//       the stuff to set options and get the db from the cache into its own
//       method so we can be more consistent. Then again, it may not really be
//       an issue for simulations.
//

const avtDatabaseMetaData *
SimFileServer::GetMetaData(const std::string &/*host*/,
                           const std::string &filename,
                           const bool forceReadAllCyclesAndTimes)
{
    return engine->GetMetaData(filename);
}

const avtDatabaseMetaData *
SimFileServer::GetMetaDataForState(const std::string &/*host*/,
                                   const std::string &filename,
                                   int timeState,
                                   const std::string &forcedFileType)
{
    return engine->GetMetaDataForState(filename, timeState);
}

const avtDatabaseMetaData *
SimFileServer::GetMetaDataForState(const std::string &/*host*/,
                                   const std::string &filename,
                                   int timeState,
                                   bool forceReadAllCyclesAndTimes,
                                   const std::string &forcedFileType)
{
    return engine->GetMetaDataForState(filename, timeState);
}

bool
SimFileServer::MetaDataIsInvariant(const std::string &host,
                                   const std::string &filename,
                                   int timeState)
{
    const avtDatabaseMetaData *md = GetMetaDataForState(host, 
                                                        filename,
                                                        timeState,
                                                        std::string());

    return (md != NULL) && (!md->GetMustRepopulateOnStateChange());
}

const avtSIL *
SimFileServer::GetSIL(const std::string &/*host*/, const std::string &filename)
{
    return engine->GetSIL(filename);
}

const avtSIL *
SimFileServer::GetSILForState(const std::string &/*host*/,
                              const std::string &filename,
                              int timeState)
{
    return engine->GetSILForState(filename, timeState);
}

std::string
SimFileServer::ExpandedFileName(const std::string &/*host*/, const std::string &db)
{
    return FileFunctions::ExpandPath(db, 
               FileFunctions::GetCurrentWorkingDirectory());
}

void
SimFileServer::ExpandDatabaseName(std::string &hostDBName,
                                  std::string &host,
                                  std::string &db)
{
    // Split the host database name into host + db.
    FileFunctions::SplitHostDatabase(hostDBName, host, db);

    //
    // Expand the file name so it does not contain wildcards or
    // relatve paths.
    //
    db = FileFunctions::ExpandPath(db, 
             FileFunctions::GetCurrentWorkingDirectory());

    //
    // Compose the new filename.
    //
    hostDBName = FileFunctions::ComposeDatabaseName(host, db);
}

void
SimFileServer::ClearFile(const std::string &fullName, bool forgetPlugin)
{
    engine->GetNetMgr()->ClearNetworksWithDatabase(fullName);
}

void
SimFileServer::CloseFile(const std::string &/*host*/, const std::string &database)
{
    engine->GetNetMgr()->ClearNetworksWithDatabase(database);
}

bool
SimFileServer::IsDatabase(const std::string &fullname)
{
    stringVector dbs(GetOpenDatabases());
    return std::find(dbs.begin(), dbs.end(), fullname) != dbs.end();
}

stringVector
SimFileServer::GetOpenDatabases()
{
    return engine->GetNetMgr()->GetOpenDatabases();
}

void
SimFileServer::NoFaultStartServer(const std::string &/*host*/, const stringVector &args)
{
    // Does nothing.
}

void
SimFileServer::StartServer(const std::string &/*host*/, const stringVector &args)
{
    // Does nothing.
}

void
SimFileServer::CloseServer(const std::string &/*host*/, bool close)
{
    // Does nothing.
}

void
SimFileServer::CloseServers()
{
    // Does nothing.
}

void
SimFileServer::ConnectServer(const std::string &/*host*/, const stringVector &args)
{
    // Does nothing.
}

void
SimFileServer::SendKeepAlives()
{
    // Does nothing.
}

std::string
SimFileServer::GetPluginErrors(const std::string &host)
{
    // Any errors loading plugins? Batch them up and return them.

    return std::string();
}

void
SimFileServer::UpdateDBPluginInfo(const std::string &host)
{
    // NOTE: We could do the same thing as in MDServerConnection::GetDBPluginInfo.
    //       However, since this file server is for a sim, we may not care about
    //       exposing any read options for other database plugins.
}

void
SimFileServer::BroadcastUpdatedFileOpenOptions()
{
    // Sends new file open options 

    // GetMDServerMethods()->SetDefaultFileOpenOptions(*GetViewerState()->GetFileOpenOptions());
}

void
SimFileServer::CreateNode(DataNode *node)
{
    // Does nothing.
}

void
SimFileServer::SetFromNode(DataNode *node)
{
    // Does nothing.
}

