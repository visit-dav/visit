// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_FILE_SERVER_H
#define VIEWER_FILE_SERVER_H
#include <viewer_exports.h>
#include <ViewerFileServerInterface.h>

// ****************************************************************************
// Class: ViewerFileServer
//
// Purpose:
//   This class defines the interface that a "ViewerFileServer" must provide.
//
// Notes:      This class exposes the functions in ViewerFileServerImplementation
//             (formerly known as ViewerFileServer) as a 
//             ViewerFileServerInterface.
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:54:32 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWER_API ViewerFileServer : public ViewerFileServerInterface
{
public:
    ViewerFileServer();
    virtual ~ViewerFileServer();

public:
    // *************************************************************************
    // METADATA METHODS
    // ************************************************************************

    //
    // Use this method to give the file server new metadata for a simulation.
    //
    virtual void               SetSimulationMetaData(const std::string &host,
                                                   const std::string &filename,
                                                   const avtDatabaseMetaData&);

    virtual void               SetSimulationSILAtts(const std::string &host,
                                                   const std::string &filename,
                                                   const SILAttributes&);

    //
    // Use this method when you need metadata about the file but metadata
    // from any time state will suffice.
    //
    virtual const avtDatabaseMetaData *GetMetaData(const std::string &host,
                                           const std::string &filename,
                                           const bool forceReadAllCyclesAndTimes = false);

    //
    // Use this method when you need metadata about the file at
    // the specified time state.
    //
    virtual const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   const std::string &forcedFileType = "");
    virtual const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   bool forceReadAllCyclesAndTimes,
                                                   const std::string &forcedFileType = "");

    //
    // Use this method when you need to determine if the metadata for a
    // file varies over time.
    //
    virtual bool                       MetaDataIsInvariant(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState);
public:
    // *************************************************************************
    // SIL METHODS
    // ************************************************************************

    //
    // Use this method when you need the file's SIL but the SIL from any
    // time state will suffice.
    //
    virtual const avtSIL              *GetSIL(const std::string &host,
                                              const std::string &filename);

    //
    // Use this method when you need the file's SIL at the specified
    // time state.
    //
    virtual const avtSIL              *GetSILForState(const std::string &host,
                                              const std::string &filename,
                                              int timeState);

public:
    // *************************************************************************
    // FILE METHODS
    // ************************************************************************

    virtual std::string                ExpandedFileName(const std::string &host,
                                                const std::string &db);
    virtual void                       ExpandDatabaseName(std::string &hostDBName,
                                                  std::string &host,
                                                  std::string &dbName);
    virtual void                       ClearFile(const std::string &fullName,
                                                  bool forgetPlugin);
    virtual void                       CloseFile(const std::string &host,
                                                 const std::string &database = std::string());

    virtual bool                       IsDatabase(const std::string &fullname);
    virtual stringVector               GetOpenDatabases();
    virtual void                       SaveSession(const std::string &host,
                                                   const std::string &filename,
                                                   const std::string &sessionFile);
    virtual void                       RestoreSession(const std::string &host,
                                                      const std::string &filename,
                                                      std::string &sessionFile);

public:
    // *************************************************************************
    // LAUNCH/CONNECTION METHODS
    // ************************************************************************

    virtual void                       NoFaultStartServer(const std::string &host,
                                                          const stringVector &args);
    virtual void                       StartServer(const std::string &host,
                                                   const stringVector &args);
    virtual void                       CloseServer(const std::string &host,
                                                   bool close);
    virtual void                       CloseServers();
    virtual void                       ConnectServer(const std::string &host,
                                                     const stringVector &args);
    virtual void                       SendKeepAlives();

public:
    // *************************************************************************
    // PLUGIN METHODS
    // ************************************************************************

    virtual std::string                GetPluginErrors(const std::string &host);
    virtual void                       UpdateDBPluginInfo(const std::string &host);
    virtual void                       BroadcastUpdatedFileOpenOptions();

public:
    virtual void                       CreateNode(DataNode *);
    virtual void                       SetFromNode(DataNode *);
};

#endif
