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

// ************************************************************************* //
//                    avtZipWrapperFileFormatInterface.C                     //
// ************************************************************************* //
#include <avtZipWrapperFileFormatInterface.h>

#include <snprintf.h>
#include <DatabasePluginInfo.h>
#include <DatabasePluginManager.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidDBTypeException.h>
#include <InvalidFilesException.h>
#include <Init.h>
#include <StringHelpers.h>

#include <avtDatabase.h>
#include <avtDatabaseFactory.h>
#include <avtDatabaseMetaData.h>
#include <avtFileFormatInterface.h>
#include <avtGenericDatabase.h>
#include <avtMTMDFileFormat.h>
#include <avtParallel.h>

#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <string>
#include <vector>
#include <typeinfo>

using std::string;
using std::vector;

string avtZipWrapperFileFormatInterface::tmpDir;
string avtZipWrapperFileFormatInterface::decompCmd;
vector<avtZipWrapperFileFormatInterface*> avtZipWrapperFileFormatInterface::objList;
int avtZipWrapperFileFormatInterface::maxDecompressedFiles = 50;
static bool atExiting = false;

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
    bool                   HasInvariantMetaData()
                               { return realFileFormat->HasInvariantMetaData(); };
    bool                   HasInvariantSIL()
                               { return realFileFormat->HasInvariantSIL(); };
    bool                   CanCacheVariable(const char *v)
                               { return realFileFormat->CanCacheVariable(v); };
    const char            *GetType(void)
                               { return "ZipWrapperDummy"; }; 
    bool                   HasVarsDefinedOnSubMeshes()
                               { return realFileFormat->HasVarsDefinedOnSubMeshes(); }; 
    bool                   PerformsMaterialSelection()
                               { return realFileFormat->PerformsMaterialSelection(); }; 
    bool                   CanDoDynamicLoadBalancing()
                               { return realFileFormat->CanDoDynamicLoadBalancing(); };

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
    avtZWFileFormatInterface *iface = (avtZWFileFormatInterface*) item;
    string filename = iface->GetFilename(0);
    debug5 << "Removing decompressed file \"" << filename << "\"" << endl;
    delete iface;
    errno = 0;
    if (unlink(filename.c_str()) != 0 && errno != ENOENT)
    {
        static int issuedWarning = 0;
	if (issuedWarning < 5)
	{
	    debug5 << "Unable to unlink() decompressed file \"" << filename << "\"" << endl;
	    debug5 << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
	    cerr << "Unable to remove decompressed file \"" << filename << "\"" << endl;
	    cerr << "unlink() reported errno=" << errno << " (\"" << strerror(errno) << "\")" << endl;
	    issuedWarning++;
        }
    }
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
//  Programmer: Mark C. Miller 
//  Creation:   July 31, 2007 
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::CleanUpAtExit()
{
    // delete all the zip wrapper objects that still exist
    //debug5 << "At-Exiting, deleting " << objList.size() << " objects." << endl;
    atExiting = true;
    for (int i = 0; i < objList.size(); i++)
        delete objList.at(i);
    if (objList.size() != 0)
    {
	//debug5 << "ZipWrapper may have left decompressed files" << endl;
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
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::Initialize(int procNum, int procCount)
{

#ifndef MDSERVER
    // See note in CleanUpAtExit 
    atexit(avtZipWrapperFileFormatInterface::CleanUpAtExit);
#endif

    // Decide on root temporary directory
    char *usrtmp = "/usr/tmp";
    char *tmpdir = getenv("VISIT_ZIPWRAPPER_TMPDIR");
    if (tmpdir == 0)
        tmpdir = getenv("TMPDIR");
    if (tmpdir == 0)
        tmpdir = getenv("HOME");
    if (tmpdir == 0)
        tmpdir = usrtmp;

    // Decide on user's name
    char *username = getenv("VISIT_ZIPWRAPPER_USER");
    if (username == 0)
        username = getenv("USER");
    if (username == 0)
        username = getenv("USERNAME");
    string userName = username ? string(username) : "user";

    char procNumStr[32];
    SNPRINTF(procNumStr, sizeof(procNumStr), "_%04d", procNum);
    tmpDir = string(tmpdir) + "/visitzw_" + userName + "_" +
             string(Init::GetComponentName()) +
	     (procCount > 1 ? string(procNumStr) : "");

    // Get maximum file count
    maxDecompressedFiles /= procCount;
    char *maxfiles = getenv("VISIT_ZIPWRAPPER_MAXFILES");
    if (maxfiles)
    {
	errno = 0;
        int tmpInt = (int) strtol(maxfiles, 0, 10);
	if (errno == 0)
	{
	    if (tmpInt < 0)
                maxDecompressedFiles = -tmpInt;
	    else
                maxDecompressedFiles = tmpInt / procCount;
        }
	else
	{
	    cerr << "Unable to set maximum file count to \"" << maxfiles << "\"" << endl;
	}
    }
    debug5 << "ZipWrapper will maintain a maximum of " << maxDecompressedFiles
           << " decompressed files " << (procCount > 1 ? "per-processor" : "")
	   << " at any one time." << endl;

    char *decomp_cmd = getenv("VISIT_ZIPWRAPPER_DCMD");
    if (decomp_cmd)
        decompCmd = string(decomp_cmd);

    // Make the temporary directory
    // (will have different name on mdserver and engine)
    debug5 << "ZipWrapper is using \"" << tmpDir << "\" as the temporary directory" << endl;
    errno = 0;
    if (mkdir(tmpDir.c_str(), 0777) != 0 && errno != EEXIST)
    {
        static char errMsg[1024];
	SNPRINTF(errMsg, sizeof(errMsg), "mkdir failed with errno=%d (\"%s\")",
	    errno, strerror(errno));
        EXCEPTION1(InvalidFilesException, errMsg);
    }
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
    if (rmdir(tmpDir.c_str()) != 0 && errno != ENOENT)
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
// ****************************************************************************
avtZipWrapperFileFormatInterface::avtZipWrapperFileFormatInterface(
    const char *const *list, int nl, int nb) : 
    inputFileListSize(nl), inputFileBlockCount(nb),
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
        avtZipWrapperFileFormatInterface::Initialize(procNum, procCount);
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
    string ext = StringHelpers::ExtractRESubstr(inputFileList[0][0].c_str(), "<\\.gz$|\\.bz$|\\.bz2$|\\.zip$>");
    const char *bname = StringHelpers::Basename(inputFileList[0][0].c_str());
    string dcname = StringHelpers::ExtractRESubstr(bname, "<(.*)\\.gz$|\\.bz$|\\.bz2$|\\.zip$> \\1");
    DatabasePluginManager *dbmgr = DatabasePluginManager::Instance();

    //
    // Find right plugin, load it and open the first file.
    // Note: matching plugins that don't open the file get loaded
    // and never unloaded.
    //
    pluginId = "";
    dummyInterface = 0;
    const bool searchAllPlugins = true;
    vector<string> ids = dbmgr->GetMatchingPluginIds(dcname.c_str(), searchAllPlugins);
    for (int i = 0; i < ids.size() && dummyInterface == 0; i++)
    {
        realPluginWasLoadedByMe = dbmgr->LoadSinglePluginNow(ids[i]);
        TRY
        {
            // when creating the file format interface object for the dummy format
            // don't cache it in the MRU cache.
            const bool dontCache = true;
            dummyInterface = GetRealInterface(0, 0, dontCache);
	    pluginId = ids[i];
        }
        CATCH2(InvalidDBTypeException, e)
        {
            dummyInterface = 0;
        }
        ENDTRY
    }
    debug5 << "Determined file \"" << dcname << "\" requires plugin id=\"" << pluginId << "\"" << endl;

    // get the database type from the info
    CommonDatabasePluginInfo *info = dbmgr->GetCommonPluginInfo(pluginId);
    if (info == 0)
    {
        char errMsg[1024];
	SNPRINTF(errMsg, sizeof(errMsg),
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

    // We use FreeUpCacheSlot here even though dummyInterface isn't cached
    // because FreeUpCacheSlot is where all the logic for deleting the
    // decompressed file associated with an interface resides.
    FreeUpCacheSlot(dummyInterface);

    // if this is the last instance we have, finalize the class too
    if (objList.size() == 0)
    {
	debug5 << "Calling finalize on \"" << Init::GetComponentName() << "\"" << endl;
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
// ****************************************************************************
void
avtZipWrapperFileFormatInterface::UpdateRealFileFormatInterface(avtFileFormatInterface *iface) const
{
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
        avtFileFormatInterface *retval = decompressedFilesCache[compressedName];
	// Always update the interface object to whatever the dummy format thinks
	// is right before returning the object for use.
	UpdateRealFileFormatInterface(retval);
        return retval; 
    }
    debug5 << "Interface object for file \"" << compressedName << "\" not in cache" << endl;

    string ext = StringHelpers::ExtractRESubstr(compressedName.c_str(), "<\\.gz$|\\.bz$|\\.bz2$|\\.zip$>");
    const char *bname = StringHelpers::Basename(compressedName.c_str());
    string dcname = StringHelpers::ExtractRESubstr(bname, "<(.*)\\.gz$|\\.bz$|\\.bz2$|\\.zip$> \\1");

    string dcmd = decompCmd;
    if (dcmd == "")
    {
        if (ext == ".gz")
            dcmd = "gunzip -f";
        else if (ext == ".bz")
            dcmd = "bunzip";
        else if (ext == ".bz2")
            dcmd = "bunzip2";
        else if (ext == "zip")
            dcmd = "unzip";
    }

    char tmpcmd[1024];
    SNPRINTF(tmpcmd, sizeof(tmpcmd), "cd %s ; cp %s . ; %s %s", tmpDir.c_str(), compressedName.c_str(), dcmd.c_str(), bname);
    debug5 << "Using decompression command: \"" << tmpcmd << "\"" << endl;
    int ret = system(tmpcmd);
    if (WIFEXITED(ret))
    {
        if (WEXITSTATUS(ret) != 0)
	{
            char errMsg[1024];
	    SNPRINTF(errMsg, sizeof(errMsg), "Decompression command apparently "
	        "exited normally but returned non-zero exit status %d", WEXITSTATUS(ret));
            EXCEPTION1(InvalidFilesException, errMsg);
	}
    }
    else
    {
        EXCEPTION1(InvalidFilesException, "Decompression command exited abnormally");
    }

    string newfname = tmpDir + "/" + dcname;
    const char *tmpstr = newfname.c_str();

    vector<string> dummyPlugins;
    bool forceReadAllCyclesAndTimes = false;
    bool treatAllDBsAsTimeVarying = false;
    avtDatabase *dummyDatabaseWithRealInterface = avtDatabaseFactory::FileList(&tmpstr,
        1, 0, dummyPlugins, pluginId.c_str(),
        forceReadAllCyclesAndTimes, treatAllDBsAsTimeVarying);
    avtZWGenericDatabase *dummyDatabaseWithRealInterface_tmp =
        (avtZWGenericDatabase *) dummyDatabaseWithRealInterface;
    avtFileFormatInterface *realInterface = dummyDatabaseWithRealInterface_tmp->GetFileFormatInterface();

    delete dummyDatabaseWithRealInterface;

    realInterface->SetDatabaseMetaData(&mdCopy, 0);
    if (!dontCache)
        decompressedFilesCache[compressedName] = realInterface;

    return realInterface;
}

// ****************************************************************************
//  Method: GetFilename 
//
//  Purpose: Implement GetFilename method for avtFileFormat class.
//
//  Programmer: Mark C. Miller 
//  Creation:   July 31, 2007 
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
}

// ****************************************************************************
//  Method: GetAuxiliaryData 
//
//  Purpose: Forward call to real file format interface 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 31, 2007 
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
}

// ****************************************************************************
//  Method: GetVar 
//
//  Purpose: Forward call to real file format interface 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 31, 2007 
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
}

// ****************************************************************************
//  Method: GetVectorVar 
//
//  Purpose: Forward call to real file format interface 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 31, 2007 
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
}
