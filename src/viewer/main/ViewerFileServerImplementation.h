// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_FILE_SERVER_IMPLEMENTATION_H
#define VIEWER_FILE_SERVER_IMPLEMENTATION_H
#include <viewer_exports.h>
#include <ViewerServerManager.h>
#include <map>
#include <string>
#include <vectortypes.h>
#include <maptypes.h>
#include <avtTypes.h>

class avtDatabaseMetaData;
class avtSIL;
class DataNode;
class SILAttributes;

// ****************************************************************************
// Class: ViewerFileServerImplementation
//
// Purpose:
//   Launches metadata servers and gets metadata from them.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 16 15:39:29 PST 2000
//
// Modifications:
//    Jeremy Meredith, Fri Apr 20 10:34:23 PDT 2001
//    Added ability to pass other arguments to the server when starting.
//
//    Brad Whitlock, Tue Apr 24 17:10:44 PST 2001
//    Added CloseServer method.
//
//    Brad Whitlock, Thu Apr 26 15:54:10 PST 2001
//    Added TerminateConnectionRequest method.
//
//    Jeremy Meredith, Fri Apr 27 15:45:45 PDT 2001
//    Added fail bit to TerminateConnectionRequest.
//
//    Hank Childs, Fri Jun 15 11:39:46 PDT 2001
//    Added GetSIL.
//
//    Brad Whitlock, Thu Feb 7 15:16:19 PST 2002
//    Made it store avtSIL instead of avtSILAttributes.
//
//    Brad Whitlock, Tue Feb 12 15:03:26 PST 2002
//    Added a method that can be used to validate filenames on the mdserver.
//
//    Brad Whitlock, Fri May 3 16:15:20 PST 2002
//    Made it inherit from ViewerServerManager.
//
//    Brad Whitlock, Mon Jul 29 15:23:34 PST 2002
//    I added a method to clear out file metadata and another method to
//    close the open file.
//
//    Brad Whitlock, Thu Dec 26 15:42:10 PST 2002
//    I added support for security checking.
//
//    Brad Whitlock, Mon Jan 13 09:00:13 PDT 2003
//    I added support for explicitly launching an mdserver with arguments.
//
//    Brad Whitlock, Fri Feb 28 07:38:47 PDT 2003
//    I added another ServerMap to contain a list of servers that have failed.
//
//    Brad Whitlock, Wed May 14 11:46:36 PDT 2003
//    I added the ability to request the metadata or the SIL for a specific
//    time state by adding a default argument to the GetMetaData and GetSIL
//    methods. The default timeState (-1) tells the mdserver to return
//    the metadata for any time state in the database. If it is some other
//    value, it tells the mdserver to return metadata for a specific time state.
//
//    Brad Whitlock, Mon Jun 16 13:19:54 PST 2003
//    I made the method to terminate connections be more general so other
//    types of connections can be used.
//
//    Mark C. Miller, Wed Oct  8 23:46:25 PDT 2003
//    Added methods to query a given database's MetaData or SIL for invariance
//
//    Brad Whitlock, Fri Jan 23 09:37:54 PDT 2004
//    I added state objects for the for database correlations.
//
//    Brad Whitlock, Fri Mar 12 12:04:53 PDT 2004
//    I added SendKeepAlives.
//
//    Brad Whitlock, Fri Jan 30 22:54:24 PST 2004
//    I added a method to create database correlations. I also added a private
//    method for splitting map keys for the metadata and SIL maps. I removed
//    the optional time state argument from GetMetaData and GetSIL and 
//    added the GetMetaDataForState and GetSILForState methods for when a
//    time state is required. I added CreateNode, SetFromNode,
//    DetermineVarType, GetMetaDataForState, GetSILForState.
//
//    Brad Whitlock, Wed Mar 31 09:52:59 PDT 2004
//    I added the ExpansionRequired method.
//
//    Brad Whitlock, Tue Apr 13 23:15:22 PST 2004
//    I added methods to query if the user has ever declined to create a 
//    database correlation for a given set of files.
//
//    Brad Whitlock, Tue Jul 27 10:39:34 PDT 2004
//    I added UpdateDatabaseCorrelation.
//
//    Jeremy Meredith, Wed Aug 25 11:47:37 PDT 2004
//    Added methods to poke new metadata (and sil) for a simulation.
//
//    Brad Whitlock, Fri Feb 4 09:57:49 PDT 2005
//    Overloaded CloseFile.
//
//    Jeremy Meredith, Tue Feb  8 08:56:55 PST 2005
//    Added the ability to query for errors detected during plugin
//    initialization.
//
//    Brad Whitlock, Fri Feb 18 10:06:02 PDT 2005
//    Added some convenience methods for expressions.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg, forceReadAllCyclesAndTimes, to GetMetaData methods
//
//    Hank Childs, Wed May 25 10:48:50 PDT 2005
//    Added DBPluginInfo.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added methods to Set/Get tryHarderCyclesTimes
//
//    Kathleen Bonnell, Tue Jul  5 14:46:52 PDT 2005 
//    Added DetermineRealVarType method. 
//
//    Jeremy Meredith, Mon Aug 28 16:55:01 EDT 2006
//    Added ability to force using a specific plugin when opening a file.
//
//    Brad Whitlock, Wed Mar 14 20:33:41 PST 2007
//    Added overload of GetMetaDataForState.
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Removed methods to Set/Get TryHarderCyclesTimes and
//    TreatAllDBsAsTimeVarying
//
//    Jeremy Meredith, Wed Jan 23 16:25:45 EST 2008
//    Store the current default file opening options.  Added method that can
//    be used to update existing mdservers with new ones when they change.
//
//    Kathleen Bonnell, Thu Aug 14 16:13:41 PDT 2008 
//    Added SendFileOpenOptions.
//
//    Jeremy Meredith, Fri Jan 29 10:25:16 EST 2010
//    Added saved knowledge of what plugin was used to open a file.
//    Added extra flag to tell ClearFile whether or not we want to 
//    forget which plugin was used to open it.  In this case, we do not.
//
//    Jeremy Meredith, Fri Mar 26 10:39:17 EDT 2010
//    Though we do not need to use the command line to specify assumed and
//    fallback formats anymore, such usage still has some conveniences.
//    Added support to munge the preferred list when given those options.
//
//    Brad Whitlock, Fri Aug 19 10:32:57 PDT 2011
//    Expose ANY_STATE as a value that can be passed as a time state.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added Save and Restore Session file on remote host.
//
// ****************************************************************************

class VIEWER_API ViewerFileServerImplementation : public ViewerServerManager
{
    typedef std::map<std::string, avtDatabaseMetaData *> FileMetaDataMap;
    typedef std::map<std::string, avtSIL *> FileSILMap;
public:
    static const int ANY_STATE;

    virtual                   ~ViewerFileServerImplementation();

    static ViewerFileServerImplementation   *Instance();

    //
    // Use this method to give the file server new metadata for a simulation.
    //
    void                       SetSimulationMetaData(const std::string &host,
                                                   const std::string &filename,
                                                   const avtDatabaseMetaData&);

    void                       SetSimulationSILAtts(const std::string &host,
                                                   const std::string &filename,
                                                   const SILAttributes&);

    //
    // Use this method when you need metadata about the file but metadata
    // from any time state will suffice.
    //
    const avtDatabaseMetaData *GetMetaData(const std::string &host,
                                           const std::string &filename,
                                           const bool forceReadAllCyclesAndTimes = false);

    //
    // Use this method when you need metadata about the file at
    // the specified time state.
    //
    const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   const std::string &forcedFileType = "");
    const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState,
                                                   bool forceReadAllCyclesAndTimes,
                                                   const std::string &forcedFileType = "");

    //
    // Use this method when you need to determine if the metadata for a
    // file varies over time.
    //
    bool                       MetaDataIsInvariant(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState);

    //
    // Use this method when you need the file's SIL but the SIL from any
    // time state will suffice.
    //
    const avtSIL              *GetSIL(const std::string &host,
                                      const std::string &filename);

    //
    // Use this method when you need the file's SIL at the specified
    // time state.
    //
    const avtSIL              *GetSILForState(const std::string &host,
                                              const std::string &filename,
                                              int timeState);

    std::string                ExpandedFileName(const std::string &host,
                                                const std::string &db);
    void                       ExpandDatabaseName(std::string &hostDBName,
                                                  std::string &host,
                                                  std::string &dbName);

    void                       NoFaultStartServer(const std::string &host,
                                                  const stringVector &args);
    void                       StartServer(const std::string &host,
                                           const stringVector &args);
    void                       CloseServer(const std::string &host,
                                           bool close);
    void                       CloseServers();
    void                       ConnectServer(const std::string &host,
                                             const stringVector &args);
    void                       SendKeepAlives();

    void                       ClearFile(const std::string &fullName,
                                         bool forgetPlugin);
    void                       CloseFile(const std::string &host,
                                         const std::string &database);

    void                       SaveSession(const std::string &host, 
                                           const std::string &filename,
                                           const std::string &contents);
    void                       RestoreSession(const std::string &host, 
                                              const std::string &filename,
                                              std::string &contents);

    bool                       IsDatabase(const std::string &fullname) const;
    stringVector               GetOpenDatabases() const;


    void                       CreateNode(DataNode *);
    void                       SetFromNode(DataNode *);
    std::string                GetPluginErrors(const std::string &host);

    void                       UpdateDBPluginInfo(const std::string &host);

    void                       BroadcastUpdatedFileOpenOptions();

protected:
    ViewerFileServerImplementation();
    ViewerFileServerImplementation(const ViewerFileServerImplementation&);
    void TerminateConnectionRequest(const stringVector &, int failCode);
    void SplitKey(const std::string &key, std::string &hdb, int &ts) const;
    const avtDatabaseMetaData *GetMetaDataHelper(const std::string &host, 
                                                 const std::string &db,
                                                 int timeState,
                                                 bool forceReadAllCyclesAndTimes,
                                                 const std::string &forcedFileType = "");
    const avtSIL *GetSILHelper(const std::string &host, 
                               const std::string &db,
                               int timeState);
    bool  ExpansionRequired(const std::string &) const;
    void  SendFileOpenOptions(const std::string &host);
private:
    static ViewerFileServerImplementation *instance;

    FileMetaDataMap           fileMetaData;
    StringStringMap           filePlugins;
    FileSILMap                fileSIL;

    bool                      shouldSendFileOpenOptions;
};

#endif
