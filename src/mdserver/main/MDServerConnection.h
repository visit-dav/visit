// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef MDSERVER_CONNECTION_H
#define MDSERVER_CONNECTION_H
#include <map>
#include <string>
#include <vector>
#include <vectortypes.h>
#include <GetFileListRPC.h>
#include <FileOpenOptions.h>
#include <MDServerState.h>

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
class GetDBPluginInfoRPC;
class GetDBPluginInfoRPCExecutor;
class GetDirectoryRPC;
class GetDirectoryRPCExecutor;
class GetFileListRPCExecutor;
class GetMetaDataRPC;
class GetMetaDataRPCExecutor;
class GetPluginErrorsRPC;
class GetPluginErrorsRPCExecutor;
class GetSILRPC;
class GetSILRPCExecutor;
class KeepAliveRPC;
class KeepAliveRPCExecutor;
class LoadPluginsRPC;
class LoadPluginsRPCExecutor;
class MDServerApplication;
class CreateGroupListRPC;
class CreateGroupListRPCExecutor;
class Observer;
class ParentProcess;
class DBPluginInfoAttributes;
class SILAttributes;
class QuitRPC;
class QuitRPCExecutor;
class SetMFileOpenOptionsRPC;
class SetMFileOpenOptionsRPCExecutor;
class SaveSessionRPCExecutor;
class RestoreSessionRPCExecutor;
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
//    Brad Whitlock, Fri Feb 4 08:31:57 PDT 2005
//    Added a database argument to CloseDatabase and added the
//    SetFileGroupingOptions method.
//
//    Jeremy Meredith, Tue Feb  8 08:52:09 PST 2005
//    Added the ability to query for errors detected during plugin
//    initialization.
//
//    Brad Whitlock, Tue Apr 26 14:33:09 PST 2005
//    Added PartFilePattern, ConsolidateVirtualDatabases methods and added
//    digitLength member to VirtualFileInformation.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool arg to force reading all cycles/times to ReadMetaData  
//
//    Hank Childs, Mon May 23 14:04:43 PDT 2005
//    Added GetDBPluginInfo.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool arg to GetDatabase for forcing reading all cycles/times
//
//    Brad Whitlock, Wed Dec 14 16:58:24 PST 2005
//    Added ReadFileListAttributes.
//
//    Brad Whitlock, Tue Jan 17 17:05:59 PST 2006
//    I changed VirtualFileInformation so the filter used when we create a
//    virtual database is stored along with the definition in case we have to
//    reread the definition later.
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Hank Childs, Thu Jan 11 16:14:25 PST 2007
//    Add argument to GetDatabase so we can retrieve the names of plugins
//    we used to open a file.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to ReadSIL
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added flags to ReadMetaData for controlling creation of MeshQuality 
//    and TimeDerivative expressions.
//
//    Cyrus Harrison, Wed Nov 28 11:17:23 PST 2007
//    Added flag for creation of vector magnitude expressions
//
//    Jeremy Meredith, Wed Jan 23 16:18:17 EST 2008
//    Added ability to set default file open options.
//
//    David Camp, Thu Aug 27 09:25:33 PDT 2015
//    Added ability to save sessions remotely.
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
        int          digitLength;
        stringVector filterList;
    };

    typedef std::map<VirtualFileName, VirtualFileInformation> VirtualFileInformationMap;

    const VirtualFileInformationMap::iterator
        GetVirtualFileDefinition(const std::string &file);
public:
    MDServerConnection(MDServerApplication *, int *argc, char **argv[]);
    ~MDServerConnection();

    bool KeepGoing() const;
    bool ProcessInput();
    Connection *GetWriteConnection() const;

    // Functions used by the RPC Executors.
    void ReadMetaData(std::string file, int timeState,
                      bool forceReadAllCyclesAndTimes,
                      std::string forcedFileType,
                      bool treatAllDBsAsTimeVarying,
                      bool createMeshQualityExpressions,
                      bool createTimeDerivativeExpressions,
                      bool createVectorMagnitudeExpressions);

    avtDatabaseMetaData *GetCurrentMetaData() const;

    void ReadSIL(std::string file, int timeState,
        bool treatAllDBsAsTimeVarying);
    SILAttributes *GetCurrentSIL() const;

    DBPluginInfoAttributes *GetDBPluginInfo();

    void CloseDatabase(const std::string &db);
    void LoadPlugins();

    std::string GetPluginErrors();

    int  ChangeDirectory(const std::string &dir);
    const std::string &GetCurrentWorkingDirectory() const;

    int GetReadFileListReturnValue() const;
    GetFileListRPC::FileList *GetCurrentFileList();
    void GetFilteredFileList(GetFileListRPC::FileList &files);
    void SetFileGroupingOptions(const std::string &, bool);
    std::string ExpandPath(const std::string &path);
    void SetDefaultFileOpenOptions(const FileOpenOptions &opts);

private:
    void        ReadCWD();
    void        ReadFileList();

    bool FileMatchesFilterList(const std::string &) const;
    bool FileMatchesFilter(const char *filter, const char *str, int &j) const;
    bool GetPattern(const std::string &file, std::string &p, int &) const;
    bool FileHasVisItExtension(const std::string &file) const;
    bool FileLooksLikePartFile(const VirtualFileInformationMap &newVirtualFiles,
                               const std::string &pattern) const;
    void ConsolidateVirtualDatabases(VirtualFileInformationMap &newVirtualFiles,
                                     GetFileListRPC::FileList &files);
    void ReadFileListAttributes(GetFileListRPC::FileList &, bool);

private:
    MDServerApplication        *app;
    ParentProcess              *parent;    
    Xfer                       *xfer;

    // RPCs
    QuitRPC                    *quitRPC;
    KeepAliveRPC               *keepAliveRPC;
    MDServerState              *mdstate;

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
    GetPluginErrorsRPCExecutor *getPluginErrorsRPCExecutor;
    GetDBPluginInfoRPCExecutor *getDBPluginInfoRPCExecutor;
    SetMFileOpenOptionsRPCExecutor *setMFileOpenOptionsRPCExecutor;
    SaveSessionRPCExecutor     *saveSessionRPCExecutor;
    RestoreSessionRPCExecutor  *restoreSessionRPCExecutor;

    // State information for the program using this MDServer.
    avtDatabaseMetaData        *currentMetaData;
    SILAttributes              *currentSIL;
    std::string                currentWorkingDirectory;
    GetFileListRPC::FileList   currentFileList;
    int                        readFileListReturnValue;
    bool                       validFileList;
    stringVector               filterList;
    bool                       extraSmartFileGrouping;

    // Static members for all connections.
    static bool                       staticInit;
    static bool                       pluginsLoaded;
    static std::string                currentDatabaseName;
    static avtDatabase               *currentDatabase;
    static int                        currentDatabaseTimeState;
    static bool                       currentDatabaseHasInvariantMD;
    static VirtualFileInformationMap  virtualFiles;
    static std::map<std::string, std::string> filePlugins;

    avtDatabase               *GetDatabase(std::string, int timeState,
                                           bool forceReadAllCyclesAndTimes,
                                           std::vector<std::string> &,
                                           std::string forcedFileType="",
                                           bool treatAllDBsAsTimeVarying=false);
};

#endif
