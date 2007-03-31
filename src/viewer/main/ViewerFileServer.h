#ifndef VIEWER_FILE_SERVER_H
#define VIEWER_FILE_SERVER_H
#include <viewer_exports.h>
#include <ViewerServerManager.h>
#include <map>
#include <string>
#include <vectortypes.h>

class MDServerProxy;
class avtDatabaseMetaData;
class avtSIL;

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
// ****************************************************************************

class VIEWER_API ViewerFileServer : public ViewerServerManager
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

    const avtDatabaseMetaData *GetMetaData(const std::string &host,
                                           const std::string &filename,
                                           const int timeState = -1);
    bool               MetaDataIsInvariant(const std::string &host,
                                           const std::string &filename,
                                           const int timeState = -1);
    const avtSIL              *GetSIL(const std::string &host,
                                      const std::string &filename,
                                      const int timeState = -1);
    std::string                ExpandedFileName(const std::string &host,
                                                const std::string &filename);

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

    void                       ClearFile(const std::string &fullName);
    void                       CloseFile(const std::string &host);
protected:
    ViewerFileServer();
    ViewerFileServer(const ViewerFileServer&);
    void TerminateConnectionRequest(const stringVector &, int failCode);
private:
    static ViewerFileServer *instance;

    ServerMap                servers, failedServers;
    FileMetaDataMap          fileMetaData;
    FileSILMap               fileSIL;
};

#endif
