#ifndef MDSERVER_CONNECTION_H
#define MDSERVER_CONNECTION_H
#include <map>
#include <string>
#include <vectortypes.h>
#include <GetFileListRPC.h>

// Forward declarations
class avtDatabase;
class avtDatabaseMetaData;
class ChangeDirectoryRPC;
class ChangeDirectoryRPCExecutor;
class CloseDatabaseRPC;
class CloseDatabaseRPCExecutor;
class Connection;
class ConnectRPC;
class ConnectRPCExecutor;
class ExpandPathRPC;
class ExpandPathRPCExecutor;
class GetDirectoryRPC;
class GetDirectoryRPCExecutor;
class GetFileListRPCExecutor;
class GetMetaDataRPC;
class GetMetaDataRPCExecutor;
class GetSILRPC;
class GetSILRPCExecutor;
class KeepAliveRPC;
class KeepAliveRPCExecutor;
class LoadPluginsRPC;
class LoadPluginsRPCExecutor;
class CreateGroupListRPC;
class CreateGroupListRPCExecutor;
class Observer;
class ParentProcess;
class SILAttributes;
class QuitRPC;
class QuitRPCExecutor;
class Xfer;

// ****************************************************************************
// Class: MDServerConnection
//
// Purpose:
//   This class contains all of the stuff needed to communicate with a 
//   remote process that uses the MDServerProxy. This is part of what allows
//   the MDServer to talk to more than one process.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov 17 13:10:12 PST 2000
//
// Modifications:
//    Sean Ahern, Wed Feb 28 14:31:30 PST 2001
//    Added the CreateGroupListRPC.
//   
//    Hank Childs, Thu Mar 29 17:01:44 PST 2001
//    Added GetSILRPC.
//
//    Jeremy Meredith, Wed Oct 10 14:47:35 PDT 2001
//    Made currentDatabase and currentDatabaseName static.
//
//    Brad Whitlock, Tue Feb 12 13:46:10 PST 2002
//    Added ExpandPath rpc,
//
//    Brad Whitlock, Tue Mar 26 12:00:45 PDT 2002
//    Changed GetWriteDescriptor to GetWriteConnection. Removed groups
//    since they are not portable.
//
//    Brad Whitlock, Tue Jul 30 10:34:18 PDT 2002
//    I added the CloseDatabase method and objects to handle that new RPC.
//
//    Brad Whitlock, Mon Mar 24 15:43:03 PST 2003
//    I added support for guessing related databases.
//
//    Brad Whitlock, Fri May 9 16:52:56 PST 2003
//    I added a method to return an iterator to a virtual file definition.
//
//    Brad Whitlock, Tue May 13 15:41:33 PST 2003
//    I added a timeState to ReadMetaData and ReadSIL.
//
//    Brad Whitlock, Mon Jun 9 10:51:52 PDT 2003
//    I added a method that lets us explicitly load plugins.
//
//    Brad Whitlock, Mon Oct 27 11:41:47 PDT 2003
//    Added VirtualFileName type so VirtualFileInformationMap sorts
//    using the numeric sorting algorithm.
//
//    Hank Childs, Thu Jan 22 21:02:56 PST 2004
//    Added LoadPluginsRPC.
//
//    Brad Whitlock, Fri Mar 12 10:17:58 PDT 2004
//    I added KeepAliveRPC.
//
//    Brad Whitlock, Thu Jul 29 11:29:12 PDT 2004
//    I added FileHasVisItExtension.
//
//    Jeremy Meredith, Wed Aug 25 11:41:54 PDT 2004
//    Handle errors through exceptions instead of error codes.   This allows
//    real error messages to make it to the user.
//
// ****************************************************************************

class MDServerConnection
{
    class VirtualFileName
    {
    public:
        VirtualFileName();
        VirtualFileName(const VirtualFileName &);
        VirtualFileName(const std::string &);
        ~VirtualFileName();
        void operator = (const VirtualFileName &);
        bool operator == (const VirtualFileName &) const;
        bool operator < (const VirtualFileName &) const;

        std::string name;
    };

    class VirtualFileInformation
    {
    public:
        VirtualFileInformation();
        VirtualFileInformation(const VirtualFileInformation &);
        virtual ~VirtualFileInformation();
        void operator = (const VirtualFileInformation &);

        std::string  path;
        stringVector files;
    };

    typedef std::map<VirtualFileName, VirtualFileInformation> VirtualFileInformationMap;

    const VirtualFileInformationMap::iterator
        GetVirtualFileDefinition(const std::string &file);
public:
    MDServerConnection(int *argc, char **argv[]);
    ~MDServerConnection();

    bool KeepGoing() const;
    bool ProcessInput();
    Connection *GetWriteConnection() const;

    // Functions used by the RPC Executors.
    void ReadMetaData(std::string file, int timeState);
    avtDatabaseMetaData *GetCurrentMetaData() const;

    void ReadSIL(std::string file, int timeState);
    SILAttributes *GetCurrentSIL() const;

    void CloseDatabase();
    void LoadPlugins();

    int  ChangeDirectory(const std::string &dir);
    const std::string &GetCurrentWorkingDirectory() const;

    int GetReadFileListReturnValue() const;
    GetFileListRPC::FileList *GetCurrentFileList();
    void GetFilteredFileList(GetFileListRPC::FileList &files,
                             const std::string &filter,
                             bool extraSmartGrouping);
    std::string ExpandPath(const std::string &path);
private:
    std::string FilteredPath(const std::string &path) const;
    void        ReadCWD();
    void        ReadFileList();

    bool FileMatchesFilterList(const std::string &, const stringVector &) const;
    bool FileMatchesFilter(const char *filter, const char *str, int &j) const;
    bool GetPattern(const std::string &file, std::string &p) const;
    std::string ExpandPathHelper(const std::string &path,
                                 const std::string &workingDir) const;
    bool FileHasVisItExtension(const std::string &file) const;
private:
    ParentProcess              *parent;    
    Xfer                       *xfer;

    // RPCs
    QuitRPC                    *quitRPC;
    KeepAliveRPC               *keepAliveRPC;
    GetDirectoryRPC            *getDirectoryRPC;
    ChangeDirectoryRPC         *changeDirectoryRPC;
    GetFileListRPC             *getFileListRPC;
    GetMetaDataRPC             *getMetaDataRPC;
    GetSILRPC                  *getSILRPC;
    ConnectRPC                 *connectRPC;
    CreateGroupListRPC         *createGroupListRPC;
    ExpandPathRPC              *expandPathRPC;
    CloseDatabaseRPC           *closeDatabaseRPC;
    LoadPluginsRPC             *loadPluginsRPC;

    // RPC Executors.
    QuitRPCExecutor            *quitExecutor;
    KeepAliveRPCExecutor       *keepAliveExecutor;
    GetDirectoryRPCExecutor    *getDirectoryExecutor;
    ChangeDirectoryRPCExecutor *changeDirectoryExecutor;
    GetFileListRPCExecutor     *getFileListExecutor;
    GetMetaDataRPCExecutor     *getMetaDataExecutor;
    GetSILRPCExecutor          *getSILExecutor;
    ConnectRPCExecutor         *connectExecutor;
    Observer                   *createGroupListExecutor;
    ExpandPathRPCExecutor      *expandPathExecutor;
    CloseDatabaseRPCExecutor   *closeDatabaseExecutor;
    LoadPluginsRPCExecutor     *loadPluginsExecutor;

    // State information for the program using this MDServer.
    avtDatabaseMetaData        *currentMetaData;
    SILAttributes              *currentSIL;
    std::string                currentWorkingDirectory;
    GetFileListRPC::FileList   currentFileList;
    int                        readFileListReturnValue;
    bool                       validFileList;

    // Static members for all connections.
    static bool                       staticInit;
    static bool                       pluginsLoaded;
    static std::string                currentDatabaseName;
    static avtDatabase               *currentDatabase;
    static int                        currentDatabaseTimeState;
    static bool                       currentDatabaseHasInvariantMD;
    static VirtualFileInformationMap  virtualFiles;

    avtDatabase               *GetDatabase(std::string, int timeState);
};

#endif
