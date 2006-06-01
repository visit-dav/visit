/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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
class CreateGroupListRPC;
class CreateGroupListRPCExecutor;
class Observer;
class ParentProcess;
class DBPluginInfoAttributes;
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
    MDServerConnection(int *argc, char **argv[]);
    ~MDServerConnection();

    bool KeepGoing() const;
    bool ProcessInput();
    Connection *GetWriteConnection() const;

    // Functions used by the RPC Executors.
    void ReadMetaData(std::string file, int timeState, bool forceReadAllCyclesAndTimes);
    avtDatabaseMetaData *GetCurrentMetaData() const;

    void ReadSIL(std::string file, int timeState);
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
private:
    std::string FilteredPath(const std::string &path) const;
    void        ReadCWD();
    void        ReadFileList();

    bool FileMatchesFilterList(const std::string &) const;
    bool FileMatchesFilter(const char *filter, const char *str, int &j) const;
    bool GetPattern(const std::string &file, std::string &p, int &) const;
    std::string ExpandPathHelper(const std::string &path,
                                 const std::string &workingDir) const;
    bool FileHasVisItExtension(const std::string &file) const;
    bool FileLooksLikePartFile(const VirtualFileInformationMap &newVirtualFiles,
                               const std::string &pattern) const;
    void ConsolidateVirtualDatabases(VirtualFileInformationMap &newVirtualFiles,
                                     GetFileListRPC::FileList &files);
    void ReadFileListAttributes(GetFileListRPC::FileList &, bool);

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
    GetPluginErrorsRPC         *getPluginErrorsRPC;
    GetDBPluginInfoRPC         *getDBPluginInfoRPC;

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

    avtDatabase               *GetDatabase(std::string, int timeState,
                                           bool forceReadAllCyclesAndTimes);
};

#endif
