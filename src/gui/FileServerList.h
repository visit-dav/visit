#ifndef FILE_SERVER_LIST_H
#define FILE_SERVER_LIST_H
#include <gui_exports.h>
#include <map>
#include <string>
#include <maptypes.h>
#include <AttributeSubject.h>
#include <ConnectCallback.h>
#include <MDServerProxy.h>
#include <QualifiedFilename.h>

// Forward declarations.
class avtDatabaseMetaData;
class avtSIL;
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
// ****************************************************************************

class GUI_API FileServerList : public AttributeSubject
{
    typedef struct
    {
        std::string    path;
        std::string    filter;
        MDServerProxy *server;
    } ServerInfo;

    typedef std::map<std::string, ServerInfo *> ServerMap;
    typedef std::map<std::string, avtDatabaseMetaData *> FileMetaDataMap;
    typedef std::map<std::string, avtSIL *> SILMap;

public:
    FileServerList();
    virtual ~FileServerList();
    virtual void Notify();
    void SilentNotify();
    void Initialize();

    virtual bool CreateNode(DataNode *, bool);
    virtual void SetFromNode(DataNode *);

    void SetHost(const std::string &host);
    void SetPath(const std::string &path);
    void SetFilter(const std::string &filter);
    void SetAppliedFileList(const QualifiedFilenameVector &newFiles);
    void SetUseCurrentDirectory(bool val);
    void SetAutomaticFileGrouping(bool val);

    void OpenFile(const QualifiedFilename &filename, int timeState);
    void ReplaceFile(const QualifiedFilename &filename);
    void OverlayFile(const QualifiedFilename &filename);
    void CloseFile();
    void ClearFile(const QualifiedFilename &filename);
    void CreateGroupList(const std::string &filename,
                         const stringVector &groupList);

    const std::string &GetHost() const;
    const std::string &GetPath() const;
    const stringVector &GetRecentPaths(const std::string &host) const;
          std::string GetHomePath();
          std::string ExpandPath(const std::string &p);
          stringVector GetRecentHosts() const;
    const std::string &GetFilter() const;
    const MDServerProxy::FileList &GetFileList() const;
    const QualifiedFilenameVector &GetAppliedFileList();
          QualifiedFilenameVector GetFilteredFileList();
    bool GetUseCurrentDirectory() const;
    bool GetAutomaticFileGrouping() const;
    const QualifiedFilename &GetOpenFile();
    const avtDatabaseMetaData *GetMetaData();
    const avtDatabaseMetaData *GetMetaData(const QualifiedFilename &f);
    const avtSIL *GetSIL(const QualifiedFilename &f);
    char GetSeparator();
    char GetSeparator(const std::string &host);
    std::string GetSeparatorString();
    std::string GetSeparatorString(const std::string &host);
    bool HaveOpenedFile(const QualifiedFilename &filename) const;

    int GetFileIndex(const QualifiedFilename &fileName);
    QualifiedFilename QualifiedName(const std::string &fileName);
    stringVector GetVirtualFileDefinition(const QualifiedFilename &) const;

    void SetConnectCallback(ConnectCallback *cb, void *data);
    void SetProgressCallback(bool (*cb)(void *, int), void *data);

    // Convenience functions to determine if a component is selected.
    bool HostChanged() const;
    bool PathChanged() const;
    bool FilterChanged() const;
    bool FileListChanged() const;
    bool AppliedFileListChanged() const;
    bool FileChanged() const;
    bool OpenedFile() const;
    bool ReplacedFile() const;
    bool OverlayedFile() const;
    bool ClosedFile() const;

    MessageAttributes *GetMessageAttributes();
private:
    virtual void SelectAll();
    void StartServer(const std::string &host);
    void CloseServer(const std::string &host);
    void OpenAndGetMetaData(const QualifiedFilename &filename, int timeState,
                            int action);
    void ParseFilterString(const std::string &, stringVector &);
    bool FileMatchesFilterList(const std::string &, const stringVector &);
    bool FileMatchesFilter(const char *filter, const char *str, int &index);
    void AddPathToRecentList(const std::string &host, const std::string &path);
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

    // Information about the open md servers.
    ServerMap   servers;
    std::string activeHost;

    // The file list for the current host.
    MDServerProxy::FileList fileList;

    // The applied file list
    QualifiedFilenameVector appliedFileList;

    // Information about the current file.
    QualifiedFilename openFile;
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
