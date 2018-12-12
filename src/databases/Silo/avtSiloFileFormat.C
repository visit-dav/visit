/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                           avtSiloFileFormat.C                             //
// ************************************************************************* //
#include <avtSiloFileFormat.h>
#include <avtSiloOptions.h>

// includes from visit_vtk/full
#ifndef MDSERVER
#include <vtkCSGGrid.h>
#endif

#include <float.h>

#include <vtkBitArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIdList.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkLongLongArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRectilinearGrid.h>
#include <vtkShortArray.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVisItUtility.h>

#include <avtCallback.h>
#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtFacelist.h>
#include <avtGhostData.h>
#include <avtIntervalTree.h>
#include <avtIOInformation.h>
#include <avtMaterial.h>
#include <avtMixedVariable.h>
#include <avtResampleSelection.h>
#include <avtSpecies.h>
#include <avtStructuredDomainBoundaries.h>
#include <avtStructuredDomainNesting.h>
#include <avtTypes.h>
#include <avtVariableCache.h>

#include <BJHash.h>
#include <Utility.h>
#include <Expression.h>
#include <FileFunctions.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <BufferConnection.h>
#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <InvalidZoneTypeException.h>
#include <SiloException.h>
#include <avtExecutionManager.h>

#include <avtStructuredDomainBoundaries.h>

#include <visit-config.h>

#include <cerrno>
#include <sstream>
#include <snprintf.h>
#include <stdlib.h> // for qsort

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

#include <map>
#include <set>
#include <string>
#include <vector>


using std::map;
using std::set;
using std::string;
using std::vector;
using std::ostringstream;
using namespace SiloDBOptions;

static void      ExceptionGenerator(char *);
static char     *GenerateName(const char *, const char *, const char *);
static string    PrepareDirName(const char *, const char *);
static void      SplitDirVarName(const char *dirvar, const char *curdir,
                                 string &dir, string &var);

static void      AddDefvars(const char *, avtDatabaseMetaData *);

static int  SiloZoneTypeToVTKZoneType(int);
static void TranslateSiloWedgeToVTKWedge(const int *, vtkIdType [6]);
static void TranslateSiloPyramidToVTKPyramid(const int *, vtkIdType [5]);
static void TranslateSiloTetrahedronToVTKTetrahedron(const int *,
                                                     vtkIdType [4]);
static bool TetIsInverted(const int *siloTetrahedron,
                            vtkUnstructuredGrid *ugrid);

static void ArbInsertArbitrary(vtkUnstructuredGrid *ugrid,
    int nsdims, DBphzonelist *phzl, int gz, const vector<int> &nloffs,
    const vector<int> &floffs, unsigned int ocdata[2],
    vector<int> *cellReMap, vector<int> *nodeReMap);

static string GuessCodeNameFromTopLevelVars(DBfile *dbfile);
static void AddAle3drlxstatEnumerationInfo(avtScalarMetaData *smd);

static string ResolveSiloIndObjAbsPath(DBfile *dbfile,
    string primary_objname_incl_any_abs_or_rel_path,
    string indirect_objname_incl_any_abs_or_rel_path);

static DBgroupelmap * 
GetCondensedGroupelMap(DBfile *dbfile, string mrgtnm_abspath,
    DBmrgtnode *rootNode, int forceSingle, int gpel_type);
static void HandleMrgtreeNodelistVars(DBfile *dbfile, const string& mname,
    const string& tname, avtDatabaseMetaData *md);
static void HandleMrgtreeAMRGroups(DBfile *dbfile, DBmultimesh *mm,
    const char *multimesh_name, avtMeshType *mt, int *num_groups,
    vector<int> *group_ids, vector<string> *block_names, int forceSingle);
static void BuildDomainAuxiliaryInfoForAMRMeshes(DBfile *dbfile, DBmultimesh *mm,
    const char *meshName, int timestate, int type, avtVariableCache *cache,
    int forceSingle);

static int MultiMatHasAllMatInfo(const DBmultimat *const mm);
static vtkDataArray *CreateDataArray(int silotype, void *data, int numvals);

static int FindFirstNonEmptyBlock(char const *mbobj_name, int nblocks,
    avtSiloMBObjectCacheEntry const *mb_ent,
    int repr_block_idx, int empty_cnt, int const *empty_list);

static int db_get_index(DBnamescheme const *ns, int natnum);

// ****************************************************************************
//  Class: avtSiloFileFormat
//
//  Purpose:
//      A Silo file format reader.
//
//  Arguments:
//      fnames  The filenames
//      nf      The number of files.
//
//  Notes:      It is assumed that the files are sorted so that the one with
//              the multimesh/multivars/multimaterials comes first.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Apr  9 12:54:31 PDT 2001
//    Have Silo throw an exception whenever a problem occurs.
//
//    Hank Childs, Mon Apr  7 18:04:30 PDT 2003
//    Do not make the global Silo calls repeatedly.
//
//    Hank Childs, Sat Mar  5 19:13:05 PST 2005
//    Don't do dynamic load balancing where we are in parallel.
//
//    Mark C. Miller, Mon Jun 12 22:22:38 PDT 2006
//    Enabled Silo's checksums
//
//    Mark C. Miller, Wed Nov 29 15:08:21 PST 2006
//    Initialized connectivityIsTimeVarying
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added ability to turn forcing single precision off to support testing
//
//    Mark C. Miller, Wed May 30 08:25:05 PDT 2007
//    Added this bogus comment to test svn update behavior
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Mark C. Miller, Fri Apr 25 21:06:27 PDT 2008
//    Initialize numNodeLists and broadcast info about nodelists if we
//    have any.
//
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Added read options, re-organized the routine a bit. Fixed some
//    seriously bogus code I had added for controlling force single behavior.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added logic to ignore spatial/data extents.
//
//    Mark C. Miller, Wed Mar  4 08:54:57 PST 2009
//    Improved logic to handle ignoring of spatial/data extents so that user
//    can override explicitly or let plugin handle automatically.
//
//    Mark C. Miller, Wed Mar  4 12:05:45 PST 2009
//    Made option processing for extents compatible with 'old' way of doing
//    them.
//
//    Mark C. Miller, Mon Mar 16 23:33:32 PDT 2009
//    Moved logic for 'old' extents interface to CommonPluginInfo where 
//    old (obsolete) options can be merged with current interface. Also, use
//    const char* symbol names for options defined in avtSiloOptions.h.
//
//    Mark C. Miller, Tue Dec 15 14:01:48 PST 2009
//    Added metadataIsTimeVarying
//
//    Mark C. Miller, Thu Jan  7 16:22:58 PST 2010
//    Made it always call DBForceSingle so that you couldn't get into a
//    situation where you opened a file with dontForceSingle false and later
//    set dontForceSingle to true but would then neglect to call
//    DBForceSingle unsetting the value.
//
//    Cyrus Harrison, Wed Mar 24 10:39:30 PDT 2010
//    Added init of haveAmrGroupInfo.
//
//    Mark C. Miller, Mon Mar 29 17:20:48 PDT 2010
//    Added siloDriver initialization. Set Silo's error level to DB_TOP
//    instead of DB_ALL. This is because in new versions of Silo library
//    DB_ALL will spew HDF5 diagnostics to stderr too.
//
//    Mark C. Miller Tue Mar 30 16:28:48 PDT 2010
//    Temporarily reset to using DB_ALL error level as DB_TOP is causing hangs.
//
//    Cyrus Harrison, Thu Jun 10 16:15:36 PDT 2010
//    Changed default init of 'metadataIsTimeVarying' member to true.
//    This helps to deal w/ a chicken & egg senairo when detecting if a silo
//    dataset has time varying metadata.
//
//    Cyrus Harrison, Mon Jun 14 15:25:48 PDT 2010
//    Reverted previous change to init of 'metadataIsTimeVarying' & added
//    init of 'metadataIsTimeVaryingCheck'.
//
//    Mark C. Miller, Wed Jul 21 12:18:17 PDT 2010
//    Added logic to ignore force single behavior for older versions of Silo.
//
//    Mark C. Miller, Thu Apr 12 23:06:24 PDT 2012
//    Changed maxAnnotIntLists to numAnnotIntLists.
//
//    Brad Whitlock, Sat Apr 21 23:35:34 PDT 2012
//    Change to forceSingle so it's easier to understand.
//
//    Cyrus Harrison, Wed Mar 13 09:22:30 PDT 2013
//    Init useLocalDomainBoundries.
//
//    Brad Whitlock, Thu Jun 19 11:38:46 PDT 2014
//    Set ioInfoValid.
//
// ****************************************************************************

avtSiloFileFormat::avtSiloFileFormat(const char *toc_name,
                                     DBOptionsAttributes *rdatts)
    : avtSTMDFileFormat(&toc_name, 1)
{
    //
    // Initialize class variables BEFORE processing read options 
    //
    forceSingle = 0;
    numNodeLists = 0;
    numAnnotIntLists = 0;
    tocIndex = 0; 
    ignoreSpatialExtentsAAN = Auto;
    ignoreDataExtentsAAN = Auto;
    ignoreSpatialExtents = false;
    ignoreDataExtents = false;
    searchForAnnotInt = false;
    readGlobalInfo = false;
    connectivityIsTimeVarying = false;
    metadataIsTimeVarying = false;
    metadataIsTimeVaryingChecked = false;
    groupInfo.haveGroups = false;
    haveAmrGroupInfo = false;
    useLocalDomainBoundries = false;
    hasDisjointElements = false;
    ioInfoValid = false;
    addBlockDecompositionAsVar = false;
    haveAddedBlockDecompositionAsVar = false;
    ignoreMissingBlocks = false;
    topDir = "/";
    siloDriver = DB_UNKNOWN;
    dbfiles = new DBfile*[MAX_FILES];
    for (int i = 0 ; i < MAX_FILES ; i++)
    {
        dbfiles[i] = NULL;
    }

    //
    // Process any read options, potentially overriding default behaviors
    //
    for (int i = 0; rdatts != 0 && i < rdatts->GetNumberOfOptions(); ++i)
    {
        if (rdatts->GetName(i) == SILO_RDOPT_FORCE_SINGLE)
            forceSingle = rdatts->GetBool(SILO_RDOPT_FORCE_SINGLE) ? 1 : 0;
        else if (rdatts->GetName(i) == SILO_RDOPT_SEARCH_ANNOTINT)
            searchForAnnotInt = rdatts->GetBool(SILO_RDOPT_SEARCH_ANNOTINT);
        else if (rdatts->GetName(i) == SILO_RDOPT_IGNORE_SEXTS)
            ignoreSpatialExtentsAAN = (AANTriState) rdatts->GetEnum(SILO_RDOPT_IGNORE_SEXTS);
        else if (rdatts->GetName(i) == SILO_RDOPT_IGNORE_DEXTS)
            ignoreDataExtentsAAN = (AANTriState) rdatts->GetEnum(SILO_RDOPT_IGNORE_DEXTS);
        else
            debug1 << "Ignoring unknown option \"" << rdatts->GetName(i) << "\"" << endl;
    }

    //
    // Set any necessary Silo library behavior 
    //
    DBForceSingle(forceSingle);
    
    //
    // If there is ever a problem with Silo, we want it to throw an
    // exception.
    //
    DBShowErrors(DB_ALL, ExceptionGenerator);

    //
    // Turn on silo's checksumming feature. This is harmless if the
    // underlying file DOES NOT contain checksums and will cause only
    // a small performance hit if it does.
    //
    DBSetEnableChecksums(1);
}


// ****************************************************************************
//  Method: avtSiloFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Jan  7 18:51:24 PST 2002
//    Fixed memory leak.
//
//    Kathleen Bonnell, Wed Oct  1 17:08:51 PDT 2003 
//    Call CloseFile so that files are unregistered. 
//
//    Hank Childs, Wed Jan 14 11:58:41 PST 2004
//    Use CleanUpResources so there can be one routine that does all the clean
//    up.
//
// ****************************************************************************

avtSiloFileFormat::~avtSiloFileFormat()
{
    FreeUpResources();
    delete [] dbfiles;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::ActivateTimestep
//
//  Purpose: Provides a guarenteed collective entry point for operations
//    that may involve collective parallel communication. Each of the GetMesh,
//    GetVar, etc. calls in this file can, in turn, call OpenFile. However,
//    if the file hasn't been opened with a previous call, here, those calls
//    can lead to strange failures because not all processors make calls
//    to GetMesh, GetVar, etc. All processors do call this method.
//
//  Programmer: Mark C. Miller
//  Creation:   February 9, 2004 
//
//  Modifications:
//    Cyrus Harrison, Mon Jun 14 15:34:22 PDT 2010
//    Added check for time varying metadata.
//
// ****************************************************************************
void
avtSiloFileFormat::ActivateTimestep(void)
{
    DBfile *toc = OpenFile(tocIndex);
    // note:
    // We actually create a new instance of the file format class for every
    // timestep. For each instance the 'metadataIsTimeVaryingChecked'
    // starts out as false so this flag really only saves us work if a timestep
    // is viewed more than once.
    if(!metadataIsTimeVaryingChecked)
    {
        CheckForTimeVaryingMetadata(toc);
        metadataIsTimeVaryingChecked = true;
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetFile
//
//  Purpose: Get file at specified index, assuming its already been opened
//  and throw an exception if it hasn't. This is intended to replace various
//  calls to OpenFile, that could ultimately result in MPI collective 
//  communication if the file had not been opened in the past, and could
//  cause VisIt to deadlock.
//
//  Programmer: Mark C. Miller 
//  Creation:   February 10, 2004
//
//  Modifications:
//    Mark C. Miller, Tue Mar  9 08:44:23 PST 2004
//    Added missing call to UsedFile
//
// ****************************************************************************

DBfile *
avtSiloFileFormat::GetFile(int f)
{
    if (dbfiles[f] == NULL)
    {
        EXCEPTION1(InvalidFilesException, filenames[f]);
    }

    UsedFile(f);
    return dbfiles[f];
}

// ****************************************************************************
//  Method: avtSiloFileFormat::OpenFile
//
//  Purpose:
//      Opens a Silo file.
//
//  Arguments:
//      f       The index of the file in our array.
//
//  Returns:    A handle to the open file.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 17:29:06 PST 2002
//    Made call to GetTimeVaryingInformation.
//    
//    Hank Childs, Fri Mar 22 10:32:33 PST 2002
//    Added calls to support file descriptor management.
//
//    Hank Childs, Thu Apr 10 08:45:44 PDT 2003
//    Initialize global information.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Added bool to skip global information
//
//    Mark C. Miller, Wed Dec  8 17:12:28 PST 2004
//    Changed open to try explicit silo drivers; PDB first, then HDF5
//    then everything else
//
//    Mark C. Miller, Tue Feb 13 16:24:58 PST 2007
//    Made it fail if the file it opened didn't look like a silo file
//
//    Mark C. Miller, Wed Feb  6 12:27:09 PST 2008
//    Made it handle the case a single silo file contains the whole time
//    series. In this case, the filename passed here contains a ':'
//    separating the name of the file as the filesystem sees it and the dir
//    in the file to be used as the 'top dir' for this timestep. Note also,
//    That there has to exist in the filesystem a real file (usually a symlink)
//    named as <filename>:<dirname> that opens to the desired file. 
//
//    Jeremy Meredith, Thu Aug  7 16:16:34 EDT 2008
//    Added missing filename argument to an sprintf.
//
//    Mark C. Miller, Fri Jan 30 12:48:09 PST 2009
//    Changed order of opens from PDB then HDF5 to HDF5 then PDB.
//
//    Hank Childs, Sun May 10 10:56:22 CDT 2009
//    Switch back ordering of drivers (PDB to HDF5), since HDF5 driver chokes
//    when dealing with large PDB files (>2GB).
//
//    Hank Childs, Tue Sep 22 20:47:16 PDT 2009
//    Remove assumption of new version of Silo library.
//
//    Mark C. Miller, Mon Mar 29 17:27:07 PDT 2010
//    Set siloDriver that succeeded in opening the file.
//
//    Mark C. Miller Tue Mar 30 16:28:48 PDT 2010
//    Fix DBOpen logic for DB_UNKNOWN case to test for non-NULL result.
//
//    Mark C. Miller, Thu Jan  6 17:17:25 PST 2011
//    Set driver type for unknown case by asking file what its type was.
//
//    Mark C. Miller, Tue May 23 10:28:07 PDT 2017
//    Add logic to use siloDriver to open if its not DB_UNKNOWN.
// ****************************************************************************

DBfile *
avtSiloFileFormat::OpenFile(int f, bool skipGlobalInfo)
{
    //
    // Make sure this is in range.
    //
    if (f < 0 || f >= nFiles)
    {
        EXCEPTION2(BadIndexException, f, nFiles);
    }

    //
    // Check to see if the file is already open.
    //
    if (dbfiles[f] != NULL)
    {
        UsedFile(f);
        return dbfiles[f];
    }

    debug4 << "Opening silo file \"" << filenames[f] << "\"" << endl;

    //
    // Open the Silo file. Impose priority order on drivers by first
    // trying HDF5, then PDB, then fall-back to UNKNOWN
    //
    if ((siloDriver != DB_UNKNOWN) &&
        (dbfiles[f] = DBOpen(filenames[f], siloDriver, DB_READ)) != NULL)
    {
        if (siloDriver == DB_PDB)
            debug1 << "Opened with DB_PDB driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
        else if (siloDriver == DB_HDF5)
            debug1 << "Opened with DB_HDF5 driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
        else
            debug1 << "Opened with DB_UNKNOWN driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
    }
    else if ((dbfiles[f] = DBOpen(filenames[f], DB_HDF5, DB_READ)) != NULL)
    {
        debug1 << "Opened with DB_HDF5 driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
        siloDriver = DB_HDF5;
    }
    else if ((dbfiles[f] = DBOpen(filenames[f], DB_PDB, DB_READ)) != NULL)
    {
        debug1 << "Opened with DB_PDB driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
        siloDriver = DB_PDB;
    }
    else if ((dbfiles[f] = DBOpen(filenames[f], DB_UNKNOWN, DB_READ)) != NULL)
    {
        debug1 << "Opened with DB_UNKNOWN driver; lib=" << DBVersion() << ", file=" << DBFileVersion(dbfiles[f]) <<endl;
        siloDriver = DBGetDriverType(dbfiles[f]);
    }
    else
    {
        EXCEPTION1(InvalidFilesException, filenames[f]);
    }

    //
    // Lets try to make absolutely sure this is really a Silo file and
    // not just a PDB file that PD_Open succeeded on.
    //
    bool hasSiloLibInfo = DBInqVarExists(dbfiles[f], "_silolibinfo");
    if (!hasSiloLibInfo) // newer silo files have this
    {
        //
        // See how many silo objects we have
        //
        DBtoc *toc = DBGetToc(dbfiles[f]);
        int nSiloObjects =
            toc->ncurve + toc->ncsgmesh + toc->ncsgvar + toc->ndefvars +
            toc->nmultimesh + toc->nmultimeshadj + toc->nmultivar +
            toc->nmultimat + toc->nmultimatspecies + toc->nqmesh +
            toc->nqvar + toc->nucdmesh + toc->nucdvar + toc->nptmesh +
            toc->nptvar + toc->nvar + toc->nmat + toc->nobj + toc->ndir
            + toc->nmrgtree + toc->ngroupelmap + toc->nmrgvar + toc->narray;

        //
        // We don't appear to have any silo objects, so we'll fail it
        //
        if (nSiloObjects <= 0)
        {
            char str[1024];
            SNPRINTF(str, sizeof(str), "Although the Silo library succesfully opened \"%s,\"\n" 
                     "the file contains no silo objects. It may be a PDB file.",
                     filenames[f]);
            EXCEPTION1(InvalidFilesException, str);
        }
    }

    //
    // Attempt to handle case where specified file is actually a silo
    // filename followed by ':' followed by an internal silo directory
    // name.
    //
    const char *baseFilename = FileFunctions::Basename(filenames[f]);
    const char *pColon = strrchr(baseFilename, ':');
    if (pColon != NULL)
    {
        pColon++; // move one past the ':' character
        int triedDir = DBSetDir(dbfiles[f], pColon);
        if (triedDir == 0)
        {
            debug1 << "Handling this silo file as though it is a file-dir combo" << endl;
            debug1 << "for the case where an entire time series is in one silo file." << endl;
            topDir = pColon;
        }
    }

    RegisterFile(f);

    if (f == 0 && !readGlobalInfo && !skipGlobalInfo)
    {
        ReadGlobalInformation(dbfiles[f]);
    }

    return dbfiles[f];
}


// ****************************************************************************
//  Method: avtSiloFileFormat::ReadGlobalInformation
//
//  Purpose:
//      This is a method that is guaranteed to get called before 'ReadVar'
//      calls are made.  Formerly, much of this work was done in 'ReadDir',
//      but ReadDir is not always called, leading to serious problems.
//
//  Programmer: Hank Childs
//  Creation:   April 10, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 09:20:28 PDT 2003
//    Remove support for vector defvars -- they are now correctly handled at a
//    higher level.
//
//    Mark C. Miller, Wed Dec 13 16:55:30 PST 2006
//    Added call to GetMultivarToMultimeshMap
//
// ****************************************************************************

void
avtSiloFileFormat::ReadGlobalInformation(DBfile *dbfile)
{
    //
    // Get the cycle, dtime, etc.
    //
    GetTimeVaryingInformation(dbfile);

    //
    // We can fake ghost zones for block-structured grids.  Read in
    // information related to that.
    //
    GetConnectivityAndGroupInformation(dbfile);

    //
    // Read multi-var to multi-mesh mapping information.
    //
    GetMultivarToMultimeshMap(dbfile);

    readGlobalInfo = true;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetCycle
//
//  Purpose: Return the cycle number associated with this silo file
//
//  Programmer: Mark C. Miller 
//  Creation:   May 16, 2005 
//
//  Modifications:
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Made it return INVALID_CYCLE for a bad value
//
//    Mark C. Miller, Wed Mar 21 10:37:01 PDT 2007
//    Re-factored Silo work to a static function so it can be called from
//    multiple places.
//
//    Mark C. Miller, Sun Jul  5 10:40:30 PDT 2009
//    Added logic to rethrow an invalid files exception to match test
//    suite behavior.
// ****************************************************************************

static int GetCycle(DBfile *dbfile)
{
    if (DBInqVarExists(dbfile, "cycle"))
    {
        int cycle;
        DBReadVar(dbfile, "cycle", &cycle);
        return cycle;
    }
    return avtFileFormat::INVALID_CYCLE;
}

int
avtSiloFileFormat::GetCycle()
{
    int retval = avtFileFormat::INVALID_CYCLE;
    TRY
    {
        const bool canSkipGlobalInfo = true;
        DBfile *dbfile = OpenFile(tocIndex, canSkipGlobalInfo);
        retval = ::GetCycle(dbfile);
    }
    CATCH(InvalidFilesException)
    {
        debug1 << "Unable to GetCycle()" << endl;
        RETHROW;
    }
    CATCHALL
    {
        debug1 << "Unable to GetCycle()" << endl;
    }
    ENDTRY
    return retval;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetCycleFromFilename
//
//  Purpose: Try to get a cycle number from a file name 
//
//  Notes: Although all this method does is simply call the format's base
//  class implementation of GuessCycle, doing this is a way for the Silo
//  format to "bless" the guesses that that method makes. Otherwise, VisIt
//  wouldn't know that Silo thinks those guesses are good. See notes in
//  avtSTXXFileFormatInterface::SetDatabaseMetaData for further explanation.
//
//  Programmer: Mark C. Miller 
//  Creation:   May 16, 2005 
//
// ****************************************************************************

int
avtSiloFileFormat::GetCycleFromFilename(const char *f) const
{
    return GuessCycle(f);
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetTime
//
//  Purpose: Return the time associated with this silo file
//
//  Programmer: Mark C. Miller 
//  Creation:   May 16, 2005 
//
//  Modifications:
//    Mark C. Miller, Mon Mar 19 15:52:24 PDT 2007
//    Added fall back to 'time' if can't filed 'dtime'
//
//    Mark C. Miller, Wed Mar 21 10:37:01 PDT 2007
//    Re-factored Silo work to a static function so it can be called from
//    multiple places.
//
//    Mark C. Miller, Sun Jul  5 10:40:30 PDT 2009
//    Added logic to rethrow an invalid files exception to match test
//    suite behavior.
// ****************************************************************************

static double GetTime(DBfile *dbfile)
{
    if (DBInqVarExists(dbfile, "dtime"))
    {
        double dtime;
        DBReadVar(dbfile, "dtime", &dtime);
        return dtime;
    }
    else if (DBInqVarExists(dbfile, "time"))
    {
        float time;
        DBReadVar(dbfile, "time", &time);
        return (double) time;
    }
    return avtFileFormat::INVALID_TIME;
}

double avtSiloFileFormat::GetTime()
{
    double retval = avtFileFormat::INVALID_TIME;
    TRY
    {
        const bool canSkipGlobalInfo = true;
        DBfile *dbfile = OpenFile(tocIndex, canSkipGlobalInfo);
        retval = ::GetTime(dbfile);
    }
    CATCH(InvalidFilesException)
    {
        debug1 << "Unable to GetTime()" << endl;
        RETHROW;
    }
    CATCHALL
    {
        debug1 << "Unable to GetTime()" << endl;
    }
    ENDTRY
    return retval;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetTimeVaryingInformation
//
//  Purpose:
//      Gets the time varying information out of a Silo file.  This is only
//      called once and is for getting things like time and cycle out of the
//      file.
//
//  Programmer: Hank Childs
//  Creation:   March 11, 2002
//
//  Modifications:
//    Mark C. Miller, Thu Mar 18 10:40:50 PST 2004
//    Added check for null metadata pointer and early return
//
//    Mark C. Miller, Thu Mar 18 11:00:38 PST 2004
//    Added optional md arg. Prefers setting metadata data member over
//    using md arg, when metadata data member is non-NULL
//
//    Mark C. Miller, Thu May 19 09:57:07 PDT 2005
//    Made it indicate the specified timestep has accurate cycle/time
//
//    Mark C. Miller, Wed Mar 21 10:37:01 PDT 2007
//    Modified for re-factored GetCycle/GetTime static functions to
// ****************************************************************************

void
avtSiloFileFormat::GetTimeVaryingInformation(DBfile *dbfile,
    avtDatabaseMetaData *md)
{
    avtDatabaseMetaData *tmpMd = (metadata == 0) ? md : metadata;

    if (tmpMd == 0)
        return;

    //
    // Read out the cycle number and time.
    //
    int cycle = ::GetCycle(dbfile);
    if (cycle != INVALID_CYCLE)
    {
        tmpMd->SetCycle(timestep, cycle);
        tmpMd->SetCycleIsAccurate(true, timestep);
    }
    double dtime = ::GetTime(dbfile);
    if (dtime != INVALID_TIME)
    {
        tmpMd->SetTime(timestep, dtime);
        tmpMd->SetTimeIsAccurate(true, timestep);
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::OpenFile
//
//  Purpose:
//      Opens a file by asking for a file name; adds the file name to a list
//      if we haven't heard of it before.
//
//  Arguments:
//      n       The file name.
//
//  Returns:    A handle to the open file.
//
//  Programmer: Hank Childs
//  Creation:   February 26, 2001
//
//  Modifications:
//    Brad Whitlock, Thu May 22 14:23:14 PST 2003
//    I made it use VISIT_SLASH_STRING so it works better on Windows.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Added bool to skip global info
//
//    Mark C. Miller, Tue Jul 17 20:05:52 PDT 2012
//    Permit file to be an absolute path instead of relative to toc file.
//
//    Mark C. Miller, Tue Feb 21 17:39:22 PST 2017
//    Added logic to ensure that as other files are opened, the toc file
//    remains the most recently used (MRU) file.
// *****************************************************************************

DBfile *
avtSiloFileFormat::OpenFile(const char *n, bool skipGlobalInfo)
{
    //
    // The directory of this file is all relative to the directory of the 
    // table of contents.  Reflect that here.
    //
    char name[1024];

    //
    // If the filename passed in here is already an absolute path, we
    // do not treat it as relative to the toc file. This permits root
    // files to have multi-block objects with absolute path names.
    //
#ifdef WIN32
    // Example "C:\foo\bar"
    if (strlen(n) > 2 && n[1] == ':' && n[2] == VISIT_SLASH_CHAR)
#else
    // Example "/foo/bar"
    if (n[0] == VISIT_SLASH_CHAR)
#endif
    {
        strcpy(name, n);
    }
    else
    {
        char *tocFile = filenames[tocIndex];
        char *thisSlash = tocFile, *lastSlash = tocFile;
        while (thisSlash != NULL)
        {
            lastSlash = thisSlash;
            thisSlash = strstr(lastSlash+1, VISIT_SLASH_STRING);
        }
        if (lastSlash == tocFile)
        {
            strcpy(name, n);
        }
        else
        {
            int amount = lastSlash-tocFile+1;
            strncpy(name, tocFile, amount);
            strcpy(name+amount, n);
        }
    }

    int fileIndex = -1;
    for (int i = 0 ; i < nFiles ; i++)
    {
        if (strcmp(filenames[i], name) == 0)
        {
            fileIndex = i;
            break;
        }
    }

    if (fileIndex == -1)
    {
        //
        // We have asked for a previously unseen file.  Add it to the list and
        // continue.  AddFile will automatically take care of overflow issues.
        //
        fileIndex = AddFile(name);
    }

    DBfile *dbfile = OpenFile(fileIndex, skipGlobalInfo);
    UsedFile(tocIndex); // Ensure toc file remains MRU
    return dbfile;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::CloseFile
//
//  Purpose:
//      Closes a Silo file.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Mar 22 11:10:18 PST 2002 
//    Added a method call to make sure that we are playing well with the file
//    descriptor manager.
//
// ****************************************************************************

void
avtSiloFileFormat::CloseFile(int f)
{
    if (dbfiles[f] != NULL)
    {
        debug4 << "Closing Silo file " << filenames[f] << endl;
        DBClose(dbfiles[f]);
        UnregisterFile(f);
        dbfiles[f] = NULL;
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up the resources.  This closes down file descriptors.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jan 14 11:58:41 PST 2004
//    Clean up all the cached multi- Silo objects.
//
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Clean up resources related to block structured code nodelists.
//
//    Mark C. Miller, Wed Oct 28 20:41:02 PDT 2009
//    Removed arbMeshZoneRangesToSkip. Intoduced better handling of arb.
//    polyhedral meshes.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Thu Apr 12 23:07:11 PDT 2012
//    Removed pascalsTriangleMap class member.
// ****************************************************************************

void
avtSiloFileFormat::FreeUpResources(void)
{
    int  i;

    debug4 << "Silo File Format instructed to free up resources." << endl;
    for (i = 0 ; i < nFiles ; i++)
    {
        CloseFile(i);
    }

    firstSubMesh.clear();
    firstSubMeshVarName.clear();
    actualMeshName.clear();
    allSubMeshDirs.clear();
    blocksForMultivar.clear();

    multimeshCache.Clear();
    multivarCache.Clear();
    multimatCache.Clear();
    multispecCache.Clear();

    nlBlockToWindowsMap.clear();

    map<string, map<int, vector<int>* > >::iterator it1;
    map<int, vector<int>* >::iterator it2;
    for (it1 = arbMeshCellReMap.begin(); it1 != arbMeshCellReMap.end(); it1++)
    {
        for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
            delete it2->second;
        it1->second.clear();
    }
    arbMeshCellReMap.clear();
    for (it1 = arbMeshNodeReMap.begin(); it1 != arbMeshNodeReMap.end(); it1++)
    {
        for (it2 = it1->second.begin(); it2 != it1->second.end(); it2++)
            delete it2->second;
        it1->second.clear();
    }
    arbMeshNodeReMap.clear();
}

// ****************************************************************************
//  Method: avtSiloFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 26, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Mar 15 19:34:19 PST 2001
//    Fixed multi-variables to be more careful in determining what mesh they
//    are defined on.
//
//    Hank Childs, Mon Mar 19 17:22:35 PST 2001
//    Added vectors through defvars.
//
//    Hank Childs, Mon Apr  9 13:18:34 PDT 2001
//    Added support for multi???s where the first domain is invalid.
//
//    Hank Childs, Thu Oct 18 14:21:20 PDT 2001
//    Don't assume that the bounds for the first timestep are the bounds for
//    all timesteps.
//
//    Brad Whitlock, Mon Oct 22 18:33:37 PST 2001
//    Changed the exception keywords to macros.
// 
//    Jeremy Meredith, Thu Oct 25 13:02:18 PDT 2001
//    Added code to read domain decomposition information.
//
//    Hank Childs, Thu Oct 25 17:02:39 PDT 2001
//    More changes for domain decomposition information.
//
//    Hank Childs, Tue Oct 30 16:27:29 PST 2001
//    Pushed code into a ReadDir routine.
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Jeremy Meredith, Tue Jul 15 09:44:15 PDT 2003
//    Split some of the methods out of ReadDir so it could be
//    parallelized.
//
//    Mark C. Miller, Thu Mar 18 11:00:38 PST 2004
//    Added call to set cycle/time
//
//    Jeremy Meredith, Thu Mar 25 16:52:04 EDT 2010
//    Error if we openend a file with nothing in it to prevent false
//    positives when detecting Silo files.  Happens only in strict mode.
//
//    Mark C. Miller, Fri Oct 29 10:01:16 PDT 2010
//    Removed logic looking for empty md and throwing invalid file exception.
//    Thats handled by VisIt now in the format classes.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

void
avtSiloFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    DBfile *dbfile = OpenFile(tocIndex);

    //
    // Sub-vars from a multi-var are defined on the sub-meshes of a multi-mesh.
    // To figure out which multi-mesh a multi-var is defined on, we need to
    // determine it from the sub-vars.  This array stores the names of the
    // first sub-meshes so the sub-vars can check against them later.
    //
    firstSubMesh.clear();
    actualMeshName.clear();
    blocksForMesh.clear();

    multimeshCache.Clear();
    multivarCache.Clear();
    multimatCache.Clear();
    multispecCache.Clear();

    //
    // We're just interested in metadata for now, so tell Silo not
    // to read the extra data arrays, except for material names and 
    // numbers and colors.
    //
    DBSetDataReadMask2(DBMatMatnames|DBMatMatnos|DBMatMatcolors);

    //
    // Do a recursive search through the subdirectories.
    //
    ReadDir(dbfile, topDir.c_str(), md);
    BroadcastGlobalInfo(md);
    DoRootDirectoryWork(md);

    //
    // Set time/cycle information
    //
    GetTimeVaryingInformation(dbfile, md);

    // To be nice to other functions, tell Silo to turn back on reading all
    // of the data.
    DBSetDataReadMask2(DBAll);
}

// ****************************************************************************
//  Methods for reading information about different classes of Silo objects. 
//
//  This comment block is kept here for legacy reasons as it contains 
//  information prior to re-factoring of avtSiloFileFormat::ReadDir(). Most
//  of the meat of that function was re-factored into the ReadXXX() methods,
//  below, one for each class of Silo object.
//
//  DO NOT ADD ANY COMMENTS TO THIS BLOCK.
//
//  Modifications:
//    Jeremy Meredith, Wed Nov 21 14:11:35 PST 2001
//    Fixed the logic so the structured domain boundary info works with 
//    more file types.
//
//    Eric Brugger, Tue Nov 27 16:21:17 PST 2001
//    I modified the routine to only search directories in _meshtv_searchpath
//    if present.
//
//    Hank Childs, Mon Dec  3 09:55:54 PST 2001
//    Do not read in boundary information if we are only going to serve up
//    meta-data.
//
//    Eric Brugger, Tue Dec  4 15:05:56 PST 2001
//    I modified the routine to read an alternate representation of the
//    domain connectivity for a 3d structured mesh.  If the alternate
//    representation is present it uses it instead of the normal one.  I
//    also corrected a bug setting the extents for ucd meshes.
//
//    Jeremy Meredith, Thu Dec 13 11:46:06 PST 2001
//    Fixed a bug with the reading of 3d structured connectivity.
//
//    Jeremy Meredith, Tue Dec 18 15:40:05 PST 2001
//    Added multi-species support.  Renamed individual species so they are
//    one-origin.
//
//    Hank Childs, Fri Jan  4 10:44:44 PST 2002
//    Fix UMR.
//
//    Jeremy Meredith, Thu Jan 17 10:44:33 PST 2002
//    Reworked the interdomain connectivity reading.  Now in parallel only
//    the first processor reads it and then broadcasts it to the other
//    processors.  This can be a big speedup.
//
//    Sean Ahern, Fri Feb  8 11:44:55 PST 2002
//    Added support for material names.  Also got rid of all but one
//    GetComponent calls.
//
//    Jeremy Meredith, Mon Feb 25 14:26:35 PST 2002
//    Added code to make sure the assumed directories exist when reading
//    interdomain connectivity.  Some files use a different naming scheme.
//
//    Hank Childs, Mon Apr 15 09:13:51 PDT 2002
//    Add support for distributed Silo files.  This used to happen
//    automatically, but there have been some changes to DBGetComponent.
//
//    Hank Childs, Mon Apr 15 14:45:53 PDT 2002
//    Explicitly check if a directory exists rather than issuing a DBSetDir.
//    This will clean up our log files.
//
//    Hank Childs, Wed Apr 17 12:11:04 PDT 2002
//    Add ENDTRY (this only breaks when we are using fake exceptions).
//
//    Hank Childs, Sun Jun 16 19:55:52 PDT 2002
//    Initialize the cell origin.
//
//    Hank Childs, Mon Jun 24 12:45:09 PDT 2002
//    Add support for groups.
//
//    Hank Childs, Fri Jul 12 16:33:56 PDT 2002
//    Added current directory name to directory tracking routines to better
//    support multi-vars that have relative pathnames.
//
//    Hank Childs, Sun Aug 18 11:53:02 PDT 2002
//    Added support for meshes that are made up of all disjoint elements.
//
//    Hank Childs, Wed Sep  4 16:43:34 PDT 2002
//    Added a call to parse the defvars and put them in the meta-data object.
//
//    Hank Childs, Fri Sep 27 14:58:14 PDT 2002
//    Added support for mesh units.
//
//    Kathleen Bonnell, Wed Oct 23 13:55:55 PDT 2002  
//    Added tdims, so that topological dimension for multi-point-meshes will 
//    be set correctly. 
//
//    Hank Childs, Fri Oct 25 15:44:28 PDT 2002
//    Fix a bug where the topological dimension was not being set correctly,
//    causing the facelist filter to not execute.
//
//    Hank Childs, Tue Nov 12 09:50:12 PST 2002
//    If a multi-mesh is located in a sub-directory, name the multi-mesh with
//    the subdirectory name.
//
//    Hank Childs, Tue Dec 10 12:43:46 PST 2002
//    Do not give up on invalid variables.  Just mark them as invalid.
//
//    Jeremy Meredith, Wed Mar 19 12:26:35 PST 2003
//    Added extra information needed to allow multivars defined on only
//    some domains of its associated multimesh.
//
//    Hank Childs, Thu Apr 10 08:45:44 PDT 2003
//    Do not parse the defvars or read in the connectivity information, since
//    that is now done when the file is opened (because ReadDir is not always
//    called).
//
//    Hank Childs, Tue May  6 18:16:40 PDT 2003
//    Some calls were being made in every pass of ReadDir, although they were
//    only necessary in the first pass.  Also only register the domain
//    directories for multi-meshes (the MeshTV method).
//
//    Hank Childs, Fri May 16 18:21:20 PDT 2003
//    Make sure that all return values are valid.
//
//    Jeremy Meredith, Tue Jul 15 09:44:15 PDT 2003
//    Split some of the methods out of ReadDir so it could be
//    parallelized.  Only allow the first process to read the root file.
//
//    Hank Childs, Fri Sep 12 14:48:33 PDT 2003
//    Allow for '0' to be a valid material.
//
//    Jeremy Meredith, Wed Oct 15 17:04:33 PDT 2003
//    Fixed up the multi-species check to "fail correctly" if the actual
//    species object did not actually exist in the file.
//    Also, added a workaround for a Silo bug where it will crash instead
//    of returning NULL from DBGetMatspecies if you have registered a
//    callback function.  This workaround should be removed if Silo gets
//    fixed.
//
//    Hank Childs, Mon Dec  1 15:05:43 PST 2003
//    Look for visit_defvars first.  Only if it does not exist, then look for
//    meshtv_defvars.
//
//    Hank Childs, Wed Jan 14 11:19:24 PST 2004
//    Use caching mechanism for DBGetMultiThing.
//
//    Jeremy Meredith, Fri Jun 11 14:36:59 PDT 2004
//    Fixed two problems with _meshtv_searchpath.  First, it assumed there was
//    at most one entry per semicolon, but you can have one less separator
//    than num entries.  Second, it was walking off the end of the string.
//
//    Kathleen Bonnell, Thu Jul 22 12:30:22 PDT 2004 
//    Use value of ascii_labels option for variables to set treatAsASCII
//    in ScalarMetaData. 
//
//    Brad Whitlock, Tue Jul 20 15:48:04 PST 2004
//    Added support for passing the variable units back up to VisIt via
//    the metadata. I also added support for labels and a database comment.
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Mark C. Miller, Wed Jun 29 08:49:14 PDT 2005
//    Made it NOT throw an exception if first non-empty mesh is invalid
//
//    Mark C. Miller, Wed Aug 10 08:15:21 PDT 2005
//    Added code to handle Silo's new defvars objects
//
//    Eric Brugger, Fri Aug 12 08:28:04 PDT 2005
//    Modified the handling of multi-meshes with all EMPTY blocks to
//    avoid a crash.
//
//    Eric Brugger, Fri Aug 12 11:36:35 PDT 2005
//    Corrected a memory leak.
//
//    Hank Childs, Fri Aug 19 08:31:47 PDT 2005
//    Corrected another memory leak.
//
//    Jeremy Meredith, Thu Aug 25 11:35:32 PDT 2005
//    Added group origin to mesh metadata constructor.
//
//    Mark C. Miller, Mon Sep 26 14:05:52 PDT 2005
//    Added code to query for AlphabetizeVariables
//
//    Jeremy Meredith, Fri Oct  7 17:08:21 PDT 2005
//    Added VARTYPE to defvar defines to avoid namespace conflict.
//
//    Mark C. Miller, Fri Nov 11 09:45:42 PST 2005
//    Made it more fault tolerant when multimats are corrupted
//
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Removed spoofing of CSG mesh as a surface mesh  
//
//    Mark C. Miller, Wed Jan 18 19:58:47 PST 2006
//    Made it more fault tolerant for multivar, multimat and multimatspecies
//    objects that contained all EMPTY pieces.
//
//    Kathleen Bonnell, Wed Feb  8 09:41:45 PST 2006 
//    Set mmd->meshCoordType from coord_sys. 
//
//    Mark C. Miller, Thu Mar  2 00:03:40 PST 2006
//    Added support for curve objects
// 
//    Hank Childs, Thu May 18 11:33:27 PDT 2006
//    Fix UMR with mesh coord types and point meshes.
//
//    Mark C. Miller, Thu Jul 13 22:41:56 PDT 2006
//    Added reading of material colors, if available
//
//    Mark C. Miller, Thu Jul  6 15:14:46 PDT 2006
//    Fixed case where GetCsgmesh can return csgm but not csgm->zones
//
//    Mark C. Miller, Wed Nov 29 15:08:21 PST 2006
//    Set value for connectivityIsTimeVarying
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Removed all EXCEPTIONs from this routine except for NULL toc to make
//    the plugin more fault tolerant. Added code to support multi-block CSG
//    meshes
//
//    Mark C. Miller, Wed Dec 13 16:55:30 PST 2006
//    Added calls to RegisterDomainDirs to loops over multivars, multimats
//    and multimatspecies
//
//    Mark C. Miller, Wed Feb  6 12:27:09 PST 2008
//    Changed DBSetDir("/") calls to use topDir instead. Changed interface
//    to GenerateFileName to accept topDir too.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Added setting of guiHide feature on many md objects.
//    Added nodelist enumeration variables, when present42`
//
//    Mark C. Miller, Wed Apr 23 12:08:56 PDT 2008
//    Made material name parsing handle material names that point to NULL.
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added logic to ignore data extents for block structured code
//
//    Mark C. Miller, Tue Jul 29 18:16:59 PDT 2008
//    Added logic to ensure _meshtv_defvars and _visit_defvars are indeed
//    simple silo variables before reading them as such. This is to prevent
//    situations where users have switched to using DBPutDefvars to create
//    the same information but wind up also calling it the same thing.
//
//    Brad Whitlock, Wed Sep  3 10:07:32 PDT 2008
//    Prevent multimesh and multimat names from being printed when all entries
//    are EMPTY since it was causing a crash (array out of bounds).
//
//    Mark C. Miller, Tue Jan  6 22:11:33 PST 2009
//    Added support for explicit specification of topological dimension of a
//    ucd mesh from the database via the DBOPT_TOPO_DIM option.
//
//    Mark C. Miller, Wed Feb 25 17:35:05 PST 2009
//    Tightened logic for triggering ANNOTATION_INT nodelist search to ensure
//    it happens only when ReadDir is in the root (topDir) directory. Also,
//    Added a call to CloseFile(1) just prior to calling AddAnnotInt... as 
//    a work-around for a bug in HDF5.
//
//    Mark C. Miller, Mon Mar  2 11:50:08 PST 2009
//    Removed call to CloseFile(1) just prior to adding annot-int nodelists.
//    The issue that addressed is now handled in the AddAnnotInt... routine.
//
//    Mark C. Miller, Wed Mar  4 08:54:57 PST 2009
//    Improved logic to handle ignoring of spatial/data extents so that user
//    can override explicitly or let plugin handle automatically.
//
//    Mark C. Miller, Wed Mar  4 13:39:58 PST 2009
//    Backed out preceding change. It had backwards compatibility problems.
//
//    Mark C. Miller, Fri Mar 20 04:38:56 PDT 2009
//    Added logic to NOT descend to first non-EMPTY block of a multi-mat if
//    the multi-mat appears to have enough information about the global
//    material context (#mats, names and colors).
//
//    Mark C. Miller, Tue Mar 24 11:46:22 PDT 2009
//    Changed #if defined(SILO_VERSION_GE) && SILO_VERSION_GE(4,6,2) to nested
//    #if statements. The former assumes short-circuit evaluation and not
//    all C pre-processors apparently obey it.
//
//    Mark C. Miller, Tue May  5 11:11:19 PDT 2009
//    Changed level of info returned from MultiMatHasAllMatInfo necessary to
//    skirt reading individual material object to 3 or greater. This fixes
//    cases where material numbers are known at the multi-block level but
//    all other material info is known only on the individual material blocks.
//
//    Hank Childs, Mon May 25 11:07:17 PDT 2009
//    Add support for Silo releases before 4.6.3.
//
//    Mark C. Miller, Thu Jun  4 20:53:32 PDT 2009
//    Added missing hideFromGUI support for several object types.
//
//    Mark C. Miller, Tue Jun 16 18:20:18 PDT 2009
//    Removed EXCEPTIONs from this function. PLEASE DON'T PUT ANY HERE. It
//    can wind up making a whole file unuseable when only one object in the
//    file is bad!
//
//    Mark C. Miller, Wed Jun 17 10:39:48 PDT 2009
//    Re-factored all of the loops over Silo objects into their own functions.
//    This was done to easily wrap the loops in try/catch blocks so that
//    Exceptions thrown below this function due to any single object being
//    bad DO NOT result in invalidating the whole darn file.
//
//    DO NOT ADD ANY MODIFICATIONS COMMENTS TO THIS BLOCK. SEE NOTE AT TOP.
// ****************************************************************************



// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:55:49 PDT 2009
//    Removed DBtoc* arg.
//
//    Mark C. Miller, Tue Dec 15 14:01:48 PST 2009
//    Added metadataIsTimeVarying
//
//    Cyrus Harrison, Thu Jun 10 16:15:36 PDT 2010
//    Set 'metadataIsTimeVarying' member var to false if silo var
//    'MetadataIsTimeVarying' is not equal 1 or does not exist.
//
//    Cyrus Harrison.
//    Reverted previous change. Moved init of 'metadataIsTimeVarying'
//    out of this function & into CheckForTimeVaryingMetadata().
//
//    Eric Brugger, Wed Sep 30 13:47:14 PDT 2015
//    I corrected a bug that caused "_meshtv_searchpath" to be ignored.
//    I also added "_visit_searchpath" as a synonym for "_meshtv_searchpath"
//    since "_visit_searchpath" is much more appropriate for VisIt.
//
//    Mark C. Miller, Wed Feb 10 20:47:43 PST 2016
//    Add logic to ignore searchpath vars if once read they consist entirely
//    of blank or ';' characters.
//
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Added logic to support adding of block decomposition as a variable.
// ****************************************************************************
void
avtSiloFileFormat::ReadTopDirStuff(DBfile *dbfile, const char *dirname,
    avtDatabaseMetaData *md, char **searchpath_strp)
{
    TRY
    {

        //
        // The dbfile will probably change, so read in the meshtv_defvars and
        // meshtv_searchpath while we can.
        //
        if (strcmp(dirname, topDir.c_str()) == 0)
        {
            codeNameGuess = GuessCodeNameFromTopLevelVars(dbfile);

            // Decide whether or not to ignore data extents
            if (ignoreDataExtentsAAN == Always)
                ignoreDataExtents = true;
            else if (ignoreDataExtentsAAN == Never)
                ignoreDataExtents = false;
            else // Auto
            {
                if (codeNameGuess == "BlockStructured")
                    ignoreDataExtents = true;
                else if (codeNameGuess == "Ale3d")
                    ignoreDataExtents = true;
            }

            // Decide whether or not to ignore spatial extents
            if (ignoreSpatialExtentsAAN == Always)
                ignoreSpatialExtents = true;
            else if (ignoreSpatialExtentsAAN == Never)
                ignoreSpatialExtents = false;
            else // Auto
            {
                // Nothing automaticly to do for sptail extents, yet
            }

            if (DBInqVarExists(dbfile, "ConnectivityIsTimeVarying"))
            {
                int tvFlag;
                DBReadVar(dbfile, "ConnectivityIsTimeVarying", &tvFlag);
                if (tvFlag == 1)
                    connectivityIsTimeVarying = true;
            }

            if (DBInqVarExists(dbfile, "AlphabetizeVariables"))
            {
                int alphaFlag;
                DBReadVar(dbfile, "AlphabetizeVariables", &alphaFlag);
                if (alphaFlag == 0)
                    md->SetMustAlphabetizeVariables(false);
            }

            if (DBInqVarExists(dbfile, "_disjoint_elements"))
            {
                hasDisjointElements = true;
            }

            if (DBInqVarExists(dbfile, "_visit_allow_empty_blocks"))
            {
                int mbFlag;
                DBReadVar(dbfile, "_visit_allow_empty_blocks", &mbFlag);
                if (mbFlag == 1)
                    ignoreMissingBlocks = true;
            }

            bool hadVisitDefvars = false;
            if (DBInqVarExists(dbfile, "_visit_defvars") &&
                DBInqVarType(dbfile, "_visit_defvars") == DB_VARIABLE)
            {
                int    ldefvars = DBGetVarLength(dbfile, "_visit_defvars");
                if (ldefvars > 0)
                {
                    char  *defvar_str = new char[ldefvars+1];
                    for (int i = 0 ; i < ldefvars+1 ; i++)
                    {
                        defvar_str[i] = '\0';
                    }
                    DBReadVar(dbfile, "_visit_defvars", defvar_str);
                    AddDefvars(defvar_str, md);
                    delete [] defvar_str;
                }
                hadVisitDefvars = true;
            }

            if (!hadVisitDefvars && DBInqVarExists(dbfile, "_meshtv_defvars") &&
                DBInqVarType(dbfile, "_meshtv_defvars") == DB_VARIABLE)
            {
                int    ldefvars = DBGetVarLength(dbfile, "_meshtv_defvars");
                if (ldefvars > 0)
                {
                    char  *defvar_str = new char[ldefvars+1];
                    for (int i = 0 ; i < ldefvars+1 ; i++)
                    {
                        defvar_str[i] = '\0';
                    }
                    DBReadVar(dbfile, "_meshtv_defvars", defvar_str);
                    AddDefvars(defvar_str, md);
                    delete [] defvar_str;
                }
            }

            bool hadVisitSearchpath = false;
            if (DBInqVarExists(dbfile, "_visit_searchpath"))
            {
                int    lsearchpath = DBGetVarLength(dbfile, "_visit_searchpath");
                if (lsearchpath > 0)
                {
                    char *searchpath_str = new char[lsearchpath+1];
                    DBReadVar(dbfile, "_visit_searchpath", searchpath_str);
                    searchpath_str[lsearchpath] = '\0';
                    if (strspn(searchpath_str, " ;\t") < strlen(searchpath_str))
                    {
                        debug1 << "Setting search path from _visit_searchpath" << endl;
                        *searchpath_strp = searchpath_str;
                    }
                }
                hadVisitSearchpath = true;
            }

            if (!hadVisitSearchpath &&
                DBInqVarExists(dbfile, "_meshtv_searchpath"))
            {
                int    lsearchpath = DBGetVarLength(dbfile, "_meshtv_searchpath");
                if (lsearchpath > 0)
                {
                    char *searchpath_str = new char[lsearchpath+1];
                    DBReadVar(dbfile, "_meshtv_searchpath", searchpath_str);
                    searchpath_str[lsearchpath] = '\0';
                    if (strspn(searchpath_str, " ;\t") < strlen(searchpath_str))
                    {
                        debug1 << "Setting search path from _meshtv_searchpath" << endl;
                        *searchpath_strp = searchpath_str;
                    }
                }
            }

            if (DBInqVarExists(dbfile, "_fileinfo"))
            {
                int lfileinfo = DBGetVarLength(dbfile, "_fileinfo");
                if (lfileinfo > 0)
                {
                    char *fileinfo_str = new char[lfileinfo+1];
                    DBReadVar(dbfile, "_fileinfo", fileinfo_str);
                    md->SetDatabaseComment(fileinfo_str);
                    delete [] fileinfo_str;
                }
            }


            // See if we should add the block decomp as a scalar var
            if (codeNameGuess == "BlockStructured" &&
                DBInqVarExists(dbfile, "Decomposition/Domains_BlockNums"))
                addBlockDecompositionAsVar = true;
        }

    }
    CATCHALL
    {
        debug1 << "Giving up on top-level directory miscellaneous stuff" << endl;
    }
    ENDTRY
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names. Also added logic to
//    handle freeing of multimesh object during exceptions.
//
//    Cyrus Harrison, Tue Oct 13 08:59:59 PDT 2009
//    Construct avtMeshMetaData object after mrgtree processing  b/c this
//    may change the mesh type to indicate an AMR mesh.
//
//    Mark C. Miller, Mon Nov  9 10:41:48 PST 2009
//    Added 'dontForceSingle' to call to HandleMrgtree
//
//    Cyrus Harrison, Wed Mar 24 10:41:20 PDT 2010
//    Set haveAmrGroupInfo if we have amr level info.
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Tue Jul 10 19:55:37 PDT 2012
//    Added logic to handle nodelist vars defined in MRG Trees.
//
//    Katheen Biagas, Thu Jun 6 13:11:27 PDT 2013
//    Enable reading of mrgtrees on Windows.
//
//    Cyrus Harrison, Fri Aug 16 10:07:47 PDT 2013
//    Added support for nodelists placed @ /Nodelists/
//
//    Mark C. Miller, Tue Feb  2 14:50:26 PST 2016
//    Added logic to handle all-empty multi-block meshes. Moved a lot of
//    functionality related to finding first non-empty block to a function.
//    Collapsed DB_QUAD_CURV, DB_QUAD_RECT switch cases to a single case and
//    added support for DB_QUADMESH where actual mesh type is known only on
//    first non-empty block via the coordtype member of a DBquadmesh.
//    
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Added logic to support adding of block decomposition as a variable.
//
//    Kathleen Biagas, Thu Nov 17 16:18:32 PST 2016
//    Change use of std::vector::reserve to adding the size to the
//    constructor.  Prevents segv for attempting to access a slot in the
//    vector that hasn't been allocated yet.
//
//    Mark C. Miller, Sat Dec 24 00:35:41 PST 2016
//    Fix real and potential issues with layer namescheme by ensuring
//    layer ids as given internally to VisIt are 0...numLayers-1 and that
//    groupNames are specified explicitly.
// ****************************************************************************

void
avtSiloFileFormat::ReadMultimeshes(DBfile *dbfile, 
    int nmultimesh, char **multimesh_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    map<string, bool> haveAddedNodelistEnumerations;
    for (i = 0 ; i < nmultimesh ; i++)
    {
        char *name_w_dir = 0;
        DBmultimesh *mm = 0;
        avtSiloMultiMeshCacheEntry *mm_ent = NULL;
        TRY
        {
            name_w_dir = GenerateName(dirname, multimesh_names[i], topDir.c_str());
            bool valid_var = true;
            bool is_all_empty = false;
            int silo_mt = -1;
            int meshnum = 0;
            string mb_meshname = "";
            GetMultiMesh(dirname, multimesh_names[i], &mm_ent, &valid_var);

            if(mm_ent != NULL)
                mm = mm_ent->DataObject();

            if (mm != NULL && valid_var)
            {
                RegisterDomainDirs(mm_ent,dirname);
                meshnum = FindFirstNonEmptyBlock(multimesh_names[i], mm->nblocks, mm_ent,
                    mm->repr_block_idx, mm->empty_cnt, mm->empty_list);
                if (meshnum < 0)
                {
                    is_all_empty = true;
                    if (firstAllEmptyMultimesh == "")
                        firstAllEmptyMultimesh = name_w_dir;
                    emptyObjectsList[name_w_dir] = true;
                }
                else
                {
                    mb_meshname  = mm_ent->GenerateName(meshnum);
                    silo_mt = mm_ent->MeshType(meshnum);
                }
            }
            else
            {
                debug1 << "Invalidating mesh \"" << multimesh_names[i] << "\"" << endl;
                valid_var = false;
            }

            //
            // CSG meshes require special handling because we use CSG
            // "regions" in place of VisIt's notion of "domains" and the
            // pieces of the multi-mesh as VisIt's "groups."
            //
            if (silo_mt == DB_CSGMESH)
            {
                AddCSGMultimesh(dirname, multimesh_names[i], md, mm_ent, dbfile);
            }
            else if (mm)
            {
                avtMeshType mt = AVT_UNKNOWN_MESH;
                avtMeshCoordType mct = AVT_XY;
                int ndims = 0; //TODO: check on unint var
                int tdims = 0; //TODO: check on uninit var
                int cellOrigin = 0; //TODO: check on unint var
                int groupOrigin = 0;
                string xUnits, yUnits, zUnits;
                string xLabel, yLabel, zLabel;
                switch (silo_mt)
                {
                  case DB_UCDMESH:
                    {
                        mt = AVT_UNSTRUCTURED_MESH;
                        string realvar;
                        DBfile *correctFile = dbfile;
                        DetermineFileAndDirectory(mb_meshname.c_str(),"",
                                                  correctFile,
                                                  realvar);
                        DBucdmesh *um = DBGetUcdmesh(correctFile, realvar.c_str());
                        if (um == NULL)
                        {
                            debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                                   << "\" since its first non-empty block (" << mb_meshname
                                   << ") is invalid." << endl;
                            valid_var = false;
                            break;
                        }
                        ndims = um->ndims;
                        tdims = ndims;
                        if (um->topo_dim != -1)
                            tdims = um->topo_dim;
                        cellOrigin = um->origin;
                        if (um->units[0] != NULL)
                            xUnits = um->units[0];
                        if (um->units[1] != NULL)
                            yUnits = um->units[1];
                        if (um->units[2] != NULL)
                            zUnits = um->units[2];

                        if (um->labels[0] != NULL)
                            xLabel = um->labels[0];
                        if (um->labels[1] != NULL)
                            yLabel = um->labels[1];
                        if (um->labels[2] != NULL)
                            zLabel = um->labels[2];

                        if (ndims ==2 && um->coord_sys == DB_CYLINDRICAL)
                            mct = AVT_RZ;
                        else 
                            mct = AVT_XY;

                        DBFreeUcdmesh(um);
                    }
                    break;
                  case DB_POINTMESH:
                    {
                        mt = AVT_POINT_MESH;
                        string realvar;
                        DBfile *correctFile = dbfile;

                        DetermineFileAndDirectory(mb_meshname.c_str(),"",
                                                  correctFile,
                                                  realvar);
                        DBpointmesh *pm = DBGetPointmesh(correctFile, realvar.c_str());
                        if (pm == NULL)
                        {
                            debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                                   << "\" since its first non-empty block (" << mb_meshname
                                   << ") is invalid." << endl;
                            valid_var = false;
                            break;
                        }
                        ndims = pm->ndims;
                        tdims = 0;
                        cellOrigin = pm->origin;
                        if (pm->units[0] != NULL)
                            xUnits = pm->units[0];
                        if (pm->units[1] != NULL)
                            yUnits = pm->units[1];
                        if (pm->units[2] != NULL)
                            zUnits = pm->units[2];

                        if (pm->labels[0] != NULL)
                            xLabel = pm->labels[0];
                        if (pm->labels[1] != NULL)
                            yLabel = pm->labels[1];
                        if (pm->labels[2] != NULL)
                            zLabel = pm->labels[2];

                        DBFreePointmesh(pm);
                    }
                    break;
                  case DB_QUAD_RECT:
                  case DB_QUAD_CURV:
                  case DB_QUADMESH:
                    {
                        string realvar;
                        DBfile *correctFile = dbfile;
                        DetermineFileAndDirectory(mb_meshname.c_str(),"", correctFile, realvar);
                        DBquadmesh *qm = DBGetQuadmesh(correctFile, realvar.c_str());
                        if (qm == NULL)
                        {
                            debug1 << "Invalidating mesh \"" << multimesh_names[i] 
                                   << "\" since its first non-empty block (" << mb_meshname
                                   << ") is invalid." << endl;
                            valid_var = false;
                            break;
                        }
                        if (silo_mt == DB_QUAD_RECT)
                            mt = AVT_RECTILINEAR_MESH;
                        else if (silo_mt == DB_QUAD_CURV)
                            mt = AVT_CURVILINEAR_MESH;
                        else
                        {
                            if (qm->coordtype == DB_COLLINEAR)
                                mt = AVT_RECTILINEAR_MESH;
                            else if (qm->coordtype == DB_NONCOLLINEAR)
                                mt = AVT_CURVILINEAR_MESH;
                        }
                        ndims = qm->ndims;
                        tdims = ndims;
                        cellOrigin = qm->origin;
                        if (qm->units[0] != NULL)
                            xUnits = qm->units[0];
                        if (qm->units[1] != NULL)
                            yUnits = qm->units[1];
                        if (qm->units[2] != NULL)
                            zUnits = qm->units[2];

                        if (qm->labels[0] != NULL)
                            xLabel = qm->labels[0];
                        if (qm->labels[1] != NULL)
                            yLabel = qm->labels[1];
                        if (qm->labels[2] != NULL)
                            zLabel = qm->labels[2];

                        if (ndims ==2 && qm->coord_sys == DB_CYLINDRICAL)
                            mct = AVT_RZ;
                        else 
                            mct = AVT_XY;

                        DBFreeQuadmesh(qm);
                    }
                    break;
                  default:
                    {
                        mt = AVT_UNKNOWN_MESH;
                        ndims = 0;
                        tdims = 0;
                        cellOrigin = 0;
                    }
                    break;
                }

                int block_cnt = mm?mm->nblocks:0;
                if (is_all_empty) block_cnt = 1;

                avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                    block_cnt, mm?mm->blockorigin:0, cellOrigin,
                    groupOrigin, ndims, tdims, mt);

                mmd->hideFromGUI = mm->guihide;
                mmd->validVariable = valid_var;
                mmd->groupTitle = "blocks";
                mmd->groupPieceName = "block";
                mmd->xUnits = xUnits;
                mmd->yUnits = yUnits;
                mmd->zUnits = zUnits;
                mmd->xLabel = xLabel;
                mmd->yLabel = yLabel;
                mmd->zLabel = zLabel;
                mmd->meshCoordType = mct;

                //
                // Handle mrgtree on the multimesh
                //
                int num_amr_groups = 0;
                vector<int> amr_group_ids;
                vector<string> amr_block_names;

                if (mm->mrgtree_name != 0 && !is_all_empty)
                {
                    // So far, we've coded only for MRG trees representing AMR hierarchies
                    HandleMrgtreeAMRGroups(dbfile, mm, multimesh_names[i],
                        &mt, &num_amr_groups, &amr_group_ids, &amr_block_names,
                        forceSingle);

                    HandleMrgtreeNodelistVars(dbfile, name_w_dir, mm->mrgtree_name, md);
                }

                //
                // Handle possible layer namescheme for this mesh
                //
                map<int, int> layer_to_nlayer_map;
                vector<int> block_to_nlayer_map; 
                vector<string> layer_names; 
                int nlayer_id = 0;
                string candidate_layer_namescheme = string(name_w_dir) + "_layer_namescheme";
                if (DBInqVarExists(dbfile, candidate_layer_namescheme.c_str()))
                {
                    char *layer_ns_str = (char*) DBGetVar(dbfile, candidate_layer_namescheme.c_str());
                    DBnamescheme *layer_ns = DBMakeNamescheme(layer_ns_str);
                    if (layer_ns)
                    {
                        // Normalize layer ids to range 0...numLayers-1 
                        for (int b = 0; b < (mm?mm->nblocks:0); b++)
                        {
                            int layer_id = db_get_index(layer_ns, b);
                            if (layer_to_nlayer_map.find(layer_id) == 
                                layer_to_nlayer_map.end())
                            {
                                char layer_name[64];
                                SNPRINTF(layer_name, sizeof(layer_name), "layer_%d", layer_id);
                                layer_names.push_back(layer_name);
                                layer_to_nlayer_map[layer_id] = nlayer_id;
                                nlayer_id++;
                            }
                        }
                        for (int b = 0; b < (mm?mm->nblocks:0); b++)
                        {
                            int layer_id = db_get_index(layer_ns, b);
                            block_to_nlayer_map.push_back(layer_to_nlayer_map[layer_id]);
                        }
                        DBFreeNamescheme(layer_ns);
                    }
                    free(layer_ns_str);
                }

                if (mt == AVT_UNSTRUCTURED_MESH)
                    mmd->disjointElements = hasDisjointElements || mm->disjoint_mode != 0; 

                if (num_amr_groups > 0 && !is_all_empty)
                {
                    mmd->numGroups = num_amr_groups;
                    mmd->groupTitle = "levels";
                    mmd->groupPieceName = "level";
                    mmd->blockNames = amr_block_names;
                    mmd->meshType = AVT_AMR_MESH;
                    md->Add(mmd);
                    groupInfo.haveGroups = false;
                    // On the engine groupInfo.haveGroups may get changed when connectivty
                    // info is read. To avoid clobbering amr group info we set haveAmrGroupInfo
                    // to true to guard against md->AddGroupInformation getting called in
                    // DoRootDirectroyWork()
                    haveAmrGroupInfo = true;
                    md->AddGroupInformation(num_amr_groups, mm?mm->nblocks:0, amr_group_ids);
                    debug1 << "Using AMR levels as Group Information"<<endl;
                }
                else if (nlayer_id>1 && !is_all_empty)
                {
                    mmd->numGroups = nlayer_id;
                    mmd->groupTitle = "layers";
                    mmd->groupPieceName = "layer";
                    mmd->groupNames = layer_names;
                    md->Add(mmd);
                    md->AddGroupInformation(nlayer_id, mm?mm->nblocks:0, block_to_nlayer_map);
                    debug1 << "Using Layers as Group Information"<<endl;
                }
                else
                {
                    md->Add(mmd);
                }

                //
                // Handle special case for enumerated scalar rep for nodelists
                //
                if ( !haveAddedNodelistEnumerations[name_w_dir])
                {
                    haveAddedNodelistEnumerations[name_w_dir] = true;
                    AddNodelistEnumerations(dbfile, md, name_w_dir);
                }
                if (searchForAnnotInt && strcmp(dirname, topDir.c_str()) == 0)
                {
                    AddAnnotIntNodelistEnumerations(dbfile, md, name_w_dir, mm_ent);
                }

                // Store off the important info about this multimesh
                // so we can match other multi-objects to it later
                StoreMultimeshInfo(dirname, name_w_dir, meshnum, mm_ent);
            }

        }
        CATCHALL
        {
            // We explicitly free the multimesh here in the catch block and NOT at
            // the exit from this function because in 'normal' operation, the multimesh
            // objects are actually cached by the plugin and free'd later.
            if (mm)
            {
                // Make sure its removed from the plugin's cache, too.
                multimeshCache.RemoveEntry(dirname, multimesh_names[i]);
            }

            debug1 << "Invalidating multi-mesh \"" << multimesh_names[i] << "\"" << endl;
            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                0, 0, 0, 0, 0, 0, AVT_UNKNOWN_MESH);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        // Add block decomp as variable if needed
        if (i == 0 && 
            addBlockDecompositionAsVar &&
            !haveAddedBlockDecompositionAsVar)
        {
            char cwd[512];
            DBGetDir(dbfile, cwd);
            if (!strcmp(cwd, "/"))
            {
                SNPRINTF(cwd, sizeof(cwd), "%s_block_nums", name_w_dir);
                md->Add(new avtScalarMetaData(cwd, name_w_dir, AVT_ZONECENT));
                haveAddedBlockDecompositionAsVar = true;
            }
        }

        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Added logic to support adding of block decomposition as a variable.
// ****************************************************************************
void
avtSiloFileFormat::ReadQuadmeshes(DBfile *dbfile,
    int nqmesh, char **qmesh_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j;
    for (i = 0 ; i < nqmesh ; i++)
    {
        char *name_w_dir = 0;
        DBquadmesh *qm = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, qmesh_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;

            DetermineFileAndDirectory(qmesh_names[i],"", correctFile, realvar);
            qm = DBGetQuadmesh(correctFile, realvar.c_str());
            if (qm == NULL)
            {
                valid_var = false;
                qm = DBAllocQuadmesh(); // to fool code block below
            }

            avtMeshType   mt;
            switch (qm->coordtype)
            {
              case DB_QUAD_RECT:
                mt = AVT_RECTILINEAR_MESH;
                break;
              case DB_QUAD_CURV:
                mt = AVT_CURVILINEAR_MESH;
                break;
              default:
                mt = AVT_UNKNOWN_MESH;
                break;
            }

            double extents[6];
            double *extents_to_use = NULL;
            if (nTimesteps == 1 && valid_var)
            {
                if (qm->datatype == DB_DOUBLE)
                {
                    double *min_extents_double = (double *) qm->min_extents;
                    double *max_extents_double = (double *) qm->max_extents;
                    for (j = 0 ; j < qm->ndims ; j++)
                    {
                        extents[2*j    ] = min_extents_double[j];
                        extents[2*j + 1] = max_extents_double[j];
                    }
                }
                else
                {
                    for (j = 0 ; j < qm->ndims ; j++)
                    {
                        extents[2*j    ] = qm->min_extents[j];
                        extents[2*j + 1] = qm->max_extents[j];
                    }
                }
                extents_to_use = extents;
            }

            avtMeshMetaData *mmd = new avtMeshMetaData(NULL,
                                                   extents_to_use,
                                                   name_w_dir, 1, 0,
                                                   qm->origin, 0,
                                                   qm->ndims, qm->ndims,
                                                   mt);
            if (qm->units[0] != NULL)
                mmd->xUnits = qm->units[0];
            if (qm->units[1] != NULL)
                mmd->yUnits = qm->units[1];
            if (qm->units[2] != NULL)
                mmd->zUnits = qm->units[2];

            if (qm->labels[0] != NULL)
                mmd->xLabel = qm->labels[0];
            if (qm->labels[1] != NULL)
                mmd->yLabel = qm->labels[1];
            if (qm->labels[2] != NULL)
                mmd->zLabel = qm->labels[2];

            if (qm->ndims == 2 && qm->coord_sys == DB_CYLINDRICAL)
                mmd->meshCoordType = AVT_RZ;
    
            mmd->validVariable = valid_var;
            mmd->groupTitle = "blocks";
            mmd->groupPieceName = "block";
            mmd->hideFromGUI = qm->guihide;
            md->Add(mmd);
        }
        CATCHALL
        {
            debug1 << "Invalidating quad mesh \"" << qmesh_names[i] << "\"" << endl;
            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                0, 0, 0, 0, 0, 0, AVT_CURVILINEAR_MESH);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        // Add block decomp as variable if needed
        if (i == 0 && 
            addBlockDecompositionAsVar &&
            !haveAddedBlockDecompositionAsVar)
        {
            char cwd[512];
            DBGetDir(dbfile, cwd);
            if (!strcmp(cwd, "/"))
            {
                SNPRINTF(cwd, sizeof(cwd), "%s_block_nums", name_w_dir);
                md->Add(new avtScalarMetaData(cwd, name_w_dir, AVT_ZONECENT));
                haveAddedBlockDecompositionAsVar = true;
            }
        }

        if (name_w_dir) delete [] name_w_dir;
        if (qm) DBFreeQuadmesh(qm);
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Tue Jul 10 19:55:37 PDT 2012
//    Added logic to handle nodelist vars defined in MRG Trees.
//
//    Katheen Biagas, Thu Jun 6 13:11:27 PDT 2013
//    Enable reading of mrgtrees on Windows.
//
// ****************************************************************************

void
avtSiloFileFormat::ReadUcdmeshes(DBfile *dbfile,
    int nucdmesh, char **ucdmesh_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j;
    for (i = 0 ; i < nucdmesh ; i++)
    {
        char *name_w_dir = 0;
        DBucdmesh *um = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, ucdmesh_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;

            DetermineFileAndDirectory(ucdmesh_names[i],"",
                                      correctFile,
                                      realvar);
            um = DBGetUcdmesh(correctFile, realvar.c_str());
            if (um == NULL)
            {
                valid_var = false;
                um = DBAllocUcdmesh(); // to fool code block below
            }

            double   extents[6];
            double  *extents_to_use = NULL;
            if (nTimesteps == 1 && valid_var)
            {
                if (um->datatype == DB_DOUBLE)
                {
                    double *min_extents_double = (double *) um->min_extents;
                    double *max_extents_double = (double *) um->max_extents;
                    for (j = 0 ; j < um->ndims ; j++)
                    {
                        extents[2*j    ] = min_extents_double[j];
                        extents[2*j + 1] = max_extents_double[j];
                    }
                }
                else
                {
                    for (j = 0 ; j < um->ndims ; j++)
                    {
                        extents[2*j    ] = um->min_extents[j];
                        extents[2*j + 1] = um->max_extents[j];
                    }
                }
                extents_to_use = extents;
            }

                if (um->mrgtree_name != 0)
                {
                    // So far, we've coded only for MRG trees representing AMR hierarchies
                    HandleMrgtreeNodelistVars(dbfile, name_w_dir, um->mrgtree_name, md);
                }

            // Handle data-specified topological dimension if its available
            int tdims = um->ndims;
            if (um->topo_dim != -1)
                tdims = um->topo_dim;

            avtMeshMetaData *mmd = new avtMeshMetaData(NULL,
                extents_to_use, name_w_dir, 1, 0, um->origin, 0,
                um->ndims, tdims, AVT_UNSTRUCTURED_MESH);
            if (um->units[0] != NULL)
               mmd->xUnits = um->units[0];
            if (um->units[1] != NULL)
               mmd->yUnits = um->units[1];
            if (um->units[2] != NULL)
               mmd->zUnits = um->units[2];

            if (um->labels[0] != NULL)
                mmd->xLabel = um->labels[0];
            if (um->labels[1] != NULL)
                mmd->yLabel = um->labels[1];
            if (um->labels[2] != NULL)
                mmd->zLabel = um->labels[2];

            if (um->ndims == 2 && um->coord_sys == DB_CYLINDRICAL)
                mmd->meshCoordType = AVT_RZ;

            mmd->groupTitle = "blocks";
            mmd->groupPieceName = "block";
            mmd->disjointElements = hasDisjointElements;
            mmd->validVariable = valid_var;
            mmd->hideFromGUI = um->guihide;
            md->Add(mmd);
        }
        CATCHALL
        {
            debug1 << "Invalidating ucd mesh \"" << ucdmesh_names[i] << "\"" << endl;
            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                0, 0, 0, 0, 0, 0, AVT_UNSTRUCTURED_MESH);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        if (um) DBFreeUcdmesh(um);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadPointmeshes(DBfile *dbfile,
    int nptmesh, char **ptmesh_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < nptmesh ; i++)
    {
        char *name_w_dir = 0;
        DBpointmesh *pm = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, ptmesh_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;

            DetermineFileAndDirectory(ptmesh_names[i],"",
                                      correctFile,
                                      realvar);
            pm = DBGetPointmesh(correctFile, realvar.c_str());
            if (pm == NULL)
            {
                valid_var = false;
                pm = DBAllocPointmesh(); // to fool code block below
            }

            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir, 1, 0,pm->origin,
                0, pm->ndims, 0, AVT_POINT_MESH);

            mmd->groupTitle = "blocks";
            mmd->hideFromGUI = pm->guihide;
            mmd->groupPieceName = "block";
            mmd->validVariable = valid_var;

            if (pm->units[0] != NULL)
                mmd->xUnits = pm->units[0];
            if (pm->units[1] != NULL)
                mmd->yUnits = pm->units[1];
            if (pm->units[2] != NULL)
                mmd->zUnits = pm->units[2];

            if (pm->labels[0] != NULL)
                mmd->xLabel = pm->labels[0];
            if (pm->labels[1] != NULL)
                mmd->yLabel = pm->labels[1];
            if (pm->labels[2] != NULL)
                mmd->zLabel = pm->labels[2];

            md->Add(mmd);
        }
        CATCHALL
        {
            debug1 << "Invalidating point mesh \"" << ptmesh_names[i] << "\"" << endl;
            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                0, 0, 0, 0, 0, 0, AVT_POINT_MESH);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        if (pm) DBFreePointmesh(pm);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadCurves(DBfile *dbfile,
    int ncurves, char **curve_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < ncurves; i++)
    {
        char *name_w_dir = 0;
        DBcurve *cur = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, curve_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;

            DetermineFileAndDirectory(curve_names[i],"",correctFile,realvar);
            cur = DBGetCurve(correctFile, realvar.c_str());
            if (cur == NULL)
            {
                valid_var = false;
                cur = DBAllocCurve(); // to fool code block below
            }


            avtCurveMetaData *cmd = new avtCurveMetaData(name_w_dir);
            if (cur->xlabel != NULL)
                cmd->xLabel = cur->xlabel;
            if (cur->ylabel != NULL)
                cmd->yLabel = cur->ylabel;
            if (cur->xunits != NULL)
                cmd->xUnits = cur->xunits;
            if (cur->yunits != NULL)
                cmd->yUnits = cur->yunits;
            cmd->validVariable = valid_var;
            cmd->hideFromGUI = cur->guihide;
            md->Add(cmd);


        }
        CATCHALL
        {
            debug1 << "Invalidating curve \"" << curve_names[i] << "\"" << endl;
            avtCurveMetaData *cmd = new avtCurveMetaData(name_w_dir);
            cmd->validVariable = false;
            md->Add(cmd);
        }
        ENDTRY

        if (cur) DBFreeCurve(cur);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadCSGmeshes(DBfile *dbfile,
    int ncsgmesh, char **csgmesh_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    size_t i,j;
    for (i = 0 ; i < (size_t)ncsgmesh ; i++)
    {
        char *name_w_dir = 0;
        DBcsgmesh *csgm = 0;

        TRY
        {
           name_w_dir = GenerateName(dirname, csgmesh_names[i], topDir.c_str());
           string realvar;
           DBfile *correctFile = dbfile;
           bool valid_var = true;

           DetermineFileAndDirectory(csgmesh_names[i], "", correctFile, realvar);

           // We want to read the header for the csg zonelist too
           // so we can serve up the "zones" of a csg mesh as "blocks"
           unsigned long long mask = DBGetDataReadMask2();
           DBSetDataReadMask2(mask|DBCSGMZonelist|DBCSGZonelistZoneNames);
           csgm = DBGetCsgmesh(correctFile, realvar.c_str());
           DBSetDataReadMask2(mask);
           if (csgm == NULL || csgm->zones == NULL)
           {
               debug1 << "Unable to read mesh \"" << csgmesh_names[i]
                      << "\". Skipping it" << endl; 
               valid_var = false;
               csgm = DBAllocCsgmesh();
               csgm->zones = DBAllocCSGZonelist();
           }

           double   extents[6];
           double  *extents_to_use = NULL;
           if (!((csgm->min_extents[0] == 0.0 && csgm->max_extents[0] == 0.0 &&
                  csgm->min_extents[1] == 0.0 && csgm->max_extents[1] == 0.0 &&
                  csgm->min_extents[2] == 0.0 && csgm->max_extents[2] == 0.0) ||
                 (csgm->min_extents[0] == -DBL_MAX && csgm->max_extents[0] == DBL_MAX &&
                  csgm->min_extents[1] == -DBL_MAX && csgm->max_extents[1] == DBL_MAX &&
                  csgm->min_extents[2] == -DBL_MAX && csgm->max_extents[2] == DBL_MAX)))
           {
               for (j = 0 ; j < (size_t)csgm->ndims ; j++)
               {
                   extents[2*j    ] = csgm->min_extents[j];
                   extents[2*j + 1] = csgm->max_extents[j];
               }
               extents_to_use = extents;
           }

           avtMeshMetaData *mmd = new avtMeshMetaData(NULL,
               extents_to_use, name_w_dir, csgm->zones->nzones, 0, csgm->origin, 0,
               csgm->ndims, csgm->ndims, AVT_CSG_MESH);
           if (csgm->units[0] != NULL)
              mmd->xUnits = csgm->units[0];
           if (csgm->units[1] != NULL)
              mmd->yUnits = csgm->units[1];
           if (csgm->units[2] != NULL)
              mmd->zUnits = csgm->units[2];

           if (csgm->labels[0] != NULL)
               mmd->xLabel = csgm->labels[0];
           if (csgm->labels[1] != NULL)
               mmd->yLabel = csgm->labels[1];
           if (csgm->labels[2] != NULL)
               mmd->zLabel = csgm->labels[2];

           mmd->blockTitle = "regions";
           mmd->validVariable = valid_var;
           mmd->hideFromGUI = csgm->guihide;
           if (csgm->zones->zonenames)
           {
               vector<string> znames;
               for (j = 0; j < (size_t)csgm->zones->nzones; j++)
                   znames.push_back(csgm->zones->zonenames[j]);
               mmd->blockNames = znames;
           }

           md->Add(mmd);
        }
        CATCHALL
        {
            debug1 << "Invalidating csg mesh \"" << csgmesh_names[i] << "\"" << endl;
            avtMeshMetaData *mmd = new avtMeshMetaData(name_w_dir,
                0, 0, 0, 0, 0, 0, AVT_CSG_MESH);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        if (csgm) DBFreeCsgmesh(csgm);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Function: GetRestrictedMaterialIndices
//
//  Purpose:
//     Get the material indices (not the same as matnos) to which a
//     subsetting variable is restricted.
//
//  Programmer: Mark C. Miller
//  Creation:   August 30, 2010
//
//  Modifications:
//
//   Hank Childs, Sat Jan  1 15:11:10 PST 2011
//   Fix sscanf problem from compiler warning.
//
// ****************************************************************************

static bool
GetRestrictedMaterialIndices(const avtDatabaseMetaData *md, const char *const varname,
    const char *const meshname, const char *const *const region_pnames,
    vector<int>* restrictToMats)
{
    // Find associated material info.
    const avtMaterialMetaData *mmd = md->GetMaterialOnMesh(meshname);

    if (!mmd)
    {
        debug3 << "The variable \"" << varname << "\" appears to be defined on specific regions." << endl
               << "However, an attempt to get a material object for the associated mesh" << endl
               << "\"" << meshname << "\" has failed. So, it is being ignored" << endl;
        return false;
    }

    size_t i, j;
    bool regionNamesButMaterialNumbers = true;
    debug3 << "For mat-restricted variable \"" << varname << "\"..." << endl;
    for (i = 0; region_pnames[i]; i++)
    {
        debug3 << "    Looking for materials matching region_pname[" << i
               << "] = \"" << region_pnames[i] << "\"..." << endl;
        int rnlen = strlen(region_pnames[i]);
        int rnspn = strspn(region_pnames[i], "0123456789");
        if (rnlen == rnspn) // Must be just a number
        {
            int regno = strtol(region_pnames[i], 0, 10);
            regionNamesButMaterialNumbers = false;
            debug3 << "        Comparing using regno=" << regno << "..." << endl; 
            for (j = 0; j < (size_t)mmd->numMaterials; j++)
            {
                // The 'materialNames' Silo plugin creates are either
                // "%d (matno)" or "%d (matno) %s (matname)". Either way,
                // strtol should convert the number part correctly.
                errno = 0;
                int matno = strtol(mmd->materialNames[j].c_str(), 0, 10);
                debug3 << "            for \"" << mmd->materialNames[j]
                       << "\" got matno=" << matno; 
                if (errno == 0 && regno == matno)
                {
                    debug3 << " matched" << endl;
                    restrictToMats->push_back(j);
                    break;
                }
                else
                {
                    debug3 << " DID NOT match" << endl;
                }
            }
        }
        else // Not just a number (a name?)
        {
            debug3 << "        Comparing using region name..." << endl;
            for (j = 0; j < (size_t)mmd->numMaterials; j++)
            {
                int matno;
                char matname[256];
                int nmatches = sscanf(mmd->materialNames[j].c_str(), "%d %s", &matno, matname);
                debug3 << "            matno=" << matno << ", matname=\"" << matname << "\"";
                if (nmatches == 1) // have matno only
                {
                    debug3 << " CANNOT match matno alone" << endl;
                    continue;
                }
                else
                {
                    if (!strcmp(region_pnames[i], matname))
                    {
                        debug3 << " matched" << endl;
                        restrictToMats->push_back(j);
                        break;
                    }
                    else
                    {
                        debug3 << " DID NOT match" << endl;
                    }
                }
            }
        }
    }

    if (restrictToMats->size() == 0)
    {
        debug3 << "The variable \"" << varname << "\" appears to be defined on subsets of the mesh." << endl
               << "However, attempts to match the named subsets to material names or numbers" << endl
               << "has failed. " << endl;
        if (regionNamesButMaterialNumbers)
        {
            debug3 << "This is due to fact that the database specifies materials by number only" << endl
                   << "and yet the region(s) the variable is defined on are specified by name" << endl;
        }
        return false;
    }

    debug3 << "Variable \"" << varname << "\" is restricted to the following regions..." << endl;
    for (i = 0; i < restrictToMats->size(); i++)
        debug3 << "   \"" << mmd->materialNames[restrictToMats->operator[](i)] << "\" (mat-index = "
               << restrictToMats->operator[](i) << ")" << endl;

    return true;
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names. Also added logic to
//    handle freeing of multivar during exceptions.
//
//    Mark C. Miller, Wed Nov  3 09:24:48 PDT 2010
//    Don't call GetRestrictedMaterialIndices on individual blocks if we
//    already have 'em from the multi-block.
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Mon Aug 11 20:09:51 PDT 2014
//    Decode and utilize multivar's tensor_rank member if its set. Populate
//    metadata accordingly.
//
//    Eric Brugger, Wed Mar 11 11:03:57 PDT 2015
//    I corrected a bug where a NULL pointer would be de-referenced causing
//    a crash if a multivar was completely empty.
//
//    Mark C. Miller, Mon Nov  9 17:14:21 PST 2015
//    Adjusted setting of associated meshname from objects 'mmesh_name' member
//    to use absolute path name instead of just whatever the Silo file had
//    stored.
//
//    Mark C. Miller, Tue Feb  2 15:01:05 PST 2016
//    Add support for all-empty multi-vars.
//
//    Mark C. Miller, Thu Feb 25 12:40:17 PST 2016
//    Add logic to check mesh identified by mmesh_name member and then fall
//    back to fuzzy match if it doesn't exist.
// ****************************************************************************
void
avtSiloFileFormat::ReadMultivars(DBfile *dbfile,
    int nmultivar, char **multivar_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < nmultivar ; i++)
    {
        char *name_w_dir = 0;
        DBmultivar *mv = 0;
        avtSiloMultiVarCacheEntry *mv_ent = NULL;
        TRY
        {

            name_w_dir = GenerateName(dirname, multivar_names[i], topDir.c_str());
            string meshname;
            string mb_varname = "";
            int meshnum = 0;
            int tensor_rank = 0;
            bool valid_var = true;
            bool is_all_empty = false;
            GetMultiVar(dirname, multivar_names[i], &mv_ent, &valid_var);

            if(mv_ent != NULL)
                mv = mv_ent->DataObject();

            if (mv != NULL)
            {
                RegisterDomainDirs(mv_ent, dirname);
                meshnum = FindFirstNonEmptyBlock(multivar_names[i], mv->nvars,
                    mv_ent, mv->repr_block_idx, mv->empty_cnt, mv->empty_list);
                if (meshnum < 0)
                {
                    is_all_empty = true;
                    emptyObjectsList[name_w_dir] = true;
                    meshname = firstAllEmptyMultimesh;
                }
                else
                    mb_varname = mv_ent->GenerateName(meshnum);

                if (valid_var && !is_all_empty)
                {
                    if (mv->mmesh_name != 0)
                    {
                        char cwd[512];
                        DBGetDir(dbfile, cwd);
                        meshname = FileFunctions::Absname(cwd,mv->mmesh_name,"/");
                        if (!DBInqVarExists(dbfile, meshname.c_str()))
                        {
                            debug5 << "Although 'mmesh_name' member indicates variable \"" << multivar_names[i] 
                                   << "\" is defined on mesh \"" << meshname.c_str() << "\", "
                                   << "the associated mesh does not exist." << endl;
                            meshname = DetermineMultiMeshForSubVariable(dbfile,
                                multivar_names[i], mv_ent, dirname);
                            debug5 << "Guessing variable \"" << multivar_names[i] 
                                   << "\" is defined on mesh \""
                                   << meshname.c_str() << "\"" << endl;
                        }
                        else
                        {
                            debug5 << "Variable \"" << multivar_names[i] 
                                   << "\" indicates it is defined on mesh \""
                                   << meshname.c_str() << "\"" << endl;
                            // VisIt won't match this var to its mesh if leading slash is presnet
                            meshname.erase(meshname.begin());
                        }
                    }
                    else
                    {
                        // NOTE: There is an explicit assumption that the corresponding
                        //       multimesh has already been read.  Thus it must reside
                        //       in the same directory (or a previously read one) as
                        //       this variable.
                            meshname = DetermineMultiMeshForSubVariable(dbfile,
                                multivar_names[i], mv_ent, dirname);
                            debug5 << "Guessing variable \"" << multivar_names[i] 
                                   << "\" is defined on mesh \""
                                   << meshname.c_str() << "\"" << endl;
                    }
                }
            }
            else
            {
                debug1 << "Invalidating var \"" << multivar_names[i] << "\"" << endl;
                valid_var = false;
            }

            //
            // If this variable is defined on material subset(s), determine
            // the associated material numbers and indi
            //
            vector<int> selectedMats;
            if (mv && mv->region_pnames)
                valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                    meshname.c_str(), mv->region_pnames, &selectedMats);

            //
            // Get the centering and dimension information.
            //
            avtCentering   centering = AVT_ZONECENT; ///TODO: check on uniinit variable
            bool           treatAsASCII = false;
            string realvar;
            DBfile *correctFile = dbfile;
            string  varUnits;
            int nvals = 1;
            double missing_value = DB_MISSING_VALUE_NOT_SET;
            if (valid_var && mv && !is_all_empty)
            {
                if (mv->missing_value != DB_MISSING_VALUE_NOT_SET)
                    missing_value = mv->missing_value;
                if (mv->tensor_rank != 0)
                    tensor_rank = mv->tensor_rank;

                DetermineFileAndDirectory(mb_varname.c_str(),"",
                                          correctFile, realvar);

                switch (mv_ent->VarType(meshnum))
                {
                  case DB_UCDVAR:
                    {
                        DBucdvar *uv = NULL;
                        uv = DBGetUcdvar(correctFile, realvar.c_str());
                        if (uv == NULL)
                        {
                            valid_var = false;
                            break;
                        }
                        centering = (uv->centering == DB_ZONECENT ? AVT_ZONECENT 
                                                                  : AVT_NODECENT);
                        if (uv->region_pnames && !selectedMats.size())
                            valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                                meshname.c_str(), uv->region_pnames, &selectedMats);
                        nvals = uv->nvals;
                        treatAsASCII = (uv->ascii_labels);
                        if(uv->units != 0)
                            varUnits = string(uv->units);
                        if (missing_value == DB_MISSING_VALUE_NOT_SET &&
                            uv->missing_value != DB_MISSING_VALUE_NOT_SET)
                            missing_value = uv->missing_value;
                        DBFreeUcdvar(uv);
                    }
                    break;
        
                  case DB_QUADVAR:
                    {
                        DBquadvar *qv = DBGetQuadvar(correctFile, realvar.c_str());
                        if (qv == NULL)
                        {
                            valid_var = false;
                            break;
                        }
                        centering = (qv->align[0] == 0. ? AVT_NODECENT 
                                                        : AVT_ZONECENT);
                        if (qv->region_pnames && !selectedMats.size())
                            valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                                meshname.c_str(), qv->region_pnames, &selectedMats);
                        nvals = qv->nvals;
                        treatAsASCII = (qv->ascii_labels);
                        if(qv->units != 0)
                            varUnits = string(qv->units);
                        if (missing_value == DB_MISSING_VALUE_NOT_SET &&
                            qv->missing_value != DB_MISSING_VALUE_NOT_SET)
                            missing_value = qv->missing_value;
                        DBFreeQuadvar(qv);
                    }
                    break;
        
                  case DB_POINTVAR:
                    {
                        centering = AVT_NODECENT;   // Only one possible
                        DBmeshvar *pv = DBGetPointvar(correctFile, realvar.c_str());
                        if (pv == NULL)
                        {
                            valid_var = false;
                            break;
                        }
                        if (pv->region_pnames && !selectedMats.size())
                            valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                                meshname.c_str(), pv->region_pnames, &selectedMats);
                        nvals = pv->nvals;
                        treatAsASCII = (pv->ascii_labels);
                        if(pv->units != 0)
                            varUnits = string(pv->units);
                        if (missing_value == DB_MISSING_VALUE_NOT_SET &&
                            pv->missing_value != DB_MISSING_VALUE_NOT_SET)
                            missing_value = pv->missing_value;
                        DBFreeMeshvar(pv);
                    }
                    break;

                  case DB_CSGVAR:
                    {
                        DBcsgvar *csgv = DBGetCsgvar(correctFile, realvar.c_str());
                        centering = csgv->centering == DB_BNDCENT ? AVT_NODECENT
                                                                  : AVT_ZONECENT;
                        if (csgv == NULL)
                        {
                            valid_var = false;
                            break;
                        }
                        if (csgv->region_pnames && !selectedMats.size())
                            valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                                meshname.c_str(), csgv->region_pnames, &selectedMats);
                        nvals = csgv->nvals;
                        treatAsASCII = (csgv->ascii_labels);
                        if(csgv->units != 0)
                            varUnits = string(csgv->units);
                        if (missing_value == DB_MISSING_VALUE_NOT_SET &&
                            csgv->missing_value != DB_MISSING_VALUE_NOT_SET)
                            missing_value = csgv->missing_value;
                        DBFreeCsgvar(csgv);
                    }
                    break;
                  default:
                    ; // No-op. Compiler complains about a break here.
                }
            }

            if (nvals == 1 || tensor_rank == DB_VARTYPE_SCALAR)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                           meshname, centering);
                smd->validVariable = valid_var;
                smd->treatAsASCII = treatAsASCII;
                smd->hideFromGUI = mv ? mv->guihide : 0;
                smd->matRestricted = selectedMats;
                if(varUnits != "")
                {
                    smd->hasUnits = true;
                    smd->units = varUnits;
                }

                //
                // Handle special cases for enumerated variables
                //
                if (valid_var && codeNameGuess == "Ale3d" &&
                    strstr(multivar_names[i], "rlxstat") != 0)
                    AddAle3drlxstatEnumerationInfo(smd);

                if (missing_value != DB_MISSING_VALUE_NOT_SET)
                {
                    double arr[2] = {0., 0.};
                    arr[0] = missing_value;
                    smd->SetMissingData(arr);
                    smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
                }
                md->Add(smd);
            }
            else if (treatAsASCII)
            {
                avtLabelMetaData *lmd = new avtLabelMetaData(name_w_dir, meshname, centering);
                lmd->validVariable = valid_var;
                lmd->hideFromGUI = mv ? mv->guihide : 0;
                lmd->matRestricted = selectedMats;
                md->Add(lmd);
            }
            else
            {
                if (tensor_rank == DB_VARTYPE_TENSOR)
                {
                    avtTensorMetaData *tmd = new avtTensorMetaData(name_w_dir, meshname, centering, nvals);
                    tmd->validVariable = valid_var;
                    tmd->hideFromGUI = mv ? mv->guihide: 0;
                    tmd->matRestricted = selectedMats;
                    if(varUnits != "")
                    {
                        tmd->hasUnits = true;
                        tmd->units = varUnits;
                    }
                    md->Add(tmd);
                }
                else if (tensor_rank == DB_VARTYPE_SYMTENSOR)
                {
                    avtSymmetricTensorMetaData *tmd = new avtSymmetricTensorMetaData(name_w_dir, meshname, centering, nvals);
                    tmd->validVariable = valid_var;
                    tmd->hideFromGUI = mv ? mv->guihide: 0;
                    tmd->matRestricted = selectedMats;
                    if(varUnits != "")
                    {
                        tmd->hasUnits = true;
                        tmd->units = varUnits;
                    }
                    md->Add(tmd);
                }
                else if (tensor_rank == DB_VARTYPE_ARRAY)
                {
                    avtArrayMetaData *amd = new avtArrayMetaData(name_w_dir, meshname, centering, nvals);
                    amd->validVariable = valid_var;
                    amd->hideFromGUI = mv ? mv->guihide: 0;
                    amd->matRestricted = selectedMats;
                    if(varUnits != "")
                    {
                        amd->hasUnits = true;
                        amd->units = varUnits;
                    }
                    md->Add(amd);
                }
                else
                {
                    avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir, meshname, centering, nvals);
                    vmd->validVariable = valid_var;
                    vmd->hideFromGUI = mv ? mv->guihide: 0;
                    vmd->matRestricted = selectedMats;
                    if(varUnits != "")
                    {
                        vmd->hasUnits = true;
                        vmd->units = varUnits;
                    }
                    md->Add(vmd);
                }
            }
        }
        CATCHALL
        {
            // We explicitly free the multivar here and NOT at the end of this function
            // because in 'normal' operation, the multivar gets cached during the
            // GetMultivar() call, above, and free'd later.
            if (mv)
            {
                // Make sure its removed from the plugin's cache, too.
                multivarCache.RemoveEntry(dirname, multivar_names[i]);
            }

            debug1 << "Invalidating multi-var \"" << multivar_names[i] << "\"" << endl;
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       "unknown", AVT_UNKNOWN_CENT);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadQuadvars(DBfile *dbfile,
    int nqvar, char **qvar_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < nqvar ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBquadvar *qv = 0;

        TRY
        {

            name_w_dir = GenerateName(dirname, qvar_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;
            DetermineFileAndDirectory(qvar_names[i], "", correctFile, realvar);
            qv = DBGetQuadvar(correctFile, realvar.c_str());
            if (qv == NULL)
            {
                valid_var = false;
                qv = DBAllocQuadvar();
            }

            char meshname[256];
            DBInqMeshname(correctFile, realvar.c_str(), meshname);
            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());

            //
            // Get the centering information.
            //
            avtCentering   centering = (qv->align[0] == 0. ? AVT_NODECENT :
                                                             AVT_ZONECENT);
            bool guiHide = qv->guihide;

            //
            // If this variable is defined on material subset(s), determine
            // the associated material numbers and indi
            //
            vector<int> selectedMats;
            if (qv->region_pnames)
                valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                    meshname_w_dir, qv->region_pnames, &selectedMats);

            //
            // Get the dimension of the variable.
            //
            if (qv->nvals == 1)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                             meshname_w_dir, centering);
                smd->treatAsASCII = (qv->ascii_labels);
                smd->validVariable = valid_var;
                smd->hideFromGUI = guiHide;
                smd->matRestricted = selectedMats;
                if(qv->units != 0)
                {
                    smd->hasUnits = true;
                    smd->units = string(qv->units);
                }

                if (qv->missing_value != DB_MISSING_VALUE_NOT_SET)
                {
                    double arr[2] = {0., 0.};
                    arr[0] = qv->missing_value;
                    smd->SetMissingData(arr);
                    smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
                }
                md->Add(smd);
            }
            else if (qv->ascii_labels)
            {
                avtLabelMetaData *lmd = new avtLabelMetaData(name_w_dir,
                                            meshname_w_dir, centering);
                lmd->validVariable = valid_var;
                lmd->hideFromGUI = guiHide;
                lmd->matRestricted = selectedMats;
                md->Add(lmd);
            }
            else
            {
                avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                             meshname_w_dir, centering, qv->nvals);
                vmd->validVariable = valid_var;
                vmd->hideFromGUI = guiHide;
                vmd->matRestricted = selectedMats;
                if(qv->units != 0)
                {
                    vmd->hasUnits = true;
                    vmd->units = string(qv->units);
                }
                md->Add(vmd);
            }
        }
        CATCHALL
        {
            debug1 << "Invalidating quad var \"" << qvar_names[i] << "\"" << endl;
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       "unknown", AVT_UNKNOWN_CENT);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (qv) DBFreeQuadvar(qv);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadUcdvars(DBfile *dbfile,
    int nucdvar, char **ucdvar_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < nucdvar ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBucdvar *uv = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, ucdvar_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;
            DetermineFileAndDirectory(ucdvar_names[i], "", correctFile, realvar);
            uv = DBGetUcdvar(correctFile, realvar.c_str());
            if (uv == NULL)
            {
                valid_var = false;
                uv = DBAllocUcdvar();
            }

            char meshname[256];
            DBInqMeshname(correctFile, realvar.c_str(), meshname);
            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());

            //
            // Get the centering information.
            //
            avtCentering centering = (uv->centering == DB_ZONECENT ? AVT_ZONECENT
                                                               : AVT_NODECENT);
            bool guiHide = uv->guihide;

            //
            // If this variable is defined on material subset(s), determine
            // the associated material numbers and indi
            //
            vector<int> selectedMats;
            if (uv->region_pnames)
                valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                    meshname_w_dir, uv->region_pnames, &selectedMats);
            //
            // Get the dimension of the variable.
            //
            if (uv->nvals == 1)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                    meshname_w_dir, centering);
                smd->validVariable = valid_var;
                smd->treatAsASCII = (uv->ascii_labels);
                smd->hideFromGUI = guiHide;
                smd->matRestricted = selectedMats;
                if(uv->units != 0)
                {
                    smd->hasUnits = true;
                    smd->units = string(uv->units);
                }

                if (uv->missing_value != DB_MISSING_VALUE_NOT_SET)
                {
                    double arr[2] = {0., 0.};
                    arr[0] = uv->missing_value;
                    smd->SetMissingData(arr);
                    smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
                }
                md->Add(smd);
            }
            else if (uv->ascii_labels)
            {
                avtLabelMetaData *lmd = new avtLabelMetaData(name_w_dir, meshname_w_dir, centering);
                lmd->validVariable = valid_var;
                lmd->hideFromGUI = guiHide;
                lmd->matRestricted = selectedMats;
                md->Add(lmd);
            }
            else
            {
                avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                             meshname_w_dir, centering, uv->nvals);
                vmd->validVariable = valid_var;
                vmd->hideFromGUI = guiHide;
                vmd->matRestricted = selectedMats;
                if(uv->units != 0)
                {
                    vmd->hasUnits = true;
                    vmd->units = string(uv->units);
                }
                md->Add(vmd);
            }
        }
        CATCHALL
        {
            debug1 << "Invalidating ucd var \"" << ucdvar_names[i] << "\"" << endl;
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       "unknown", AVT_UNKNOWN_CENT);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (uv) DBFreeUcdvar(uv);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadPointvars(DBfile *dbfile,
    int nptvar, char **ptvar_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < nptvar ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBmeshvar *pv = 0;

        TRY
        {

            name_w_dir = GenerateName(dirname, ptvar_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;
            DetermineFileAndDirectory(ptvar_names[i], "", correctFile, realvar);
            pv = DBGetPointvar(correctFile, realvar.c_str());
            if (pv == NULL)
            {
                valid_var = false;
                pv = DBAllocMeshvar();
            }

            char meshname[256];
            DBInqMeshname(correctFile, realvar.c_str(), meshname);

            //
            // Get the dimension of the variable.
            //
            bool guiHide = pv->guihide;
            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());
            if (pv->nvals == 1)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                meshname_w_dir, AVT_NODECENT);
                smd->treatAsASCII = (pv->ascii_labels);
                smd->validVariable = valid_var;
                smd->hideFromGUI = guiHide;
                if(pv->units != 0)
                {
                    smd->hasUnits = true;
                    smd->units = string(pv->units);
                }
                if (pv->missing_value != DB_MISSING_VALUE_NOT_SET)
                {
                    double arr[2] = {0., 0.};
                    arr[0] = pv->missing_value;
                    smd->SetMissingData(arr);
                    smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
                }
                md->Add(smd);
            }
            else if (pv->ascii_labels)
            {
                avtLabelMetaData *lmd = new avtLabelMetaData(name_w_dir, meshname_w_dir, AVT_NODECENT);
                lmd->validVariable = valid_var;
                lmd->hideFromGUI = guiHide;
                md->Add(lmd);
            }
            else
            {
                avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                          meshname_w_dir, AVT_NODECENT, pv->nvals);
                vmd->validVariable = valid_var;
                vmd->hideFromGUI = guiHide;
                if(pv->units != 0)
                {
                    vmd->hasUnits = true;
                    vmd->units = string(pv->units);
                }
                md->Add(vmd);
            }
        }
        CATCHALL
        {
            debug1 << "Invalidating point var \"" << ptvar_names[i] << "\"" << endl;
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       "unknown", AVT_UNKNOWN_CENT);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (pv) DBFreeMeshvar(pv);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadCSGvars(DBfile *dbfile,
    int ncsgvar, char **csgvar_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i;
    for (i = 0 ; i < ncsgvar ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBcsgvar *csgv = 0;

        TRY
        {

            name_w_dir = GenerateName(dirname, csgvar_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;
            DetermineFileAndDirectory(csgvar_names[i], "", correctFile, realvar);
            csgv = DBGetCsgvar(correctFile, realvar.c_str());
            if (csgv == NULL)
            {
                valid_var = false;
                csgv = DBAllocCsgvar();
            }

            char meshname[256]; 
            DBInqMeshname(correctFile, realvar.c_str(), meshname);

            //
            // Get the centering information.
            //
            // AVT doesn't have a 'boundary centering'. So, use node centering.
            avtCentering centering = (csgv->centering == DB_BNDCENT ? AVT_NODECENT
                                                                   : AVT_ZONECENT);
            bool guiHide = csgv->guihide;

            //
            // If this variable is defined on material subset(s), determine
            // the associated material numbers and indi
            //
            vector<int> selectedMats;
            if (csgv->region_pnames)
                valid_var = GetRestrictedMaterialIndices(md, name_w_dir,
                    meshname_w_dir, csgv->region_pnames, &selectedMats);

            //
            // Get the dimension of the variable.
            //
            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());
            if (csgv->nvals == 1)
            {
                avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                    meshname_w_dir, centering);
                smd->treatAsASCII = (csgv->ascii_labels);
                smd->validVariable = valid_var;
                smd->hideFromGUI = guiHide;
                smd->matRestricted = selectedMats;
                if(csgv->units != 0)
                {
                    smd->hasUnits = true;
                    smd->units = string(csgv->units);
                }

#ifdef DBOPT_MISSING_VALUE
                if (csgv->missing_value != DB_MISSING_VALUE_NOT_SET)
                {
                    double arr[2] = {0., 0.};
                    arr[0] = csgv->missing_value;
                    smd->SetMissingData(arr);
                    smd->SetMissingDataType(avtScalarMetaData::MissingData_Value);
                }
#endif

                md->Add(smd);
            }
            else if (csgv->ascii_labels)
            {
                avtLabelMetaData *lmd = new avtLabelMetaData(name_w_dir, meshname_w_dir, centering);
                lmd->validVariable = valid_var;
                lmd->hideFromGUI = guiHide;
                lmd->matRestricted = selectedMats;
                md->Add(lmd);
            }
            else
            {
                avtVectorMetaData *vmd = new avtVectorMetaData(name_w_dir,
                                             meshname_w_dir, centering, csgv->nvals);
                vmd->validVariable = valid_var;
                vmd->hideFromGUI = guiHide;
                vmd->matRestricted = selectedMats;
                if(csgv->units != 0)
                {
                    vmd->hasUnits = true;
                    vmd->units = string(csgv->units);
                }
                md->Add(vmd);
            }
        }
        CATCHALL
        {
            debug1 << "Invalidating csg var \"" << csgvar_names[i] << "\"" << endl;
            avtScalarMetaData *smd = new avtScalarMetaData(name_w_dir,
                                                       "unknown", AVT_UNKNOWN_CENT);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (csgv) DBFreeCsgvar(csgv);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Mark C. Miller, Wed Aug 26 11:09:29 PDT 2009
//    Uncommented hidFromGUI setting.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadMaterials(DBfile *dbfile,
    int nmat, char **mat_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j;
    for (i = 0 ; i < nmat ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBmaterial *mat = 0;

        TRY
        {
            name_w_dir = GenerateName(dirname, mat_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true;
            DetermineFileAndDirectory(mat_names[i], "", correctFile, realvar);
            mat = DBGetMaterial(correctFile, realvar.c_str());
            if (mat == NULL)
            {
                valid_var = false;
                mat = DBAllocMaterial();
            }

            char meshname[256];
            DBInqMeshname(correctFile, realvar.c_str(), meshname);

            //
            // Give the materials names based on their material number.  If
            // they have names in the Silo file, use those as well.
            //
            vector<string>  matnames;
            vector<string>  matcolors;
            for (j = 0 ; j < mat->nmat ; j++)
            {
                //
                // Deal with material names
                //
                char *num = NULL;
                int dlen = int(log10(float(mat->matnos[j]+1))) + 1;
                if (mat->matnames == NULL || mat->matnames[j] == NULL)
                {
                    num = new char[dlen + 2];
                    sprintf(num, "%d", mat->matnos[j]);
                }
                else
                {
                    int len = strlen(mat->matnames[j]);
                    num = new char[len + 1 + dlen + 1];
                    sprintf(num, "%d %s", mat->matnos[j], mat->matnames[j]);
                }
                matnames.push_back(num);
                delete[] num;

                //
                // Deal with material colors
                //
                if (mat->matcolors)
                {
                    if (mat->matcolors[j] && mat->matcolors[j][0])
                        matcolors.push_back(mat->matcolors[j]);
                    else
                        matcolors.push_back("");
                }
            }

            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());
            avtMaterialMetaData *mmd;
            if (matcolors.size())
                mmd = new avtMaterialMetaData(name_w_dir, meshname_w_dir,
                                              mat->nmat, matnames, matcolors);
            else
                mmd = new avtMaterialMetaData(name_w_dir, meshname_w_dir,
                                              mat->nmat, matnames);
            mmd->validVariable = valid_var;
            mmd->hideFromGUI = mat->guihide;
            md->Add(mmd);
        }
        CATCHALL
        {
            debug1 << "Invalidating material \"" << mat_names[i] << "\"" << endl;
            vector<string>  no_matnames;
            avtMaterialMetaData *mmd = new avtMaterialMetaData(name_w_dir, "unknown",
                                          0, no_matnames);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        if (mat) DBFreeMaterial(mat);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names. Also added logic to
//    handle free of multimat during exceptions.
//
//    Mark C. Miller, Wed Aug 26 11:09:55 PDT 2009
//    Uncommented hideFromGUI setting.
//
//    Cyrus Harrison, Tue Nov 24 14:05:28 PST 2009
//    Added guard to avoid crash from unset material name.
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Cyrus Harrison, Fri Aug 16 14:25:36 PDT 2013
//    Improve way multimat and mat info are inspected to create material names.
//    Avoid using using a dummy DBMaterial struct and pointer stealing.
//
//    Mark C. Miller, Mon Nov  9 17:14:21 PST 2015
//    Adjusted setting of associated meshname from object's 'mmesh_name' member
//    to use absolute path name instead of just whatever the Silo file had
//    stored.
//
//    Mark C. Miller, Tue Feb  2 15:01:05 PST 2016
//    Add support for all-empty multi-mats.
//
//    Mark C. Miller, Thu Feb 25 12:40:17 PST 2016
//    Add logic to check mesh identified by mmesh_name member and then fall
//    back to fuzzy match if it doesn't exist.
// ****************************************************************************
void
avtSiloFileFormat::ReadMultimats(DBfile *dbfile,
    int nmultimat, char **multimat_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j;
    for (i = 0 ; i < nmultimat ; i++)
    {
        char *name_w_dir = 0;
        DBmultimat *mm = 0;
        DBmaterial *mat = 0;
        avtSiloMultiMatCacheEntry *mm_ent = NULL;
        TRY
        {
            name_w_dir = GenerateName(dirname, multimat_names[i], topDir.c_str());
            bool valid_var = true;
            bool is_all_empty = false;
            GetMultiMat(dirname, multimat_names[i], &mm_ent, &valid_var);
            string mb_matname = "";

            if(mm_ent != NULL)
                mm = mm_ent->DataObject();

            if (mm == NULL)
            {
                valid_var = false;
                mm = DBAllocMultimat(0);
            }
            else
            {
                // if we have a valid object, register the domain dirs.
                RegisterDomainDirs(mm_ent, dirname);
            }

            // use these temp vars 
            int    minfo_nmats = 0;
            int   *minfo_matnos = NULL;
            char **minfo_matnames = NULL;
            char **minfo_matcolors = NULL;

            if (MultiMatHasAllMatInfo(mm) < 3)
            {
                int meshnum = FindFirstNonEmptyBlock(multimat_names[i], mm->nmats,
                    mm_ent, mm->repr_block_idx, mm->empty_cnt, mm->empty_list);
                if (meshnum < 0)
                {
                    is_all_empty = true;
                    emptyObjectsList[name_w_dir] = true;
                }
                else
                    mb_matname = mm_ent->GenerateName(meshnum);

                string realvar;
                DBfile *correctFile = dbfile;

                if (valid_var && !is_all_empty)
                {
                    DetermineFileAndDirectory(mb_matname.c_str(),"", correctFile, realvar);
                    mat = DBGetMaterial(correctFile, realvar.c_str());
                }

                if (mat)
                {
                    //Get all the info from the mat obj.
                    minfo_nmats     = mat->nmat;
                    minfo_matnos    = mat->matnos;
                    minfo_matnames  = mat->matnames;
                    minfo_matcolors = mat->matcolors;

                    bool invalidateVar = false;
                    if (mm->nmatnos > 0 && mm->nmatnos != mat->nmat)
                        invalidateVar = true;
                    if (invalidateVar)
                    {
                        debug1 << "Invalidating material \"" << multimat_names[i] 
                               << "\" since its first non-empty block ";
                        if(valid_var) {
                            debug1 << "(" << mb_matname << ") ";
                        }
                        debug1 << "has different # materials than its parent multimat." << endl;
                        valid_var = false;
                    }
                }
                else if (!is_all_empty)
                {
                    debug1 << "Invalidating material \"" << multimat_names[i] 
                           << "\" since its first non-empty block ";
                    if(valid_var) {
                        debug1 << "(" << mb_matname << ") ";
                    }
                    debug1 << "is invalid." << endl;
                    valid_var = false;
                }
            }
            else
            {
                //Get all the info from the multi-mat.
                minfo_nmats     = mm->nmatnos;
                minfo_matnos    = mm->matnos;
                minfo_matnames  = mm->material_names;
                minfo_matcolors = mm->matcolors;
            }

            //
            // Give the materials names based on their material number.  If
            // there are names in the Silo file, use those as well.
            //
            vector<string>  matnames;
            vector<string>  matcolors;
            string meshname;
            if (valid_var)
            {
                for (j = 0 ; j < minfo_nmats ; j++)
                {
                    char *num = NULL;
                    int dlen = int(log10(float(minfo_matnos[j]+1))) + 1;
                    if (minfo_matnames == NULL || minfo_matnames[j] == NULL)
                    {
                        num = new char[dlen + 2];
                        sprintf(num, "%d", minfo_matnos[j]);
                    }
                    else
                    {
                        int len = strlen(minfo_matnames[j]);
                        num = new char[len + 1 + dlen + 1];
                        sprintf(num, "%d %s", minfo_matnos[j], minfo_matnames[j]);
                    }
                    matnames.push_back(num);
                    delete[] num;

                    if (minfo_matcolors)
                    {
                        if (minfo_matcolors[j] && minfo_matcolors[j][0])
                            matcolors.push_back(minfo_matcolors[j]);
                        else
                            matcolors.push_back("");
                    }
                }

                if (mm->mmesh_name != 0)
                {
                    char cwd[512];
                    DBGetDir(dbfile, cwd);
                    meshname = FileFunctions::Absname(cwd,mm->mmesh_name,"/");
                    if (!DBInqVarExists(dbfile, meshname.c_str()))
                    {
                        debug5 << "Although 'mmesh_name' member indicates material \"" << multimat_names[i]
                               << "\" is defined on mesh \"" << meshname.c_str() << "\", "
                               << "the associated mesh does not exist." << endl;
                        meshname = DetermineMultiMeshForSubVariable(dbfile,
                                       multimat_names[i], mm_ent, dirname);
                        debug5 << "Guessing material \"" << multimat_names[i]
                               << "\" is defined on mesh \""
                               << meshname.c_str() << "\"" << endl;
                    }
                    else
                    {
                        debug5 << "Material \"" << multimat_names[i]
                               << "\" indicates it is defined on mesh \""
                               << meshname.c_str() << "\"" << endl;
                        // VisIt won't match this var to its mesh if leading slash is presnet
                        meshname.erase(meshname.begin());
                    }
                }
                else if (!is_all_empty)
                {
                    meshname = DetermineMultiMeshForSubVariable(dbfile,
                                                                multimat_names[i],
                                                                mm_ent,
                                                                dirname);
                    debug5 << "Guessing material \"" << multimat_names[i]
                           << "\" is defined on mesh \""
                           << meshname.c_str() << "\"" << endl;
                }
                else
                {
                    meshname = firstAllEmptyMultimesh;
                    minfo_nmats = 1;
                    matnames.push_back("unknown");
                }
            }

            avtMaterialMetaData *mmd;
            if (matcolors.size())
                mmd = new avtMaterialMetaData(name_w_dir, meshname,
                                              minfo_nmats, matnames,
                                              matcolors);
            else
                mmd = new avtMaterialMetaData(name_w_dir, meshname,
                                              minfo_nmats, matnames);

            mmd->validVariable = valid_var;
            mmd->hideFromGUI = mm->guihide;
            md->Add(mmd);

        }
        CATCHALL
        {
            // We explicitly free the multimat here and NOT at the end of the loop
            // becaue in 'normal' operation, it gets cached during the GetMultimat()
            // call and free'd later.
            if (mm != NULL)
            {
                if(mm_ent != NULL)
                {
                    // Make sure its removed from the plugin's cache too.
                    multimatCache.RemoveEntry(dirname,multimat_names[i]);
                }
                else
                {
                    // we alloced a dummy var, free it
                    DBFreeMultimat(mm);
                }
            }

            debug1 << "Giving up on multi-mat \"" << multimat_names[i] << "\"" << endl;
            vector<string> no_matnames;
            avtMaterialMetaData *mmd = new avtMaterialMetaData(name_w_dir, "unknown",
                                          0, no_matnames);
            mmd->validVariable = false;
            md->Add(mmd);
        }
        ENDTRY

        if (mat) DBFreeMaterial(mat);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Cyrus Harrison, Thu Mar 14 15:16:43 PDT 2013
//    Support species names from silo species objects.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadSpecies(DBfile *dbfile,
    int nmatspecies, char **matspecies_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j,k;
    for (i = 0 ; i < nmatspecies ; i++)
    {
        char *name_w_dir = 0;
        char *meshname_w_dir = 0;
        DBmatspecies *spec = 0;

        TRY
        {

            name_w_dir = GenerateName(dirname, matspecies_names[i], topDir.c_str());
            string realvar;
            DBfile *correctFile = dbfile;
            bool valid_var = true; (void) valid_var;
            DetermineFileAndDirectory(matspecies_names[i],"", correctFile, realvar);

            spec = DBGetMatspecies(correctFile, realvar.c_str());
            if (spec == NULL)
            {
                valid_var = false;
                spec = DBAllocMatspecies();
            }

            char meshname[256];
            GetMeshname(dbfile, spec->matname, meshname);

            vector<int>   numSpecies;
            vector<vector<string> > speciesNames;
            int spec_name_idx =0;
            ostringstream oss;
            for (j = 0 ; j < spec->nmat ; j++)
            {
                numSpecies.push_back(spec->nmatspec[j]);
                vector<string>  tmp_string_vector;
                //
                // Species do not currently have names, so just use their index.
                //
                for (k = 0 ; k < spec->nmatspec[j] ; k++)
                {
                    oss.str("");
                    oss << (k+1);
                    if(spec->specnames && spec->specnames[spec_name_idx])
                    {
                        //
                        //add spec name if it exists
                        //
                        oss << " ("
                            << string(spec->specnames[spec_name_idx])
                            << ")";
                    }
                    spec_name_idx++;
                    tmp_string_vector.push_back(oss.str());
                }
                speciesNames.push_back(tmp_string_vector);
            }
            meshname_w_dir = GenerateName(dirname, meshname, topDir.c_str());
            avtSpeciesMetaData *smd = new avtSpeciesMetaData(name_w_dir,
                                      meshname_w_dir, spec->matname, spec->nmat,
                                      numSpecies, speciesNames);
            //smd->hideFromGUI = spec->guihide;
            md->Add(smd);
        }
        CATCHALL
        {
            debug1 << "Giving up on species \"" << matspecies_names[i] << "\"" << endl;
            vector<vector<string> > no_matspecies_names;
            vector<int> no_num_species;
            avtSpeciesMetaData *smd = new avtSpeciesMetaData(name_w_dir,
                "unknown", "unknown", 0, no_num_species, no_matspecies_names);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (spec) DBFreeMatspecies(spec);
        if (name_w_dir) delete [] name_w_dir;
        if (meshname_w_dir) delete [] meshname_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names. Also added logic to
//    handle freeing of multimat species during exceptions.
//
//    Mark C. Miller, Mon Mar 29 17:27:43 PDT 2010
//    Reset Silo error level to DB_TOP as that is correct setting for 
//    newer versions of Silo library.
//
//    Mark C. Miller Tue Mar 30 16:28:48 PDT 2010
//    Temporarily reset to using DB_ALL error level as DB_TOP is causing hangs.
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Cyrus Harrison, Thu Mar 14 15:16:43 PDT 2013
//    Support species names from silo species objects. 
//
//    Mark C. Miller, Tue Feb  2 15:01:05 PST 2016
//    Add support for all-empty multi-matspecies.
//
//    Mark C. Miller, 04Apr17
//    Adjust logic for building species names to allow nulls in Silo's names.
// ****************************************************************************
void
avtSiloFileFormat::ReadMultispecies(DBfile *dbfile,
    int nmultimatspecies, char **multimatspecies_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    int i,j,k;
    for (i = 0 ; i < nmultimatspecies ; i++)
    {
        char *name_w_dir = 0;
        DBmultimatspecies *ms = 0;
        DBmatspecies *spec = 0;
        avtSiloMultiSpecCacheEntry *ms_ent;
        TRY
        {
            bool valid_var = true;
            bool is_all_empty = false;
            GetMultiSpec(dirname, multimatspecies_names[i], &ms_ent, &valid_var);

            if(ms_ent != NULL )
                ms = ms_ent->DataObject();

            if (ms == NULL)
            {
                valid_var = false;
                ms = DBAllocMultimatspecies(0);
            }

            char *name_w_dir = GenerateName(dirname, multimatspecies_names[i], topDir.c_str());

            RegisterDomainDirs(ms_ent,dirname);

            string mb_specname;
            int meshnum = FindFirstNonEmptyBlock(multimatspecies_names[i], ms->nspec,
                    ms_ent, ms->repr_block_idx, ms->empty_cnt, ms->empty_list);
            if (meshnum < 0)
            {
                is_all_empty = true;
                emptyObjectsList[name_w_dir] = true;
            }
            else
                mb_specname = ms_ent->GenerateName(meshnum);

            string meshname;

            if (valid_var)
            {
                // get the associated multimat

                // We can only get this "matname" using GetComponent.  It it not
                // yet a part of the DBmultimatspec structure, so this is the only
                // way we can get this info.
                char *multimatName = (char *) GetComponent(dbfile,
                                                  multimatspecies_names[i], "matname");

                // get the multimesh for the multimat
                DBmultimat *mm = NULL;
                avtSiloMultiMatCacheEntry *mm_ent = NULL;
                GetMultiMat(dirname, multimatName, &mm_ent, &valid_var);

                if(mm_ent != NULL)
                    mm = mm_ent->DataObject();

                if (mm == NULL)
                {
                    valid_var = false;
                }
                else if (!is_all_empty)
                {
                    meshname = DetermineMultiMeshForSubVariable(dbfile,
                                                              multimatspecies_names[i],
                                                              mm_ent,
                                                              dirname);
                    //
                    // get the species info

                    string realvar;
                    DBfile *correctFile = dbfile;
                    DetermineFileAndDirectory(mb_specname.c_str(), "", correctFile, realvar);

                    DBShowErrors(DB_NONE, NULL);
                    spec = DBGetMatspecies(correctFile, realvar.c_str());
                    DBShowErrors(DB_ALL, ExceptionGenerator);
                    if (spec == NULL)
                    {
                        debug1 << "Giving up on species \"" << multimatspecies_names[i]
                               << "\" since its first non-empty block (" << mb_specname
                               << ") is invalid." << endl;
                        valid_var = false;
                    }
                }
                else
                {
                    meshname = firstAllEmptyMultimesh;
                }
            }

            vector<int>              numSpecies;
            vector< vector<string> > speciesNames;
            if (valid_var)
            {
                ostringstream oss;
                int spec_name_idx = 0;
                for (j = 0 ; j < spec->nmat ; j++)
                {
                    numSpecies.push_back(spec->nmatspec[j]);
                    vector<string>  tmp_string_vector;
                    for (k = 0 ; k < spec->nmatspec[j] ; k++)
                    {
                        oss.str("");
                        oss << (k+1);
                        if(spec->specnames && spec->specnames[spec_name_idx])
                        {
                            //
                            //add spec name if it exists
                            //
                            oss << " ("
                                << string(spec->specnames[spec_name_idx])
                                << ")";
                        }
                        spec_name_idx++;
                        tmp_string_vector.push_back(oss.str());
                    }
                    speciesNames.push_back(tmp_string_vector);
                }
            }
            avtSpeciesMetaData *smd;
            if (valid_var)
                smd = new avtSpeciesMetaData(name_w_dir,
                                             meshname, spec->matname, spec->nmat,
                                             numSpecies, speciesNames);
            else
                smd = new avtSpeciesMetaData(name_w_dir, "", "", 0,
                                             numSpecies, speciesNames);
            //smd->hideFromGUI = ms->guihide;
            smd->validVariable = valid_var;
            md->Add(smd);

        }
        CATCHALL
        {
            // We explicitly free the multimat species object here and NOT and
            // the end of the loop because in 'normal' operation, the object
            // is cached in the plugin and free'd later.
            if (ms)
            {
                // Make sure it's removed from the plugin's cache, too.
                multispecCache.RemoveEntry(dirname,multimatspecies_names[i]);
            }

            debug1 << "Giving up on species \"" << multimatspecies_names[i] << "\"" << endl;
            vector<vector<string> > no_matspecies_names;
            vector<int> no_num_species;
            avtSpeciesMetaData *smd = new avtSpeciesMetaData(name_w_dir,
                "unknown", "unknown", 0, no_num_species, no_matspecies_names);
            smd->validVariable = false;
            md->Add(smd);
        }
        ENDTRY

        if (spec) DBFreeMatspecies(spec);
        if (name_w_dir) delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Programmer: Mark C. Miller (re-factored here from ReadDir()
//  Created: Wed Jun 17 10:42:42 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:56:08 PDT 2009
//    Replaced DBtoc* arg. with list of object names.
//
//    Mark C. Miller, Thu Jan  6 17:17:47 PST 2011
//    Handle gui hide flag for expressions. There is a bug causing segv in
//    PDB driver for version of silo prior to 4.8.
//
//    Mark C. Miller, Thu Jan  6 19:33:41 PST 2011
//    Simplifed logic for setting gui hide flag for expressions.
// ****************************************************************************
void
avtSiloFileFormat::ReadDefvars(DBfile *dbfile,
    int ndefvars, char **defvars_names,
    const char *dirname, avtDatabaseMetaData *md)
{
    for (int i = 0; i < ndefvars; i++)
    {
        DBdefvars *defv = 0;

        TRY
        {
            defv = DBGetDefvars(dbfile, defvars_names[i]);
            if (defv == NULL)
            {
                char msg[256];
                SNPRINTF(msg,sizeof(msg),"Unable to read defvars object \"%s\"",
                    defvars_names[i]);
                EXCEPTION1(SiloException,msg);
            }

            for (int j = 0; j < defv->ndefs; j++)
            {
                Expression::ExprType vartype = Expression::Unknown;
                switch (defv->types[j])
                {
                    case DB_VARTYPE_SCALAR:   vartype = Expression::ScalarMeshVar; break;
                    case DB_VARTYPE_VECTOR:   vartype = Expression::VectorMeshVar; break;
                    case DB_VARTYPE_TENSOR:   vartype = Expression::TensorMeshVar; break;
#ifdef DB_VARTYPE_ARRAY
                    case DB_VARTYPE_ARRAY:    vartype = Expression::ArrayMeshVar; break;
                    case DB_VARTYPE_MATERIAL: vartype = Expression::Material; break;
                    case DB_VARTYPE_SPECIES:  vartype = Expression::Species ; break;
#endif
                    default:        vartype = Expression::Unknown; break;
                }

                if (vartype == Expression::Unknown)
                {
                    debug5 << "Warning: unknown defvar type for derived "
                           << "variable \"" << defv->names[j] << "\"" << endl;
                }
                else
                {

                    Expression expr;
                        expr.SetName(defv->names[j]);
                        expr.SetDefinition(defv->defns[j]);
                        expr.SetType(vartype);
                        if (defv->guihides)
                            expr.SetHidden(defv->guihides[j]);
                    md->AddExpression(&expr);
                }
            }
        }
        CATCHALL
        {
            debug1 << "Giving up on defvars object \"" << defvars_names[i] << "\"" << endl;
        }
        ENDTRY

        if (defv) DBFreeDefvars(defv);

    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::ReadDir
//
//  Purpose:
//      Get all of the variables from a directory.
//
//  Arguments:
//      dbfile   The handle to the Silo file.
//      dirname  The name of the current directory.
//      md       The meta-data object to put information into.
//
//  Programmer:  Mark C. Miller (Re-factored from orig. of Hank Childs)
//  Creation:    June 18, 2009 
//
//  Modifications:
//    Note: If you are looking for modification comments prior to re-factor,
//    that whole block of comments was moved above all the individual
//    ReadXXX() methods which precede this method.
//
//    Mark C. Miller, Wed Aug 19 11:21:37 PDT 2009
//    Reformatted with TOC_ENTRY macros to reduce size.
//
//    Eric Brugger, Wed Sep 30 13:47:14 PDT 2015
//    I corrected a bug that caused "_meshtv_searchpath" to be ignored.
//    I also added "_visit_searchpath" as a synonym for "_meshtv_searchpath"
//    since "_visit_searchpath" is much more appropriate for VisIt.
//
//    Mark C. Miller, Thu Feb 11 17:40:32 PST 2016
//    Adjusted logic for going into directories to only call ReadDir() when
//    preceding call to DBSetDir succeeds.
// ****************************************************************************
#define COPY_TOC_ENTRY(NM)                        \
    int      n ## NM = toc->n ## NM;                    \
    char   **NM ## _names = new char*[n ## NM];                \
    for (i = 0 ; i < n ## NM; i++)                    \
    {                                    \
        NM ## _names[i] = new char[strlen(toc->NM ## _names[i])+1];    \
        strcpy(NM ## _names[i], toc->NM ## _names[i]);            \
    }
#define FREE_COPIED_TOC_ENTRY(NM)                    \
    for (i = 0 ; i < n ## NM; i++)                    \
    {                                    \
        delete [] NM ## _names[i];                    \
    }                                    \
    delete [] NM ## _names;


void
avtSiloFileFormat::ReadDir(DBfile *dbfile, const char *dirname,
                           avtDatabaseMetaData *md)
{
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return;
#endif

    int    i;
    DBtoc *toc = DBGetToc(dbfile);
    if (toc == NULL)
        return;

    //
    // Copy relevant info from the toc. Otherwise, it'll get lost on
    // successive calls to DBSetDir().
    //
    int      norigdir      = toc->ndir;
    char   **origdir_names = new char*[norigdir];
    for (i = 0 ; i < norigdir ; i++)
    {
        origdir_names[i] = new char[strlen(toc->dir_names[i])+1];
        strcpy(origdir_names[i], toc->dir_names[i]);
    }
    COPY_TOC_ENTRY(dir);
    COPY_TOC_ENTRY(multimesh);
    COPY_TOC_ENTRY(qmesh);
    COPY_TOC_ENTRY(ucdmesh);
    COPY_TOC_ENTRY(ptmesh);
    COPY_TOC_ENTRY(multivar);
    COPY_TOC_ENTRY(qvar);
    COPY_TOC_ENTRY(ucdvar);
    COPY_TOC_ENTRY(ptvar);
    COPY_TOC_ENTRY(mat);
    COPY_TOC_ENTRY(multimat);
    COPY_TOC_ENTRY(matspecies);
    COPY_TOC_ENTRY(multimatspecies);
    COPY_TOC_ENTRY(curve);
    COPY_TOC_ENTRY(csgvar);
    COPY_TOC_ENTRY(csgmesh);
    COPY_TOC_ENTRY(defvars);

    // Miscellany in the root directory
    char *searchpath_str = NULL;
    ReadTopDirStuff(dbfile, dirname, md, &searchpath_str);

    // Meshes
    ReadMultimeshes(dbfile, nmultimesh, multimesh_names, dirname, md);
    ReadQuadmeshes(dbfile, nqmesh, qmesh_names, dirname, md);
    ReadUcdmeshes(dbfile, nucdmesh, ucdmesh_names, dirname, md);
    ReadPointmeshes(dbfile, nptmesh, ptmesh_names, dirname, md);
    ReadCSGmeshes(dbfile, ncsgmesh, csgmesh_names, dirname, md);
    ReadCurves(dbfile, ncurve, curve_names, dirname, md);

    // Mats and Species
    ReadMaterials(dbfile, nmat, mat_names, dirname, md);
    ReadMultimats(dbfile, nmultimat, multimat_names, dirname, md);
    ReadSpecies(dbfile, nmatspecies, matspecies_names, dirname, md);
    ReadMultispecies(dbfile, nmultimatspecies, multimatspecies_names, dirname, md);

    // Vars
    ReadMultivars(dbfile, nmultivar, multivar_names, dirname, md);
    ReadQuadvars(dbfile, nqvar, qvar_names, dirname, md);
    ReadUcdvars(dbfile, nucdvar, ucdvar_names, dirname, md);
    ReadPointvars(dbfile, nptvar, ptvar_names, dirname, md);
    ReadCSGvars(dbfile, ncsgvar, csgvar_names, dirname, md);

    // Defined variables
    ReadDefvars(dbfile, ndefvars, defvars_names, dirname, md);

    //
    // If the meshtv searchpath is defined then replace the list of
    // directories with the directories in the search path.
    //
    if (searchpath_str != NULL)
    {
        //
        // Free up the old list of directories.
        //
        for (i = 0 ; i < ndir ; i++)
        {
            delete [] dir_names[i];
        }
        delete [] dir_names;

        //
        // Determine the maximum number of directories in the string.
        //
        int max_ndir = 1;
        for (i = 0; searchpath_str[i] != '\0'; i++)
        {
            if (searchpath_str[i] == ';')
            {
                max_ndir++;
            }
        }

        //
        // Create the new list of directories.
        //
        debug1 << "Set list of directories to search from searchpath_str..." << endl;
        dir_names = new char*[max_ndir];
        ndir = 0;
        int searchpath_strlen = strlen(searchpath_str);
        for (i = 0; i < searchpath_strlen; i++)
        {
            char *dirname = &searchpath_str[i];

            while (searchpath_str[i] != ';' && searchpath_str[i] != '\0')
            {
                i++;
            }
            searchpath_str[i] = '\0';

            if (strlen(dirname) > 0)
            {
                dir_names[ndir] = new char[strlen(dirname)+1];
                strcpy(dir_names[ndir], dirname);
                debug1 << "... \"" << dirname << "\"" << endl;
                ndir++;
            }
        }
          
        delete [] searchpath_str;
        searchpath_str = NULL;
    }

    //
    // Call recursively on the sub-directories.
    //
    for (i = 0 ; i < ndir ; i++)
    {
        char path[1024];

        TRY
        {

            int length = strlen(dirname);
            if (length > 0 && dirname[length-1] != '/')
            {
                sprintf(path, "%s/%s", dirname, dir_names[i]);
            }
            else
            {
                sprintf(path, "%s%s", dirname, dir_names[i]);
            }
            if (ShouldGoToDir(path))
            {
                int dir_status = DBSetDir(dbfile, dir_names[i]);
                if (dir_status == 0) 
                {
                    ReadDir(dbfile, path, md);
                    DBSetDir(dbfile, "..");
                }
                else
                {
                    debug1 << "Unable to DBSetDir(\"" << path << "\")" << endl;
                }
            }

        }
        CATCHALL
        {
            debug1 << "Giving up on directory \"" << path << "\"" << endl;
        }
        ENDTRY
    }

    //
    // Ok, cleanup the info we copied from the toc.
    //
    for (i = 0 ; i < norigdir ; i++)
    {
        delete [] origdir_names[i];
    }
    delete [] origdir_names;
    FREE_COPIED_TOC_ENTRY(multimesh);
    FREE_COPIED_TOC_ENTRY(qmesh);
    FREE_COPIED_TOC_ENTRY(ucdmesh);
    FREE_COPIED_TOC_ENTRY(ptmesh);
    FREE_COPIED_TOC_ENTRY(multivar);
    FREE_COPIED_TOC_ENTRY(qvar);
    FREE_COPIED_TOC_ENTRY(ucdvar);
    FREE_COPIED_TOC_ENTRY(ptvar);
    FREE_COPIED_TOC_ENTRY(mat);
    FREE_COPIED_TOC_ENTRY(multimat);
    FREE_COPIED_TOC_ENTRY(matspecies);
    FREE_COPIED_TOC_ENTRY(multimatspecies);
    FREE_COPIED_TOC_ENTRY(curve);
    FREE_COPIED_TOC_ENTRY(dir);
    FREE_COPIED_TOC_ENTRY(csgmesh);
    FREE_COPIED_TOC_ENTRY(csgvar);
    FREE_COPIED_TOC_ENTRY(defvars);
}

// ****************************************************************************
//  Method:  avtSiloFileFormat::BroadcastGlobalInfo
//
//  Purpose:
//    Send information collected only by the first processor to the other ones.
//    This includes the avtDatabaseMetaData as well as some class data members.
//
//  Arguments:
//    metadata   the avtDatabaseMetadata to broadcast.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 15 08:01:51 PDT 2003
//    Remove broadcasting of defvars.  This is now handled through a different
//    mechanism at a higher level.
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
//
//    Brad Whitlock, Fri Mar 16 11:53:10 PDT 2007
//    Make sure that SelectAll is called on the metadata so the atts and 
//    attVectors that it contains will figure into the message size.
//
//    Mark C. Miller, Tue Apr 15 10:24:59 PDT 2008
//    Added missing call to broadcast the code name guess
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added logic to pass along ignore extents bools.
//
//    Cyrus Harrison, Thu Sep 23 11:02:22 PDT 2010
//    Added missing broadcast for 'haveAMRGroupInfo'.
//
//    Mark C. Miller, Thu Apr 12 23:07:36 PDT 2012
//    Removed calls to build NChooseRMaps supporting nodelists.
//
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Added logic to support adding of block decomposition as a variable.
// ****************************************************************************
void
avtSiloFileFormat::BroadcastGlobalInfo(avtDatabaseMetaData *metadata)
{
#ifdef PARALLEL
    int rank = PAR_Rank();

    //
    // Broadcast Full DatabaseMetaData
    //
    BufferConnection tmp;
    int n;
    if (rank==0)
    {
        metadata->SelectAll();
        n = metadata->CalculateMessageSize(tmp);
    }
    BroadcastInt(n);
    unsigned char *buff = new unsigned char[n];

    if (rank == 0)
    {
        metadata->SelectAll();
        metadata->Write(tmp);
        for (int i=0; i<n; i++)
        {
            tmp.Read(&buff[i]);
        }
    }
    MPI_Bcast(buff, n, MPI_UNSIGNED_CHAR, 0, VISIT_MPI_COMM);
    if (rank != 0)
    {
        tmp.Append(buff, n);
        metadata->Read(tmp);
    }
    delete[] buff;

    //
    // Broadcast Database-level Info
    //
    BroadcastStringVector(firstSubMesh, rank);
    BroadcastStringVector(firstSubMeshVarName, rank);
    BroadcastStringVectorVector(allSubMeshDirs, rank);
    BroadcastStringVector(actualMeshName, rank);
    BroadcastIntVector(blocksForMesh, rank);
    BroadcastString(codeNameGuess, rank);

    //
    // Broadcast info about nodelists
    //
    BroadcastInt(numNodeLists);
    if (numNodeLists > 0)
    {
        int nlMapSize = nlBlockToWindowsMap.size();
        BroadcastInt(nlMapSize);
        if (rank == 0)
        {
            map<int, vector<int> >::iterator nlit;
            for (nlit = nlBlockToWindowsMap.begin(); nlit != nlBlockToWindowsMap.end(); nlit++)
            {
                int nlMapKey = nlit->first;
                BroadcastInt(nlMapKey);
                BroadcastIntVector(nlit->second, rank);
            }
        }
        else
        {
            for (int k = 0; k < nlMapSize; k++)
            {
                int nlMapKey;
                BroadcastInt(nlMapKey);
                BroadcastIntVector(nlBlockToWindowsMap[nlMapKey], rank);
            }
        }
    }
    BroadcastInt(numAnnotIntLists);

    //
    // Broadcast Group Info
    //
    int haveGroups = groupInfo.haveGroups;
    BroadcastInt(haveGroups);
    groupInfo.haveGroups = haveGroups;
    BroadcastInt(groupInfo.ndomains);
    BroadcastInt(groupInfo.numgroups);
    BroadcastIntVector(groupInfo.ids,  rank);

    //
    // Broadcast domainToBlockGrouping info
    //
    int dtbg_size = addBlockDecompositionAsVar ? (int) domainToBlockGrouping.size() : 0;
    BroadcastInt(dtbg_size);
    if (dtbg_size)
        BroadcastIntVector(domainToBlockGrouping, rank);

    int ignore_extents = ignoreSpatialExtents;
    BroadcastInt(ignore_extents);
    ignoreSpatialExtents = ignore_extents;
    ignore_extents = ignoreDataExtents;
    BroadcastInt(ignore_extents);
    ignoreDataExtents = ignore_extents;
    int have_amr_group_info =haveAmrGroupInfo;
    BroadcastInt(have_amr_group_info);
    haveAmrGroupInfo = have_amr_group_info;

#endif
}

// ****************************************************************************
//  Method:  avtSiloFileFormat::StoreMultimeshInfo
//
//  Purpose: Update plugin's cache of multimesh variables and names used in
//           facilitating matching variable to mesh 
//
//  Programmer:  Mark C. Miller (moved from ReadDir)
//  Creation:    June 26, 2006 
//
//  Modifications:
//    Mark C. Miller, Thu Jun 18 20:59:24 PDT 2009
//    Removed which_mm argument. It was not necessary and caused confusion.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Eric Brugger, Thu Mar 15 17:02:43 PDT 2018
//    Modify the method to not only add a blank name to firstSubMeshVarName
//    when the meshnum was greater than the number of blocks, but also when
//    it was negative. This ensures that allSubMeshVarName and actualMeshName
//    are the same length, which is assumed elsewhere in the plugin. The
//    meshnum can be negative when the multimesh is invalid, such as when
//    the number of blocks is specified as zero.
//
// ****************************************************************************
void
avtSiloFileFormat::StoreMultimeshInfo(const char *const dirname,
                                      const char *const name_w_dir,
                                      int meshnum,
                                      avtSiloMultiMeshCacheEntry *obj)
{
    int nblocks = obj->NumberOfBlocks();
    actualMeshName.push_back(name_w_dir);
    firstSubMesh.push_back((meshnum < nblocks) ?
                            obj->GenerateName(meshnum) : "");
    blocksForMesh.push_back(nblocks);
    allSubMeshDirs.push_back(vector<string>());
    for (int j=0; j< nblocks; j++)
    {
        string dir,var;
        SplitDirVarName(obj->GenerateName(j).c_str(), dirname, dir,var);
        if (j==meshnum)
            firstSubMeshVarName.push_back(var);
        allSubMeshDirs[allSubMeshDirs.size()-1].push_back(dir);
    }
    if (meshnum < 0 || meshnum >= nblocks)
        firstSubMeshVarName.push_back("");
}


// ****************************************************************************
//  Method:  avtSiloFileFormat::DoRootDirectoryWork
//
//  Purpose:
//    Does some collection of information that came from the root of the
//    silo file.
//
//  Note:  this used to be part of PopulateDatabaseMetaData
//
//  Arguments:
//    md         the avtDatabaseMetaData
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 15, 2003
//
//  Modifications:
//
//    Hank Childs, Thu Aug 14 09:20:28 PDT 2003
//    Removed explicit support for vector defvars -- it is all now handled
//    correctly at a higher level.
//
//    Hank Childs, Mon Oct  8 13:11:06 PDT 2007
//    Code around the case where 'treatAllDBsAsTimeVarying' is turned on
//    and we need to produce a good SIL, but haven't gotten the group info.
//
//    Cyrus Harrison, Wed Mar 24 10:45:30 PDT 2010
//    Don't set group information if we have already set it for an AMR
//    dataset.
//
//    Mark C. Miller, Tue Feb 25 15:10:29 PST 2014
//    Do the work that md->AddGroupInformation() would have done but make
//    sure we do it *only* for the relevant meshes.
//
//    Mark C. Miller, Tue Jun  3 13:07:01 PDT 2014
//    Adjust algorithm to find matching meshes to attach group info and ids
//    to make it a little clearer what it is doing/how it is working.
// ****************************************************************************

void
avtSiloFileFormat::DoRootDirectoryWork(avtDatabaseMetaData *md)
{
    //
    // We should only add the defvars if we are at the 'root' level.
    // Ditto adding group information.
    //

    void_ref_ptr vr = cache->GetVoidRef("any_mesh",
                            AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1);
    if (*vr != NULL && !groupInfo.haveGroups)
    {
        // The only way to get into this state is if we have selected 
        // "treatAllDBsAsTimeVarying".  If that's the case, the domain boundary
        // info is being re-used (incorrectly) and we decide not to use
        // the group info.
        //
        // Correct that now.
        DBfile *dbfile = OpenFile(tocIndex);
        debug1 << "Forcing re-read of connectivity information." << endl;
        debug1 << "This should only occur when treatAllDBsAsTimeVarying "
               << "is on." << endl;
        GetConnectivityAndGroupInformation(dbfile, true);
    }

    if (!haveAmrGroupInfo && groupInfo.haveGroups)
    {
        for (int i = 0; i < md->GetNumMeshes(); i++)
        {
            int namesMatchAtIndex = -1;
            for (int j = 0; j < (int) actualMeshName.size(); j++)
            {
                if (md->GetMeshes(i).name == actualMeshName[j])
                {
                    namesMatchAtIndex = j;
                    break;
                }
            }

            if (namesMatchAtIndex != -1 &&
                groupInfo.ndomains == blocksForMesh[namesMatchAtIndex] && 
                groupInfo.ndomains == md->GetMeshes(i).numBlocks)
            {
                md->GetMeshes(i).numGroups = groupInfo.numgroups;
                md->GetMeshes(i).groupIds = groupInfo.ids;
            }
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::FindDecomposedMeshType
//
//  Purpose:
//      Finds and returns the type of the first multimesh at the root of this
//      Silo file. Returns AVT_UNKNOWN_MESH if no multimesh is found.
//
//  Programmer: Cyrus Harrison
//  Creation:   Monday October 27, 2008
//
//  Modifications:
//
// ****************************************************************************

avtMeshType
avtSiloFileFormat::FindDecomposedMeshType(DBfile *dbfile)
{
    char current_dir[512];
    DBGetDir(dbfile,current_dir);
    DBSetDir(dbfile,"/");
    avtMeshType res = AVT_UNKNOWN_MESH;
    // read toc
    DBtoc *toc = DBGetToc(dbfile);
    if (toc == NULL)
        EXCEPTION1(InvalidFilesException,
                   "Could not obtain Silo Table of Contents when looking for "
                   " decomposed mesh type.");
    
    stringVector mmeshes;
    int nmmesh = toc->nmultimesh;    
    for (int i = 0 ; i < nmmesh ; i++)
        mmeshes.push_back(toc->multimesh_names[i]);
        
    // loop over meshes and look for first non empty
    for (int i = 0 ; i < nmmesh && res == AVT_UNKNOWN_MESH; i++)
    {
        DBmultimesh *mm = DBGetMultimesh(dbfile, mmeshes[i].c_str());
        if (mm)
        {
            for(int j = 0; j < mm->nblocks && res == AVT_UNKNOWN_MESH; j++)
            {
                int silo_type = DB_NONE;
                if(mm->meshtypes != NULL)
                    silo_type = mm->meshtypes[j];
                else
                    silo_type = mm->block_type;
                if(silo_type== DB_QUAD_RECT)
                    res = AVT_RECTILINEAR_MESH;
                else if(silo_type == DB_QUAD_CURV || silo_type == DB_QUADMESH)
                    res = AVT_CURVILINEAR_MESH;
                else if(silo_type == DB_POINTMESH)
                    res = AVT_POINT_MESH;
                else if(silo_type == DB_UCDMESH)
                    res = AVT_UNSTRUCTURED_MESH;
            }
            // free multimesh
            DBFreeMultimesh(mm);
        }
    }
    
    debug4 << "avtSiloFileFormat::FindDecomposedMeshType result = " << res << endl;
    // toc points to internal structure, so we do not need to free it.
    // set current dir back to where we were before this method
    DBSetDir(dbfile,current_dir);
    return res;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetConnectivityAndGroupInformation
//
//  Purpose:
//      Traverses directory structures to find information about how blocks
//      connect up and how what groups each block lies in.
//
//  Notes:      The original code comes from PopulateDatabaseMetaData and was
//              written by Jeremy Meredith.
//
//  Programmer: Hank Childs
//  Creation:   June 24, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Apr 10 08:51:23 PDT 2003
//    Removed meta-data argument, since the acquired data will now be stored
//    off and used later when the meta-data is asked for.
//
//    Hank Childs, Wed Nov 12 16:45:19 PST 2003
//    Use curvilinear domain boundary as structured domain boundary is now
//    an abstract type.
//
//    Jeremy Meredith and Hank Childs, Thu Nov 20 15:28:24 PST 2003
//    Do not try to re-read domain connectivity if it has already been
//    cached.  This provides a nice speedup when changing time steps.  It also
//    avoids a bug where changing time steps could cause problems because 
//    the processors with no data don't get back to this function anyway.
//
//    Mark C. Miller, Wed Nov 29 14:56:26 PST 2006
//    Cached connectivity and group information for given timestep or -1
//    depending on database setting for "ConnectivityIsTimeVarying" var
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// 
//    Hank Childs, Mon Oct  8 13:01:31 PDT 2007
//    Added an argument to force the operation.
//
//    Hank Childs, Tue Feb 19 19:45:43 PST 2008
//    Rename "dynamic" to "streaming", since we really care about whether we
//    are streaming, not about whether we are doing dynamic load balancing.
//    And the two are no longer synonymous.
//
//    Added call to FindDecomposedMeshType() to help with creating the 
//    correct type of domain boundries object.
//
//    Hank Childs, Wed Dec 22 15:14:33 PST 2010
//    Early return when we are streaming.
//
//    Mark C. Miller, Tue Jun  3 13:09:26 PDT 2014
//    Adjust conditional logic to determine if domain boundary information is
//    served up from 'ndomains > 0' to 'ndomains > 1' to deal with Silo files
//    with one domain and one group.
// ****************************************************************************

void
avtSiloFileFormat::GetConnectivityAndGroupInformation(DBfile *dbfile, 
                                                      bool force)
{
    //
    // This routine is not implemented for streaming.  We declared earlier that
    // the Silo format can never do streaming.  And yet here we are doing
    // streaming.  This means we are likely pulling out a single chunk of 
    // data.  If that's the case, we don't need the conn and group info.  
    // So just return.
    //
    if (doingStreaming)
        return;

    int ts = (connectivityIsTimeVarying || force) ? timestep : -1;

    void_ref_ptr vr = cache->GetVoidRef("any_mesh",
                            AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, ts, -1);
    if (*vr != NULL && !force)
    {
        // We've already got it from a previous time step;
        // don't re-read it for later time steps.
        return;
    }

    //
    // Connectivity information.
    //
    int  ndomains    = -1;
    int *extents     = NULL;
    int *nneighbors  = NULL;
    int  lneighbors  = -1;
    int *neighbors   = NULL;

    //
    // Group information.
    //
    int  numGroups   = -1;
    int *groupIds    = NULL;

#ifdef PARALLEL
    int rank = 0;
    MPI_Comm_rank(VISIT_MPI_COMM, &rank);
    if (rank == 0)
    {
#endif
       GetConnectivityAndGroupInformationFromFile(dbfile, ndomains, nneighbors,
                          extents, lneighbors, neighbors, numGroups, groupIds);

#ifdef PARALLEL
    }

    int useldbi = useLocalDomainBoundries;
    MPI_Bcast(&useldbi, 1, MPI_INT, 0, VISIT_MPI_COMM);
    useLocalDomainBoundries = useldbi;
    //
    // Communicate processor 0's information to the rest of the processors.
    //
    MPI_Bcast(&ndomains, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (ndomains != -1)
    {
        // avoid broadcast if we are using local dbi
        if(!useLocalDomainBoundries)
        {
            if (rank != 0)
            {
                extents = new int[ndomains*6];
                nneighbors = new int[ndomains];
            }
            MPI_Bcast(extents,     ndomains*6, MPI_INT, 0, VISIT_MPI_COMM);
            MPI_Bcast(nneighbors,  ndomains,   MPI_INT, 0, VISIT_MPI_COMM);
            MPI_Bcast(&lneighbors, 1,          MPI_INT, 0, VISIT_MPI_COMM);
            if (rank != 0)
            {
                neighbors = new int[lneighbors];
            }
            MPI_Bcast(neighbors,   lneighbors, MPI_INT, 0, VISIT_MPI_COMM);
        }
    }

    MPI_Bcast(&numGroups, 1, MPI_INT, 0, VISIT_MPI_COMM);
    if (numGroups > 0)
    {
        if (rank != 0)
        {
            groupIds = new int[ndomains];
        }
        MPI_Bcast(groupIds, ndomains, MPI_INT, 0, VISIT_MPI_COMM);
    }
#endif

    //
    // If we found connectivity information, go ahead and create the 
    // appropriate data structure and register it.
    //
    if (!useLocalDomainBoundries &&
        ndomains > 1 &&
        !avtDatabase::OnlyServeUpMetaData())
    {
        avtStructuredDomainBoundaries *dbi = NULL;
        avtMeshType mesh_type = FindDecomposedMeshType(dbfile);
        if(mesh_type == AVT_RECTILINEAR_MESH)
            dbi =new avtRectilinearDomainBoundaries();
        else if(mesh_type == AVT_CURVILINEAR_MESH)
            dbi =new avtCurvilinearDomainBoundaries();
        else
        {
            // Error: Unknown connectivity type!
            EXCEPTION1(InvalidVariableException,
                  "Could not determine mesh type for Connectivity "
                  "and Group information.");
        }

        dbi->SetNumDomains(ndomains);

        int l = 0;
        for (int j = 0; j < ndomains; j++)
        {
            dbi->SetExtents(j, &extents[j*6]);
            for (int k = 0; k < nneighbors[j]; k++)
            {
                dbi->AddNeighbor(j, neighbors[l+0], neighbors[l+1],
                                 &neighbors[l+2], &neighbors[l+5]);
                l += 11;
            }
            dbi->Finish(j);
        }

        void_ref_ptr vr = void_ref_ptr(dbi,
                                      avtStructuredDomainBoundaries::Destruct);
        cache->CacheVoidRef("any_mesh",
                       AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, ts, -1, vr);

        //
        // Hard to characterize when we would or would not be
        // able to do dynamic load balancing, so just turn it off.
        //
        canDoStreaming = false;
    }

    if (numGroups > 0)
    {
        vector<int> ids;
        for (int i = 0 ; i < ndomains ; i++)
        {
            ids.push_back(groupIds[i]);
        }
        groupInfo.haveGroups = true;
        groupInfo.ndomains = ndomains;
        groupInfo.numgroups = numGroups;
        groupInfo.ids = ids;
    }

    //
    // Clean up memory.
    //
    if (neighbors != NULL)
    {
        delete [] neighbors;
    }
    if (nneighbors != NULL)
    {
        delete [] nneighbors;
    }
    if (extents != NULL)
    {
        delete [] extents;
    }
    if (groupIds != NULL)
    {
        delete [] groupIds;
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetConnectivityAndGroupInformationFromFile
//
//  Purpose:
//      Traverse the file and gets the connectivity information as well as the
//      group information.  This is done together because these pieces of
//      information are located in the same subdirectory.
//
//  Notes:      The original code comes from PopulateDatabaseMetaData and was
//              written by Jeremy Meredith.
//
//  Programmer: Hank Childs
//  Creation:   June 24, 2002
//
//  Modifications:
//
//    Hank Childs, Sat Aug 10 09:10:08 PDT 2002
//    Fix bug where codes that did not fully meet the interface for groups
//    ended up with a partially completed structure that eventually caused a
//    crash.
//
//    Mark C. Miller, Tue Nov 22 13:28:31 PST 2005
//    Added alternative group information
//
//    Cyrus Harrison, Fri Sep  7 10:58:11 PDT 2007
//    Added option for connectivity from the MultiMeshadj object.
//
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Read the Domains_BlockNums data to support adding block decompposition
//    as a variable.
// ****************************************************************************

void
avtSiloFileFormat::GetConnectivityAndGroupInformationFromFile(DBfile *dbfile,
               int &ndomains, int *&nneighbors, int *&extents, int &lneighbors, 
               int *&neighbors, int &numGroups, int *&groupIds)
{
    bool needGroupInfo = true;
    bool needConnectivityInfo = !avtDatabase::OnlyServeUpMetaData();
   
    //
    // We can read in the connectivity info through this variable.  We cannot
    // assume its existence, though.
    //
    if (DBInqVarExists(dbfile, "ConnectivityNumDomains") != 0)
    {
        DBReadVar(dbfile, "ConnectivityNumDomains", &ndomains);

        extents = new int[ndomains*6];
        nneighbors = new int[ndomains];

        DBReadVar(dbfile, "ConnectivityExtents", extents);
        DBReadVar(dbfile, "ConnectivityNumNeighbors", nneighbors);
        DBReadVar(dbfile, "ConnectivityLenNeighbors", &lneighbors);

        neighbors = new int[lneighbors];

        DBReadVar(dbfile, "ConnectivityNeighbors", neighbors);
        needConnectivityInfo = false;
    }

    //
    // Note that this 'if' test will always be true right now -- we will always
    // need the group info.
    //
    if (needGroupInfo || needConnectivityInfo)
    {
        //
        // We need to start looking through the subdirectories.
        //
        bool found = false;
        bool hasGlobal = false;
        if (DBInqVarType(dbfile, "Global") == DB_DIR)
        {
            hasGlobal = true;
        }
        bool hasDecomposition = false;
        if (DBInqVarType(dbfile, "Decomposition") == DB_DIR)
        {
            hasDecomposition = true;
        }

        if (hasGlobal)
        {
            DBSetDir(dbfile, "Global");
            if (DBSetDir(dbfile, "Decomposition") == 0)
            {
                found = true;
            }
            else
            {
                debug5 << "Has Global directory, but not "
                       << " Decomposition underneath it." << endl;
                DBSetDir(dbfile, "..");
            }
        }
        if (!found && hasDecomposition)
        {
            DBSetDir(dbfile, "Decomposition");
            found = true;
        }

        if (found)
        {
            if (DBInqVarType(dbfile, "gmap0") == DB_DIR)
            {
                FindGmapConnectivity(dbfile, ndomains, nneighbors, extents,
                                     lneighbors, neighbors, numGroups,groupIds,
                                     needConnectivityInfo, needGroupInfo);
            }
            else if(DBInqVarType(dbfile,"Domain_Decomposition") == DB_MULTIMESHADJ)
            {
                FindMultiMeshAdjConnectivity(dbfile, ndomains, nneighbors,
                                             extents, lneighbors, neighbors,
                                             numGroups, groupIds,
                                             needConnectivityInfo, needGroupInfo);
            }
            else
            {
                FindStandardConnectivity(dbfile, ndomains, nneighbors, extents,
                                     lneighbors, neighbors, numGroups,groupIds,
                                     needConnectivityInfo, needGroupInfo);
            }

            /* While we're here, get domain-to-block mapping information too */
            if (DBInqVarExists(dbfile, "Domains_BlockNums"))
            {
                domainToBlockGrouping.resize(ndomains);
                DBReadVar(dbfile, "Domains_BlockNums", &domainToBlockGrouping[0]);
            }

            DBSetDir(dbfile, topDir.c_str());
        }

        //
        // Check for alternative definition of group information
        // from "_visit_domain_groups"
        //
        if (numGroups == -1 && DBInqVarExists(dbfile, "_visit_domain_groups") != 0)
        {
            int numEntries = DBGetVarLength(dbfile, "_visit_domain_groups");

            groupIds = new int[numEntries];
            DBReadVar(dbfile, "_visit_domain_groups", groupIds);

            map<int, bool> groupMap;
            for (int i = 0; i < numEntries; i++)
                groupMap[groupIds[i]] = true;
            numGroups = (int)groupMap.size();
        }
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::FindStandardConnectivity
//
//  Purpose:
//      Finds the connectivity assuming the standard conventions --
//      Domain_0, Domain_1, etc.  We assume that we are already in the
//      appropriate 'Decomposition' when this routine is called.
//
//  Notes:      The original code comes from PopulateDatabaseMetaData and was
//              written by Jeremy Meredith.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Oct 15 11:18:42 PDT 2002
//    Moved routine out of GetConnectivityAndGroupInformationFromFile.  Also
//    added some logic to not read group information if there is only one
//    group.
//
//    Cyrus Harrison, Fri Jul  6 13:54:15 PDT 2007
//    Added support for packed connectivity info from Decomp_pack var.
//
//    Cyrus Harrison, Fri Jul 20 09:28:40 PDT 2007
//    Fixed typos (Decomp_Pack vs [correct] Decomp_pack)
//
//    Jeremy Meredith, Thu Dec 18 12:15:02 EST 2008
//    Recoded the section to read the non-packed representation so that it
//    reads variables in the Domain_%d subdirectories directly; DBSetDir
//    became very slow on some files with lots of domains, but reading
//    the variables directly seemed to avoid these speed issues.
//
//    Jeremy Meredith, Tue Dec 30 15:54:04 EST 2008
//    DBInqVarExists turned out to be an unreliable test for the existence
//    of a directory.  Instead, just skip the test entirely and bail if
//    there's an error.
//
//    Mark C. Miller, Wed Jan 20 16:33:56 PST 2010
//    Replaced the 'break' when err is set with a 'return'. Otherwise, the
//    succeeding code block does work with uninitialized/bad lneighbors.
//    Also, it looked like the succeeding code block assumed that err was
//    never set as in that case, ndomains would be set to -1 and the loop
//    over domains in the succeeding block would be meaningless anyways.
//
//    Cyrus Harrison, Wed Mar 13 09:20:23 PDT 2013
//    Check for presence of "scalable_format" flag. If it exists and is on
//    we will be using local domain boundary info in the future. For now
//    simply make sure the plugin does not attempt to bindly read the
//    local info as global if it were global and crash. Also support
//    Domain to Block mapping via Domains_BlockNums array.
//
//    Mark C. Miller, Tue Jun  3 13:10:31 PDT 2014
//    If numGroups is 1 *and* ndomains is also 1, set needConnectivityInfo to
//    false. This is to deal with Silo files with one domain and one group.
// ****************************************************************************


void
avtSiloFileFormat::FindStandardConnectivity(DBfile *dbfile, int &ndomains,
            int *&nneighbors, int *&extents, int &lneighbors, int *&neighbors,
            int &numGroups, int *&groupIds, bool needConnectivityInfo,
            bool needGroupInfo)
{
    bool packed_conn_info = DBInqVarExists(dbfile,"NumDecomp_pack") != 0;
    bool scalable_fmt = DBInqVarExists(dbfile,"scalable_format") != 0;
    if(scalable_fmt)
    {
        int scalable_fmt_val = 0;
        // check the value of scalable_fmt
        DBReadVar(dbfile, "scalable_format", &scalable_fmt_val);
        scalable_fmt = (scalable_fmt_val == 1);
    }

    if(scalable_fmt)
    {
        useLocalDomainBoundries = true;
    }

    DBReadVar(dbfile, "NumDomains", &ndomains);
    if (needConnectivityInfo)
    {
        extents = new int[ndomains*6];
        nneighbors = new int[ndomains];
        lneighbors = 0;
    }

    if (needGroupInfo)
    {
         DBReadVar(dbfile, "NumBlocks", &numGroups);
         if (numGroups > 1)
         {
             groupIds = new int[ndomains];
             // check for Domains_BlockNums if we are using local
             // domain boundries
            if(useLocalDomainBoundries)
            {
                if(DBInqVarExists(dbfile,"Domains_BlockNums") != 0)
                {
                    DBReadVar(dbfile,"Domains_BlockNums",groupIds);
                }
                else
                {
                    //disable group info if we don't have the proper data.
                    numGroups = 0;
                    needGroupInfo = false;
                }
            }
         }
         else if (numGroups == 1)
         {
             groupIds = new int[ndomains];
             for (int i = 0 ; i < ndomains ; i++)
             {
                 groupIds[i] = 0;
             }
             needGroupInfo = false;
             if (ndomains == 1)
                 needConnectivityInfo = false;
         }
         else
         {
             needGroupInfo = false;  // What else to do?!?
         }
    }

    if (!packed_conn_info) // use standard connectivity info
    {
        debug1 << "avtSiloFileFormat: using standard connectivity info" <<endl;
        for (int j = 0 ; j < ndomains ; j++)
        {
            bool err = false;
            char varname[256];
            if (needConnectivityInfo)
            {
                sprintf(varname, "Domain_%d/Extents", j);
                err |= DBReadVar(dbfile, varname, &extents[j*6]) != 0;

                sprintf(varname, "Domain_%d/NumNeighbors", j);
                err |= DBReadVar(dbfile, varname, &nneighbors[j]) != 0;

                lneighbors += nneighbors[j] * 11;
            }

            if (needGroupInfo)
            {
                sprintf(varname, "Domain_%d/BlockNum", j);
                err |= DBReadVar(dbfile, varname, &(groupIds[j])) != 0;
            }

            if (err)
            {
                ndomains = -1;
                numGroups = -1;
                return;
            }
        }

        if (needConnectivityInfo)
        {
            if (lneighbors > 0)
            {
                neighbors = new int[lneighbors];
                int index = 0;
                for (int j = 0 ; j < ndomains ; j++)
                {
                    for (int k = 0 ; k < nneighbors[j] ; k++)
                    {
                        char neighborname[256];
                        sprintf(neighborname, "Domain_%d/Neighbor_%d",j,k);
                        DBReadVar(dbfile, neighborname, &neighbors[index]);
                        index += 11;
                    }
                }
            }
        }
    }
    else if(!useLocalDomainBoundries) // used packed connectivity info
    {
        debug1 << "avtSiloFileFormat: using Decomp_pack connectivity "
               << "info" <<endl;

        int numdecomp_pack = 0;
        DBReadVar(dbfile,"NumDecomp_pack",&numdecomp_pack);
        int *decomp = new int[numdecomp_pack];
        DBReadVar(dbfile,"Decomp_pack",decomp);

        int *dc_ptr = decomp;
        int i,j,k;

        // skip header
        int unblk = *dc_ptr++;
        int gnblk = *dc_ptr++;
        int nblk  = *dc_ptr++;
        dc_ptr += nblk;

        for (i=0; i < unblk; i++)
        {
            int nbnd = *dc_ptr++;
            dc_ptr += 6 + 11 * nbnd;
        }

        int *dc_start = dc_ptr;

        for (i=0; i < gnblk; i++)
        {
            if(needGroupInfo)
                groupIds[i]    = *dc_ptr++;
            else
                dc_ptr++;

            int nbnd       = *dc_ptr++;

            if(needConnectivityInfo)
            {
                nneighbors[i]  = nbnd;
                extents[6*i+0] = *dc_ptr++;
                extents[6*i+1] = *dc_ptr++;
                extents[6*i+2] = *dc_ptr++;
                extents[6*i+3] = *dc_ptr++;
                extents[6*i+4] = *dc_ptr++;
                extents[6*i+5] = *dc_ptr++;
                lneighbors    += 11*nbnd;
                dc_ptr        += 11*nbnd;
            }
            else
            {dc_ptr +=  6 + 11*nbnd;}
        }

        dc_ptr = dc_start;

        if(needConnectivityInfo)
        {
            neighbors = new int[lneighbors];
            int *nei_ptr = neighbors;
            // get boundary info
            for (i=0; i < gnblk; i++)
            {
                dc_ptr +=1; // skip group id
                int nbnd = *dc_ptr++;
                dc_ptr +=6; // skip extents

                for (k=0; k < nbnd; k++)
                {
                    for(j=0; j< 11; j++)
                        *nei_ptr++ = *dc_ptr++;
                }

            }
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::FindMultiMeshAdjConnectivity
//
//  Purpose:
//      Finds the connectivity using the silo MultiMeshadj object.
//      Currently only supports structured meshes.
//      Assumes that we are already in the appropriate 'Decomposition'
//      when this routine is called, and the name of the MultiMeshAdj object
//      is "Domain_Decomposition"
//
//  Programmer: Cyrus Harrison
//  Creation:   September 7, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Thu Feb 14 11:26:40 PST 2008
//    Guard against read mask problem that occurs with treat all dbs as time 
//    varying.
//
//    Cyrus harrison, Fri Oct  9 14:40:02 PDT 2009
//    Guard against corner case crash for disconnected mesh subset.
//
//    Mark C. Miller, Wed Nov 11 12:28:25 PST 2009
//    Added guard against case where some mmadj->nodelists arrays are null.
// ****************************************************************************

void
avtSiloFileFormat::FindMultiMeshAdjConnectivity(DBfile *dbfile, int &ndomains,
            int *&nneighbors, int *&extents, int &lneighbors, int *&neighbors,
            int &numGroups, int *&groupIds, bool needConnectivityInfo,
            bool needGroupInfo)
{
    debug1 << "avtSiloFileFormat: using MultiMeshadj Object" <<endl;
    // loop indices
    int i,j;

    // guard against improper read mask that occurs when treat all dbs as 
    // time varying is enabled. 
    unsigned long long prev_read_mask = DBGetDataReadMask2();
    DBSetDataReadMask2(prev_read_mask | DBMMADJNodelists | DBMMADJZonelists);

    // Get the MultiMeshAdjacency object
    DBmultimeshadj *mmadj_obj = DBGetMultimeshadj(dbfile,
                                                  "Domain_Decomposition",
                                                  0,NULL);
    // restore prev read mask
    DBSetDataReadMask2(prev_read_mask);
    bool ok = true;
    // Make sure we only have structured meshes.
    DBReadVar(dbfile, "NumDomains", &ndomains);
    for( i =0; i < ndomains && ok ; i++)
    {
        if(mmadj_obj->meshtypes[i] != DB_QUADMESH)
            ok = false;
    }

    // make sure we have node lists
    if( mmadj_obj->lnodelists == 0 || mmadj_obj->nodelists == NULL)
        ok = false;

    if(!ok)
    {
        // Clean the  multi mesh adj object and throw an exception
        DBFreeMultimeshadj(mmadj_obj);
        EXCEPTION1(InvalidVariableException,
                "Could not find a valid Silo MultiMeshadj object");
    }

    if (needConnectivityInfo)
    {
        extents = new int[ndomains*6];
        nneighbors = new int[ndomains];
        lneighbors = 0;

        memcpy(nneighbors,mmadj_obj->nneighbors,ndomains*sizeof(int));

        // for each neighbor visit expects the following packed info:
        // neighbor domain id  (from the mmadj neighbors - 1 value )
        // index of current domain in the neighbor's neighbor list
        // (from the mmadj back array - 1 value)
        // orientation relationship (from nodelist - 3 values)
        // overlap extents (from the nodelist - 6 values)
        // (total of 11 values)

        // we need to properly pack "neighbors" from the multimesh
        // adj object neighbors array, back array and the node lists,
        // and fill "extents" from extents info from the node lists.

        // Note: Silo's MultiMesh Adjacency Object supports unstructured 
        // and point meshes - but so far we only support structured meshes.

        int nnodelists = mmadj_obj->lneighbors;
        lneighbors = nnodelists * 11;
        neighbors  = new int[lneighbors];

        int *extents_ptr = extents;
        int *neighbors_ptr = neighbors;

        int idx = 0;

        for( i =0; i < ndomains && idx < nnodelists; i++)
        {
            if (!mmadj_obj->nodelists[idx])
                continue;

            // the node list provides the overlap region between
            // the current domain and each neighbor and an orientation
            memcpy(extents_ptr, mmadj_obj->nodelists[idx],6*sizeof(int));
            extents_ptr += 6;
            for( j =0; j < nneighbors[i]; j++)
            {
                neighbors_ptr[0] = mmadj_obj->neighbors[idx];
                neighbors_ptr[1] = mmadj_obj->back[idx];
                memcpy(&neighbors_ptr[2],&mmadj_obj->nodelists[idx][12],3*sizeof(int));
                memcpy(&neighbors_ptr[5],&mmadj_obj->nodelists[idx][6],6*sizeof(int));
                idx++;
                neighbors_ptr+=11;
            }
        }
    }

    if (needGroupInfo)
    {
         DBReadVar(dbfile, "NumBlocks", &numGroups);
         if (numGroups > 1)
         {
             groupIds = new int[ndomains];
             DBReadVar(dbfile,"Domains_BlockNums",groupIds);
         }
         else if (numGroups == 1)
         {
             groupIds = new int[ndomains];
             for (int i = 0 ; i < ndomains ; i++)
             {
                 groupIds[i] = 0;
             }
             needGroupInfo = false;
         }
         else
         {
             needGroupInfo = false;  // What else to do?!?
         }
    }

    // Free the  multi mesh adjacnecy object.
    DBFreeMultimeshadj(mmadj_obj);
}


// ****************************************************************************
//  Method: avtSiloFileFormat::FindGmapConnectivity
//
//  Purpose:
//      Finds the connectivity assuming the gmap conventions -- gmap0, gmap1,
//      etc.  We assume that we are already in the appropriate 'Decomposition'
//      when this routine is called.
//
//  Programmer: Hank Childs
//  Creation:   October 15, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Apr 10 11:42:56 PDT 2003
//    The gmap information is slightly larger than what we were allocating
//    for, leading to an ABW (array bounds write) of 1.  This was enough to
//    crash VisIt in some circumstances.
//
//    Hank Childs, Wed Jun  1 10:41:59 PDT 2005
//    The block numbers is gmap[0], not gmap[1].  gmap[1] is the domain number.
//
//    Cyrus Harrison, Tue Mar 13 11:39:15 PDT 2012
//    At Gary Kerbel's request, skip periodic boundary boundaries identified
//    by the 12th entry of the neighbor array.
//
// ****************************************************************************

void
avtSiloFileFormat::FindGmapConnectivity(DBfile *dbfile, int &ndomains,
            int *&nneighbors, int *&extents, int &lneighbors, int *&neighbors,
            int &numGroups, int *&groupIds, bool needConnectivityInfo,
            bool needGroupInfo)
{
    DBReadVar(dbfile, "NumDomains", &ndomains);
    if (needConnectivityInfo)
    {
        extents = new int[ndomains*6];
        nneighbors = new int[ndomains];
        lneighbors = 0;
    }

    if (needGroupInfo)
    {
         DBReadVar(dbfile, "NumBlocks", &numGroups);
         if (numGroups > 1)
         {
             groupIds = new int[ndomains];
         }
         else if (numGroups == 1)
         {
             groupIds = new int[ndomains];
             for (int i = 0 ; i < ndomains ; i++)
             {
                 groupIds[i] = 0;
             }
             needGroupInfo = false;
         }
         else
         {
             needGroupInfo = false;  // What else to do?!?
         }
    }

    for (int j = 0 ; j < ndomains ; j++)
    {
        char dirname[256];
        if (j > 0)
            sprintf(dirname, "../gmap%d", j);
        else
            sprintf(dirname, "gmap%d", j);
        if (DBSetDir(dbfile, dirname))
        {
            ndomains = -1;
            numGroups = -1;
            break;
        }

        int gmap_buff[1024];
        DBReadVar(dbfile, "gmap", gmap_buff);

        if (needConnectivityInfo)
        {
            extents[j*6+0] = gmap_buff[3];
            extents[j*6+1] = gmap_buff[6];
            extents[j*6+2] = gmap_buff[4];
            extents[j*6+3] = gmap_buff[7];
            extents[j*6+4] = gmap_buff[5];
            extents[j*6+5] = gmap_buff[8];
            DBReadVar(dbfile, "NumNeighbors", &nneighbors[j]);
            lneighbors += nneighbors[j] * 11;
        }

        if (needGroupInfo)
        {
            groupIds[j] = gmap_buff[0];
        }

    }
    DBSetDir(dbfile, "..");

    if (needConnectivityInfo)
    {
        if (lneighbors > 0)
        {
            int nactual_neighbors = 0;
            neighbors = new int[lneighbors];
            int index = 0;
            for (int j = 0 ; j < ndomains ; j++)
            {
                int n_all_neighbors = nneighbors[j];
                for (int k = 0 ; k < n_all_neighbors; k++)
                {
                    char neighborname[256];
                    sprintf(neighborname, "gmap%d/neighbor%d",j,k);
                    int neighbor_info[512];  // We only want 11, but it can
                                             // be as big as 400.
                    // clear the 12th entry
                    // for some codes, non-zero @ neighbor_info[11]
                    // indicates a periodic boundary condition that we should
                    // not include as a neighbor
                    neighbor_info[11] = 0;
                    DBReadVar(dbfile, neighborname, neighbor_info);
                    if(neighbor_info[11] == 0)
                    {
                        memcpy(&neighbors[index],neighbor_info,11*sizeof(int));
                        index += 11;
                        nactual_neighbors+=1;
                    }
                    else
                    {
                        // remove periodic boundary conditions
                        // from neighbor count
                        nneighbors[j] += -1;
                    }
                }
            }
            //
            // this can happen if we excluded periodic boundaries
            //
            // we already alloced neighbors, simply adjust lneighbors
            // at this point reclaming the small amount of extra memory
            // for these neighbors, seems like too much work.
            if(nactual_neighbors *11 != lneighbors)
            {
                int *nei_tmp = new int[nactual_neighbors *11];
                memcpy(nei_tmp,neighbors,nactual_neighbors *11*sizeof(int));
                delete [] neighbors;
                neighbors = nei_tmp;
                lneighbors = nactual_neighbors *11;
            }
        }

    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::AddDefvars
//
//  Purpose:
//      Parses the defvars out of the _meshtv_defvar string and adds them to
//      the meta-data as expressions.
//
//  Programmer: Hank Childs
//  Creation:   September 4, 2002
//
//  Modifications:
//
//    Sean Ahern, Fri Dec 13 17:31:21 PST 2002
//    Changed how expressions are defined.
//
//    Hank Childs, Mon Dec  1 14:43:59 PST 2003
//    Add support for tensors.
//
//    Hank Childs, Mon Jun  6 14:12:34 PDT 2005
//    Add support for slash characters.
//
//    Hank Childs, Thu Jul 21 14:12:39 PDT 2005
//    Add support for array variables.
//
// ****************************************************************************

inline void GetWord(char *&s, char *word, bool allowSlash)
{
    while (isspace(*s) && *s != '\0')
    {
        s++;
    }

    int  len = 0;
    while ((isalnum(*s) || *s == '_' || (allowSlash && *s == '/')) 
           && *s != '\0')
    {
        word[len] = *s;
        len++;
        s++;
    }
    word[len] = '\0';

    while (isspace(*s) && *s != '\0')
    {
        s++;
    }
}


void
AddDefvars(const char *defvars, avtDatabaseMetaData *md)
{
    char *dv_tmp = new char[strlen(defvars)+1];
    strcpy(dv_tmp, defvars);

    //
    // First determine the individual statements by separating them based on
    // semi-colons.
    //
    vector<char *> stmts;
    char *start = dv_tmp;
    char *end   = NULL;
    do
    {
        //
        // Find the end of the substatement -- denoted by a semi-colon.
        //
        end = strstr(start, ";");

        //
        // Put a null character where the semi-colon used to be.  If we hit
        // the end of the string, then end will be NULL, so we don't need to
        // do anything.
        //
        if (end != NULL)
        {
            *end = '\0';
        }

        //
        // Push back the start of the string so we can work on it later.
        //
        stmts.push_back(start);
        start = end+1;
    }
    while (end != NULL);

    //
    // Now go through each of the statements and determine whether they are
    // vector statements and get the statement if it is.
    //
    int size = stmts.size();
    for (int i = 0 ; i < size ; i++)
    {
        char *s = stmts[i];

        //
        // Get the name out.
        //
        char name[1024];
        GetWord(s, name, true);

        //
        // Make sure what we are looking at really is a vector.
        //
        char vartype_str[1024];
        GetWord(s, vartype_str, false);
        Expression::ExprType vartype = Expression::Unknown;
        if (strcmp(vartype_str, "mesh") == 0)
            vartype = Expression::Mesh;
        else if (strcmp(vartype_str, "scalar") == 0)
            vartype = Expression::ScalarMeshVar;
        else if (strcmp(vartype_str, "vector") == 0)
            vartype = Expression::VectorMeshVar;
        else if (strcmp(vartype_str, "tensor") == 0)
            vartype = Expression::TensorMeshVar;
        else if (strcmp(vartype_str, "array") == 0)
            vartype = Expression::ArrayMeshVar;
        else if (strcmp(vartype_str, "material") == 0)
            vartype = Expression::Material;
        else if (strcmp(vartype_str, "species") == 0)
            vartype = Expression::Species;

        if (vartype == Expression::Unknown)
        {
            debug5 << "Warning: unknown defvar type \"" << vartype_str << "\"."
                   << endl;
            continue;
        }

        Expression expr;
            expr.SetName(name);
            expr.SetDefinition(s);
            expr.SetType(vartype);
        md->AddExpression(&expr);
    }

    delete [] dv_tmp;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::AddCSGMultimesh
//
//  Purpose: Handle special requirements for multi-meshes composed of CSG
//           meshes
//
//  Programmer: Mark C. Miller
//  Creation:   June 26, 2006
//
//  Modifications:
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************
void
avtSiloFileFormat::AddCSGMultimesh(const char *const dirname,
    const char *const multimesh_name, avtDatabaseMetaData *md,
    avtSiloMultiMeshCacheEntry *mm_ent, DBfile *dbfile)
{
    DBmultimesh *mm = mm_ent->DataObject();
    int i,j;
    int nregions = 0;
    int ndims = -1;
    int meshnum = -1;
    vector<string> blockPieceNames;
    vector<int> groupIds;
    string xUnits, yUnits, zUnits;
    string xLabel, yLabel, zLabel;
    double   extents[6] = {DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX, DBL_MAX, -DBL_MAX};
    double  *extents_to_use = NULL;
    bool hideFromGUI = true; ///TODO: check on uninit var

    unsigned long long mask = DBGetDataReadMask2();
    DBSetDataReadMask2(mask|DBCSGMZonelist|DBCSGZonelistZoneNames);

    string mb_csgname = "";
    for (i = 0; i < mm->nblocks; i++)
    {
        mb_csgname = mm_ent->GenerateName(i);
        if ( mb_csgname  == "EMPTY")
            continue;

        string realvar;
        DBfile *correctFile = dbfile;
        DetermineFileAndDirectory(mb_csgname.c_str(),"", correctFile, realvar);
        DBcsgmesh *csgm = DBGetCsgmesh(correctFile, realvar.c_str());
        if (csgm == NULL)
            EXCEPTION1(InvalidVariableException, multimesh_name);

        if (!((csgm->min_extents[0] == 0.0 && csgm->max_extents[0] == 0.0 &&
               csgm->min_extents[1] == 0.0 && csgm->max_extents[1] == 0.0 &&
               csgm->min_extents[2] == 0.0 && csgm->max_extents[2] == 0.0) ||
              (csgm->min_extents[0] == -DBL_MAX && csgm->max_extents[0] == DBL_MAX &&
               csgm->min_extents[1] == -DBL_MAX && csgm->max_extents[1] == DBL_MAX &&
               csgm->min_extents[2] == -DBL_MAX && csgm->max_extents[2] == DBL_MAX)))
        {
            for (j = 0 ; j < csgm->ndims ; j++)
            {
                if (csgm->min_extents[j] < extents[2*j])
                    extents[2*j] = csgm->min_extents[j];
                if (csgm->max_extents[j] > extents[2*j+1])
                    extents[2*j+1] = csgm->max_extents[j];
            }
            extents_to_use = extents;
        }

        if (csgm->zones->zonenames)
        {
            for (j = 0; j < csgm->zones->nzones; j++)
            {
                blockPieceNames.push_back(csgm->zones->zonenames[j]);
                groupIds.push_back(i);
            }
        }

        nregions += csgm->zones->nzones;

        if (csgm->ndims > ndims)
            ndims = csgm->ndims;

        if (meshnum == -1)
        {
            meshnum = i;

            if (csgm->units[0] != NULL)
                xUnits = csgm->units[0];
            if (csgm->units[1] != NULL)
                yUnits = csgm->units[1];
            if (csgm->units[2] != NULL)
                zUnits = csgm->units[2];

            if (csgm->labels[0] != NULL)
                xLabel = csgm->labels[0];
            if (csgm->labels[1] != NULL)
                yLabel = csgm->labels[1];
            if (csgm->labels[2] != NULL)
                zLabel = csgm->labels[2];

            hideFromGUI = csgm->guihide;
        }

        DBFreeCsgmesh(csgm);

    }
    DBSetDataReadMask2(mask);


    // a value for meshnum of -1 at this point indicates
    // no non-EMPTY blocks found
    if (meshnum != -1)
    {

        char *name_w_dir = GenerateName(dirname, multimesh_name, topDir.c_str());
        avtMeshMetaData *mmd = new avtMeshMetaData(NULL,
                                                   extents_to_use, name_w_dir,
                                       nregions, 0, 0, 0, ndims, ndims, AVT_CSG_MESH);

        mmd->blockTitle = "regions";
        mmd->blockPieceName = "region";
        mmd->blockNames = blockPieceNames;
        mmd->xUnits = xUnits;
        mmd->yUnits = yUnits;
        mmd->zUnits = zUnits;
        mmd->xLabel = xLabel;
        mmd->yLabel = yLabel;
        mmd->zLabel = zLabel;
        mmd->hideFromGUI = hideFromGUI;
        //mmd->loadBalanceScheme = LOAD_BALANCE_DBPLUGIN_DYNAMIC;

        if (mm->nblocks > 1)
        {
            mmd->numGroups = mm->nblocks;
            mmd->groupTitle = "blocks";
            mmd->groupPieceName = "block";
            md->Add(mmd);
            md->AddGroupInformation(mm->nblocks, nregions, groupIds);
        }
        else
        {
            md->Add(mmd);
        }

        // Store off the important info about this multimesh
        // so we can match other multi-objects to it later
        StoreMultimeshInfo(dirname, name_w_dir, meshnum, mm_ent);

        delete [] name_w_dir;
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetNodelistsVar
//
//  Purpose: Return scalar variable representing (enumerated scalar) nodelists 
//           meshes
//
//  Programmer: Mark C. Miller 
//  Creation:   March 18, 2008 
//
//  Modifications:
//    Mark C. Miller, Tue Apr 15 19:53:08 PDT 2008
//    Made it deal with case where domain boundary info object is NOT
//    available by falling back to attempting to get the actual mesh dataset.
//    Also, deal with case where groupIds is NOT set.
//
//    Mark C. Miller, Thu Apr 17 10:30:54 PDT 2008
//    With help from Cyrus, fixed some indexing errors between domain-local
//    and group-global indexing.
//
//    Kathleen Bonnell, Wed Jul 2 14:43:22 PDT 2008
//    Removed unreferenced variables.
//
//    Mark C. Miller, Tue Mar  3 19:33:23 PST 2009
//    Added logic to get blockNum from groupInfo before attempting to use
//    special vtk array.
// 
//    Mark C. Miller, Thu Apr 12 23:08:49 PDT 2012
//    Replaced vtkFloatArray with vtkBitArray and support of arbitrarily
//    large numbers of nodelists
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetNodelistsVar(int domain)
{
    size_t i;
    vtkDataArray *nlvar = 0;
    string meshName = metadata->MeshForVar("Nodelists");
    const avtMeshMetaData *mmd = metadata->GetMesh(meshName); (void) mmd;

    debug5 << "Generating Nodelists variable for domain " << domain << endl;

    //
    // Look up the mesh in the cache.
    //
    vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
    if (ds == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
            "paint Nodelists variable", meshName.c_str(), domain);
        EXCEPTION1(ImproperUseException, msg);
    }

    // we expect really only two kinds of meshes here.
    int dims[3];
    if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::SafeDownCast(ds);
        rgrid->GetDimensions(dims);
    }
    else if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
        vtkStructuredGrid *sgrid = vtkStructuredGrid::SafeDownCast(ds);
        sgrid->GetDimensions(dims);
    }
    else
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Do not recognize dataset type for mesh \"%s\" "
            "for domain %d to paint Nodelists variable", meshName.c_str(), domain);
        EXCEPTION1(ImproperUseException, msg);
    }

    vtkIntArray *arr = vtkIntArray::SafeDownCast(ds->GetFieldData()->GetArray("base_index"));
    if (arr == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find field data array \"base_index\""
            "on mesh \"%s\" for domain %d to paint Nodelists variable", meshName.c_str(), domain);
        EXCEPTION1(ImproperUseException, msg);
    }
    int base_index[3];
    base_index[0] = arr->GetValue(0) ? arr->GetValue(0)-1 : 0;
    base_index[1] = arr->GetValue(1) ? arr->GetValue(1)-1 : 0;
    base_index[2] = arr->GetValue(2) ? arr->GetValue(2)-1 : 0;

    int blockNum = -1;
    if (groupInfo.haveGroups)
    {
        blockNum = groupInfo.ids[domain];
    }
    else
    {
        vtkIntArray *arr1 = vtkIntArray::SafeDownCast(ds->GetFieldData()->GetArray("group_id"));
        if (arr1 != 0)
            blockNum = arr1->GetValue(0);
    }
    if (blockNum == -1)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find obtain block number " 
            "on mesh \"%s\" for domain %d to paint Nodelists variable", meshName.c_str(), domain);
        EXCEPTION1(ImproperUseException, msg);
    }

    int group_min_idx[3] = {0,0,0};
    int group_max_idx[3] = {0,0,0};

    group_min_idx[0] = base_index[0];
    group_min_idx[1] = base_index[1];
    group_min_idx[2] = base_index[2];

    group_max_idx[0] = base_index[0] + dims[0] - 1;
    group_max_idx[1] = base_index[1] + dims[1] - 1;
    group_max_idx[2] = base_index[2] + dims[2] - 1;

    //
    // Initialize the return variable array
    //
    const int bpuc = sizeof(unsigned char)*8;
    nlvar = vtkBitArray::New();
    nlvar->SetNumberOfComponents(((numNodeLists+bpuc-1)/bpuc)*bpuc);
    nlvar->SetNumberOfTuples(dims[0]*dims[1]*(dims[2]?dims[2]:1));
    memset(nlvar->GetVoidPointer(0), 0, nlvar->GetSize()/bpuc);

    //
    // Iterate over all nodesets for this block, finding those that have
    // 'windows' on this block. 
    //
    const vector<int> &windowsOnThisBlock = nlBlockToWindowsMap[blockNum];
    for (i = 0; i < windowsOnThisBlock.size(); i += 7)
    {
        //
        // Entries in windowsOnThisBlock vector come in groups of 7.
        // First one is the nodelist id (value), next 6 are its window
        //
        int val = windowsOnThisBlock[i];
        int winExtents[6];
        for (int q = 0; q < 6; q++)
            winExtents[q] = windowsOnThisBlock[i+1+q];

        // find intersection between extents and group_min_idx & group_max_idx
        int isec[6];
        isec[0] = winExtents[0] > group_min_idx[0] ? winExtents[0] : group_min_idx[0];
        isec[1] = winExtents[1] < group_max_idx[0] ? winExtents[1] : group_max_idx[0];
        isec[2] = winExtents[2] > group_min_idx[1] ? winExtents[2] : group_min_idx[1];
        isec[3] = winExtents[3] < group_max_idx[1] ? winExtents[3] : group_max_idx[1];
        isec[4] = winExtents[4] > group_min_idx[2] ? winExtents[4] : group_min_idx[2];
        isec[5] = winExtents[5] < group_max_idx[2] ? winExtents[5] : group_max_idx[2];
                    
        // shift back to domain logical coords
        isec[0] -= base_index[0];
        isec[1] -= base_index[0];
        isec[2] -= base_index[1];
        isec[3] -= base_index[1];
        isec[4] -= base_index[2];
        isec[5] -= base_index[2];

        // For 2D, ensure we enter outermost loop, below, for one iteration 
        if (dims[2] == 0) isec[5] = isec[4] = 0;
                    
        //
        // We've got a block window that overlaps with the current domain's
        // extents. This mean's the domain contains nodes that are part of
        // this window. So, now we need to 'paint' values for this nodelist
        // into the variable array we are returning.
        //
        int nxy = dims[0] * dims[1];
        for (int zi = isec[4]; zi <= isec[5]; zi++)
        {
            for (int yi = isec[2]; yi <= isec[3]; yi++)
            {
                for (int xi = isec[0]; xi <= isec[1]; xi++)
                    nlvar->SetComponent(zi*nxy + yi*dims[0] + xi, val, 1);
            }
        }
    }

    return nlvar;
}

// ****************************************************************************
//  Function: compare_node_ids
//
//  Purpose: Callback for qsort calls to sort vector of nodes of a face. 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 19, 2008 
//
//  Modifications
//    Mark C. Miller, Fri Mar 20 11:05:22 PDT 2009
//    Made it treat negative values (unspecified node ids) as really larger
//    than any positive node. This way, negative valued node ids always wind
//    up at the end of the sorted list.
// ****************************************************************************
static int
compare_node_ids(const void *a, const void *b)
{
    int *ia = (int *) a;
    int *ib = (int *) b;
    if (*ia < 0)
    {
        if (*ib < 0)
            return 0;
        else
            return 1;
    }
    else if (*ib < 0)
        return -1;
    else if (*ia < *ib)
        return -1;
    else if (*ia > *ib)
        return 1;
    else return 0;
}

// ****************************************************************************
//  Function: compare_ev_pair 
//
//  Purpose: Callback for qsort calls to sort vector of elemid/elemvalue 
//           meshes
//
//  Programmer: Mark C. Miller 
//  Creation:   December 19, 2008 
// ****************************************************************************

typedef struct {int id; int val;} ev_pair_t;
static int
compare_ev_pair(const void *a, const void *b)
{
    ev_pair_t *eva = (ev_pair_t *) a;
    ev_pair_t *evb = (ev_pair_t *) b;
    if (eva->id < evb->id)
        return -1;
    else if (eva->id > evb->id)
        return 1;
    else return 0;
}

//
// ****************************************************************************
//  Function: PaintNodesForAnnotIntFacelist 
//
//  Purpose: Traverse a zonelist in edge- or face-centered order and paint
//           values into node-centered variable on nodes associated with
//           specific edges or faces.
//
//  We iterate over the zones in the zonelist. For each zone, we iterate over its
//  faces (3D) or edges (2D). In 2D, each edge is represented by 2 integer node ids.
//  In 3D, each face (tri or quad) is represented by 4 integer node ids. As we iterate,
//  we update the faceIdx or edgeIdx indicating the identifier for the next edge or
//  face we find. Whenever that identifier is equal to the identifier in the list of
//  edges or faces we are looking for (in elemidv), we paint data onto the nodes
//  associated with that edge or face. We use a multi level map to keep track of
//  faces or edges we've already seen so we don't wind up counting them twice and
//  screwing up the faceIdx or edgeIdx value.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 19, 2008 
//
//  Modifications:
//    Mark C. Miller, Sat Dec 20 08:31:29 PST 2008
//    Added 3D code.
//
//    Mark C. Miller, Fri Mar 20 11:10:04 PDT 2009
//    Added comment regarding effect of compare_node_ids on negative valued
//    node ids (-1) of 3-node faces.
//
//    Mark C. Miller, Thu Apr 12 23:10:07 PDT 2012
//    Replaced float* arg with vtiBitArray and changed calls to manipulate the
//    float* array with calls to SetComponents of the vtkBitArray. This enables
//    the code to support an arbitrary number of nodelists.
//
//    Mark C. Miller, Wed Dec 12 04:54:13 PST 2018
//    Adjust logic to handle cases where edges/faces appear in multiple lists.
// ****************************************************************************

static void
PaintNodesForAnnotIntFacelist(vtkBitArray *nlvar,
    const vector<ev_pair_t> &elemidv, DBucdmesh *um)
{
    DBzonelist *zl = um->zones;

    if (um->ndims == 2)
    {
        map<int, map<int, bool> > previouslySeenEdgesMap;

        int edgeIdx = 0;
        int nlIdx = 0;
        int elemIdx = 0;
        for (int seg = 0; seg < zl->nshapes; seg++)
        {
            for (int zn = 0; zn < zl->shapecnt[seg]; zn++)
            {
                //
                // For each zone we build a tiny list of its edges where
                // each edge is represented by 2 node ids in the zonelist's
                // nodelist. Note that we are using counterclockwise order
                // which is consistent with right-hand-rule for normal towards
                // the eye-point and is also consistent with Silo user's 
                // manual for 2D meshes. 
                //
                int nedges = 0;
                int edge[4][2];
                switch (zl->shapetype[seg])
                {
                    case DB_ZONETYPE_TRIANGLE:
                    {
                        edge[0][0] = zl->nodelist[nlIdx+0];
                        edge[0][1] = zl->nodelist[nlIdx+1];
                        edge[1][0] = zl->nodelist[nlIdx+1];
                        edge[1][1] = zl->nodelist[nlIdx+2];
                        edge[2][0] = zl->nodelist[nlIdx+2];
                        edge[2][1] = zl->nodelist[nlIdx+0];
                        nedges = 3;
                    }
                    case DB_ZONETYPE_QUAD:
                    {
                        edge[0][0] = zl->nodelist[nlIdx+0];
                        edge[0][1] = zl->nodelist[nlIdx+1];
                        edge[1][0] = zl->nodelist[nlIdx+1];
                        edge[1][1] = zl->nodelist[nlIdx+2];
                        edge[2][0] = zl->nodelist[nlIdx+2];
                        edge[2][1] = zl->nodelist[nlIdx+3];
                        edge[3][0] = zl->nodelist[nlIdx+3];
                        edge[3][1] = zl->nodelist[nlIdx+0];
                        nedges = 4;
                    }
                }
                nlIdx += zl->shapesize[seg];
            
                for (int i = 0; i < nedges; i++)
                {
                    bool unseenEdge = false;

                    // Ensure list of nodes for edge is in sorted order
                    // so that when we lookup an edge in the previouslySeenEdgesMap
                    // we do it consistently with lowest node id first.
                    if (edge[i][0] > edge[i][1])
                    {
                        int tmp = edge[i][0];
                        edge[i][0] = edge[i][1];
                        edge[i][1] = tmp;
                    }

                    // Do a find on node id 0 of the edge.
                    map<int, map<int, bool> >::iterator e0it =
                        previouslySeenEdgesMap.find(edge[i][0]);

                    if (e0it == previouslySeenEdgesMap.end())
                        unseenEdge = true;
                    else
                    {
                        // Do a find on node id 1 of the edge.
                        map<int, bool>::iterator e1it =
                            e0it->second.find(edge[i][1]);
                        if (e1it == e0it->second.end())
                            unseenEdge = true;
                        else
                        {
                            //
                            // Since we know we'll only ever see any given edge at most
                            // twice, when we arrive here, we know we're seeing it for
                            // the second time and we can safely erase it, reducing
                            // storage a bit as we go. In theory, this would reduce by
                            // half the average storage requirement of the map.
                            e0it->second.erase(e1it);
                            if (e0it->second.size() == 0)
                                previouslySeenEdgesMap.erase(e0it);
                        }
                    }

                    if (unseenEdge)
                    {
                        previouslySeenEdgesMap[edge[i][0]][edge[i][1]] = true;

                        //
                        // If the edge id of the current edge (edgeIdx) is the
                        // same as the current one in the list we are here to paint,
                        // then paint it. The while loop is to handle duplcate entries.
                        //
                        while (edgeIdx == elemidv[elemIdx].id)
                        {
                            nlvar->SetComponent(edge[i][0], elemidv[elemIdx].val, 1);
                            nlvar->SetComponent(edge[i][1], elemidv[elemIdx].val, 1);
                            elemIdx++;

                            //
                            // If we've reached the end of the list of edge ids we
                            // are here to paint, then we are done.
                            //
                            if ((size_t)elemIdx >= elemidv.size())
                                return;
                        }

                        edgeIdx++;
                    }
                }
            }
        }
    }
    else // 3D case
    {
        map<int, map<int, map<int, map<int,bool> > > > previouslySeenFacesMap;

        int faceIdx = 0;
        int nlIdx = 0;
        int elemIdx = 0;
        for (int seg = 0; seg < zl->nshapes; seg++)
        {
            for (int zn = 0; zn < zl->shapecnt[seg]; zn++)
            {
                //
                // For each zone we build a tiny list of its faces where
                // each face is represented by 4 node ids from the zonelist's
                // nodelist. For tris, the last id is set to -1. The face
                // and node orders represented below are taken directly from
                // the Silo user's manual.
                //
                int nfaces = 0;
                int face[6][4];
                switch (zl->shapetype[seg])
                {
                    case DB_ZONETYPE_TET:
                    {
                        face[0][0] = zl->nodelist[nlIdx+0];
                        face[0][1] = zl->nodelist[nlIdx+1];
                        face[0][2] = zl->nodelist[nlIdx+2];
                        face[0][3] = -1;
                        face[1][0] = zl->nodelist[nlIdx+0];
                        face[1][1] = zl->nodelist[nlIdx+2];
                        face[1][2] = zl->nodelist[nlIdx+3];
                        face[1][3] = -1;
                        face[2][0] = zl->nodelist[nlIdx+0];
                        face[2][1] = zl->nodelist[nlIdx+3];
                        face[2][2] = zl->nodelist[nlIdx+1];
                        face[2][3] = -1;
                        face[3][0] = zl->nodelist[nlIdx+1];
                        face[3][1] = zl->nodelist[nlIdx+3];
                        face[3][2] = zl->nodelist[nlIdx+2];
                        face[3][3] = -1;
                        nfaces = 4;
                    }
                    case DB_ZONETYPE_PYRAMID:
                    {
                        face[0][0] = zl->nodelist[nlIdx+0];
                        face[0][1] = zl->nodelist[nlIdx+1];
                        face[0][2] = zl->nodelist[nlIdx+2];
                        face[0][3] = zl->nodelist[nlIdx+3];
                        face[1][0] = zl->nodelist[nlIdx+0];
                        face[1][1] = zl->nodelist[nlIdx+3];
                        face[1][2] = zl->nodelist[nlIdx+4];
                        face[1][3] = -1;
                        face[2][0] = zl->nodelist[nlIdx+0];
                        face[2][1] = zl->nodelist[nlIdx+4];
                        face[2][2] = zl->nodelist[nlIdx+1];
                        face[2][3] = -1;
                        face[3][0] = zl->nodelist[nlIdx+1];
                        face[3][1] = zl->nodelist[nlIdx+4];
                        face[3][2] = zl->nodelist[nlIdx+2];
                        face[3][3] = -1;
                        face[4][0] = zl->nodelist[nlIdx+2];
                        face[4][1] = zl->nodelist[nlIdx+4];
                        face[4][2] = zl->nodelist[nlIdx+3];
                        face[4][3] = -1;
                        nfaces = 5;
                    }
                    case DB_ZONETYPE_PRISM:
                    {
                        face[0][0] = zl->nodelist[nlIdx+0];
                        face[0][1] = zl->nodelist[nlIdx+1];
                        face[0][2] = zl->nodelist[nlIdx+2];
                        face[0][3] = zl->nodelist[nlIdx+3];
                        face[1][0] = zl->nodelist[nlIdx+0];
                        face[1][1] = zl->nodelist[nlIdx+3];
                        face[1][2] = zl->nodelist[nlIdx+4];
                        face[1][3] = -1;
                        face[2][0] = zl->nodelist[nlIdx+0];
                        face[2][1] = zl->nodelist[nlIdx+4];
                        face[2][2] = zl->nodelist[nlIdx+5];
                        face[2][3] = zl->nodelist[nlIdx+1];
                        face[3][0] = zl->nodelist[nlIdx+1];
                        face[3][1] = zl->nodelist[nlIdx+5];
                        face[3][2] = zl->nodelist[nlIdx+2];
                        face[3][3] = -1;
                        face[4][0] = zl->nodelist[nlIdx+2];
                        face[4][1] = zl->nodelist[nlIdx+5];
                        face[4][2] = zl->nodelist[nlIdx+4];
                        face[4][3] = zl->nodelist[nlIdx+3];
                        nfaces = 5;
                    }
                    case DB_ZONETYPE_HEX:
                    {
                        face[0][0] = zl->nodelist[nlIdx+0];
                        face[0][1] = zl->nodelist[nlIdx+1];
                        face[0][2] = zl->nodelist[nlIdx+5];
                        face[0][3] = zl->nodelist[nlIdx+4];
                        face[1][0] = zl->nodelist[nlIdx+0];
                        face[1][1] = zl->nodelist[nlIdx+3];
                        face[1][2] = zl->nodelist[nlIdx+2];
                        face[1][3] = zl->nodelist[nlIdx+1];
                        face[2][0] = zl->nodelist[nlIdx+0];
                        face[2][1] = zl->nodelist[nlIdx+4];
                        face[2][2] = zl->nodelist[nlIdx+7];
                        face[2][3] = zl->nodelist[nlIdx+3];
                        face[3][0] = zl->nodelist[nlIdx+1];
                        face[3][1] = zl->nodelist[nlIdx+2];
                        face[3][2] = zl->nodelist[nlIdx+6];
                        face[3][3] = zl->nodelist[nlIdx+5]; 
                        face[4][0] = zl->nodelist[nlIdx+2];
                        face[4][1] = zl->nodelist[nlIdx+3];
                        face[4][2] = zl->nodelist[nlIdx+7];
                        face[4][3] = zl->nodelist[nlIdx+6];
                        face[5][0] = zl->nodelist[nlIdx+4];
                        face[5][1] = zl->nodelist[nlIdx+5];
                        face[5][2] = zl->nodelist[nlIdx+6];
                        face[5][3] = zl->nodelist[nlIdx+7];
                        nfaces = 6;
                    }
                }
                nlIdx += zl->shapesize[seg];
            
                for (int i = 0; i < nfaces; i++)
                {
                    bool unseenFace = false;

                    // Ensure list of nodes for face is in sorted order
                    // so that when we lookup an face in the previouslySeenEdgesMap
                    // we do it consistently with lowest node id first.
                    qsort(face[i], 4, sizeof(int), compare_node_ids);

                    // Do a find on node id 0 of the face.
                    map<int, map<int, map<int, map<int, bool> > > >::iterator f0it =
                        previouslySeenFacesMap.find(face[i][0]);

                    if (f0it == previouslySeenFacesMap.end())
                        unseenFace = true;
                    else
                    {
                        // Do a find on node id 1 of the face.
                        map<int, map<int, map<int, bool> > >::iterator f1it =
                            f0it->second.find(face[i][1]);
                        if (f1it == f0it->second.end())
                            unseenFace = true;
                        else
                        {
                            // Do a find on node id 2 of the face.
                            map<int, map<int, bool> >::iterator f2it =
                                f1it->second.find(face[i][2]);
                            if (f2it == f1it->second.end())
                                unseenFace = true;
                            else
                            {
                                // Do a find on node id 3 of the face.
                                map<int, bool>::iterator f3it =
                                    f2it->second.find(face[i][3]);
                                if (f3it == f2it->second.end())
                                    unseenFace = true;
                                else
                                {
                                    // Since we know we'll only ever see any given face at most
                                    // twice, when we arrive here, we know we're seeing it for
                                    // the second time and we can safely erase it, reducing
                                    // storage a bit as we go. In theory, this would reduce by
                                    // half the average storage requirement of the map.
                                    f2it->second.erase(f3it);
                                    if (f2it->second.size() == 0)
                                    {
                                        f1it->second.erase(f2it);
                                        if (f1it->second.size() == 0)
                                        {
                                            f0it->second.erase(f1it);
                                            if (f0it->second.size() == 0)
                                                previouslySeenFacesMap.erase(f0it);
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (unseenFace)
                    {
                        previouslySeenFacesMap[face[i][0]][face[i][1]]
                            [face[i][2]][face[i][3]] = true;

                        //
                        // If the face id of the current face (faceIdx) is the
                        // same as the current one in the list we are here to paint,
                        // then paint it. The while loop is to handle duplicate entries.
                        //
                        while (faceIdx == elemidv[elemIdx].id)
                        {
                            nlvar->SetComponent(face[i][0], elemidv[elemIdx].val, 1);
                            nlvar->SetComponent(face[i][1], elemidv[elemIdx].val, 1);
                            nlvar->SetComponent(face[i][2], elemidv[elemIdx].val, 1);
                            // The compare_node_ids comparison method used to sort the nodes
                            // of the face in the call to qsort, above, is designed to cause
                            // all -1 valued nodes to wind up at the end of the sorted list.
                            if (face[i][3] != -1)
                                nlvar->SetComponent(face[i][3], elemidv[elemIdx].val, 1);
                            elemIdx++;

                            //
                            // If we've reached the end of the list of face ids we
                            // are here to paint, then we are done.
                            //
                            if ((size_t)elemIdx >= elemidv.size())
                                return;
                        }

                        faceIdx++;
                    }
                }
            }
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetAnnotIntNodelistsVar
//
//  Purpose: Return scalar variable representing (enumerated scalar) nodelists 
//           meshes based on contents of ANNOTATION_INT object.
//
//  Programmer: Mark C. Miller 
//  Creation:   December 18, 2008 
//
//  Modifications:
//    Mark C. Miller, Wed Feb 25 17:36:51 PST 2009
//    Add missing DBZonelistInfo flag from setting of data read mask just
//    prior to getting the ucdmesh.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Thu Apr 12 23:12:08 PDT 2012
//    Replaced vtkFloatArray with vtkBitArray to support arbitrary number
//    of nodelists.
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 15:03:13 PST 2016
//    Add logic to return immediately if the associated meshName is the name
//    of an all-empty multi-mesh.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetAnnotIntNodelistsVar(int domain, string listsname)
{
    size_t i;
    string meshName = metadata->MeshForVar(listsname);

    if (emptyObjectsList.find(meshName) != emptyObjectsList.end())
        return 0;

    //
    // Look up the mesh in the cache.
    //
    vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
    if (ds == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
            "paint \"%s\" variable", meshName.c_str(), domain, listsname.c_str());
        EXCEPTION1(InvalidVariableException, msg);
    }

    debug5 << "Generating " << listsname << " variable for domain " << domain << endl;

    //
    // Initialize the return variable array
    //
    const int bpuc = sizeof(unsigned char)*8;
    int npts = ds->GetNumberOfPoints();
    vtkBitArray *nlvar = vtkBitArray::New();
    nlvar->SetNumberOfComponents(((numAnnotIntLists+bpuc-1)/bpuc)*bpuc);
    nlvar->SetNumberOfTuples(npts);
    memset(nlvar->GetVoidPointer(0), 0, nlvar->GetSize()/bpuc);

    //
    // Try to get the ANNOTATION_INT object. In theory, this could fail on a
    // domain for which no annotation's were defined. Use the GetMeshHelper
    // function to get the correct file to query for ANNOTATION_INT object.
    //
    DBfile *domain_file = GetFile(tocIndex);
    string domain_mesh;
    GetMeshHelper(domain, meshName.c_str(), 0, 0, &domain_file, domain_mesh);
    DBcompoundarray *ai = DBGetCompoundarray(domain_file, "ANNOTATION_INT");
    if (ai == 0)
        return nlvar;

    //
    // Using scalar metadata, determine the 'value' to be associated with
    // each list name we find in the ANNOTATION_INT object.
    //
    const avtScalarMetaData *smd = metadata->GetScalar(listsname);
    map<string, int> nameToValMap;
    for (i = 0; i < smd->enumNames.size(); i++)
        nameToValMap[smd->enumNames[i]] = (int) smd->enumRanges[2*i];

    //
    // As we iterate over this compound array's members, we need to
    // take care that we inspect only the '_node' named ones if we're
    // here to handle an AnnotInt_Nodelist object and the '_face' ones
    // if we're here to handle an AnnotInt_Facelist object. This loop
    // gathers all the node/face ids to be painted along with the values
    // to paint into elemidv vector.
    //
    int elemoff = 0;
    int *elemvals = (int *) ai->values;
    vector<ev_pair_t> elemidv;
    for (i = 0; i < (size_t)ai->nelems; i++)
    {
        int len = strlen(ai->elemnames[i]);
        if (listsname == "AnnotInt_Nodelists" && 
            strncmp("_node",&(ai->elemnames[i][len-5]),5) == 0)
        {
            for (int j = 0; j < ai->elemlengths[i]; j++)
            {
                ev_pair_t idv = {elemvals[elemoff+j],
                                 nameToValMap[string(ai->elemnames[i],0,len-5)]};
                elemidv.push_back(idv);
            }
        }
        else if (listsname == "AnnotInt_Facelists" && 
            strncmp("_face",&(ai->elemnames[i][len-5]),5) == 0)
        {
            for (int j = 0; j < ai->elemlengths[i]; j++)
            {
                ev_pair_t idv = {elemvals[elemoff+j],
                                 nameToValMap[string(ai->elemnames[i],0,len-5)]};
                elemidv.push_back(idv);
            }
        }

        elemoff += ai->elemlengths[i];
    }

    //
    // If we're here for nodelists, then the elemidv already contains the
    // node ids we need to paint enum values onto. So, just iterate over
    // those nodes and set their values accordingly.
    //
    if (listsname == "AnnotInt_Nodelists")
    {
        for (i = 0; i < elemidv.size(); i++)
            nlvar->SetComponent(elemidv[i].id, elemidv[i].val, 1);
    }
    else
    {

        //
        // If we're in this block, we must be here for facelists. So, we now
        // need to read and traverse the mesh's zonelist structure, enumerating
        // faces so we can determine which nodes we need to paint values onto.
        //

        //
        // Use mesh helper func. to determine file and mesh object name. 
        //
        int type;
        string directory_mesh;
        DBmultimesh *mm;
        DBfile *dbfile = GetFile(tocIndex);
        DBfile *domain_file = dbfile;
        GetMeshHelper(domain, meshName.c_str(), &mm, &type, &domain_file, directory_mesh);

        //
        // Read the mesh header and just the zonelist for it.
        //
        unsigned long long oldMask = DBSetDataReadMask2(DBUMZonelist|DBZonelistInfo);
        DBucdmesh  *um = DBGetUcdmesh(domain_file, directory_mesh.c_str());
        DBSetDataReadMask2(oldMask);

        if (um == NULL)
        {
            if (!ignoreMissingBlocks)
            {
                char msg[256];
                SNPRINTF(msg, sizeof(msg), "DBGetUcdmesh() failed for \"%s\" for domain %d to "
                    "paint \"%s\" variable", meshName.c_str(), domain, listsname.c_str());
                EXCEPTION1(InvalidVariableException, msg);
            }
            return 0;
        }

        //
        // Call the method that traverses the zonelist, enumerating faces and then
        // for each face specified in elemids, paint values onto the nodes. We
        // need to sort the elemidv data in increasing face id first because the
        // method we're calling is written to assume that.
        //
        qsort(&elemidv[0], elemidv.size(), sizeof(ev_pair_t), compare_ev_pair);
        PaintNodesForAnnotIntFacelist(nlvar, elemidv, um);

        DBFreeUcdmesh(um);
    }

    DBFreeCompoundarray(ai);

    return nlvar;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMrgTreeNodelistsVar
//
//  Purpose: Return scalar variable representing (enumerated scalar) nodelists 
//           of meshes based on contents of MRG Tree nodesets. Currently, this
//           is implemented only for UCD meshes and only for NODEsets.
//
//  Creation: Mark C. Miller, Tue Jul 10 19:56:44 PDT 2012
//
//  Modifications:
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 15:03:59 PST 2016
//    Add logic to return immediately if the associated mesh is an all-empty
//    multi-block mesh.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetMrgTreeNodelistsVar(int domain, string listsname)
{
    string meshName = metadata->MeshForVar(listsname);

    if (emptyObjectsList.find(meshName) != emptyObjectsList.end())
        return 0;

    //
    // Look up the mesh in the cache.
    //
    vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
    if (ds == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
            "paint \"%s\" variable", meshName.c_str(), domain, listsname.c_str());
        EXCEPTION1(InvalidVariableException, msg);
    }

    debug3 << "Generating " << listsname << " variable for domain " << domain << endl;

    const avtScalarMetaData *smd = metadata->GetScalar(listsname);
    int nenumVals = smd->enumNames.size();

    //
    // Initialize the return variable array
    //
    const int bpuc = sizeof(unsigned char)*8;
    int npts = ds->GetNumberOfPoints();
    vtkBitArray *nlvar = vtkBitArray::New();
    nlvar->SetNumberOfComponents(((nenumVals+bpuc-1)/bpuc)*bpuc);
    nlvar->SetNumberOfTuples(npts);
    memset(nlvar->GetVoidPointer(0), 0, nlvar->GetSize()/bpuc);

    //
    // For the failure cases below here, why don't we cleanup everything including
    // nlvar and trigger an exception? The reason is that a given domain may not
    // have (e.g. contain) any of the MRG tree subsets of interest here and that is ok.
    // We will wind up returing an all zero nlvar indicating nothing in this domain is
    // relevant regardless of which nodesets are selected in the current selection.
    //

    //
    // Try to get the MRG Tree object for this mesh.
    //
    DBfile *domain_file = GetFile(tocIndex);
    string domain_mesh;
    GetMeshHelper(domain, meshName.c_str(), 0, 0, &domain_file, domain_mesh);

    // Only get the mesh header information, none of the problem sized data.
    unsigned long long oldMask = DBSetDataReadMask2(0x0);
    DBucdmesh  *um = DBGetUcdmesh(domain_file, domain_mesh.c_str());
    DBSetDataReadMask2(oldMask);
    if (!um)
    {
        debug3 << "Unable to get mesh \"" << meshName << "\" for domain " << domain << endl;
        return nlvar;
    }
    if (!um->mrgtree_name)
    {
        debug3 << "Mesh \"" << meshName << "\" for domain " << domain << " has no MRG Tree" << endl;
        DBFreeUcdmesh(um);
        return nlvar;
    }

    string mrgtnm_abspath = ResolveSiloIndObjAbsPath(domain_file, domain_mesh, um->mrgtree_name);
    DBFreeUcdmesh(um);

    DBmrgtree *mrgt = DBGetMrgtree(domain_file, mrgtnm_abspath.c_str());
    if (!mrgt)
    {
        debug3 << "Unable to get MRG Tree \"" << mrgtnm_abspath << "\" for domain " << domain << endl;
        return nlvar;
    }
    if (DBSetCwr(mrgt, listsname.substr(0,8).c_str()) < 0)
    {
        debug3 << "MRG Tree \"" << mrgtnm_abspath << "\" for domain " << domain 
               << " has no top node named \"" << listsname.substr(0,8) << endl;
        DBFreeMrgtree(mrgt);
        return nlvar;
    }

    DBgroupelmap *cgm = GetCondensedGroupelMap(domain_file, mrgtnm_abspath,
        mrgt->cwr, forceSingle, DB_NODECENT);
    DBFreeMrgtree(mrgt);
    if (!cgm)
    {
        debug3 << "Unable to get condensed groupel map for MRG Tree node \"" << mrgtnm_abspath
               << "\" for domain " << domain << endl;
        return nlvar;
    }

    for (int i = 0; i < cgm->num_segments; i++)
        for (int j = 0; j < cgm->segment_lengths[i]; j++)
            nlvar->SetComponent(cgm->segment_data[i][j], i, 1);
    DBFreeGroupelmap(cgm);

    return nlvar;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetBlockDecompositionVar
//
//  Mark C. Miller, Wed Jun 15 09:28:04 PDT 2016
//
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetBlockDecompositionAsVar(int domain, string meshName)
{
    if (emptyObjectsList.find(meshName) != emptyObjectsList.end())
        return 0;

    //
    // Look up the mesh in the cache.
    //
    vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
    if (ds == 0)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
            "paint \"block_nums\" variable", meshName.c_str(), domain);
        EXCEPTION1(InvalidVariableException, msg);
    }

    debug5 << "Generating \"block_nums\" variable for domain " << domain << endl;

    //
    // Construct the return variable array
    //
    int ncells = ds->GetNumberOfCells();
    vtkIntArray *bnvar = vtkIntArray::New();
    bnvar->SetNumberOfComponents(1);
    bnvar->SetNumberOfTuples(ncells);
    for (int i = 0; i < ncells; i++)
        bnvar->SetComponent(i, 0, domainToBlockGrouping[domain]);

    return bnvar;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetVar
//
//  Purpose:
//      Gets the variable and converts it to a vtkScalars object.
//
//  Arguments:
//      domain   The domain to fetch.
//      v        The name of the variable to fetch.
//
//  Returns:     The variable as a vtkScalars.
//
//  Programmer:  Hank Childs
//  Creation:    November 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, made it work with Silo objects
//    distributed across multiple files.
//
//    Hank Childs, Thu Sep 27 10:29:46 PDT 2001
//    Sent down the domain and variable name to some function calls for caching
//    purposes.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
//    Jeremy Meredith, Wed Mar 19 12:23:25 PST 2003
//    Allow for the case where a multi-var is only defined on a subset
//    of its associated multimesh.
//
//    Hank Childs, Wed Jan 14 11:58:41 PST 2004
//    Use the cached multi-var to prevent too many DBGetMultivar calls.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to convert domain id for CSG meshes; no-op for other meshes.
//    Added support for CSG variables.
//
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. In this case, the location return had to be constructed
//    and allocated. So, needed to add bool indicating that.
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Handle special case for 'Nodelists' variable
//
//    Mark C. Miller, Tue Dec 23 22:13:00 PST 2008
//    Handle special case of ANNOTATION_INT nodelists.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Tue Jul 10 20:08:37 PDT 2012
//    Add support for nodelist vars in MRG Trees.
//
//    Cyrus Harrison, Fri Aug 16 10:07:47 PDT 2013
//    Added support for nodelists placed @ /Nodelists/
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 15:03:59 PST 2016
//    Add logic to return immediately if the var is an all-empty multi-var.
//
//    Mark C. Miller, Wed Jun 15 09:22:14 PDT 2016
//    Added logic to support adding of block decomposition as a variable.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetVar(int domain, const char *v)
{
    if (emptyObjectsList.find(v) != emptyObjectsList.end()) return 0;

    // Use knowledge from MD to check if this is a label var because in that
    // case we'll actually want to use the GetXxxVectorVar routines
    bool isLabelVar = metadata->DetermineVarType(v, false) == AVT_LABEL_VAR;

    //
    // Handle possible special case of nodelists spoofed as a variable
    //
    if (string(v) == "Nodelists")
    {
        vtkDataArray *nlvar = GetNodelistsVar(domain);
        if (nlvar != 0)
            return nlvar;
    }
    else if (string(v) == "AnnotInt_Nodelists" || string(v) == "AnnotInt_Facelists")
    {
        vtkDataArray *nlvar = GetAnnotIntNodelistsVar(domain, v);
        if (nlvar != 0)
            return nlvar;
    }

    // If the variable name begings with "nodesets_" or "facesets_"...
    if (string(v).find("nodesets_") == 0 || string(v).find("facesets_") == 0)
    {
        vtkDataArray *nlvar = GetMrgTreeNodelistsVar(domain, v);
        if (nlvar != 0)
            return nlvar;
    }

    // If the variable is the block decomposition as a variable
    // domainToBlockGrouping size will be non-zero only when this feature
    // is activated.
    if (domainToBlockGrouping.size() && string(v).find("_block_nums") != string::npos)
    {
        string meshName = metadata->MeshForVar(v);
        if (string(v) == meshName + "_block_nums")
        {
            vtkDataArray *bnvar = GetBlockDecompositionAsVar(domain, meshName);
            if (bnvar) return bnvar;
        }
    }

    int localdomain = domain;
    if (blocksForMultivar.count(v))
    {
        localdomain = blocksForMultivar[v][domain];
        if (localdomain == -1)
            return NULL;
    }

    debug5 << "Reading in domain " << domain << ", variable " << v << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // Get the file handle, throw an exception if it hasn't been opened 
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *var = const_cast<char *>(v);

    //
    // Start off by finding out what kind of var it is.  Note: we have
    // already cached the multivars.  See if we have a multivar in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //

    DBmultivar *mv = NULL;
    avtSiloMultiVarCacheEntry *mv_ent = QueryMultiVar("", var);

    if(mv_ent != NULL)
        mv = mv_ent->DataObject();

    int type;
    if (mv != NULL)
        type = DB_MULTIVAR;
    else
        type = DBInqVarType(dbfile, var);

    //
    // Sort out the bad cases.
    //
    if (type != DB_UCDVAR && type != DB_QUADVAR && type != DB_POINTVAR
        && type != DB_CSGVAR && type != DB_MULTIVAR)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    string var_location = "";
    if (type == DB_MULTIVAR)
    {
        if (mv == NULL)
        {
            GetMultiVar("", var, &mv_ent);

            if(mv_ent != NULL)
                mv = mv_ent->DataObject();

        }
        if (mv == NULL)
            EXCEPTION1(InvalidVariableException, var);
        if (localdomain < 0 || localdomain >= mv->nvars)
        {
            EXCEPTION2(BadDomainException, localdomain, mv->nvars);
        }
        type = mv_ent->VarType(localdomain);
        var_location = mv_ent->GenerateName(localdomain);
    }
    else
    {
        if (domain != 0)
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        var_location = string(var);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_var;
    const char *var_dirname = FileFunctions::Dirname(var);

    DetermineFileAndDirectory(var_location.c_str(), var_dirname, domain_file, directory_var);

    //
    // We only need to worry about quadvars, ucdvars, and pointvars, since we
    // have reduced the multivar case to one of those.
    //
    vtkDataArray *rv = NULL;
    if (type == DB_UCDVAR)
    {
        if (isLabelVar)
            rv = GetUcdVectorVar(domain_file, directory_var.c_str(), v, domain);
        else
            rv = GetUcdVar(domain_file, directory_var.c_str(), v, domain);
    }
    else if (type == DB_QUADVAR)
    {
        if (isLabelVar)
            rv = GetQuadVectorVar(domain_file, directory_var.c_str(), v, domain);
        else
            rv = GetQuadVar(domain_file, directory_var.c_str(), v, domain);
    }
    else if (type == DB_POINTVAR)
    {
        if (isLabelVar)
            rv = GetPointVectorVar(domain_file, directory_var.c_str());
        else
            rv = GetPointVar(domain_file, directory_var.c_str());
    }
    else if (type == DB_CSGVAR)
    {
        if (isLabelVar)
            rv = GetCsgVectorVar(domain_file, directory_var.c_str());
        else
            rv = GetCsgVar(domain_file, directory_var.c_str());
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetVectorVar
//
//  Purpose:
//      Gets the variable and converts it to a vtkVectors object.
//
//  Arguments:
//      domain   The domain to fetch.
//      v        The name of the variable to fetch.
//
//  Returns:     The variable as a vtkVectors.
//
//  Programmer:  Hank Childs
//  Creation:    March 20, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkVectors has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Hank Childs, Fri May 17 14:59:39 PDT 2002
//    Made the routine a front-end to ones that retrieve standard vector
//    vars and defvars.
//
//    Hank Childs, Thu Apr 10 09:11:08 PDT 2003
//    Force the defvars to get read in if they haven't been already.
//
//    Hank Childs, Thu Aug 14 09:18:13 PDT 2003
//    Removed all defvar support -- it is now handled correctly at a different
//    level.
//
//    Hank Childs, Wed Jan 14 12:04:19 PST 2004
//    Remove redundant DBGetMultivar calls.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to convert domain id for CSG meshes; no-op for other meshes.
//    Added support for CSG variables.
//
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. In this case, the location return had to be constructed
//    and allocated. So, needed to add bool indicating that.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 15:03:59 PST 2016
//    Add logic to return immediately if the var is an all-empty multi-var.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetVectorVar(int domain, const char *v)
{
    debug5 << "Reading in vector variable " << v << ", domain " << domain
           << endl;

    if (emptyObjectsList.find(v) != emptyObjectsList.end()) return 0;

    int localdomain = domain;
    if (blocksForMultivar.count(v))
    {
        localdomain = blocksForMultivar[v][domain];
        if (localdomain == -1)
            return NULL;
    }

    //
    // Get the file handle, throw an exception if it hasn't already been opened
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *var = const_cast<char *>(v);

    //
    // Start off by finding out what kind of var it is.  Note: we have
    // already cached the multivars.  See if we have a multivar in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //
    DBmultivar *mv = NULL;
    avtSiloMultiVarCacheEntry *mv_ent = QueryMultiVar("", var);

    if(mv_ent != NULL)
        mv = mv_ent->DataObject();

    int type;
    if (mv != NULL)
        type = DB_MULTIVAR;
    else
        type = DBInqVarType(dbfile, var);

    //
    // Sort out the bad cases.
    //
    if (type != DB_UCDVAR && type != DB_QUADVAR && type != DB_POINTVAR
        && type != DB_MULTIVAR)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    string var_location = "";
    if (type == DB_MULTIVAR)
    {
        if (mv == NULL)
        {
            GetMultiVar("", var, &mv_ent); 
            if(mv_ent != NULL)
                mv = mv_ent->DataObject();
        }
        if (mv == NULL)
            EXCEPTION1(InvalidVariableException, var);
        if (localdomain < 0 || localdomain >= mv->nvars)
        {
            EXCEPTION2(BadDomainException, localdomain, mv->nvars);
        }
        type = mv_ent->VarType(localdomain);
        var_location = mv_ent->GenerateName(localdomain);
    }
    else
    {
        if (domain != 0)
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        var_location = string(var);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_var;
    const char *var_dirname = FileFunctions::Dirname(var);

    DetermineFileAndDirectory(var_location.c_str(),var_dirname, domain_file, directory_var);

    //
    // We only need to worry about quadvars, ucdvars, and pointvars, since we
    // have reduced the multivar case to one of those.
    //
    vtkDataArray *rv = NULL;
    if (type == DB_UCDVAR)
    {
        rv = GetUcdVectorVar(domain_file, directory_var.c_str(), v, domain);
    }
    else if (type == DB_QUADVAR)
    {
        rv = GetQuadVectorVar(domain_file, directory_var.c_str(), v, domain);
    }
    else if (type == DB_POINTVAR)
    {
        rv = GetPointVectorVar(domain_file, directory_var.c_str());
    }
    else if (type == DB_CSGVAR)
    {
        rv = GetCsgVectorVar(domain_file, directory_var.c_str());
    }

    return rv;
}

// ****************************************************************************
// Method: CopyAndPadUcdVar
//
// Purpose: 
//   Copies data from a ucdvar into a new vtkDataArray.
//
// Arguments:
//
// Returns:    
//
// Note:       I moved this code from GetUcdVar and I templated it.
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  7 10:19:52 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Mon Oct 19 20:23:08 PDT 2009
//    Replaced skipping logic (old way) with remapping logic for arb.
//    polyhedral meshes.
//
//    Mark C. Miller, Tue Oct 20 16:51:06 PDT 2009
//    Made it static.
//
//    Mark C. Miller, Wed Oct 28 20:29:23 PDT 2009
//    Added logic to handle vector variables too. If input ucdvar has 2
//    components, it gets 'promoted' to 3 by inserting zeros for 3rd component.
//    Otherwise, output vtkDataArray will have same number of components
//    as input ucdvar.
//
//    Mark C. Miller, Wed Aug 13 21:37:44 PDT 2014
//    Add support for re-mapping and zero padding of symmetric tensor
//    variables too.
// ****************************************************************************

template <typename T, typename Tarr>
static vtkDataArray *
CopyAndPadUcdVar(const DBucdvar *uv, const vector<int> &remap, avtVarType vtype = AVT_UNKNOWN_TYPE)
{
    int const X = -1; // Place holder for zero'd components 
    int const j_remap_2d[9] = {0,2,X,2,1,X,X,X,X}; // map 2d symm. tensor comp Voigt order to VTK
    int const j_remap_3d[9] = {0,5,4,5,1,3,4,3,2}; // map 3d symm. tensor comp Voigt order to VTK
    size_t i;
    int j, k, n, cnt;
    T *ptr = 0;
    Tarr *vtkvar = Tarr::New();

    //
    // Handle remapping data to due zones that have been decomposed. 
    //
    int nvtkcomps = uv->nvals;
    if (vtype == AVT_UNKNOWN_TYPE && uv->nvals == 2)
        nvtkcomps = 3;
    else if (vtype == AVT_VECTOR_VAR)
        nvtkcomps = 3; // zero pad extra components
    else if (vtype == AVT_SYMMETRIC_TENSOR_VAR && (uv->nvals == 3 || uv->nvals == 6))
        nvtkcomps = 9; // fill and/or zero pad extra components
    vtkvar->SetNumberOfComponents(nvtkcomps);
    if (remap.size() > 0)
    {
        if (uv->centering == DB_ZONECENT)
        {
            vtkvar->SetNumberOfTuples(remap.size());
            ptr = (T *) vtkvar->GetVoidPointer(0);
            if (vtype == AVT_SYMMETRIC_TENSOR_VAR)
            {
                int const *j_remap = uv->nvals==3?j_remap_2d:j_remap_3d;
                for (i = 0; i < remap.size(); i++)
                {
                    for (j = 0; j < nvtkcomps; j++)
                    {
                        if (j_remap[j] < 0)
                            ptr[i*nvtkcomps+j] = ((T)0);
                        else
                            ptr[i*nvtkcomps+j] = ((T**)(uv->vals))[j_remap[j]][remap[i]];
                    }
                }
            }
            else
            {
                for (i = 0; i < remap.size(); i++)
                {
                    for (j = 0; j < nvtkcomps; j++)
                    {
                        if (j < uv->nvals)
                            ptr[i*nvtkcomps+j] = ((T**)(uv->vals))[j][remap[i]];
                        else
                            ptr[i*nvtkcomps+j] = ((T)0);
                    }
                }
            }
        }
        else if (uv->centering == DB_NODECENT)
        {
            //
            // Determine # of 'extra' nodes
            //
            n = 0;
            i = 0;
            while (i < remap.size())
            {
                cnt = remap[i++];
                for (j = 0; j < cnt; j++)
                    i++;
                n++;
            }

            //
            // Add data from original nodes.
            //
            vtkvar->SetNumberOfTuples(uv->nels+n);
            ptr = (T *) vtkvar->GetVoidPointer(0);
            for (i = 0; i < (size_t)uv->nels; i++)
            {
                for (j = 0; j < nvtkcomps; j++)
                {
                    if (j < uv->nvals)
                        ptr[i*nvtkcomps+j] = ((T**)(uv->vals))[j][i];
                    else
                        ptr[i*nvtkcomps+j] = ((T)0);
                }
            }

            //
            // Remap data on 'extra' nodes. Note, this sum/average
            // is almost certainly not appropriate for all variables.
            // I think we need to know which are 'intensive' and which
            // are 'extensive'. Silo still needs to be enhanced for
            // that.
            //
            n = uv->nels;
            i = 0;
            while (i < remap.size())
            {
                double sum = 0.0;
                cnt = remap[i++];
                for (j = 0; j < nvtkcomps; j++)
                {
                    if (j < uv->nvals)
                    {
                        size_t itmp;
                        for (k = 0, itmp = i; k < cnt; k++, itmp++)
                            sum += (double) ((T**)(uv->vals))[j][remap[itmp]];
                    }
                    else
                    {
                        sum = (double) 0;
                    }
                    ptr[n*nvtkcomps+j] = (T) (sum / cnt);
                }
                i+=cnt;
                n++;
            }
        }
    }
    else
    {
        //
        // Populate the variable as we normally would.
        //
        vtkvar->SetNumberOfTuples(uv->nels);
        ptr = (T *) vtkvar->GetVoidPointer(0);
        if (vtype == AVT_SYMMETRIC_TENSOR_VAR)
        {
            int const *j_remap = uv->nvals==3?j_remap_2d:j_remap_3d;
            for (i = 0; i < (size_t)uv->nels; i++)
            {
                for (j = 0; j < nvtkcomps; j++)
                {
                    if (j_remap[j] < 0)
                        ptr[i*nvtkcomps+j] = ((T)0);
                    else
                        ptr[i*nvtkcomps+j] = ((T**)(uv->vals))[j_remap[j]][i];
                }
            }
        }
        else
        {
            for (i = 0; i < (size_t)uv->nels; i++)
            {
                for (j = 0; j < nvtkcomps; j++)
                {
                    if (j < uv->nvals)
                        ptr[i*nvtkcomps+j] = ((T**)(uv->vals))[j][i];
                    else
                        ptr[i*nvtkcomps+j] = ((T)0);
                }
            }
        }
    }

    return vtkvar;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetUcdVectorVar
//
//  Purpose:
//      Gets a single block of a vector ucdvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//      tvn        The True Variable Name.
//      domain     The domain that this variable belongs to.
//
//  Returns:       A vtkDataArray of the vector ucdvar.
//
//  Programmer:    Hank Childs
//  Creation:      May 17, 2002
//
//  Modifications:
//    Eric Brugger, Tue Mar  2 16:50:47 PST 2004
//    I modified the routine to use nvals as the number of components in
//    the variable.
//
//    Mark C. Miller, Thu Oct 21 22:11:28 PDT 2004
//    Added code to remove values from the array for arb. zones that have
//    been removed
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Mark C. Miller, Wed Oct 28 20:42:14 PDT 2009
//    Replaced arb. polyhederal zone skipping logic with real remapping
//    as now Silo plugin will read and decompose arb. polyhedral mesh.
//    Replaced CopyUcdVectorVar a vector-enhanced version of CopyAndPadUcdVar.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
//
//    Mark C. Miller, Sun Aug 29 23:19:50 PDT 2010
//    Added logic to expand (material) subsetted variables.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetUcdVectorVar(DBfile *dbfile, const char *vname,
                                   const char *tvn, int domain)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBucdvar  *uv = DBGetUcdvar(dbfile, varname);
    if (uv == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        return 0;
    }

    //
    // If the region_pnames option is set, this variable is defined
    // on only those subsets of the mesh listed there. So, expand
    // it out to behave like a 'full' variable here and now.
    //
    if (uv->region_pnames)
        ExpandUcdvar(uv, vname, tvn, domain);

    string meshName = metadata->MeshForVar(tvn);
    const bool DoExpressions = true;
    avtVarType vtype = metadata->DetermineVarType(tvn, !DoExpressions);
    vector<int> noremap;
    vector<int>* remap = &noremap;
    map<string, map<int, vector<int>* > >::iterator domit;
    if (uv->centering == DB_ZONECENT &&
        (domit = arbMeshCellReMap.find(meshName)) != arbMeshCellReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        remap = domit->second[domain];
    }
    else if (uv->centering == DB_NODECENT &&
        (domit = arbMeshNodeReMap.find(meshName)) != arbMeshNodeReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        remap = domit->second[domain];
    }

    vtkDataArray *vectors = 0;
    if(uv->datatype == DB_DOUBLE)
        vectors = CopyAndPadUcdVar<double,vtkDoubleArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_FLOAT)
        vectors = CopyAndPadUcdVar<float,vtkFloatArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_LONG_LONG)
        vectors = CopyAndPadUcdVar<long long,vtkLongLongArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_LONG)
        vectors = CopyAndPadUcdVar<long,vtkLongArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_INT)
        vectors = CopyAndPadUcdVar<int,vtkIntArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_SHORT)
        vectors = CopyAndPadUcdVar<short,vtkShortArray>(uv, *remap, vtype);
    else if(uv->datatype == DB_CHAR)
        vectors = CopyAndPadUcdVar<char,vtkCharArray>(uv, *remap, vtype);

    DBFreeUcdvar(uv);

    return vectors;
}

// ****************************************************************************
// Method: CopyAndPadPointOrQuadVectorVar
//
// Purpose: 
//   Copy Silo point data into a vtkDataArray.
//
// Arguments:
//   mv : The DBmeshvar object that contains the data.
//
// Returns:    A vtkDataArray of the appropriate type.
//
// Note:       I took this code body from GetPointVectorVar and templated it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug  6 15:12:29 PDT 2009
//
// Modifications:
//
//    Mark C. Miller, Tue Oct 20 16:50:41 PDT 2009
//    Made it static.
//   
//    Kathleen Bonnell, Thu May  6 15:36:11 PDT 2010
//    Fix error in vector dimensionality test.
//
//    Mark C. Miller, Wed Aug 13 22:20:07 PDT 2014
//    Add support for tensor variables. Re-factor code for quad and point
//    vars to this one template method.
// ****************************************************************************

template <typename T, typename Tarr, typename DBvar>
static vtkDataArray *
CopyAndPadPointOrQuadVectorVar(const DBvar *mv, avtVarType vtype)
{
    int const X = -1; // Place holder for zero'd components 
    int const j_remap_2d[9] = {0,2,X,2,1,X,X,X,X}; // map 2d symm. tensor comp Voigt order to VTK
    int const j_remap_3d[9] = {0,5,4,5,1,3,4,3,2}; // map 3d symm. tensor comp Voigt order to VTK
    size_t i;
    int j, nvtkcomps = mv->nvals;
    Tarr *vectors = Tarr::New();

    if (vtype == AVT_UNKNOWN_TYPE && mv->nvals == 2)
        nvtkcomps = 3;
    else if (vtype == AVT_VECTOR_VAR)
        nvtkcomps = 3; // zero pad extra components
    else if (vtype == AVT_SYMMETRIC_TENSOR_VAR && (mv->nvals == 3 || mv->nvals == 6))
        nvtkcomps = 9; // fill and/or zero pad extra components
    vectors->SetNumberOfComponents(nvtkcomps);
    vectors->SetNumberOfTuples(mv->nels);
    T *ptr = vectors->GetPointer(0);

    if (vtype == AVT_SYMMETRIC_TENSOR_VAR)
    {
        int const *j_remap = mv->nvals==3?j_remap_2d:j_remap_3d;
        for (i = 0; i < (size_t)mv->nels; i++)
        {
            for (j = 0; j < nvtkcomps; j++)
            {
                if (j_remap[j] < 0)
                    ptr[i*nvtkcomps+j] = ((T)0);
                else
                    ptr[i*nvtkcomps+j] = ((T**)(mv->vals))[j_remap[j]][i];
            }
        }
    }
    else
    {
        for (i = 0; i < (size_t)mv->nels; i++)
        {
            for (j = 0; j < nvtkcomps; j++)
            {
                if (j < mv->nvals)
                    ptr[i*nvtkcomps+j] = ((T**)(mv->vals))[j][i];
                else
                    ptr[i*nvtkcomps+j] = ((T)0);
            }
        }
    }

    return vectors;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetQuadVectorVar
//
//  Purpose:
//      Gets a single block of a vector quadvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//      tvn        The True Variable Name.
//      domain     The domain that this variable belongs to.
//
//  Returns:       A vtkDataArray of the vector quadvar.
//
//  Programmer:    Hank Childs
//  Creation:      May 17, 2002
//
//  Modifications:
//    Eric Brugger, Tue Mar  2 16:50:47 PST 2004
//    I modified the routine to use nvals as the number of components in
//    the variable.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added support for double precision quad vars (for testing xform mngr)
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetQuadVectorVar(DBfile *dbfile, const char *vname,
                                    const char *tvn, int domain)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBquadvar  *qv = DBGetQuadvar(dbfile, varname);
    if (qv == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    else if (DBIsEmptyQuadvar(qv))
    {
        DBFreeQuadvar(qv);
        return NULL;
    }

    const bool DoExpressions = true;
    avtVarType vtype = metadata->DetermineVarType(tvn, !DoExpressions);

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *vectors = 0;
    if(qv->datatype == DB_DOUBLE)
        vectors = CopyAndPadPointOrQuadVectorVar<double,vtkDoubleArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_FLOAT)
        vectors = CopyAndPadPointOrQuadVectorVar<float,vtkFloatArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_LONG_LONG)
        vectors = CopyAndPadPointOrQuadVectorVar<long long,vtkLongLongArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_LONG)
        vectors = CopyAndPadPointOrQuadVectorVar<long,vtkLongArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_INT)
        vectors = CopyAndPadPointOrQuadVectorVar<int,vtkIntArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_SHORT)
        vectors = CopyAndPadPointOrQuadVectorVar<short,vtkShortArray,DBquadvar>(qv, vtype);
    else if(qv->datatype == DB_CHAR)
        vectors = CopyAndPadPointOrQuadVectorVar<char,vtkCharArray,DBquadvar>(qv, vtype);

    DBFreeQuadvar(qv);

    return vectors;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetPointVectorVar
//
//  Purpose:
//      Gets a single block of a vector pointvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//
//  Returns:       A vtkDataArray of the vector pointvar.
//
//  Programmer:    Hank Childs
//  Creation:      Novemeber 1, 2000
//
//  Modifications:
//    Eric Brugger, Tue Mar  2 16:50:47 PST 2004
//    I modified the routine to use nvals as the number of components in
//    the variable.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Thu Aug  6 14:55:49 PDT 2009
//    I added support for non-float data types.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetPointVectorVar(DBfile *dbfile, const char *vname)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBmeshvar  *mv = DBGetPointvar(dbfile, varname);
    if (mv == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    const bool DoExpressions = true;
    avtVarType vtype = metadata->DetermineVarType(vname, !DoExpressions);

    vtkDataArray *vectors = 0;
    if(mv->datatype == DB_DOUBLE)
        vectors = CopyAndPadPointOrQuadVectorVar<double,vtkDoubleArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_FLOAT)
        vectors = CopyAndPadPointOrQuadVectorVar<float,vtkFloatArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_LONG_LONG)
        vectors = CopyAndPadPointOrQuadVectorVar<long long,vtkLongLongArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_LONG)
        vectors = CopyAndPadPointOrQuadVectorVar<long,vtkLongArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_INT)
        vectors = CopyAndPadPointOrQuadVectorVar<int,vtkIntArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_SHORT)
        vectors = CopyAndPadPointOrQuadVectorVar<short,vtkShortArray,DBmeshvar>(mv, vtype);
    else if(mv->datatype == DB_CHAR)
        vectors = CopyAndPadPointOrQuadVectorVar<char,vtkCharArray,DBmeshvar>(mv, vtype);

    DBFreeMeshvar(mv);

    return vectors;
}

vtkDataArray *
avtSiloFileFormat::GetCsgVectorVar(DBfile *dbfile, const char *vname)
{
    EXCEPTION1(InvalidVariableException, vname);
    return 0;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetMeshHelper
//
//  Purpose: Preliminary work involved in getting a mesh, refactored here to
//     support access to this functionality from multiple routines other than
//     just GetMesh().
//
//  Created: Tue Dec 23 22:17:53 PST 2008
//  Programmer: Mark C. Miller
//
//  Modifications:
//    Mark C. Miller, Wed Dec 24 01:39:04 PST 2008
//    Added this missing function pre-amble. Added missing delete of
//    meshLocation.
//
//    Jeremy Meredith, Mon Dec 29 16:57:31 EST 2008
//    Can't delete meshLocation here because it's used after the function
//    returns in most cases.  Alas, can't defer to caller, because caller
//    doesn't necessarily have a pointer to meshLocation (i.e. 
//    directory_mesh might point to meshlocation+N, so even if you
//    didn't allocate directory_mesh, you can't delete it in the caller
//    by deleting directory_mesh -- it's not the same chunk of memory
//    in all cases).  The only safe way around this is to always copy,
//    then let the caller always delete.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
// ****************************************************************************
void
avtSiloFileFormat::GetMeshHelper(int domain, const char *m, DBmultimesh **_mm,
    int *_type, DBfile **_domain_file, string &directory_mesh_out)
{
    debug5 << "Reading in domain " << domain << ", mesh " << m << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // Get the file handle, throw an exception if it hasn't already been opened
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *mesh = const_cast<char *>(m);

    //
    // Start off by finding out what kind of mesh it is.  Note: we have
    // already cached the multimeshes.  See if we have a multimesh in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //
    DBmultimesh *mm = NULL;
    avtSiloMultiMeshCacheEntry *mm_ent = QueryMultiMesh("", mesh);

    if(mm_ent != NULL)
        mm = mm_ent->DataObject();

    int type;
    if (mm != NULL)
        type = DB_MULTIMESH;
    else
        type = DBInqVarType(dbfile, mesh);

    //
    // Sort out the bad cases.
    //
    if (type != DB_UCDMESH && type != DB_QUADMESH && type != DB_QUAD_CURV &&
        type != DB_QUAD_RECT && type != DB_POINTMESH && type != DB_MULTIMESH &&
        type != DB_CSGMESH && type != DB_CURVE)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    string mesh_location;
    if (type == DB_MULTIMESH)
    {
        if (mm == NULL)
        {
            GetMultiMesh("", mesh, &mm_ent);
            if(mm_ent != NULL)
                mm = mm_ent->DataObject();
        }
        if (mm == NULL)
            EXCEPTION1(InvalidVariableException, mesh);
        if (domain < 0 || domain >= mm->nblocks)
        {
            EXCEPTION2(BadDomainException, domain, mm->nblocks);
        }
        type = mm_ent->MeshType(domain);
        mesh_location = mm_ent->GenerateName(domain);
    }
    else
    {
        if (domain != 0 && type != DB_CSGMESH)
        {
            EXCEPTION2(BadDomainException, domain, 1);
        }
        mesh_location = string(mesh);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    const char *mesh_dirname = FileFunctions::Dirname(mesh);
    DetermineFileAndDirectory(mesh_location.c_str(), mesh_dirname, domain_file, directory_mesh_out);

    if (_mm) *_mm = mm;
    if (_type) *_type = type;
    if (_domain_file) *_domain_file = domain_file;

}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh and converts it to a vtkDataSet object.
//
//  Arguments:
//      domain   The domain to fetch.
//      m        The name of the mesh to fetch.
//
//  Returns:     The mesh as a vtkDataSet.
//
//  Programmer:  Hank Childs
//  Creation:    November 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, made it work with Silo objects
//    distributed across multiple files.
//
//    Hank Childs, Wed Jan 14 11:20:18 PST 2004
//    Make use of cached multimeshes if available.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Kathleen Bonnell, Tue Feb  8 17:00:46 PST 2005 
//    Added domain to args for GetQuadMesh. 
//
//    Mark C. Miller, Mon Feb 14 20:28:47 PST 2005
//    Added test for DB_QUAD_CURV/RECT for valid type
//
//    Mark C. Miller, Thu Mar  2 00:03:40 PST 2006
//    Added support for curves
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code convert domain id for CSG meshes; no-op for other meshes.
//
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. In this case, the location return had to be constructed
//    and allocated. So, needed to add bool indicating that.
//
//    Mark C. Miller, Tue Nov 18 18:11:56 PST 2008
//    Added support for mesh region grouping trees being used to spedify
//    AMR representation in Silo.
//
//    Mark C. Miller, Mon Nov  9 10:42:13 PST 2009
//    Added dontForceSingle to call to BuildDomainAux... for AMR cases.
//
//    Cyrus Harrison, Fri Mar 26 09:07:59 PDT 2010
//    Resolve mesh type before constructing amr domain boundries.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Brad Whitlock, Thu Sep 13 16:15:43 PDT 2012
//    Pass domain to GetPointMesh.
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
//    Mark C. Miller, Tue Feb  2 15:03:59 PST 2016
//    Add logic to return immediately if the mesh is an all-empty multi-mesh.
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetMesh(int domain, const char *m)
{ 
    int type;
    string directory_mesh;
    DBmultimesh *mm;
    DBfile *dbfile = GetFile(tocIndex);
    DBfile *domain_file = dbfile;

    if (emptyObjectsList.find(m) != emptyObjectsList.end()) return 0;

    GetMeshHelper(domain, m, &mm, &type, &domain_file, directory_mesh);

    //
    // We only need to worry about quadmeshes, ucdmeshes, and pointmeshes,
    // since we have reduced the multimesh case to one of those.
    //
    vtkDataSet *rv = NULL;
    if (type == DB_UCDMESH)
    {
        rv = GetUnstructuredMesh(domain_file, directory_mesh.c_str(), domain, m);
    }
    else if (type==DB_QUADMESH || type==DB_QUAD_RECT || type==DB_QUAD_CURV)
    {

        // We need to wait to create domain boundries until after we obtain
        // the actual mesh so we know if it is rectilinear or curvilinear.

        rv = GetQuadMesh(domain_file, directory_mesh.c_str(), domain);

        // DB_QUADMESH won't cut it for the call to BuildDomainAuxiliaryInfoForAMRMeshes
        // we need DB_QUAD_RECT or DB_QUAD_CURV to be able to create the proper domain
        // boundries.
        //
        // Resolve this using the type of the newly created mesh.
        //
        if (rv != NULL && metadata->GetMesh(m)->meshType == AVT_AMR_MESH)
        {
            int true_type = type;
            if(type == DB_QUADMESH)
            {
                if(rv->IsA("vtkRectilinearGrid"))
                    true_type  = DB_QUAD_RECT;
                else
                    true_type  = DB_QUAD_CURV;
            }

            BuildDomainAuxiliaryInfoForAMRMeshes(dbfile, mm, m, timestep,
                                                 true_type, cache,
                                                 forceSingle);
        }
    }
    else if (type == DB_POINTMESH)
    {
        rv = GetPointMesh(domain_file, directory_mesh.c_str(), domain);
    }
    else if (type == DB_CSGMESH)
    {
        rv = GetCSGMesh(domain_file, directory_mesh.c_str(), domain);
    }
    else if (type == DB_CURVE)
    {
        rv = GetCurve(domain_file, directory_mesh.c_str());
    }
    else
    {
        EXCEPTION0(ImproperUseException);
    }

    return rv;
}

// ****************************************************************************
// Function: CreateDataArray
//
// Purpose: 
//   Creates a vtkDataArray suitable for the given Silo type. Also return the
//   size of 1 element.
//
// Arguments:
//   silotype : The Silo type.
//   sz       : The size of 1 element.
//
// Returns:    A suitable vtkDataArray instance.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  7 10:39:35 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Jan 12 17:49:18 PST 2010
//    Made it it able to push data into the resultant vtkDataArray. Added
//    support for DB_LONG_LONG.
// ****************************************************************************

static vtkDataArray *
CreateDataArray(int silotype, void *data, int numvals)
{ 
    vtkDataArray *da = 0;
    switch(silotype)
    {
    case DB_DOUBLE:
    {
        vtkDoubleArray *d = vtkDoubleArray::New();
        if (data) d->SetArray((double*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_INT:
    {
        vtkIntArray *d = vtkIntArray::New();
        if (data) d->SetArray((int*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_LONG:
    {
        vtkLongArray *d = vtkLongArray::New();
        if (data) d->SetArray((long*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_SHORT:
    {
        vtkShortArray *d = vtkShortArray::New();
        if (data) d->SetArray((short*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_CHAR:
    {
        vtkCharArray *d = vtkCharArray::New();
        if (data) d->SetArray((char*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_LONG_LONG:
    {
        vtkLongLongArray *d = vtkLongLongArray::New();
        if (data) d->SetArray((long long*)data, numvals, 0);
        da = d;
        break;
    }
    case DB_FLOAT:
    default:
    {
        vtkFloatArray *d = vtkFloatArray::New();
        if (data) d->SetArray((float*)data, numvals, 0);
        da = d;
        break;
    }
    }
    if (data) da->SetNumberOfComponents(1);

    return da;
}

// ****************************************************************************
// Method: ConvertToFloat
//
// Purpose: 
//   Converts the input array to float, returning a new float array that must
//   be freed by the caller. The exception is if the input was already float.
//   In that case, the input array is returned unmodified.
//
// Arguments:
//   silotype : The type of data stored in the data array.
//   data     : The data array.
//   nels     : The number of elements in the data array.
//
// Returns:    A float array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug  7 10:51:03 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

static float *
ConvertToFloat(int silotype, void *data, int nels)
{ 
    float *retval = 0;

    if (!data) return 0;
    if (nels <= 0) return 0;
    
    switch(silotype)
    {
    case DB_DOUBLE:
        { const double *ptr = (const double *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_INT:
        { const int *ptr = (const int *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_LONG_LONG:
        { const long long *ptr = (const long long *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_LONG:
        { const long *ptr = (const long *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_SHORT:
        { const short *ptr = (const short *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_CHAR:
        { const char *ptr = (const char *)data;
        retval = new float[nels];
        for(int i = 0; i < nels; ++i)
            retval[i] = (float)ptr[i];
        }
        break;
    case DB_FLOAT:
        retval = (float*)data;
        break;
    default:
        break;
    }

    return retval;
}

// ****************************************************************************
// Function: TraverseMaterialForSubsettedUcdvar
//
// Purpose: Help expand a subsetted ucdvar defined on (some) materials to
// a 'full' ucdvar. Templated to deal with all of Silo's datatypes.
//
// Created: Mark C. Miller, Sun Aug 29 23:18:32 PDT 2010
//
// Modifications:
//    Mark C. Miller, Wed Sep  1 13:06:03 PDT 2010
//    Added support for node-centered variables, which require the associated
//    mesh's zonelist to perform the traversal.
//
//    Mark C. Miller, Thu Sep  2 21:06:06 PDT 2010
//    Replace new with malloc. Doh? We're using the data to populate a DBucdvar
//    which is later going to be free'd by a DBFreeUcdvar() which assumes 
//    it was malloc'd.
//
//    Mark C. Miller, Wed Sep  8 14:09:33 PDT 2010
//    Fix initialization of vals arrays to go over nnodes or nzones depending
//    on whether ugrid is present.
//
//    Mark C. Miller, Tue Jun 21 11:03:03 PDT 2011
//    Fixed indexing problem with use of 'nvals++' inside of loops over
//    components.
// ****************************************************************************

template <typename T>
static void
TraverseMaterialForSubsettedUcdvar(const DBucdvar *const uv,
    avtMaterial *mat, vtkUnstructuredGrid *ugrid,
    const intVector& restrictToMats,
    void ***_newvals, void ***_newmixvals)
{
    size_t i, j;
    int nzones = mat->GetNZones();
    int nnodes = ugrid?ugrid->GetNumberOfPoints():0;
    T **newvals = (T**) malloc((uv->nvals)*sizeof(T*));
    for (i = 0; i < (size_t)uv->nvals; i++)
    {
        newvals[i] = (T*) malloc((ugrid?nnodes:nzones)*sizeof(T));
        // Initialize the array to be same as zero entry.
        for (j = 0; j < (size_t)(ugrid?nnodes:nzones); j++)
            newvals[i][j] = ((T**)uv->vals)[i][0];
    }
    T **newmixvals = 0;
    if (mat->GetMixlen() > 0)
    {
        newmixvals = (T**) malloc((uv->nvals)*sizeof(T*));
        for (i = 0; i < (size_t)uv->nvals; i++)
        {
            newmixvals[i] = (T*) malloc((mat->GetMixlen())*sizeof(T));
            // Initialize the array to be same as zero entry.
            if (uv->mixvals)
            {
                for (j = 0; j < (size_t)mat->GetMixlen(); j++)
                    newmixvals[i][j] = ((T**)uv->mixvals)[i][0]; 
            }
            else
            {
                for (j = 0; j < (size_t)mat->GetMixlen(); j++)
                    newmixvals[i][j] = ((T**)uv->vals)[i][0]; 
            }
        }
    }
    map<vtkIdType,bool> haveVisitedPoint;
    int nvals = 0;
    int nmixvals = 0;
    for (i = 0; i < (size_t)nzones; i++)
    {
        if (mat->GetMatlist()[i] >= 0) // clean case
        {
            bool is_selected = false;
            for (j = 0; j < restrictToMats.size() && !is_selected; j++)
                if (mat->GetMatlist()[i] == restrictToMats[j]) is_selected = true;
            if (is_selected)
            {
                if (ugrid) // node-centered case
                {
                    vtkCell *cell = ugrid->GetCell(i);
                    for (j = 0; j < (size_t)cell->GetNumberOfPoints(); j++)
                    {
                        vtkIdType ptId = cell->GetPointId(j);
                        if (haveVisitedPoint.find(ptId) == haveVisitedPoint.end())
                        {
                            haveVisitedPoint[ptId] = true;
                            for (size_t k = 0; k < (size_t)uv->nvals; k++)
                                newvals[k][(int)ptId] = ((T**)uv->vals)[k][nvals];
                            nvals++;
                        }
                    }
                }
                else // zone-centered case
                {
                    for (j = 0; j < (size_t)uv->nvals; j++)
                        newvals[j][i] = ((T**)uv->vals)[j][nvals];
                    nvals++;
                }
            }
        }
        else // mixed case
        {
            int mix_idx = -(mat->GetMatlist()[i]) - 1;
            while (mix_idx >= 0)
            {
                bool is_selected = false;
                for (j = 0; j < restrictToMats.size() && !is_selected; j++)
                    if (mat->GetMixMat()[mix_idx] == restrictToMats[j]) is_selected = true;
                if (is_selected)
                {
                    if (ugrid) // node-centered case
                    {
                        if (restrictToMats.size() == 1) // single material optimization
                        {
                            vtkCell *cell = ugrid->GetCell(i);
                            for (j = 0; j < (size_t)cell->GetNumberOfPoints(); j++)
                            {
                                vtkIdType ptId = cell->GetPointId(j);
                                if (haveVisitedPoint.find(ptId) == haveVisitedPoint.end())
                                {
                                    haveVisitedPoint[ptId] = true;
                                    for (int k = 0; k < uv->nvals; k++)
                                        newvals[k][(int)ptId] = ((T**)uv->vals)[k][nvals];
                                    nvals++;
                                }
                            }
                        }
                        else // multiple material case
                        {
                            vtkCell *cell = ugrid->GetCell(i);
                            for (j = 0; j < (size_t)cell->GetNumberOfPoints(); j++)
                            {
                                vtkIdType ptId = cell->GetPointId(j);
                                if (haveVisitedPoint.find(ptId) == haveVisitedPoint.end())
                                {
                                    haveVisitedPoint[ptId] = true;
                                    for (int k = 0; k < uv->nvals; k++)
                                        newmixvals[k][(int)ptId] = ((T**)uv->mixvals)[k][nmixvals];
                                    nmixvals++;
                                }
                            }
                        }
                    }
                    else // zone-centered case
                    {
                        if (restrictToMats.size() == 1) // single material optimization
                        {
                            for (j = 0; j < (size_t)uv->nvals; j++)
                                newmixvals[j][mix_idx] = ((T**)uv->vals)[j][nvals];
                            nvals++;
                        }
                        else // multiple material case
                        {
                            for (j = 0; j < (size_t)uv->nvals; j++)
                                newmixvals[j][mix_idx] = ((T**)uv->mixvals)[j][nmixvals];
                            nmixvals++;
                        }
                    }
                }
                mix_idx = mat->GetMixNext()[mix_idx]-1;
            }
        }
    }
    *_newvals = (void**) newvals;
    *_newmixvals = (void**) newmixvals;
}

// ****************************************************************************
// Method: ExpandUcdvar
//
// Purpose: Expand a subsetted ucdvar to a 'full' ucdvar object by traversing
// associated material object.
//
// Created: Mark C. Miller, Sun Aug 29 23:18:32 PDT 2010
//
// Modifications:
//    Mark C. Miller, Wed Sep  1 13:06:03 PDT 2010
//    Added support for node-centered variables, which require the associated
//    mesh's zonelist to perform the traversal.
// ****************************************************************************
void
avtSiloFileFormat::ExpandUcdvar(DBucdvar *uv,  
    const char *vname, const char *tvn, int domain)
{
    // Obtain the avtMaterial object associated with the mesh this
    // variable is defined on.
    string meshName = metadata->MeshForVar(tvn);
    string matObjectName = metadata->MaterialOnMesh(meshName);
    avtMaterial *mat;

    // First, see if material object is already cached
    void_ref_ptr vr = cache->GetVoidRef(matObjectName.c_str(),
        AUXILIARY_DATA_MATERIAL, timestep, domain);
    mat = (avtMaterial*) *vr;

    // If we don't have it, read it and cache it
    if (!mat)
    {
        DestructorFunction df;
        void *p = GetAuxiliaryData(matObjectName.c_str(), domain,
            AUXILIARY_DATA_MATERIAL, (void*)0, df);
        mat = (avtMaterial*) p;
        void_ref_ptr vrtmp = void_ref_ptr(p, df);
        cache->CacheVoidRef(matObjectName.c_str(), AUXILIARY_DATA_MATERIAL,
            timestep, domain, vrtmp);
    }

    // If we still don't have it, we're in trouble
    if (!mat)
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Unable to obtain material object \"%s\" required to"
            "compute subsets upon which the variable \"%s\" is defined.",
            matObjectName.c_str(), vname);
        EXCEPTION1(InvalidVariableException, msg);
    }

    // For node centered variables, we also need the mesh's zonelist.
    // Because VisIt's generic db always issues the associated GetMesh()
    // BEFORE issuing GetVar(), we can safely assume the mesh must be in
    // the cache and look ONLY there for it. Failure to obtain it is a 
    // problem we cannot recover from.
    vtkUnstructuredGrid  *ugrid = 0;
    if (uv->centering == DB_NODECENT)
    {
        vtkDataSet *ds = (vtkDataSet *) cache->GetVTKObject(meshName.c_str(),
                                            avtVariableCache::DATASET_NAME,
                                            timestep, domain, "_all");
        if (ds == 0)
        {
            char msg[256];
            SNPRINTF(msg, sizeof(msg), "Cannot find cached mesh \"%s\" for domain %d to "
                "traverse subsetted node-centered variable, \"%s\"", meshName.c_str(),
                 domain, vname);
            EXCEPTION1(ImproperUseException, msg);
        }

        ugrid = vtkUnstructuredGrid::SafeDownCast(ds);
    }

    vector<int> restrictToMats;
    if (!GetRestrictedMaterialIndices(metadata, vname, meshName.c_str(),
        uv->region_pnames, &restrictToMats))
    {
        char msg[256];
        SNPRINTF(msg, sizeof(msg), "Unable to determine material indices "
            "variable \"%s\" is restricted to", vname); 
        EXCEPTION1(InvalidVariableException, msg);
    }

    int i, nzones = mat->GetNZones(), nnodes = ugrid?ugrid->GetNumberOfPoints():0;
    void **newvals=0, **newmixvals=0;
    if (uv->datatype == DB_DOUBLE)
        TraverseMaterialForSubsettedUcdvar<double>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_FLOAT)
        TraverseMaterialForSubsettedUcdvar<float>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_INT)
        TraverseMaterialForSubsettedUcdvar<int>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_CHAR)
        TraverseMaterialForSubsettedUcdvar<char>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_SHORT)
        TraverseMaterialForSubsettedUcdvar<short>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_LONG)
        TraverseMaterialForSubsettedUcdvar<long>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);
    else if (uv->datatype == DB_LONG_LONG)
        TraverseMaterialForSubsettedUcdvar<long long>(uv, mat, ugrid, restrictToMats,
            &newvals, &newmixvals);

    // Overwite DBucdvar's old contents with stuff we just expanded.
    for (i = 0; i < uv->nvals; i++)
        free(uv->vals[i]);
    free(uv->vals);
    uv->nels = ugrid?nnodes:nzones;
    uv->vals = newvals;
    if (uv->mixvals)
    {
        for (i = 0; i < uv->nvals; i++)
            free(uv->mixvals[i]);
        free(uv->mixvals);
    }
    uv->mixlen = mat->GetMixlen();
    uv->mixvals = newmixvals;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetUcdVar
//
//  Purpose:
//      Gets a single block of a ucdvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//      tvn        The True Variable Name.
//      domain     The domain that this variable belongs to.
//
//  Returns:       A vtkScalars of the ucdvar.
//
//  Programmer:    Hank Childs
//  Creation:      Novmeber 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Hank Childs, Thu Apr 26 16:09:39 PDT 2001
//    Sped up routine.
//
//    Hank Childs, Thu Sep 27 10:21:30 PDT 2001
//    Add support for mixed variables.
//
//    Hank Childs, Tue Oct 16 07:53:35 PDT 2001
//    Account for funny data files that write out all NULL mixval arrays.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Hank Childs, Fri Jul  5 15:03:23 PDT 2002
//    Add the name of the mixed variable to its constructor.
//
//    Mark C. Miller, Thu Oct 21 22:11:28 PDT 2004
//    Added code to remove values from array for arb. zones that were
//    removed from the mesh
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Fri Aug  7 10:19:32 PDT 2009
//    I added support for non-float types.
//
//    Mark C. Miller, Mon Oct 19 20:25:08 PDT 2009
//    Replaced skipping logic with remapping logic.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
//
//    Mark C. Miller, Sun Aug 29 23:19:50 PDT 2010
//    Added logic to expand (material) subsetted variables.
//
//    Mark C. Miller, Tue Sep  7 22:57:28 PDT 2010
//    Just as for meshes, with vars defined on subsets, we need to accomodate
//    possible EMPTY domains.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetUcdVar(DBfile *dbfile, const char *vname,
                             const char *tvn, int domain)
{
    if (string(vname) == "EMPTY")
        return 0;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBucdvar  *uv = DBGetUcdvar(dbfile, varname);
    if (uv == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, varname);
        }
        return 0;
    }
    else if (DBIsEmptyUcdvar(uv))
    {
        DBFreeUcdvar(uv);
        return NULL;
    }


    //
    // If the region_pnames option is set, this variable is defined
    // on only those subsets of the mesh listed there. So, expand
    // it out to behave like a 'full' variable here and now.
    //
    if (uv->region_pnames)
        ExpandUcdvar(uv, vname, tvn, domain);

    string meshName = metadata->MeshForVar(tvn);
    vector<int> noremap;
    vector<int>* remap = &noremap;
    map<string, map<int, vector<int>* > >::iterator domit;
    if (uv->centering == DB_ZONECENT &&
        (domit = arbMeshCellReMap.find(meshName)) != arbMeshCellReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        remap = domit->second[domain];
    }
    else if (uv->centering == DB_NODECENT &&
        (domit = arbMeshNodeReMap.find(meshName)) != arbMeshNodeReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        remap = domit->second[domain];
    }

    vtkDataArray *scalars = 0;
    if(uv->datatype == DB_DOUBLE)
        scalars = CopyAndPadUcdVar<double,vtkDoubleArray>(uv, *remap);
    else if(uv->datatype == DB_FLOAT)
        scalars = CopyAndPadUcdVar<float,vtkFloatArray>(uv, *remap);
    else if(uv->datatype == DB_LONG_LONG)
        scalars = CopyAndPadUcdVar<long long,vtkLongLongArray>(uv, *remap);
    else if(uv->datatype == DB_LONG)
        scalars = CopyAndPadUcdVar<long,vtkLongArray>(uv, *remap);
    else if(uv->datatype == DB_INT)
        scalars = CopyAndPadUcdVar<int,vtkIntArray>(uv, *remap);
    else if(uv->datatype == DB_SHORT)
        scalars = CopyAndPadUcdVar<short,vtkShortArray>(uv, *remap);
    else if(uv->datatype == DB_CHAR)
        scalars = CopyAndPadUcdVar<char,vtkCharArray>(uv, *remap);

    if (uv->mixvals != NULL && uv->mixvals[0] != NULL)
    {
        float *mixvals = ConvertToFloat(uv->datatype, uv->mixvals[0], uv->mixlen);

        avtMixedVariable *mv = new avtMixedVariable(mixvals, uv->mixlen, tvn);
        void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
        cache->CacheVoidRef(tvn, AUXILIARY_DATA_MIXED_VARIABLE, timestep, 
                            domain, vr);

        if(mixvals != (float *)uv->mixvals[0])
            delete [] mixvals;
    }

    DBFreeUcdvar(uv);

    return scalars;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetQuadVar
//
//  Purpose:
//      Gets a single block of a quadvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//      tvn        The True Variable Name.
//      domain     The domain that this variable belongs to.
//
//  Returns:       A vtkScalars of the quadvar.
//
//  Programmer:    Hank Childs
//  Creation:      Novemeber 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Hank Childs, Thu Apr 26 16:09:39 PDT 2001
//    Sped up routine.
//
//    Hank Childs, Thu Sep 27 10:21:30 PDT 2001
//    Add support for mixed variables.
//
//    Hank Childs, Tue Oct 16 07:53:35 PDT 2001
//    Account for funny data files that write out all NULL mixval arrays.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Eric Brugger, Thu May 23 14:23:23 PDT 2002
//    I modified the routine to take into account the major order of
//    the variable being read.
//
//    Hank Childs, Fri Jul  5 15:03:23 PDT 2002
//    Add the name of the mixed variable to its constructor.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added support for double precision quad vars (for testing xform mngr)
//
//    Mark C. Miller, Tue Nov 18 18:12:54 PST 2008
//    Add some additional datatypes to test behavior for non-4-byte sized
//    types.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Fri Aug  7 10:33:26 PDT 2009
//    I created more types of vtkDataArray to add support beyond float/double.
//
//    Mark C. Miller, Tue Jan 12 17:50:52 PST 2010
//    Use CreateDataArray.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

template <class T>
static void CopyAndReorderQuadVar(T* var2, int nx, int ny, int nz, const void *const src)
{
    const T *const var  = (const T *const) src;
    int nxy = nx * ny;
    int nyz = ny * nz;

    for (int k = 0; k < nz; k++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int i = 0; i < nx; i++)
            {
                var2[k*nxy + j*nx + i] = var[k + j*nz + i*nyz];
            }
        }
    }
}

vtkDataArray *
avtSiloFileFormat::GetQuadVar(DBfile *dbfile, const char *vname,
                              const char *tvn, int domain)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBquadvar  *qv = DBGetQuadvar(dbfile, varname);
    if (qv == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    else if (DBIsEmptyQuadvar(qv))
    {
        DBFreeQuadvar(qv);
        return NULL;
    }

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *scalars;
    if (qv->major_order == DB_ROWMAJOR || qv->ndims <= 1)
    {
        scalars = CreateDataArray(qv->datatype, (void*) qv->vals[0], qv->nels);
        qv->vals[0] = 0; // the vtkDataArray now owns the data.
    }
    else
    {
        scalars = CreateDataArray(qv->datatype, 0, 0);
        scalars->SetNumberOfTuples(qv->nels);
        void *var2 = scalars->GetVoidPointer(0);
        void *var  = qv->vals[0];

        int nx = qv->dims[0];
        int ny = qv->dims[1];
        int nz = qv->ndims == 3 ? qv->dims[2] : 1;
        if (qv->datatype == DB_DOUBLE)
            CopyAndReorderQuadVar((double *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_LONG)
            CopyAndReorderQuadVar((long *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_LONG_LONG)
            CopyAndReorderQuadVar((long long *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_LONG)
            CopyAndReorderQuadVar((long *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_INT)
            CopyAndReorderQuadVar((int *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_SHORT)
            CopyAndReorderQuadVar((short *) var2, nx, ny, nz, var);
        else if (qv->datatype == DB_CHAR)
            CopyAndReorderQuadVar((char *) var2, nx, ny, nz, var);
        else
            CopyAndReorderQuadVar((float *) var2, nx, ny, nz, var);
    }

    if (qv->mixvals != NULL && qv->mixvals[0] != NULL)
    {
        float *mixvals = ConvertToFloat(qv->datatype, qv->mixvals[0], qv->mixlen);

        avtMixedVariable *mv = new avtMixedVariable(mixvals, qv->mixlen, tvn);
        void_ref_ptr vr = void_ref_ptr(mv, avtMixedVariable::Destruct);
        cache->CacheVoidRef(tvn, AUXILIARY_DATA_MIXED_VARIABLE, timestep, 
                            domain, vr);

        if(mixvals != (float*)qv->mixvals[0])
            delete [] mixvals;
    }

    DBFreeQuadvar(qv);

    return scalars;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetPointVar
//
//  Purpose:
//      Gets a single block of a pointvar.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      vname      The variable name.  This may also have a prepended path.
//
//  Returns:       A vtkScalars of the pointvar.
//
//  Programmer:    Hank Childs
//  Creation:      Novemeber 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Hank Childs, Thu Apr 26 16:09:39 PDT 2001
//    Sped up routine.
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Brad Whitlock, Fri Aug  7 10:38:34 PDT 2009
//    I added support for non-float data types.
//
//    Mark C. Miller, Thu Nov 12 14:56:15 PST 2009
//    Changed logic for exception for variable with more than 1 component
//    to use 'nvals' instead of 'ndims'
//
//    Mark C. Miller, Tue Jan 12 17:51:42 PST 2010
//    Changed interface to CreateDataArray.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetPointVar(DBfile *dbfile, const char *vname)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBmeshvar  *mv = DBGetPointvar(dbfile, varname);
    if (mv == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    else if (DBIsEmptyPointvar(mv))
    {
        DBFreePointvar(mv);
        return NULL;
    }

    if(mv->nvals > 1)
    {
        EXCEPTION1(InvalidVariableException, "Pointvar with more than 1 component. Fix Silo reader.");
    }

    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *scalars = CreateDataArray(mv->datatype, (void*)mv->vals[0], mv->nels);
    mv->vals[0] = 0; // vtkDataArray now owns the data.
    DBFreeMeshvar(mv);

    return scalars;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetCsgVar
//
//  Purpose: Gets a CSG variable
//
//  Programmer: Mark C. Miller
//  Creation:   December 3, 2006 
//
//  Modifications:
//    Brad Whitlock, Fri Aug  7 11:01:59 PDT 2009
//    I added non-float support.
//
//    Mark C. Miller, Tue Jan 12 17:51:59 PST 2010
//    Changed interface to CreateDataArray.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetCsgVar(DBfile *dbfile, const char *vname)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *varname  = const_cast<char *>(vname);

    //
    // Get the Silo construct.
    //
    DBcsgvar *csgv = DBGetCsgvar(dbfile, varname);
    if (csgv == NULL)
    {
        EXCEPTION1(InvalidVariableException, varname);
    }
    else if (DBIsEmptyCsgvar(csgv))
    {
        DBFreeCsgvar(csgv);
        return NULL;
    }


    //
    // Populate the variable.  This assumes it is a scalar variable.
    //
    vtkDataArray *scalars = CreateDataArray(csgv->datatype, (void*)csgv->vals[0], csgv->nels); 
    csgv->vals[0] = 0; // vtkDataArray no owns the data.
    DBFreeCsgvar(csgv);

    return scalars;
}

// ****************************************************************************
// Method: CopyUnstructuredMeshCoordinates
//
// Purpose: 
//   This function copies ucdmesh coordinates into an interleaved vtkPoints array.
//
// Arguments:
//   T : The destination array.
//   um : The ucdmesh.
//
// Returns:    
//
// Note:       I moved this code from avtSiloFileFormat::GetUnstructuredMesh
//             and templated it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug  6 11:59:25 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Oct 20 16:51:50 PDT 2009
//    Made it static.
// ****************************************************************************

template <typename T>
static void
CopyUnstructuredMeshCoordinates(T *pts, const DBucdmesh *um)
{
    int nnodes = um->nnodes;
    bool dim3 = (um->coords[2] != NULL ? true : false); (void) dim3;
    T *tmp = pts;
    const T *coords0 = (T*) um->coords[0];
    const T *coords1 = (T*) um->coords[1];
    if (um->coords[2] != NULL)
    {
        const T *coords2 = (T*) um->coords[2];
        for (int i = 0 ; i < nnodes ; i++)
        {
            *tmp++ = *coords0++;
            *tmp++ = *coords1++;
            *tmp++ = *coords2++;
        }
    }
    else if (um->coords[1] != NULL)
    {
        for (int i = 0 ; i < nnodes ; i++)
        {
            *tmp++ = *coords0++;
            *tmp++ = *coords1++;
            *tmp++ = 0.;
        }
    }
    else if (um->coords[0] != NULL)
    {
        for (int i = 0 ; i < nnodes ; i++)
        {
            *tmp++ = *coords0++;
            *tmp++ = 0.;
            *tmp++ = 0.;
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::HandleGlobalZoneIds
//
//  Purpose: Handle re-mapping global zone ids as per arb. poly remeshing
//      and sticking global zone ids into cache.
//
//  Programmer: Mark C. Miller, Thu Oct 29 15:00:54 PDT 2009
//
//    Mark C. Miller, Fri Oct 30 14:03:13 PDT 2009
//    Handle Silo's DB_DTPTR configuration option.
//
//    Mark C. Miller, Tue Jan 12 17:52:15 PST 2010
//    Handle different version of Silo interface. Added support for all of
//    Silo's integral types and not just DB_INT.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

void
avtSiloFileFormat::HandleGlobalZoneIds(const char *meshname, int domain,
    void *gzoneno, int lgzoneno, int gnznodtype)
{
    //
    // Lookup an arb. poly remap data we might need.
    //
    vector<int> noremap;
    vector<int> *remap = &noremap;
    map<string, map<int, vector<int>* > >::iterator domit;
    if ((domit = arbMeshCellReMap.find(meshname)) != arbMeshCellReMap.end() &&
         domit->second.find(domain) != domit->second.end())
        remap = domit->second[domain];

    //
    // Create a vtkInt array whose contents are the actual gzoneno data
    // Create a temp. DBucdvar object so we can use CopyAndPadUcdVar
    //
    DBucdvar tmp;
    tmp.centering = DB_ZONECENT;
    tmp.datatype = DB_INT;
    tmp.datatype = gnznodtype;
    tmp.nels = lgzoneno;
    tmp.nvals = 1;
    tmp.vals = (void**) malloc(sizeof(void*)); /* use malloc because this is a Silo object */
    tmp.vals[0] = (void*) gzoneno; 

    vtkDataArray *arr = NULL;
    if (tmp.datatype == DB_SHORT)
        arr = CopyAndPadUcdVar<short,vtkShortArray>(&tmp, *remap);
    else if (tmp.datatype == DB_INT)
        arr = CopyAndPadUcdVar<int,vtkIntArray>(&tmp, *remap);
    else if (tmp.datatype == DB_LONG)
        arr = CopyAndPadUcdVar<long,vtkLongArray>(&tmp, *remap);
    else if (tmp.datatype == DB_LONG_LONG)
        arr = CopyAndPadUcdVar<long long,vtkLongLongArray>(&tmp, *remap);
    free(tmp.vals);

    //
    // Cache this VTK object but in the VoidRefCache, not the VTK cache
    // so that it can be obtained through the GetAuxiliaryData call
    //
    void_ref_ptr vr = void_ref_ptr(arr, avtVariableCache::DestructVTKObject);
    cache->CacheVoidRef(meshname, AUXILIARY_DATA_GLOBAL_ZONE_IDS, timestep, 
        domain, vr);
}

// ****************************************************************************
//  Method: avtSiloFileFormat::RemapFacelistForPolyhedronZones
//
//  Purpose:
//      Correct the zoneno array in a facelist where the associated
//      zonelist has polyhedral elements that will get split up into zoo
//      elements.
//
//  Arguments:
//      sfl       The Silo facelist object to correct.
//      szl       The Silo zonelist object associated with the facelist.
//
//  Programmer:   Eric Brugger
//  Creation:     May 27, 2010
//
//  Modifications:
//      Eric Brugger, Fri May 28 12:11:37 PDT 2010
//      I added a test to check if either the facelist or zonelist are NULL
//      and return if so.  Either one can be NULL and it is still a valid
//      Silo unstructured mesh.
//
//    Mark C. Miller, Tue Jul 10 20:12:33 PDT 2012
//    Made this a static function in the source file since it doesn't use
//    any class members.
//
//    Mark C. Miller, Thu Apr 10 17:52:45 PDT 2014
//    Fixed logic for updating inewzoneno. It was either incorrect to begin
//    with or inconsistent with newer algorithm for decomposing an arbitrary
//    polyhedron. The new algorithm decomposes a zone such that a given face
//    of more than 4 nodes is 'stripped' into a sequence of quads only or 
//    quads followed by a triangle. For example, an arb-poly face of 6 nodes
//    is decomposed into 2 quads. A face of 7 nodes is decomposed into 2 quads
//    and one triangle. A face of 8 nodes is decomposed into 3 quads. A face
//    of 9 nodes is decomposed into 3 quads and a triangle.
// ****************************************************************************

static void
RemapFacelistForPolyhedronZones(DBfacelist *sfl, DBzonelist *szl)
{
    //
    // If either the facelist or zonelist is NULL, return.  Either one
    // can be NULL and it i s a valid mesh.
    //
    if (sfl == 0 || szl == 0)
        return;

    //
    // Check if the facelist needs remapping and if not return.
    //
    bool needsRemapping = false;
    for (int i = 0 ; i < szl->nshapes ; i++)
    {
        if (szl->shapetype[i] == DB_ZONETYPE_POLYHEDRON)
        {
            needsRemapping = true;
            break;
        }
    }
    if (!needsRemapping)
        return;

    //
    // Form the remap array for the normal zones.
    //
    int nfaces = sfl->nfaces;
    int *zoneno = sfl->zoneno;

    int nzones = szl->nzones;
    int *remap = new int[nzones];

    int iremap = 0, inewzoneno = 0;
    for (int i = 0 ; i < szl->nshapes ; i++)
    {
        const int shapecnt = szl->shapecnt[i];
        const int shapesize = szl->shapesize[i]; (void) shapesize;
        const int shapetype = szl->shapetype[i];
        if (shapetype != DB_ZONETYPE_POLYHEDRON)
        {
            for (int j = 0; j < shapecnt; j++)
            {
                remap[iremap++] = inewzoneno;
                inewzoneno++;
            }
        }
        else
        {
            iremap += shapecnt;
        }
    }

    //
    // Form the remap array for the polyhedral zones.
    //
    int izonelist = 0;
    int *zonelist = szl->nodelist;
    iremap = 0;
    for (int i = 0 ; i < szl->nshapes ; i++)
    {
        const int shapecnt = szl->shapecnt[i];
        const int shapesize = szl->shapesize[i];
        const int shapetype = szl->shapetype[i];
        if (shapetype == DB_ZONETYPE_POLYHEDRON)
        {
            for (int j = 0; j < shapecnt; j++)
            {
                remap[iremap++] = inewzoneno;

                const int nfaces = zonelist[izonelist++];
                for (int k = 0; k < nfaces; k++)
                {
                    const int nnodes = zonelist[izonelist];
                    izonelist += nnodes + 1;
                    if (nnodes < 5)
                        inewzoneno++; // face is a single tri or quad
                    else
                        inewzoneno += (1 + (nnodes-4)/2 + (nnodes-4)%2); // face is 1+ quads & 0|1 tri
                }
            }
        }
        else
        {
            izonelist += shapecnt * shapesize;
            iremap += shapecnt;
        }
    }

    //
    // Remap the zoneno array.
    //
    int *zoneno2 = new int[nfaces];

    for (int i = 0; i < nfaces; i++)
    {
         zoneno2[i] = remap[zoneno[i]];
    }
    for (int i = 0; i < nfaces; i++)
    {
         zoneno[i] = zoneno2[i];
    }

    delete [] remap;
    delete [] zoneno2;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetUnstructuredMesh
//
//  Purpose:
//      Creates a VTK unstructured grid from a Silo unstructured mesh.
//
//  Arguments:
//      dbfile     A handle to the file this variable lives in.
//      mn         The name of the mesh.
//      domain     The domain we are operating on.
//      mesh       The unqualified name of the mesh (for caching purposes).
//
//  Returns:      A VTK unstructured grid created from mn.
//
//  Programmer:   Hank Childs
//  Creation:     November 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Hank Childs, Thu Apr 26 16:09:39 PDT 2001
//    Sped up routine.
//
//    Kathleen Bonnell, Wed May 23 15:41:14 PDT 2001 
//    Added call to TranslateSiloPyramidToVTKPyramid. 
//
//    Kathleen Bonnell, Tue Jun 12 14:34:02 PDT 2001 
//    Added code to determine and set a ghost-levels array.
//
//    Hank Childs, Thu Sep 20 16:08:21 PDT 2001
//    Added argument domain.  Captured facelist and added that to the variable
//    cache.
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001
//    VTK 4.0 API requires use of vtkIdType or vtkIdList for InsertNextCell
//    methods, so created the arrays where necessary.
//
//    Hank Childs, Mon Apr 15 14:38:31 PDT 2002
//    Fix memory leak.
//
//    Hank Childs, Fri May 17 14:27:18 PDT 2002
//    Some Silo 2D-UCD meshes have facelists.  They are worthless to us and
//    they break the code since they don't have shapesizes and shapetypes, so
//    don't read them in.
//
//    Hank Childs, Sun Jun 16 19:55:52 PDT 2002
//    Account for non 0-origin meshes.
//
//    Hank Childs, Thu Jun 20 14:39:09 PDT 2002
//    Separate reading the connectivity into its own routine.  Also account
//    for meshes that do not have connectivity, but have facelists to serve
//    that role in its place.
//
//    Hank Childs, Mon Dec  9 13:21:13 PST 2002
//    Use the facelist's origin rather than the unstructured meshes.
//
//    Hank Childs, Wed Jan 14 13:40:33 PST 2004
//    Sped up routine by minimizing cache misses.
//
//    Mark C. Miller, August 9, 2004
//    Added code to read and cache optional global node and zone ids
//
//    Mark C. Miller, Thu Oct 21 22:11:28 PDT 2004
//    Added code to set arbMeshZoneRangesToSkip and issue warning for meshes
//    that have arbitrary polyhedra embedded in an ordinary DBzonelist
//
//    Hank Childs, Wed Mar  9 07:53:16 PST 2005
//    Do not send a C-array into a C++-construct.  It will eventually call 
//    "delete" when it should call "free".
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Mark C. Miller, Wed Feb 11 23:22:37 PST 2009
//    Added support for 1D unstructured mesh, mainly to test funky curves
//
//    Brad Whitlock, Thu Aug  6 12:00:39 PDT 2009
//    I added support for double coordinates.
//
//    Mark C. Miller, Mon Oct 19 20:22:27 PDT 2009
//    Added support to read in arbitrary polyhedral data.
//
//    Mark C. Miller, Wed Oct 28 08:21:14 PDT 2009
//    Added logic to deal with global zone numbers on arb. poly mesh.
//
//    Mark C. Miller, Wed Oct 28 20:43:45 PDT 2009
//    Removed logic/warning for skipping arb. polyhedral zones that are
//    embedded in 'ordinary' zonelist. They are now correctly handled in
//    ReadInConnectivity.
//
//    Mark C. Miller, Tue Jan 12 17:53:17 PST 2010
//    Use CreateDataArray for global node numbers and handle long long case
//    as well. Vary interface to HandleGlobalZoneIds for versions of Silo. 
//
//    Eric Brugger, Thu May 27 16:00:03 PDT 2010
//    I added a call to a newly written method that remaps the zoneno
//    array of a Silo facelist object when the zonelist associated with the
//    facelist has polyhedral elements.
//
//    Mark C. Miller, Wed Jul 11 10:41:56 PDT 2012
//    Changed interface to ReadInConnectivity to support repeated calls to
//    ReadInArbConnectivity for different segments of same zonelist.
//
//    Kathleen Biagas, Wed Sep 17 09:56:05 PDT 2014
//    Create avtOriginaNodeNumbers array when nodes added (eg arb poly).
//
//    Mark C. Miller, Wed Feb 11 17:06:02 PST 2015
//    Made it return a point mesh for a DBucdmesh with no topology defined.
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetUnstructuredMesh(DBfile *dbfile, const char *mn,
                                       int domain, const char *mesh)
{
    //
    // Allow empty data sets
    //
    if (string(mn) == "EMPTY")
        return NULL;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *meshname  = const_cast<char *>(mn);

    //
    // Get the Silo construct.
    //
    DBucdmesh  *um = DBGetUcdmesh(dbfile, meshname);
    if (um == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, meshname);
        }
        return 0;
    }
    if (DBIsEmptyUcdmesh(um))
    {
        DBFreeUcdmesh(um);
        return NULL;
    }

    vtkPoints *points  = vtkPoints::New();

    //
    // vtkPoints assumes float data type
    //
    if (um->datatype == DB_DOUBLE)
        points->SetDataTypeToDouble();

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    points->SetNumberOfPoints(um->nnodes);
    if(um->datatype == DB_DOUBLE)
        CopyUnstructuredMeshCoordinates((double *)points->GetVoidPointer(0), um);
    else if(um->datatype == DB_FLOAT)
        CopyUnstructuredMeshCoordinates((float *)points->GetVoidPointer(0), um);
    else
    {
        points->Delete();
        DBFreeUcdmesh(um);
        EXCEPTION1(InvalidVariableException, "The Silo reader supports only "
            "float and double precision coordinates in unstructured meshes.");
    }

    // 
    // Quick check to see if this is really a point mesh 
    //
    if (um->faces == 0 && um->zones == 0 && um->edges == 0 && um->phzones == 0)
    {
        //
        // Create the VTK objects and connect them up.
        //
        vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();
        ugrid->SetPoints(points);
        ugrid->Allocate(um->nnodes);
        vtkIdType onevertex[1];
        for (int i = 0 ; i < um->nnodes; i++)
        {
            onevertex[0] = i;
            ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
        }

        //
        // If we have global node ids, set them up and cache 'em
        //
        if (um->gnodeno != NULL)
        {
            vtkDataArray *arr = CreateDataArray(um->gnznodtype, um->gnodeno, um->nnodes);
            um->gnodeno = 0; // vtkDataArray now owns the data.

            //
            // Cache this VTK object but in the VoidRefCache, not the VTK cache
            // so that it can be obtained through the GetAuxiliaryData call
            //
            void_ref_ptr vr = void_ref_ptr(arr, avtVariableCache::DestructVTKObject);
            cache->CacheVoidRef(mn, AUXILIARY_DATA_GLOBAL_NODE_IDS, timestep, domain, vr);
        }

        points->Delete();
        DBFreeUcdmesh(um);
        return ugrid;
    }
    
    //
    // We already got the facelist read in free of charge.  Let's use it.
    // This is done before constructing the connectivity because this is used
    // in place of the connectivity in some places.
    //
    avtFacelist *fl = NULL;
    if (um->faces != NULL && um->ndims == 3)
    {
        DBfacelist *sfl = um->faces;
        DBzonelist *szl = um->zones;

        RemapFacelistForPolyhedronZones(sfl, szl);

        fl = new avtFacelist(sfl->nodelist, sfl->lnodelist,
                             sfl->nshapes, sfl->shapecnt, sfl->shapesize,
                             sfl->zoneno, sfl->origin);
        void_ref_ptr vr = void_ref_ptr(fl, avtFacelist::Destruct);
        cache->CacheVoidRef(mesh, AUXILIARY_DATA_EXTERNAL_FACELIST, timestep, 
                            domain, vr);
    }

    //
    // If we have global node ids, set them up and cache 'em
    //
    if (um->gnodeno != NULL)
    {
        vtkDataArray *arr = CreateDataArray(um->gnznodtype, um->gnodeno, um->nnodes); 
        um->gnodeno = 0; // vtkDataArray now owns the data.

        //
        // Cache this VTK object but in the VoidRefCache, not the VTK cache
        // so that it can be obtained through the GetAuxiliaryData call
        //
        void_ref_ptr vr = void_ref_ptr(arr, avtVariableCache::DestructVTKObject);
        cache->CacheVoidRef(mesh, AUXILIARY_DATA_GLOBAL_NODE_IDS, timestep, 
                            domain, vr);
    }

    //
    // Some ucdmeshes uses facelists instead of zonelists.  I think this is
    // freakish behavior and should not be supported, but if there are files
    // this way then we have to honor that.
    //
    vtkDataSet *rv = NULL;
    if (um->zones != NULL)
    {
        vtkUnstructuredGrid  *ugrid = vtkUnstructuredGrid::New(); 
        ugrid->SetPoints(points);
        ReadInConnectivity(ugrid, um, mesh, domain);
        rv = ugrid;

        if (um->zones->gzoneno != NULL)
            HandleGlobalZoneIds(mesh, domain, um->zones->gzoneno,
                um->zones->nzones, um->zones->gnznodtype);
    }
    else if (fl != NULL && um->phzones == NULL)
    {
        vtkPolyData *pd = vtkPolyData::New();
        fl->CalcFacelistFromPoints(points, pd);
        rv = pd;
    }

    //
    // Handle arbitrary polyhedral zonelist
    //
    if (um->phzones != NULL)
    {
        vtkUnstructuredGrid  *ugrid = 0;
        if (rv == 0)
        {
            ugrid = vtkUnstructuredGrid::New(); 
            ugrid->SetPoints(points);
            rv = ugrid;
        }
        else
        {
            ugrid = vtkUnstructuredGrid::SafeDownCast(rv);
        }
        ReadInArbConnectivity(mesh, ugrid, um, domain);

        if (um->phzones->gzoneno != NULL)
            HandleGlobalZoneIds(mesh, domain, um->phzones->gzoneno,
                um->phzones->nzones, um->phzones->gnznodtype);
    }

    if (rv->GetNumberOfPoints() > um->nnodes)
    {
        int numCurrentNodes = rv->GetNumberOfPoints();
        vtkIntArray *origNodes = vtkIntArray::New();
        origNodes->SetName("avtOriginalNodeNumbers");
        origNodes->SetNumberOfComponents(2);
        origNodes->SetNumberOfTuples(numCurrentNodes);
        int *ptr = origNodes->GetPointer(0);
        for (int i = 0; i < um->nnodes; ++i)
        {
            *ptr = domain;
            ptr++;
            *ptr = i;
            ptr++;
        }
        int numNewNodes = numCurrentNodes - um->nnodes;
        for (int i = 0; i < numNewNodes; ++i)
        {
            *ptr = domain;
            ptr++;
            *ptr = -1;
            ptr++;
        }
        rv->GetPointData()->AddArray(origNodes);
        origNodes->Delete();
    }

    points->Delete();
    DBFreeUcdmesh(um);

    return rv;
}

// ****************************************************************************
//  Function: LookupPHZonelistFaceIdInFaceHash
//
//  Purpose: Support method for building a DBphzonelist object from a 'normal'
//  DBzonelist object by maintaining a hash of unique, canonically ordered
//  faces.
//
//  Creation: Mark C. Miller, Wed Jul 11 10:44:42 PDT 2012
//
// ****************************************************************************
static int
LookupPHZonelistFaceIdInFaceHash(const vector<int>& faceNodes,
    const vector<int>& canonicalFaceNodes,
    const map<unsigned int, vector<std::pair<int, vector<int> > > >& faceHash)
{
    unsigned int hv = BJHash::Hash((const unsigned char*) &canonicalFaceNodes[0],
        (unsigned int)canonicalFaceNodes.size()*sizeof(int), 0x0);
    map<unsigned int, vector<std::pair<int, vector<int> > > >::const_iterator it
        = faceHash.find(hv);
    if (it == faceHash.end())
        return -INT_MAX;

    // Maybe a hash collision. So, now check all entries at 'hv' key.
    for (size_t i = 0; i < it->second.size(); i++)
    {
        std::pair<int, vector<int> > p(it->second[i]);
        if (canonicalFaceNodes == p.second) return p.first;
    } 

    return -INT_MAX;
}

// ****************************************************************************
//  Function: GetPHZonelistFaceId
//
//  Purpose: Support method for building a DBphzonelist object from a 'normal'
//  DBzonelist object by building canonical face ordering and then looking it
//  and its reverse ordered (opposite normal) variant in the hash and if 
//  neither is found, adding it to the hash as a new, unique face.
//
//  Creation: Mark C. Miller, Wed Jul 11 10:44:42 PDT 2012
//
// ****************************************************************************
static int
GetPHZonelistFaceId(int nnodes, const int *const nl,
    map<unsigned int, vector<std::pair<int, vector<int> > > >& faceHash,
    vector<int>& nodecnt, vector<int>& nodelist)
{
    // Make a vector of this face's nodes
    vector<int> faceNodes;
    for (int i = 0; i < nnodes; i++)
        faceNodes.push_back(*(nl+i));

    // Two pass loop for forward and reverse node order over the face
    vector<int> faceNodesF = faceNodes;
    for (int pass = 0; pass < 2; pass++)
    {
        vector<int> canonicalFaceNodes = faceNodesF;
        if (pass == 1)
            reverse(canonicalFaceNodes.begin(), canonicalFaceNodes.end());

        // Rotate face's list of nodes so that lowest node id is first
        int lowIdx = 0;
        for (int j = 1; j < nnodes; j++)
        {
            if (canonicalFaceNodes[j] < canonicalFaceNodes[lowIdx])
                lowIdx = j;
        }
        rotate(canonicalFaceNodes.begin(),canonicalFaceNodes.begin()+lowIdx,canonicalFaceNodes.end()); 
        if (pass == 0) faceNodesF = canonicalFaceNodes;

        // Lookup the face
        int phzlFaceId = LookupPHZonelistFaceIdInFaceHash(faceNodes, canonicalFaceNodes, faceHash);
        if (phzlFaceId != -INT_MAX)
        {
            // This is where normal orientation (e.g. inward vs. outward) gets handled.
            if (pass == 0) return phzlFaceId;
            else return ~phzlFaceId;
        }
    }

    // If we get here, we didn't find the face in the faceHash.
    // So, we need to update everything for this new face.
    unsigned int hv = BJHash::Hash((const unsigned char *)&faceNodesF[0], (unsigned int)faceNodesF.size()*sizeof(int), 0x0);
    int phzlFaceId = nodecnt.size();
    faceHash[hv].push_back(std::pair<int, vector<int> >(phzlFaceId, faceNodesF));
    nodecnt.push_back((int)faceNodes.size());
    for (size_t j = 0; j < faceNodes.size(); j++)
        nodelist.push_back(faceNodes[j]);
    return phzlFaceId;
}

// ****************************************************************************
//  Function: MakePHZonelistFromZonelistArbFragment
//
//  Purpose: Create a DBphzonelist object from a fragment of an ordinary
//      zonelist defining DB_ZONETYPE_POLYHEDRON type zones.
//
//  Programmer: Mark C. Miller, Wed Oct 28 20:46:22 PDT 2009
//
//  Modifications:
//    Comlete re-write to fix numerous problems most prominent of which is
//    neglecting to build *unique* facelist. The input DBzonelist object from
//    which this is being constructed is guaranteed NOT to handle the *unique*
//    faces properly.
// ****************************************************************************

static DBphzonelist* 
MakePHZonelistFromZonelistArbFragment(const int *nl, int shapecnt)
{
    vector<int> nodecnt, nodelist, facecnt, facelist;
    map<unsigned int, vector<std::pair<int, vector<int> > > > faceHash;
    int nzones = 0;
    while (nzones < shapecnt)
    {
        int nfaces = *nl++;
        facecnt.push_back(nfaces);
        for (int i = 0; i < nfaces; i++)
        {
            int nnodes = *nl++;
            int phzlFaceId = GetPHZonelistFaceId(nnodes, nl, faceHash, nodecnt, nodelist);
            facelist.push_back(phzlFaceId);
            nl += nnodes;
        }
        nzones++;
    }

    //
    // Use malloc here instead of new since we're creating a Silo structure
    // that Silo will later expect to be able to call free on.
    //
    DBphzonelist *phzl = DBAllocPHZonelist();
    phzl->nfaces = (int)nodecnt.size();
    phzl->nodecnt = (int *) malloc(nodecnt.size() * sizeof(int));
    memcpy(phzl->nodecnt, &nodecnt[0], nodecnt.size() * sizeof(int));
    phzl->lnodelist = (int) nodelist.size();
    phzl->nodelist = (int *) malloc(nodelist.size() * sizeof(int));
    memcpy(phzl->nodelist, &nodelist[0], nodelist.size() * sizeof(int));
    phzl->nzones = shapecnt;
    phzl->facecnt = (int *) malloc(facecnt.size() * sizeof(int));
    memcpy(phzl->facecnt, &facecnt[0], facecnt.size() * sizeof(int));
    phzl->lfacelist = (int) facelist.size();
    phzl->facelist = (int *) malloc(facelist.size() * sizeof(int));
    memcpy(phzl->facelist, &facelist[0], facelist.size() * sizeof(int));
    phzl->lo_offset = 0;
    phzl->hi_offset = shapecnt-1;

    return phzl;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::ReadInConnectivity
//
//  Purpose:
//      Reads in the connectivity array.  Also creates ghost zone information.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jan 14 07:07:01 PST 2004
//    Re-wrote to eliminate virtual function calls in inner loops.
//
//    Hank Childs, Thu Jan 15 07:24:23 PST 2004
//    Accounted for quads that are stored as hexahedrons.
//
//    Hank Childs, Thu Jun 24 09:15:03 PDT 2004
//    Make sure the cell locations for quads that are stored as hexahedrons
//    is correct.
//
//    Hank Childs, Fri Aug 27 17:18:37 PDT 2004
//    Rename ghost zone arrays.
//
//    Hank Childs, Tue Jan 11 14:48:38 PST 2005
//    Translate tetrahedrons as well.
//
//    Hank Childs, Tue Sep  5 17:19:35 PDT 2006
//    Add logic for case where there really is one real zone and the rest
//    are ghost ['7279].
//
//    Hank Childs, Thu Jun  5 09:53:39 PDT 2008
//    Add support for polygons that store the shapesize as 0.
//
//    Mark C. Miller, Wed Oct 28 18:38:12 PDT 2009
//    Handle arb. polyhedra better. Now uses same logic as truly arb. zonelist
//    does.
//
//    Mark C. Miller, Fri Oct 30 14:03:13 PDT 2009
//    Handle Silo's DB_DTPTR configuration option.
//
//    Mark C. Miller, Sun Nov  1 17:51:05 PST 2009
//    Fixed off by one indexing problem for last+1...numCells loop to create
//    ghost values. Fixed missing call to set VTK array name for ghost data.
//    Fixed missing delete for gvals.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Mark C. Miller, Tue Apr 13 18:00:45 PDT 2010
//    When handling 2D arbitrary polygons, if its 3 or 4 nodes, call it a
//    triangle or quad instead of leaving it as a polygon.
//
//    Cyrus Harrison, Thu Oct 20 13:00:53 PDT 2011
//    For 2D arbitrary polygons: When we short cut the cell type to be
//    a triangle or quad, make sure we reset the effective zone type to
//    VTK_POLYGON when we encounter a cell that is *not* a triangle or
//    quad.
//
//    Mark C. Miller, Wed Jul 11 10:55:15 PDT 2012
//    Changed interface so that calls to ReadInArbConnectivity can be made
//    repeatedly for different segments of same zonelist. Also, adjusted
//    how a DBzonelist with zones of type DB_ZONETYPE_POLYHEDRON get handled
//    so that the same code used to detect zoo-type elements in DBphzonelists
//    can be used to detect them here too. Also, fixed a problem with ghost
//    zone variable in presence of arbitrary polyhedra.
//
//    Kathleen Biagas, Mon Aug 15 14:09:55 PDT 2016
//    VTK-8, API for updating GhostLevel changed.
//
// ****************************************************************************

void
avtSiloFileFormat::ReadInConnectivity(vtkUnstructuredGrid *ugrid,
    DBucdmesh *um, const char *meshname, int domain)
{
    //
    // A 'normal' zonelist in silo may contain some zones of arb. polyhedral
    // type. In the past, the logic here simply 'skipped' these zones and we
    // kept track of that skipping for zone-centered variables defined on
    // the mesh. Now, we handle the arb. polyehdral zones 'correctly'. But,
    // that is all handled here at the bottom of this function. All the
    // logic prior to that simply walks over the arb. polyhedral zones but
    // keeps track of where they occur in the zonelist so we can handle them
    // later. Note that a setting of 'vtk_zonetype' of -2 represents the
    // arb. polyhedral zonetype.
    //
    size_t   i, j, k;
    int nsdims = um->ndims; (void) nsdims;
    const DBzonelist *const zl = um->zones;
    int origin = um->zones->origin;

    //
    // Tell the ugrid how many cells it will have.
    //
    int  numCells = 0;
    int  totalSize = 0;
    const int *tmp = zl->nodelist;
    for (i = 0 ; i < (size_t)zl->nshapes ; i++)
    {
        int vtk_zonetype = SiloZoneTypeToVTKZoneType(zl->shapetype[i]);
        if (vtk_zonetype != -2)
        {
            numCells += zl->shapecnt[i];
            if (zl->shapesize[i] > 0)
            {
                totalSize += zl->shapecnt[i] * (zl->shapesize[i]+1);
                tmp += zl->shapecnt[i] * zl->shapesize[i];
            }
            else
            {
                // Some polygons representations have the shapesize be 0 and 
                // each polygons # of nodes encoded as the first point ID.
                for (j = 0 ; j < (size_t)zl->shapecnt[i] ; j++)
                {
                    totalSize += *tmp + 1;
                    tmp += *tmp+1;
                }
            }
        }
    }

    //
    // Tell the ugrid what its zones are.
    //
    int *nodelist = zl->nodelist;

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues(totalSize);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(numCells);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIdTypeArray *cellLocations = vtkIdTypeArray::New();
    cellLocations->SetNumberOfValues(numCells);
    vtkIdType *cl = cellLocations->GetPointer(0);

    int currentIndex = 0;
    int zoneIndex = 0;
    bool mustResize = false;
    vector<int> arbZoneIdxOffs;
    vector<int> arbZoneCounts;
    vector<int*> arbZoneNlOffs;
    for (i = 0 ; i < (size_t)zl->nshapes ; i++)
    {
        const int shapecnt = zl->shapecnt[i];
        const int shapesize = zl->shapesize[i];

        int vtk_zonetype = SiloZoneTypeToVTKZoneType(zl->shapetype[i]);
        int effective_vtk_zonetype = vtk_zonetype;
        int effective_shapesize = shapesize;

        if (vtk_zonetype < 0 && vtk_zonetype != -2)
        {
            EXCEPTION1(InvalidZoneTypeException, zl->shapetype[i]);
        }

        //
        // Some users store out quads as hexahedrons -- they store quad
        // (a,b,c,d) as hex (a,b,c,d,a,b,c,d).  Unfortunately, we have
        // to detect this and account for it.  I think it is safe to 
        // assume that if the first hex is that way, they all are.
        // Similarly, if the first hex is not that way, none of them are.
        //
        if (vtk_zonetype == VTK_HEXAHEDRON)
        {
            if (shapecnt > 0) // Make sure there is at least 1 hex.
            {
                if ((nodelist[0] == nodelist[4]) &&
                    (nodelist[1] == nodelist[5]) &&
                    (nodelist[2] == nodelist[6]) &&
                    (nodelist[3] == nodelist[7]))
                {
                    vtk_zonetype = -1;
                    effective_vtk_zonetype = VTK_QUAD;
                    effective_shapesize = 4;
                    mustResize = true;
                }
            }
        }

        //
        // "Handle" arbitrary polyhedra by skipping over them here.
        // We deal with them later on in this func.
        //
        if (vtk_zonetype == -2)
        {
            //
            // There are shapecnt zones of arb. type in this segment
            // of the zonelist. Record their count, indices and
            // offset into nodelist for now.
            //
            arbZoneIdxOffs.push_back(zoneIndex); 
            arbZoneCounts.push_back(shapecnt);
            arbZoneNlOffs.push_back(nodelist);
            zoneIndex += shapecnt;
            nodelist += shapesize;
        }
        else
        {
            bool tetsAreInverted = false;
            bool firstTet = true;
            for (j = 0 ; j < (size_t)shapecnt ; j++, zoneIndex++)
            {
                *cl++ = currentIndex;

                if (vtk_zonetype != VTK_WEDGE &&
                    vtk_zonetype != VTK_PYRAMID &&
                    vtk_zonetype != VTK_TETRA &&
                    vtk_zonetype != VTK_POLYGON &&
                    vtk_zonetype != -1)
                {
                    *nl++ = shapesize;
                    for (k = 0 ; k < (size_t)shapesize ; k++)
                        *nl++ = *(nodelist+k) - origin;
                }
                else if (vtk_zonetype == VTK_POLYGON)
                {
                    // Handle both forms of storing polygons:
                    //  1) having all of the polygons with the same # of nodes
                    //     in a group with a valid shapesize.
                    //  2) having the shapesize be 0 and each polygons # of nodes
                    //     encoded as the first point ID.
                    if (shapesize > 0)
                    {
                        *nl++ = shapesize;
                        for (k = 0 ; k < (size_t)shapesize ; k++)
                            *nl++ = *(nodelist+k) - origin;
                        /* correct stored cell type if its really a tri or quad */
                        if (shapesize == 3)
                            effective_vtk_zonetype = VTK_TRIANGLE;
                        else if (shapesize == 4)
                            effective_vtk_zonetype = VTK_QUAD;
                        else
                            effective_vtk_zonetype = VTK_POLYGON;
                    }
                    else
                    {
                        int ss = *nodelist;
                        *nl++ = ss;
                        for (k = 0 ; k < (size_t)ss; k++)
                            *nl++ = *(nodelist+k+1) - origin;
                        nodelist += ss+1;
                        /* correct stored cell type if its really a tri or quad */
                        if (ss == 3)
                           effective_vtk_zonetype = VTK_TRIANGLE;
                        else if (ss == 4)
                            effective_vtk_zonetype = VTK_QUAD;
                        else
                            effective_vtk_zonetype = VTK_POLYGON;
                        effective_shapesize = ss;
                    }
                }
                else if (vtk_zonetype == VTK_WEDGE)
                {
                    *nl++ = 6;

                    vtkIdType vtk_wedge[6];
                    TranslateSiloWedgeToVTKWedge(nodelist, vtk_wedge);
                    for (k = 0 ; k < 6 ; k++)
                    {
                        *nl++ = vtk_wedge[k]-origin;
                    }
                }
                else if (vtk_zonetype == VTK_PYRAMID)
                {
                    *nl++ = 5;

                    vtkIdType vtk_pyramid[5];
                    TranslateSiloPyramidToVTKPyramid(nodelist, vtk_pyramid);
                    for (k = 0 ; k < 5 ; k++)
                    {
                        *nl++ = vtk_pyramid[k]-origin;
                    }
                }
                else if (vtk_zonetype == VTK_TETRA)
                {
                    *nl++ = 4;

                    if (firstTet)
                    {
                        firstTet = false;
                        tetsAreInverted = TetIsInverted(nodelist, ugrid);
                        static bool haveIssuedWarning = false; (void) haveIssuedWarning;
                        if (tetsAreInverted)
                        {
                            haveIssuedWarning = true;
                            char msg[1024];
                            SNPRINTF(msg, sizeof(msg), "An examination of the first tet "
                                "element in this mesh indicates that the node order is "
                                "inverted from Silo's standard conventions. All tets are "
                                "being automatically re-ordered.\n"
                                "Further messages of this issue will be suppressed.");
                            avtCallback::IssueWarning(msg);
                        }
                    }

                    vtkIdType vtk_tetra[4];
                    if (tetsAreInverted)
                    {
                        for (k = 0 ; k < 4 ; k++)
                            vtk_tetra[k] = nodelist[k];
                    }
                    else
                    {
                        TranslateSiloTetrahedronToVTKTetrahedron(nodelist,
                                                                 vtk_tetra);
                    }

                    for (k = 0 ; k < 4 ; k++)
                    {
                        *nl++ = vtk_tetra[k]-origin;
                    }
                }
                else if (vtk_zonetype == -1)
                {
                    *nl++ = 4;
                    for (k = 0 ; k < 4 ; k++)
                        *nl++ = *(nodelist+k);
                }

                *ct++ = effective_vtk_zonetype;
                nodelist += shapesize;
                currentIndex += effective_shapesize+1;
            }
        }
    }

    //
    // This only comes up when somebody says they have hexahedrons, but they
    // actually have quadrilaterals.  In that case, we have allocated too much
    // memory and need to resize.  If we don't, VTK will get confused.
    //
    if (mustResize)
    {
        vtkIdTypeArray *nlist2 = vtkIdTypeArray::New();
        vtkIdType *nl_orig = nlist->GetPointer(0);
        int nvalues = nl-nl_orig;
        nlist2->SetNumberOfValues(nvalues);
        vtkIdType *nl2 = nlist2->GetPointer(0);
        memcpy(nl2, nl_orig, nvalues*sizeof(vtkIdType));
        nlist->Delete();
        nlist = nlist2;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(numCells, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    //
    // Now, deal with any arbitrary polyhedral zones we encountered above.
    //
    vector<int> *cellReMap = 0;
    map<string, map<int, vector<int>* > >::iterator domit;
    if (arbZoneIdxOffs.size())
    {
        //
        // Associate cell/node maps with the mesh so we can find and use 'em
        // in subsequent GetVar calls.
        //
        cellReMap = new vector<int>;
        if ((domit = arbMeshCellReMap.find(meshname)) != arbMeshCellReMap.end() &&
            domit->second.find(domain) != domit->second.end())
            delete domit->second[domain];
        arbMeshCellReMap[meshname][domain] = cellReMap;
        vector<int> *nodeReMap = new vector<int>;
        if ((domit = arbMeshNodeReMap.find(meshname)) != arbMeshNodeReMap.end() &&
            domit->second.find(domain) != domit->second.end())
            delete domit->second[domain];
        arbMeshNodeReMap[meshname][domain] = nodeReMap;

        //
        // Go ahead and add an empty avtOriginalCellNumbers array now.
        // We'll populate it below.
        //
        vtkUnsignedIntArray *oca = vtkUnsignedIntArray::New();
        oca->SetName("avtOriginalCellNumbers");
        oca->SetNumberOfComponents(2);
        ugrid->GetCellData()->AddArray(oca);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");

        //
        // Build up the cellReMap array for all the non-arb. zones we've
        // already put into the grid.
        //
        for (i = j = 0; i < (size_t)zl->nzones; i++)
        {
            if (j < arbZoneIdxOffs.size() &&
                i == (size_t)arbZoneIdxOffs[j])
            {
                i += arbZoneCounts[j]-1; // -1 to undue i++ of for stmt.
                j++;
                continue;
            }
            unsigned int ocdata[2] = {static_cast<unsigned int>(domain),
                                      static_cast<unsigned int>(i)};
            oca->InsertNextTypedTuple(ocdata);
            cellReMap->push_back(i);
        }
        oca->Delete();

        for (i = 0; i < arbZoneIdxOffs.size(); i++)
        {
            int *nl = arbZoneNlOffs[i];
            int gzOff = arbZoneIdxOffs[i];

            //
            // Create a temp. Silo DBphzonelist object to call ReadInArbConnectivity. 
            //
            DBphzonelist *phzl = 
                MakePHZonelistFromZonelistArbFragment(nl, arbZoneCounts[i]);

            DBphzonelist *tmpphzl = um->phzones;
            um->phzones = phzl;
            ReadInArbConnectivity(meshname, ugrid, um, domain, gzOff);
            um->phzones = tmpphzl;
            DBFreePHZonelist(phzl);

            numCells += arbZoneCounts[i];
        }

        if ((domit = arbMeshCellReMap.find(meshname)) != arbMeshCellReMap.end() &&
            domit->second.find(domain) != domit->second.end())
            cellReMap = domit->second[domain];
        else
            cellReMap = 0;
    }
    
    // --- Read Ghost Node Labels --- //
    if (um->ghost_node_labels) {
        GetUcdGhostNodesFromLabels(um, ugrid, cellReMap);
        debug5 << "GetUcdGhostNodesFromLabels" << std::endl;
    }
    
    // --- Read Ghost Zone Labels --- //
    if (um->zones->ghost_zone_labels) {
        GetUcdGhostZonesFromLabels(um->zones, ugrid, cellReMap);
        debug5 << "GetUcdGhostZonesFromLabels" << std::endl;
    } else {
    
        //
        //  Tell the ugrid which of its zones are real (avtGhostZone = 0),
        //  which are ghost (avtGhostZone = 1), but only create the ghost
        //  zones array if ghost zones are actually present.
        //
        const int first = zl->min_index;  // where the real zones start
        const int last = zl->max_index;   // where the real zones end

        if (first == 0 && last == 0  && numCells > 27)
        {
           debug5 << "Cannot tell if ghost zones are present"
                  << " because min_index & max_index are both zero!" << endl;
        }
        else if (first < 0 || first >= numCells ||
                 last  < 0 || last  >= numCells)  
        {
           // bad min or max index
           debug5 << "Invalid min/max index for determining ghost zones:  "
                  << "\n\tnumCells: " << numCells
                  << "\n\tmin_index: " << zl->min_index
                  << "\n\tmax_index: " << zl->max_index << endl;
        }
        else if (first != 0 || last != numCells -1)
        {
            //
            // We now know that ghost zones are present.
            //
            debug5 << "Creating ghost zones, real zones are indexed"
                   << " from " << first << " to " << last 
                   << " of " << numCells << " Cells." << endl;

            //
            // Populate a zone-centered array of ghost zone values.
            //
            unsigned char *gvals = new unsigned char[numCells];
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            for (i = 0; i < (size_t)first; i++)
                gvals[i] = val;
            for (i = first; i <= (size_t)last; i++)
                gvals[i] = 0;
            for (i = last+1; i < (size_t)numCells; i++)
                gvals[i] = val;

            //
            // Create a temp. DBucdvar object to call CopyAndPadUcdVar. That will handle
            // both creation of the vtkUnsignedCharArray object as well as re-mapping
            // of array as per existence of any arb. polyhedral zones.
            //
            DBucdvar tmp;
            tmp.centering = DB_ZONECENT;
            tmp.datatype = DB_CHAR;
            tmp.nels = numCells;
            tmp.nvals = 1;
            tmp.vals = (void**) malloc(sizeof(void*)); /* use malloc because this is a Silo object */
            tmp.vals[0] = (void*) gvals;
            vector<int> noremap;
            vtkDataArray *ghostZones = CopyAndPadUcdVar<unsigned char,vtkUnsignedCharArray>(&tmp,
                cellReMap?*cellReMap:noremap);
            free(tmp.vals);
            delete [] gvals;

            ghostZones->SetName("avtGhostZones");
            ugrid->GetCellData()->AddArray(ghostZones);
            ghostZones->Delete();
        }
    }
    ugrid->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
}

// ****************************************************************************
//  Function: QuadFaceIsTwisted
//
//  Purpose: Use goemetric shape to determine if a Quadrilateral is twisted.
//
//  Programmer: Mark C. Miller, Mon Jul 26 23:11:56 PDT 2010
//
//  Modifications:
//
//    Mark C. Miller, Mon Jul 26 23:13:06 PDT 2010
//    Replaced assumption of float data with GetTuple, converting all to
//    double for purposes of the computation performed here. This is necessary
//    as newer versions of the Silo plugin can serve up arbitrary type.
// ****************************************************************************

static bool
QuadFaceIsTwisted(vtkUnstructuredGrid *ugrid, int *nids)
{
    int i, j;

    //
    // initialize set of 4 points of tet
    //
    vtkDataArray *pda = ugrid->GetPoints()->GetData();
    double p[4][3];
    for (int i = 0; i < 4; i++)
        pda->GetTuple(nids[i], p[i]);

    // Walk around quad, computing inner product of two edge vectors.
    // You can have at most 2 negative inner products. If it is twisted,
    // you will have 4 negative inner products. However, there
    // is a numerical issue for a 'perfectly rectangular' quad because
    // each inner product will be near zero but randomly to either
    // side of it. So, we compare the inner product magnitude to
    // an average of the two vector magnitudes and consider the
    // inner product sign only when it is sufficiently large. There is
    // somewhat of an assumption of planarity here. However, for near
    // planar quads, the algorithm is expected to still work as the
    // off-plane components that can skew the inner product are
    // expected to be small. For very much non planar quads, this
    // algorithm can fail.
    int numNegiProds = 0;
    for (i = 0; i < 4; i++)
    {
        double dotsum = 0.0;
        double mag1sum = 0.0;
        double mag2sum = 0.0;
        for (j = 0; j < 3; j++)
        {
            double v1j = p[(i+1)%4][j] - p[(i+0)%4][j];
            double v2j = p[(i+2)%4][j] - p[(i+1)%4][j];
            mag1sum += v1j*v1j;
            mag2sum += v2j*v2j;
            dotsum += v1j * v2j;
        }
        if (dotsum < 0)
        {
            dotsum *= dotsum;
            if (mag1sum * mag2sum > 0)
            {
                if (dotsum / (mag1sum * mag2sum) > 1e-8)
                    numNegiProds++;
            }
        }
    }

    if (numNegiProds > 2)
        return true;

    return false;
}

// ****************************************************************************
//  Function: ArbInsertTriangle
//
//  Purpose: Insert a triangle element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Mon Feb 13 19:09:23 PST 2012
//
// ****************************************************************************
static void
ArbInsertTriangle(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    vtkIdType ids[3];
    ids[0] = (vtkIdType)nids[0];
    ids[1] = (vtkIdType)nids[1];
    ids[2] = (vtkIdType)nids[2];

    ugrid->InsertNextCell(VTK_TRIANGLE, 3, ids);
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertQuadrilateral
//
//  Purpose: Insert a quad element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Mon Feb 13 19:09:23 PST 2012
//
// ****************************************************************************
static void
ArbInsertQuadrilateral(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    vtkIdType ids[4];
    ids[0] = (vtkIdType)nids[0];
    ids[1] = (vtkIdType)nids[1];
    ids[2] = (vtkIdType)nids[2];
    ids[3] = (vtkIdType)nids[3];

    if (QuadFaceIsTwisted(ugrid, nids))
    {
        vtkIdType tmp = ids[2];
        ids[2] = ids[3];
        ids[3] = tmp;
    }

    ugrid->InsertNextCell(VTK_QUAD, 4, ids);
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertTet
//
//  Purpose: Insert a tet element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
// ****************************************************************************
static void
ArbInsertTet(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    // Use the 'TetIsInverted' method here to determine whether this tet
    // Is ordered correctly, relative to VTK. Note, in this context an
    // affirmative response means that the Tet is 'ok' for VTK.
    if (!TetIsInverted(nids, ugrid))
    {
        int tmp = nids[0];
        nids[0] = nids[1];
        nids[1] = tmp;
    }

    vtkIdType ids[4];
    ids[0] = (vtkIdType)nids[0];
    ids[1] = (vtkIdType)nids[1];
    ids[2] = (vtkIdType)nids[2];
    ids[3] = (vtkIdType)nids[3];

    ugrid->InsertNextCell(VTK_TETRA, 4, ids);
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertPyramid
//
//  Purpose: Insert a pyramid element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
// ****************************************************************************
static void
ArbInsertPyramid(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    // The nodes of the pyramid are given in the order that the
    // first 4 define the quad 'base' and the last node defines
    // the 'apex.'

    // Now the idea is to compute a normal to the 'base' quad face
    // and confirm the fifth point is on the correct side of that face.
    // However, we already have code that basically does that for a tet,
    // forming a plane with the first 3 nodes and then determining if
    // the 4th node is on the INward or OUTward side of that plane.
    // So, we spoof up a set of 4 nodes here to pass to that routine.
    // The first three nodes are taken from the first three nodes of
    // the base quad of the pyramid. The last node is the 5th node of the
    // pyramid.  Again, an affirmative from TetIsInverted means order
    // is ok for VTK
    int tet[4];
    tet[0] = nids[0];
    tet[1] = nids[1];
    tet[2] = nids[2];
    tet[3] = nids[4];
    if (!TetIsInverted(tet, ugrid))
    {
        // Reverse the order of the 'base' quad's nodes.
        vtkIdType ids[5];
        ids[0] = (vtkIdType)nids[3];
        ids[1] = (vtkIdType)nids[2];
        ids[2] = (vtkIdType)nids[1];
        ids[3] = (vtkIdType)nids[0];
        ids[4] = (vtkIdType)nids[4];
        ugrid->InsertNextCell(VTK_PYRAMID, 5, ids);
    }
    else
    {
        vtkIdType ids[5];
        ids[0] = (vtkIdType)nids[0];
        ids[1] = (vtkIdType)nids[1];
        ids[2] = (vtkIdType)nids[2];
        ids[3] = (vtkIdType)nids[3];
        ids[4] = (vtkIdType)nids[4];
        ugrid->InsertNextCell(VTK_PYRAMID, 5, ids);
    }
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertWedge
//
//  Purpose: Insert a wedge element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
// ****************************************************************************
static void
ArbInsertWedge(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    vtkIdType ids[6];
    ids[0] = (vtkIdType)nids[0];
    ids[1] = (vtkIdType)nids[1];
    ids[2] = (vtkIdType)nids[2];
    ids[3] = (vtkIdType)nids[3];
    ids[4] = (vtkIdType)nids[4];
    ids[5] = (vtkIdType)nids[5];

    ugrid->InsertNextCell(VTK_WEDGE, 6, ids);
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertHex
//
//  Purpose: Insert a hex element from the arbitrary connectivity.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
// ****************************************************************************
static void
ArbInsertHex(vtkUnstructuredGrid *ugrid, int *nids, unsigned int ocdata[2],
    vector<int> *cellReMap)
{
    vtkIdType ids[8];
    ids[0] = (vtkIdType)nids[0];
    ids[1] = (vtkIdType)nids[1];
    ids[2] = (vtkIdType)nids[2];
    ids[3] = (vtkIdType)nids[3];
    ids[4] = (vtkIdType)nids[4];
    ids[5] = (vtkIdType)nids[5];
    ids[6] = (vtkIdType)nids[6];
    ids[7] = (vtkIdType)nids[7];

    ugrid->InsertNextCell(VTK_HEXAHEDRON, 8, ids);
    vtkUnsignedIntArray *oca = vtkUnsignedIntArray::SafeDownCast(
        ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"));
    oca->InsertNextTypedTuple(ocdata);
    cellReMap->push_back(ocdata[1]);
}

// ****************************************************************************
//  Function: ArbInsertArbitrary
//
//  Purpose: Insert a truly 'arbitrary' zone into the ucdmesh object by
//  decomposing it into zoo-type pyramid and tet cells. We do this by first
//  computing a cell center.  Then, for each face, find the node of lowest
//  global node number. Start walking around the face from there in two
//  directions; one up around the 'top' and the other down around the
//  'bottom.' Using either 4 or 3 nodes on the face along with the newly
//  created cell center node (created above), define either pyramid or tet
//  elements. If the number of nodes around the face is even, we'll have
//  only pyramids. If it is odd, we'll have one tet, too. Otherwise, we'll
//  have only pyramids. By starting on the node of lowest global node number,
//  we ensure that a given face is always decomposed the same way by both
//  zones that share the face. We repeat this process for each face of the
//  arbitrary zone.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
//  Modifications:
//    Mark C. Miller, Wed Oct 21 03:48:24 PDT 2009
//    Fixed setting of 'mingn' when a new min is found.
//
//    Mark C. Miller, Wed Oct 28 20:47:40 PDT 2009
//    Adjusted interface to take DBphzonelist* as second arg instead of 
//    DBucdmesh*.
// ****************************************************************************
static void
ArbInsertArbitrary(vtkUnstructuredGrid *ugrid, int nsdims, DBphzonelist *phzl, int gz,
    const vector<int> &nloffs, const vector<int> &floffs, unsigned int ocdata[2],
    vector<int> *cellReMap, vector<int> *nodeReMap)
{
    //
    // Compute cell center and insert it into the ugrid
    //
    bool allFacesAre2NodeEdges = true;
    double coord_sum[3] = {0.0, 0.0, 0.0};
    int fcnt;
    if (phzl->nzones == 0)
        fcnt = phzl->nodecnt[gz];
    else
        fcnt = phzl->facecnt[gz];
    vector<int> lnmingnvec;
    map<int,int> nodemap;
    int ncnttot = 0, lf, k;
    for (lf = 0; lf < fcnt; lf++)           // lf = local face #
    {
        int lnmingn;
        int mingn = INT_MAX;
        int flidx, sgf;
        if (phzl->nzones == 0)
        {
            flidx = nloffs[gz]+lf;
            sgf = gz;
        }
        else
        {
            flidx = floffs[gz]+lf;
            sgf = phzl->facelist[flidx];
        }
        int gf = sgf < 0 ? ~sgf : sgf;      // gf = global face #
        int ncnt = phzl->nodecnt[gf];       // ncnt = # nodes for this face
        if (ncnt != 2) allFacesAre2NodeEdges = false;
        for (int ln = 0; ln < ncnt; ln++)   // ln = local node # 
        {
            int nlidx = nloffs[gf]+ln;      // nlidx = index into nodelist
            int gn = phzl->nodelist[nlidx]; // gn = global node #
            if (gn < mingn)
            {
                mingn = gn;
                lnmingn = ln;
            }
            if (nodemap.find(gn) != nodemap.end())
                continue;
            nodemap[gn] = 1;
            for (k = 0; k < 3; k++)
                coord_sum[k] += ugrid->GetPoints()->GetPoint(gn)[k];
            ncnttot++;
        }
        lnmingnvec.push_back(lnmingn);
    }
    for (k = 0; k < 3; k++)
        coord_sum[k] /= ncnttot;
    int cmidn = ugrid->GetPoints()->InsertNextPoint(coord_sum);
    nodeReMap->push_back((int)nodemap.size());
    for (map<int,int>::iterator it = nodemap.begin(); it != nodemap.end(); it++)
        nodeReMap->push_back(it->first);

    if (phzl->nzones && !allFacesAre2NodeEdges)
    {
        //
        // Loop over faces, creating pyramid and tets using 4 or
        // 3 nodes on the face and the cell center.
        //
        for (lf = 0; lf < fcnt; lf++)               // lf = local face #
        {
            int flidx = floffs[gz]+lf;              // flidx = index into facelist
            int sgf = phzl->facelist[flidx];        // sgf = signed global face #
            int gf = sgf < 0 ? ~sgf : sgf;          // gf = global face #
            int ncnt = phzl->nodecnt[gf];           // ncnt = # nodes for this face
            int newcellcnt = ncnt / 2 - ((ncnt%2)?0:1);
            int lnmingn = lnmingnvec[lf];

            int toplast = (lnmingn==ncnt-1)?0:lnmingn+1;
            int botlast = lnmingn;
            int topcur =  (toplast==ncnt-1)?0:toplast+1;
            int botcur =  (botlast==0)?ncnt-1:botlast-1;

            int nloff = nloffs[gf];
            for (int c = 0; c < newcellcnt; c++)
            {
                if (c == newcellcnt-1 && ncnt%2)
                {
                    int nids[4];
                    nids[0] =  phzl->nodelist[nloff+botcur];
                    nids[1] =  phzl->nodelist[nloff+botlast];
                    nids[2] =  phzl->nodelist[nloff+toplast];
                    nids[3] = cmidn;
                    ArbInsertTet(ugrid, nids, ocdata, cellReMap);
                }
                else
                {
                    int nids[5];
                    nids[0] =  phzl->nodelist[nloff+botcur];
                    nids[1] =  phzl->nodelist[nloff+botlast];
                    nids[2] =  phzl->nodelist[nloff+toplast];
                    nids[3] =  phzl->nodelist[nloff+topcur];
                    nids[4] = cmidn;
                    ArbInsertPyramid(ugrid, nids, ocdata, cellReMap);
                }

                toplast = topcur;
                topcur = (topcur==ncnt-1)?0:topcur+1;
                botlast = botcur;
                botcur = (botcur==0)?ncnt-1:botcur-1;
            } 
        }
    }
    else
    {
        //
        // Loop over edges, creating triangles using 2 nodes on the 
        // edge and the cell center.
        //
        for (lf = 0; lf < fcnt; lf++)
        {
            int nloff;
            if (phzl->facecnt == 0)
            {
                nloff = nloffs[gz]+lf;
            }
            else
            {
                int flidx = floffs[gz]+lf;
                int sgf = phzl->facelist[flidx];
                int gf = sgf < 0 ? ~sgf : sgf;
                nloff = nloffs[gf];
            }
            int nids[3];
            nids[0] =  phzl->nodelist[nloff+0];
            if (phzl->facecnt == 0)
                nids[1] =  phzl->nodelist[(lf==fcnt-1)?nloffs[gz]:nloff+1];
            else
                nids[1] =  phzl->nodelist[nloff+1];
            nids[2] = cmidn;
            ArbInsertTriangle(ugrid, nids, ocdata, cellReMap);
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::ReadInArbConnectivity
//
//  Purpose: Read arbitrary connectivity, smartly. That is, recognize elements
//  stored here, using arbitrary connectivity that are really the known zoo
//  type elements and then treat them as such. Otherwise, for all other zones,
//  decompose them into collections of pyramids and tets by adding a single
//  new node at the cell centers and 'fanning out' pyramids and tets for each
//  face around the zone to fill in the volume of the zone.
//
//  Programmer: Mark C. Miller, Wed Oct  7 11:24:34 PDT 2009
//
//    Mark C. Miller, Wed Oct 28 08:20:41 PDT 2009
//    Added logic to manage ghost zone information through the remap process.
//
//    Mark C. Miller, Fri Oct 30 14:03:13 PDT 2009
//    Handle Silo's DB_DTPTR configuration option.
//
//    Mark C. Miller, Sun Nov  1 17:51:05 PST 2009
//    Fixed off by one indexing problem for hi_off+1...numCells loop to create
//    ghost values. Fixed missing delete for gvals.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Mark C. Miller, Mon Mar 29 17:28:33 PDT 2010
//    Fixed cut-n-paste error where arbMeshCellReMap entry was NOT erased
//    at the end of this routine if no new points were inserted.
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Support proper face order for negative index case.
//
//    Mark C. Miller, Fri Feb 10 12:47:22 PST 2012
//    Several fixes to deal with zoo-type elements properly and ensure
//    logic is impervious to ordering variations that still preserve shape.
//    Also, fixed a bug in logic to re-map ghost-zoning.
//
//    Mark C. Miller, Wed Jul 11 10:57:26 PDT 2012
//    Changed interface to support repeated calls for different segments of
//    the same zonelist.
//
//    Kathleen Biagas, Mon Aug 15 14:09:55 PDT 2016
//    VTK-8, API for updating GhostLevel changed.
//
// ****************************************************************************
void
avtSiloFileFormat::ReadInArbConnectivity(const char *meshname,
    vtkUnstructuredGrid *ugrid, DBucdmesh *um, int domain, int gzOffset)
{
    int i, j, sum;

    DBphzonelist *phzl = um->phzones;
    if (!phzl)
        return;
    
    int nsdims = um->ndims;
    if (nsdims != 2 && nsdims != 3)
        return;

    //
    // Go ahead and add an empty avtOriginalCellNumbers array now.
    // We'll populate it as we proceed but, if we never encounter 
    // truly arbitrary zones, we'll remove it at the end because we
    // won't actually need it.
    //
    bool origCellNumbersAdded = false;
    if (!ugrid->GetCellData()->GetArray("avtOriginalCellNumbers"))
    {
        vtkUnsignedIntArray *oca = vtkUnsignedIntArray::New();
        oca->SetName("avtOriginalCellNumbers");
        oca->SetNumberOfComponents(2);
        ugrid->GetCellData()->AddArray(oca);
        ugrid->GetCellData()->CopyFieldOn("avtOriginalCellNumbers");
        oca->Delete();
        origCellNumbersAdded = true;
    }

    //
    // Instantiate cell- and node- re-mapping arrays. Just as for
    // avtOriginalCellNumbers, if we find out we don't need 'em, we
    // remove them at the end. Note that due to the way we traverse
    // the arb. polyhedral connectivity information and produce zones
    // in the output mesh, the nodes will always be organized such
    // that all the nodes of the original mesh come first, followed
    // by all the nodes that get inserted for arb. polyhedra cell
    // centers. For this reason, the nodeReMap vector is defined
    // only for those 'extra' nodes.
    //
    vector<int> *nodeReMap;
    bool nodeReMapAdded = false;
    map<string, map<int, vector<int>* > >::iterator domit;
    if ((domit = arbMeshNodeReMap.find(meshname)) != arbMeshNodeReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        nodeReMap = domit->second[domain];
    }
    else
    {
        nodeReMap = new vector<int>;
        arbMeshNodeReMap[meshname][domain] = nodeReMap;
        nodeReMapAdded = true;
    }

    vector<int> *cellReMap;
    bool cellReMapAdded = false;
    if ((domit = arbMeshCellReMap.find(meshname)) != arbMeshCellReMap.end() &&
         domit->second.find(domain) != domit->second.end())
    {
        cellReMap = domit->second[domain];
    }
    else
    {
        cellReMap = new vector<int>;
        arbMeshCellReMap[meshname][domain] = cellReMap;
        cellReMapAdded = true;
    }

    // build up random access offset indices into nodelist and facelist lists
    vector<int> nloffs;
    for (i = 0, sum = 0; i < phzl->nfaces; sum += phzl->nodecnt[i], i++)
        nloffs.push_back(sum);
    vector<int> floffs;
    for (i = 0, sum = 0; i < phzl->nzones; sum += phzl->facecnt[i], i++)
        floffs.push_back(sum);
    
    //
    // Main loop over all zones in this phzl
    //
    bool encounteredFullyArbitraryCase = false;
    int nthings = (phzl->nzones)?phzl->nzones:phzl->nfaces;
    for (int gz = 0; gz < nthings; gz++)
    {
        int fcnt;
        if (phzl->nzones)
            fcnt = phzl->facecnt[gz];
        else
            fcnt = phzl->nodecnt[gz];

        unsigned int ocdata[2] = {static_cast<unsigned int>(domain), 
                                  static_cast<unsigned int>(gz+gzOffset)};

        if (((nsdims == 3) && (fcnt == 3 || // Must be tri
                               fcnt == 4 || // Maybe tet or quad
                               fcnt == 5 || // Maybe pyramid or prism/wedge
                               fcnt == 6)) || // Maybe hex
            ((nsdims == 2) && (fcnt == 3 || // Must be tri
                               fcnt == 4))) // Must be quad
        {
            // Iterate over all faces for this zone finding the UNIQUE
            // set of nodes the union of all the faces references. Also,
            // along the way, keep track of total number of 3 node (tri)
            // and 4 node (quad) faces as well as the first of these
            // encountered and the face that 'opposes' those first faces,
            // if any.
            bool isNotZooElement = false;
            int num3NodeFaces = 0;
            int num4NodeFaces = 0;
            int firstFace = -INT_MAX;
            int firstFaceNodes[4] = {0,0,0,-INT_MAX};
            int opposingFace = -INT_MAX;
            int opposingFaceNodes[4] = {0,0,0,-INT_MAX};
            int first4NodeFace = -INT_MAX;
            set<int> uniqnodes;
            int nloff;
            for (int lf = 0; lf < fcnt; lf++)     // lf = local face #
            {
                int flidx;                        // flidx = index into facelist
                int sgf;                          // sgf = signed global face #
                if (phzl->nzones == 0)
                {
                    flidx = nloffs[gz]+lf;
                    sgf = gz;
                }
                else
                {
                    flidx = floffs[gz]+lf;
                    sgf = phzl->facelist[flidx];
                }
                int gf = sgf < 0 ? ~sgf : sgf;    // gf = global face #
                int ncnt = phzl->nodecnt[gf];     // ncnt = # nodes for this face

                // If face is not a tri or quad, cannot be a zoo element.
#if 0
                if ((nsdims == 3 && ncnt != 3 && ncnt != 4) ||
                    (nsdims == 2 && ncnt != 2 && ncnt != 3 && ncnt != 4))
#endif
                if (ncnt != 2 && ncnt != 3 && ncnt != 4)
                {
                    isNotZooElement = true;
                    break;
                }

                // Maintain counts of 3 and 4 node faces
                if (nsdims == 3)
                {
                    if (ncnt == 3)
                    {
                        num3NodeFaces++;
                    }
                    else if (ncnt == 4)
                    {
                        if (num4NodeFaces == 0)
                            first4NodeFace = sgf;
                        num4NodeFaces++;
                    }
                }

                // Maintain list of unique nodes
                for (int ln = 0; ln < ncnt; ln++)
                {
                    int nlidx = nloffs[gf]+ln;
                    int gn = phzl->nodelist[nlidx];
                    uniqnodes.insert(gn);
                }

                // Just initialize firstFace if this is the first face
                if (lf == 0)
                {
                    firstFace = sgf;
                    for (int ln = 0; ln < ncnt; ln++)
                    {
                        int nlidx = nloffs[gf]+ln;
                        int gn = phzl->nodelist[nlidx];
                        firstFaceNodes[ln] = gn;
                    }
                    continue;
                }

                //
                // Detect an 'opposing' face to the first; one with no nodes
                // in common with it. Can happen only for prism or hex elements. 
                // Since a hex has no tri faces, if we encounter a tri and the
                // first face is currently a quad, we replace it with the tri.
                // Thus, after iterating over all faces, opposingFace will be
                // set to something other than -INT_MAX only if we have indeed
                // encountered a prism or a hex. Only do this for 3D.
                // 
                if (nsdims == 3)
                {
                    if (firstFaceNodes[3] == -INT_MAX) // first face is a tri
                    {
                        if (ncnt == 3) // curr face is a tri
                        {
                            // check any nodes in common with first
                            const int *ff = firstFaceNodes;
                            const int *nl = phzl->nodelist;
                            int n = nloffs[gf];
                            bool hasNodesInCommonWithFirst =
                                ff[0] == nl[n+0] || ff[0] == nl[n+1] || ff[0] == nl[n+2] ||
                                ff[1] == nl[n+0] || ff[1] == nl[n+1] || ff[1] == nl[n+2] ||
                                ff[2] == nl[n+0] || ff[2] == nl[n+1] || ff[2] == nl[n+2];
                            if (!hasNodesInCommonWithFirst)
                            {
                                opposingFace = sgf;
                                for (int ln = 0; ln < ncnt; ln++)    // ln = local node # 
                                {
                                    int nlidx = nloffs[gf]+ln;       // nlidx = index into nodelist
                                    int gn = phzl->nodelist[nlidx];  // gn = global node #
                                    opposingFaceNodes[ln] = gn;
                                }
                            }
                        }
                        else if (ncnt == 4)          // curr face is a quad
                        {
                            // do nothing
                        }
                    }
                    else                          // first face is a quad
                    {
                        if (ncnt == 3) // curr face is a tri
                        {
                            // Replace first face with this tri
                            firstFace = sgf;
                            for (int ln = 0; ln < ncnt; ln++)
                            {
                                int nlidx = nloffs[gf]+ln;
                                int gn = phzl->nodelist[nlidx];
                                firstFaceNodes[ln] = gn;
                            }
                            firstFaceNodes[3] = -INT_MAX;
                        }
                        else if (ncnt == 4)          // curr face is a quad
                        {
                            const int *ff = firstFaceNodes;
                            const int *nl = phzl->nodelist;
                            int n = nloffs[gf];
                            bool hasNodesInCommonWithFirst =
                                ff[0] == nl[n+0] || ff[0] == nl[n+1] || ff[0] == nl[n+2] || ff[0] == nl[n+3] ||
                                ff[1] == nl[n+0] || ff[1] == nl[n+1] || ff[1] == nl[n+2] || ff[1] == nl[n+3] ||
                                ff[2] == nl[n+0] || ff[2] == nl[n+1] || ff[2] == nl[n+2] || ff[2] == nl[n+3] ||
                                ff[3] == nl[n+0] || ff[3] == nl[n+1] || ff[3] == nl[n+2] || ff[3] == nl[n+3];
                            if (!hasNodesInCommonWithFirst)
                            {
                                opposingFace = sgf;
                                for (int ln = 0; ln < ncnt; ln++)
                                {
                                    int nlidx = nloffs[gf]+ln;
                                    int gn = phzl->nodelist[nlidx];
                                    opposingFaceNodes[ln] = gn;
                                }
                            }
                        }
                    }
                }
            } // done iterating over faces and collecting info about this zone

            //
            // Now, based on information we gathered iterating over all the 
            // faces of this zone, decide what case it is and handle it.
            //
            int nids[8];
            set<int>::iterator it;
            if (isNotZooElement)                // Arbitrary
            {
                ArbInsertArbitrary(ugrid, nsdims, phzl, gz, nloffs, floffs, ocdata,
                    cellReMap, nodeReMap);
                encounteredFullyArbitraryCase = true;
            }
            else if (num3NodeFaces == 0 && num4NodeFaces == 0)
            {
                if (fcnt == 3 && uniqnodes.size() == 3)
                {
                    for (it = uniqnodes.begin(), j = 0; it != uniqnodes.end() && j < 3; it++, j++)
                        nids[j] = *it;
                    ArbInsertTriangle(ugrid, nids, ocdata, cellReMap);
                }
                else if (fcnt == 4 && uniqnodes.size() == 4)
                {
                    for (it = uniqnodes.begin(), j = 0; it != uniqnodes.end() && j < 4; it++, j++)
                        nids[j] = *it;
                    ArbInsertQuadrilateral(ugrid, nids, ocdata, cellReMap);
                }
                else
                {
                    ArbInsertArbitrary(ugrid, nsdims, phzl, gz, nloffs, floffs, ocdata,
                        cellReMap, nodeReMap);
                    encounteredFullyArbitraryCase = true;
                }
            }
            else if (fcnt == 4 && uniqnodes.size() == 4 &&
                     num3NodeFaces == 4 && num4NodeFaces == 0)    // Tet
            {
                // Just get all 4 nodes from the uniqnodes
                for (it = uniqnodes.begin(), j = 0; it != uniqnodes.end() && j < 4; it++, j++)
                    nids[j] = *it;
                ArbInsertTet(ugrid, nids, ocdata, cellReMap);
            }
            else if (fcnt == 5 && uniqnodes.size() == 5 && 
                     num3NodeFaces == 4 && num4NodeFaces == 1)    // Pyramid
            {
                // Get first 4 nodes from first4NodeFace
                nloff = nloffs[first4NodeFace<0?~first4NodeFace:first4NodeFace];
                for (j = 0; j < 4; j++)
                {
                    // We need to take 'em in reverse order if face index is negative
                    int n = first4NodeFace<0?nloff+3-j:nloff+j;
                    nids[j] = phzl->nodelist[n];
                    uniqnodes.erase(nids[j]);
                }

                // Get last node from only remaining node in uniqnodes
                nids[4] = *(uniqnodes.begin());

                ArbInsertPyramid(ugrid, nids, ocdata, cellReMap);
            }
            else if (fcnt == 5 && uniqnodes.size() == 6 && 
                     num3NodeFaces == 2 && num4NodeFaces == 3)    // Prism/Wedge
            {
                // The 6 integers to specify a prism can be conceptually organized
                // into two groups of 3 for the two triangle faces where the
                // first 3 loop around the 'first' (left in the Silo manual)
                // face with righthand rule yielding an inward normal and the
                // second 3 loop around the 'second' (right in the Silo manual)
                // face with righthand rule yielding an outward normal.
                // Consequently, if firstFace has a positive value, the order of
                // nodes in firstFaceNodes as taken directly from PHZL yield an
                // outward normal and the order needs to be reversed. Likewise,
                // if opposingFace has a negative value, the order of nodes in
                // opposingFaceNodes as taken directly from PHZL yield an inward
                // normal and the order needs to be reversed.
                if (firstFace >= 0)
                {
                    int tmp = firstFaceNodes[0];
                    firstFaceNodes[0] = firstFaceNodes[2];
                    firstFaceNodes[2] = tmp;
                }
                if (opposingFace < 0)
                {
                    int tmp = opposingFaceNodes[0];
                    opposingFaceNodes[0] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = tmp;
                }

                // Detect rotational offset between first and opposing faces
                // by iterating over the faces that are neither the first or opposing
                // and match nodes from firstFace to those in opposingFace and then
                // compute the offset between their positions in the two 4 int arrays.
                // These nested loops are designed to terminate early quickly.
                int delta = -INT_MAX;
                for (int lf = 0; lf < fcnt && delta == -INT_MAX; lf++)
                {
                    int flidx = floffs[gz]+lf;
                    int sgf = phzl->facelist[flidx];
                    if (sgf == firstFace || sgf == opposingFace) continue;
                    int gf = sgf < 0 ? ~sgf : sgf;
                    int ncnt = phzl->nodecnt[gf];
                    for (int ln = 0; ln < ncnt; ln++)
                    {
                        int nlidx0 = nloffs[gf]+ln;
                        int gn0 = phzl->nodelist[nlidx0];
                        int nlidx1 = (ln==(ncnt-1))?nloffs[gf]:nlidx0+1;
                        int gn1 = phzl->nodelist[nlidx1];
                        int i0f = -1;
                        int i0o = -1;
                        int i1f = -1;
                        int i1o = -1;
                        for (int qq = 0; qq < 3; qq++)
                        {
                            if (i0f == -1 && firstFaceNodes[qq] == gn0)
                                i0f = qq;
                            if (i0o == -1 && opposingFaceNodes[qq] == gn0)
                                i0o = qq;
                            if (i1f == -1 && firstFaceNodes[qq] == gn1)
                                i1f = qq;
                            if (i1o == -1 && opposingFaceNodes[qq] == gn1)
                                i1o = qq;
                        }
                        if      (i0f != -1 && i1o != -1)
                        {
                            delta = i1o - i0f;
                            if (delta == 0)
                                delta = 0;
                            else if (delta == -2 || delta == 1)
                                delta = 1;
                            else if (delta == -1 || delta == 2)
                                delta = 2;
                            break;

                        }
                        else if (i1f != -1 && i0o != -1)
                        {
                            delta = i0o - i1f;
                            if (delta == 0)
                                delta = 0;
                            else if (delta == -2 || delta == 1)
                                delta = 1;
                            else if (delta == -1 || delta == 2)
                                delta = 2;
                            break;
                        }
                    }
                }

                // Ok, now rotate the opposing face to correct position
                // as detected by 'delta' logic above
                if (delta == 1)
                {
                    int tmp = opposingFaceNodes[0];
                    opposingFaceNodes[0] = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = tmp;
                }
                else if (delta == 2)
                {
                    int tmp = opposingFaceNodes[2];
                    opposingFaceNodes[2] = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[0];
                    opposingFaceNodes[0] = tmp;
                }

                // Now, we're ready to fill in the 6 integer array
                // from first and opposing face values and create the wedge
                nids[0] = opposingFaceNodes[2];
                nids[1] = opposingFaceNodes[0];
                nids[2] = opposingFaceNodes[1];
                nids[3] = firstFaceNodes[2];
                nids[4] = firstFaceNodes[0];
                nids[5] = firstFaceNodes[1];
                ArbInsertWedge(ugrid, nids, ocdata, cellReMap);
            }
            else if (fcnt == 6 && uniqnodes.size() == 8 && 
                     num3NodeFaces == 0 && num4NodeFaces == 6)    // Hex
            {
                // The 8 integers to specify a hex need to be ordered such that the
                // first 4 loop around the 'first' (bottom in the Silo manual)
                // face with righthand rule yielding an inward normal and the
                // second 4 loop around the 'second' (top in the Silo manual)
                // face with righthand rule yielding an outward normal.
                // Consequently, if firstFace has a positive value, the order of
                // nodes in firstFaceNodes as taken directly from PHZL yield an
                // outward normal and the order needs to be reversed. Likewise,
                // if opposingFace has a negative value, the order of nodes in
                // opposingFaceNodes as taken directly from PHZL yield an inward
                // normal and the order needs to be reversed.
                if (firstFace >= 0)
                {
                    int tmp = firstFaceNodes[0];
                    firstFaceNodes[0] = firstFaceNodes[3];
                    firstFaceNodes[3] = tmp;
                    tmp = firstFaceNodes[1];
                    firstFaceNodes[1] = firstFaceNodes[2];
                    firstFaceNodes[2] = tmp;
                }
                if (opposingFace < 0)
                {
                    int tmp = opposingFaceNodes[0];
                    opposingFaceNodes[0] = opposingFaceNodes[3];
                    opposingFaceNodes[3] = tmp;
                    tmp = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = tmp;
                }

                // Detect rotational offset between first and opposing faces
                // by iterating over the faces that are neither the first or opposing
                // and match nodes from firstFace to those in opposingFace and then
                // compute the offset between their positions in the two 4 int arrays.
                // These nested loops are designed to terminate early quickly.
                int delta = -INT_MAX;
                for (int lf = 0; lf < fcnt && delta == -INT_MAX; lf++)
                {
                    int flidx = floffs[gz]+lf;
                    int sgf = phzl->facelist[flidx];
                    if (sgf == firstFace || sgf == opposingFace) continue;
                    int gf = sgf < 0 ? ~sgf : sgf;
                    int ncnt = phzl->nodecnt[gf];
                    for (int ln = 0; ln < ncnt; ln++)
                    {
                        int nlidx0 = nloffs[gf]+ln;
                        int gn0 = phzl->nodelist[nlidx0];
                        int nlidx1 = (ln==(ncnt-1))?nloffs[gf]:nlidx0+1;
                        int gn1 = phzl->nodelist[nlidx1];
                        int i0f = -1;
                        int i0o = -1;
                        int i1f = -1;
                        int i1o = -1;
                        for (int qq = 0; qq < 4; qq++)
                        {
                            if (i0f == -1 && firstFaceNodes[qq] == gn0)
                                i0f = qq;
                            if (i0o == -1 && opposingFaceNodes[qq] == gn0)
                                i0o = qq;
                            if (i1f == -1 && firstFaceNodes[qq] == gn1)
                                i1f = qq;
                            if (i1o == -1 && opposingFaceNodes[qq] == gn1)
                                i1o = qq;
                        }
                        if      (i0f != -1 && i1o != -1)
                        {
                            delta = i1o - i0f;
                            if (delta == 0)
                                delta = 0;
                            else if (delta == -3 || delta == 1)
                                delta = 1;
                            else if (delta == -2 || delta == 2)
                                delta = 2;
                            else if (delta == -1 || delta == 3)
                                delta = 3;
                            break;

                        }
                        else if (i1f != -1 && i0o != -1)
                        {
                            delta = i0o - i1f;
                            if (delta == 0)
                                delta = 0;
                            else if (delta == -3 || delta == 1)
                                delta = 1;
                            else if (delta == -2 || delta == 2)
                                delta = 2;
                            else if (delta == -1 || delta == 3)
                                delta = 3;
                            break;
                        }
                    }
                }

                // Ok, now rotate the opposing face to correct position
                // as detected by 'delta' logic above
                if (delta == 1)
                {
                    int tmp = opposingFaceNodes[0];
                    opposingFaceNodes[0] = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = opposingFaceNodes[3];
                    opposingFaceNodes[3] = tmp;
                }
                else if (delta == 2)
                {
                    int tmp = opposingFaceNodes[0];
                    opposingFaceNodes[0] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = tmp;
                    tmp = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[3];
                    opposingFaceNodes[3] = tmp;
                }
                else if (delta == 3)
                {
                    int tmp = opposingFaceNodes[3];
                    opposingFaceNodes[3] = opposingFaceNodes[2];
                    opposingFaceNodes[2] = opposingFaceNodes[1];
                    opposingFaceNodes[1] = opposingFaceNodes[0];
                    opposingFaceNodes[0] = tmp;
                }

                // Now, we're ready to fill in the 8 integer array
                // from first and opposing face values and create the hex
                nids[0] = firstFaceNodes[0];
                nids[1] = firstFaceNodes[1];
                nids[2] = firstFaceNodes[2];
                nids[3] = firstFaceNodes[3];
                nids[4] = opposingFaceNodes[0];
                nids[5] = opposingFaceNodes[1];
                nids[6] = opposingFaceNodes[2];
                nids[7] = opposingFaceNodes[3];
                ArbInsertHex(ugrid, nids, ocdata, cellReMap);
            }
            else                        // Arbitrary
            {
                ArbInsertArbitrary(ugrid, nsdims, phzl, gz, nloffs, floffs, ocdata,
                    cellReMap, nodeReMap);
                encounteredFullyArbitraryCase = true;
            }
        }
        else                            // Arbitrary
        {
            ArbInsertArbitrary(ugrid, nsdims, phzl, gz, nloffs, floffs, ocdata,
                cellReMap, nodeReMap);
            encounteredFullyArbitraryCase = true;
        }
    } // end of loop over all zones
    
    // --- Read Ghost Node Labels --- //
    if (um->ghost_node_labels) {
        GetUcdGhostNodesFromLabels(um, ugrid, cellReMap);
        debug5 << "GetUcdGhostNodesFromLabels" << std::endl;
    }
    
    // --- Read Ghost Zone Labels --- //
    if (um->phzones->ghost_zone_labels) {
        GetUcdPolyhedralGhostZonesFromLabels(um->phzones, ugrid, cellReMap);
        debug5 << "GetUcdPolyhedralGhostZonesFromLabels" << std::endl;
    } else {
        //
        // Handle the ghost zoning, if necessary. We can do this easily now that
        // the mesh has been re-mapped to all ucd elements. We use the cell
        // centered re-mapping, if any, to make it work.
        //
        if (phzl->nzones > 0 && (phzl->lo_offset != 0 || phzl->hi_offset != phzl->nzones-1))
        {
            //
            // Populate a zone-centered array of ghost zone values.
            //
            unsigned char *gvals = new unsigned char[phzl->nzones];
            unsigned char val = 0;
            avtGhostData::AddGhostZoneType(val, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            for (i = 0; i < phzl->lo_offset; i++)
                gvals[i] = val;
            for (i = phzl->lo_offset; i <= phzl->hi_offset; i++)
                gvals[i] = 0;
            for (i = phzl->hi_offset+1; i < phzl->nzones; i++)
                gvals[i] = val;
           
            //
            // Remap the zone-centered array based on arb-poly remapping
            // computed above.
            //
            vector<int> noremap;
            vector<int> *remap = &noremap;    
            if (encounteredFullyArbitraryCase)
                remap = cellReMap;
            DBucdvar tmp;
            tmp.centering = DB_ZONECENT;
            tmp.datatype = DB_CHAR;
            tmp.nels = phzl->nzones;
            tmp.nvals = 1;
            tmp.vals = (void**) malloc(sizeof(void*));
            tmp.vals[0] = (void*) gvals;
            vtkDataArray *ghostZones = CopyAndPadUcdVar<unsigned char,vtkUnsignedCharArray>(&tmp, *remap);
            free(tmp.vals);
            delete [] gvals;

            //
            // Assign the ghost zone array to the ugrid object.
            //
            ghostZones->SetName("avtGhostZones");
            ugrid->GetCellData()->AddArray(ghostZones);
            ghostZones->Delete();
        }
    }
    ugrid->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

    //
    // Remove the avtOriginalCellNumbers array if we don't really
    // need it.
    //
    if (!encounteredFullyArbitraryCase)
    {
        if (origCellNumbersAdded)
            ugrid->GetCellData()->RemoveArray("avtOriginalCellNumbers");
        if (nodeReMapAdded)
        {
            arbMeshNodeReMap.find(meshname)->second.erase(domain);
            delete nodeReMap;
        }
        if (cellReMapAdded)
        {
            arbMeshCellReMap.find(meshname)->second.erase(domain);
            delete cellReMap;
        }
    }
    else
    {
        // Let the rest of VisIt know that the mesh was split from its
        // original form.
        metadata->SetZonesWereSplit(meshname, true);
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetQuadMesh
//
//  Purpose:
//      Determines if the quadmesh is a rectilinear or curvilinear mesh and
//      calls the right routine to fetch it.
//
//  Arguments:
//      dbfile   A handle to the file this variable lives in.
//      mn       The mesh name.
//      domain   The domain.
//
//  Returns:     The vtkDataSet corresponding to mn.
//
//  Programmer:  Hank Childs
//  Creation:    November 1, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Kathleen Bonnell, Tue Jun 12 14:34:02 PDT 2001 
//    Added call to method GetQuadGhostZones.
//
//    Hank Childs, Tue Mar  5 16:53:16 PST 2002
//    Verify that the dimensions of a quadmesh are correct.
//
//    Hank Childs, Tue Jun 25 16:29:12 PDT 2002
//    Add the base indices as field data to the VTK dataset.
//
//    Kathleen Bonnell, Tue Feb  8 13:41:05 PST 2005 
//    Added int arg for domain, to be used in retrieving connectivity extents
//    for setting base_index when necessary. 
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Cyrus Harrison, Thu Apr 26 10:14:42 PDT 2007
//    Added group_id as field data to the VTK dataset.
//
//    Mark C. Miller, Tue Mar  3 19:35:35 PST 2009
//    Predicated addition of "group_id" as field data on it having
//    non-negative value.
//
//    Brad Whitlock, Fri Aug  7 11:11:29 PDT 2009
//    I added some exception handling.
//
//    Mark C. Miller, Wed Sep 25 10:30:05 PDT 2013
//    Added logic to handle 3D, co-linear, cylindrical meshes from Silo
//    
//    Mark C. Miller, Tue Jan  7 10:28:47 PST 2014
//    Only query cached SDB for extents if all meshes have same block count.
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetQuadMesh(DBfile *dbfile, const char *mn, int domain)
{
    //
    // Allow empty data sets
    //
    if (string(mn) == "EMPTY")
        return NULL;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *meshname  = const_cast<char *>(mn);

    //
    // Get the Silo construct.
    //
    DBquadmesh  *qm = DBGetQuadmesh(dbfile, meshname);
    if (qm == NULL)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }
    else if (DBIsEmptyQuadmesh(qm))
    {
        DBFreeQuadmesh(qm);
        return NULL;
    }

    VerifyQuadmesh(qm, meshname);

    vtkDataSet *ds = NULL;
    TRY
    {
        if (qm->coordtype == DB_COLLINEAR)
        {
            if (qm->ndims == 3 && qm->coord_sys == DB_CYLINDRICAL)
                ds = CreateCurvilinearMesh(qm);
            else
                ds = CreateRectilinearMesh(qm);
        }
        else
            ds = CreateCurvilinearMesh(qm);
    }
    CATCH(VisItException)
    {
        DBFreeQuadmesh(qm);
        RETHROW;
    }
    ENDTRY

    GetQuadGhostZones(qm, ds);

    //
    // Add group id as field data
    //
    if (qm->group_no >= 0)
    {
        vtkIntArray *grp_id_arr = vtkIntArray::New();
        grp_id_arr->SetNumberOfTuples(1);
        grp_id_arr->SetValue(0, qm->group_no);
        grp_id_arr->SetName("group_id");
        ds->GetFieldData()->AddArray(grp_id_arr);
        grp_id_arr->Delete();
    }

    //
    // Determine the indices of the mesh within its group.  Add that to the
    // VTK dataset as field data.
    //
    vtkIntArray *arr = vtkIntArray::New();
    arr->SetNumberOfTuples(3);
    arr->SetValue(0, qm->base_index[0]);
    arr->SetValue(1, qm->base_index[1]);
    arr->SetValue(2, qm->base_index[2]);
    arr->SetName("base_index");

    //
    //  If we're not really sure that the base_index was set in the file,
    //  check for connectivity info.
    //
    if (qm->base_index[0] == 0 &&
        qm->base_index[1] == 0 &&
        qm->base_index[2] == 0) 
    {
        bool allSameBlockCount = true;
        for (int i = 0; (i < (int) blocksForMesh.size() - 1) && allSameBlockCount; i++)
        {
            if (blocksForMesh[i] != blocksForMesh[i+1])
                allSameBlockCount = false;
        }

        void_ref_ptr vr;
        if (allSameBlockCount)
            vr = cache->GetVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION, -1, -1);

        if (*vr != 0)
        {
            avtStructuredDomainBoundaries *dbi = 
                (avtStructuredDomainBoundaries*)*vr;
            if (dbi != NULL)
            {
                int ext[6];
                dbi->GetExtents(domain, ext);
                arr->SetValue(0, ext[0]);
                arr->SetValue(1, ext[2]);
                arr->SetValue(2, ext[4]); 
            }
        }
    }
    ds->GetFieldData()->AddArray(arr);
    arr->Delete();

    DBFreeQuadmesh(qm);

    return ds;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::VerifyQuadmesh
//
//  Purpose:
//      Determines if a quadmesh has invalid dimensions and tries to
//      compensate if it does.
//
//  Arguments:
//      qm         A quad mesh.
//      meshname   The name of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Mar  6 15:00:21 PST 2002  
//    Added better support for 2D, since sloppy coding had caused perfectly
//    fine 2D meshes to be tagged as bad.
//
// ****************************************************************************

void
avtSiloFileFormat::VerifyQuadmesh(DBquadmesh *qm, const char *meshname)
{
    if (qm->ndims == 3)
    {
        //
        // Make sure the dimensions are correct.
        //
        if (qm->nnodes != qm->dims[0]*qm->dims[1]*qm->dims[2])
        {
            if (qm->dims[0] > 100000 || qm->dims[1] > 100000 
                || qm->dims[2] > 100000)
            {
                int orig[3];
                orig[0] = qm->dims[0];
                orig[1] = qm->dims[1];
                orig[2] = qm->dims[2];

                //
                // See if the max_index has any clues, first without ghost
                // zones, then with.
                //
                if (qm->nnodes == qm->max_index[0] * qm->max_index[1]
                                  * qm->max_index[2])
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1];
                    qm->dims[2] = qm->max_index[2];
                }
                else if (qm->nnodes == (qm->max_index[0]+1) * 
                                   (qm->max_index[1]+1) * (qm->max_index[2]+1))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1]+1;
                    qm->dims[2] = qm->max_index[2]+1;
                }
                else if (qm->nnodes == (qm->max_index[0]) *(qm->max_index[1]+1)
                                        * (qm->max_index[2]+1))
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1]+1;
                    qm->dims[2] = qm->max_index[2]+1;
                }
                else if (qm->nnodes == (qm->max_index[0]+1) *(qm->max_index[1])
                                        * (qm->max_index[2]+1))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1];
                    qm->dims[2] = qm->max_index[2]+1;
                }
                else if (qm->nnodes == (qm->max_index[0]+1) *
                                     (qm->max_index[1]+1) * (qm->max_index[2]))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1]+1;
                    qm->dims[2] = qm->max_index[2];
                }
                else if (qm->nnodes == (qm->max_index[0]) * (qm->max_index[1])
                                        * (qm->max_index[2]+1))
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1];
                    qm->dims[2] = qm->max_index[2]+1;
                }
                else if (qm->nnodes == (qm->max_index[0]) *(qm->max_index[1]+1)
                                        * (qm->max_index[2]))
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1]+1;
                    qm->dims[2] = qm->max_index[2];
                }
                else if (qm->nnodes == (qm->max_index[0]+1) *(qm->max_index[1])
                                        * (qm->max_index[2]))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1];
                    qm->dims[2] = qm->max_index[2];
                }
                else
                {
                    debug1 << "The dimensions of the mesh did not match the "
                           << "number of nodes, cannot continue" << endl;
                    EXCEPTION1(InvalidVariableException, meshname);
                }
                char msg[1024];
                sprintf(msg, "A quadmesh had dimensions %d, %d, %d, which did "
                         "not result in the specified number of nodes (%d).   "
                         "The dimensions are being adjusted to %d, %d, %d",
                          orig[0], orig[1], orig[2], qm->nnodes, qm->dims[0],
                          qm->dims[1], qm->dims[2]);
                avtCallback::IssueWarning(msg);
            }
            else if (qm->nnodes > 10000000)
            {
                int orig = qm->nnodes;
                qm->nnodes = qm->dims[0]*qm->dims[1]*qm->dims[2];
                if (orig < 10000000)
                {
                    char msg[1024];
                    sprintf(msg, "The number of nodes is stored as %d, but "
                                 "that not agree with the dimensions "
                                 "(= %d %d %d).  Correcting to: %d",
                                  orig, qm->dims[0], qm->dims[1], qm->dims[2],
                                  qm->nnodes);
                    avtCallback::IssueWarning(msg);
                }
                else
                {
                    debug1 << "The dimensions of the mesh did not match the "
                           << "number of nodes, cannot continue" << endl;
                    EXCEPTION1(InvalidVariableException, meshname);
                }
            }
            else
            {
                debug1 << "The dimensions of the mesh did not match the "
                       << "number of nodes, cannot continue" << endl;
                EXCEPTION1(InvalidVariableException, meshname);
            }
        }
    }
    else if (qm->ndims == 2)
    {
        //
        // Make sure the dimensions are correct.
        //
        if (qm->nnodes != qm->dims[0]*qm->dims[1])
        {
            if (qm->dims[0] > 100000 || qm->dims[1] > 100000)
            {
                int orig[2];
                orig[0] = qm->dims[0];
                orig[1] = qm->dims[1];

                //
                // See if the max_index has any clues, first without ghost
                // zones, then with.
                //
                if (qm->nnodes == qm->max_index[0] * qm->max_index[1])
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1];
                }
                else if (qm->nnodes == (qm->max_index[0]+1) * 
                                       (qm->max_index[1]+1))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1]+1;
                }
                else if (qm->nnodes == (qm->max_index[0])*(qm->max_index[1]+1))
                {
                    qm->dims[0] = qm->max_index[0];
                    qm->dims[1] = qm->max_index[1]+1;
                }
                else if (qm->nnodes == (qm->max_index[0]+1)*(qm->max_index[1]))
                {
                    qm->dims[0] = qm->max_index[0]+1;
                    qm->dims[1] = qm->max_index[1];
                }
                else
                {
                    debug1 << "The dimensions of the mesh did not match the "
                           << "number of nodes, cannot continue" << endl;
                    EXCEPTION1(InvalidVariableException, meshname);
                }
                char msg[1024];
                sprintf(msg, "A quadmesh had dimensions %d, %d, which did "
                         "not result in the specified number of nodes (%d).   "
                         "The dimensions are being adjusted to %d, %d",
                          orig[0], orig[1],qm->nnodes,qm->dims[0],qm->dims[1]);
                avtCallback::IssueWarning(msg);
            }
            else if (qm->nnodes > 10000000)
            {
                int orig = qm->nnodes;
                qm->nnodes = qm->dims[0]*qm->dims[1];
                if (orig < 10000000)
                {
                    char msg[1024];
                    sprintf(msg, "The number of nodes is stored as %d, but "
                                 "that not agree with the dimensions "
                                 "(= %d %d).  Correcting to: %d",
                                  orig, qm->dims[0], qm->dims[1], qm->nnodes);
                    avtCallback::IssueWarning(msg);
                }
                else
                {
                    debug1 << "The dimensions of the mesh did not match the "
                           << "number of nodes, cannot continue" << endl;
                    EXCEPTION1(InvalidVariableException, meshname);
                }
            }
            else
            {
                debug1 << "The dimensions of the mesh did not match the "
                       << "number of nodes, cannot continue" << endl;
                EXCEPTION1(InvalidVariableException, meshname);
            }
        }
    }
}

// ****************************************************************************
// Method: CreateCurve
//
// Purpose: 
//   Creates a curve from a DBcurve.
//
// Arguments:
//   cur : The curve
//   curvename : The curve name.
//   vtkType   : The type of coordinates to create.
//
// Returns:    A new vtkRectilinearGrid.
//
// Note:       
//
// Programmer: Mark Miller
// Creation:   Thu Aug  6 12:16:00 PDT 2009
//
// Modifications:
//   Brad Whitlock, Thu Aug  6 12:16:09 PDT 2009
//   I moved this block out from GetCurve and I templated it.
//
//   Mark C. Miller, Tue Oct 20 16:51:36 PDT 2009
//   Made it static.
//
//   Mark C. Miller, Thu Dec  5 09:35:15 PST 2013
//   Support Silo curves with polar (r,theta) coords.
// ****************************************************************************

template <typename T, typename Tarr>
static vtkRectilinearGrid *
CreateCurve(DBcurve *cur, const char *curvename, int vtkType)
{
    T *px = (T *) cur->x;
    T *py = (T *) cur->y;
    vtkRectilinearGrid *rg = vtkVisItUtility::Create1DRGrid(cur->npts, vtkType);
    Tarr *xc = Tarr::SafeDownCast(rg->GetXCoordinates());
    Tarr *yv = Tarr::New();
    yv->SetNumberOfComponents(1);
    yv->SetNumberOfTuples(cur->npts);
    yv->SetName(curvename);
    if (cur->coord_sys == DB_SPHERICAL || cur->coord_sys == DB_CYLINDRICAL)
    {
        for (int i = 0 ; i < cur->npts; i++)
        {
            T x = px[i] * cos(py[i]);
            T y = px[i] * sin(py[i]);
            xc->SetValue(i, x);
            yv->SetValue(i, y);
        }
    }
    else
    {
        for (int i = 0 ; i < cur->npts; i++)
        {
            xc->SetValue(i, px[i]);
            yv->SetValue(i, py[i]);
        }
    }
    rg->GetPointData()->SetScalars(yv);
    yv->Delete();

    return rg;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetCurve
//
//  Purpose: Read a Silo curve object and return a vtkDataSet for it
//
//  Arguments:
//      dbfile   A handle to the file this variable lives in.
//      mn       The curve name.
//
//  Returns:     The vtkDataSet corresponding to mn.
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 1, 2006 
//
//  Modifications:
//    Kathleen Bonnell, Mon Jul 14 14:55:48 PDT 2008
//    Specify curves as 1D rectilinear grids with yvalues stored in point data.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Thu Aug  6 12:15:50 PDT 2009
//    Use templates.
//
//    Mark C. Miller, Tue Jul 20 19:21:34 PDT 2010
//    Added support for LONG LONG types.
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetCurve(DBfile *dbfile, const char *cn)
{
    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *curvename  = const_cast<char *>(cn);

    //
    // Get the Silo construct.
    //
    DBcurve *cur = DBGetCurve(dbfile, curvename);
    if (cur == NULL)
    {
        EXCEPTION1(InvalidVariableException, curvename);
    }

    vtkRectilinearGrid *rg = 0;
    if (cur->datatype == DB_FLOAT)
        rg = CreateCurve<float,vtkFloatArray>(cur, curvename, VTK_FLOAT);
    else if (cur->datatype == DB_DOUBLE)
        rg = CreateCurve<double,vtkDoubleArray>(cur, curvename, VTK_DOUBLE);
    else if (cur->datatype == DB_LONG_LONG)
        rg = CreateCurve<long long,vtkLongLongArray>(cur, curvename, VTK_LONG_LONG);
    else if (cur->datatype == DB_LONG)
        rg = CreateCurve<long,vtkLongArray>(cur, curvename, VTK_LONG);
    else if (cur->datatype == DB_INT)
        rg = CreateCurve<int,vtkIntArray>(cur, curvename, VTK_INT);
    else if (cur->datatype == DB_SHORT)
        rg = CreateCurve<short,vtkShortArray>(cur, curvename, VTK_SHORT);
    else if (cur->datatype == DB_CHAR)
        rg = CreateCurve<char,vtkCharArray>(cur, curvename, VTK_CHAR);

    DBFreeCurve(cur);

    return rg;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::CreateRectilinearMesh
//
//  Purpose:
//      Creates a rectilinear mesh from a DBquadmesh.
//
//  Arguments:
//      qm      A quadmesh.
//
//  Returns:    A vtkRectilinearMesh from qm.
//
//  Programmer: Hank Childs
//  Creation:   November 1, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Nov  8 09:20:50 PST 2001 
//    Use vtkFloatArray instead of vtkScalars for rgrid coordinates in
//    order to match VTK 4.0 API.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Thu Aug  6 11:38:58 PDT 2009
//    Use doubles for the coordinates if they are doubles.
//
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::CreateRectilinearMesh(DBquadmesh *qm)
{
    int   i, j;

    if(qm->datatype != DB_FLOAT && qm->datatype != DB_DOUBLE)
    {
        EXCEPTION1(InvalidVariableException, "The Silo reader expects float or "
            "double precision coordinates for rectilinear meshes.");
    }

    vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    int           dims[3];
    vtkDataArray   *coords[3];
    for (i = 0 ; i < 3 ; i++)
    {
        // Default number of components for an array is 1.
        if(qm->datatype == DB_DOUBLE)
            coords[i] = vtkDoubleArray::New();
        else
            coords[i] = vtkFloatArray::New();

        if (i < qm->ndims)
        {
            dims[i] = qm->dims[i];
            coords[i]->SetNumberOfTuples(dims[i]);
            if(qm->datatype == DB_DOUBLE)
            {
                for (j = 0 ; j < dims[i] ; j++)
                    coords[i]->SetComponent(j, 0, ((double**)qm->coords)[i][j]);
            }
            else
            {
                for (j = 0 ; j < dims[i] ; j++)
                    coords[i]->SetComponent(j, 0, ((float**)qm->coords)[i][j]);
            }
        }
        else
        {
            dims[i] = 1;
            coords[i]->SetNumberOfTuples(1);
            coords[i]->SetComponent(0, 0, 0.);
        }
    }

    rgrid->SetDimensions(dims);
    rgrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rgrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rgrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rgrid;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::CreateCurvilinearMesh
//
//  Purpose:
//      Takes Silo representation of a curvilinear mesh and makes a VTK
//      curvilinear mesh (ie structured grid).
//
//  Arguments:
//      qm       A Silo quad mesh -- known to be a curvilinear mesh.
//
//  Returns:     The VTK curvilinear grid representation of qm.
//
//  Programmer:  Hank Childs
//  Creation:    November 1, 2000
//
//  Modifications:
//
//    Hank Childs, Thu Apr 26 16:09:39 PDT 2001
//    Sped up routine.
//
//    Eric Brugger, Thu May 23 14:23:23 PDT 2002
//    I modified the routine to take into account the major order of
//    the variable being read.
//
//    Hank Childs, Wed Jan 14 13:40:33 PST 2004
//    Marginal improvement in performance.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added support for double precision coordinates in a quad mesh
//
//    Brad Whitlock, Fri Aug  7 11:21:16 PDT 2009
//    I modified the row major case so it just uses increment for index
//    calculations. Use unsigned int.
//
//    Mark C. Miller, Wed Sep 25 10:30:43 PDT 2013
//    Added logic to handle 3d, co-linear, cylindrical meshes from Silo
// ****************************************************************************

template <class T>
static void CopyQuadCoordinates(T *dest, int nx, int ny, int nz, int morder,
    const T *const c0, const T *const c1, const T *const c2)
{
    if (morder == DB_ROWMAJOR)
    {
        unsigned int nxnynz = nx * ny * nz;
        for (unsigned int idx = 0; idx < nxnynz; ++idx)
        {
            *dest++ = c0 ? c0[idx] : 0.;
            *dest++ = c1 ? c1[idx] : 0.;
            *dest++ = c2 ? c2[idx] : 0.;
        }
    }
    else
    {
        unsigned int nyz = ny * nz; 
        for (unsigned int k = 0; k < (unsigned int)nz; k++)
        {
            for (unsigned int j = 0; j < (unsigned int)ny; j++)
            {
                for (unsigned int i = 0; i < (unsigned int)nx; i++)
                {
                    unsigned int idx = k + j*nz + i*nyz;
                    *dest++ = c0 ? c0[idx] : 0.;
                    *dest++ = c1 ? c1[idx] : 0.;
                    *dest++ = c2 ? c2[idx] : 0.;
                }
            }
        }
    }
}

template <class T>
static void ConvertQuadCylindricalCoordinates(T *dest, int nx, int ny, int nz,
    const T *const c0, const T *const c1, const T *const c2)
{
    for (int k = 0; k < nz; k++)
    {
        for (int j = 0; j < ny; j++)
        {
            for (int i = 0; i < nx; i++)
            {
                T x = c0[i] * cos(M_PI/180.0*c1[j]);
                T y = c0[i] * sin(M_PI/180.0*c1[j]);
                T z = c2[k];
                *dest++ = x;
                *dest++ = y;
                *dest++ = z;
            }
        }
    }
}

vtkDataSet *
avtSiloFileFormat::CreateCurvilinearMesh(DBquadmesh *qm)
{
    if(qm->datatype != DB_FLOAT && qm->datatype != DB_DOUBLE)
    {
        EXCEPTION1(InvalidVariableException, "The Silo reader expects float or "
            "double precision coordinates for curvilinear meshes.");
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkStructuredGrid    *sgrid   = vtkStructuredGrid::New(); 
    vtkPoints            *points  = vtkPoints::New();
    sgrid->SetPoints(points);
    points->Delete();

    //
    // Tell the grid what its dimensions are and populate the points array.
    //
    int dims[3];
    dims[0] = (qm->dims[0] > 0 ? qm->dims[0] : 1);
    dims[1] = (qm->dims[1] > 0 ? qm->dims[1] : 1);
    dims[2] = (qm->dims[2] > 0 ? qm->dims[2] : 1);
    sgrid->SetDimensions(dims);

    //
    // vtkPoints assumes float data type
    //
    if (qm->datatype == DB_DOUBLE)
        points->SetDataTypeToDouble();

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    int nx = qm->dims[0];
    int ny = qm->dims[1];
    int nz = qm->ndims <= 2 ? 1 : qm->dims[2];
    points->SetNumberOfPoints(qm->nnodes);
    void *pts = points->GetVoidPointer(0);
    if (qm->datatype == DB_DOUBLE)
    {
        if (qm->ndims == 3 && qm->coord_sys == DB_CYLINDRICAL)
            ConvertQuadCylindricalCoordinates((double *) pts, nx, ny, nz,
               (double *) qm->coords[0], (double *) qm->coords[1], (double *) qm->coords[2]);
        else
            CopyQuadCoordinates((double *) pts, nx, ny, nz, qm->major_order,
                (double *) qm->coords[0], (double *) qm->coords[1],
                qm->ndims <= 2 ? 0 : (double *) qm->coords[2]);
    }
    else
    {
        if (qm->ndims == 3 && qm->coord_sys == DB_CYLINDRICAL)
            ConvertQuadCylindricalCoordinates((float *) pts, nx, ny, nz,
               (float *) qm->coords[0], (float *) qm->coords[1], (float *) qm->coords[2]);
        else
            CopyQuadCoordinates((float *) pts, nx, ny, nz, qm->major_order,
                (float *) qm->coords[0], (float *) qm->coords[1],
                qm->ndims <= 2 ? 0 : (float *) qm->coords[2]);
    }

    return sgrid;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetQuadGhostZones
//
//  Purpose:
//    Retrieves ghost zone information from the quad mesh and adds it
//    to the dataset.
//
//  Arguments:
//      qm       A Silo quad mesh. 
//      ds       The vtkDataSet in which to store the ghost level information. 
//
//  Programmer:  Kathleen Bonnell 
//  Creation:    June 12, 2001
//
//  Modifications:
//    Eric Brugger, Thu May 23 14:23:23 PDT 2002
//    I modified the routine to always store the ghost zone array in row
//    major order.
//
//    Kathleen Bonnell, Wed Jul 10 16:02:56 PDT 2002 
//    I added a field-data array that indicates the extents of the real
//    data.  Used during ghostzone removal.  
//    
//    Kathleen Bonnell, Wed Jun 25 13:45:04 PDT 2003 
//    Allocate space for realDims via SetNumberOfValues instead of Allocate.
//    Allocate does not set some internal values needed for later calls to
//    GetNumberOfTuples and the like. 
//    
//    Hank Childs, Fri Aug 27 17:22:19 PDT 2004
//    Rename ghost data array.
//
//    Kathleen Biagas, Mon Aug 15 14:09:55 PDT 2016
//    VTK-8, API for updating GhostLevel changed.
//
// ****************************************************************************

void 
avtSiloFileFormat::GetQuadGhostZones(DBquadmesh *qm, vtkDataSet *ds)
{
    //
    // Find the dimensions of the quad mesh. 
    //
    int dims[3];
    dims[0] = (qm->dims[0] > 0 ? qm->dims[0] : 1);
    dims[1] = (qm->dims[1] > 0 ? qm->dims[1] : 1);
    dims[2] = (qm->dims[2] > 0 ? qm->dims[2] : 1);

    //
    //  Determine if we have ghost points
    //
    
    // --- Read Ghost Node Labels --- //
    if (qm->ghost_node_labels) {
        GetQuadGhostNodesFromLabels(qm, ds);
        debug5 << "GetQuadGhostNodesFromLabels" << std::endl;
    }
    
    // --- Read Ghost Zone Labels --- //
    if (qm->ghost_zone_labels) {
        GetQuadGhostZonesFromLabels(qm, ds);
        debug5 << "GetQuadGhostZonesFromLabels" << std::endl;
    } else {
        
        int first[3];
        int last[3];
        bool ghostPresent = false;
        bool badIndex = false;
        for (int i = 0; i < 3; i++)
        {
            first[i] = (i < qm->ndims ? qm->min_index[i] : 0);
            last[i]  = (i < qm->ndims ? qm->max_index[i] : 0);

            if (first[i] < 0 || first[i] >= dims[i])
            {
                debug1 << "bad Index on first[" << i << "] dims is: " 
                       << dims[i] << endl;
                badIndex = true;
            }

            if (last[i] < 0 || last[i] >= dims[i])
            {
                debug1 << "bad Index on last[" << i << "] dims is: " 
                       << dims[i] << endl;
                badIndex = true;
            }

            if (first[i] != 0 || last[i] != dims[i] -1)
            {
                ghostPresent = true;
            }
        }        

        //
        //  Create the ghost zones array if necessary
        //
        
        if (ghostPresent && !badIndex)
        {
            bool *ghostPoints = new bool[qm->nnodes];
            //
            // Initialize as all ghost levels
            //
            for (int ii = 0; ii < qm->nnodes; ii++) {
                ghostPoints[ii] = true;
            }

            //
            // Set real values
            //
            for (int k = first[2]; k <= last[2]; k++) {
                for (int j = first[1]; j <= last[1]; j++) {
                    for (int i = first[0]; i <= last[0]; i++)
                    {
                        int index = k*dims[1]*dims[0] + j*dims[0] + i;
                        ghostPoints[index] = false; 
                    }
                }
            }

            //
            //  okay, now we have ghost points, but what we really want
            //  are ghost cells ... convert:  if all points associated with
            //  cell are 'real' then so is the cell.
            //
            unsigned char realVal = 0;
            unsigned char ghostVal = 0;
            avtGhostData::AddGhostZoneType(ghostVal, 
                                           DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
            int ncells = ds->GetNumberOfCells();
            vtkIdList *ptIds = vtkIdList::New();
            vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
            ghostCells->SetName("avtGhostZones");
            ghostCells->Allocate(ncells);
     
            for (int i = 0; i < ncells; i++)
            {
                ds->GetCellPoints(i, ptIds);
                bool ghost = false;
                for (int idx = 0; idx < ptIds->GetNumberOfIds(); idx++)
                    ghost |= ghostPoints[ptIds->GetId(idx)];

                if (ghost)
                    ghostCells->InsertNextValue(ghostVal);
                else
                    ghostCells->InsertNextValue(realVal);
     
            } 
            
            
            ds->GetCellData()->AddArray(ghostCells);
            delete [] ghostPoints;
            ghostCells->Delete();
            ptIds->Delete();

            vtkIntArray *realDims = vtkIntArray::New();
            realDims->SetName("avtRealDims");
            realDims->SetNumberOfValues(6);
            realDims->SetValue(0, first[0]);
            realDims->SetValue(1, last[0]);
            realDims->SetValue(2, first[1]);
            realDims->SetValue(3, last[1]);
            realDims->SetValue(4, first[2]);
            realDims->SetValue(5, last[2]);
            ds->GetFieldData()->AddArray(realDims);
            ds->GetFieldData()->CopyFieldOn("avtRealDims");
            realDims->Delete();
        }
    }
    ds->GetInformation()->Set(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);
}

// ****************************************************************************
// Method: GetQuadGhostNodesFromLabels
//
// Purpose: 
//   Creates array of ghost nodes from DBquadmesh::ghost_node_labels.
//
// Arguments:
//   DBquadmesh *qm : the mesh from which to source the data
//   vtkDataset *ds : the object that stores the array
//
// Programmer: Edward Rusu
// Creation:   Tue Sept 11 11:06:24 PDT 2018
// ****************************************************************************
void
avtSiloFileFormat::GetQuadGhostNodesFromLabels(DBquadmesh *qm,
                                               vtkDataSet *ds)
{
    // --- Populate ghostNodes --- //
    int numNodes = qm->nnodes;
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostNodeType(ghostVal, DUPLICATED_NODE);
    
    vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
    ghostNodes->SetName("avtGhostNodes");
    ghostNodes->Allocate(numNodes);
    for (int i = 0; i < numNodes; i++) {
        if (qm->ghost_node_labels[i] == DB_GHOSTTYPE_NOGHOST)
            ghostNodes->InsertNextValue(realVal);
        else if (qm->ghost_node_labels[i] == DB_GHOSTTYPE_INTDUP)
            ghostNodes->InsertNextValue(ghostVal);
    }
    
    // --- Populate Dataset --- //
    ds->GetPointData()->AddArray(ghostNodes);
    ghostNodes->Delete();
}

// ****************************************************************************
// Method: GetQuadGhostZonesFromLabels
//
// Purpose: 
//   Creates array of ghost zones from DBquadmesh::ghost_zone_labels.
//
// Arguments:
//   DBquadmesh *qm : the mesh from which to source the data
//   vtkDataset *ds : the object that stores the array
//
// Programmer: Edward Rusu
// Creation:   Tue Sept 11 13:50:24 PDT 2018
// ****************************************************************************
void
avtSiloFileFormat::GetQuadGhostZonesFromLabels(DBquadmesh *qm,
                                               vtkDataSet *ds)
{
    // --- Populate ghostZones --- //
    int numZones = ds->GetNumberOfCells();
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    
    vtkUnsignedCharArray *ghostZones = vtkUnsignedCharArray::New();
    ghostZones->SetName("avtGhostZones");
    ghostZones->Allocate(numZones);
    for (int i = 0; i < numZones; i++) {
        if (qm->ghost_zone_labels[i] == DB_GHOSTTYPE_NOGHOST)
            ghostZones->InsertNextValue(realVal);
        else if (qm->ghost_zone_labels[i] == DB_GHOSTTYPE_INTDUP)
            ghostZones->InsertNextValue(ghostVal);
    }
    
    // --- Populate Dataset --- //
    ds->GetCellData()->AddArray(ghostZones);
    ghostZones->Delete();
}

// ****************************************************************************
// Method: GetUcdGhostNodesFromLabels
//
// Purpose: 
//   Creates array of ghost nodes from DBucdmesh::ghost_node_labels.
//
// Arguments:
//   DBucdmesh *um : the mesh from which to source the data
//   vtkUnstructuredGrid *ugrid : the object that stores the array
//   std::vector<int> *cellRemap : vector that stores remapping information.
//
// Programmer: Edward Rusu
// Creation:   Tue Sept 11 14:05:24 PDT 2018
// ****************************************************************************
void
avtSiloFileFormat::GetUcdGhostNodesFromLabels(DBucdmesh *um,
                                              vtkUnstructuredGrid *ugrid,
                                              std::vector<int> *cellReMap)
{
    // --- Populate ghostNodes --- //
    int numNodes = um->nnodes;
    unsigned char *gvals = new unsigned char[numNodes];
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostNodeType(ghostVal, DUPLICATED_NODE);

    for (int i = 0; i < numNodes; i++) {
        if (um->ghost_node_labels[i] == DB_GHOSTTYPE_NOGHOST)
            gvals[i] = realVal;
        else if (um->ghost_node_labels[i] == DB_GHOSTTYPE_INTDUP)
            gvals[i] = ghostVal;    
    }

    // --- Create temporary DBucdvar --- //
    DBucdvar tmp;
    tmp.centering = DB_NODECENT;
    tmp.datatype = DB_CHAR;
    tmp.nels = numNodes;
    tmp.nvals = 1;
    tmp.vals = (void**) malloc(sizeof(void*));
    tmp.vals[0] = (void*) gvals;

    // --- Create ghostNodes --- //
    vector<int> noremap;
    vtkDataArray *ghostNodes = CopyAndPadUcdVar<unsigned char,
        vtkUnsignedCharArray>(&tmp, cellReMap?*cellReMap:noremap);
    free(tmp.vals);
    delete [] gvals;

    // --- Add Array to vtkDataSet --- //
    ghostNodes->SetName("avtGhostNodes");
    ugrid->GetPointData()->AddArray(ghostNodes);
    ghostNodes->Delete();
}

// ****************************************************************************
// Method: GetUcdGhostZonesFromLabels
//
// Purpose: 
//   Creates array of ghost zones from DBucdmesh::Dbzonelist::ghost_zone_labels.
//
// Arguments:
//   DBzonelist *zl : the list of zones to source data
//   vtkUnstructuredGrid *ugrid : the object that stores the array
//   std::vector<int> *cellRemap : vector that stores remapping information.
//
// Programmer: Edward Rusu
// Creation:   Tue Sept 11 14:14:24 PDT 2018
// ****************************************************************************
void
avtSiloFileFormat::GetUcdGhostZonesFromLabels(DBzonelist *zl,
                                              vtkUnstructuredGrid *ugrid,
                                              std::vector<int> *cellReMap)
{
    // --- Populate ghostZones --- //
    int numCells = zl->nzones;
    unsigned char *gvals = new unsigned char[numCells];
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    
    for (int i = 0; i < numCells; i++) {
        if (zl->ghost_zone_labels[i] == DB_GHOSTTYPE_NOGHOST)
            gvals[i] = realVal;
        else if (zl->ghost_zone_labels[i] == DB_GHOSTTYPE_INTDUP)
            gvals[i] = ghostVal;    
    }
    
    // --- Create temporary DBucdvar --- //
    DBucdvar tmp;
    tmp.centering = DB_ZONECENT;
    tmp.datatype = DB_CHAR;
    tmp.nels = numCells;
    tmp.nvals = 1;
    tmp.vals = (void**) malloc(sizeof(void*));
    tmp.vals[0] = (void*) gvals;
    
    // --- Create ghostZones --- //
    vector<int> noremap;
    vtkDataArray *ghostZones = CopyAndPadUcdVar<unsigned char,
        vtkUnsignedCharArray>(&tmp, cellReMap ? *cellReMap : noremap);
    free(tmp.vals);
    delete [] gvals;
    
    // --- Add Array to vtkDataSet --- //
    ghostZones->SetName("avtGhostZones");
    ugrid->GetCellData()->AddArray(ghostZones);
    ghostZones->Delete();
}

// ****************************************************************************
// Method: GetUcdPolyhedralGhostZonesFromLabels
//
// Purpose: 
//   Creates array of ghost zones from DBucdmesh::Dbphzonelist::ghost_zone_labels.
//
// Arguments:
//   DBphzonelist *pzl : the list of polyhedral zones to source ghost data.
//   DBucdmesh *um : the mesh from which to source the data
//   vtkUnstructuredGrid *ugrid : the object that stores the array
//
// Programmer: Edward Rusu
// Creation:   Tue Sept 11 14:14:24 PDT 2018
// ****************************************************************************
void
avtSiloFileFormat::GetUcdPolyhedralGhostZonesFromLabels(DBphzonelist *pzl,
                                              vtkUnstructuredGrid *ugrid,
                                              std::vector<int> *cellReMap)
{
    // --- Populate ghostZones --- //
    int numCells = pzl->nzones;
    unsigned char *gvals = new unsigned char[numCells];
    unsigned char realVal = 0;
    unsigned char ghostVal = 0;
    avtGhostData::AddGhostZoneType(ghostVal, DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    
    for (int i = 0; i < numCells; i++) {
        if (pzl->ghost_zone_labels[i] == DB_GHOSTTYPE_NOGHOST)
            gvals[i] = realVal;
        else if (pzl->ghost_zone_labels[i] == DB_GHOSTTYPE_INTDUP)
            gvals[i] = ghostVal;    
    }
    
    // --- Create temporary DBucdvar --- //
    DBucdvar tmp;
    tmp.centering = DB_ZONECENT;
    tmp.datatype = DB_CHAR;
    tmp.nels = numCells;
    tmp.nvals = 1;
    tmp.vals = (void**) malloc(sizeof(void*));
    tmp.vals[0] = (void*) gvals;
    
    // --- Create ghostZones --- //
    vector<int> noremap;
    vtkDataArray *ghostZones = CopyAndPadUcdVar<unsigned char,
        vtkUnsignedCharArray>(&tmp, cellReMap ? *cellReMap : noremap);
    free(tmp.vals);
    delete [] gvals;
    
    // --- Add Array to vtkDataSet --- //
    ghostZones->SetName("avtGhostZones");
    ugrid->GetCellData()->AddArray(ghostZones);
    ghostZones->Delete();
}

// ****************************************************************************
// Method: CopyPointMeshCoordinates
//
// Purpose: 
//   Copies DBpointmesh coordinates into an interleaved form for vtkPoints.
//
// Arguments:
//   T : The destination array for the point data.
//   pm : The source point mesh.
//
// Returns:    
//
// Note:       I moved this block from avtSiloFileFormat::GetPointMesh and
//             templated it.
//
// Programmer: Brad Whitlock
// Creation:   Thu Aug  6 11:48:54 PDT 2009
//
// Modifications:
//   
//    Mark C. Miller, Tue Oct 20 16:51:18 PDT 2009
//    Made it static.
// ****************************************************************************

template <typename T>
static void
CopyPointMeshCoordinates(T *pts, const DBpointmesh *pm)
{
    for (int i = 0 ; i < 3 ; i++)
    {
        T *tmp = pts + i;
        if (pm->coords[i] != NULL)
        {
            for (int j = 0 ; j < pm->nels ; j++)
            {
                *tmp = ((T**)pm->coords)[i][j];
                tmp += 3;
            }
        }
        else
        {
            for (int j = 0 ; j < pm->nels ; j++)
            {
                *tmp = 0.;
                tmp += 3;
            }
        }
    }
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetPointMesh
//
//  Purpose:
//      Gets the Silo construct for mn and creates a vtkDataSet for it.
//
//  Arguments:
//      dbfile    A handle to the file this variable lives in.
//      mn        The name of the point mesh.
//      domain    The domain
//
//  Returns:      The vtkDataSet for mn.
//
//  Programmer:   Hank Childs
//  Creation:     June 22, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Hank Childs, Sat Jun 22 19:53:24 PDT 2002
//    Finally populated routine.
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Thu Aug  6 11:50:13 PDT 2009
//    I added support for double coordinates.
//
//    Brad Whitlock, Thu Sep 13 16:13:03 PDT 2012
//    I added support for global node ids.
//
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetPointMesh(DBfile *dbfile, const char *mn, int domain)
{
    //
    // Allow empty data sets
    //
    if (string(mn) == "EMPTY")
        return NULL;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *meshname  = const_cast<char *>(mn);

    //
    // Get the Silo construct.
    //
    DBpointmesh  *pm = DBGetPointmesh(dbfile, meshname);
    if (pm == NULL)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }
    else if (DBIsEmptyPointmesh(pm))
    {
        DBFreePointmesh(pm);
        return NULL;
    }

    if(pm->datatype != DB_FLOAT && pm->datatype != DB_DOUBLE)
    {
        DBFreePointmesh(pm);
        EXCEPTION1(InvalidVariableException, "The Silo reader expects float or "
            "double precision coordinates for point meshes.");
    }

    vtkPoints *points  = vtkPoints::New();

    //
    // vtkPoints assumes float data type
    //
    if (pm->datatype == DB_DOUBLE)
        points->SetDataTypeToDouble();

    //
    // Populate the coordinates.  Put in 3D points with z=0 if the mesh is 2D.
    //
    points->SetNumberOfPoints(pm->nels);
    if(pm->datatype == DB_DOUBLE)
        CopyPointMeshCoordinates((double *)points->GetVoidPointer(0), pm);
    else
        CopyPointMeshCoordinates((float *)points->GetVoidPointer(0), pm);

    //
    // Create the VTK objects and connect them up.
    //
    vtkUnstructuredGrid    *ugrid   = vtkUnstructuredGrid::New(); 
    ugrid->SetPoints(points);
    ugrid->Allocate(pm->nels);
    vtkIdType onevertex[1];
    for (int i = 0 ; i < pm->nels ; i++)
    {
        onevertex[0] = i;
        ugrid->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    //
    // If we have global node ids, set them up and cache 'em
    //
    if (pm->gnodeno != NULL)
    {
        vtkDataArray *arr = CreateDataArray(pm->gnznodtype, pm->gnodeno, pm->nels); 
        pm->gnodeno = 0; // vtkDataArray now owns the data.

        //
        // Cache this VTK object but in the VoidRefCache, not the VTK cache
        // so that it can be obtained through the GetAuxiliaryData call
        //
        void_ref_ptr vr = void_ref_ptr(arr, avtVariableCache::DestructVTKObject);
        cache->CacheVoidRef(meshname, AUXILIARY_DATA_GLOBAL_NODE_IDS, timestep, 
                            domain, vr);
    }
    
    // --- Read Ghost Node Labels --- //
    if (pm->ghost_node_labels) {
        int numNodes = pm->nels;
        unsigned char realVal = 0;
        unsigned char ghostVal = 0;
        avtGhostData::AddGhostNodeType(ghostVal, DUPLICATED_NODE);
        
        vtkUnsignedCharArray *ghostNodes = vtkUnsignedCharArray::New();
        ghostNodes->SetName("avtGhostNodes");
        ghostNodes->Allocate(numNodes);
        for (int i = 0; i < numNodes; i++) {
            if (pm->ghost_node_labels[i] == DB_GHOSTTYPE_NOGHOST)
                ghostNodes->InsertNextValue(realVal);
            else if (pm->ghost_node_labels[i] == DB_GHOSTTYPE_INTDUP)
                ghostNodes->InsertNextValue(ghostVal);
        }
        
        ugrid->GetPointData()->AddArray(ghostNodes);
        ghostNodes->Delete();
    }

    points->Delete();
    DBFreePointmesh(pm);
    return ugrid;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetCSGMesh
//
//  Purpose:
//      Gets the Silo construct for a CSG mesh and creates a vtkDataSet for it.
//
//  Arguments:
//      dbfile    A handle to the file this variable lives in.
//      mn        The name of the csg mesh.
//
//  Returns:      The vtkDataSet for mn.
//
//  Programmer:   Mark C. Miller 
//  Creation:     August 8, 2005 
//
//  Modifications:
//  
//    Mark C. Miller, Wed Nov 16 10:46:36 PST 2005
//    Added some more primitives. Moved discretization calls to
//    generic database
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Changed interface to vtkCSGGrid class to accept raw Silo representation
//
//    Mark C. Miller, Thu Aug 14 19:52:12 PDT 2008
//    Added code to explicitly cache csg grid objects from within the plugin.
//    Note that the plugin is caching its objects in the 'cache' object but
//    is doing so in a 'place' that VisIt itself can never find due to 
//    different name of the 'type' of the cache.
//
//    Mark C. Miller, Wed Aug 20 11:39:39 PDT 2008
//    Had to force a copy on the csg object returned here to prevent collisions
//    in the variable cache with stuff that generic database is doing.
// ****************************************************************************

vtkDataSet *
avtSiloFileFormat::GetCSGMesh(DBfile *dbfile, const char *mn, int dom)
{
#ifdef MDSERVER
    return 0;
#else
    //
    // Allow empty data sets
    //
    if (string(mn) == "EMPTY")
        return NULL;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *meshname  = const_cast<char *>(mn);

    //
    // Check in cache for this domain before trying to read it.
    // We have to explicitly handle caching of CSG meshes here in the plugin
    // because of single vtkCSGGrid object maps to many VisIt domains. Also,
    // we cache the object using a 'type' (SILO_CSG_GRID) that is unique to
    // the SILO plugin so that VisIt's Generic Database class will never 
    // find it.
    //
    vtkCSGGrid *cached_csggrid = (vtkCSGGrid*) cache->GetVTKObject(meshname,
        "SILO_CSG_GRID", timestep, dom, "none");
    if (cached_csggrid)
    {
        debug5 << "Avoiding re-read of CSG grid by returning cached grid: name=\""
               << meshname << "\", d=" << dom << ", t=" << timestep << endl;
        vtkCSGGrid *cache_copy = vtkCSGGrid::New(); 
        cache_copy->ShallowCopy(cached_csggrid);
        return cache_copy;
    }

    //
    // Get the Silo construct.
    //
    DBcsgmesh *csgm = DBGetCsgmesh(dbfile, meshname);
    if (csgm == NULL)
    {
        EXCEPTION1(InvalidVariableException, meshname);
    }
    else if (DBIsEmptyCsgmesh(csgm))
    {
        DBFreeCsgmesh(csgm);
        return NULL;
    }

    //
    // Create the VTK objects and connect them up.
    //
    vtkCSGGrid *csggrid   = vtkCSGGrid::New(); 

    double minX = -10.0, minY = -10.0, minZ = -10.0;
    double maxX =  10.0, maxY =  10.0, maxZ =  10.0;

    // set bounds *before* anything else
    if (!((csgm->min_extents[0] == 0.0 && csgm->max_extents[0] == 0.0 &&
           csgm->min_extents[1] == 0.0 && csgm->max_extents[1] == 0.0 &&
           csgm->min_extents[2] == 0.0 && csgm->max_extents[2] == 0.0) ||
          (csgm->min_extents[0] == -DBL_MAX && csgm->max_extents[0] == DBL_MAX &&
           csgm->min_extents[1] == -DBL_MAX && csgm->max_extents[1] == DBL_MAX &&
           csgm->min_extents[2] == -DBL_MAX && csgm->max_extents[2] == DBL_MAX)))
    {
        minX = csgm->min_extents[0];
        maxX = csgm->max_extents[0];
        minY = csgm->min_extents[1];
        maxY = csgm->max_extents[1];
        minZ = csgm->min_extents[2];
        maxZ = csgm->max_extents[2];
    }
    csggrid->SetBounds(minX, maxX, minY, maxY, minZ, maxZ);

    if (csgm->datatype == DB_DOUBLE)
        csggrid->AddBoundaries(csgm->nbounds, csgm->typeflags, csgm->lcoeffs, (double*) csgm->coeffs);
    else
        csggrid->AddBoundaries(csgm->nbounds, csgm->typeflags, csgm->lcoeffs, (float*) csgm->coeffs);
    csggrid->AddRegions(csgm->zones->nregs, csgm->zones->leftids, csgm->zones->rightids,
                        csgm->zones->typeflags, 0, 0);
    csggrid->AddZones(csgm->zones->nzones, csgm->zones->zonelist);


    DBFreeCsgmesh(csgm);

    //
    // Cache this csggrid object. We'll probably be asked to read this same one
    // many times, each for a different CSG region but all part of the same vtkCSGGrid
    // object.  So, we have to explicitly handle caching of CSG meshes here in the plugin
    // because of single vtkCSGGrid object maps to many VisIt domains. Also,
    // we cache the object using a 'type' (SILO_CSG_GRID) that is unique to
    // the SILO plugin so that VisIt's Generic Database class will never 
    // find it.
    //
    cache->CacheVTKObject(meshname, "SILO_CSG_GRID", timestep, dom, "none", csggrid);

    vtkCSGGrid *csgcopy = vtkCSGGrid::New(); 
    csgcopy->ShallowCopy(csggrid);
    return csgcopy;

    return 0;
#endif
}

// ****************************************************************************
//  Method: avtSiloFileFormat::DetermineFilenameAndDirectory
//
//  Purpose:
//      Parses a string from a Silo file that has form "filename:directory/var"
//      and determines which part is "filename" and which is "directory/var".
//
//      input       The input string ("filename:directory/var")
//      filename    The "filename" part of the string.  This will copy a string
//                  into a buffer, so a buffer must be provided as input.
//      location    The "directory/var" part of the string.  This will just
//                  point into "input" so there is no memory management to
//                  worry about.
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. In this case, the location return had to be constructed
//    and allocated. So, needed to add bool indicating that.
//
//    Mark C. Miller, Thu Sep 13 20:51:56 PDT 2007
//    Take care not to preprend mdirname if it 'looks' like it already has
//    it. This logic isn't foolproof. Its a heuristic that should work 'most'
//    of the time. We have no way of knowing for sure if the string
//    'foo/foo/bar' is really intended or not.
//
//    Cyrus Harrison, Thu Dec 22 09:24:38 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Cyrus Harrison, Wed Jan 18 08:40:38 PST 2012
//    Fix string compare to examine the proper substring.
//
// ****************************************************************************

void
avtSiloFileFormat::DetermineFilenameAndDirectory(const char *input,
                                                 const char *mesh_dirname,
                                                 string &filename_out,
                                                 string &location_out)
{
    location_out = "";
    filename_out = "";
    //
    // Figure out if there is a ':' in the string.
    //

    string input_std = string(input);
    string mesh_dir_std = string(mesh_dirname);
    size_t colon_pos =  input_std.find(":");

    if (colon_pos == string::npos)
    {
        //
        // There is no colon, so the variable must be in the current file.
        // Leave the file handle alone.
        //
        filename_out = ".";
        if ( mesh_dir_std == ""  ||
            input_std == "EMPTY" ||
            input[0] == '/' || (input[0] == '.' && input[1] == '/'))
        {
            location_out = string(input);
        }
        else
        {
            // check if input_std doesn't start with mesh_dir_std
            if ( input_std.compare(0, mesh_dir_std.length(), mesh_dir_std) != 0)
            {
                location_out = mesh_dir_std + string("/") +  input_std;
            }
            else
            {
                location_out = input_std;
            }
        }
    }
    else
    {
        //
        // Make a copy of the string all the way up to the colon.
        //
        filename_out = input_std.substr(0,colon_pos);
        //
        // The location of the variable is the substr after the colon.
        //
        location_out = input_std.substr(colon_pos+1);
    }
}



// ****************************************************************************
//  Method: avtSiloFileFormat::DetermineFileAndDirectory
//
//  Purpose:
//      Takes a string from a Silo file that as "filename:directory/var" and
//      returns the correct file as well as the substring that is only
//      directory and var.
//
//  Arguments:
//      input       The input string ("filename:directory/var")
//      cFile       The correct file corresponding to filename.
//      location    The "directory/var" part of the string.  This will just
//                  point into the input so there is no memory management to
//                  worry about.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2001
//
//  Modifications:
//
//    Hank Childs, Fri May 11 14:40:36 PDT 2001
//    Call DetermineFilenameAndDirectory instead of calculating it ourselves.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Added optional bool to tell OpenFile to skip global info as a
//    pre-caution against opening the tocIndex file for the first time and
//    winding up attempting to engage in collective communication for global
//    information that other processors can't be guarenteed of reaching.
//
//    Mark C. Miller, Tue Aug 28 19:17:44 PDT 2007
//    Made it deal with case where multimesh and blocks are all in same
//    dir in the file. In this case, the location return had to be constructed
//    and allocated. So, needed to add bool indicating that.
//
//    Kathleen Bonnell, Wed Jul 2 14:43:22 PDT 2008
//    Removed unreferenced variables.
//
//    Cyrus Harrison, Thu Dec 22 09:24:38 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

void
avtSiloFileFormat::DetermineFileAndDirectory(const char *input,
                                             const char *mesh_dirname,
                                             DBfile    *&dbfile_out,
                                             string     &location_out)
{
    string filename;
    DetermineFilenameAndDirectory(input, mesh_dirname, filename, location_out);
    if (filename != string("."))
    {
        //
        // The variable is in a different file, so open that file.  This will
        // create the filename and add it to our registry if necessary.
        //
        // Since this call to OpenFile is made from within a GetMesh or GetVar
        // call, it cannot wind up causing the plugin to engage in collective
        // communication for global information. Typically, the tocIndex file
        // would have already been opened by this point in execution. However,
        // telling OpenFile to explicitly skip collective communication removes
        // all doubt.
        //
        bool skipGlobalInfo = true;
        dbfile_out = OpenFile(filename.c_str(), skipGlobalInfo);
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetRelativeVarName
//
//  Purpose:
//      If a variable (like "/block1/mesh") has pointers to other variables
//      (like "fl") then they come unqualified with a path (should be
//      "/block1/fl").  This routine qualifies the variable.
//
//  Arguments:
//      initVar     The original variable ("/block1/mesh").
//      newVar      The variable initVar points to ("fl").
//      outVar      The relative variable name for newVar ("/block1/fl").
//
//  Programmer: Hank Childs
//  Creation:   March 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 10:54:06 PST 2001
//    Handle newVars that have a prepended path already gracefully.
//
//    Hank Childs, Sat Jun 15 13:12:18 PDT 2002
//    Respect built in filenames (identified with colons).
//
//    Jeremy Meredith, Tue Sep 13 15:58:10 PDT 2005
//    Make sure we add the filename to an otherwise fully-qualified variable
//    name.
//
// ****************************************************************************

void
avtSiloFileFormat::GetRelativeVarName(const char *initVar, const char *newVar,
                                      char *outVar)
{
    //
    // If the new variable starts with a slash, it is already qualified, so
    // just return that.
    //
    int len = strlen(initVar);
    if (newVar[0] == '/')
    {
        int colonPosition = -1;
        for (int i = 0 ; i < len; i++)
        {
            if (initVar[i] == ':')
            {
                colonPosition = i;
                break;
            }
        }
        int numToCopy = (colonPosition < 0 ? 0 : colonPosition+1);
        strncpy(outVar, initVar, numToCopy);
        strcpy(outVar+numToCopy, newVar);
        return;
    }

    int lastToken = -1;
    for (int i = len-1 ; i >= 0 ; i--)
    {
        if (initVar[i] == '/' || initVar[i] == ':')
        {
            lastToken = i;
            break;
        }
    }

    int numToCopy = (lastToken < 0 ? 0 : lastToken+1);
    strncpy(outVar, initVar, numToCopy);
    strcpy(outVar+numToCopy, newVar);
}


// ****************************************************************************
//  Method: avtSiloFileFormat::DetermineMultiMeshForSubVariable
//
//  Purpose:
//      Multi-vars are not defined on multi-meshes, so the only way to
//      determine which mesh a var is defined on is to look at the first sub
//      variable and see what mesh it is and compare that to the first
//      sub mesh of the multi-mesh.  This is not foolproof: it will fail
//      for multi-meshes that share submeshes.
//
//  Arguments:
//      dbfile     The currently open Silo file.
//      varname    The name of the sub-var.
//      nblocks    The number of blocks for 'varname'.
//
//  Returns:       The name of the multi-mesh that the multi-var is defined on.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Nov  2 15:36:31 PST 2001
//    Added checks to make sure the number of blocks match up.
//
//    Hank Childs, Mon Dec  9 16:20:15 PST 2002
//    Throw an exception and let the levels above this routine handle it.
//
//    Jeremy Meredith, Wed Mar 19 12:27:41 PST 2003
//    Added some logic to handle the case where a multi-var is only
//    defined on some domains of its associated multi-mesh.
//
//    Jeremy Meredith, Tue Jun  7 10:55:18 PDT 2005
//    Allowed EMPTY domains.
//
//    Mark C. Miller, Fri Nov 11 09:45:42 PST 2005
//    Added code to try an exact match with the leading slash, if one exists,
//    removed from the meshname. This is to work around a bug in data files
//    generated with earlier versions of the HDF5 driver.
//
//    Mark C. Miller, Wed Dec 13 16:55:30 PST 2006
//    Added code to use explicit multivar to multimesh mapping information,
//    when available.
//
//    Mark C. Miller, Tue Feb  6 19:39:35 PST 2007
//    Added Brad's fix for reducing large amount of string matching in 
//    'fuzzy' matching logic. Also added matching on block counts.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Thu Jun  8 14:52:32 PDT 2017
//    Fix logic seeking first non-empty block to not make more than 20 attempts
//    in the namescheme'd (implicit) case.
// ****************************************************************************

string
avtSiloFileFormat::DetermineMultiMeshForSubVariable(DBfile *dbfile,
                                                    const char *name,
                                                    avtSiloMBObjectCacheEntry *obj,
                                                    const char *curdir)
{
    size_t i;
    char subMesh[256];
    char subMeshTmp[256];

    //
    // First, see if we've got the answer in the multivarToMultimeshMap
    //
    map<string,string>::const_iterator cit = multivarToMultimeshMap.find(name);
    if (cit != multivarToMultimeshMap.end())
    {
        debug5 << "Matched multivar \"" << name << "\" to multimesh \""
               << cit->second << "\" using multivarToMultimeshMap" << endl;
        return cit->second;
    }

    // Find the first non-empty mesh
    int meshnum = 0;
    string mb_varname = obj->GenerateName(meshnum);
    int nblocks = obj->NumberOfBlocks();
    
    if (obj->IsExplicit())
    {
        while (mb_varname == "EMPTY")
        {
            meshnum++;
            if (meshnum >= nblocks)
            {
                EXCEPTION1(InvalidVariableException,  name);
            }
            mb_varname = obj->GenerateName(meshnum);
        }
        GetMeshname(dbfile, mb_varname.c_str(), subMesh);
    }
    else
    {
        while (GetMeshname(dbfile, mb_varname.c_str(), subMesh) != 0)
        {
            meshnum++;
            if (meshnum >= nblocks)
            {
                EXCEPTION1(InvalidVariableException,  name);
            }
            else if (meshnum > 20)
            {
                char str[1024];
                SNPRINTF(str, sizeof(str), "Giving up after 20 attempts to find first non-empty submesh "
                    "of a namescheme'd multivar \"%s\". This typically leads to the variable being invalidated "
                    "(grayed out) in the GUI.", name);
                EXCEPTION1(SiloException, str);
            }
            mb_varname = obj->GenerateName(meshnum);
        }
    }

    //
    // The code involving subMeshTmp is to maintain backward compability
    // with Silo/HDF5 files in which HDF5 driver had a bug in that it
    // *always* added a leading slash to the name of the mesh associated
    // with an object. Eventually, this code can be eliminated
    //
    if (subMesh[0] == '/')
    {
        for (i = 0; i < strlen(subMesh); i++)
            subMeshTmp[i] = subMesh[i+1];
    }

    //
    // varname is very likely qualified with a file name.  We need to figure
    // out what it's mesh's name looks like the prepended file name, so we can
    // meaningfully compare it with our list of submeshes.
    //
    char subMeshWithFile[1024];
    char subMeshWithFileTmp[1024];
    GetRelativeVarName(mb_varname.c_str(), subMesh, subMeshWithFile);
    if (subMesh[0] == '/')
        GetRelativeVarName(mb_varname.c_str(), subMeshTmp, subMeshWithFileTmp);

    //
    // Attempt an "exact" match, where the first mesh for the multivar is
    // an exact match and the number of domains is the same.
    //
    size_t size = actualMeshName.size();
    for (i = 0 ; i < size ; i++)
    {
        if (firstSubMesh[i] == subMeshWithFile && nblocks == blocksForMesh[i])
        {
            return actualMeshName[i];
        }
    }
    if (subMesh[0] == '/')
    {
        for (i = 0 ; i < size ; i++)
        {
            if (firstSubMesh[i] == subMeshWithFileTmp && nblocks == blocksForMesh[i])
            {
                return actualMeshName[i];
            }
        }
    }

    //
    // Couldn't find an exact match, so try something fuzzier:
    // Look for a multimesh which has the same name as the mesh for
    // the multivar, and match up domains by directory name.
    //
    debug5 << "Using fuzzy logic to match multivar \"" << name << "\" to a multimesh" << endl;
    string dir,varmesh;
    SplitDirVarName(subMesh, curdir, dir, varmesh);
    for (i = 0 ; i < (size_t)size ; i++)
    {
        if (firstSubMeshVarName[i] == varmesh &&
            blocksForMesh[i] == nblocks)
        {
#ifndef MDSERVER

            string *dirs = new string[nblocks];
            for (int k = 0; k < nblocks; k++)
                SplitDirVarName(obj->GenerateName(k).c_str(), curdir, dirs[k], varmesh);

            for (size_t j = 0; j < allSubMeshDirs[i].size(); j++)
            {
                int match = -1;
                for (int k = 0; k < nblocks && match == -1; k++)
                {
                    if (dirs[k] == allSubMeshDirs[i][j])
                    {
                        match = k;
                    }
                }
                blocksForMultivar[name].push_back(match);
            }

            delete [] dirs;

#endif
            return actualMeshName[i];
        }
    }

    //
    // We weren't able to find a match -- throw an exception and let the
    // levels above us determine what the right thing to do is.
    //
    char str[1024];
    SNPRINTF(str, sizeof(str), "Was not able to match multivar \"%s\" and its first \n" 
                 "non-empty submesh \"%s\" in file %s to a multi-mesh.\n"
                 "This typically leads to the variable being invalidated\n"
                 "(grayed out) in the GUI",
              name, mb_varname.c_str(), subMeshWithFile);
    EXCEPTION1(SiloException, str);
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetMeshname
//
//  Purpose:
//      Gets the mesh name of a variable, even if it is in a different file.
//
//  Arguments:
//      dbfile    The dbfile that mesh came from.
//      var       A var name, possibly with a prepended directory and filename.
//      meshname  A place to put the name of the mesh.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2001
//
//  Modification:
//
//    Hank Childs, Thu Aug 16 11:06:27 PDT 2001
//    Throw an exception here instead of in the handler.
//
//    Mark C. Miller, Tue Sep 13 14:25:42 PDT 2005
//    Permit absolute pathnames for meshes (leading slashes)
//
//    Mark C. Miller, Thu Nov 10 21:12:36 PST 2005
//    Undid above change.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

int
avtSiloFileFormat::GetMeshname(DBfile *dbfile, const char *var, char *meshname)
{
    string dirvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(var, "", correctFile, dirvar);
    int rv = DBInqMeshname(correctFile, dirvar.c_str(), meshname);
    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetComponent
//
//  Purpose:
//      Gets the component for a variable, even if it is in a different file.
//
//  Arguments:
//      dbfile    The dbfile that mesh came from.
//      var       A var name, possibly with a prepended directory and filename.
//      compname  The name of the component.
//
//  Returns:    The return value of the DBGetComponent call, this should be
//              free'd by the calling function.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2001
//
//  Modification:
//
//    Hank Childs, Thu Aug 16 11:06:27 PDT 2001
//    Throw an exception here instead of in the handler.
//
//    Jeremy Meredith, Thu Aug  7 16:16:52 EDT 2008
//    Accept const char*'s as input.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

void *
avtSiloFileFormat::GetComponent(DBfile *dbfile, char *var,
                                const char *compname)
{
    string dirvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(var,"", correctFile, dirvar);
    void *rv = DBGetComponent(correctFile, dirvar.c_str(), compname);
    if (rv == NULL && strcmp(compname, "facelist") != 0)
    {
        char str[1024];
        sprintf(str, "Unable to get component '%s' for variable '%s'",
                compname, var);
        EXCEPTION1(SiloException, str);
    }
    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Gets the auxiliary data from a Silo file.
//
//  Arguments:
//      var        The variable of interest.
//      domain     The domain of interest.
//      type       The type of auxiliary data.
//      <unnamed>  The arguments for that -- not used for any Silo types.
//
//  Returns:    The auxiliary data.  Throws an exception if this is not a
//              supported data type.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Thu Dec 13 17:46:46 PST 2001
//    Added species support.
//
//    Hank Childs, Wed Sep 25 08:49:34 PDT 2002
//    Add destructor functions.
//
//    Mark C. Miller, August 9, 2004
//    Added code to read global node and zone ids 
//
//    Mark C. Miller, Mon Oct 18 13:02:37 PDT 2004
//    Added support for data/spatial extents
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Added code to convert CSG domain id; no-op for other meshes
//
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Removed disablement of spatial and data extents (for Ale3d)
//
//    Mark C. Miller, Tue Jun 10 22:36:25 PDT 2008
//    Added logic to ignore extents
//
//    Eric Brugger, Wed Nov 19 08:39:15 PST 2014
//    I reduced the number of reads of CSG meshes to only once per CSG mesh
//    instead of once per region in order to reduce the number of times the
//    same CSG mesh was cached. Typically there is one CSG mesh with many
//    regions, so this is a significant saving. CSG meshes with thousands
//    of regions were exhausting memory in the previous scheme.
//
// ****************************************************************************

void *
avtSiloFileFormat::GetAuxiliaryData(const char *var, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    void *rv = NULL;

    // If the named object is known to be empty, return immediately
    if (emptyObjectsList.find(var) != emptyObjectsList.end()) return rv;

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        rv = (void *) GetMaterial(domain, var);
        df = avtMaterial::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_SPECIES) == 0)
    {
        rv = (void *) GetSpecies(domain, var);
        df = avtSpecies::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_EXTERNAL_FACELIST) == 0)
    {
        rv = (void *) GetExternalFacelist(domain, var);
        df = avtFacelist::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_NODE_IDS) == 0)
    {
        rv = (void *) GetGlobalNodeIds(domain, var);
        df = avtVariableCache::DestructVTKObject;
    }
    else if (strcmp(type, AUXILIARY_DATA_GLOBAL_ZONE_IDS) == 0)
    {
        rv = (void *) GetGlobalZoneIds(domain, var);
        df = avtVariableCache::DestructVTKObject;
    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (ignoreSpatialExtents)
        {
            debug1 << "Read options ignore request for spatial extents" << endl;
            return 0;
        }
        rv = (void *) GetSpatialExtents(var);
        df = avtIntervalTree::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_DATA_EXTENTS) == 0)
    {
        if (ignoreDataExtents)
        {
            debug1 << "Read options ignore request for data extents" << endl;
            return 0;
        }
        rv = (void *) GetDataExtents(var);
        df = avtIntervalTree::Destruct;
    }
    else if (strcmp(type, AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION) == 0)
    {
        debug1  << "GetAuxiliaryData Local::AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION"  <<endl;
        rv = GetLocalDomainBoundaryInfo(domain,var);
        df = avtLocalStructuredDomainBoundaryList::Destruct;
    }

    //
    // Note -- although the Silo file format can get mixed variables, it does
    // not return them this way.  It instead caches them when it does a normal
    // variable read.  It does not try to get them here for performance
    // reasons.
    //

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMaterial
//
//  Purpose:
//      Gets the material from the Silo file and converts it into an
//      avtMaterial.
//
//  Arguments:
//      dom     The domain of the material.
//      mat     The material we want.
//
//  Returns:    The material information.
//
//  Programmer: Hank Childs
//  Creation:   November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, made it work with Silo objects
//    distributed across multiple files.
//
//    Hank Childs, Wed Jan 14 12:04:19 PST 2004
//    Avoid redundant DBGetMultimat calls.
//
//    Hank Childs, Fri Feb 13 17:18:04 PST 2004
//    Add the domain to the CalcMaterial call.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
// ****************************************************************************

avtMaterial *
avtSiloFileFormat::GetMaterial(int dom, const char *mat)
{
    debug5 << "Reading in domain " << dom << ", material " << mat << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // Get the file handle, throw an exception if it hasn't already been opened
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // Silo can't accept consts, so cast it away.
    //
    char *m = const_cast< char * >(mat);

    //
    // Start off by finding out what kind of mat it is.  Note: we have
    // already cached the multimats.  See if we have a multimat in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //
    DBmultimat *mm = NULL;
    avtSiloMultiMatCacheEntry *mm_ent = QueryMultiMat("", m);

    if(mm_ent != NULL)
        mm = mm_ent->DataObject();

    int type;
    if (mm != NULL)
        type = DB_MULTIMAT;
    else
        type = DBInqVarType(dbfile, m);

    if (type != DB_MULTIMAT && type != DB_MATERIAL)
    {
        //
        // This is not a material or a multi-mat.  This is probably an error.
        //
        return NULL;
    }

    string mb_matname;

    if (type == DB_MULTIMAT)
    {
        if (mm == NULL)
        {
            GetMultiMat("", m, &mm_ent);
            if(mm_ent != NULL)
                mm = mm_ent->DataObject();
        }
        if (mm == NULL)
            EXCEPTION1(InvalidVariableException, m);
        if (dom >= mm->nmats || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, mm->nmats);
        }

        mb_matname = mm_ent->GenerateName(dom);
        if ( mb_matname == "EMPTY")
            return NULL;
    }
    else // (type == DB_MATERIAL)
    {
        if (dom != 0)
        {
            EXCEPTION2(BadDomainException, dom, 1);
        }
        mb_matname = string(mat);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_mat;
    DetermineFileAndDirectory(mb_matname.c_str(),"", domain_file, directory_mat);

    avtMaterial *rv = CalcMaterial(domain_file, directory_mat.c_str(), mat, dom);

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetSpecies
//
//  Purpose:
//      Gets the species from the Silo file and converts it into an
//      avtSpecies.
//
//  Arguments:
//      dom     The domain of the material.
//      spec    The species we want.
//
//  Returns:    The species information.
//
//  Programmer: Jeremy Meredith
//  Creation:   December 13, 2001
//
//  Modifications:
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Jeremy Meredith, Tue Jun  7 08:32:46 PDT 2005
//    Added support for "EMPTY" domains in multi-objects.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Cyrus Harrison, Tue Jan 24 11:56:17 PST 2012
//    Fetch and catch multispec object if not already seen.
// ****************************************************************************

avtSpecies *
avtSiloFileFormat::GetSpecies(int dom, const char *spec)
{
    debug5 << "Reading in domain " << dom << ", species " << spec << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // Get the file handle, throw an exception if it hasn't already been opened
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // Silo can't accept consts, so cast it away.
    //
    char *s = const_cast< char * >(spec);

    //
    // Start off by finding out what kind of species it is.  Note: we have
    // already cached the multispecies.  See if we have a multispecies in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //
    DBmultimatspecies *ms = NULL;
    avtSiloMultiSpecCacheEntry *ms_ent = QueryMultiSpec("", s);
    if(ms_ent != NULL)
        ms = ms_ent->DataObject();

    int type;
    if (ms != NULL)
        type = DB_MULTIMATSPECIES;
    else
        type = DBInqVarType(dbfile, s);

    if (type != DB_MULTIMATSPECIES && type != DB_MATSPECIES)
    {
        //
        // This is not a species or a multi-species. This is probably an error.
        //
        return NULL;
    }

    string specname = "";

    if (type == DB_MULTIMATSPECIES)
    {
        if (ms_ent == NULL)
        {
            GetMultiSpec("", s, &ms_ent);
            if(ms_ent != NULL)
                ms = ms_ent->DataObject();
        }
        if (ms == NULL)
            EXCEPTION1(InvalidVariableException, s);
        if (dom >= ms->nspec || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, ms->nspec);
        }

        specname = ms_ent->GenerateName(dom);
        if (specname ==  "EMPTY")
            return NULL;
    }
    else // (type == DB_MATSPECIES)
    {
        if (dom != 0)
        {
            EXCEPTION2(BadDomainException, dom, 1);
        }
        specname = string(spec);
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_spec;
    DetermineFileAndDirectory(specname.c_str(), "", domain_file, directory_spec);
    avtSpecies *rv = CalcSpecies(domain_file, directory_spec.c_str());

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::AllocAndDetermineMeshnameForUcdmesh
//
//  Purpose:
//      Allocate space for and determine the real mesh name for a ucd mesh,
//      which may be a multimesh 
//
//  Arguments:
//      dom     The domain of the mesh.
//      mesh    The mesh we want the real name for 
//
//  Returns:    The real mesh name 
//
//  Programmer: Mark C. Miller 
//  Creation:   August 4, 2004
//
//  Modifications:
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

char *
avtSiloFileFormat::AllocAndDetermineMeshnameForUcdmesh(int dom, const char *mesh)
{
    //
    // Get the file handle, throw an exception if it hasn't already been opened
    //
    DBfile *dbfile = GetFile(tocIndex);

    //
    // Silo can't accept consts, so cast it away.
    //
    char *m = const_cast< char * >(mesh);
    VisitMutexLock("avtSiloFileFormat");
    int type = DBInqVarType(dbfile, m);
    VisitMutexUnlock("avtSiloFileFormat");

    if (type != DB_MULTIMESH && type != DB_UCDMESH)
    {
        //
        // This is not a ucd mesh or a multi-mesh, so it does not have a
        // facelist.
        //
        return NULL;
    }

    DBmultimesh *mm = NULL;
    avtSiloMultiMeshCacheEntry *mm_ent = NULL;
    string       mb_meshname = "";
    char        *meshname = NULL;

    if (type == DB_MULTIMESH)
    {
        GetMultiMesh("", m, &mm_ent);
        if(mm_ent != NULL)
            mm = mm_ent->DataObject();
        if (mm == NULL)
        {
            EXCEPTION1(InvalidFilesException, m);
        }
        if (dom >= mm->nblocks || dom < 0 )
        {
            EXCEPTION2(BadDomainException, dom, mm->nblocks);
        }
        if (mm_ent->MeshType(dom) != DB_UCDMESH)
        {
            return NULL;
        }
        mb_meshname = mm_ent->GenerateName(dom);
        meshname = CXX_strdup(mb_meshname.c_str());
    }
    else // (type == DB_UCDMESH)
    {
        if (dom != 0)
        {
            EXCEPTION2(BadDomainException, dom, 1);
        }
        meshname = CXX_strdup(mesh);
    }

    return meshname;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetExternalFacelist
//
//  Purpose:
//      Gets the facelist from the Silo file and converts it into an
//      avtFacelist.
//
//  Arguments:
//      dom     The domain of the mesh.
//      mesh    The mesh we want a face list for.
//
//  Returns:    The facelist information.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, made it work with Silo objects
//    distributed across multiple files.
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, August 9, 2004
//    Moved code common to several auxiliary data 'Get' methods to 
//    AllocAndDetermineMeshnameForUcdmesh
//
//    Mark C. Miller, Tue Jun 28 17:28:56 PDT 2005
//    Made it handle the new "EMPTY" domain convention
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
// ****************************************************************************

avtFacelist *
avtSiloFileFormat::GetExternalFacelist(int dom, const char *mesh)
{
    debug5 << "Reading in domain " << dom << ", facelist " << mesh << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    DBfile *dbfile = GetFile(tocIndex);

    char *meshname = AllocAndDetermineMeshnameForUcdmesh(dom, mesh);
    if (meshname == NULL || string(meshname) == "EMPTY")
        return NULL;

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_mesh;
    DetermineFileAndDirectory(meshname, "", domain_file,  directory_mesh);

    avtFacelist *rv = CalcExternalFacelist(domain_file, directory_mesh.c_str());

    if (meshname != NULL)
    {
        delete [] meshname;
        meshname = NULL;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetGlobalNodeIds
//
//  Purpose:
//      Gets the global node ids from the Silo file
//
//  Programmer: Mark C. Miller
//  Creation:   August 4, 2004
//
//  Modifications:
//    Mark C. Miller, Thu Oct 14 15:18:31 PDT 2004
//    Uncommented the data read mask
//
//    Mark C. Miller, Tue Jun 28 17:28:56 PDT 2005
//    Made it handle the new "EMPTY" domain convention
//
//    Hank Childs, Wed Jul 13 10:04:33 PDT 2005
//    Fix memory leak.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Moved code to set data read mask back to its original value to *before*
//    throwing of exeption.
//
//    Mark C. Miller, Tue Jan 12 17:55:14 PST 2010
//    Use CreateDataArray for global node numbers, handling long long case.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Mon Jul 16 22:42:14 PDT 2012
//    Moved debug5 output lines indicating that global node ids are being read
//    down so that it appears in debug logs only when there is actually data
//    being read and returned from the plugin.
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetGlobalNodeIds(int dom, const char *mesh)
{
    DBfile *dbfile = GetFile(tocIndex);

    char *meshname = AllocAndDetermineMeshnameForUcdmesh(dom, mesh);
    if (meshname == NULL || string(meshname) == "EMPTY")
    {
        if (meshname != NULL)
            delete [] meshname;
        return NULL;
    }

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.  
    //
    DBfile *domain_file = dbfile;
    string directory_mesh;
    DetermineFileAndDirectory(meshname, "", domain_file, directory_mesh);

    // We want to get just the global node ids.  So we need to get the ReadMask,
    // set it to read global node ids, then set it back.
    unsigned long long mask = DBGetDataReadMask2();
    DBSetDataReadMask2(DBUMGlobNodeNo);
    DBucdmesh *um = DBGetUcdmesh(domain_file, directory_mesh.c_str());
    DBSetDataReadMask2(mask);
    if (um == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
        return 0;
    }

    vtkDataArray *rv = NULL;
    if (um->gnodeno != NULL)
    {

        debug5 << "Reading in domain " << dom << ", global node ids for " << mesh << endl;
        debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

        rv = CreateDataArray(um->gnznodtype, um->gnodeno, um->nnodes);
        um->gnodeno = 0; // vtkDataArray owns the data now.

    }

    DBFreeUcdmesh(um);

    delete [] meshname;

    return rv;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetLocalDomainBoundaryInfo
//
//  Purpose:
//      Constructs a neighbor list from a scalable format convention.
//
//  Programmer: Cyrus Harrison
//  Creation:  Tue Apr 17 13:00:29 PDT 2012
//
//  Modifications:
//
//
// ****************************************************************************

avtLocalStructuredDomainBoundaryList *
avtSiloFileFormat::GetLocalDomainBoundaryInfo(int domain, const char *var)
{
    debug5 << "Reading in domain " << domain << ", local boundary info for "
           << var << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    string mmesh_name = metadata->MeshForVar(var);

    debug5 << "Reading in domain " << domain
           << ", local boundary info for " << var
           << " mesh name = : " << mmesh_name << endl;

    // now get the multimesh

    int type;
    DBfile *dbfile = GetFile(tocIndex);
    string directory_mesh;
    DBmultimesh *mm;
    DBfile *domain_file = dbfile;

    GetMeshHelper(domain, mmesh_name.c_str(), &mm, &type, &domain_file, directory_mesh);

    int numdecomp_pack = 0;

    string mesh_dir_parent(FileFunctions::Dirname(directory_mesh.c_str()));
    ostringstream oss;
    oss << mesh_dir_parent << "/" << "NumDecomp_pack";
    if(!DBInqVarExists(domain_file,oss.str().c_str()))
    {
        debug5 << "Skipping read of local domain boundaries: "
               << oss.str().c_str() << " does not exist."  <<endl;
        return NULL;
    }
    DBReadVar(domain_file,oss.str().c_str(),&numdecomp_pack);
    int *decomp = new int[numdecomp_pack];
    oss.str("");
    oss << mesh_dir_parent << "/" << "Decomp_pack";
    if(!DBInqVarExists(domain_file,oss.str().c_str()))
    {
        debug5 << "Skipping read of local domain boundaries: "
               << oss.str().c_str() << " does not exist." <<endl;
        return NULL;
    }
    DBReadVar(domain_file,oss.str().c_str(),decomp);

    //
    // extract the packed info into a avtLocalStructuredDomainBoundaryList
    //

    int *dc_ptr = decomp;
    int lid     = *dc_ptr++;
    int nbnd    = *dc_ptr++;
    avtLocalStructuredDomainBoundaryList *res =
                    new avtLocalStructuredDomainBoundaryList(domain,dc_ptr);
    dc_ptr+=6;
    for (int i=0; i < nbnd; i++)
    {
        res->AddNeighbor(dc_ptr[0],   // nei id
                         dc_ptr[1],   // match
                         &dc_ptr[2],  // orient
                         &dc_ptr[5]); // extents
        dc_ptr += 11;
    }

    //res->Print(cout);
    return res;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetGlobalZoneIds
//
//  Purpose:
//      Gets the global zone ids from the Silo file
//
//  Programmer: Mark C. Miller
//  Creation:   August 9, 2004
//
//  Modifications:
//    Mark C. Miller, Thu Oct 14 15:18:31 PDT 2004
//    Uncommented the data read mask
//
//    Mark C. Miller, Tue Jun 28 17:28:56 PDT 2005
//    Made it handle the new "EMPTY" domain convention
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Moved code to set data read mask back to its original value to *before*
//    throwing of exeption.
//
//    Mark C. Miller, Thu Oct 15 21:31:07 PDT 2009
//    Add DBZonelistInfo to data read mask to work around a bug in Silo
//    library where attempt to DBGetUcdmesh causes call to DBGetZonelist
//    and a subsequent segv down in the bowels of Silo due to invalid
//    assumptions regarding the existence of certain zonelist strutures.
//
//    Mark C. Miller, Tue Jan 12 17:55:54 PST 2010
//    Use CreateDataArray for global zone numbers, handling long long too.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

vtkDataArray *
avtSiloFileFormat::GetGlobalZoneIds(int dom, const char *mesh)
{
    debug5 << "Reading in domain " << dom << ", global zone ids for " << mesh << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    DBfile *dbfile = GetFile(tocIndex);

    char *meshname = AllocAndDetermineMeshnameForUcdmesh(dom, mesh);
    if (meshname == NULL || string(meshname) == "EMPTY")
        return NULL;

    //
    // Some Silo objects are distributed across several files,
    // so handle that here.
    //
    DBfile *domain_file = dbfile;
    string directory_mesh;
    DetermineFileAndDirectory(meshname, "", domain_file, directory_mesh);

    // We want to get just the global node ids.  So we need to get the ReadMask,
    // set it to read global node ids, then set it back.
    unsigned long long mask = DBGetDataReadMask2();
    DBSetDataReadMask2(DBUMZonelist|DBZonelistGlobZoneNo|DBZonelistInfo);
    DBucdmesh *um = DBGetUcdmesh(domain_file, directory_mesh.c_str());
    DBSetDataReadMask2(mask);
    if (um == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
        return 0;
    }

    vtkDataArray *rv = NULL;
    if (um->zones->gzoneno != NULL)
    {
        rv = CreateDataArray(um->zones->gnznodtype, um->zones->gzoneno, um->zones->nzones); 
        um->zones->gzoneno = 0; // vtkDataArray owns the data now.
    }

    DBFreeUcdmesh(um);

    delete [] meshname;

    return rv;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetSpatialExtents
//
//  Purpose:
//      Gets the spatial extents from a multimesh, if they exist in the file
//
//  Programmer: Mark C. Miller
//  Creation:   October 14, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

avtIntervalTree *
avtSiloFileFormat::GetSpatialExtents(const char *meshName)
{
    debug5 << "Getting spatial extents for \"" << meshName << "\"" << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *mesh = const_cast<char *>(meshName);

    //
    // Start off by finding out what kind of mesh it is.  Note: we have
    // already cached the multimeshes.  See if we have a multimesh in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //
    DBmultimesh *mm = NULL;

    // Note: Just call GetMultiMesh, since it checks the cache
    // just like QueryMultiMesh
    // old code:
    // mm = QueryMultimesh("", mesh);
    // if (mm == NULL)
    //      mm = GetMultimesh("", mesh);

    avtSiloMultiMeshCacheEntry *mm_ent = 0;
    GetMultiMesh("", mesh, &mm_ent);
    if(mm_ent != NULL)
        mm = mm_ent->DataObject();

    // if this mesh doesn't exist or doesn't have extents, return nothing
    if (mm == NULL || mm->extents == NULL)
        return NULL;

    int ndims = mm->extentssize / 2; 
    avtIntervalTree *itree = new avtIntervalTree(mm->nblocks, ndims);

    for (int i = 0; i < mm->nblocks; i++)
    {
        double range[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
        int esize = mm->extentssize;

        // silo stores all the mins before all the maxs
        for (int j = 0; j < ndims; j++)
        {
            range[j*2  ] = mm->extents[i*esize + j];
            range[j*2+1] = mm->extents[i*esize + j + ndims];
        }

        itree->AddElement(i, range);
    }

    itree->Calculate(true);

    return itree;

}

// ****************************************************************************
//  Method: avtSiloFileFormat::GetDataExtents
//
//  Purpose:
//      Gets the data extents from a multivar, if they exist in the file 
//
//  Programmer: Mark C. Miller
//  Creation:   October 14, 2004 
//
//  Modifications:
//    Kathleen Bonnell, Mon Aug 14 16:40:30 PDT 2006
//    API change for avtIntervalTree.
//
// ****************************************************************************

avtIntervalTree *
avtSiloFileFormat::GetDataExtents(const char *varName)
{
    debug5 << "Getting data extents for \"" << varName << "\"" << endl;
    debug5 << "Reading in from toc " << filenames[tocIndex] << endl;

    //
    // It's ridiculous, but Silo does not have all of the `const's in their
    // library, so let's cast it away.
    //
    char *var = const_cast<char *>(varName);

    //
    // Start off by finding out what kind of mesh it is.  Note: we have
    // already cached the multimeshes.  See if we have a multimesh in the
    // cache already -- this could potentially save us a DBInqVarType call.
    //

    // Note: Just call GetMultiVar, since it checks the cache
    // just like QueryMultiVar
    // old code:
    //DBmultivar *mv = QueryMultivar("", var);
    //if (mv == NULL)
    //    mv = GetMultivar("", var);

    DBmultivar *mv = NULL;
    avtSiloMultiVarCacheEntry *mv_ent = 0;
    GetMultiVar("", var, &mv_ent);
    if (mv_ent != NULL)
        mv = mv_ent->DataObject();

    // if this mesh doesn't exist or doesn't have extents, return nothing
    if (mv == NULL || mv->extents == NULL)
        return NULL;

    int ncomps = mv->extentssize / 2; 
    int esize = mv->extentssize;
    avtIntervalTree *itree = new avtIntervalTree(mv->nvars, ncomps);
    double *range = new double[esize];

    for (int i = 0; i < mv->nvars; i++)
    {
        // silo stores all the mins before all the maxs
        for (int j = 0; j < ncomps; j++)
        {
            range[j*2  ] = mv->extents[i*esize + j];
            range[j*2+1] = mv->extents[i*esize + j + ncomps];
        }

        itree->AddElement(i, range);
    }

    delete [] range;

    itree->Calculate(true);

    return itree;

}

// ****************************************************************************
//  Method: avtSiloFileFormat::CalcMaterial
//
//  Purpose:
//      Gets the Silo material and creates an avtMaterial.
//
//  Arguments:
//      dbfile    The Silo file handle to use.
//      matname   The name of the material.
//
//  Returns:      The material.
//
//  Programmer:   Hank Childs
//  Creation:     November 7, 2000
//
//  Modifications:
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Sean Ahern, Fri Feb  8 11:12:38 PST 2002
//    Added support for material names.
//
//    Eric Brugger, Thu May 23 14:23:23 PDT 2002
//    I modified the routine to take into account the major order of
//    the variable being read.
//
//    Hank Childs, Fri Feb 13 17:18:04 PST 2004
//    Add the domain to the material constructor.
//
//    Hank Childs, Wed Apr 14 07:52:48 PDT 2004
//    Attach the material number to the material name.
//
//    Mark C. Miller, Thu Oct 21 22:11:28 PDT 2004
//    Added code to remove entries from matlist array for arb. zones that
//    were removed from the mesh
//
//    Cyrus Harrison, Thu Oct  4 11:11:26 PDT 200
//    Removed limit of 128 characters for material names. I increased the 
//    limit to 256 + room for the material number - to safely handle valid 
//    silo material names. This resolves '8257.
//
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Mark C. Miller, Fri Mar 20 04:36:49 PDT 2009
//    Added logic to obtain global material context (# materials, names and
//    numbers) from parent multimat, if it exists and has that info. This has
//    the effect of freeing the data-producer of having to re-enumerate that
//    information on each and every individual material object in a large
//    multi-mesh with many materials.
//
//    Hank Childs, Mon May 25 11:07:17 PDT 2009
//    Add support for Silo releases before 4.6.3.
//
//    Tom Fogal, Mon May 25 18:53:30 MDT 2009
//    Fixed some compilation errors && simplified some ifdef magic.
//
//    Mark C. Miller, Thu Jun  4 20:43:29 PDT 2009
//    Fixed a slew of syntax errors preventing compilation.
//
//    Brad Whitlock, Fri Aug  7 11:40:37 PDT 2009
//    Convert double mix_vf to float for now since avtMaterial can't store it.
//
//    Mark C. Miller, Thu Oct 29 14:34:55 PDT 2009
//    Replaced zone skipping logic (old way of handling meshes with arb. poly
//    zones) to use remapping logic. 
//
//    Mark C. Miller, Fri Oct 30 14:03:13 PDT 2009
//    Handle Silo's DB_DTPTR configuration option.
//
//    Mark C. Miller, Wed Jan 27 13:14:03 PST 2010
//    Added extra level of indirection to arbMeshXXXRemap objects to handle
//    multi-block case.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Eric Brugger, Fri Feb 15 09:39:52 PST 2013
//    I corrected a memory error where a buffer used to hold material names
//    was underallocated. This resulted in crashes in some instances.
//
//    Mark C. Miller, Fri Nov  6 09:13:22 PST 2015
//    Avert segv on deref of material_names in block level mat object.
// ****************************************************************************

avtMaterial *
avtSiloFileFormat::CalcMaterial(DBfile *dbfile, const char *matname, const char *tmn,
    int dom)
{
    VisitMutexLock("avtSiloFileFormat");
    DBmaterial *silomat = DBGetMaterial(dbfile, matname);
    VisitMutexUnlock("avtSiloFileFormat");
    if (silomat == NULL)
    {
        EXCEPTION1(InvalidVariableException, matname);
    }

    //
    // Get the parent multi-mat object, if there is any, because it could
    // have information about global material context that the individual
    // material block object read here doesn't have.
    //
    DBmultimat *mm = NULL;
    avtSiloMultiMatCacheEntry *mm_ent = QueryMultiMat("", const_cast< char * >(tmn));
    if(mm_ent != NULL)
        mm = mm_ent->DataObject();

    char dom_string[128];
    sprintf(dom_string, "Domain %d", dom);

    //
    // If we have material names, create a new set of material names that have
    // the material number encoded.
    //
    char **matnames = NULL;
    char *buffer = NULL;
    bool haveMatnames = silomat->matnames;
    if (mm&&mm->material_names)
        haveMatnames = true;
    if (haveMatnames)
    {
        int nmat = silomat->nmat;
        if (mm&&mm->nmatnos>0) 
            nmat = mm->nmatnos;
        int max_dlen = 0;
        for (int i = 0 ; i < nmat ; i++)
        {
            int matno = silomat->matnos[i];
            if (mm&&mm->matnos)
                matno = mm->matnos[i];
            int dlen = int(log10(float(matno+1))) + 1;
            if(dlen>max_dlen)
                max_dlen = dlen;
        }
        
        matnames = new char*[nmat];
        buffer = new char[nmat*(256+max_dlen)];
        
        for (int i = 0 ; i < nmat ; i++)
        {
            matnames[i] = buffer + (256+max_dlen)*i;
            int matno = silomat->matnos[i];
            const char *matname = silomat->matnames ? silomat->matnames[i] : 0;
            if (mm&&mm->matnos)
                matno = mm->matnos[i];
            if (mm&&mm->material_names)
                matname = mm->material_names[i];
            sprintf(matnames[i], "%d %s", matno, matname);
        }
    }

    //
    // Handle cases for arb. poly meshes where arb. zones have been
    // decomposed into zoo-type zones. We need only do this for matlist
    // array, even in the presence of mixing.
    //
    vtkDataArray *matListArr = 0;
    string meshName = metadata->MeshForVar(tmn);
    map<string, map<int, vector<int>* > >::iterator domit;
    if ((domit = arbMeshCellReMap.find(meshName)) != arbMeshCellReMap.end() &&
         domit->second.find(dom) != domit->second.end())
    {
        vector<int> *remap = domit->second[dom];

        int nzones = 1;
        for (int i = 0; i < silomat->ndims; i++)
            nzones *= silomat->dims[i];

        //
        // Create a temp. ucdvar so we can use CopyAndPadUcdVar to remap matlist
        //
        DBucdvar tmp;
        tmp.centering = DB_ZONECENT;
        tmp.datatype = DB_INT;
        tmp.nels = nzones;
        tmp.nvals = 1;
        tmp.vals = (void**) malloc(sizeof(void*)); /* use malloc because this is a Silo object */
        tmp.vals[0] = (void*) silomat->matlist;
        matListArr = CopyAndPadUcdVar<int,vtkIntArray>(&tmp, *remap);

        //
        // Adjust the Silo DBmaterial object a bit for call to newMaterial
        //
        silomat->ndims = 1;
        silomat->dims[0] = (int) remap->size();
        silomat->dims[1] = 1;
        silomat->dims[2] = 1;
        free(tmp.vals);
    }

    int nummats = silomat->nmat;
    int *matnos = silomat->matnos;
    if (mm&&mm->nmatnos>0)
        nummats = mm->nmatnos;
    if (mm&&mm->matnos)
        matnos = mm->matnos;
    if(silomat->datatype != DB_FLOAT)
    {
        debug5 << "IMPORTANT: The Silo reader is converting mix_vf data for "
               << matname << " to single precision." << endl;
    }
    float *mix_vf = ConvertToFloat(silomat->datatype, silomat->mix_vf, silomat->mixlen);

    avtMaterial *mat = 0;
    if (nummats && silomat->matlist && silomat->ndims)
    {
       mat = new avtMaterial(nummats, matnos, matnames, silomat->ndims,
            silomat->dims, silomat->major_order,
            matListArr?(int*)(matListArr->GetVoidPointer(0)):silomat->matlist,
            silomat->mixlen, silomat->mix_mat, silomat->mix_next, silomat->mix_zone,
            mix_vf, dom_string ,silomat->allowmat0);
    }

    if(mix_vf && mix_vf != (float*)silomat->mix_vf)
        delete [] mix_vf;
    if (matListArr)
        matListArr->Delete();
    DBFreeMaterial(silomat);
    if (matnames != NULL)
        delete [] matnames;
    if (buffer != NULL)
        delete [] buffer;

    return mat;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::CalcSpecies
//
//  Purpose:
//      Gets the Silo species and creates an avtSpecies.
//
//  Arguments:
//      dbfile    The Silo file handle to use.
//      specname  The name of the species.
//
//  Returns:      The species.
//
//  Programmer:   Jeremy Meredith
//  Creation:     December 13, 2001
//
//  Modifications:
//    Mark C. Miller, Tue Dec 16 09:36:56 PST 2008
//    Added casts to deal with new Silo API where datatype'd pointers
//    have been changed from float* to void*.
//
//    Brad Whitlock, Fri Aug  7 11:48:39 PDT 2009
//    Convert other data types to float for now since avtSpecies can't 
//    store them.
//
//    Cyrus Harrison, Wed Aug 25 12:21:54 PDT 2010
//    Use force single for species reads.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller Sat Jan 19 22:24:36 PST 2013
//    Swap silospec->mixlen for silospec->nspecis_mf in call to ConverToFloat.
// ****************************************************************************

avtSpecies *
avtSiloFileFormat::CalcSpecies(DBfile *dbfile, const char *specname)
{
    // TODO: Fix when Silo issue is resolved.
    // Currently if force single is *off* we get garbage mass fractions
    // from Silo. This doesn't appear to be related to the prev
    // datatype mismatch oissue. To work around this we turn force single on
    // just for this read.

    DBForceSingle(1);
    DBmatspecies *silospec = DBGetMatspecies(dbfile, specname);
    DBForceSingle(forceSingle);

    if (silospec == NULL)
    {
        EXCEPTION1(InvalidVariableException, specname);
    }

    if(silospec->datatype != DB_FLOAT)
    {
        debug5 << "IMPORTANT: The Silo reader is converting species_mf data for "
               << specname << " to single precision." << endl;
    }
    float *species_mf = ConvertToFloat(silospec->datatype, silospec->species_mf,
                                       silospec->nspecies_mf);

    avtSpecies *spec = 0;
    if (silospec->speclist && silospec->nmat && silospec->ndims)
    {
        spec = new avtSpecies(silospec->nmat,
                              silospec->nmatspec,
                              silospec->ndims,
                              silospec->dims,
                              silospec->speclist,
                              silospec->mixlen,
                              silospec->mix_speclist,
                              silospec->nspecies_mf,
                              species_mf);
    }

    if(species_mf && species_mf != (float*)silospec->species_mf)
        delete [] species_mf;

    DBFreeMatspecies(silospec);

    return spec;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::CalcExternalFacelist
//
//  Purpose:
//      Reads the facelist in from the Silo file and creates an avtFacelist
//      out of it.
//
//  Arguments:
//      dbfile    The Silo file handle to use.
//      mesh      The name of the mesh to get the facelist for.
//
//  Returns:    The facelist, NULL if one doesn't exist.
//
//  Programmer: Hank Childs
//  Creation:   October 30, 2000
//
//  Modifications:
//
//    Hank Childs, Mon Dec 18 13:05:44 PST 2000
//    Turned off error handling when getting facelists in case they weren't
//    there.
//
//    Hank Childs, Wed Feb 28 10:49:17 PST 2001
//    Moved code from avtSiloTimeStep, added file argument to account for
//    Silo objects distributed across multiple files.
//
//    Sean Ahern, Fri Feb  8 14:10:59 PST 2002
//    Retrofitted to not use GetComponent.
//
//    Hank Childs, Mon Jun 17 19:38:10 PDT 2002
//    Respect non 0-origin facelists.
//
//    Hank Childs, Mon Dec  9 13:21:13 PST 2002
//    Use the facelist's origin rather than the unstructured meshes.
//
//    Mark C. Miller, Thu Apr 21 09:37:41 PDT 2005
//    Freed ucdmesh prior to early return for null facelist
//
//    Hank Childs, Tue Jun 13 14:23:48 PDT 2006
//    Add flag to read mask.  The current flag works with PDB, but not HDF5.
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Moved code to set data read mask back to its original value to *before*
//    throwing of exeption.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

avtFacelist *
avtSiloFileFormat::CalcExternalFacelist(DBfile *dbfile, const char *mesh)
{
    string realvar;
    DBfile *correctFile = dbfile;
    DetermineFileAndDirectory(mesh, "", correctFile, realvar);

    // We want to get just the facelist.  So we need to get the ReadMask,
    // set it to read facelists, then set it back.
    VisitMutexLock("avtSiloFileFormat::CalcExternalFacelist");
    unsigned long long mask = DBGetDataReadMask2();
    DBSetDataReadMask2(DBUMFacelist | DBFacelistInfo);
    DBucdmesh *um = DBGetUcdmesh(correctFile, realvar.c_str());
    DBSetDataReadMask2(mask);
    VisitMutexUnlock("avtSiloFileFormat::CalcExternalFacelist");
    if (um == NULL)
    {
        if (!ignoreMissingBlocks)
        {
            EXCEPTION1(InvalidVariableException, mesh);
        }
        return 0;
    }

    DBfacelist *fl = um->faces;

    if (fl == NULL)
    {
        //
        // There is no facelist object, this may be an interior block.
        //
        DBFreeUcdmesh(um);
        return NULL;
    }

    avtFacelist *rv = new avtFacelist(fl->nodelist, fl->lnodelist, fl->nshapes,
                                      fl->shapecnt, fl->shapesize, fl->zoneno,
                                      fl->origin);

    DBFreeUcdmesh(um);

    return rv;
}

// ****************************************************************************
// Method: avtSiloFileFormat::PopulateIOInformation(
//
// Purpose:
//   Makes I/O groupings based on the layouts of the files.
//
// Arguments:
//   meshname : The name of the mesh whose io information we want.
//   io       : The IO information object we're populating.
//
// Returns:    True on success; false on failure
//
// Note:       Calls the original version of the method which was renamed to
//             PopulateIOInformationEx. This method tries to get the IO
//             information once and reuses any cached results. This behavior
//             was moved down from the avtDatabase level so file formats could
//             decide how often they really get the IO information.
//
// Programmer: Brad Whitlock
// Creation:   Thu Jun 19 11:40:24 PDT 2014
//
// Modifications:
//
// ****************************************************************************

bool
avtSiloFileFormat::PopulateIOInformation(const std::string &meshname, avtIOInformation &io)
{
    if(!ioInfoValid)
    {
        ioInfoValid = PopulateIOInformationEx(meshname, ioInfo);
    }

    io = ioInfo;

    return ioInfoValid;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::PopulateIOInformationEx
//
//  Purpose:
//      Makes I/O groupings based on the layouts of the files.
//
//  Arguments:
//      ioInfo    The information object to populate.
//
//  Notes:        Other routines could fail if there are multiple top level
//                multi-meshes within the same root directory, so that case is
//                not handled.
//
//  Programmer:   Hank Childs
//  Creation:     May 11, 2001
//
//  Modifications:
//
//    Hank Childs, Mon May 21 10:46:25 PDT 2001
//    Added missing return statement.
//
//    Jeremy Meredith, Thu Sep 20 01:05:19 PDT 2001
//    Added setting of NDomains.
//
//    Hank Childs, Thu Sep 20 17:43:31 PDT 2001
//    Set nDomains with early returns.
//
//    Mark C. Miller, Thu Feb  1 19:44:03 PST 2007
//    Exclude CSG meshes from consideration
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Brad Whitlock, Thu Jun 19 11:43:13 PDT 2014
//    I renamed the method to PopulateIOInformationEx and pass in the the
//    mesh name. Note that the mesh name is not currently used but the intent
//    is to allow different ioInfo for different multi-meshes.
//
//    Cyrus Harrison, Mon Dec 21 11:35:05 PST 2015
//    Remove n^2 algorithm when constructing map from filenames to domain id 
//    lists. 
//
// ****************************************************************************

bool
avtSiloFileFormat::PopulateIOInformationEx(const std::string &meshname, avtIOInformation &ioInfo)
{
    bool retval = false;
    TRY
    {
        int   i, j;
        int nMeshes = metadata->GetNumMeshes();

        if (nMeshes < 1)
        {
            debug1 << "Cannot populate I/O info since there are no meshes" << endl;
            ioInfo.SetNDomains(0);
            return false;
        }
        
        //
        // If there are different sized meshes, then we need to give up.  
        // Check to see.
        //
        int blocks = -1;
        int firstNonCSGMesh = -1;
        for (i = 0 ; i < nMeshes ; i++)
        {
            if (metadata->GetMesh(i)->meshType == AVT_CSG_MESH)
                continue;

            if (blocks == -1)
                blocks = metadata->GetMesh(i)->numBlocks;

            if (firstNonCSGMesh == -1)
                firstNonCSGMesh = i;

            if (metadata->GetMesh(i)->numBlocks != blocks)
            {
                debug1 << "Cannot populate I/O Information since the meshes have "
                       << "a different number of blocks." << endl;
                ioInfo.SetNDomains(0);
                return false;
            }
        }

        if (blocks <= 1)
        {
            debug5 << "No need to do I/O optimization because there is only "
                   << "one block" << endl;
            ioInfo.SetNDomains(1);
            return true;
        }

        //
        // Looks like all of the meshes are the same size.  ASSUME they are all
        // the same and pick the first one.  This is done for I/O optimizations, so
        // it is okay to be wrong if our assumption is not true.
        //
        string meshname = metadata->GetMesh(firstNonCSGMesh)->name;

        DBmultimesh *mm = NULL;
        avtSiloMultiMeshCacheEntry *mm_ent = 0;
        GetMultiMesh("", meshname.c_str(), &mm_ent);
        if(mm_ent != NULL)
            mm = mm_ent->DataObject();

        if (mm == NULL)
        {
            debug1 << "Cannot populate I/O Information because unable "
                   << "to get multimesh object \"" << meshname.c_str() << "\"." << endl;
            ioInfo.SetNDomains(0);
            return false;
        }

        //
        // ioInfo needs a vector from each file's index, to a vector with a 
        // list of domain ids it contains. 
        // the groups vector holds this info.
        vector<vector<int> > groups;
        
        // use a map to avoid previous n^2 lookup implementation 
        map<string,int> filename_index_map;

        /// we know we need an entry for each domain, so init groups to the proper size
        groups.resize(mm->nblocks);
        
        // loop over all domains and find which file (and file index) the domain is 
        // associated with 
        for (i = 0 ; i < mm->nblocks ; i++)
        {
            string filename;
            string location;
            DetermineFilenameAndDirectory(mm_ent->GenerateName(i).c_str(),
                                          "", filename, location);

            // check if we have already seen this filename
            int fname_index = -1;
            map<string,int>::const_iterator itr = filename_index_map.find(filename);
            if (itr != filename_index_map.end())
            {
                // if found, use the index
                fname_index = itr->second;
            }
            else
            {
                // if not, assign it a new index
                fname_index = filename_index_map.size();
                filename_index_map[filename] = fname_index;
            }

            /// add this domain id to the list of ids for the found filename
            groups[fname_index].push_back(i);
        }

        ioInfo.SetNDomains(mm->nblocks);
        ioInfo.AddHints(groups);
        retval = true;
    }
    CATCHALL
    {
        debug1 << "Unable to populate I/O information" << endl;
    }
    ENDTRY

    return retval;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::ShouldGoToDir
//
//  Purpose:
//      Determines if the directory is one we should go to.  If we have already
//      seen a multi-var that has a block in that directory, don't bother going
//      in there.
//
//  Arguments:
//      dirname   The directory of interest.
//
//  Returns:      true if we should go in the directory, false otherwise.
//
//  Programmer:   Hank Childs
//  Creation:     October 31, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 13 16:00:16 PDT 2005
//    Changed domainDirs to a set to ensure log(n) access times.
//
//    Jeremy Meredith, Thu Dec 18 12:17:40 EST 2008
//    Avoid descending through the top-level decomposition directories.
//    In that convention, there will be no mesh variables in those trees,
//    but in files with many domains, walking that subtree can be very slow.
//    Also, re-use the count of dirname in domainDirs; don't check twice.
//
// ****************************************************************************

bool
avtSiloFileFormat::ShouldGoToDir(const char *dirname)
{
    bool shouldGo = (domainDirs.count(dirname) == 0);
    if (shouldGo)
        debug5 << "Deciding to go into dir \"" << dirname << "\"" << endl;
    else
        debug5 << "Skipping dir \"" << dirname << "\"" << endl;
    if (strcmp(dirname, "/Decomposition") == 0)
        shouldGo = false;
    if (strcmp(dirname, "/Global") == 0)
        shouldGo = false;
    return (shouldGo);
}


// ****************************************************************************
//  Method: avtSiloFileFormat::RegisterDomainDirs
//
//  Purpose:
//      Registers directories that we know have blocks from a domain in them.
//      This way we won't traverse that directory later.
//
//  Arguments:
//      dirlist   A list of directories (with appended variable names).
//      nList     The number of elements in dirlist.
//      curDir    The name of the current directory.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jul 12 16:22:02 PDT 2002
//    Add support for variables that are specified absolutely instead of
//    relatively.
//
//    Jeremy Meredith, Tue Sep 13 16:00:16 PDT 2005
//    Changed domainDirs to a set to ensure log(n) access times.
//    Added check to make sure we don't even bother for an EMPTY domain.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

void
avtSiloFileFormat::RegisterDomainDirs(avtSiloMBObjectCacheEntry *obj,
                                      const char *cur_dir)
{

    int nblocks = obj->NumberOfBlocks();
    string mb_name = "";

    for (int i = 0 ; i < nblocks ; i++)
    {
        mb_name = obj->GenerateName(i);
        if ( mb_name == "EMPTY")
            continue;

        string res = PrepareDirName(mb_name.c_str(), cur_dir);
        domainDirs.insert(res );
    }
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultiMesh
//
//  Purpose:
//      Returns a multimesh from the cache.  Only returns a multimesh if
//      we have called "GetMultimesh" on it before.  It is entirely possible
//      that "QueryMultimesh" returns NULL and there *is* a multi-mesh.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

avtSiloMultiMeshCacheEntry *
avtSiloFileFormat::QueryMultiMesh(const char *path, const char *name)
{
    avtSiloMultiMeshCacheEntry *res = NULL;
    avtSiloMBObjectCacheEntry  *cache_ent = multimeshCache.FetchEntry(path,name);

    if(cache_ent != NULL)
        res = (avtSiloMultiMeshCacheEntry*) cache_ent; // dynamic cast?
    return res;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultiMesh
//
//  Purpose:
//      Gets a multimesh and caches it for later use.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Wed Mar 13 23:02:58 PDT 2013
//    Pass object's directory when creating a new CacheEntry.
//
//    Mark C. Miller, Tue Feb  2 15:14:08 PST 2016
//    Adjust to track multi-meshes that have zero blocks (e.g. all empty)
// ****************************************************************************

void
avtSiloFileFormat::GetMultiMesh(const char *path, const char *name,
    avtSiloMultiMeshCacheEntry **cache_ent, bool *valid_var)
{
    //
    // First, check to see if we have already gotten the multi-block obj.
    //
    *cache_ent = QueryMultiMesh(path, name);
    if (*cache_ent) return;

    //
    // We haven't seen this object before -- read it in.
    //

    DBfile *dbfile = GetFile(tocIndex);
    string full_path = avtSiloMBObjectCache::CombinePath(path,name);
    DBmultimesh *mm = DBGetMultimesh(dbfile, full_path.c_str());
    if(mm != NULL)
    {
        *cache_ent = new avtSiloMultiMeshCacheEntry(dbfile,
            FileFunctions::Dirname(full_path.c_str()),mm);
        if (mm->nblocks > 0 && (*cache_ent)->GenerateName(0) == "")
        {
            if (valid_var) *valid_var = false;
        }
        multimeshCache.AddEntry(full_path,*cache_ent);
    }
    return;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultiVar
//
//  Purpose:
//      Returns a multivar from the cache.  Only returns a multivar if
//      we have called "GetMultivar" on it before.  It is entirely possible
//      that "QueryMultivar" returns NULL and there *is* a multi-var.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

avtSiloMultiVarCacheEntry *
avtSiloFileFormat::QueryMultiVar(const char *path, const char *name)
{
    avtSiloMultiVarCacheEntry *res = NULL;
    avtSiloMBObjectCacheEntry  *cache_ent = multivarCache.FetchEntry(path,name);

    if(cache_ent != NULL)
        res = (avtSiloMultiVarCacheEntry*) cache_ent; // dynamic cast?
    return res;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultiVar
//
//  Purpose:
//      Gets a multivar and caches it for later use.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Wed Mar 13 23:02:58 PDT 2013
//    Pass object's directory when creating a new CacheEntry.
//
//    Mark C. Miller, Tue Feb  2 15:15:49 PST 2016
//    Adjust to track even multivars that have no blocks (e.g. all empty)
// ****************************************************************************

void
avtSiloFileFormat::GetMultiVar(const char *path, const char *name,
    avtSiloMultiVarCacheEntry **cache_ent, bool *valid_var)
{
    //
    // First, check to see if we have already gotten the multi-block obj.
    //
    *cache_ent = QueryMultiVar(path, name);

    if (*cache_ent != NULL) return;

    //
    // We haven't seen this object before -- read it in.
    //

    DBfile *dbfile = GetFile(tocIndex);
    string full_path = avtSiloMBObjectCache::CombinePath(path,name);
    DBmultivar *mv = DBGetMultivar(dbfile, full_path.c_str());
    if(mv != NULL)
    {
        *cache_ent = new avtSiloMultiVarCacheEntry(dbfile,
            FileFunctions::Dirname(full_path.c_str()),mv);
        if (mv->nvars > 0 && (*cache_ent)->GenerateName(0) == "")
        {
            if (valid_var) *valid_var = false;
        }
        multivarCache.AddEntry(full_path,*cache_ent);
    }
    return;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultiMat
//
//  Purpose:
//      Returns a multimat from the cache.  Only returns a multimat if
//      we have called "GetMultimat" on it before.  It is entirely possible
//      that "QueryMultimat" returns NULL and there *is* a multi-mat.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.

// ****************************************************************************

avtSiloMultiMatCacheEntry *
avtSiloFileFormat::QueryMultiMat(const char *path, const char *name)
{
    avtSiloMultiMatCacheEntry *res = NULL;
    avtSiloMBObjectCacheEntry  *cache_ent = multimatCache.FetchEntry(path,name);

    if(cache_ent != NULL)
        res = (avtSiloMultiMatCacheEntry*) cache_ent; // dynamic cast?
    return res;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultiMat
//
//  Purpose:
//      Gets a multimat and caches it for later use.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Wed Mar 13 23:02:58 PDT 2013
//    Pass object's directory when creating a new CacheEntry.
//
//    Mark C. Miller, Tue Feb  2 15:15:49 PST 2016
//    Adjust to track even multimats that have no blocks (e.g. all empty)
// ****************************************************************************

void
avtSiloFileFormat::GetMultiMat(const char *path, const char *name,
    avtSiloMultiMatCacheEntry **cache_ent, bool *valid_var)
{
    //
    // First, check to see if we have already gotten the multi-block obj.
    //
    *cache_ent = QueryMultiMat(path, name);

    if (*cache_ent != NULL) return;

    //
    // We haven't seen this object before -- read it in.
    //

    DBfile *dbfile = GetFile(tocIndex);
    string full_path = avtSiloMBObjectCache::CombinePath(path,name);
    DBmultimat *mm = DBGetMultimat(dbfile, full_path.c_str());
    if(mm != NULL)
    {
        *cache_ent = new avtSiloMultiMatCacheEntry(dbfile,
            FileFunctions::Dirname(full_path.c_str()),mm);
        if (mm->nmats > 0 && (*cache_ent)->GenerateName(0) == "")
        {
            if (valid_var) *valid_var = false;
        }
        multimatCache.AddEntry(full_path,*cache_ent);
    }
    return;
}

// ****************************************************************************
//  Method: avtSiloFileFormat::QueryMultiSpec
//
//  Purpose:
//      Returns a multimatspec from the cache.  Only returns a multimatspec if
//      we have called "GetMultimatspec" on it before.  It is entirely possible
//      that "QueryMultimatspec" returns NULL and there *is* a multi-matspec.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
// ****************************************************************************

avtSiloMultiSpecCacheEntry *
avtSiloFileFormat::QueryMultiSpec(const char *path, const char *name)
{
    avtSiloMultiSpecCacheEntry *res = NULL;
    avtSiloMBObjectCacheEntry  *cache_ent = multispecCache.FetchEntry(path,name);

    if(cache_ent != NULL)
        res = (avtSiloMultiSpecCacheEntry*) cache_ent; // dynamic cast?
    return res;
}


// ****************************************************************************
//  Method: avtSiloFileFormat::GetMultiSpec
//
//  Purpose:
//      Gets a multimatspec and caches it for later use.
//
//  Programmer: Hank Childs
//  Creation:   January 14, 2004
//
//  Modifications:
//
//    Mark C. Miller, Mon Feb 23 12:02:24 PST 2004
//    Changed call to OpenFile() to GetFile()
//
//    Mark C. Miller, Wed Nov  9 21:30:45 PST 2011
//    Add protections for multi-block objects with zero blocks
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Wed Mar 13 23:02:58 PDT 2013
//    Pass object's directory when creating a new CacheEntry.
//
//    Mark C. Miller, Tue Feb  2 15:15:49 PST 2016
//    Adjust to track even multi-specs that have no blocks (e.g. all empty)
// ****************************************************************************

void
avtSiloFileFormat::GetMultiSpec(const char *path, const char *name,
    avtSiloMultiSpecCacheEntry **cache_ent, bool *valid_var)
{
    //
    // First, check to see if we have already gotten the multi-block obj.
    //
    *cache_ent = QueryMultiSpec(path, name);

    if (*cache_ent != NULL) return;

    //
    // We haven't seen this object before -- read it in.
    //

    DBfile *dbfile = GetFile(tocIndex);
    string full_path = avtSiloMBObjectCache::CombinePath(path,name);
    DBmultimatspecies *ms = DBGetMultimatspecies(dbfile, full_path.c_str());
    if(ms != NULL)
    {
        *cache_ent = new avtSiloMultiSpecCacheEntry(dbfile,
            FileFunctions::Dirname(full_path.c_str()),ms);
        if (ms->nspec > 0 && (*cache_ent)->GenerateName(0) == "")
        {
            if (valid_var) *valid_var = false;
        }
        multispecCache.AddEntry(full_path,*cache_ent);
    }
    return;
}


// ****************************************************************************
//  Function: CheckForTimeVaryingMetadata
//
//  Purpose:
//      Checks the existance & value of the flag 'MetadataIsTimeVarying'
//      to determine if the metadata & sil potentially change between
//      time steps.
//
//  Programmer: Cyrus Harrison
//  Creation:   Mon Jun 14 15:30:11 PDT 2010
//
//  Modifications:
//
// ****************************************************************************
void
avtSiloFileFormat::CheckForTimeVaryingMetadata(DBfile *toc)
{
    if (DBInqVarExists(toc, "MetadataIsTimeVarying"))
    {
        int mdFlag;
        DBReadVar(toc, "MetadataIsTimeVarying", &mdFlag);
        if (mdFlag == 1)
            metadataIsTimeVarying = true;
    }
}

// ****************************************************************************
//  Function: ExceptionGenerator
//
//  Purpose:
//      Generates a Silo exception so that Silo's error handling can hook into
//      VisIt's error handling library.
//
//  Programmer: Hank Childs
//  Creation:   April 9, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Aug 16 11:06:27 PDT 2001
//    No longer throw an exception.
//
//    Jeremy Meredith, Tue Oct  4 16:08:28 PDT 2005
//    The msg variable can actually be NULL as sent from Silo.  Trying to
//    print this to a debug log can actually put the log itself into a
//    bad state.  (It's clearly not a good idea anyway.)  I changed the
//    error message for when msg==NULL.
//
//    Mark C. Miller, Wed Feb  8 10:37:38 PST 2012
//    Made it a static function
//
//    Mark C. Miller, Thu May  3 11:10:06 PDT 2018
//    Made it issue warning message too.
// ****************************************************************************
static void
ExceptionGenerator(char *msg)
{
    if (msg)
    {
        if (strstr(msg, "Checksum failure"))
            avtCallback::IssueWarning(msg);
        debug1 << "The following Silo error occurred: " << msg << endl;
    }
    else
    {
        debug1 << "A Silo error occurred, but the Silo library did not "
               << "generate an error message." << endl;
    }
}


// ****************************************************************************
//  Function: GenerateName
//
//  Purpose:
//      Generates a name from a directory and a file name.
//
//  Arguments:
//      dirname   A directory name.
//      varname   The name of the variable.
//
//  Returns:      A string for the directory and variable.
//
//  Notes:        The caller must free the return value.
//
//  Programmer: Hank Childs
//  Creation:   October 30, 2001
//
//  Modifications:
//
//    Hank Childs, Mon Dec  1 14:13:30 PST 2003
//    Do a better job of handling variables that have absolute paths.
//
//    Mark C. Miller, Wed Feb  6 12:23:56 PST 2008
//    Made it handle the case where a single silo file contains multiple
//    timesteps -- topDir is NOT "/". In this case, it has to filter everything
//    out of either dirname or varname that is the top directory name.
//
//    Tom Fogal, Mon Jul 21 14:43:14 EDT 2008
//    I added an early exit clause for the case where we lack a '/' in the
//    directory name.  This fixes an invalid read.
//
// ****************************************************************************

char *
GenerateName(const char *dirname, const char *varname, const char *topdirname)
{
    if (varname[0] == '/')
    {
        //
        // Figure out any adjustment necessary in the case that we're treating this
        // silo file as though the 'top' dir is not "/" -- this is the case where
        // a single silo file contains a whole time series in separate dirs.
        //
        int partOfPathThatIsReallyTopDirName = 0;
        if (! (topdirname[0] == '/' && topdirname[1] == '\0'))
        {
            while (varname[partOfPathThatIsReallyTopDirName] == 
                   topdirname[partOfPathThatIsReallyTopDirName])
                partOfPathThatIsReallyTopDirName++;
        }
        int tdOffset = partOfPathThatIsReallyTopDirName; // shorter name

        int len = strlen(&varname[tdOffset]);
        int num_slash = 0;
        for (int i = 0 ; i < len ; i++)
            if (varname[tdOffset+i] == '/')
                num_slash++;

        //
        // If there are lots of slashes, then we have a fully qualified path,
        // so leave them all in.  If there is only one slash (and it is the
        // first one), then take out the slash -- since the var would be
        // referred to as "Mesh", not "/Mesh".
        //
        int offset = (num_slash > 1 ? 0 : 1); 
        char *rv = new char[strlen(varname+tdOffset)+1];
        strcpy(rv, varname+offset+tdOffset);
        return rv;
    }

    //
    // Figure out any adjustment necessary in the case that we're treating this
    // silo file as though the 'top' dir is not "/" -- this is the case where
    // a single silo file contains a whole time series in separate dirs.
    //
    int partOfPathThatIsReallyTopDirName2 = 0;
    if (! (topdirname[0] == '/' && topdirname[1] == '\0'))
    {
        while (dirname[partOfPathThatIsReallyTopDirName2] == 
               topdirname[partOfPathThatIsReallyTopDirName2] &&
               dirname[partOfPathThatIsReallyTopDirName2] != '\0')
                partOfPathThatIsReallyTopDirName2++;
    }
    int tdOffset2 = partOfPathThatIsReallyTopDirName2; // shorter name

    int amtForSlash = 1;
    int amtForNullChar = 1;
    int amtForMiddleSlash = 1;
    int len = strlen(dirname) + strlen(varname) - amtForSlash + amtForNullChar
                                                + amtForMiddleSlash;

    char *rv = new char[len];

    // In the case that the string is `simple', then the calculation
    // tdOffset2+1 actually points beyond the end of the string.  We need to
    // make sure we don't deref the pointer we'll make with it, so check for
    // that simple case and bail out here if possible.
    if (strlen(dirname) < (size_t)tdOffset2+1)
    {
        sprintf(rv, "%s", varname);
        return rv;
    }

    const char *dir_without_leading_slash = dirname+1+tdOffset2;

    bool needMiddleSlash = false;

    if (*dir_without_leading_slash != '\0')
    {
        needMiddleSlash = true;
    }

    if (needMiddleSlash)
    {
        sprintf(rv, "%s/%s", dir_without_leading_slash, varname);
    }
    else
    {
        sprintf(rv, "%s%s", dir_without_leading_slash, varname);
    }

    return rv;
}


// ****************************************************************************
//  Function: PrepareDirName
//
//  Purpose:
//      Removes the appended variable and '/' from a character string and
//      returns a directory name.
//
//  Arguments:
//      dirvar  The directory and variable name in a string.
//      curdir  The current directory.
//
//  Programmer: Hank Childs
//  Creation:   October 31, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Jul 12 16:33:56 PDT 2002
//    Renamed routine from RemoveVarName and added support for relative
//    directory names.
//
//    Jeremy Meredith, Wed Mar 19 09:19:52 PST 2003
//    Changed a '>=' test to a '>' test to prevent a possible ABR.  Renamed
//    to PrepareDirName to match its intended use a little better.
//
// ****************************************************************************

string
PrepareDirName(const char *dirvar, const char *curdir)
{
    int len = strlen(dirvar);
    const char *last = dirvar + (len-1);
    while (*last != '/' && last > dirvar)
    {
        last--;
    }

    if (*last != '/')
    {
        debug1 << "Unexpected case -- no dirs what-so-ever." << endl;
    }

    char str[1024];
    size_t dirlen = 0;
    if (dirvar[0] != '/')
    {
        //
        // We have a relative path -- prepend the current directory.
        //
        strcpy(str, curdir);
        dirlen = strlen(str);
    }
    strncpy(str+dirlen, dirvar, last-dirvar);
    str[dirlen + (last-dirvar)] = '\0';

    return string(str);
}


// ****************************************************************************
//  Function: SplitDirVarName
//
//  Purpose:
//      Removes the prepended directory and '/' from a character string and
//      returns the directory and variable name.
//
//  Arguments:
//      dirvar  The directory and variable name in a string.
//      curdir  The current directory in a string.
//
//  Programmer: Jeremy Meredith
//  Creation:   March 19, 2003
//
//  Modifications:
//
//    Mark C. Miller, Sun Dec  3 12:20:11 PST 2006
//    Fixed memory problem when strlen dirvar is zero
// ****************************************************************************

void
SplitDirVarName(const char *dirvar, const char *curdir,
                string &dir, string &var)
{
    dir="";
    var="";
    size_t len;

    if (!dirvar || ((len = strlen(dirvar)) == 0))
    {
        dir=curdir;
        var="";
        return;
    }

    const char *last = dirvar + (len-1);
    while (*last != '/' && last > dirvar)
    {
        last--;
    }

    if (*last != '/')
    {
        dir=curdir;
        var=dirvar;
        return;
    }

    char str[1024];
    size_t dirlen = 0;
    if (dirvar[0] != '/')
    {
        //
        // We have a relative path -- prepend the current directory.
        //
        strcpy(str, curdir);
        dirlen = strlen(str);
    }
    strcpy(str+dirlen, dirvar);

    str[dirlen + (last-dirvar)] = '\0';

    dir=str;
    var=str + dirlen + (last-dirvar) + 1;
}


// ****************************************************************************
//  Function: SiloZoneTypeToVTKZoneType
//
//  Purpose:
//      Converts a zone type in Silo to a zone type in VTK.
//
//  Arguments:
//      zonetype      The zone type in Silo.
//
//  Returns:     The zone type in VTK.
//
//  Programmer:  Hank Childs
//  Creation:    August 15, 2000
//
// ****************************************************************************

int
SiloZoneTypeToVTKZoneType(int zonetype)
{
    int  vtk_zonetype = -1;

    switch (zonetype)
    {
      case DB_ZONETYPE_POLYGON:
        vtk_zonetype = VTK_POLYGON;
        break;
      case DB_ZONETYPE_TRIANGLE:
        vtk_zonetype = VTK_TRIANGLE;
        break;
      case DB_ZONETYPE_QUAD:
        vtk_zonetype = VTK_QUAD;
        break;
      case DB_ZONETYPE_POLYHEDRON:
        vtk_zonetype = -2;
        break;
      case DB_ZONETYPE_TET:
        vtk_zonetype = VTK_TETRA;
        break;
      case DB_ZONETYPE_PYRAMID:
        vtk_zonetype = VTK_PYRAMID;
        break;
      case DB_ZONETYPE_PRISM:
        vtk_zonetype = VTK_WEDGE;
        break;
      case DB_ZONETYPE_HEX:
        vtk_zonetype = VTK_HEXAHEDRON;
        break;
      case DB_ZONETYPE_BEAM:
        vtk_zonetype = VTK_LINE;
        break;
    }

    return vtk_zonetype;
}


// ****************************************************************************
//  Function: TranslateSiloWedgeToVTKWedge
//
//  Purpose:
//      The silo and VTK wedges are stored differently; translate between them.
//
//  Arguments:
//      siloWedge     A list of nodes from a Silo node list.
//      vtkWedge      The list of nodes in VTK ordering.
//
//  Programmer: Hank Childs
//  Creation:   September 26, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Wed May 23 17:09:48 PDT 2001
//    Changed orientation.
//
//    Kathleen Bonnell, Thu Aug  9 15:50:15 PDT 2001
//    Changed parameters from int to vtkIdType to match VTK 4.0 API.
//
//    Hank Childs, Tue Jan 11 14:48:38 PST 2005
//    We were following the comment in vtkWedge.h, which turns out to be
//    wrong.  Correct it now.
//
// ****************************************************************************

void
TranslateSiloWedgeToVTKWedge(const int *siloWedge, vtkIdType vtkWedge[6])
{
    //
    // The Silo wedge stores the four base nodes as 0, 1, 2, 3 and the two
    // top nodes as 4, 5.  The VTK wedge stores them as two triangles.  When
    // getting the exact translation, it is useful to look at the face lists
    // and edge lists in vtkWedge.cxx.
    //
    vtkWedge[0] = siloWedge[2];
    vtkWedge[1] = siloWedge[1];
    vtkWedge[2] = siloWedge[5];
    vtkWedge[3] = siloWedge[3];
    vtkWedge[4] = siloWedge[0];
    vtkWedge[5] = siloWedge[4];
}


// ****************************************************************************
//  Function: TranslateSiloPyramidToVTKPyramid
//
//  Purpose:
//    The silo and VTK pyramids are stored differently; translate between them.
//
//  Arguments:
//    siloPyramid     A list of nodes from a Silo node list.
//    vtkPyramid      The list of nodes in VTK ordering.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   May 23, 2001
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug  9 15:50:15 PDT 2001
//    Changed parameters from int to vtkIdType to match VTK 4.0 API.
//
// ****************************************************************************

void
TranslateSiloPyramidToVTKPyramid(const int *siloPyramid, vtkIdType vtkPyramid[5])
{
    //
    // The Silo pyramid stores the four base nodes as 0, 1, 2, 3 in 
    // opposite order from the VTK wedge. When getting the exact translation, 
    // it is useful to look at the face lists and edge lists in 
    // vtkPyramid.cxx.
    //
    vtkPyramid[0] = siloPyramid[0];
    vtkPyramid[1] = siloPyramid[3];
    vtkPyramid[2] = siloPyramid[2];
    vtkPyramid[3] = siloPyramid[1];
    vtkPyramid[4] = siloPyramid[4];
}


// ****************************************************************************
//  Function: TranslateSiloTetrahedronToVTKTetrahedron
//
//  Purpose:
//    The silo and VTK tetrahedrons are stored differently; translate between
//     them.
//
//  Arguments:
//    siloTetrahedron     A list of nodes from a Silo node list.
//    vtkTetrahedron      The list of nodes in VTK ordering.
//
//  Programmer:  Hank Childs 
//  Creation:    January 11, 2005
//
// ****************************************************************************

void
TranslateSiloTetrahedronToVTKTetrahedron(const int *siloTetrahedron,
                                         vtkIdType vtkTetrahedron[4])
{
    //
    // The Silo and VTK tetrahedrons are inverted.
    //
    vtkTetrahedron[0] = siloTetrahedron[1];
    vtkTetrahedron[1] = siloTetrahedron[0];
    vtkTetrahedron[2] = siloTetrahedron[2];
    vtkTetrahedron[3] = siloTetrahedron[3];
}

// ****************************************************************************
//  Function: TetIsInverted 
//
//  Purpose: Determine if Tets in Silo are inverted from Silo's Normal ordering
//
//  Programmer:  Mark C. Miller 
//  Creation:    March 21, 2007 
//
//  Modifications:
//
//    Mark C. Miller, Mon Jul 26 23:13:06 PDT 2010
//    Replaced assumption of float data with GetTuple, converting all to
//    double for purposes of the computation performed here. This is necessary
//    as newer versions of the Silo plugin can serve up arbitrary type.
// ****************************************************************************

bool
TetIsInverted(const int *siloTetrahedron, vtkUnstructuredGrid *ugrid)
{
    //
    // initialize set of 4 points of tet
    //
    vtkDataArray *pda = ugrid->GetPoints()->GetData();
    double p[4][3];
    for (int i = 0; i < 4; i++)
        pda->GetTuple(siloTetrahedron[i], p[i]);

    //
    // Compute a vector normal to plane of first 3 points
    //
    double n1[3] = {p[1][0] - p[0][0], p[1][1] - p[0][1], p[1][2] - p[0][2]};
    double n2[3] = {p[2][0] - p[0][0], p[2][1] - p[0][1], p[2][2] - p[0][2]};
    double n1Xn2[3] = {  n1[1]*n2[2] - n1[2]*n2[1],
                      -(n1[0]*n2[2] - n1[2]*n2[0]),
                        n1[0]*n2[1] - n1[1]*n2[0]};
    
    //
    // Compute a dot-product of normal with a vector to the 4th point.
    // If the tet is specified as Silo normally expects it, this dot
    // product should be negative. If it is not negative, then tets
    // are inverted
    //
    double n3[3] = {p[3][0] - p[0][0], p[3][1] - p[0][1], p[3][2] - p[0][2]};
    double n3Dotn1Xn2 = n3[0]*n1Xn2[0] + n3[1]*n1Xn2[1] + n3[2]*n1Xn2[2];

    if (n3Dotn1Xn2 > 0)
        return true;
    else
        return false;
}


// ****************************************************************************
//  Function: GetMultivarToMultimeshMap
//
//  Purpose: Handle explicit multivar to multimesh mapping convention 
//
//  Programmer: Mark C. Miller 
//  Creation:   December 12, 2006 
//
// ****************************************************************************
void
avtSiloFileFormat::GetMultivarToMultimeshMap(DBfile *dbfile)
{
    int lvars = DBGetVarLength(dbfile, "MultivarToMultimeshMap_vars");
    int lmeshes = DBGetVarLength(dbfile, "MultivarToMultimeshMap_meshes");
    if (lvars > 0 && lmeshes > 0)
    {
        size_t i;
        string tmpStr;

        char  *vars = new char[lvars+1];
        for (i = 0 ; i < (size_t)lvars+1 ; i++)
            vars[i] = '\0';
        DBReadVar(dbfile, "MultivarToMultimeshMap_vars", vars);

        vector<string> varVec;
        tmpStr.clear();
        for (i = 0 ; i < (size_t)lvars+1; i++)
        {
            if (vars[i] == ';' || (i == (size_t)lvars))
            {
                varVec.push_back(tmpStr);
                tmpStr.clear();
            }
            else
            {
                tmpStr += vars[i];
            }
        }
        delete [] vars;

        char  *meshes = new char[lmeshes+1];
        for (i = 0 ; i < (size_t)lmeshes+1 ; i++)
            meshes[i] = '\0';
        DBReadVar(dbfile, "MultivarToMultimeshMap_meshes", meshes);

        vector<string> meshVec;
        tmpStr.clear();
        for (i = 0 ; i < (size_t)lmeshes+1; i++)
        {
            if (meshes[i] == ';' || (i == (size_t)lmeshes))
            {
                meshVec.push_back(tmpStr);
                tmpStr.clear();
            }
            else
            {
                tmpStr += meshes[i];
            }
        }
        delete [] meshes;

        if (varVec.size() != meshVec.size())
        {
            debug1 << "Unable to build multivarToMultimeshMap because "
                   << "varVec.size()=" << varVec.size() << ", but "
                   << "meshVec.size()=" << meshVec.size() << endl;
        }

        debug5 << "Building multivarToMultimeshMap" << endl;
        for (i = 0; i < varVec.size(); i++)
        {
            debug5 << "    var \"" << varVec[i] << "\" : mesh \"" << meshVec[i] << "\"" << endl;
            multivarToMultimeshMap[varVec[i]] = meshVec[i];
        }
    }
}

// ****************************************************************************
//  Function: GuessCodeNameFromTopLevelVars
//
//  Purpose: Guess name of the code that produced the data 
//
//  Programmer: Mark C. Miller 
//  Creation:   July 9, 2007 
//
//  Modifications:
//
//    Hank Childs, Fri Oct  5 09:16:29 PDT 2007
//    Fix typo in print statement.
//
//    Mark C. Miller, Thu Mar 27 09:57:06 PST 2008
//    Added Block Structured code detection.
//
// ****************************************************************************
static string
GuessCodeNameFromTopLevelVars(DBfile *dbfile)
{
    string retval = "Unknown";

    if (DBInqVarExists(dbfile, "lineage") &&
        DBInqVarExists(dbfile, "version_number") &&
        DBInqVarExists(dbfile, "znburn_flag") &&
        DBInqVarExists(dbfile, "chemistry_flag"))
    {
        retval = "Ale3d";
    }
    else if (DBInqVarExists(dbfile, "silo_file_date") &&
             DBInqVarType(dbfile, "Global") == DB_DIR &&
             DBInqVarType(dbfile, "Decomposition") == DB_DIR)
    {
        retval = "BlockStructured";
    }

    debug5 << "Guessing this Silo file was produced by code \"" << retval << "\"" << endl;
    return retval;
}

// ****************************************************************************
//  Function: AddAle3drlxstatEnumerationInfo
//
//  Purpose: Add enumeration info for Ale3d's rlxstat variable. These names and
//  values were taken directly from Ale3d's RelaxTest.h file.
//
//  Programmer: Mark C. Miller 
//  Creation:   July 9, 2007 
//
//  Modifications:
//    Mark C. Miller, Mon Apr 14 15:41:21 PDT 2008
//    Changed interface to enumerated scalars
// ****************************************************************************
static void
AddAle3drlxstatEnumerationInfo(avtScalarMetaData *smd)
{
    smd->SetEnumerationType(avtScalarMetaData::ByValue);
    smd->AddEnumNameValue("RLX_Uninitialized_-1",-1);
    smd->AddEnumNameValue("RLX_Constrained_0",0);
    smd->AddEnumNameValue("RLX_JustRelaxed_1",1);
    smd->AddEnumNameValue("RLX_Relaxing_2",2);
    smd->AddEnumNameValue("RLX_MustRelax_3",3);
    smd->AddEnumNameValue("RLX_DispShortEdge_4",4);
    smd->AddEnumNameValue("RLX_DispLagMotion_5",5);
    smd->AddEnumNameValue("RLX_InflowOutflow_6",6);
    smd->AddEnumNameValue("RLX_Symmetry_7",7);
    smd->AddEnumNameValue("RLX_AngleWall_8",8);
    smd->AddEnumNameValue("RLX_MustRelaxLimited_9",9);
    smd->AddEnumNameValue("RLX_AdvectTangential_10",10);
    smd->AddEnumNameValue("RLX_Reaction_13",13);
    smd->AddEnumNameValue("RLX_ChemGrad_14",14);
    smd->AddEnumNameValue("RLX_HoldNodeset_15",15);
    smd->AddEnumNameValue("RLX_IntHist_16",16);
    smd->AddEnumNameValue("RLX_Velocity_20",20);
    smd->AddEnumNameValue("RLX_LightingTime_21",21);
    smd->AddEnumNameValue("RLX_Region_22",22);
    smd->AddEnumNameValue("RLX_ZNBurn_23",23);
    smd->AddEnumNameValue("RLX_PlasticStrain_24",24);
    smd->AddEnumNameValue("RLX_SALE_25",25);
    smd->AddEnumNameValue("RLX_AdvTime_26",26);
    smd->AddEnumNameValue("RLX_HoldUntilActive_27",27);
    smd->AddEnumNameValue("RLX_SlaveExtension_29",29);
    smd->AddEnumNameValue("RLX_PartialInvalid_30",30);
    smd->AddEnumNameValue("RLX_FreeSurface_31",31);
    smd->AddEnumNameValue("RLX_MixedNode_32",32);
    smd->AddEnumNameValue("RLX_IgnitionPt_33",33);
    smd->AddEnumNameValue("RLX_SlideMaster_34",34);
    smd->AddEnumNameValue("RLX_PeriodicRelax_35",35);
    smd->AddEnumNameValue("RLX_HoldUntilGrace_36",36);
    smd->AddEnumNameValue("RLX_HeldIntHist_37",37);
    smd->AddEnumNameValue("RLX_Equilibrated_38",38);
    smd->AddEnumNameValue("RLX_ShellNode_39",39);
    smd->AddEnumNameValue("RLX_BeamNode_40",40);
}

// ****************************************************************************
//  Method: AddNodelistEnumerations
//
//  Purpose: Add node list enumerations for block structured codes. 
//
//  Programmer: Mark C. Miller 
//  Creation:   March 18, 2008 
//
//  Modifications:
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Added call to clear nlBlockToWindowsMap before build-it, or possibly
//    re-building from a second or more call to this method.
//
//    Mark C. Miller, Tue Mar  3 19:31:37 PST 2009
//    As per Cyrus' recommendation, forced it to work only if mesh name
//    is specifically 'hydro_mesh' but left all other logic (which supports
//    perhaps multiple meshes) in place.
//
//    Mark C. Miller, Thu Apr 12 23:14:21 PDT 2012
//    Replaced use of NChooseR enumeration mode with ByBitMap.
//
//    Cyrus Harrison, Fri Aug 16 10:07:47 PDT 2013
//    Added support for nodelists placed @ /Nodelists/
//
// ****************************************************************************
void
avtSiloFileFormat::AddNodelistEnumerations(DBfile *dbfile, avtDatabaseMetaData *md,
    string meshname)
{
    // mesh names that adhere to this nodelist format
    if (!( meshname != "hydro_mesh" ||
           meshname != "MMESH" ||
           meshname != "MESH") )
        return;

    // check for nodelists
    ostringstream oss;
    string nodelist_base = "/Global/Nodelists";

    if (DBInqVarType(dbfile, nodelist_base.c_str()) != DB_DIR)
    {
        // it not found @ "/Global/Nodelists", check for "Nodelists" in the root of the silo file.
        nodelist_base  = "/Nodelists";
        if (DBInqVarType(dbfile, nodelist_base.c_str()) != DB_DIR)
            return;
    }

    debug5 << "Adding Nodelist for mesh = " << meshname <<endl;
    debug5 << "Nodelist silo base path = " << nodelist_base <<endl;

    oss.str("");
    oss << nodelist_base << "/NumberNodelists";
    DBReadVar(dbfile, oss.str().c_str(), &numNodeLists);

    // Note, if we ever remove the restriction on meshname, above, we need
    // to make sure we don't wind up defining the same name scalar on different
    // meshes.
    avtScalarMetaData *smd = new avtScalarMetaData("Nodelists",
                                     meshname, AVT_NODECENT);

    int i;
    nlBlockToWindowsMap.clear();
    for (i = 0; i < numNodeLists; i++)
    {
        oss.str("");
        oss << nodelist_base << "/Nodelist" << i << "/Name";
        char *tmpName = (char*) DBGetVar(dbfile, oss.str().c_str());

        debug5 << "For nodelist \"" << tmpName << "\", value = " << i << endl;
        smd->AddEnumNameValue(tmpName, i);
        free(tmpName);

        oss.str("");
        oss << nodelist_base << "/Nodelist" << i << "/NumberWindows";
        int numWindows;
        DBReadVar(dbfile, oss.str().c_str(), &numWindows);

        debug5 << "    NumberWindows = " << numWindows << endl;
        for (int j = 0; j < numWindows; j++)
        {
            debug5 << "        For Window " << j << endl;
            oss.str("");
            oss <<  nodelist_base << "/Nodelist" << i << "/Block" << j;
            int blockNum;
            DBReadVar(dbfile, oss.str().c_str(), &blockNum);
            nlBlockToWindowsMap[blockNum].push_back(i);
            debug5 << "            Block = " << blockNum << endl;

            debug5 << "            Extents = ";
            oss.str("");
            oss <<  nodelist_base << "/Nodelist" << i << "/Extents" << j;
            int extents[6];
            DBReadVar(dbfile, oss.str().c_str(), extents);
            for (int k = 0; k < 6; k++)
            {
                nlBlockToWindowsMap[blockNum].push_back(extents[k]);
                debug5 << extents[k] << ", ";
            }
            debug5 << endl;
        }
    }

    smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
    smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
    smd->hideFromGUI = true;

    md->Add(smd);
}

// ****************************************************************************
//  Method: AddAnnotIntNodelistEnumerations
//
//  Purpose: Add ANNOTATION_INT node list enumerations  
//
//  Programmer: Mark C. Miller 
//  Creation:   December 18, 2008 
//
//  Modifications:
//    Mark C. Miller, Tue Dec 23 11:12:31 PST 2008
//    Fixed wrong parameter based to setting of NChoosR min/max values.
//
//    Mark C. Miller, Tue Feb 24 16:58:31 PST 2009
//    Replaced use of DetermineFileAndDirectory with
//    DetermineFilenameAndDirectory and explicit opens. The latter DOES NOT
//    use the plugin's file management routines and this is important because
//    this method is being called from within ReadDir we cannot allow file
//    pointer stuff to change out from underneath ReadDir while it is still
//    completing. 
//
//    Mark C. Miller, Mon Mar  2 11:46:47 PST 2009
//    Undid previous change and instead added a call to the loop which has
//    the effect of keeping the toc file open. The previous logic failed
//    whenever the file being opened was not in '.' and it made more sense to
//    just try to re-use the file opening logic and management routines of
//    the plugin instead of solve problems with file naming here. Also, added
//    logic to deal with EMTPY blocks in the mesh.
//
//    Cyrus Harrison, Wed Dec 21 15:22:21 PST 2011
//    Limited support for Silo nameschemes, use new multi block cache data
//    structures.
//
//    Mark C. Miller, Thu Apr 12 23:15:08 PDT 2012
//    Replaced NChooseR enumeration mode with ByBitMap
// ****************************************************************************
void
avtSiloFileFormat::AddAnnotIntNodelistEnumerations(DBfile *dbfile,
    avtDatabaseMetaData *md, string meshname, avtSiloMultiMeshCacheEntry *obj)
{
    //
    // This is expensive as it will wind up iterating over all subfiles.
    // But, we have no choice. Fortunately, we ONLY get here if the user
    // explicitly set the read option that controls it. We also make use
    // of the fact that DetermineFileAndDirectory, will wind up opening
    // each unique silo file only once. Otherwise, we'd have to make this
    // loop smart. We do this so we can collect up all the names of all
    // the different nodelist objects.
    //
    map<string,int> arr_names;
    bool haveFaceLists = false;
    bool haveNodeLists = false;
    string mb_meshname = "";
    int nblocks = obj->NumberOfBlocks();

    for (int i = 0; i < nblocks; i++)
    {
        mb_meshname = obj->GenerateName(i);
        if ( mb_meshname == "EMPTY")
            continue;

        string realvar;
        DBfile *correctFile = 0;
        DetermineFileAndDirectory(mb_meshname.c_str(),"",
                                  correctFile, realvar);

        if (correctFile == 0)
            correctFile = dbfile;

        DBcompoundarray *ai = DBGetCompoundarray(correctFile, "ANNOTATION_INT");
        if (ai)
        {
            debug5 << "Found ANNOTATION_INT object for block " << i << endl;
            for (int j = 0; j < ai->nelems; j++)
            {
                arr_names[ai->elemnames[j]] = 1;
                int len = strlen(ai->elemnames[j]);
                if (strncmp("_face",&(ai->elemnames[j][len-5]),5) == 0)
                    haveFaceLists = true;
                else if (strncmp("_node",&(ai->elemnames[j][len-5]),5) == 0)
                    haveNodeLists = true;
            }
            DBFreeCompoundarray(ai);
        }

        //
        // Ensure that the toc file remains open while iterating through
        // this loop because the caller, ReadDir(), will expect that it
        // will not be closed.
        //
        OpenFile(tocIndex);
    }

    //
    // If we didn't find anything, we're done.
    //
    if (!haveNodeLists && !haveFaceLists)
    {
        debug5 << "Although we were asked to search for ANNOTATION_INT nodelists, none were found." << endl;
        return;
    }

    //
    // Populate the enumeration names. This is a two pass loop;
    // first for nodelists, second for facelists.
    //
    for (int pass = 0; pass < 2; pass++)
    {
        if ((pass == 0 && !haveNodeLists) || (pass == 1 && !haveFaceLists))
            continue;

        avtScalarMetaData *smd = new avtScalarMetaData(
            pass == 0 ? "AnnotInt_Nodelists" : "AnnotInt_Facelists",
            meshname, AVT_NODECENT);

        map<string, int>::const_iterator it;
        int j = 0;
        for (it = arr_names.begin(); it != arr_names.end(); it++)
        {
            int len = strlen(it->first.c_str());
            if (strncmp(pass==0?"_node":"_face",&(it->first.c_str()[len-5]),5) == 0)
            {
                smd->AddEnumNameValue(string(it->first,0,len-5), j++);
                debug5 << "Adding \"" << string(it->first,0,len-5) << "\"," << j << " to" <<
                    (pass == 0 ? " nodelist " : " zonelist ") << "enumeration" << endl;
            }
        }

        if (pass == 0)
            numAnnotIntLists = (int)smd->enumNames.size();
        else
        {
            if (smd->enumNames.size() > (size_t)numAnnotIntLists)
                numAnnotIntLists = (int)smd->enumNames.size();
        }

        smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
        smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
        smd->hideFromGUI = true;

        md->Add(smd);
    }
}

// ****************************************************************************
//  Function: GatherChildMRGTreeRegionNames
//
//  Purpose: Get all the MRG Tree region names that are direct descendents of
//  'top' MRG Tree node.
//
//  Creation: Mark C. Miller, Wed Jul 11 10:58:59 PDT 2012
//
//  Modifications:
//    Mark C. Miller, Wed Oct 31 15:52:34 PDT 2012
//    Updated to use improved Silo DBnamescheme constructor that can handle
//    external array references internally itself.
// ****************************************************************************
static void
GatherChildMRGTreeRegionNames(DBfile *dbfile, const DBmrgtnode *top, vector<string>& theNames)
{
    int i;

    if (top->num_children == 1)
    {
        if (top->children[0]->narray > 0 &&
            top->children[0]->names)
        {
            //
            // Array-based representation for the patches
            //
            DBmrgtnode *patchesArrayNode = top->children[0];

            //
            // Handle the names of the patches 
            //
            if (strchr(patchesArrayNode->names[0],'%') == 0)
            {
                // Explicitly stored names
                for (i = 0; i < patchesArrayNode->narray; i++)
                    theNames.push_back(patchesArrayNode->names[i]);
            }
            else
            {
                DBnamescheme *ns = DBMakeNamescheme(patchesArrayNode->names[0], 0, dbfile);
                for (i = 0; i < patchesArrayNode->narray; i++)
                    theNames.push_back(DBGetName(ns, i));
                DBFreeNamescheme(ns);
            }
        }
        else if (top->children[0]->narray == 0)
        {
            //
            // Single block case.
            //
            theNames.push_back(top->children[0]->name);
        }
    }
    else if (top->num_children > 1)
    {
        //
        // Individual MRG Tree nodes for each patch 
        //
        for (int q = 0; q < top->num_children; q++)
        {
            DBmrgtnode *patchChild = top->children[q];
            theNames.push_back(patchChild->name);
        }
    }
}

// ****************************************************************************
//  Function: GetCondensedGroupelMap
//
//  Purpose:  Simplify handling groupel maps for levels/children. Whether the
//  maps are stored on level/patches nodes, arrays of children of these nodes
//  or individual children of these nodes, returns a single groupel map object
//  representing the same information as the possibly one or more groupel maps
//  in the database.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 18, 2008 
//
//  Modifications
//    Mark C. Miller Wed Nov 19 20:30:19 PST 2008
//    Changed conditional for Silo version to 4.6.3
//
//    Mark C. Miller, Mon Nov 24 17:33:20 PST 2008
//    Testing gpl commit hook
//
//    Mark C. Miller, Mon Nov 24 17:33:47 PST 2008
//    Testing gpl commit hook with gnu general
//    public      license text.
//
//    Hank Childs, Mon May 25 11:26:25 PDT 2009
//    Fix macro compilation problem with old versions of Silo.
//
//    Mark C. Miller, Mon Nov  9 08:54:28 PST 2009
//    Protected calls to DBForceSingle with check for whether plugin is
//    actually forcing single precision.
//
//    Mark C. Miller, Wed Jan 20 16:35:37 PST 2010
//    Made calls to ForceSingle on and off UNconditional.
//
//    Mark C. Miller, Wed Jul 11 10:59:24 PDT 2012
//    Changed interface so that this function can be used in several different
//    MRG Tree contexts determined by the MRG Tree name, root node at which
//    maps are gathered and condensed and the groupel type.
// ****************************************************************************

static DBgroupelmap * 
GetCondensedGroupelMap(DBfile *dbfile, string mrgtnm_abspath,
    DBmrgtnode *rootNode, int forceSingle, int gpel_type)
{
    int i,k,q = 0,pass;
    DBgroupelmap *retval = 0;

    // We do this to prevent Silo for re-interpreting integer data in
    // groupel maps
    DBForceSingle(0);

    if (rootNode->num_children == 1 && rootNode->children[0]->narray == 0)
    {
        retval = DBAllocGroupelmap(0, DB_NOTYPE);
    }
    else if ((rootNode->num_children == 1 && rootNode->children[0]->narray > 0) ||
             (rootNode->num_children > 1 && rootNode->maps_name))
    {
        int nseg_mult = 1;
        DBmrgtnode *mapNode;
        if (rootNode->num_children == 1 && rootNode->children[0]->narray > 0)
        {
            nseg_mult = rootNode->children[0]->narray;
            mapNode = rootNode->children[0];
        }
        else
            mapNode = rootNode;
            
        //
        // Get the groupel map.
        //
        if (!mapNode->maps_name)
            return 0;

        string mapsName = ResolveSiloIndObjAbsPath(dbfile, mrgtnm_abspath, mapNode->maps_name);
        DBgroupelmap *gm = DBGetGroupelmap(dbfile, mapsName.c_str());

        //
        // One pass to count parts of map we'll be needing and a 2nd 
        // pass to allocate and transfer those parts to the returned map.
        //
        for (pass = 0; pass < 2 && gm; pass++)
        {
            if (pass == 1) /* allocate on 2nd pass */
            {
                retval = DBAllocGroupelmap(q, DB_NOTYPE);
                /* We won't need segment_ids because the map is condensed */ 
                free(retval->segment_ids);
                retval->segment_ids = 0;
            }

            q = 0;
            for (k = 0; k < mapNode->nsegs * nseg_mult; k++)
            {
                for (i = 0; i < gm->num_segments; i++)
                {
                    int gm_seg_id = gm->segment_ids ? gm->segment_ids[i] : i;
                    int tnode_seg_id = mapNode->seg_ids ? mapNode->seg_ids[k] : k;
                    int gm_seg_type = gm->groupel_types[i];
                    int tnode_seg_type = mapNode->seg_types[k];
                    if (gm_seg_id != tnode_seg_id ||
                        tnode_seg_type != gpel_type ||
                        gm_seg_type != gpel_type)
                        continue;

                    if (pass == 1) /* populate on 2nd pass */
                    {
                        retval->groupel_types[q] = gpel_type;
                        retval->segment_lengths[q] = gm->segment_lengths[tnode_seg_id];
                        /* Transfer ownership of segment_data to the condensed map */
                        retval->segment_data[q] = gm->segment_data[tnode_seg_id];
                        gm->segment_data[tnode_seg_id] = 0;
                    }

                    q++;
                }
            }
        }
        DBFreeGroupelmap(gm);
    }
    else
    {
        //
        // Multiple groupel maps are stored, one for each node
        //
        retval = DBAllocGroupelmap(rootNode->num_children, DB_NOTYPE);
        for (q = 0; q < rootNode->num_children; q++)
        {
            DBmrgtnode *rootChild = rootNode->children[q];
            if (!rootChild->maps_name) continue;
            string mapsName = ResolveSiloIndObjAbsPath(dbfile, mrgtnm_abspath, rootChild->maps_name);
            DBgroupelmap *gm = DBGetGroupelmap(dbfile, mapsName.c_str());
            for (k = 0; k < rootChild->nsegs && gm; k++)
            {
                for (i = 0; i < gm->num_segments; i++)
                {
                    int gm_seg_id = gm->segment_ids ? gm->segment_ids[i] : i;
                    int tnode_seg_id = rootChild->seg_ids ? rootChild->seg_ids[k] : k;
                    int gm_seg_type = gm->groupel_types[i];
                    int tnode_seg_type = rootChild->seg_types[k];
                    if (gm_seg_id != tnode_seg_id ||
                        tnode_seg_type != gpel_type ||
                        gm_seg_type != gpel_type)
                        continue;

                    retval->groupel_types[q] = gpel_type;
                    retval->segment_lengths[q] = gm->segment_lengths[i];
                    retval->segment_data[q] = gm->segment_data[i];
                    gm->segment_data[i] = 0;
                }
            }
            DBFreeGroupelmap(gm);
        }
    }

    DBForceSingle(forceSingle);
    return retval;
}

// ****************************************************************************
//  Function: HandleMrgtreeAMRGroups
//
//  Purpose: Process the AMR parts of a mesh region grouping (mrg) tree. Also
//  handles whatever naming scheme the database specifies for levels and
//  patches.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 18, 2008 
//
//  Modifications
//
//    Mark C. Miller Wed Nov 19 20:30:19 PST 2008
//    Changed conditional for Silo version to 4.6.3
//
//    Hank Childs, Mon May 25 11:26:25 PDT 2009
//    Fix macro compilation problem with old versions of Silo.
//
//    Mark C. Miller, Mon Nov  9 10:43:05 PST 2009
//    Added forceSingle arg.
//
//    Mark C. Miller, Wed Jul 11 11:00:54 PDT 2012
//    Changed name to make it clearer what his function is used for. Adjusted
//    to use abs pathname for MRG Tree as well. Also, re-factored code to
//    get all the child MRG Tree region names to a new function.
// ****************************************************************************

static void
HandleMrgtreeAMRGroups(DBfile *dbfile, DBmultimesh *mm, const char *multimesh_name,
    avtMeshType *mt, int *num_groups, vector<int> *group_ids, vector<string> *block_names,
    int forceSingle)
{
    int i, j;
    bool probablyAnAMRMesh = true; (void) probablyAnAMRMesh;
    DBgroupelmap *gm = 0;  (void) gm;

    if (mm == 0)
        return;

    if (*mt != AVT_CURVILINEAR_MESH && *mt != AVT_RECTILINEAR_MESH)
        return;

    //
    // Get the mesh region grouping tree
    //
    if (mm->mrgtree_name == 0)
    {
        debug3 << "No mrgtree specified for mesh \"" << multimesh_name << "\"" << endl;
        return;
    }

    string mrgtnm_abspath = ResolveSiloIndObjAbsPath(dbfile, multimesh_name, mm->mrgtree_name);

    DBmrgtree *mrgTree = DBGetMrgtree(dbfile, mrgtnm_abspath.c_str());
    if (mrgTree == 0)
    {
        debug3 << "Unable to find mrgtree named \"" << mrgtnm_abspath << "\"" << endl;
        return;
    }

    //
    // Try to go to the amr_decomp node in the tree
    //
    if (DBSetCwr(mrgTree, "amr_decomp") < 0)
    {
        debug3 << "Although mrgtree \"" << mm->mrgtree_name << "\" exists, "
               << "it does not contain node named \"amr_decomp\"." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    //
    // Try to go to the 'levels' part of the amr_decomp
    //
    if (DBSetCwr(mrgTree, "levels") < 0)
    {
        debug3 << "Although a node named \"amr_decomp\" exists in \" "
               << mm->mrgtree_name << "\", it does not contain a child node"
               << "named \"levels\"." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }
    DBmrgtnode *levelsNode= mrgTree->cwr;

    //
    // Get level grouping information from the levels subtree
    //
    DBgroupelmap *lvlgm = GetCondensedGroupelMap(dbfile, mrgtnm_abspath, levelsNode,
        forceSingle, DB_BLOCKCENT);
    if (!lvlgm)
    {
        debug3 << "Unable got get condensed groupel map for \"levels\"" << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    *num_groups = lvlgm->num_segments;
    group_ids->resize(mm->nblocks,-1);
    for (i = 0; i < lvlgm->num_segments; i++)
    {
        for (j = 0; j < lvlgm->segment_lengths[i]; j++)
        {
            int patch_no = ((int**) lvlgm->segment_data)[i][j];
            (*group_ids)[patch_no] = i; 
        }
    }
    DBFreeGroupelmap(lvlgm);

    DBSetCwr(mrgTree, "..");
    if (DBSetCwr(mrgTree, "patches") < 0)
    {
        debug3 << "Although a node named \"amr_decomp\" exists in \" "
               << mm->mrgtree_name << "\", it does not contain a child node"
               << "named \"patches\"." << endl;
        *num_groups = 0;
        group_ids->clear();
        DBFreeMrgtree(mrgTree);
        return;
    }

    //
    // Override the mesh type to be an AMR mesh
    //
    *mt = AVT_AMR_MESH;

    //
    // Set the block names according to contents of MRG Tree
    //
    DBmrgtnode *patchesNode = mrgTree->cwr;
    GatherChildMRGTreeRegionNames(dbfile, patchesNode, *block_names);

    DBFreeMrgtree(mrgTree);
    return;
}

// ****************************************************************************
//  Function: BuildDomainAuxiliaryInfoForAMRMeshes 
//
//  Purpose: Builds domain nesting and boundary objects for AMR meshes. 
//  patches.
//
//  Programmer: Mark C. Miller 
//  Creation:   November 18, 2008 
//
//  Modifications
//    Mark C. Miller Wed Nov 19 20:30:19 PST 2008
//    Changed conditional for Silo version to 4.6.3
//
//    Mark C. Miller, Tue Dec  9 00:26:27 PST 2008
//    Testing hooks so adding comment to force update
//
//    Mark C. Miller, Tue Dec  9 23:34:39 PST 2008
//    Testing hooks by adding tab characters
//
//    Hank Childs, Mon May 25 11:26:25 PDT 2009
//    Add support for old versions of Silo.
//
//    Mark C. Miller, Mon Nov  9 08:54:59 PST 2009
//    Protecting calls to DBForceSingle with check to see if plugin is
//    really forcing single.
//
//    Mark C. Miller, Wed Jan 20 16:35:37 PST 2010
//    Made calls to ForceSingle on and off UNconditional.
//
//    Cyrus Harrison, Mon Mar 22 15:07:25 PDT 2010
//    Use curvi domain boundries if db_mesh_type == DB_QUADMESH.
//
//    Cyrus Harrison, Mon Mar 22 15:07:25 PDT 2010
//    This function requires db_mesh_type to be either DB_QUAD_RECT or DB_QUAD_CURV
//    (DB_QUADMESH is ambiguous).
//
//    Mark C. Miller, Wed Jul 11 11:03:35 PDT 2012
//    Adjusted to use absolute path of MRG Tree in calls to get it or its maps.
// ****************************************************************************
static void
BuildDomainAuxiliaryInfoForAMRMeshes(DBfile *dbfile, DBmultimesh *mm,
    const char *meshName, int timestate, int db_mesh_type,
    avtVariableCache *cache, int forceSingle)
{
#ifdef MDSERVER

    return;

#else

    int i, j;
    int num_levels = 0;
    int num_patches = 0;
    int num_dims = 0;

    //
    // First, look to see if we don't already have it cached
    // Note that we compute BOTH domain nesting and domain boundary
    // information here. However, we use only existance of domain
    // nesting object in cache as trigger for whether to compute
    // both objects or not.
    //
    void_ref_ptr vrTmp = cache->GetVoidRef("any_mesh",
                                   AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
                                   timestate, -1);
    if (*vrTmp != NULL)
        return;

    //
    // Get the mesh region grouping tree
    //
    if (mm->mrgtree_name == 0)
    {
        debug3 << "No mrgtree specified for mesh \"" << meshName << "\"" << endl;
        return;
    }

    string mrgtnm_abspath = ResolveSiloIndObjAbsPath(dbfile, meshName, mm->mrgtree_name);

    DBmrgtree *mrgTree = DBGetMrgtree(dbfile, mrgtnm_abspath.c_str());
    if (mrgTree == 0)
    {
        debug3 << "Unable to find mrgtree named \"" << mrgtnm_abspath << "\"" << endl;
        return;
    }

    //
    // Look through all the mrgtree's variable object names to see if
    // any define 'ratios' or 'ijk' extents. They are needed to
    // compute domain nesting and neighbor information.
    //
    char **vname = mrgTree->mrgvar_onames;
    string ratioVarName;
    string ijkExtsVarName;
    while (vname && *vname != 0)
    {
        string vnameTmp = *vname;
        for (size_t k = 0; k < vnameTmp.size(); k++)
            vnameTmp[k] = tolower(vnameTmp[k]);

        if (vnameTmp.find("ratio") != string::npos)
            ratioVarName = *vname;
        if (vnameTmp.find("ijk") != string::npos)
            ijkExtsVarName = *vname;

        vname++;
    }
    if (ratioVarName == "")
    {
        debug3 << "Although mrgtree \"" << mm->mrgtree_name << "\" exists, "
               << "it does not appear to have either a ratios variable." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }
    if (ijkExtsVarName == "")
    {
        debug3 << "Although mrgtree \"" << mm->mrgtree_name << "\" exists, "
               << "it does not appear to have either an ijk extents variable." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    //
    // Try to go to the amr_decomp node in the tree
    //
    if (DBSetCwr(mrgTree, "amr_decomp") < 0)
    {
        debug3 << "Although mrgtree \"" << mm->mrgtree_name << "\" exists, "
               << "it does not contain node named \"amr_decomp\"." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    //
    // Try to go to the 'levels' part of the amr_decomp
    //
    if (DBSetCwr(mrgTree, "levels") < 0)
    {
        debug3 << "Although a node named \"amr_decomp\" exists in \" "
               << mm->mrgtree_name << "\", it does not contain a child node"
               << "named \"levels\"." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }
    DBmrgtnode *levelsNode = mrgTree->cwr;

    //
    // Get level grouping information from tree
    //
    DBgroupelmap *lvlgm = GetCondensedGroupelMap(dbfile, mrgtnm_abspath, levelsNode,
        forceSingle, DB_BLOCKCENT);
    if (!lvlgm)
    {
        debug3 << "Unable to get condensed groupel map for \"levels\"" << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    num_levels = lvlgm->num_segments;
    debug5 << "num_levels = " << num_levels << endl;
    vector<int> levelId;
    levelId.resize(mm->nblocks,-1);
    for (i = 0; i < lvlgm->num_segments; i++)
    {
        for (j = 0; j < lvlgm->segment_lengths[i]; j++)
        {
            int patch_no = ((int**) lvlgm->segment_data)[i][j];
            levelId[patch_no] = i; 
        }
    }
    DBFreeGroupelmap(lvlgm);

    //
    // Try to go to the patches part of the amr_decomp
    //
    DBSetCwr(mrgTree, "..");
    if (DBSetCwr(mrgTree, "patches") < 0)
    {
        debug3 << "Although a node named \"amr_decomp\" exists in \" "
               << mm->mrgtree_name << "\", it does not contain a child node"
               << "named \"patches\"." << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }
    DBmrgtnode *childsNode = mrgTree->cwr;

    //
    // Get Parent/Child maps
    //
    DBgroupelmap *chldgm = GetCondensedGroupelMap(dbfile, mrgtnm_abspath, childsNode,
        forceSingle, DB_BLOCKCENT);
    if (!chldgm)
    {
        debug3 << "Unable to get condensed groupel map for \"parent/child\"" << endl;
        DBFreeMrgtree(mrgTree);
        return;
    }

    //
    // Read the ratios variable (on the levels) and the parent/child
    // map.
    //
    DBForceSingle(0);
    DBmrgvar *ratvar = DBGetMrgvar(dbfile, ratioVarName.c_str());
    DBmrgvar *ijkvar = DBGetMrgvar(dbfile, ijkExtsVarName.c_str());
    DBForceSingle(forceSingle);

    //
    // The number of patches can be inferred from the size of the child groupel map.
    //
    num_patches = chldgm->num_segments;
    debug5 << "num_patches = " << num_patches << endl;

    //
    // The number of dimensions can be inferred from the number of components in
    // the ratios variable.
    // 
    num_dims = ratvar->ncomps;
    debug5 << "num_dims = " << num_dims << endl;

    //
    // build the avtDomainNesting object
    //
    avtStructuredDomainNesting *dn =
        new avtStructuredDomainNesting(num_patches, num_levels);

    dn->SetNumDimensions(num_dims);

    //
    // Set refinement level ratio information
    //
    vector<int> ratios(3,1);
    dn->SetLevelRefinementRatios(0, ratios);
    for (i = 1; i < num_levels; i++)
    {
        int **ratvar_data = (int **) ratvar->data;
        ratios[0] = ratvar_data[0][i]; 
        ratios[1] = ratvar_data[1][i];
        ratios[2] = num_dims == 3 ? (int) ratvar_data[2][i]: 0;
            debug5 << "ratios = " << ratios[0] << ", " << ratios[1] << ", " << ratios[2] << endl;
        dn->SetLevelRefinementRatios(i, ratios);
    }

    //
    // set each domain's level, children and logical extents
    //
    for (i = 0; i < num_patches; i++)
    {
        vector<int> childPatches;
        for (j = 0; j < chldgm->segment_lengths[i]; j++)
            childPatches.push_back(chldgm->segment_data[i][j]);

        vector<int> logExts(6,0);
        int **ijkvar_data = (int **) ijkvar->data;
        logExts[0] = (int) ijkvar_data[0][i];
        logExts[1] = (int) ijkvar_data[2][i];
        logExts[2] = num_dims == 3 ? (int) ijkvar_data[4][i] : 0;
        logExts[3] = (int) ijkvar_data[1][i];
        logExts[4] = (int) ijkvar_data[3][i];
        logExts[5] = num_dims == 3 ? (int) ijkvar_data[5][i] : 0;
        debug5 << "logExts = " << logExts[0] << ", " << logExts[1] << ", " << logExts[2] << endl;
        debug5 << "          " << logExts[3] << ", " << logExts[4] << ", " << logExts[5] << endl;

        dn->SetNestingForDomain(i, levelId[i], childPatches, logExts);
    }

    //
    // Cache the domain nesting object we've just created
    //
    void_ref_ptr vr = void_ref_ptr(dn, avtStructuredDomainNesting::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_NESTING_INFORMATION,
        timestate, -1, vr);

    //
    // Ok, now move on to compute domain boundary information
    //
    bool canComputeNeighborsFromExtents = true;
    avtStructuredDomainBoundaries *sdb = 0;

    if (db_mesh_type == DB_QUAD_RECT)
    {
        sdb = new avtRectilinearDomainBoundaries(canComputeNeighborsFromExtents);
        debug5 << "using rectilinear boundaries" << endl;
    }
    else if(db_mesh_type == DB_QUAD_CURV)
    {
        sdb = new avtCurvilinearDomainBoundaries(canComputeNeighborsFromExtents);
        debug5 << "using curvilinear boundaries" << endl;
    }
    else
    {
        // DB_QUADMESH is ambiguous in this case, we need either
        //  DB_QUAD_RECT or DB_QUAD_CURV.
        //  throw an exception if we end up here to warn a developer.
        ostringstream oss;
        oss << "Could not determine coordinate type for AMR mesh=\""
            << meshName << "\".\n"
            << "Expected: QB_QUAD_RECT (DB_COLLINEAR) or "
            << "DB_QUAD_CURV (DB_NONCOLLINEAR).";
        EXCEPTION1(ImproperUseException, oss.str().c_str());
    }

    sdb->SetNumDomains(num_patches);
    for (int i = 0 ; i < num_patches ; i++)
    {
        int **ijkvar_data = (int **) ijkvar->data;
        int e[6];
        e[0] = (int) ijkvar_data[0][i];
        e[1] = (int) ijkvar_data[1][i]+1;
        e[2] = (int) ijkvar_data[2][i];
        e[3] = (int) ijkvar_data[3][i]+1;
        e[4] = num_dims == 3 ? (int) ijkvar_data[4][i] : 0;
        e[5] = num_dims == 3 ? (int) ijkvar_data[5][i]+1 : 1;
        sdb->SetIndicesForAMRPatch(i, levelId[i], e);
    }
    sdb->CalculateBoundaries();

    //
    // Cache the domain boundary object we've created
    //
    void_ref_ptr vsdb = void_ref_ptr(sdb,avtStructuredDomainBoundaries::Destruct);
    cache->CacheVoidRef("any_mesh", AUXILIARY_DATA_DOMAIN_BOUNDARY_INFORMATION,
        timestate, -1, vsdb);

    if (ratvar)
       DBFreeMrgvar(ratvar);
    if (ijkvar)
       DBFreeMrgvar(ijkvar);
    if (chldgm)
       DBFreeGroupelmap(chldgm);

#endif
}

// ****************************************************************************
//  Function: GetNodesetEnumerationsFromMRGTree
//
//  Purpose: Builds an numerated scalar variable from MRG Tree region names
//  known to define nodesets or facesets
//
//  Creation: Mark C. Miller, Wed Jul 11 11:06:44 PDT 2012
// ****************************************************************************
static void
GetNodesetEnumerationsFromMRGTree(DBfile *dbfile, const DBmrgtnode *nsnode,
    const string& mname, avtDatabaseMetaData *md)
{
    vector<string> nsnames;
    GatherChildMRGTreeRegionNames(dbfile, nsnode, nsnames);

    //
    // Because we use the strings "nodesets" or "facesets" to indicate MRG Tree
    // nodes that define nodesets or facesets we essentially force any writier
    // with multiple meshes in one Silo file to have the same names for the
    // nodesets and facesets variables. To disambiguate them, we need to add
    // the name of the mesh with which they are associated.
    //
    avtScalarMetaData *smd = new avtScalarMetaData(string(nsnode->name)+"_"+mname, mname, AVT_NODECENT);

    for (size_t i = 0; i < nsnames.size(); i++)
        smd->AddEnumNameValue(nsnames[i], i);
    smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
    smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
    smd->hideFromGUI = true;

    md->Add(smd);
}

// ****************************************************************************
//  Function: HandleMrgtreeNodelistVars
//
//  Purpose: Examines an MRG Tree for tell-tale signs that it defines nodesets
//  or facesets.
//
//  Creation: Mark C. Miller, Wed Jul 11 11:06:44 PDT 2012
// ****************************************************************************
static void
HandleMrgtreeNodelistVars(DBfile *dbfile, const string& mname, const string& tname,
    avtDatabaseMetaData *md)
{
    DBmrgtree *mrgt = DBGetMrgtree(dbfile, tname.c_str());
    if (!mrgt)
    {
        debug3 << "Unable to find mrgtree named \"" << tname << "\"" << endl;
        return;
    }

    //
    // Try to go to the nodesets node in the tree
    //
    const char* nsnames[] = {"nodesets", "facesets"};
    for (size_t i = 0; i < sizeof(nsnames)/sizeof(nsnames[0]); i++)
    {
        if (DBSetCwr(mrgt, nsnames[i]) < 0)
        {
            debug3 << "Although mrgtree \"" << tname << "\" exists, "
                   << "it does not contain node named \"" << nsnames[i] << "\"." << endl;
        }
        else
        {
            GetNodesetEnumerationsFromMRGTree(dbfile, mrgt->cwr, mname, md);
            DBSetCwr(mrgt, "..");
        }
    }
}

// ****************************************************************************
//  Function: MultiMatHasAllMatInfo
//
//  Purpose: Return an int indicating if a multi-mat object has all the
//  pieces of information necessary to correctly define a material variable.
//
//  0 ==> don't even know how many materials there are
//  1 ==> know how many materials, but not their numbers, names or colors
//  2 ==> know how many materials and their numbers, but not names or colors
//  3 ==> know how many materials, their numbers and names but not colors
//  4 ==> know everything.
//
//  Programmer: Mark C. Miller 
//  Creation:   March 19, 2009 
//
//  Modifications:
//
//    Hank Childs, Mon May 25 11:07:17 PDT 2009
//    Add support for Silo releases before 4.6.3.
//
// ****************************************************************************

static int
MultiMatHasAllMatInfo(const DBmultimat *const mm)
{
    if (mm->nmatnos <= 0)
        return 0; // has nothing

    if (mm->matnos)
    {
        if (mm->material_names)
        {
            if (mm->matcolors)
                return 4; // has everything
            else
                return 3; // has everything but matcolors
        }
        else
            return 2; // has just enough to be workable
    }
    else
        return 1;
    return 0;
}

// ****************************************************************************
//  Function: ResolveSiloIndObjAbsPath
//
//  Purpose: A fool proof and general mechanism to resolve the absolute path
//  for a Silo object name indirection. Here's the situation. A Silo object is
//  read. That object contains a reference to some other Silo object (e.g. an
//  indirection). How do we ensure we know the absolute path for that indirect
//  object? This funciton will compute it given the Silo file handle which
//  includes the current working directory of the Silo file, the first
//  (or primary) object just read from the file that contains a string which
//  names the indirect object.
//
//  Creation: Mark C. Miller, Wed Jul 11 09:12:59 PDT 2012
//
//  Modifications:
//    Katheen Biagas, Thu Jun 6 13:11:27 PDT 2013
//    Pass "/" as pathSeparator to Absname.
//
// ****************************************************************************

static string ResolveSiloIndObjAbsPath(
    DBfile *dbfile,
    string primary_objname_incl_any_abs_or_rel_path,
    string indirect_objname_incl_any_abs_or_rel_path)
{
    string retval = "unresolved_silo_object_indirection";

    char dbcwd[1024];
    if (DBGetDir(dbfile, dbcwd) < 0)
        return retval;

    // If primary object str is the name of an object as opposed to
    // the dir the object lives in, then compute the dirname
    string obj_abspath = FileFunctions::Absname(dbcwd,
        primary_objname_incl_any_abs_or_rel_path.c_str(), "/");
    int vtype = DBInqVarType(dbfile, obj_abspath.c_str());
    if (vtype >= 0 && vtype != DB_DIR)
    {
        size_t last_slash_idx = obj_abspath.find_last_of('/');
        if (last_slash_idx != std::string::npos)
        {
           if (last_slash_idx == 0) last_slash_idx++;
            obj_abspath.erase(last_slash_idx);
        }
    }

    string indobj_abspath = FileFunctions::Absname(obj_abspath.c_str(),
        indirect_objname_incl_any_abs_or_rel_path.c_str(), "/");
    retval = string(indobj_abspath);
    return retval;
}

// ****************************************************************************
//  Function: FindFirstNonEmptyBlock
//
//  Purpose: Apply various algorithms, in order, to determine the first
//  non-empty block of a multi-block object depending on how the object was
//  constructed.
//
//  Creation: Mark C. Miller, Tue Feb  2 15:17:36 PST 2016
// ****************************************************************************

static int FindFirstNonEmptyBlock(char const *mbobj_name, int nblocks,
    avtSiloMBObjectCacheEntry const *mb_ent,
    int repr_block_idx, int empty_cnt, int const *empty_list)
{
    int const is_all_empty = -1;
    int blocknum = 0;
    if (nblocks == 0)
    {
        return is_all_empty;
    }
    else if (repr_block_idx >= 0)
    {
        if (repr_block_idx < nblocks)
        { 
            blocknum = repr_block_idx;
            return blocknum;
        }
        else
        {
            debug1 << "Multi-block object \"" << mbobj_name
                   << "\" is all empty." << endl;
            return is_all_empty;
        }
    }
    else if (empty_list)
    {
        if (empty_cnt == nblocks)
        {
            debug1 << "Multi-block object \"" << mbobj_name
                   << "\" is all empty." << endl;
            return is_all_empty;
        }
        else
        {
            int qq;
            for (qq = 0; qq < empty_cnt; qq++)
            {
                if (empty_list[qq] != qq)
                    return qq;
            }
            return is_all_empty;
        }
    }
    while (mb_ent->GenerateName(blocknum) == "EMPTY")
    {
        blocknum++;
        if (blocknum >= nblocks)
        {
            debug1 << "Multi-block object \"" << mbobj_name
                   << "\" is all empty." << endl;
            return is_all_empty;
        }
    }
    return blocknum;
}

// Temporary: Replace with DBGetIndex after new Silo is available
static int
db_get_index(DBnamescheme const *ns, int natnum)
{
    char const *name_str = DBGetName(ns, natnum);
    int i = 0;

    if (!name_str) return -1;

    while (name_str[i] && !(strchr("0123456789+-",      name_str[i  ]) &&
                            strchr("0123456789.aAbBcCdDeEfFxX+-", name_str[i+1])))
        i++;

    if (!name_str[i]) return -1;

    return (int) strtol(&name_str[i], 0, 10);
}
