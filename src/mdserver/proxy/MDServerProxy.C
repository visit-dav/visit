// ************************************************************************* //
//                              MDServerProxy.C                              //
// ************************************************************************* //
#include "MDServerProxy.h"

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>

#include <DebugStream.h>

#include <visit-config.h>

using std::string;
using std::vector;

// ****************************************************************************
//  Method: MDServerProxy constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//    Brad Whitlock, Thu Nov 16 16:21:15 PST 2000
//    I added connectRPC and the launch argument.
//
//    Brad Whitlock, Mon Nov 27 17:43:45 PST 2000
//    Added initialization of argc, argv.
//
//    Sean Ahern, Wed Feb 28 14:34:51 PST 2001
//    Added the CreateGroupListRPC.
//
//    Brad Whitlock, Tue Feb 12 13:58:14 PST 2002
//    Added ExpandPathRPC.
//
//    Brad Whitlock, Mon Feb 25 15:31:25 PST 2002
//    Initialized server and xfer pointers in case Create is not a success
//    before the destructor is called.
//
//    Brad Whitlock, Fri May 3 17:25:27 PST 2002
//    Added remoteUserName.
//
//    Brad Whitlock, Tue Jul 30 10:53:59 PDT 2002
//    Added closeDatabaseRPC.
//
//    Brad Whitlock, Mon Aug 26 15:54:57 PST 2002
//    Added separator and separatorString.
//
//    Brad Whitlock, Fri Sep 27 15:35:37 PST 2002
//    Added launch progress callbacks.
//
//    Brad Whitlock, Fri May 2 15:09:04 PST 2003
//    I made it inherit from the new RemoteProxyBase class.
//
// ****************************************************************************

MDServerProxy::MDServerProxy() : RemoteProxyBase("-mdserver"),
    getDirectoryRPC(), changeDirectoryRPC(), getFileListRPC(), connectRPC(),
    createGroupListRPC(), expandPathRPC(), closeDatabaseRPC()
{
    separator = SLASH_CHAR;
}

// ****************************************************************************
//  Method: MDServerProxy destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************

MDServerProxy::~MDServerProxy()
{
}

// ****************************************************************************
// Method: MDServerProxy::GetComponentName
//
// Purpose: 
//   Returns the name of the component.
//
// Returns:    The name of the component.
//
// Programmer: Brad Whitlock
// Creation:   Tue May 6 10:17:54 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

std::string
MDServerProxy::GetComponentName() const
{
    return "metadata server";
}

// ****************************************************************************
// Method: MDServerProxy::SetupComponentRPCs
//
// Purpose: 
//   This method connects the MDServer RPC's to the xfer object. It's called
//   from RemoteProxyBase::Create.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 15:10:40 PST 2003
//
// Modifications:
//   
// ****************************************************************************

void
MDServerProxy::SetupComponentRPCs()
{
    //
    // Connect the RPCs to the xfer object.
    //
    xfer.Add(&getDirectoryRPC);
    xfer.Add(&changeDirectoryRPC);
    xfer.Add(&getFileListRPC);
    xfer.Add(&getMetaDataRPC);
    xfer.Add(&getSILRPC);
    xfer.Add(&connectRPC);
    xfer.Add(&createGroupListRPC);
    xfer.Add(&expandPathRPC);
    xfer.Add(&closeDatabaseRPC);

    // Determine the separator to use in filenames.
    DetermineSeparator();
}

// ****************************************************************************
// Method: MDServerProxy::DetermineSeparator
//
// Purpose: 
//   Determines the separator to use in filenames.
//
// Programmer: Brad Whitlock
// Creation:   Fri May 2 12:08:57 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MDServerProxy::DetermineSeparator()
{
    // Get the startup directory.
    string d = GetDirectory();

    // Determine the separator to use in filenames.
    for(int i = 0; i < d.length(); ++i)
    {
        if(d[i] == '/')
        {
            separator = '/';
            break;
        }
        else if(d[i] == '\\')
        {
            separator = '\\';
            break;
        }
    }
}

// ****************************************************************************
//  Method: MDServerProxy::ChangeDirectory
//
//  Purpose:
//      Change the current working directory.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************

void
MDServerProxy::ChangeDirectory(const string &dir)
{
    changeDirectoryRPC(dir);
}

// ****************************************************************************
//  Method: MDServerProxy::GetDirectory
//
//  Purpose:
//      Get the name of the current working directory.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************

string
MDServerProxy::GetDirectory()
{
    return getDirectoryRPC();
}

// ****************************************************************************
// Method: MDServerProxy::GetFileList
//
// Purpose:
//    Get the list of files and directories in the current directory,
//
// Arguments:
//   filter : The filter string used to match filenames.
//
// Programmer: Jeremy Meredith
// Creation:   August 18, 2000
//
// Modifications:
//   Brad Whitlock, Tue Aug 22 17:33:00 PST 2000
//   Added a call to sort the file list. Also added conditionally
//   compiled debug code to print the file list.
//
//   Brad Whitlock, Mon Mar 24 11:58:21 PDT 2003
//   I added a filter string and a flag for automatic file grouping. I also
//   made the file list contain virtual files.
//
//   Brad Whitlock, Mon Apr 14 10:33:08 PDT 2003
//   I removed the code to sort the files since that's now done as the files
//   are read from the directory list.
//
// ****************************************************************************

const MDServerProxy::FileList *
MDServerProxy::GetFileList(const std::string &filter,
    bool automaticFileGrouping)
{
    // Try and get the file list from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    const GetFileListRPC::FileList *fl =
        getFileListRPC(filter, automaticFileGrouping);

    // Clear out the file list.
    fileList.Clear();

#ifdef DEBUG
    // Write the file list to stdout.
    debug3 << "FILELIST=" << *fl << endl;
#endif

    int vfIndex = 0; int fileIndex = 0;
    for (int i = 0; i < fl->names.size(); ++i)
    {
        FileEntry tmpentry;
        tmpentry.name = fl->names[i];
        tmpentry.size = fl->sizes[i];
        tmpentry.SetAccess(fl->access[i] > 0);
        bool isVirtual = fl->types[i] == GetFileListRPC::VIRTUAL;
        tmpentry.SetVirtual(isVirtual);

        if(isVirtual)
        {
            // Create a string vector that contains the files for the
            // current virtual file.
            stringVector tmp;
            int nFiles = fl->numVirtualFiles[vfIndex];
            tmp.reserve(nFiles);
            for(int j = 0; j < nFiles; ++j, ++fileIndex)
                tmp.push_back(fl->virtualNames[fileIndex]);

            // Insert the virtual file into the virtualFiles map.
            fileList.virtualFiles[fl->names[i]].swap(tmp);

            // Indicate that we're done with this virtual file.
            ++vfIndex;
        }

        if (fl->types[i] == GetFileListRPC::REG ||
            fl->types[i] == GetFileListRPC::VIRTUAL)
            fileList.files.push_back(tmpentry);
        else if (fl->types[i] == GetFileListRPC::DIR)
            fileList.dirs.push_back(tmpentry);
        else
            fileList.others.push_back(tmpentry);
    }

    return &fileList;
}

// ****************************************************************************
//  Method: MDServerProxy::GetMetaData
//
//  Purpose:
//      Get the metadata for a given database.
//
//  Programmer: Jeremy Meredith
//  Creation:   September  1, 2000
//
//  Modifications:
//    Brad Whitlock, Tue May 13 15:36:27 PST 2003
//    I added timeState.
//
// ****************************************************************************

const avtDatabaseMetaData *
MDServerProxy::GetMetaData(const string &file, int timeState)
{
    // Try and get the file list from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    const avtDatabaseMetaData *md = getMetaDataRPC(file, timeState);

#ifdef DEBUG
    // Write the metadata to stdout.
    debug3 << "METADATA=" << endl;
    md->Print(debug3_real);
#endif

    metaData = *md;

    return &metaData;
}

// ****************************************************************************
//  Method: MDServerProxy::GetSIL
//
//  Purpose:
//      Get the sil for a given database.
//
//  Programmer: Hank Childs
//  Creation:   March 29, 2001
//
//  Modifications:
//    Brad Whitlock, Tue May 13 15:36:51 PST 2003
//    I added timeState.
//
// ****************************************************************************

const SILAttributes *
MDServerProxy::GetSIL(const string &file, int timeState)
{
    // Try and get the file list from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    const SILAttributes *s = getSILRPC(file, timeState);

#ifdef DEBUG
    // Write the metadata to stdout.
    debug3 << "SIL=" << endl;
    s->Print(debug3_real);
#endif

    sil = *s;

    return &sil;
}

// ****************************************************************************
// Method: MDServerProxy::Connect
//
// Purpose: 
//   Tells the mdserver to connect to another program.
//
// Arguments:
//   args : All of the arguments needed to connect to another program.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue Nov 21 12:57:11 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Dec 26 16:02:46 PST 2002
//   I added a security key.
//
// ****************************************************************************

void
MDServerProxy::Connect(const stringVector &args)
{
    // Tell the mdserver to connect to another program.
    connectRPC(args);
}

// ****************************************************************************
// Method: MDServerProxy::CreateGroupList
//
// Purpose: 
//   This is a member function that sends a list of files (groupList) to
//   the MDServer to collect up into a group file (filename).
//
// Arguments:
//   filename  : The filename to create
//   groupList : The list of filenames to put into the group
//
// Programmer: Sean Ahern
// Creation:   Tue Feb 13 14:36:04 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Apr 3 13:23:09 PST 2002
//   Changed prototype so it uses references.
//
// ****************************************************************************

void
MDServerProxy::CreateGroupList(const std::string &filename,
    const stringVector &groupList)
{
    createGroupListRPC(filename,groupList);
}

// ****************************************************************************
// Method: MDServerProxy::ExpandPath
//
// Purpose: 
//   Expands the supplied path to its fullest.
//
// Arguments:
//   p : The path that we're expanding.
//
// Programmer: Brad Whitlock
// Creation:   Tue Feb 12 14:18:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

std::string
MDServerProxy::ExpandPath(const std::string &p)
{
    return expandPathRPC(p);
}

// ****************************************************************************
// Method: MDServerProxy::CloseDatabase
//
// Purpose: 
//   Invokes the closeDatabase rpc.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 30 10:55:35 PDT 2002
//
// Modifications:
//   
// ****************************************************************************

void
MDServerProxy::CloseDatabase()
{
    closeDatabaseRPC();
}

// ****************************************************************************
// Method: MDServerProxy::GetSeparator
//
// Purpose: 
//   Returns the file separator character.
//
// Returns:    The file separator character.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 16:00:51 PST 2002
//
// Modifications:
//   
// ****************************************************************************

char
MDServerProxy::GetSeparator() const
{
    return separator;
}

// ****************************************************************************
// Method: MDServerProxy::GetSeparatorString
//
// Purpose: 
//   Returns the file separator character as a string.
//
// Returns:    The file separator character as a string.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 26 16:01:20 PST 2002
//
// Modifications:
//   
// ****************************************************************************

string
MDServerProxy::GetSeparatorString() const
{
    const char c[2] = {separator, '\0'};
    return string(c);
}

//
// MDServerProxy::FileList class methods.
//

// ****************************************************************************
// Method: MDServerProxy::FileList::FileList
//
// Purpose: 
//   Constructor for MDServerProxy::FileList
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:00:05 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:57:32 PDT 2003
//   I added virtualFiles.
//
// ****************************************************************************

MDServerProxy::FileList::FileList() : files(), dirs(), others(), virtualFiles()
{
    // nothing here
}

// ****************************************************************************
// Method: MDServerProxy::FileList::FileList
//
// Purpose: 
//   Copy constructor for MDServerProxy::FileList
//
// Arguments:
//   f2 : The FileList to be copied into this.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:00:05 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:58:59 PDT 2003
//   I added virtualFiles.
//
//   Brad Whitlock, Tue Apr 22 13:53:40 PST 2003
//   I fixed a crash on Windows.
//
// ****************************************************************************

MDServerProxy::FileList::FileList(const MDServerProxy::FileList &f2) :
    files(), dirs(), others(), virtualFiles()
{
    FileEntryVector::const_iterator pos;

    // Copy the file list.
    for(pos = f2.files.begin(); pos != f2.files.end(); ++pos)
        files.push_back(*pos);

    // Copy the dir list.
    for(pos = f2.dirs.begin(); pos != f2.dirs.end(); ++pos)
        dirs.push_back(*pos);

    // Copy the others list.
    for(pos = f2.others.begin(); pos != f2.others.end(); ++pos)
        others.push_back(*pos);

    // Copy the virtualFiles map.
    StringStringVectorMap::const_iterator it;
    for(it = f2.virtualFiles.begin(); it != f2.virtualFiles.end(); ++it)
        virtualFiles[it->first] = it->second;
}

// ****************************************************************************
// Method: MDServerProxy::FileList::~FileList
//
// Purpose: 
//   Destructor for MDServerProxy::FileList
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:00:52 PST 2000
//
// Modifications:
//   
// ****************************************************************************

MDServerProxy::FileList::~FileList()
{
    // nothing here
}

// ****************************************************************************
// Method: MDServerProxy::FileList::operator =
//
// Purpose: 
//   Assignment operator for MDServerProxy::FileList
//
// Arguments:
//   f2 : The FileList to be copied into this.
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:01:16 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:59:26 PDT 2003
//   I added virtualFiles.
//
//   Brad Whitlock, Tue Apr 22 13:53:01 PST 2003
//   I fixed a crash on Windows.
//
// ****************************************************************************

void
MDServerProxy::FileList::operator = (const MDServerProxy::FileList &f2)
{
    FileEntryVector::const_iterator pos;

    // Clear out the lists.
    Clear();

    // Copy the file list.
    for(pos = f2.files.begin(); pos != f2.files.end(); ++pos)
        files.push_back(*pos);

    // Copy the dir list.
    for(pos = f2.dirs.begin(); pos != f2.dirs.end(); ++pos)
        dirs.push_back(*pos);

    // Copy the others list.
    for(pos = f2.others.begin(); pos != f2.others.end(); ++pos)
        others.push_back(*pos);

    // Copy the virtualFiles map.
    StringStringVectorMap::const_iterator it;
    for(it = f2.virtualFiles.begin(); it != f2.virtualFiles.end(); ++it)
        virtualFiles[it->first] = it->second;
}

// ****************************************************************************
// Method: MDServerProxy::FileList::Clear
//
// Purpose: 
//   Clears all of the vectors in the MDServerProxy::FileList class.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 29 17:38:08 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:59:43 PDT 2003
//   I added virtualFiles.
//
// ****************************************************************************

void
MDServerProxy::FileList::Clear()
{
    files.clear();
    dirs.clear();
    others.clear();
    virtualFiles.clear();
}

// ****************************************************************************
// Method: MDServerProxy::FileList::Sort
//
// Purpose: 
//   This method sorts the file, dir and other lists in the FileList.
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:36:08 PST 2000
//
// Modifications:
//   
// ****************************************************************************

void
MDServerProxy::FileList::Sort()
{
    // Sort the file list alphabetically.
    std::sort(files.begin(), files.end(), FileEntry::LessThan);
    std::sort(dirs.begin(), dirs.end(), FileEntry::LessThan);
    std::sort(others.begin(), others.end(), FileEntry::LessThan);
}

// ****************************************************************************
// Method: MDServerProxy::FileEntry::FileEntry
//
// Purpose: 
//   Constructor for the MDServerProxy::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MDServerProxy::FileEntry::FileEntry() : name()
{
    size = 0;
    flag = 0;
}

// ****************************************************************************
// Method: MDServerProxy::FileEntry::FileEntry
//
// Purpose: 
//   Constructor for the MDServerProxy::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MDServerProxy::FileEntry::FileEntry(const MDServerProxy::FileEntry &obj)
{
    name = obj.name;
    size = obj.size;
    flag = obj.flag;
}

// ****************************************************************************
// Method: MDServerProxy::FileEntry::~FileEntry
//
// Purpose: 
//   Destructor for the MDServerProxy::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

MDServerProxy::FileEntry::~FileEntry()
{
}

// ****************************************************************************
// Method: MDServerProxy::FileEntry::operator =
//
// Purpose: 
//   Assigment operator for the MDServerProxy::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
MDServerProxy::FileEntry::operator = (const MDServerProxy::FileEntry &obj)
{
    name = obj.name;
    size = obj.size;
    flag = obj.flag;
}

// ****************************************************************************
// Method: MDServerProxy::FileEntry::LessThan
//
// Purpose: 
//   This is a comparison function that can be used in sorting the
//   FileEntry.
//
// Arguments:
//   f1 : The first FileEntry
//   f2 : The first FileEntry
//
// Programmer: Brad Whitlock
// Creation:   Tue Aug 22 17:33:24 PST 2000
//
// Modifications:
//   
// ****************************************************************************

bool
MDServerProxy::FileEntry::LessThan(const MDServerProxy::FileEntry &f1,
    const MDServerProxy::FileEntry &f2)
{
    if(f1.name == f2.name)
        return (f1.size < f2.size);
    else
        return (f1.name < f2.name);
}

