// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added Save and Restore Session functions, for remote host functionality.
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

    virtual void                       SaveSession(const std::string &host,
                                                   const std::string &filename,
                                                   const std::string &sessionFile) = 0;
    virtual void                       RestoreSession(const std::string &host,
                                                      const std::string &filename,
                                                      std::string &sessionFile) = 0;

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
