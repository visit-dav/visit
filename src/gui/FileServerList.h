// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#include <MDServerManager.h>
#include <QualifiedFilename.h>
#include <avtSIL.h>


// Forward declarations.
class avtDatabaseMetaData;
class HostProfileList;
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
//   Brad Whitlock, Fri Dec 14 17:16:51 PST 2007
//   Added ids for the fields that make up the object.
//
//   Jeremy Meredith, Fri Mar 19 13:27:07 EDT 2010
//   Replicate the logic already in the viewer, where we keep a cache
//   of which plugin we used to open a given file.  So when we re-open
//   it, we don't get it wrong.
//
//   Jeremy Meredith, Tue Mar 30 15:54:17 EDT 2010
//   Add way for GUI to pre-set a plugin we intend to use for a file.
//
//   Brad Whitlock, Mon Dec 13 10:37:33 PST 2010
//   I added "Ex" versions of GetMetaData and GetSIL.
//
//    David Camp, Thu Aug 27 09:40:00 PDT 2015
//    Added Save and Restore Session functions for remote hosts.
//
// ****************************************************************************

class GUI_API FileServerList : public AttributeSubject
{
//    typedef struct
//    {
//        std::string    path;
//        MDServerProxy *server;
//    } ServerInfo;

    // The '50' sets the *initial* number of slots in the MRU cache. It can
    // always be increased or decreased with a call to numslots(int n).
    // Note that the cache itself knows how to delete its entries
    //typedef std::map<std::string, ServerInfo *> ServerMap;
    typedef MRUCache<std::string, avtDatabaseMetaData*, MRUCache_Delete, 50> FileMetaDataMap;
    typedef MRUCache<std::string, avtSIL*, MRUCache_Delete, 50> SILMap;

public:
    static const bool ANY_STATE;
    static const bool GET_NEW_MD;

    FileServerList();
    virtual ~FileServerList();
    virtual void Notify();
    void SilentNotify();
    void Initialize();
    void SetProfiles(const HostProfileList *);

    virtual bool CreateNode(DataNode *, bool, bool);
    virtual void SetFromNode(DataNode *);

    void SetHost(const std::string &host);
    void SetPath(const std::string &path);
    void SetFilter(const std::string &filter);
    void SetAppliedFileList(const QualifiedFilenameVector &newFiles,
                            const std::vector<int>& timeStates=std::vector<int>(0));
    void SetUseCurrentDirectory(bool val);
    void SetAutomaticFileGrouping(bool val);
    void SetSmartFileGrouping(bool val);
    void SetShowDotFiles(bool val);
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
    void ClearFile(const QualifiedFilename &filename, bool forgetPlugin);
    void SetFilePlugin(const QualifiedFilename &filename, const std::string &plugin);
    void CreateGroupList(const std::string &filename,
                         const stringVector &groupList);
    void SaveSessionFile(const std::string &host, const std::string &filename, const std::string &contents);
    void RestoreSessionFile(const std::string &host, const std::string &filename, std::string &contents);

    const std::string &GetHost() const;
    const std::string &GetPath() const;
          std::string GetHomePath();
          std::string ExpandPath(const std::string &p);
          stringVector GetRecentHosts() const;
    const std::string &GetFilter() const;
    const MDServerMethods::FileList &GetFileList() const;
    const QualifiedFilenameVector &GetAppliedFileList();
          QualifiedFilenameVector GetFilteredFileList();
    bool GetUseCurrentDirectory() const;
    bool GetAutomaticFileGrouping() const;
    bool GetSmartFileGrouping() const;
    bool GetShowDotFiles() const;

    bool GetForceReadAllCyclesTimes() const;
    bool GetTreatAllDBsAsTimeVarying() const;

    bool GetCreateMeshQualityExpressions() const;
    bool GetCreateTimeDerivativeExpressions() const;
    bool GetCreateVectorMagnitudeExpressions() const;

    const stringVector &GetRecentPaths(const std::string &host) const;
    void AddPathToRecentList(const std::string &host, const std::string &path);
    void ClearRecentPathList();

    const QualifiedFilename &GetOpenFile() const;
    const avtSIL *GetSIL(const QualifiedFilename &f,
                         int timeState, bool anyStateOk,
                         bool dontGetNew, std::string *key = 0);
    const avtDatabaseMetaData *GetMetaData(const QualifiedFilename &filename,
                                           int timeState, bool anyStateOk,
                                           bool dontGetNew, std::string *key = 0);
    char GetSeparator();
    char GetSeparator(const std::string &host);
    std::string GetSeparatorString();
    std::string GetSeparatorString(const std::string &host);
    bool HaveOpenedFile(const QualifiedFilename &filename);

    int GetFileIndex(const QualifiedFilename &fileName);
    QualifiedFilename QualifiedName(const std::string &fileName);
    stringVector GetVirtualFileDefinition(const QualifiedFilename &) const;
    int GetVirtualFileDefinitionSize(const QualifiedFilename &) const;

    void SetStartServerCallback(ConnectCallback *cb, void *data);
    void SetConnectCallback(ConnectCallback *cb, void *data);
    void SetProgressCallback(bool (*cb)(void *, int), void *data);
    void GetProgressCallback(bool (*cb)(void *, int), void *data);

    // Used to poke metadata into here if the mdserver has incomplete metadata
    void SetFileMetaData(const QualifiedFilename &filename,
                         const avtDatabaseMetaData*, int timeState);
    void SetFileSIL(const QualifiedFilename &filename, const avtSIL*);
                               
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

    enum {
        ID_hostFlag,
        ID_pathFlag,
        ID_filterFlag,
        ID_fileListFlag,
        ID_appliedFileListFlag,
        ID_fileAction,
        ID_useCurrentDirectoryFlag,
        ID_automaticFileGroupingFlag,
        ID_recentPathsFlag,
        ID_smartFileGroupingFlag,
        ID_showDotFilesFlag
    };

private:
    virtual void SelectAll();
    const avtSIL *GetSILEx(const QualifiedFilename &f,
                         int timeState, bool anyStateOk,
                         bool dontGetNew, std::string *key = 0);
    const avtDatabaseMetaData *GetMetaDataEx(const QualifiedFilename &filename,
                                           int timeState, bool anyStateOk,
                                           bool dontGetNew, std::string *key = 0);

    void StartServer(const std::string &host);
    void CloseServer(const std::string &host);
    void OpenAndGetMetaData(const QualifiedFilename &filename, int timeState,
                            int action);
    void ParseFilterString(const std::string &, stringVector &);
    bool FileMatchesFilterList(const std::string &, const stringVector &);
    bool FileMatchesFilter(const char *filter, const char *str, int &index);
    void Error(const char *message);
    void Warning(const char *message);
    std::string EncodePath(const std::string &path);
    std::string DecodePath(const std::string &path);
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
    bool showDotFilesFlag;          // attribute 10

    // Information about the open md servers.
//    ServerMap   servers;
    std::string      activeHost;
    bool        connectingServer;
    std::string      filter;
    bool        forceReadAllCyclesTimes;
    bool        treatAllDBsAsTimeVarying;

    bool        createMeshQualityExpressions;
    bool        createTimeDerivativeExpressions;
    bool        createVectorMagnitudeExpressions;

    // The file list for the current host.
    MDServerMethods::FileList fileList;

    // The applied file list
    QualifiedFilenameVector appliedFileList;

    // Information about the current file.
    QualifiedFilename openFile;
    int               openFileTimeState;

    // MRU caches for MetaData and SIL
    FileMetaDataMap   fileMetaData;
    StringStringMap   filePlugins;
    SILMap            SILData;

    StringStringVectorMap recentPaths;
    StringStringVectorMap virtualFiles;

    // Used to tell the viewer to launch an mdserver.
    ConnectCallback *startServerCallback;
    void            *startServerCallbackData;
    ConnectCallback *connectCallback;
    void            *connectCallbackData;
    // Used to update the GUI while launching an mdserver.
    bool           (*progressCallback)(void *, int);
    void            *progressCallbackData;

    // Used for error messaging.
    MessageAttributes      *messageAtts;

    // Used when we need the host profiles.
    const HostProfileList  *profiles;
};

#endif
