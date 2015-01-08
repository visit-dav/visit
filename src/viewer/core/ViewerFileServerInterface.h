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

#ifndef VIEWER_FILE_SERVER_INTERFACE_H
#define VIEWER_FILE_SERVER_INTERFACE_H
#include <ViewerBase.h>
#include <string>
#include <vectortypes.h>
#include <avtTypes.h>

class avtDatabaseMetaData;
class avtSIL;
class DataNode;
class SILAttributes;

// ****************************************************************************
// Class: ViewerFileServerInterface
//
// Purpose:
//   This class defines the interface that a "ViewerFileServer" must provide.
//
// Notes:      Objects that provide this interface can provide file server
//             services to the rest of the viewer. We'll have one implementation
//             for the viewer but could provide alternate implementations if
//             this code is used elsewhere (like in the engine).
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:54:32 PDT 2014

//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerFileServerInterface : public ViewerBase
{
public:
    static const int ANY_STATE;

    ViewerFileServerInterface();
    virtual ~ViewerFileServerInterface();

public:
    // *************************************************************************
    // METADATA METHODS
    // ************************************************************************

    //
    // Use this method to give the file server new metadata for a simulation.
    //
    virtual void               SetSimulationMetaData(const std::string &host,
                                                   const std::string &filename,
                                                   const avtDatabaseMetaData&) = 0;

    virtual void               SetSimulationSILAtts(const std::string &host,
                                                   const std::string &filename,
                                                   const SILAttributes&) = 0;

    //
    // Use this method when you need metadata about the file but metadata
    // from any time state will suffice.
    //
    virtual const avtDatabaseMetaData *GetMetaData(const std::string &host,
                                           const std::string &filename,
                                           const bool forceReadAllCyclesAndTimes = false) = 0;

    //
    // Use this method when you need metadata about the file at
    // the specified time state.
    //
    virtual const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   const std::string &forcedFileType = "") = 0;
    virtual const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   bool forceReadAllCyclesAndTimes,
                                                   const std::string &forcedFileType = "") = 0;

    //
    // Use this method when you need to determine if the metadata for a
    // file varies over time.
    //
    virtual bool                       MetaDataIsInvariant(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState) = 0;
public:
    // *************************************************************************
    // SIL METHODS
    // ************************************************************************

    //
    // Use this method when you need the file's SIL but the SIL from any
    // time state will suffice.
    //
    virtual const avtSIL              *GetSIL(const std::string &host,
                                              const std::string &filename) = 0;

    //
    // Use this method when you need the file's SIL at the specified
    // time state.
    //
    virtual const avtSIL              *GetSILForState(const std::string &host,
                                              const std::string &filename,
                                              int timeState) = 0;

public:
    // *************************************************************************
    // FILE METHODS
    // ************************************************************************

    virtual std::string                ExpandedFileName(const std::string &host,
                                                const std::string &db) = 0;
    virtual void                       ExpandDatabaseName(std::string &hostDBName,
                                                  std::string &host,
                                                  std::string &dbName) = 0;
    virtual void                       ClearFile(const std::string &fullName,
                                                  bool forgetPlugin) = 0;
    virtual void                       CloseFile(const std::string &host,
                                                 const std::string &database = std::string()) = 0;

    virtual bool                       IsDatabase(const std::string &fullname) = 0;
    virtual stringVector               GetOpenDatabases() = 0;

public:
    // *************************************************************************
    // LAUNCH/CONNECTION METHODS
    // ************************************************************************

    virtual void                       NoFaultStartServer(const std::string &host,
                                                          const stringVector &args) = 0;
    virtual void                       StartServer(const std::string &host,
                                                   const stringVector &args) = 0;
    virtual void                       CloseServer(const std::string &host,
                                                   bool close) = 0;
    virtual void                       CloseServers() = 0;
    virtual void                       ConnectServer(const std::string &host,
                                                     const stringVector &args) = 0;
    virtual void                       SendKeepAlives() = 0;

public:
    // *************************************************************************
    // PLUGIN METHODS
    // ************************************************************************

    virtual std::string                GetPluginErrors(const std::string &host) = 0;
    virtual void                       UpdateDBPluginInfo(const std::string &host) = 0;
    virtual void                       BroadcastUpdatedFileOpenOptions() = 0;

public:
    virtual void                       CreateNode(DataNode *) = 0;
    virtual void                       SetFromNode(DataNode *) = 0;
};

#endif
