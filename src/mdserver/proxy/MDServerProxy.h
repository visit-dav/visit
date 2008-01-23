/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

// ************************************************************************* //
//                              MDServerProxy.h                              //
// ************************************************************************* //

#ifndef _MSSERVERPROXY_H_
#define _MDSERVERPROXY_H_
#include <mdsproxy_exports.h>

#include <RemoteProxyBase.h>
#include <GetDirectoryRPC.h>
#include <ChangeDirectoryRPC.h>
#include <CloseDatabaseRPC.h>
#include <ConnectCallback.h>
#include <ConnectRPC.h>
#include <CreateGroupListRPC.h>
#include <ExpandPathRPC.h>
#include <GetFileListRPC.h>
#include <GetDBPluginInfoRPC.h>
#include <GetMetaDataRPC.h>
#include <GetSILRPC.h>
#include <LoadPluginsRPC.h>
#include <avtDatabaseMetaData.h>
#include <SILAttributes.h>
#include <GetPluginErrorsRPC.h>
#include <SetMFileOpenOptionsRPC.h>

#include <string>
#include <vector>
#include <maptypes.h>
#include <vectortypes.h>

class     DBPluginInfoAttributes;


// ****************************************************************************
//  Class: MDServerProxy
//
//  Purpose:
//      MDServerProxy is a proxy class for creating and controlling a viewer.
//
//  Note:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//    Brad Whitlock, Mon Aug 21 14:51:36 PST 2000
//    I added a cc, assignment operator to FileList.
//
//    Jeremy Meredith, Fri Sep  1 17:17:12 PDT 2000
//    Added GetMetaData() and its associated objects (RPC, data).
//
//    Eric Brugger, Wed Oct 25 15:30:02 PDT 2000
//    I removed the argument "prog" from the Create method.
//
//    Brad Whitlock, Thu Nov 16 16:21:43 PST 2000
//    I added code that allows it to connect to a process that did not
//    launch it.
//
//    Sean Ahern, Wed Feb 28 14:34:26 PST 2001
//    Added the CreateGroupListRPC.
//
//    Hank Childs, Thu Mar 29 16:49:59 PST 2001
//    Added GetSIL.
//
//    Brad Whitlock, Tue Feb 12 13:56:43 PST 2002
//    Added ExpandPath.
//
//    Brad Whitlock, Mon Mar 25 16:17:24 PST 2002
//    Took away some methods.
//
//    Brad Whitlock, Fri May 3 17:22:54 PST 2002
//    Added a method for setting the remote user name.
//
//    Brad Whitlock, Tue Jul 30 10:52:33 PDT 2002
//    I added a method to close the active database.
//
//    Brad Whitlock, Mon Aug 26 15:53:27 PST 2002
//    I added methods to get the file separator.
//
//    Brad Whitlock, Fri Sep 27 15:34:11 PST 2002
//    I added methods to support a launch progress window.
//
//    Brad Whitlock, Thu Dec 26 16:17:27 PST 2002
//    I added support for security keys.
//
//    Brad Whitlock, Mon Mar 24 11:55:35 PDT 2003
//    I made VisIt capable of automatically detecting time-varying databases.
//
//    Brad Whitlock, Tue Apr 22 13:50:40 PST 2003
//    I fixed a crash on Windows.
//
//    Brad Whitlock, Fri May 2 12:14:28 PDT 2003
//    I made it inherit from RemoteProxyBase.
//
//    Brad Whitlock, Tue May 13 15:29:50 PST 2003
//    I made it possible to get metadata for a certain timestate.
//
//    Brad Whitlock, Mon Jun 16 13:45:31 PST 2003
//    I changed the Connect method so it only has a single argument.
//
//    Hank Childs, Thu Jan 22 21:02:56 PST 2004
//    Added LoadPluginsRPC.
//
//    Brad Whitlock, Thu Jul 29 12:26:38 PDT 2004
//    Added another argument to GetFileList.
//
//    Jeremy Meredith, Tue Feb  8 08:52:45 PST 2005
//    Added the ability to query for errors detected during plugin
//    initialization.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool forceReadAllCyclesAndTimes to GetMetaData
//
//    Hank Childs, Mon May 23 13:35:56 PDT 2005
//    Added ability to get DB plugin info.
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying to GetSIL
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added flags to GetMetaData for controlling creation of MeshQuality 
//    and TimeDerivative expressions.
//
//    Cyrus Harrison, Wed Nov 28 14:33:34 PST 2007
//    Added flag to GetMetaData for controlling auto creation of vector
//    magnitude expressions.
//
//    Jeremy Meredith, Wed Jan 23 16:10:37 EST 2008
//    Added ability to notify mdservers of the file opening options for the
//    database reader plugins.
//
// ****************************************************************************

class MDSERVER_PROXY_API MDServerProxy : public RemoteProxyBase
{
public:
    struct FileEntry
    {
        FileEntry();
        FileEntry(const FileEntry &);
        ~FileEntry();
        void operator = (const FileEntry &);

        static bool LessThan(const FileEntry &f1, const FileEntry &f2);

        std::string   name;
        int           size;

        bool          CanAccess() const    { return (flag & 1) == 1; }
        void          SetAccess(bool val)  { flag = (flag & 0xfe | (val?1:0)); }
        bool          IsVirtual() const    { return (flag & 2) == 2; }
        void          SetVirtual(bool val) { flag = (flag & 0xfd | (val?2:0)); }
    private:
        // Access  is bit 0
        // Virtual is bit 1
        unsigned char flag;
    };

    typedef std::vector<FileEntry> FileEntryVector;

    struct MDSERVER_PROXY_API FileList
    {
        FileList();
        FileList(const FileList &);
        ~FileList();
        void operator = (const FileList &);
        void Sort();
        void Clear();

        FileEntryVector       files;
        FileEntryVector       dirs;
        FileEntryVector       others;
        StringStringVectorMap virtualFiles;
    };
public:
    MDServerProxy();
    virtual ~MDServerProxy();

    virtual std::string GetComponentName() const;

    // RPCs to access functionality on the mdserver.
    void                       ChangeDirectory(const std::string &);
    void                       Connect(const stringVector &args);
    void                       CreateGroupList(const std::string &filename,
                                               const stringVector &groupList);
    std::string                GetDirectory();
    const FileList            *GetFileList(const std::string &filter, bool,
                                           bool=true);
    const avtDatabaseMetaData *GetMetaData(const std::string &, int=0,
                               bool forceReadAllCyclesTimes = false,
                               const std::string &forcedFileType = "",
                               bool treatAllDBsAsTimeVarying = false,
                               bool createMeshQualityExpressions = true,
                               bool createTimeDerivativeExpressions = true,
                               bool createVectorMagnitudeExpressions = true);
    const SILAttributes       *GetSIL(const std::string &, int=0,
                                   bool treatAllDBsAsTimeVarying = false);
    std::string                ExpandPath(const std::string &);
    void                       CloseDatabase();
    void                       CloseDatabase(const std::string &);
    void                       LoadPlugins();
    const DBPluginInfoAttributes *GetDBPluginInfo(void);
    void                       SetDefaultFileOpenOptions(
                                                       const FileOpenOptions&);

    std::string                GetPluginErrors();

    char                       GetSeparator() const;
    std::string                GetSeparatorString() const;
protected:
    virtual void               SetupComponentRPCs();
private:
    void                       DetermineSeparator();

    char                       separator;

    GetDirectoryRPC            getDirectoryRPC;
    ChangeDirectoryRPC         changeDirectoryRPC;
    GetFileListRPC             getFileListRPC;
    GetMetaDataRPC             getMetaDataRPC;
    GetSILRPC                  getSILRPC;
    ConnectRPC                 connectRPC;
    CreateGroupListRPC         createGroupListRPC;
    ExpandPathRPC              expandPathRPC;
    CloseDatabaseRPC           closeDatabaseRPC;
    LoadPluginsRPC             loadPluginsRPC;
    GetPluginErrorsRPC         getPluginErrorsRPC;
    GetDBPluginInfoRPC         getDBPluginInfoRPC;
    SetMFileOpenOptionsRPC     setMFileOpenOptionsRPC;

    FileList                   fileList;
    avtDatabaseMetaData        metaData;
    SILAttributes              sil;
};

#endif
