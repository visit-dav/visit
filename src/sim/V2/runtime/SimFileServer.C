// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
SimFileServer::SaveSession(const std::string &host, const std::string &filename, const std::string &sessionFile)
{
    // Does nothing.
}

void
SimFileServer::RestoreSession(const std::string &host, const std::string &filename, std::string &sessionFile)
{
    // Does nothing.
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

