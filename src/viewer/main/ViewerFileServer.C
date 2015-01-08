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

