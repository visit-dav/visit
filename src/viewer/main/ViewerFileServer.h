#ifndef VIEWER_FILE_SERVER_H
#define VIEWER_FILE_SERVER_H
#include <ViewerServerManager.h>
#include <map>
#include <string>
#include <vectortypes.h>
#include <avtTypes.h>

class MDServerProxy;
class avtDatabaseMetaData;
class avtSIL;
class DatabaseCorrelation;
class DatabaseCorrelationList;
class DataNode;

// ****************************************************************************
// Class: ViewerFileServer
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
// ****************************************************************************

class ViewerFileServer : public ViewerServerManager
{
    class ServerInfo
    {
    public:
        ServerInfo(MDServerProxy *, const stringVector &);
        ServerInfo(const ServerInfo &);
        virtual ~ServerInfo();

        MDServerProxy *proxy;
        stringVector  arguments;
    };

    typedef std::map<std::string, ServerInfo *> ServerMap;
    typedef std::map<std::string, avtDatabaseMetaData *> FileMetaDataMap;
    typedef std::map<std::string, avtSIL *> FileSILMap;
public:
    virtual                   ~ViewerFileServer();

    static ViewerFileServer   *Instance();
    static void                SplitHostDatabase(const std::string &hostDB,
                                                 std::string &host,
                                                 std::string &db);
    static std::string         ComposeDatabaseName(const std::string &host,
                                                   const std::string &db);

    //
    // Use this method when you need metadata about the file but metadata
    // from any time state will suffice.
    //
    const avtDatabaseMetaData *GetMetaData(const std::string &host,
                                           const std::string &filename);

    //
    // Use this method when you need metadata about the file at
    // the specified time state.
    //
    const avtDatabaseMetaData *GetMetaDataForState(const std::string &host,
                                                   const std::string &filename,
                                                   int timeState);

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
    void                       NoFaultStartServer(const std::string &host);
    void                       NoFaultStartServer(const std::string &host,
                                                  const stringVector &args);
    void                       StartServer(const std::string &host);
    void                       StartServer(const std::string &host,
                                           const stringVector &args);
    void                       CloseServer(const std::string &host,
                                           bool close);
    void                       CloseServers();
    void                       ConnectServer(const std::string &host,
                                             const stringVector &args);
    void                       SendKeepAlives();

    void                       ClearFile(const std::string &fullName);
    void                       CloseFile(const std::string &host);

    DatabaseCorrelationList   *GetDatabaseCorrelationList();
    DatabaseCorrelation       *CreateDatabaseCorrelation(const std::string &,
                                                         const stringVector &,
                                                         int, int=-1);
    void                       UpdateDatabaseCorrelation(const std::string &);
    DatabaseCorrelation       *GetMostSuitableCorrelation(const stringVector &) const;
    std::string                CreateNewCorrelationName() const;
    bool                       PreviouslyDeclinedCorrelationCreation(
                                   const stringVector &) const;
    void                       DeclineCorrelationCreation(const stringVector &);
    bool                       IsDatabase(const std::string &fullname) const;
    stringVector               GetOpenDatabases() const;

    avtVarType                 DetermineVarType(const std::string &host,
                                                const std::string &db,
                                                const std::string &var,
                                                int state);

    void                       CreateNode(DataNode *, bool detailed);
    void                       SetFromNode(DataNode *);
protected:
    ViewerFileServer();
    ViewerFileServer(const ViewerFileServer&);
    void TerminateConnectionRequest(const stringVector &, int failCode);
    void SplitKey(const std::string &key, std::string &hdb, int &ts) const;
    const avtDatabaseMetaData *GetMetaDataHelper(const std::string &host, 
                                                 const std::string &db,
                                                 int timeState);
    const avtSIL *GetSILHelper(const std::string &host, 
                               const std::string &db,
                               int timeState);
    bool  ExpansionRequired(const std::string &) const;
private:
    static ViewerFileServer *instance;

    ServerMap                 servers;
    FileMetaDataMap           fileMetaData;
    FileSILMap                fileSIL;
    DatabaseCorrelationList  *databaseCorrelationList;
    stringVector              declinedFiles;
    intVector                 declinedFilesLength;
};

#endif
