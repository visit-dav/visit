// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                    avtZipWrapperFileFormatInterface.C                     //
// ************************************************************************* //
#include <avtZipWrapperFileFormatInterface.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <typeinfo>
#include <vector>

#include <sys/stat.h>
#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <direct.h>
#include <shlwapi.h>
#include <process.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>  // for WIFEXITED and WEXITSTATUS
#endif

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <Environment.h>
#include <ImproperUseException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <VisItInit.h>
#include <StringHelpers.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtFileFormatInterface.h>
#include <avtGenericDatabase.h>
#include <avtMTMDFileFormat.h>
#include <avtParallel.h>

#include <FileFunctions.h>

using std::string;
using std::vector;

string avtZipWrapperFileFormatInterface::tmpDir = "$TMPDIR";
string avtZipWrapperFileFormatInterface::decompCmd;
vector<avtZipWrapperFileFormatInterface*> avtZipWrapperFileFormatInterface::objList;
int avtZipWrapperFileFormatInterface::maxDecompressedFiles = 50;
static bool atExiting = false;

typedef struct DecompressedFileInfo {
    string rmDirname;
    avtFileFormatInterface *iface;
} DecompressedFileInfo;

// ****************************************************************************
//  Class: avtZWGenercDatabase
//
//  Purpose: A temporary derived class we can use to cast an avtDatabse pointer
//  to gain access to private part of generic database, the file file format
//  interface object.
//
//  Programmer: Mark C. Miller
//  Creation:   Thu Jul 26 18:55:05 PDT 2007
//
// ****************************************************************************
class avtZWGenericDatabase : public avtGenericDatabase
{
  public:
                                avtZWGenericDatabase(avtFileFormatInterface *ffi)
                                    : avtGenericDatabase(ffi) {;};
    avtFileFormatInterface     *GetFileFormatInterface()
                                    { avtFileFormatInterface *tmp = Interface;
                                      Interface = 0;
                                      return tmp; };
};

// ****************************************************************************
//  Class: avtZWFileFormatInterface
//
//  Purpose: A temporary derived class to cast an avtFileFormatInterface
//  pointer to to gain access to a private part of avtFileFormatInterface,
//  the file format object.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
class avtZWFileFormatInterface : public avtFileFormatInterface
{
  public:
    avtFileFormat *GetFileFormat(int n) { return GetFormat(n); };
};


// ****************************************************************************
//  Class: avtZipWrapperFileFormat
//
//  Purpose: A dummied format object used solely to store information about
//  the real format object(s) constructed by ZipWrapperFormatInterface
//
//  Programmer: Mark C. Miller
//  Creation:   Thu Jul 26 18:55:05 PDT 2007
//
//  Modifications:
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Hank Childs, Thu Feb  3 11:40:18 PST 2011
//    Fix problem with virtual methods not matching signature of base type.
//
// ****************************************************************************
class avtZipWrapperFileFormat : public avtMTMDFileFormat
{
  public:
                           avtZipWrapperFileFormat(avtFileFormat *ff) :
                               avtMTMDFileFormat("ZipWrapperDummy"),
                               realFileFormat(ff) {;};
    virtual               ~avtZipWrapperFileFormat() {;};

    virtual void           FreeUpResources(void) {;};
    virtual vtkDataSet    *GetMesh(int, int, const char *) {return 0;} ;
    virtual vtkDataArray  *GetVar(int, int, const char *) {return 0;};

    // Methods overridden to ensure this dummied up format object behaves like
    // the real format object it parallels.
    bool                   HasInvariantMetaData() const
                               { return realFileFormat->HasInvariantMetaData(); };
    bool                   HasInvariantSIL() const
                               { return realFileFormat->HasInvariantSIL(); };
    bool                   CanCacheVariable(const char *v)
                               { return realFileFormat->CanCacheVariable(v); };
    const char            *GetType(void)
                               { return "ZipWrapperDummy"; };
    bool                   HasVarsDefinedOnSubMeshes()
                               { return realFileFormat->HasVarsDefinedOnSubMeshes(); };
    bool                   PerformsMaterialSelection()
                               { return realFileFormat->PerformsMaterialSelection(); };
    bool                   CanDoStreaming()
                               { return realFileFormat->CanDoStreaming(); };

    // Methods where data is pushed down to this dummied up format object
    // from avtFileFormatInterface loops that we record for later query/ back out.
    void                   TurnMaterialSelectionOff(void)
                               { dummiedMaterialSelection = ""; };
    void                   TurnMaterialSelectionOn(const char *name)
                               { dummiedMaterialSelection = string(name); };
    void                   RegisterVariableList(const char *var,
                                                const vector<CharStrRef> &vars2nd)
                               { dummiedRegisterVariableList_var = string(var);
                                 dummiedRegisterVariableList_2nd = vars2nd; };
    void                   RegisterDataSelections(const vector<avtDataSelection_p>& sels,
                               vector<bool> *apps)
                               { dummiedRegisterDataSelections_sels = sels;
                                 dummiedRegisterDataSelections_apps = apps; };

     // Methods to query out of this dummied up format object the information
     // we need to apply to real format objects.
     avtVariableCache     *GetCache()
                               { return cache; };
     string                GetDummiedMaterialSelection()
                               { return dummiedMaterialSelection; };
     string                GetDummiedRegisterVariableList_var()
                               { return dummiedRegisterVariableList_var; };
     const vector<CharStrRef> &GetDummiedRegisterVariableList_2nd()
                               { return dummiedRegisterVariableList_2nd; };
     const vector<avtDataSelection_p> &GetDummiedRegisterDataSelections_sels()
                               { return dummiedRegisterDataSelections_sels; };
     vector<bool>         *GetDummiedRegisterDataSelections_apps()
                               { return dummiedRegisterDataSelections_apps; };
  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int) {return;} ;

  private:
    string                 dummiedMaterialSelection;
    string                 dummiedRegisterVariableList_var;
    vector<CharStrRef>     dummiedRegisterVariableList_2nd;
    vector<avtDataSelection_p> dummiedRegisterDataSelections_sels;
    vector<bool>          *dummiedRegisterDataSelections_apps;
    avtFileFormat         *realFileFormat;

};

// ****************************************************************************
//  Static Function: FreeUpCacheSlot
//
//  Purpose: The Delete callback for the MRU cache. The decompressed file
//  should be removed from disk and the interface associated with it deleted.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
static void FreeUpCacheSlot(void *item)
{
    DecompressedFileInfo *finfo = (DecompressedFileInfo*) item;
    avtZWFileFormatInterface *iface = (avtZWFileFormatInterface*) finfo->iface;
    string filename = iface->GetFilename(0);
    debug5 << "Removing decompressed entry \"" << filename << "\"" << endl;
    delete iface;
    static int issuedWarnings = 0;

    if (finfo->rmDirname == "")
    {
        errno = 0;
        if (unlink(filename.c_str()) != 0 && errno != ENOENT)
        {
            if (issuedWarnings < 5)
            {
                debug5 << "Unable to unlink() decompressed file \"" << filename << "\"" << endl;
                debug5 << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                cerr << "Unable to remove decompressed file \"" << filename << "\"" << endl;
                cerr << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                issuedWarnings++;
            }
        }
    }
    else
    {
        DIR *dirp = opendir(finfo->rmDirname.c_str());
        if (!dirp)
        {
            if (issuedWarnings < 5)
            {
                debug5 << "Unable to descend into directory \"" << finfo->rmDirname << "\" to remove its contents." << endl;
                cerr << "Unable to descend into directory \"" << finfo->rmDirname << "\" to remove its contents." << endl;
                issuedWarnings++;
            }

        }
        dirent *dp;
        while ((dp = readdir(dirp)))
        {
            errno = 0;
            if (unlink(dp->d_name) != 0 && errno != ENOENT)
            {
                if (issuedWarnings < 5)
                {
                    debug5 << "Unable to unlink() \"" << dp->d_name << "\"" << endl;
                    debug5 << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                    cerr << "Unable to unlink() \"" << dp->d_name << "\"" << endl;
                    cerr << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                    issuedWarnings++;
                }
            }
        }
        closedir(dirp);
        errno = 0;
        if (rmdir(finfo->rmDirname.c_str()) != 0)
        {
            if (issuedWarnings < 5)
            {
                debug5 << "Unable to rmdir() \"" << finfo->rmDirname << "\"" << endl;
                debug5 << "rmdir() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                cerr << "Unable to rmdir() \"" << finfo->rmDirname << "\"" << endl;
                cerr << "rmdir() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
                issuedWarnings++;
            }
        }
    }

    delete finfo;
}

// ****************************************************************************
//  Function: CleanUpAtExit
//
//  Purpose: Enforces cleanup (removal of decompressed files from filesystem).
//  For efficiency reasons, when engine process is terminating, it does NOT
//  clean up after itself. So, we can't rely upon our destructor being called
//  on the engine. Instead, we utilize atexit() functionality. This is the
//  function we register with atexit to cleanup decompressed files.
//
//  Note: By the time we are executing this function, we don't have any
//  guarentee that rest of VisIt is available. In particular, we can't
//  issue messages to debug logs.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Removed debug5 lines that were commented out. Fixed algorithm for
//    deleting objects so that after deleting the ffi object, it also deletes
//    the pointer to the object in the list.
//
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::CleanUpAtExit()
{
    // delete all the zip wrapper objects that still exist
    atExiting = true;
    while (!objList.empty())
    {
        delete objList.back();
        objList.pop_back();
    }
    avtZipWrapperFileFormatInterface::Finalize();
}

// ****************************************************************************
//  Static Method: Initialize
//
//  Purpose: Do all the work we need to do only once for ZipWrapper instances.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Added read options. Replaced most getenv calls with read options.
//    Added broadcasting of env. results when necessary. Re-organized the
//    routine a bit too.
//
//    Mark C. Miller, Wed Jun 11 12:02:52 PDT 2008
//    Removed 'should broadcast env' logic -- it was bogus anyways.
//
//    Brad Whitlock, Wed Jun 25 11:12:50 PDT 2008
//    Check a few places for a good place to make a temp dir.
//
// ****************************************************************************

void
avtZipWrapperFileFormatInterface::Initialize(int procNum, int procCount,
    const DBOptionsAttributes *rdopts)
{
    //
    // Process any read options
    //
    bool dontAtExit = false; (void) dontAtExit;
    string userName = "$USER";
    for (int i = 0; rdopts != 0 && i < rdopts->GetNumberOfOptions(); ++i)
    {
        if (rdopts->GetName(i) == "TMPDIR for decompressed files")
            tmpDir = rdopts->GetString("TMPDIR for decompressed files");
        else if (rdopts->GetName(i) == "Don't atexit()")
            dontAtExit = rdopts->GetBool("Don't atexit()");
        else if (rdopts->GetName(i) == "Max. # decompressed files")
            maxDecompressedFiles = rdopts->GetInt("Max. # decompressed files");
        else if (rdopts->GetName(i) == "Unique moniker for dirs made in $TMPDIR")
            userName = rdopts->GetString("Unique moniker for dirs made in $TMPDIR");
        else if (rdopts->GetName(i) == "Decompression command")
            decompCmd = rdopts->GetString("Decompression command");
        else
            debug1 << "Ignoring unknown option \"" << rdopts->GetName(i) << "\"" << endl;
    }

    // Decide on root temporary directory
    if (tmpDir == "$TMPDIR" && procNum == 0)
    {
#ifdef WIN32
        if (!Environment::get("TMP").empty())
            tmpDir = Environment::get("TMP");
        else if (!Environment::get("TEMP").empty())
            tmpDir = Environment::get("TEMP");
        else
            tmpDir = Environment::get("VISITUSERHOME");
#else
        if (!Environment::get("TMPDIR").empty())
        {
            tmpDir = Environment::get("TMPDIR");
        }
        else
        {
            // Check a few places. /usr/tmp does not exist on Mac.
            FileFunctions::VisItStat_t s;
            static const char *possibleDirs[] = {"/usr/tmp", "/var/tmp"};
            bool foundDir = false;
            for(int i = 0; i < 2; ++i)
            {
                if(FileFunctions::VisItStat(possibleDirs[i], &s) == 0)
                {
                    bool isDir = S_ISDIR(s.st_mode);
                    if(isDir)
                    {
                        tmpDir = possibleDirs[i];
                        foundDir = true;
                        break;
                    }
                }
            }
            // Last resort, use HOME.
            if(!foundDir)
                tmpDir = Environment::get("HOME");
        }
#endif
    }

    // Decide on user name moniker
    if (userName == "$USER" && procNum == 0)
    {
        if (!Environment::get("USER").empty())
        {
            userName = Environment::get("USER");
        }
        else if (!Environment::get("USERNAME").empty())
        {
            userName = Environment::get("USERNAME");
        }
        else
        {
            userName = "user";
        }
    }

    //
    // We need to broadcast some stuff ONLY if we obtained it
    // by querying env. variables. This is because MPI doesn't
    // do a good job of ensuring that all procs get same env.
    //
#ifdef PARALLEL
    BroadcastString(tmpDir, procNum);
    BroadcastString(userName, procNum);
#endif

    // Set maximum file count
    if (maxDecompressedFiles < 0)
        maxDecompressedFiles = -maxDecompressedFiles;
    else
        maxDecompressedFiles /= procCount;

    debug5 << "ZipWrapper will maintain a maximum of " << maxDecompressedFiles
           << " decompressed files " << (procCount > 1 ? "per-processor" : "")
           << " at any one time." << endl;

    char procNumStr[32];
    snprintf(procNumStr, sizeof(procNumStr), "_%04d", procNum);
    tmpDir = tmpDir + VISIT_SLASH_STRING + "visitzw_" + userName + "_" +
             string(VisItInit::GetComponentName()) +
             (procCount > 1 ? string(procNumStr) : "");
    debug5 << "ZipWrapper is using \"" << tmpDir << "\" as the temporary directory" << endl;

    // Make the temporary directory
    // (will have different name on mdserver and engine)
    errno = 0;
#ifdef WIN32
    if(_mkdir(tmpDir.c_str()) != 0 && errno != EEXIST)
#else
    if (mkdir(tmpDir.c_str(), 0777) != 0 && errno != EEXIST)
#endif

    {
        static char errMsg[1024];
        snprintf(errMsg, sizeof(errMsg), "mkdir failed with errno=%d (\"%s\")",
            errno, strerror(errno));
        EXCEPTION1(InvalidFilesException, errMsg);
    }

#ifndef MDSERVER
    // See note in CleanUpAtExit
    if (dontAtExit == false)
        atexit(avtZipWrapperFileFormatInterface::CleanUpAtExit);
#endif
}

// ****************************************************************************
//  Static Method: Finalize
//
//  Purpose: Undo anything we did during initialize.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::Finalize()
{
    errno = 0;
#ifdef WIN32
    if (_rmdir(tmpDir.c_str()) != 0 && errno != ENOENT)
#else
    if (rmdir(tmpDir.c_str()) != 0 && errno != ENOENT)
#endif
    {
        debug5 << "Unable to remove temporary directory \"" << tmpDir << "\"" << endl;
        debug5 << "rmdir() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
        cerr << "Unable to remove temporary directory \"" << tmpDir << "\"" << endl;
        cerr << "rmdir() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
    }
}

// ****************************************************************************
//  Method: avtZipWrapper constructor
//
//  Purpose: Register objects getting constructed so we can keep track of and
//  delete them in CleanUpAtExit. Also, initialize everything we need.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//    Mark C. Miller, Wed Aug  8 14:48:03 PDT 2007
//    Changed it to loop over plugins until one correctly opens.
//
//    Mark C. Miller, Mon Aug 20 12:48:37 PDT 2007
//    Initialized dummyFileFormat to 0 before calling GetRealInterface
//
//    Brad Whitlock, Tue Jun 24 16:44:49 PDT 2008
//    Pass in the zipwrapper info so we can access the plugin manager.
//
//    Mark C. Miller, Tue Dec  1 18:31:52 PST 2009
//    Fix regular expressions for parsing extension and decompressed name.
// ****************************************************************************

avtZipWrapperFileFormatInterface::avtZipWrapperFileFormatInterface(
    const char *const *list, int nl, int nb, const DBOptionsAttributes *rdopts,
    CommonDatabasePluginInfo *zwinfo) :
    inputFileBlockCount(nb),
    decompressedFilesCache(FreeUpCacheSlot)
{
    procNum = 0;
    procCount = 1;
#ifdef PARALLEL
    MPI_Comm_rank(VISIT_MPI_COMM, &procNum);
    MPI_Comm_size(VISIT_MPI_COMM, &procCount);
#endif

    // keep track of objects to cleanup in CleanUpAtExit
    if (objList.size() == 0)
        avtZipWrapperFileFormatInterface::Initialize(procNum, procCount, rdopts);
    objList.push_back(this);

    // store list of files
    nTimesteps = nl / nb;
    for (int t = 0; t < nTimesteps; t++)
    {
        vector<string> blockList;
        for (int b = 0; b < nb; b++)
            blockList.push_back(list[t*nb+b]);
        inputFileList.push_back(blockList);
    }

    //
    // Make sure the necessary real plugin is loaded.
    //
    string ext = StringHelpers::ExtractRESubstr(inputFileList[0][0].c_str(), "<\\.(xz|gz|bz|bz2|zip)$>");
    const char *bname = FileFunctions::Basename(inputFileList[0][0].c_str());
    string dcname = StringHelpers::ExtractRESubstr(bname, "<(.*)\\.(xz|gz|bz|bz2|zip)$> \\1");

    // Save the pointer to the plugin manager.
    pluginManager = zwinfo->GetPluginManager();

    dummyFileFormat = 0;

    //
    // Find right plugin, load it and open the first file.
    // Note: matching plugins that don't open the file get loaded
    // and never unloaded.
    //
    pluginId = "";
    dummyInterface = 0;
    const bool searchAllPlugins = true;
    vector<string> ids = pluginManager->GetMatchingPluginIds(dcname.c_str(), searchAllPlugins);
    for (size_t i = 0; i < ids.size() && dummyInterface == 0; i++)
    {
        realPluginWasLoadedByMe = pluginManager->LoadSinglePluginNow(ids[i]);
        TRY
        {
            pluginId = ids[i];
            // when creating the file format interface object for the dummy format
            // don't cache it in the MRU cache.
            const bool dontCache = true;
            dummyInterface = GetRealInterface(0, 0, dontCache);
        }
        CATCH2(InvalidDBTypeException, e)
        {
            dummyInterface = 0;
        }
        ENDTRY
    }
    debug5 << "Determined file \"" << dcname << "\" requires plugin id=\"" << pluginId << "\"" << endl;

    // get the database type from the info
    CommonDatabasePluginInfo *info = pluginManager->GetCommonPluginInfo(pluginId);
    if (info == 0)
    {
        char errMsg[1024];
        snprintf(errMsg, sizeof(errMsg),
            "Unable to load info about plugin \"%s\" for file \"%s\"",
            pluginId.c_str(), inputFileList[0][0].c_str());
        EXCEPTION1(InvalidFilesException, errMsg);
    }
    dbType = info->GetDatabaseType();

    // set cache size (number of files we can have decompressed at one time)
    decompressedFilesCache.numslots(maxDecompressedFiles);

    // use temporary ZW format interface object to gain access to format object
    avtZWFileFormatInterface *realzwi = (avtZWFileFormatInterface *) dummyInterface;
    avtFileFormat *realf = realzwi->GetFileFormat(0);

    dummyFileFormat = new avtZipWrapperFileFormat(realf);
}

// ****************************************************************************
//  Method: avtZipWrapper destructor
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
avtZipWrapperFileFormatInterface::~avtZipWrapperFileFormatInterface()
{
    // keep track of objects for CleanUpAtExit
    if (!atExiting)
    {

#if 0 // disabled until we can unload a single plugin
#ifndef MDSERVER
        if (realPluginWasLoadedByMe)
            UnloadSinglePlugin(pluginId);
#endif
#endif

        vector<avtZipWrapperFileFormatInterface*>::iterator it = objList.begin();
        while (it != objList.end() && this != *it)
            it++;
        objList.erase(it);
    }

    // we must explicitly call clear cache here to ensure items in it are
    // deleted *before* we enter Finalize
    decompressedFilesCache.clear();

    delete dummyFileFormat;
    delete dummyInterface;

    // if this is the last instance we have, finalize the class too
    if (objList.size() == 0)
    {
        debug5 << "Calling finalize on \"" << VisItInit::GetComponentName() << "\"" << endl;
        avtZipWrapperFileFormatInterface::Finalize();
    }
}

// ****************************************************************************
//  Method: GetFormat
//
//  Purpose: Implement avtFileFormatInterface's GetFormat method. avtFFI
//  has a number of methods that loop over all formats requesting a format
//  object and then calling a method which queries the format object or passes
//  data to it. The problem with avtFFI's loops is that they result in
//  instantiating all the file format objects (one for each file) before any
//  real work is done on the file(s). This means all the files would need to
//  be decompressed when, most likely, any one file format object will be able
//  to answer all the queries avtFFI makes of it. So, here, we only ever return
//  the dummy file format object. The dummy serves two functions; first it
//  behaves like the real format objects when it is queried from avtFFI. Second,
//  it serves to store information passed to it from avtFFI for later transfer
//  to actual file format objects.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
avtFileFormat *
avtZipWrapperFileFormatInterface::GetFormat(int i) const
{
    return dummyFileFormat;
}

// ****************************************************************************
//  Method: UpdateRealFileFormatInterface
//
//  Purpose: Transfer everything we know has been applied to the dummy file
//  format object to the real file format object. The dummy object hangs
//  around getting updated by avtFileFormatInterface just so we can turn
//  around and query back out of it the stuff that needs to be applied to a
//  real file format object, here.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//    Mark C. Miller, Mon Aug 20 12:48:37 PDT 2007
//    Added code to return early if dummyFileFormat is not already initialized.
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::UpdateRealFileFormatInterface(
    avtFileFormatInterface *iface) const
{
    if (dummyFileFormat == 0)
        return;

    iface->SetCache(dummyFileFormat->GetCache());
    if (dummyFileFormat->GetDummiedMaterialSelection() != "")
        iface->TurnMaterialSelectionOn(dummyFileFormat->GetDummiedMaterialSelection().c_str());
    else
        iface->TurnMaterialSelectionOff();
    iface->RegisterVariableList(dummyFileFormat->GetDummiedRegisterVariableList_var().c_str(),
                                dummyFileFormat->GetDummiedRegisterVariableList_2nd());
    iface->RegisterDataSelections(dummyFileFormat->GetDummiedRegisterDataSelections_sels(),
                                  dummyFileFormat->GetDummiedRegisterDataSelections_apps());
}

// ****************************************************************************
//  Method: GetRealInterface
//
//  Purpose: Given timestep and domain, this method figures out which file in
//  the list of input files is involved, handles decompressing it and
//  instantiating a real file format interface object for it. It uses the
//  Database factory's SetupDatabase method to do this but immediately throws
//  away the database after obtaining the file format interface it is here to
//  create.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//    Mark C. Miller, Mon Aug 20 12:48:37 PDT 2007
//    Added call to UpdateFileFormatInterface just before last return.
//
//    Mark C. Miller, Mon Aug 20 18:25:57 PDT 2007
//    Moved UpdateFileFormatInterface call to just before call to set
//    metadata. Set timestep to construct database for real format at to
//    '-2' telling VisIt not to make any calls on the interface during its
//    construction.
//
//    Brad Whitlock, Tue Jun 24 16:46:55 PDT 2008
//    Pass the plugin manager to the database factory.
//
//    Mark C. Miller, Tue Dec  1 18:39:15 PST 2009
//    Fix regular expressions for parsing extension and decompressed name.
//    Added force options to other compression commands. Added stat and loop
//    logic to help avoid situations where multiple compression commands
//    might somehow be running in background and colliding with each other.
// ****************************************************************************
avtFileFormatInterface *
avtZipWrapperFileFormatInterface::GetRealInterface(int ts, int dom, bool dontCache)
{
    // Sometimes VisIt will issue requests for 'odd' domains and/or timesteps
    // usually with negative values. Typically, this means VisIt doesn't really
    // care for which domain or timestep data is obtained. So, we fix that here.
    if (ts < 0) ts = 0;
    if (dom < 0) dom = 0;
    if (ts > nTimesteps-1) ts = nTimesteps -1;
    if (dom > inputFileBlockCount-1) dom = inputFileBlockCount-1;

    string compressedName = inputFileList[ts][dom];

    if (decompressedFilesCache.exists(compressedName))
    {
        debug5 << "Found interface object for file \"" << compressedName << "\" in cache" << endl;
        DecompressedFileInfo *finfo = decompressedFilesCache[compressedName];
        avtFileFormatInterface *retval = finfo->iface;
        // Always update the interface object to whatever the dummy format thinks
        // is right before returning the object for use.
        UpdateRealFileFormatInterface(retval);
        return retval;
    }
    debug5 << "Interface object for file \"" << compressedName << "\" not in cache" << endl;

    string ext = StringHelpers::ExtractRESubstr(compressedName.c_str(), "<\\.(xz|gz|bz|bz2|zip)$>");
    const char *bname = FileFunctions::Basename(compressedName.c_str());
    string dcname = StringHelpers::ExtractRESubstr(bname, "<(.*)\\.(xz|gz|bz|bz2|zip)$> \\1");

    string dcmd = decompCmd;
    if (dcmd == "")
    {
#ifdef WIN32
        if(PathFileExists("C:\\Program Files\\7-zip\\7z.exe"))
        {
            dcmd = "\"C:\\Program Files\\7-zip\\7z.exe\" x -y";
        }
        else if(PathFileExists("C:\\Program Files (x86)\\7-zip\\7z.exe"))
        {
            dcmd = "\"C:\\Program Files (x86)\\7-zip\\7z.exe\" x -y";
        }
        else
        {
            // Perhaps can install 7-zip with VisIt on windows (eg installdir/7-zip).
            // Then can perform a check here for its existence by calling
            // GetVisItInstallationDirectory().

            // For now, we want to fail out of here.
            EXCEPTION1(InvalidFilesException, "No Decompression command found.");
        }

#else
        // have to handle tar archives first in this logic
        if (ext == ".tar.gz" || ext == ".tgz" ||
            ext == ".tar.bz" || ext == ".tbz" ||
            ext == ".tar.bz2" || ext == ".tbz2" ||
            ext == ".tar.xz" || ext == ".txz")
            dcmd = "tar xvf";
        else if (ext == ".gz")
            dcmd = "gunzip -f";
        else if (ext == ".xz")
            dcmd = "unxz -f";
        else if (ext == ".bz")
            dcmd = "bunzip -f";
        else if (ext == ".bz2")
            dcmd = "bunzip2 -f";
        else if (ext == "zip")
            dcmd = "unzip -o";
#endif
    }

    //
    // We need to guard against situations where the same file could be in
    // process of being decompressed and we are about to initiate a 'new'
    // decompression command on it. So, we use a sort of 'locking' mechanism
    // here by creating '.lck' files just before we begin decompressing and
    // then removing the '.lck' files after the decompression completes. If we
    // get here and find a .lck file for the given target, we loop, stat'ing
    // the target and so long as the target file size is increasing and .lck
    // exists, we wait. If .lck goes away, we know its completed. If file size
    // does NOT continue to grow after several successive stat's, we conclude
    // it is somehow hung.
    //
    off_t tsize = 0;
    int ntries = 199; // must be odd
    FileFunctions::VisItStat_t statbuf;
    int statval = FileFunctions::VisItStat(string(dcname+".lck"), &statbuf);
    if (statval == -1 && errno == ENOENT)
        ntries = 0;

    while (ntries>0)
    {
        // Wait a bit.
#ifdef WIN32
        Sleep(1);
#else
        struct timespec ts = {0, 1000000000/2}; // 1/2-second
        nanosleep(&ts, 0);
#endif

        // Stat the target so we can monitor its size
        errno = 0;
        statval = FileFunctions::VisItStat(dcname, &statbuf);
        if (statval == 0)
        {
            if (statbuf.st_size > tsize)
                tsize = statbuf.st_size;
            else
                ntries-=2;
        }
        else if (errno == ENOENT)
            ntries = 0;
        else
            ntries-=2;

        // Stat the lock. If its gone, we know the previos decomp command
        // completed.
        errno = 0;
        statval = FileFunctions::VisItStat(string(dcname+".lck").c_str(), &statbuf);
        if (statval == -1 && errno == ENOENT)
            ntries = 0;
    }

    if (ntries<0)
    {
        debug5 << "It looks like an existing decompression attempt has hung. But, proceeding anyway." << endl;
    }

#ifdef WIN32
    // no 'touch' command on Windows, so create the empty .lck file first
    char tmpcmd[1024];
    // Create full path
    snprintf(tmpcmd, sizeof(tmpcmd), "%s\\%s.lck",
             tmpDir.c_str(), dcname.c_str());
    ofstream tmpfile(tmpcmd);
    tmpfile.close();

    // don't want to call 'system' like on *nix, because it opens a window.
    // Since there are multiple commands to be executed, create a temporary
    // .bat file
    snprintf(tmpcmd, sizeof(tmpcmd), "%s\\dodecompress.bat", tmpDir.c_str());
    ofstream tp2(tmpcmd);
    tp2 << "@echo off " << endl;
    tp2 << "cd " << tmpDir << endl;
    tp2 << dcmd << " " << compressedName << " >nul 2>&1" << endl;
    tp2 << "del " << dcname << ".lck >nul 2>&1" << endl;
    tp2.close();

    if (_spawnl(_P_WAIT, tmpcmd, tmpcmd, NULL) == -1)
    {
        EXCEPTION1(InvalidFilesException, "Decompression command exited abnormally");
    }
    // delete the .bat file
    if (unlink(tmpcmd) != 0 && errno != ENOENT)
    {
        debug5 << "Unable to unlink() bat file \"" << tmpcmd << "\"" << endl;
        debug5 << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
    }
#else
    char tmpcmd[1024];
    snprintf(tmpcmd, sizeof(tmpcmd), "cd %s ; cp %s . ; touch %s.lck ; %s %s ; rm -f %s.lck",
        tmpDir.c_str(), compressedName.c_str(), dcname.c_str(), dcmd.c_str(), bname, dcname.c_str());
    debug5 << "Using decompression command: \"" << tmpcmd << "\"" << endl;
    int ret = system(tmpcmd);
    if (WIFEXITED(ret))
    {
        if (WEXITSTATUS(ret) != 0)
        {
            char errMsg[1024];
            snprintf(errMsg, sizeof(errMsg), "Decompression command apparently "
                "exited normally but returned non-zero exit status %d", WEXITSTATUS(ret));
            EXCEPTION1(InvalidFilesException, errMsg);
        }
    }
    else
    {
        EXCEPTION1(InvalidFilesException, "Decompression command exited abnormally");
    }
#endif

    string newfname = tmpDir + VISIT_SLASH_STRING + dcname;
    const char *tmpstr = newfname.c_str();

    vector<string> dummyPlugins;
    bool forceReadAllCyclesAndTimes = false;
    bool treatAllDBsAsTimeVarying = false;
    int  timeStepToConstructAt = -2;
    avtDatabase *dummyDatabaseWithRealInterface = avtDatabaseFactory::FileList(
        pluginManager, &tmpstr, 1, timeStepToConstructAt, dummyPlugins, pluginId.c_str(),
        forceReadAllCyclesAndTimes, treatAllDBsAsTimeVarying);
    avtZWGenericDatabase *dummyDatabaseWithRealInterface_tmp =
        (avtZWGenericDatabase *) dummyDatabaseWithRealInterface;
    avtFileFormatInterface *realInterface = dummyDatabaseWithRealInterface_tmp->GetFileFormatInterface();

    delete dummyDatabaseWithRealInterface;


    if (!dontCache)
    {
        DecompressedFileInfo *finfo = new DecompressedFileInfo();
        finfo->iface = realInterface;
        finfo->rmDirname = "";
        if (dcmd.substr(0,5) == "tar x")
            finfo->rmDirname = bname;
        UpdateRealFileFormatInterface(realInterface);
        realInterface->SetDatabaseMetaData(&mdCopy, 0);
        decompressedFilesCache[compressedName] = finfo;
    }

    return realInterface;
}

// ****************************************************************************
//  Method: GetFilename
//
//  Purpose: Implement GetFilename method for avtFileFormat class.
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Add return value.
//
// ****************************************************************************

const char *
avtZipWrapperFileFormatInterface::GetFilename(int ts)
{
    if      (dbType == DB_TYPE_MTMD)
        return inputFileList[0][0].c_str();
    else if (dbType == DB_TYPE_STMD)
        return inputFileList[ts][0].c_str();
    else if (dbType == DB_TYPE_MTSD)
        return inputFileList[0][0].c_str();
    else if (dbType == DB_TYPE_STSD)
        return inputFileList[ts][0].c_str();

    return NULL;
}

// ****************************************************************************
//  Method: FreeUpResources
//
//  Purpose: Remove compressed file associated with specified ts,dom
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::FreeUpResources(int ts, int dom)
{
    // We don't remove from the cache here because that would result in
    // removing decompressed files we still have room for. Instead we
    // let cache pre-emption handle that. It would be nice, though, if
    // we could mark these objects as 'ok' to delete since we know we
    // are not presently using them.
    if      (dbType == DB_TYPE_MTMD)
    {
        GetRealInterface(0,0)->FreeUpResources(ts,dom);
    }
    else if (dbType == DB_TYPE_STMD)
    {
        GetRealInterface(ts,0)->FreeUpResources(0,dom);
        //decompressedFilesCache.remove(inputFileList[ts][0]);
    }
    else if (dbType == DB_TYPE_MTSD)
    {
        GetRealInterface(0,dom)->FreeUpResources(ts,0);
        //decompressedFilesCache.remove(inputFileList[0][dom]);
    }
    else if (dbType == DB_TYPE_STSD)
    {
        GetRealInterface(ts,dom)->FreeUpResources(0,0);
        //decompressedFilesCache.remove(inputFileList[ts][dom]);
    }
}

// ****************************************************************************
//  Method: ActivateTimestep
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::ActivateTimestep(int ts)
{
    if      (dbType == DB_TYPE_MTMD)
        GetRealInterface(0,0)->ActivateTimestep(ts);
    else if (dbType == DB_TYPE_STMD)
        GetRealInterface(ts,0)->ActivateTimestep(0);
    else if (dbType == DB_TYPE_MTSD)
        GetRealInterface(0,0)->ActivateTimestep(ts);
    else if (dbType == DB_TYPE_STSD)
        GetRealInterface(ts,0)->ActivateTimestep(0); // don't loop over doms
}

// ****************************************************************************
//  Method: SetDatabaseMetaData
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::SetDatabaseMetaData(
    avtDatabaseMetaData *md, int ts, bool forceReadAllCyclesTimes)
{
    int j, nm;

    if      (dbType == DB_TYPE_MTMD)
    {
        GetRealInterface(0,0)->SetDatabaseMetaData(md, ts, forceReadAllCyclesTimes);
    }
    else if (dbType == DB_TYPE_STMD)
    {
        GetRealInterface(ts,0)->SetDatabaseMetaData(md, 0, forceReadAllCyclesTimes);
        md->SetNumStates(nTimesteps);
    }
    else if (dbType == DB_TYPE_MTSD)
    {
        GetRealInterface(0,0)->SetDatabaseMetaData(md, ts, forceReadAllCyclesTimes);
        nm = md->GetNumMeshes();
        for (j = 0 ; j < nm ; j++)
            md->SetBlocksForMesh(j, inputFileBlockCount);
        if (inputFileBlockCount > 1)
            md->UnsetExtents();
    }
    else if (dbType == DB_TYPE_STSD)
    {
        GetRealInterface(ts,0)->SetDatabaseMetaData(md, 0, forceReadAllCyclesTimes);
        md->SetNumStates(nTimesteps);
        nm = md->GetNumMeshes();
        for (j = 0 ; j < nm ; j++)
            md->SetBlocksForMesh(j, inputFileBlockCount);
        if (inputFileBlockCount > 1)
            md->UnsetExtents();
    }

    mdCopy = *md;
}

// ****************************************************************************
//  Method: SetCycleTimeInDatabaseMetaData
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::SetCycleTimeInDatabaseMetaData(
    avtDatabaseMetaData *md, int ts)
{
    if      (dbType == DB_TYPE_MTMD)
        GetRealInterface(0, 0)->SetCycleTimeInDatabaseMetaData(md, ts);
    else if (dbType == DB_TYPE_STMD)
        GetRealInterface(ts, 0)->SetCycleTimeInDatabaseMetaData(md, 0);
    else if (dbType == DB_TYPE_MTSD)
        GetRealInterface(0, 0)->SetCycleTimeInDatabaseMetaData(md, ts);
    else if (dbType == DB_TYPE_STSD)
        GetRealInterface(ts, 0)->SetCycleTimeInDatabaseMetaData(md, 0);
}

// ****************************************************************************
//  Method: GetMesh
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Add return value.
//
// ****************************************************************************

vtkDataSet *
avtZipWrapperFileFormatInterface::GetMesh(int ts, int dom, const char *meshname)
{
    if      (dbType == DB_TYPE_MTMD)
        return GetRealInterface(0,0)->GetMesh(ts,dom,meshname);
    else if (dbType == DB_TYPE_STMD)
        return GetRealInterface(ts,0)->GetMesh(0,dom,meshname);
    else if (dbType == DB_TYPE_MTSD)
        return GetRealInterface(0,dom)->GetMesh(ts,0,meshname);
    else if (dbType == DB_TYPE_STSD)
        return GetRealInterface(ts,dom)->GetMesh(0,0,meshname);
    return NULL;
}

// ****************************************************************************
//  Method: GetAuxiliaryData
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Add return value.
//
// ****************************************************************************

void *
avtZipWrapperFileFormatInterface::GetAuxiliaryData(const char *var, int ts, int dom,
    const char *type, void *args, DestructorFunction &df)
{
    if      (dbType == DB_TYPE_MTMD)
        return GetRealInterface(0,0)->GetAuxiliaryData(var, ts, dom, type, args, df);
    else if (dbType == DB_TYPE_STMD)
        return GetRealInterface(ts,0)->GetAuxiliaryData(var, 0, dom, type, args, df);
    else if (dbType == DB_TYPE_MTSD)
        return GetRealInterface(0,dom)->GetAuxiliaryData(var, ts, 0, type, args, df);
    else if (dbType == DB_TYPE_STSD)
        return GetRealInterface(ts,dom)->GetAuxiliaryData(var, 0, 0, type, args, df);
    return NULL;
}

// ****************************************************************************
//  Method: avtZipWrapperFileFormatInterface::CreateCacheNAmeIncludingSelections
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Hank Childs
//  Creation:   December 20, 2011
//
// ****************************************************************************

std::string
avtZipWrapperFileFormatInterface::CreateCacheNameIncludingSelections(std::string v, int ts, int dom)
{
    if      (dbType == DB_TYPE_MTMD)
        return GetRealInterface(0,0)->CreateCacheNameIncludingSelections(v, ts, dom);
    else if (dbType == DB_TYPE_STMD)
        return GetRealInterface(ts,0)->CreateCacheNameIncludingSelections(v, 0, dom);
    else if (dbType == DB_TYPE_MTSD)
        return GetRealInterface(0,dom)->CreateCacheNameIncludingSelections(v, ts, 0);
    else if (dbType == DB_TYPE_STSD)
        return GetRealInterface(ts,dom)->CreateCacheNameIncludingSelections(v, 0, 0);
    return v;
}

// ****************************************************************************
//  Method: GetVar
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Add return value.
//
// ****************************************************************************

vtkDataArray *
avtZipWrapperFileFormatInterface::GetVar(int ts, int dom, const char *varname)
{
    if      (dbType == DB_TYPE_MTMD)
        return GetRealInterface(0,0)->GetVar(ts,dom,varname);
    else if (dbType == DB_TYPE_STMD)
        return GetRealInterface(ts,0)->GetVar(0,dom,varname);
    else if (dbType == DB_TYPE_MTSD)
        return GetRealInterface(0,dom)->GetVar(ts,0,varname);
    else if (dbType == DB_TYPE_STSD)
        return GetRealInterface(ts,dom)->GetVar(0,0,varname);
    return NULL;
}

// ****************************************************************************
//  Method: GetVectorVar
//
//  Purpose: Forward call to real file format interface
//
//  Programmer: Mark C. Miller
//  Creation:   July 31, 2007
//
//  Modifications:
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Add return value.
//
// ****************************************************************************

vtkDataArray *
avtZipWrapperFileFormatInterface::GetVectorVar(int ts, int dom, const char *varname)
{
    if      (dbType == DB_TYPE_MTMD)
        return GetRealInterface(0,0)->GetVectorVar(ts,dom,varname);
    else if (dbType == DB_TYPE_STMD)
        return GetRealInterface(ts,0)->GetVectorVar(0,dom,varname);
    else if (dbType == DB_TYPE_MTSD)
        return GetRealInterface(0,dom)->GetVectorVar(ts,0,varname);
    else if (dbType == DB_TYPE_STSD)
        return GetRealInterface(ts,dom)->GetVectorVar(0,0,varname);
    return NULL;
}
