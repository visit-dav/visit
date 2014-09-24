#include <MDServerMethods.h>

#include <MDServerState.h>

#include <GetDirectoryRPC.h>
#include <ChangeDirectoryRPC.h>
#include <GetFileListRPC.h>
#include <GetMetaDataRPC.h>
#include <GetSILRPC.h>
#include <ConnectRPC.h>
#include <CreateGroupListRPC.h>
#include <ExpandPathRPC.h>
#include <CloseDatabaseRPC.h>
#include <LoadPluginsRPC.h>
#include <GetPluginErrorsRPC.h>
#include <GetDBPluginInfoRPC.h>
#include <SetMFileOpenOptionsRPC.h>

#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

#include <DebugStream.h>
#include <TimingsManager.h>

#include <visit-config.h>


// ****************************************************************************
//  Method: MDServerMethods::Constructor
//
//  Purpose:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************
MDServerMethods::MDServerMethods(MDServerState *_state)
{
    state = _state;
}

// ****************************************************************************
//  Method: MDServerMethods::Destructor
//
//  Purpose:
//
//  Programmer: Jeremy Meredith
//  Creation:   August 18, 2000
//
//  Modifications:
//
// ****************************************************************************
MDServerMethods::~MDServerMethods()
{}

// ****************************************************************************
//  Method: MDServerMethods::ChangeDirectory
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
MDServerMethods::ChangeDirectory(const std::string &dir)
{
    state->changeDirectoryRPC(dir);
}

// ****************************************************************************
//  Method: MDServerMethods::GetDirectory
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

std::string
MDServerMethods::GetDirectory()
{
    return state->getDirectoryRPC();
}

// ****************************************************************************
//  Method:  MDServerMethods::GetPluginErrors
//
//  Purpose:
//    Returns the error string from plugin initialization.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February  7, 2005
//
// ****************************************************************************
std::string
MDServerMethods::GetPluginErrors()
{
    return state->getPluginErrorsRPC();
}

// ****************************************************************************
//  Method:  MDServerMethods::GetDBPluginInfo
//
//  Purpose:
//    Returns the types of database plugins and info about them.
//
//  Arguments:
//    none
//
//  Programmer:  Hank Childs
//  Creation:    May 25, 2005
//
// ****************************************************************************
const DBPluginInfoAttributes *
MDServerMethods::GetDBPluginInfo()
{
    return state->getDBPluginInfoRPC();
}

// ****************************************************************************
// Method: MDServerMethods::GetFileList
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
//   Brad Whitlock, Thu Jul 29 12:27:06 PDT 2004
//   I added the smartFileGrouping flag.
//
// ****************************************************************************

const MDServerMethods::FileList *
MDServerMethods::GetFileList(const std::string &filter,
    bool automaticFileGrouping, bool smartFileGrouping)
{
    // Try and get the file list from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    const GetFileListRPC::FileList *fl =
        state->getFileListRPC(filter, automaticFileGrouping, smartFileGrouping);

    // Clear out the file list.
    fileList.Clear();

#ifdef DEBUG
    // Write the file list to stdout.
    debug3 << "FILELIST=" << *fl << endl;
#endif

    int vfIndex = 0; int fileIndex = 0;
    for (size_t i = 0; i < fl->names.size(); ++i)
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
//  Method: MDServerMethods::GetMetaData
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
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added bool to forceReadAllCyclesAndTimes
//
//    Jeremy Meredith, Mon Aug 28 16:48:30 EDT 2006
//    Added ability to force using a specific plugin when reading
//    the metadata from a file (if it causes the file to be opened).
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added support to treat all databases as time varying
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007
//    Added flags for controlling creation of MeshQuality and
//    TimeDerivative expressions.
//
//    Cyrus Harrison, Wed Nov 28 14:33:34 PST 2007
//    Added flag to GetMetaData for controlling auto creation of vector
//    magnitude expressions.
//
//    Hank Childs, Tue Dec 11 16:35:25 PST 2007
//    Added timings information.
//
// ****************************************************************************

const avtDatabaseMetaData *
MDServerMethods::GetMetaData(const std::string &file, int timeState,
                           bool forceReadAllCyclesTimes,
                           const std::string &forcedFileType,
                           bool treatAllDBsAsTimeVarying,
                           bool createMeshQualityExpressions,
                           bool createTimeDerivativeExpressions,
                           bool createVectorMagnitudeExpressions)
{
    // Try and get the meta data from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    int t0 = visitTimer->StartTimer();
    const avtDatabaseMetaData *md = state->getMetaDataRPC(file, timeState,
                                         forceReadAllCyclesTimes,
                                         forcedFileType,
                                         treatAllDBsAsTimeVarying,
                                         createMeshQualityExpressions,
                                         createTimeDerivativeExpressions,
                                         createVectorMagnitudeExpressions);
#ifdef DEBUG
cout << "GetMetaData: file=" << file
     << ", timeState=" << timeState
     << ", forceReadAllCyclesTimes=" << forceReadAllCyclesTimes
     << ", forcedFileType=" << forcedFileType
     << ", treatAllDBsAsTimeVarying=" << treatAllDBsAsTimeVarying
     << ", createMeshQualityExpressions=" << createMeshQualityExpressions
     << ", createTimeDerivativeExpressions=" << createTimeDerivativeExpressions
     << ", createVectorMagnitudeExpressions=" << createVectorMagnitudeExpressions
     << endl;

    // Write the metadata to stdout.
    debug3 << "METADATA=" << endl;
    md->Print(debug3_real);
#endif

    metaData = *md;

    visitTimer->StopTimer(t0, "MDServerMethods::GetMetaData");
    return &metaData;
}

// ****************************************************************************
//  Method: MDServerMethods::GetSIL
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
//    Mark C. Miller, Wed Aug 22 20:16:59 PDT 2007
//    Added treatAllDBsAsTimeVarying
//
//    Hank Childs, Tue Dec 11 16:35:25 PST 2007
//    Added timings information.
//
// ****************************************************************************

const SILAttributes *
MDServerMethods::GetSIL(const std::string &file, int timeState,
    bool treatAllDBsAsTimeVarying)
{
    int t0 = visitTimer->StartTimer();

    // Try and get the SIL from the MD Server. This could throw an
    // exception, but we don't want to catch it here.
    const SILAttributes *s = state->getSILRPC(file, timeState,
                                 treatAllDBsAsTimeVarying);

#ifdef DEBUG
    // Write the metadata to stdout.
    debug3 << "SIL=" << endl;
    s->Print(debug3_real);
#endif

    sil = *s;

    visitTimer->StopTimer(t0, "MDServerMethods::GetSIL");

    return &sil;
}

// ****************************************************************************
// Method: MDServerMethods::Connect
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
MDServerMethods::Connect(const stringVector &args)
{
    // Tell the mdserver to connect to another program.
    state->connectRPC(args);
}

// ****************************************************************************
// Method: MDServerMethods::LoadPlugins
//
// Purpose:
//   Tells the mdserver that now is a good time to load its plugins.
//
// Programmer: Hank Childs
// Creation:   January 22, 2004
//
// ****************************************************************************

void
MDServerMethods::LoadPlugins(void)
{
    state->loadPluginsRPC();
}

// ****************************************************************************
// Method: MDServerMethods::CreateGroupList
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
MDServerMethods::CreateGroupList(const std::string &filename,
    const stringVector &groupList)
{
    state->createGroupListRPC(filename,groupList);
}

// ****************************************************************************
// Method: MDServerMethods::ExpandPath
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
MDServerMethods::ExpandPath(const std::string &p)
{
    return state->expandPathRPC(p);
}

// ****************************************************************************
// Method: MDServerMethods::CloseDatabase
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
MDServerMethods::CloseDatabase()
{
    state->closeDatabaseRPC("");
}

void
MDServerMethods::CloseDatabase(const std::string &db)
{
    state->closeDatabaseRPC(db);
}

// ****************************************************************************
//  Method:  MDServerMethods::SetDefaultFileOpenOptions
//
//  Purpose:
//    Sets new default file open options.
//
//  Arguments:
//    opts       the new options
//
//  Programmer:  Jeremy Meredith
//  Creation:    January 18, 2008
//
// ****************************************************************************
void
MDServerMethods::SetDefaultFileOpenOptions(const FileOpenOptions &opts)
{
    state->setMFileOpenOptionsRPC(opts);
}

// ****************************************************************************
// Method: MDServerMethods::GetSeparator
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
MDServerMethods::GetSeparator() const
{
    return state->separator;
}

// ****************************************************************************
// Method: MDServerMethods::GetSeparatorString
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

std::string
MDServerMethods::GetSeparatorString() const
{
    const char c[2] = {state->separator, '\0'};
    return std::string(c);
}


//
// MDServerMethods::FileList class methods.
//

// ****************************************************************************
// Method: MDServerMethods::FileList::FileList
//
// Purpose:
//   Constructor for MDServerMethods::FileList
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:00:05 PST 2000
//
// Modifications:
//   Brad Whitlock, Mon Mar 31 11:57:32 PDT 2003
//   I added virtualFiles.
//
// ****************************************************************************

MDServerMethods::FileList::FileList() : files(), dirs(), others(), virtualFiles()
{
    // nothing here
}

// ****************************************************************************
// Method: MDServerMethods::FileList::FileList
//
// Purpose:
//   Copy constructor for MDServerMethods::FileList
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

MDServerMethods::FileList::FileList(const MDServerMethods::FileList &f2) :
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
// Method: MDServerMethods::FileList::~FileList
//
// Purpose:
//   Destructor for MDServerMethods::FileList
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 21 15:00:52 PST 2000
//
// Modifications:
//
// ****************************************************************************

MDServerMethods::FileList::~FileList()
{
    // nothing here
}

// ****************************************************************************
// Method: MDServerMethods::FileList::operator =
//
// Purpose:
//   Assignment operator for MDServerMethods::FileList
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
MDServerMethods::FileList::operator = (const MDServerMethods::FileList &f2)
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
// Method: MDServerMethods::FileList::Clear
//
// Purpose:
//   Clears all of the vectors in the MDServerMethods::FileList class.
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
MDServerMethods::FileList::Clear()
{
    files.clear();
    dirs.clear();
    others.clear();
    virtualFiles.clear();
}

// ****************************************************************************
// Method: MDServerMethods::FileList::Sort
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
MDServerMethods::FileList::Sort()
{
    // Sort the file list alphabetically.
    std::sort(files.begin(), files.end(), FileEntry::LessThan);
    std::sort(dirs.begin(), dirs.end(), FileEntry::LessThan);
    std::sort(others.begin(), others.end(), FileEntry::LessThan);
}

// ****************************************************************************
// Method: MDServerMethods::FileEntry::FileEntry
//
// Purpose:
//   Constructor for the MDServerMethods::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//
// ****************************************************************************

MDServerMethods::FileEntry::FileEntry() : name()
{
    size = 0;
    flag = 0;
}

// ****************************************************************************
// Method: MDServerMethods::FileEntry::FileEntry
//
// Purpose:
//   Constructor for the MDServerMethods::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//
// ****************************************************************************

MDServerMethods::FileEntry::FileEntry(const MDServerMethods::FileEntry &obj)
{
    name = obj.name;
    size = obj.size;
    flag = obj.flag;
}

// ****************************************************************************
// Method: MDServerMethods::FileEntry::~FileEntry
//
// Purpose:
//   Destructor for the MDServerMethods::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//
// ****************************************************************************

MDServerMethods::FileEntry::~FileEntry()
{
}

// ****************************************************************************
// Method: MDServerMethods::FileEntry::operator =
//
// Purpose:
//   Assigment operator for the MDServerMethods::FileEntry class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Mar 28 11:58:09 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
MDServerMethods::FileEntry::operator = (const MDServerMethods::FileEntry &obj)
{
    name = obj.name;
    size = obj.size;
    flag = obj.flag;
}

// ****************************************************************************
// Method: MDServerMethods::FileEntry::LessThan
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
MDServerMethods::FileEntry::LessThan(const MDServerMethods::FileEntry &f1,
    const MDServerMethods::FileEntry &f2)
{
    if(f1.name == f2.name)
        return (f1.size < f2.size);
    else
        return (f1.name < f2.name);
}
