// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef _GETFILELIST_RPC_H_
#define _GETFILELIST_RPC_H_
#include <mdsrpc_exports.h>

#include <VisItRPC.h>
#include <vector>
#include <string>

// ****************************************************************************
// Class: GetFileListRPC
//
// Purpose:
//   This class encapsulates a call to get a list of files from a
//   remote file system.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   Tue Aug 29 10:45:58 PDT 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 29 10:46:53 PDT 2000
//   I moved the definitions of FileList's methods to the .C file.
//
//   Brad Whitlock, Mon Mar 24 14:15:14 PST 2003
//   I added a filter string and a boolean flag to the invokation method.
//   I also added a new VIRTUAL file type. I also modified FileList so that
//   it has methods to clear and sort itself.
//
//   Brad Whitlock, Thu Jul 29 12:15:30 PDT 2004
//   I added a new flag to turn on extra smart file grouping.
//
//   Brad Whitlock, Wed Dec 14 16:57:00 PST 2005
//   I added a new UNCHECKED enum to file_types to fill in for when we have
//   not checked for a file type yet.
//
//   Brad Whitlock, Fri Dec  7 11:41:32 PST 2007
//   Added TypeName overrides.
//
// ****************************************************************************

class MDSERVER_RPC_API GetFileListRPC : public BlockingRPC
{
public:
    enum file_types
    {
        DIR,
        REG,
        VIRTUAL,
        UNKNOWN,

        // internal use only
        UNCHECKED,
        UNCHECKED_REMOVE_IF_NOT_DIR
    };

    struct MDSERVER_RPC_API FileList : public AttributeSubject
    {
        stringVector names;
        intVector    types;
        stringVector virtualNames;
        intVector    numVirtualFiles;
    public:
        FileList();
        FileList(const FileList &obj);
        virtual ~FileList();
        virtual void SelectAll();
        virtual const std::string TypeName() const;

        void Clear();
        void Sort();
    };
public:
    GetFileListRPC();
    virtual ~GetFileListRPC();

    // Invokation method
    const FileList *operator()(const std::string &f, bool, bool=true);

    // Property selection methods
    virtual void SelectAll();

    const std::string &GetFilter() const;
    bool               GetAutomaticFileGrouping() const;
    bool               GetSmartFileGrouping() const;

    virtual const std::string TypeName() const;
private:
    FileList    fileList;
    std::string filter;
    bool        automaticFileGrouping;
    bool        smartFileGrouping;
};

// Method to print the file list.
MDSERVER_RPC_API ostream &operator << (ostream &os,
                                       const GetFileListRPC::FileList &fl);

#endif
