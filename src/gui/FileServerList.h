/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

#ifndef FILE_SERVER_LIST_H
#define FILE_SERVER_LIST_H
#include <gui_exports.h>
#include <map>
#include <string>
#include <vector>
#include <maptypes.h>
#include <MRUCache.h>
#include <AttributeSubject.h>
#include <ConnectCallback.h>
#include <MDServerProxy.h>
#include <QualifiedFilename.h>
#include <avtSIL.h>


using std::string;
using std::map;
using std::vector;

// Forward declarations.
class avtDatabaseMetaData;
class MessageAttributes;

// ****************************************************************************
// Class: FileServerList
//
// Purpose:
//   This class manages a list of MetaData servers running on various
//   remote machines and provides access to the files on those remote
//   machines. Furthermore, since it is derived from AttributeSubject,
//   it can tell Observer objects about changes in its file list.
//
// Notes:      
//   This "state object" is not communicable over the network. It is
//   only implemented as an AttributeSubject so it can have Observers
//   and tell them exactly what attributes changed. The attributes
//   are flags that indicate which real attribute changed.
//
//   We maintain a cache of MetaData. In general, care must be taken
//   to manage this cache correctly when the MetaData for a single
//   database varies with time. However, we are lucky in that the GUI
//   only queries certain members from MetaData that indeed don't
//   vary with time. For example, the number of states or materials.
//   There is only one place in the GUI where the pieces of MetaData
//   that can vary with time are actually used. This is in the File
//   Information Window. So, we deal with that case by making an
//   explicit call to the MD server to get the meta data if indeed
//   the MetaData is not invariant.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:02:59 PST 2000
//
// Modifications:
//   Brad Whitlock, Tue Nov 21 13:14:35 PST 2000
//   Added the SetConnectCallback method.
//
//   Sean Ahern, Wed Feb 28 14:38:21 PST 2001
//   Added the CreateGroupList method.
//
//   Brad Whitlock, Tue Apr 24 16:38:47 PST 2001
//   Added CloseServer method.
//
//   Brad Whitlock, Wed Apr 25 13:39:12 PST 2001
//   Added message attributes and methods.
//
//   Brad Whitlock, Fri May 25 14:46:45 PST 2001
//   Added GetSIL method.
//
//   Eric Brugger, Thu Nov 29 11:44:25 PST 2001
//   Added caching of the SIL. 
//
//   Brad Whitlock, Wed Feb 13 12:43:59 PDT 2002
//   Added support for tracking recent paths for a host.
//
//   Brad Whitlock, Fri Jul 26 13:47:24 PST 2002
//   I added a flag that tells the GUI to use the current directory by
//   default when starting up.
//
//   Brad Whitlock, Mon Jul 29 14:44:11 PST 2002
//   I added a ClearFile method that clears the metadata and SIL for the
//   specified file.
//
//   Brad Whitlock, Mon Aug 19 11:03:02 PDT 2002
//   I made the current directory flag an attribute.
//
//   Brad Whitlock, Thu Aug 22 14:24:54 PST 2002
//   I added some private methods for encoding/decoding spaces in path names.
//
//   Brad Whitlock, Mon Aug 26 15:35:49 PST 2002
//   I made it so the file server can understand both UNIX and Windows
//   filenames at the same time.
//
//   Brad Whitlock, Thu Sep 12 14:29:26 PST 2002
//   I changed the FileMatchesFilter prototype.
//
//   Brad Whitlock, Mon Sep 30 08:03:33 PDT 2002
//   I added a method to set the progress callback.
//
//   Brad Whitlock, Fri Feb 28 08:21:53 PDT 2003
//   I added HaveOpenedFile.
//
//   Brad Whitlock, Thu Mar 27 09:16:05 PDT 2003
//   I added support for automatic file grouping.
//
//   Brad Whitlock, Tue Apr 22 14:09:23 PST 2003
//   I fixed a crash on Windows.
//
//   Brad Whitlock, Tue Apr 29 12:06:51 PDT 2003
//   I added ExpandPath.
//
//   Brad Whitlock, Tue May 13 15:15:24 PST 2003
//   I made it possible to open a file at a non-zero time state.
//
//   Brad Whitlock, Tue May 20 14:56:58 PST 2003
//   Changed CreateNode interface.
//
//   Brad Whitlock, Mon Jun 23 11:31:14 PDT 2003
//   Added SilentNotify so we can execute several RPCs without having to
//   tell the observers each time we do something.
//
//   Mark C. Miller, Wed Oct  8 17:20:23 PDT 2003
//   Added GetMetaDataFromMDServer method
//
//   Brad Whitlock, Fri Oct 10 16:04:21 PST 2003
//   Added a method to clear out the recent paths.
//
//   Brad Whitlock, Thu Dec 18 11:33:32 PDT 2003
//   I changed the CreateNode interface.
//
//   Brad Whitlock, Mon Dec 29 11:37:22 PDT 2003
//   I added GetVirtualFileDefinitionSize so it is cheaper to ask for the
//   size of a virtual database definition.
//
//   Hank Childs, Sat Jan 24 11:32:01 PST 2004
//   Added LoadPlugins.
//
//   Brad Whitlock, Fri Mar 12 14:21:37 PST 2004
//   Added SendKeepAlives.
//
//   Jeremy Meredith, Wed Jul  7 17:03:33 PDT 2004
//   Made the selection filter global to all hosts.
//
//   Jeremy Meredith, Wed Aug 25 10:50:42 PDT 2004
//   Added methods for databases that return complete metadata only from
//   the engine (not the mdserver) to place the correct metadata her.
//   This is used for simulations.
//
//   Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//   Added forceReadAllCyclesTimes and methods to Set/Get it
//
//   Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//   Made fileMetaData and SILData MRUCache's
//   Changed interfaces to GetMetaData and GetSIL
//   Added ANY_STATE and GET_NEW_MD constants. Repalced isntances of 'std::'
//   with using statements, above.
//
//   Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//   Added support to treat all databases as time varying
//
//   Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//   Added support for turning on/off automatic creation of MeshQuality and
//   TimeDerivative expressions.
//
//   Cyrus Harrison, Wed Nov 28 13:24:51 PST 2007
//   Added flag for auto creation of vector magnitude expressions
//
// ****************************************************************************
class GUI_API FileServerList : public AttributeSubject
{
    typedef struct
    {
        string    path;
        MDServerProxy *server;
    } ServerInfo;

    // The '50' sets the *initial* number of slots in the MRU cache. It can
    // always be increased or decreased with a call to numslots(int n).
    // Note that the cache itself knows how to delete its entries
    typedef std::map<string, ServerInfo *> ServerMap;
    typedef MRUCache<string, avtDatabaseMetaData*, MRUCache_Delete, 50> FileMetaDataMap;
    typedef MRUCache<string, avtSIL*, MRUCache_Delete, 50> SILMap;

public:
    static const bool ANY_STATE;
    static const bool GET_NEW_MD;

    FileServerList();
    virtual ~FileServerList();
    virtual void Notify();
    void SilentNotify();
    void Initialize();

    virtual bool CreateNode(DataNode *, bool, bool);
    virtual void SetFromNode(DataNode *);

    void SetHost(const string &host);
    void SetPath(const string &path);
    void SetFilter(const string &filter);
    void SetAppliedFileList(const QualifiedFilenameVector &newFiles,
                            const vector<int>& timeStates=vector<int>(0));
    void SetUseCurrentDirectory(bool val);
    void SetAutomaticFileGrouping(bool val);
    void SetSmartFileGrouping(bool val);
    void LoadPlugins();
    void SendKeepAlives();

    void SetForceReadAllCyclesTimes(bool set);
    void SetTreatAllDBsAsTimeVarying(bool set);

    void SetCreateMeshQualityExpressions(bool set);
    void SetCreateTimeDerivativeExpressions(bool set);
    void SetCreateVectorMagnitudeExpressions(bool set);

    void OpenFile(const QualifiedFilename &filename, int timeState);
    void ReplaceFile(const QualifiedFilename &filename);
    void OverlayFile(const QualifiedFilename &filename);
    void CloseFile();
    void ClearFile(const QualifiedFilename &filename);
    void CreateGroupList(const string &filename,
                         const stringVector &groupList);

    const string &GetHost() const;
    const string &GetPath() const;
          string GetHomePath();
          string ExpandPath(const string &p);
          stringVector GetRecentHosts() const;
    const string &GetFilter() const;
    const MDServerProxy::FileList &GetFileList() const;
    const QualifiedFilenameVector &GetAppliedFileList();
          QualifiedFilenameVector GetFilteredFileList();
    bool GetUseCurrentDirectory() const;
    bool GetAutomaticFileGrouping() const;
    bool GetSmartFileGrouping() const;

    bool GetForceReadAllCyclesTimes() const;
    bool GetTreatAllDBsAsTimeVarying() const;

    bool GetCreateMeshQualityExpressions() const;
    bool GetCreateTimeDerivativeExpressions() const;
    bool GetCreateVectorMagnitudeExpressions() const;

    const stringVector &GetRecentPaths(const string &host) const;
    void AddPathToRecentList(const string &host, const string &path);
    void ClearRecentPathList();

    const QualifiedFilename &GetOpenFile() const;
    const avtSIL *GetSIL(const QualifiedFilename &f,
                         int timeState, bool anyStateOk,
                         bool dontGetNew, string *key = 0);
    const avtDatabaseMetaData *GetMetaData(const QualifiedFilename &filename,
                                           int timeState, bool anyStateOk,
                                           bool dontGetNew, string *key = 0);
    const avtDatabaseMetaData *GetCachedMetaData(const QualifiedFilename &filename,
                                                 int timeState) const;
    char GetSeparator();
    char GetSeparator(const string &host);
    string GetSeparatorString();
    string GetSeparatorString(const string &host);
    bool HaveOpenedFile(const QualifiedFilename &filename);

    int GetFileIndex(const QualifiedFilename &fileName);
    QualifiedFilename QualifiedName(const string &fileName);
    stringVector GetVirtualFileDefinition(const QualifiedFilename &) const;
    int GetVirtualFileDefinitionSize(const QualifiedFilename &) const;

    void SetConnectCallback(ConnectCallback *cb, void *data);
    void SetProgressCallback(bool (*cb)(void *, int), void *data);

    // Used to poke metadata into here if the mdserver has incomplete metadata
    void SetOpenFileMetaData(const avtDatabaseMetaData*, int timeState);
    void SetOpenFileSIL(const avtSIL*);

    // Convenience functions to determine if a component is selected.
    bool HostChanged() const;
    bool PathChanged() const;
    bool FilterChanged() const;
    bool FileListChanged() const;
    bool AppliedFileListChanged() const;
    bool FileChanged() const;
    bool RecentPathsChanged() const;
    bool OpenedFile() const;
    bool ReplacedFile() const;
    bool OverlayedFile() const;
    bool ClosedFile() const;

    MessageAttributes *GetMessageAttributes();
private:
    virtual void SelectAll();
    void StartServer(const string &host);
    void CloseServer(const string &host);
    void OpenAndGetMetaData(const QualifiedFilename &filename, int timeState,
                            int action);
    void ParseFilterString(const string &, stringVector &);
    bool FileMatchesFilterList(const string &, const stringVector &);
    bool FileMatchesFilter(const char *filter, const char *str, int &index);
    void Error(const char *message);
    void Warning(const char *message);
    string EncodePath(const string &path);
    string DecodePath(const string &path);
    void DefineVirtualFiles();
private:
    bool hostFlag;                  // attribute 0
    bool pathFlag;                  // attribute 1
    bool filterFlag;                // attribute 2
    bool fileListFlag;              // attribute 3
    bool appliedFileListFlag;       // attribute 4
    int  fileAction;                // attribute 5
    bool useCurrentDirectoryFlag;   // attribute 6
    bool automaticFileGroupingFlag; // attribute 7
    bool recentPathsFlag;           // attribute 8
    bool smartFileGroupingFlag;     // attribute 9

    // Information about the open md servers.
    ServerMap   servers;
    string      activeHost;
    bool        connectingServer;
    string      filter;
    bool        forceReadAllCyclesTimes;
    bool        treatAllDBsAsTimeVarying;

    bool        createMeshQualityExpressions;
    bool        createTimeDerivativeExpressions;
    bool        createVectorMagnitudeExpressions;

    // The file list for the current host.
    MDServerProxy::FileList fileList;

    // The applied file list
    QualifiedFilenameVector appliedFileList;

    // Information about the current file.
    QualifiedFilename openFile;
    int               openFileTimeState;

    // MRU caches for MetaData and SIL
    FileMetaDataMap   fileMetaData;
    SILMap            SILData;

    StringStringVectorMap recentPaths;
    StringStringVectorMap virtualFiles;

    // Used to tell the viewer to launch an mdserver.
    ConnectCallback *connectCallback;
    void            *connectCallbackData;
    // Used to update the GUI while launching an mdserver.
    bool           (*progressCallback)(void *, int);
    void            *progressCallbackData;

    // Used to tell the
    // Used for error messaging.
    MessageAttributes      *messageAtts;
};

#endif
