// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <ViewerFileServer.h>
#include <ViewerFileServerImplementation.h>

#include <DataNode.h>

#define IMPL()    ViewerFileServerImplementation::Instance()

ViewerFileServer::ViewerFileServer() : ViewerFileServerInterface()
{
}

ViewerFileServer::~ViewerFileServer()
{
    delete ViewerFileServerImplementation::Instance();
}

void
ViewerFileServer::SetSimulationMetaData(const std::string &host,
                                        const std::string &filename,
                                        const avtDatabaseMetaData &md)
{
    IMPL()->SetSimulationMetaData(host, filename, md);
}

void
ViewerFileServer::SetSimulationSILAtts(const std::string &host,
                                       const std::string &filename,
                                       const SILAttributes &silAtts)
{
    IMPL()->SetSimulationSILAtts(host, filename, silAtts);
}

const avtDatabaseMetaData *
ViewerFileServer::GetMetaData(const std::string &host,
                              const std::string &filename,
                              const bool forceReadAllCyclesAndTimes)
{
    return IMPL()->GetMetaData(host, filename, forceReadAllCyclesAndTimes);
}

const avtDatabaseMetaData *
ViewerFileServer::GetMetaDataForState(const std::string &host,
                                      const std::string &filename,
                                      int timeState,
                                      const std::string &forcedFileType)
{
    return IMPL()->GetMetaDataForState(host, filename, timeState, forcedFileType);
}

const avtDatabaseMetaData *
ViewerFileServer::GetMetaDataForState(const std::string &host,
                                      const std::string &filename,
                                      int timeState,
                                      bool forceReadAllCyclesAndTimes,
                                      const std::string &forcedFileType)
{
    return IMPL()->GetMetaDataForState(host, filename, timeState, 
                                       forceReadAllCyclesAndTimes, 
                                       forcedFileType);
}

bool
ViewerFileServer::MetaDataIsInvariant(const std::string &host,
                                      const std::string &filename,
                                      int timeState)
{
    return IMPL()->MetaDataIsInvariant(host, filename, timeState);
}

const avtSIL *
ViewerFileServer::GetSIL(const std::string &host, const std::string &filename)
{
    return IMPL()->GetSIL(host, filename);
}

const avtSIL *
ViewerFileServer::GetSILForState(const std::string &host,
                                 const std::string &filename,
                                 int timeState)
{
    return IMPL()->GetSILForState(host, filename, timeState);
}

std::string
ViewerFileServer::ExpandedFileName(const std::string &host, const std::string &db)
{
    return IMPL()->ExpandedFileName(host, db);
}

void
ViewerFileServer::ExpandDatabaseName(std::string &hostDBName,
                                     std::string &host,
                                     std::string &dbName)
{
    IMPL()->ExpandDatabaseName(hostDBName, host, dbName);
}

void
ViewerFileServer::ClearFile(const std::string &fullName, bool forgetPlugin)
{
    IMPL()->ClearFile(fullName, forgetPlugin);
}

void
ViewerFileServer::CloseFile(const std::string &host, const std::string &database)
{
    IMPL()->CloseFile(host, database);
}

bool
ViewerFileServer::IsDatabase(const std::string &fullname)
{
    return IMPL()->IsDatabase(fullname);
}

stringVector
ViewerFileServer::GetOpenDatabases()
{
    return IMPL()->GetOpenDatabases();
}

void
ViewerFileServer::SaveSession(const std::string &host, const std::string &filename, const std::string &sessionFile)
{
    IMPL()->SaveSession(host, filename, sessionFile);
}

void
ViewerFileServer::RestoreSession(const std::string &host, const std::string &filename, std::string &sessionFile)
{
    IMPL()->RestoreSession(host, filename, sessionFile);
}

void
ViewerFileServer::NoFaultStartServer(const std::string &host, const stringVector &args)
{
    IMPL()->NoFaultStartServer(host, args);
}

void
ViewerFileServer::StartServer(const std::string &host, const stringVector &args)
{
    IMPL()->StartServer(host, args);
}

void
ViewerFileServer::CloseServer(const std::string &host, bool close)
{
    IMPL()->CloseServer(host, close);
}

void
ViewerFileServer::CloseServers()
{
    IMPL()->CloseServers();
}

void
ViewerFileServer::ConnectServer(const std::string &host, const stringVector &args)
{
    IMPL()->ConnectServer(host, args);
}

void
ViewerFileServer::SendKeepAlives()
{
    IMPL()->SendKeepAlives();
}

std::string
ViewerFileServer::GetPluginErrors(const std::string &host)
{
    return IMPL()->GetPluginErrors(host);
}

void
ViewerFileServer::UpdateDBPluginInfo(const std::string &host)
{
    IMPL()->UpdateDBPluginInfo(host);
}

void
ViewerFileServer::BroadcastUpdatedFileOpenOptions()
{
    IMPL()->BroadcastUpdatedFileOpenOptions();
}

void
ViewerFileServer::CreateNode(DataNode *node)
{
    IMPL()->CreateNode(node);
}

void
ViewerFileServer::SetFromNode(DataNode *node)
{
    IMPL()->SetFromNode(node);
}

