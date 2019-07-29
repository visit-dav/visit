// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            MDServerMethods.h                              //
// ************************************************************************* //

#ifndef MDSERVER_METHODS_H
#define MDSERVER_METHODS_H


#include <mdsrpc_exports.h>
#include <string>
#include <maptypes.h>
#include <vectortypes.h>
#include <FileOpenOptions.h>
#include <SILAttributes.h>
#include <avtDatabaseMetaData.h>
#include <DBPluginInfoAttributes.h>

class MDServerState;


class MDSERVER_RPC_API MDServerMethods
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
        void          SetAccess(bool val)  { flag = ((flag & 0xfe) | (val?1:0)); }
        bool          IsVirtual() const    { return (flag & 2) == 2; }
        void          SetVirtual(bool val) { flag = ((flag & 0xfd) | (val?2:0)); }
    private:
        // Access  is bit 0
        // Virtual is bit 1
        unsigned char flag;
    };

    typedef std::vector<FileEntry> FileEntryVector;

    struct MDSERVER_RPC_API FileList
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
    MDServerMethods(MDServerState*);
    ~MDServerMethods();

    void                       ChangeDirectory(const std::string &);
    void                       Connect(const stringVector &args);
    void                       CreateGroupList(const std::string &filename,
                                               const stringVector &groupList);
    std::string                GetDirectory();
    const FileList             *GetFileList(const std::string &filter, bool,
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
    void                       SaveSession(const std::string &filename,
                                           const std::string &contents);
    void                       RestoreSession(const std::string &filename,
                                                    std::string &contents);

    std::string                GetPluginErrors();

    char                       GetSeparator() const;
    std::string                GetSeparatorString() const;
private:
    MDServerState* state;

    FileList                   fileList;
    avtDatabaseMetaData        metaData;
    SILAttributes              sil;
};
#endif
